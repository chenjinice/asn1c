#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>

#include "common.h"
#include "map_about.h"
#include "rsi_about.h"


static char *s_fix[] = {"│ ","├ "};


// 自己的打印函数
void myprint(int type,const char *format,...)
{
    char str[1024] = {0};
    char *pre = "";
    char *suf = "";
    if(type == MY_ERR){pre = "\e[1;31m";suf = "\e[0m";}
    if(type == MY_OK) {pre = "\e[1;32m";suf = "\e[0m";}
    sprintf(str,"%s%s%s",pre,format,suf);

    va_list ap;
    va_start(ap,format);
    vprintf(str,ap);
    va_end(ap);
}

// 前缀,后缀
char *getPreSuf(int level,char *key)
{
    int i;
    char *format;
    static char pre[10][400] = {0};
    memset(pre[level],0,sizeof(pre[level]));
    for(i=0;i<level;i++){
        if(i== level-1)format = s_fix[1];
        else format = s_fix[0];

        sprintf(pre[level]+strlen(pre[level]),"%s",format);
    }
    sprintf(pre[level]+strlen(pre[level]),"%s",key);
    return pre[level];
}

// 获取文件大小
int getFileSize(char *path)
{
    int ret = -1;
    struct stat statbuff;

    if(stat(path, &statbuff) < 0){
        myerr("%s : get file size fail , %s\n",path,strerror(errno));
        return ret;
    }
    return statbuff.st_size;
}

/*
 * 读取文件内容，并保存到 buffer,
 * 如果读取成功，buffer需要手动释放
 * 返回值： 读取到的数据大小
 */
int readFile(char *path,uint8_t **buffer)
{
    int ret = -1;
    int filesize,fd;
    *buffer = NULL;

    filesize = getFileSize(path);
    if(filesize < 0)return ret;

    fd = open(path,O_RDONLY);
    if(fd == -1){
        myerr("open %s failed\n",path);
        return ret;
    }

    *buffer = (uint8_t*)malloc(filesize);
    int len = read(fd,*buffer,filesize);
    close(fd);
    if(len != filesize){
        myerr("read %s error : len = %d,filesize = %d\n",path,len,filesize);
        free(*buffer);
        *buffer = NULL;
        return ret;
    }
    return filesize;
}

//  把buffer内容保存到文件中
void writeToFile(char *path, uint8_t *buffer, int length)
{
    int fd = open(path,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(fd == -1){
        printf("open %s failed\n",path);
        return;
    }
    int ret = write(fd,buffer,length);
    if(ret != length){
        myerr("write to %s fail : ret = %d,length = %d\n",path,ret,length);
    }else{
        myok("write to %s ok\n",path);
    }
    close(fd);
}

/*
 * 从文件读取数据，并用cJSON解析,
 * 解析成功的话，需要调用 cJSON_Delete 函数释放
 */
cJSON *readJson(char *path)
{
    cJSON *json = NULL;

    uint8_t *buffer;
    readFile(path,&buffer);
    if(!buffer)return NULL;

    json = cJSON_Parse((char *)buffer);
    free(buffer);

    if (!json) {
        myerr("%s : json parse error : [%s]\n",path,cJSON_GetErrorPtr());
    }
    return json;
}

// 读取文件内容，解码
void decodePerFile(char *per_file)
{
    printf("=== decode <%s> ===\n",per_file);

    asn_dec_rval_t rval;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    MessageFrame_t *msg = NULL;

    uint8_t *buffer;
    int len = readFile(per_file,&buffer);
    if(len < 0)return;
    printf("%s : file size = %d\n",per_file,len);

    rval = uper_decode_complete(opt_codec_ctx,&asn_DEF_MessageFrame,(void **)&msg,buffer,len);
    free(buffer);buffer = NULL;
    if(rval.code != RC_OK){
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        myerr("decode fail\n");
        return;
    }else{
        myok("decode OK\n");
    }
//    asn_fprint(stdout,&asn_DEF_MessageFrame,msg);
    switch (msg->present) {
        case MessageFrame_PR_mapFrame:
            mapPrint(msg);
            break;
        case MessageFrame_PR_rsiFrame:
            rsiPrint(msg);
            break;
        default:
            myerr("decode : unknow msg type , msg->present = %d \n",msg->present);
            break;
    }
    ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
}

// asn编码 , 并保存到文件
void encodeJsonFile(char *json_file,char *per_file)
{
    printf("=== encode from <%s> to <%s> ===\n",json_file,per_file);
    if(!per_file){ printf("error : no per_file \n");return; }
    if( strcmp(json_file,per_file) == 0 ){printf("error : the same file\n");return;}

    char * msg_type = (char *)"map";
    cJSON *json = readJson(json_file);
    if(!json)return;

    cJSON *type = cJSON_GetObjectItem(json,"type");
    if(type)msg_type = type->valuestring;

    if(strcmp(msg_type,"map") == 0){
        mapEncode(json,per_file);
    }else if(strcmp(msg_type,"rsi") == 0){
        rsiEncode(json,per_file);
    }else{
        myerr("invalid type : %s \n",msg_type);
    }
}

// asn编码 MessageFrame_t 类型数据，并将编码结果保存到文件
void encode(char *path, MessageFrame_t *msg)
{
    uint8_t buffer[BUFF_SIZE];
    asn_enc_rval_t rval  = uper_encode_to_buffer(&asn_DEF_MessageFrame, NULL, msg, buffer, BUFF_SIZE);
//    printf("encode size = %d \n",rval.encoded);
    if (rval.encoded == -1)
    {
        myerr( "encode fail : error = %s\n",(char *)rval.failed_type->name);
        return;
    }
    if (rval.encoded > BUFF_SIZE) {
        myerr("encode fail : size(%d) > buffer_size(%d) \n",rval.encoded,BUFF_SIZE);
        return;
    }
    myok("encode OK , size = %d\n",rval.encoded);
    writeToFile(path,buffer,rval.encoded);
}



