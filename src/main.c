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
        printf("decode from [%s]\n",argv[2]);
        decode_from_file(argv[2]);
        return 0;
    }

    if(argc == 4){
        if(strcmp(argv[1],"-map") == 0){
            printf("encode map from [%s] to [%s]\n",argv[2],argv[3]);
            encode_map(argv[2],argv[3]);
            return 0;
        }

        if(strcmp(argv[1],"-rsi") == 0){
            printf("encode rsi from [%s] to [%s]\n",argv[2],argv[3]);
            encode_rsi(argv[2],argv[3]);
            return 0;
        }
    }

    show_useage();
    return 0;
}




