#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "common.h"
#include "msg_common.h"




static void show_useage(){
    printf("ltevTool - V2019.12.21\n");
    printf("asn - 国标2017\n");
    printf("  -d perFile : 解码per文件，并打印\n");
    printf("  -l perFile : 解码per文件，并打印,且有asn_fprint详细打印\n");
    printf("  -e jsonFile perFile : 读取json文件，编码并保存到per文件\n");
    printf("修改记录:\n"\
           "  2020.07.27-rsi添加description，修改经纬度点转换的隐藏bug\n"\
           "  2019.12.21-修改map的maneuvers字段(BIT STRING)，之前搞反，需字节倒序\n"\
           );
}



int main(int argc,char *argv[])
{
    int opt;
    char *string = (char *)":e:d:l:";

    while ((opt = getopt(argc, argv, string)) != -1){
        switch (opt) {
        case 'e': encodeJsonFile(optarg,argv[optind]); break;
        case 'd': decodePerFile(optarg,0); break;
        case 'l': decodePerFile(optarg,1); break;
        default : show_useage(); break;
        }
    }

    if(argc < 2)show_useage();

    return 0;
}




