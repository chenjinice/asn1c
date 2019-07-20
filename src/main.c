#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "MessageFrame.h"
#include "encode.h"
#include "decode.h"

void test()
{
    MessageFrame_t *msgframe = NULL;

    static int ltev_msgcnt = 0;
    ltev_msgcnt++;
    if(ltev_msgcnt > 127)ltev_msgcnt = 0;

    msgframe = (MessageFrame_t*)malloc(sizeof(MessageFrame_t));
    memset(msgframe,0,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_rsiFrame;
    msgframe->choice.rsiFrame.msgCnt = ltev_msgcnt;

    msgframe->choice.rsiFrame.msgCnt = ltev_msgcnt;
    int rsu_id = 1234;
    msgframe->choice.rsiFrame.id.buf = malloc(8);
    memcpy(msgframe->choice.rsiFrame.id.buf,&rsu_id,sizeof(rsu_id));   // rsu_id
    msgframe->choice.rsiFrame.id.size = 8;

    msgframe->choice.rsiFrame.rsiId = 0;
    msgframe->choice.rsiFrame.refPos.lat = 28.11 * 1e7;
    msgframe->choice.rsiFrame.refPos.Long = 127.22 * 1e7;

    msgframe->choice.rsiFrame.alertType = 35;	// 前方施工提醒
    msgframe->choice.rsiFrame.description = calloc(sizeof(IA5String_t),1);

    msgframe->choice.rsiFrame.description->buf =(uint8_t*)calloc(1,sizeof(int));

    msgframe->choice.rsiFrame.description->size = sizeof(int);
    msgframe->choice.rsiFrame.alertRadius = 0;

    {
        int i;
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
    }

    uint8_t pBuf[BUFF_SIZE];
    asn_enc_rval_t EncRVal  = uper_encode_to_buffer(&asn_DEF_MessageFrame, NULL, msgframe, pBuf, BUFF_SIZE);
    printf("encoded = %d\n",EncRVal.encoded);
    if (EncRVal.encoded == -1)
    {
        printf( "encode error : %s\n",(char *)EncRVal.failed_type->name);
        return;
    }
    if (EncRVal.encoded > UINT16_MAX) {
        printf("encode error : %d > %d \n",EncRVal.encoded,UINT16_MAX);
        return;
    }

    write_to_file("map.uper",pBuf,EncRVal.encoded);
    decode_from_file("map.uper");

    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}


void show_useage(){
    printf("Useage : ltev_tool \n");
    printf("-map cfg_file per_file : encode map cfg_file to per_file\n");
    printf("-rsi cfg_file per_file : encode rsi cfg_file to per_file\n");
    printf("-d   per_file : decode per_file\n");
}

int main(int argc,char *argv[])
{
    if( (argc == 3) && (strcmp(argv[1],"-d") == 0) ){
        printf("decode from [%s]\n",argv[2]);
        decode_from_file(argv[2]);
        return 0;
    }

    if(argc == 4){
        if(strcmp(argv[1],"-map") == 0){
            printf("encode map from [%s] to [%s]\n",argv[2],argv[3]);
            return 0;
        }

        if(strcmp(argv[1],"-map") == 0){
            printf("encode rsi from [%s] to [%s]\n",argv[2],argv[3]);
            return 0;
        }
    }

    show_useage();
    return 0;
}




