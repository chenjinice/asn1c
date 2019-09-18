#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include "common.h"
#include "encode_map.h"
#include "encode_rsi.h"


//
static void show_useage(){
    printf("version : 2019.09.18\n");
    //解码per文件
//    printf("-d perFile : decode perFile\n");
	printf("-d perFile : 解码per文件，并打印\n");
    // 从json文件编码并保存到per文件
//    printf("-e jsonFile perFile : encode from jsonFile to perFile\n");
	printf("-e jsonFile perFile : 读取json文件，编码并保存到per文件\n");
}


// 读取文件内容，解码
static void decode_file(char *per_file)
{
    asn_dec_rval_t rval;
    asn_codec_ctx_t *opt_codec_ctx = 0;
    MessageFrame_t *msg = NULL;

    uint8_t *buffer;
    int len = read_file(per_file,&buffer);
    if(len < 0)return;
    printf("%s : file size = %d\n",per_file,len);

    rval = uper_decode_complete(opt_codec_ctx,&asn_DEF_MessageFrame,(void **)&msg,buffer,len);
    free(buffer);buffer = NULL;
    if(rval.code != RC_OK){
        ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
        printf("decode \e[31;40mfail\e[0m\n");
        return;
    }else{
        printf("decode \e[32;40mOK\e[0m\n");
    }
//    asn_fprint(stdout,&asn_DEF_MessageFrame,msg);
    switch (msg->present) {
        case MessageFrame_PR_mapFrame:
            print_map(msg);
            break;
        case MessageFrame_PR_rsiFrame:
            print_rsi(msg);
            break;
        default:
            printf("decode : unknow msg type , msg->present = %d \n",msg->present);
            break;
    }
    ASN_STRUCT_FREE(asn_DEF_MessageFrame,msg);
}

// asn编码 , 并保存到文件
void encode_file(char *json_file,char *per_file)
{
    char * msg_type = "map";
    cJSON *json = read_json(json_file);
    if(!json)return;

    cJSON *type = cJSON_GetObjectItem(json,"type");
    if(type)msg_type = type->valuestring;

    if(strcmp(msg_type,"map") == 0){
        encode_map(json,per_file);
    }else if(strcmp(msg_type,"rsi") == 0){
        encode_rsi(json,per_file);
    }else{
        printf("invalid type : %s \n",msg_type);
    }
}


int main(int argc,char *argv[])
{
    int opt;
    char *string = ":e:d:";

    while ((opt = getopt(argc, argv, string)) != -1){
        char *per_file = NULL, *json_file = NULL;
        switch (opt) {
            case 'e':
                json_file = optarg;
                per_file  = argv[optind];
                if(json_file && per_file){
                    printf("=== encode from <%s> to <%s> ===\n",json_file,per_file);
                    if( strcmp(json_file,per_file) == 0 ){printf("error : the same file\n");return 0;}
                    encode_file(json_file,per_file);
                    return 0;
                }
                break;
            case 'd':
                per_file = optarg;
                if(per_file){
                    printf("=== decode <%s> ===\n",per_file);
                    decode_file(per_file);
                    return 0;
                }
                break;
        }
    }
    show_useage();
    return 0;
}




