#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "decode.h"
#include "encode.h"
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
