#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "encode_rsi.h"
#include "common.h"


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



