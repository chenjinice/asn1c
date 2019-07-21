#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "common.h"
#include "MessageFrame.h"


static print_rsi(MessageFrame_t *msg)
{
    printf("print rsi\n");
    RSI_t rsi = msg->choice.rsiFrame;
    printf("lng = %ld,lat = %ld\n",rsi.refPos.Long,rsi.refPos.lat);
}


static print_map(MessageFrame_t *msg)
{
    printf("print map\n");
    MAP_t map = msg->choice.mapFrame;
}

void decode_from_file(char *path)
{
    asn_dec_rval_t rval;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    MessageFrame_t *msg = NULL;

    int fd = open(path,O_RDONLY);
    if(fd == -1){
        printf("decode : open [%s] failed\n",path);
        return;
    }
    uint8_t buffer[BUFF_SIZE] = {0};
    int ret = read(fd,buffer,BUFF_SIZE);
    close(fd);

    if(ret <= 0){
        printf("decode error : read length = %d\n",ret);
        return;
    }

    rval = uper_decode_complete(opt_codec_ctx,&asn_DEF_MessageFrame,(void **)&msg,buffer,ret);
    if(rval.code != RC_OK){
        printf("decode faild\n");
        return;
    }

    switch (msg->present) {
        case MessageFrame_PR_mapFrame:
            break;
        case MessageFrame_PR_rsiFrame:
            print_rsi(msg);
            break;
        default:
            printf("print nothing : msg->present = %d\n",msg->present);
            break;
    }
}

cJSON *read_json(char *path)
{
    char *data = NULL;
    long filesize = -1;
    struct stat statbuff;
    int fd;
    cJSON *json = NULL;

    if(stat(path, &statbuff) < 0){
        printf("%s : get file size error \n",path);
        return json;
    }else{
        filesize = statbuff.st_size;
    }

    fd = open(path,O_RDONLY);
    if(fd == -1){
        printf("open %s failed\n",path);
        return json;
    }

    data=(char*)malloc(filesize);
    int ret = read(fd,data,filesize);
    close(fd);
    if(ret != filesize){
        printf("read %s error : ret = %d,filesize = %ld\n",path,ret,filesize);
        free(data);
        return json;
    }

    json = cJSON_Parse(data);
    if (!json) {
        printf("json parse error : [%s]\n",cJSON_GetErrorPtr());
        free(data);
        return json;
    }

    free(data);
    return json;
}


void encode(char *path, MessageFrame_t *msg)
{
    uint8_t buffer[BUFF_SIZE];
    asn_enc_rval_t rval  = uper_encode_to_buffer(&asn_DEF_MessageFrame, NULL, msg, buffer, BUFF_SIZE);
    printf("encode size = %d\n",rval.encoded);
    if (rval.encoded == -1)
    {
        printf( "encode error : %s\n",(char *)rval.failed_type->name);
        return;
    }
    if (rval.encoded > UINT16_MAX) {
        printf("encode error : %d > %d \n",rval.encoded,UINT16_MAX);
        return;
    }
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msg);

    write_to_file(path,buffer,BUFF_SIZE);
}


void write_to_file(char *path, uint8_t *buffer, int length)
{
    int fd = open(path,O_WRONLY|O_CREAT,0666);
    if(fd == -1){
        printf("open %s failed\n",path);
        return;
    }
    int ret = write(fd,buffer,length);
    if(ret != length){
        printf("write to file error : ret = %d,length = %d\n",ret,length);
    }else{
        printf("write to %s success !\n",path);
    }
    close(fd);
}


