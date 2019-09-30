#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "msg_common.h"




static void show_useage(){
    printf("ltevTool - V2019.09.30\n");
    printf("asn - 国标2017\n");
    printf("  -d perFile : 解码per文件，并打印\n");
    printf("  -e jsonFile perFile : 读取json文件，编码并保存到per文件\n");
}



int main(int argc,char *argv[])
{
    int opt;
    char *string = (char *)":e:d:";

    while ((opt = getopt(argc, argv, string)) != -1){
        switch (opt) {
            case 'e': encodeJsonFile(optarg,argv[optind]); break;
            case 'd': decodePerFile(optarg); break;
            default : show_useage(); break;
        }
    }

    if(argc < 2)show_useage();

    return 0;
}




