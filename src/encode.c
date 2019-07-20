#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "encode.h"
#include "MessageFrame.h"
#include "cJSON.h"

static cJSON *read_json(char *path)
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
    if(ret != filesize){
        printf("read %s error : ret = %d,filesize = %d\n",ret,filesize);
        free(data);
        return json;
    }

    json = cJSON_Parse(data);
    if (!json) {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
        free(data);
        return json;
    }

    free(data);
    return json;
}

static void write_to_file(char *path, uint8_t *buffer, int length)
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


static void encode(char *path, MessageFrame_t *msg)
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


void encode_rsi(char *json_file, char *uper_file)
{
    MessageFrame_t *msgframe = NULL;
    int i;
    int rsu_id = 0; // rsu_id

    cJSON *json = read_json(json_file);
    if(json == NULL)return;

    msgframe = (MessageFrame_t*)malloc(sizeof(MessageFrame_t));
    memset(msgframe,0,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_rsiFrame;
    msgframe->choice.rsiFrame.msgCnt = 0;
    msgframe->choice.rsiFrame.id.buf = malloc(8);
    memcpy(msgframe->choice.rsiFrame.id.buf,&rsu_id,sizeof(rsu_id));
    msgframe->choice.rsiFrame.id.size = 8;
    msgframe->choice.rsiFrame.rsiId = 0;
    msgframe->choice.rsiFrame.refPos.lat = 44.44 * 1e7;
    msgframe->choice.rsiFrame.refPos.Long = 111.11 * 1e7;
    msgframe->choice.rsiFrame.alertType = 35;	// 前方施工提醒
    msgframe->choice.rsiFrame.description = calloc(sizeof(IA5String_t),1);
    msgframe->choice.rsiFrame.description->buf =(uint8_t*)calloc(1,sizeof(int));
    msgframe->choice.rsiFrame.description->size = sizeof(int);
    msgframe->choice.rsiFrame.alertRadius = 0;

    for(i = 0;i<2;i++)
    {
        struct PositionOffsetLLV* ptemp_alertPath;
        ptemp_alertPath  =calloc(sizeof(struct PositionOffsetLLV),1);
        memset(ptemp_alertPath,0,sizeof(struct PositionOffsetLLV));
        ptemp_alertPath->offsetLL.present = PositionOffsetLL_PR_position_LL6;
        ptemp_alertPath->offsetLL.choice.position_LL6.lat = 0;
        ptemp_alertPath->offsetLL.choice.position_LL6.lon = 0;
        ASN_SET_ADD(&msgframe->choice.rsiFrame.alertPath.list,ptemp_alertPath);
    }

    encode(uper_file,msgframe);
}

void encode_map(char *json_file, char *uper_file)
{
    MessageFrame_t *msgframe = NULL;
    cJSON *json = read_json(json_file);

    if(json == NULL)return;

}

