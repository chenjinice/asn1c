#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "common.h"
#include "convert_common.h"


static void show_useage(){
    printf("ltevTool - V2021.05.06\n");
    printf("asn - 国标2020新四跨版本\n");
    printf("  -d perFile : 解码per文件\n");
    printf("  -e jsonFile perFile : 读取json文件，编码并保存到per文件\n");
    printf("修改记录:\n"\
           "  2021.05.06-rsi添加description，修改经纬度点转换的隐藏bug\n"\
           );
}



int main(int argc,char *argv[])
{
    int opt;
    char *string = (char *)":e:d:l:";

    /*
    while ((opt = getopt(argc, argv, string)) != -1){
        switch (opt) {
        case 'e': encodeJsonFile(optarg,argv[optind]); break;
        case 'd': decodePerFile(optarg); break;
        case 'l': decodePerFile(optarg); break;
        default : show_useage(); break;
        }
    }
*/

    if(argc < 2)show_useage();

    return 0;
}
