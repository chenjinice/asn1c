#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "encode_map.h"
#include "encode_rsi.h"


void show_useage(){
    printf("Useage : ltev_tool \n");
    printf("-d   per_file : decode per_file\n");
    printf("-map json_file per_file : encode map json_file to per_file\n");
    printf("-rsi json_file per_file : encode rsi json_file to per_file\n");

}

int main(int argc,char *argv[])
{
    if( (argc == 3) && (strcmp(argv[1],"-d") == 0) ){
        printf("=== decode from [%s] ===\n",argv[2]);
        MessageFrame_t * msg = decode(argv[2]);
        if(!msg)return 0;
        switch (msg->present) {
            case MessageFrame_PR_mapFrame:
                printf("=== map ===\n");
                print_map(msg);
                break;
            case MessageFrame_PR_rsiFrame:
                printf("=== rsi ===\n");
                print_rsi(msg);
                break;
            default:
                printf("=== ??? === :unknow type , msg->present = %d\n",msg->present);
                break;
        }
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        return 0;
    }

    if(argc == 4){
        if(strcmp(argv[2],argv[3]) == 0){
            printf("=== stop : same file name:[%s = %s] ===\n",argv[2],argv[3]);
            return 0;
        }
        if(strcmp(argv[1],"-map") == 0){
            printf("=== encode map from [%s] to [%s] ===\n",argv[2],argv[3]);
            encode_map(argv[2],argv[3]);
            return 0;
        }
        if(strcmp(argv[1],"-rsi") == 0){
            printf("=== encode rsi from [%s] to [%s] ===\n",argv[2],argv[3]);
            encode_rsi(argv[2],argv[3]);
            return 0;
        }
    }

    show_useage();
    return 0;
}




