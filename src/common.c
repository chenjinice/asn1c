#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"
#include "MessageFrame.h"


int get_file_size(char *path)
{
    int ret = -1;
    struct stat statbuff;

    if(stat(path, &statbuff) < 0){
        printf("%s : get file size error \n",path);
        perror("error ");
        return ret;
    }
    return statbuff.st_size;
}

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


MessageFrame_t *decode(char *path)
{
    asn_dec_rval_t rval;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    MessageFrame_t *msg = NULL;

    uint8_t *buffer;
    int len = read_file(path,&buffer);
    if(!buffer)return msg;

    rval = uper_decode_complete(opt_codec_ctx,&asn_DEF_MessageFrame,(void **)&msg,buffer,len);
    free(buffer);
    if(rval.code != RC_OK){
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        msg = NULL;
        printf("decode \e[31;40mfail\e[0m\n");
    }
    return msg;
}

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
        printf("encode \e[31;40mfail\e[0m : size(%d) > %d \n",rval.encoded,BUFF_SIZE);
        return;
    }
    printf("encode \e[32;40mOK\e[0m \n");
    write_file(path,buffer,rval.encoded);
}




