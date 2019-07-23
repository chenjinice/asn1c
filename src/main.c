#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include "encode_map.h"
#include "encode_rsi.h"


void show_useage(){
    printf("ltev_tool :  \n");
    // 从文件解码
    printf("-d    file : decode from file\n");
    // 从json文件编码 map，并保持到文件
    printf("-map  json_file per_file : encode map from json_file to per_file\n");
    // 从json文件编码 rsi，并保持到文件
    printf("-rsi  json_file per_file : encode rsi from json_file to per_file\n");

}

int main(int argc,char *argv[])
{
    if( (argc == 3) && (strcmp(argv[1],"-d") == 0) ){
        printf("=== decode from <%s> ===\n",argv[2]);
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
                printf("=== ??? === : unknow type , msg->present = %d\n",msg->present);
                break;
        }
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        return 0;
    }

    if(argc == 4){
        if(strcmp(argv[2],argv[3]) == 0){
            printf("=== error : same file ( from <%s> to <%s> ) ===\n",argv[2],argv[3]);
            return 0;
        }
        if(strcmp(argv[1],"-map") == 0){
            printf("=== encode map from <%s> to <%s> ===\n",argv[2],argv[3]);
            encode_map(argv[2],argv[3]);
            return 0;
        }
        if(strcmp(argv[1],"-rsi") == 0){
            printf("=== encode rsi from <%s> to <%s> ===\n",argv[2],argv[3]);
            encode_rsi(argv[2],argv[3]);
            return 0;
        }
    }

    show_useage();
    return 0;
}




