#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "encode_rsi.h"
#include "common.h"

static int check_rsi_json(cJSON *json)
{
    int ret = -1;
    cJSON *lon = cJSON_GetObjectItem(json,"lon");
    cJSON *lat = cJSON_GetObjectItem(json,"lat");
    cJSON *alertType = cJSON_GetObjectItem(json,"alertType");
    if(lon == NULL){printf("error : no lon\n");return ret;}
    if(lat == NULL){printf("error : no lat\n");return ret;}
    if(alertType == NULL){printf("error : no alertType\n");return ret;}
    printf("lon = %d \n",lon->valueint);
    printf("lat = %d \n",lat->valueint);
    printf("alertType = %d\n",alertType->valueint);
    return 0;
}

void encode_rsi(char *json_file, char *uper_file)
{
    int i;
    MessageFrame_t *msgframe = NULL;
    cJSON *json = read_json(json_file);
    if(!json)return;

    printf("———————— check rsi json start ————————\n");
    int ret = check_rsi_json(json);
    if(ret == 0)printf("———————— check rsi json \e[32;40mOK\e[0m ————————\n");
    else{printf("———————— check rsi json \e[31;40mFAIL\e[0m ————————\n");return;}

    int lon = cJSON_GetObjectItem(json,"lon")->valueint;
    int lat = cJSON_GetObjectItem(json,"lat")->valueint;
    int alertType = cJSON_GetObjectItem(json,"alertType")->valueint;
    int rsu_id = 0;

    msgframe = (MessageFrame_t*)malloc(sizeof(MessageFrame_t));
    memset(msgframe,0,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_rsiFrame;
    msgframe->choice.rsiFrame.msgCnt = 0;
    msgframe->choice.rsiFrame.id.buf = malloc(8);
    memcpy(msgframe->choice.rsiFrame.id.buf,&rsu_id,sizeof(rsu_id));
    msgframe->choice.rsiFrame.id.size = 8;
    msgframe->choice.rsiFrame.rsiId = 0;
    msgframe->choice.rsiFrame.refPos.lat = lat;
    msgframe->choice.rsiFrame.refPos.Long = lon;
    msgframe->choice.rsiFrame.alertType = alertType;
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

    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}

void print_rsi(MessageFrame_t *msg)
{
    RSI_t rsi = msg->choice.rsiFrame;
    printf("lon = %ld \n",rsi.refPos.Long);
    printf("lat = %ld \n",rsi.refPos.lat);
    printf("alertType = %ld\n",rsi.alertType);
}

