#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"
#include "MessageFrame.h"



// 打印消息前缀
void get_pre(char *pre,char *name,int level)
{
    int i;
    char *format = "│ ";
    char *format_last = "├ ";
    pre[0] = 0;
    for(i=0;i<level;i++){
        if(i== level-1)sprintf(pre+strlen(pre),"%s",format_last);
        else sprintf(pre+strlen(pre),"%s",format);
    }
    sprintf(pre+strlen(pre),"%s",name);
}

// 判断整数是否在范围内
int check_int(int num ,int min,int max,char *pre,char *name)
{
    if( (num < min) || (num > max) ){
        printf("%s.%s error : value = %d, must be (%d ~ %d)\n",pre,name,num,min,max);
        return -1;
    }else{
        return 0;
    }
}

// 判断浮点数是否在范围内
int check_double(double num ,double min,double max,char *pre,char *name)
{
    if( (num < min) || (num > max) ){
        printf("%s.%s error : value = %lf, must be (%.2lf ~ %.2lf)\n",pre,name,num,min,max);
        return -1;
    }else{
        return 0;
    }
}


// 获取文件大小
int get_file_size(char *path)
{
    int ret = -1;
    struct stat statbuff;

    if(stat(path, &statbuff) < 0){
        printf("%s : get file status fail\n",path);
        perror("error ");
        return ret;
    }
    return statbuff.st_size;
}


/*
 * 读取文件内容，并保存到 buffer,
 * 如果读取成功，buffer需要手动释放
 * 返回值： 读取到的数据大小
 */
int read_file(char *path,uint8_t **buffer)
{
    int ret = -1;
    int filesize,fd;
    *buffer = NULL;

    filesize = get_file_size(path);
    if(filesize < 0)return ret;
    if(filesize == 0){printf("%s : empty file\n",path);return ret;}

    fd = open(path,O_RDONLY);
    if(fd == -1){
        printf("open %s failed\n",path);
        return ret;
    }

    *buffer = (char*)malloc(filesize);
    int len = read(fd,*buffer,filesize);
    close(fd);
    if(len != filesize){
        printf("read %s error : len = %d,filesize = %d\n",path,len,filesize);
        free(buffer);
        return ret;
    }
    return filesize;
}


//  把buffer内容保存到文件中
void write_file(char *path, uint8_t *buffer, int length)
{
    int fd = open(path,O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(fd == -1){
        printf("open %s failed\n",path);
        return;
    }
    int ret = write(fd,buffer,length);
    if(ret != length){
        printf("write to %s \e[31;40mfail\e[0m : ret = %d,length = %d\n",path,ret,length);
    }else{
        printf("write to %s \e[32;40mOK\e[0m\n",path);
    }
    close(fd);
}

/*
 * 从文件读取数据，并用cJSON解析,
 * 解析成功的话，需要调用 cJSON_Delete 函数释放
 */
cJSON *read_json(char *path)
{
    cJSON *json = NULL;

    uint8_t *buffer;
    read_file(path,&buffer);
    if(!buffer)return;

    json = cJSON_Parse((char *)buffer);
    free(buffer);

    if (!json) {
        printf("%s : json parse error : [%s]\n",path,cJSON_GetErrorPtr());
    }
    return json;
}


/*
 * 读取文件内容，并用asn解码函数解码
 * 解码成功的话，需要调用 ASN_STRUCT_FREE 函数释放
 */
MessageFrame_t *decode(char *path)
{
    asn_dec_rval_t rval;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    MessageFrame_t *msg = NULL;

    uint8_t *buffer;
    int len = read_file(path,&buffer);
    printf("%s : file size = %d\n",path,len);
    if(!buffer)return msg;

    rval = uper_decode_complete(opt_codec_ctx,&asn_DEF_MessageFrame,(void **)&msg,buffer,len);
    free(buffer);
    if(rval.code != RC_OK){
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        msg = NULL;
        printf("decode \e[31;40mfail\e[0m\n");
    }else{
        printf("decode \e[32;40mOK\e[0m\n");
    }
    return msg;
}

// asn编码 MessageFrame_t 类型数据，并将编码结果保存到文件
void encode(char *path, MessageFrame_t *msg)
{
    uint8_t buffer[BUFF_SIZE];
    asn_enc_rval_t rval  = uper_encode_to_buffer(&asn_DEF_MessageFrame, NULL, msg, buffer, BUFF_SIZE);
    printf("encode size = %d , buffer_size = %d\n",rval.encoded,BUFF_SIZE);
    if (rval.encoded == -1)
    {
        printf( "encode \e[31;40mfail\e[0m : error = %s\n",(char *)rval.failed_type->name);
        return;
    }
    if (rval.encoded > BUFF_SIZE) {
        printf("encode \e[31;40mfail\e[0m : size(%d) > buffer_size(%d) \n",rval.encoded,BUFF_SIZE);
        return;
    }
    printf("encode \e[32;40mOK\e[0m \n");
    write_file(path,buffer,rval.encoded);
}




