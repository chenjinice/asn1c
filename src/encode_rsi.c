#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "encode_rsi.h"
#include "common.h"



// ---------------------------------- check ---------------------------------------


// 检查json 文件 alertPath
static int check_alertPath(cJSON *alertPath,int level)
{
    int ret = -1,i,lng_max,lat_max,lng_int,lat_int;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    char str[100] = {0};
    get_pre(pre,"alertPath",level);

    int count =  cJSON_GetArraySize(alertPath);
    for(i=0;i<count;i++){
        int point_bits = 0;
        sprintf(log,"%s[%d] : ",pre,i);
        cJSON *point = cJSON_GetArrayItem(alertPath,i);
        cJSON *lng = cJSON_GetObjectItem(point,"lng");
        cJSON *lat = cJSON_GetObjectItem(point,"lat");
        cJSON *bits = cJSON_GetObjectItem(point,"bits");
        if(!lng){printf("%s error : has no lng\n",pre);return ret;}
        if(!lat){printf("%s error : has no lat\n",pre);return ret;}
        if(bits){point_bits = bits->valueint;}
        lng_int = lng->valueint;
        lat_int = lat->valueint;
        if(check_int(lng_int,-LNG_MAX,LNG_MAX,pre,"lng") !=0 )return ret;
        if(check_int(lat_int,-LAT_MAX,LAT_MAX,pre,"lat") !=0 )return ret;
        PositionOffsetLL_PR point_type = get_point_type(lng_int,lat_int,point_bits);
        if(point_type == PositionOffsetLL_PR_NOTHING){printf("%s point type error : nothing\n",pre);return ret;}
        get_type_str(point_type,str);
        sprintf(log+strlen(log),"lng=%d,lat=%d   (%s)",lng_int,lat_int,str);
        printf("%s\n",log);
    }
    return 0;
}

// 检查 rsi 的 json 文件数据是否符合要求
static int check_rsi_json(cJSON *json)
{
    int ret = -1,level = 0;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"rsi",level);

    float radius = 0;
    int path_num = 0,radius_int = 0;
    cJSON *lng = cJSON_GetObjectItem(json,"lng");
    cJSON *lat = cJSON_GetObjectItem(json,"lat");
    cJSON *alertType = cJSON_GetObjectItem(json,"alertType");
    cJSON *alertRadius = cJSON_GetObjectItem(json,"alertRadius");
    cJSON *alertPath = cJSON_GetObjectItem(json,"alertPath");
    if(lng == NULL){printf("error : no lng\n");return ret;}
    if(lat == NULL){printf("error : no lat\n");return ret;}
    if(alertType == NULL){printf("error : no alertType\n");return ret;}
    if(alertRadius)radius = alertRadius->valuedouble;
    if(alertPath)path_num = cJSON_GetArraySize(alertPath);
	float tmp = radius/ALERTRADIUS_RESOLUTION;
	radius_int = tmp;
    printf("%s : lng=%d,lat=%d,alertType=%d,alertRadius=%.1fm(%d),alertPath[%d]\n",pre,
           lng->valueint,lat->valueint,alertType->valueint,radius,radius_int,path_num);

    if(check_int(lng->valueint,-LNG_MAX,LNG_MAX,pre,"lng") != 0)return ret;
    if(check_int(lat->valueint,-LAT_MAX,LAT_MAX,pre,"lat") != 0)return ret;
    if(check_int(alertType->valueint,0,U16_MAX,pre,"alertType") != 0)return ret;
    if(alertRadius)if(check_double(radius,0,ALERTRADIUS_MAX,pre,"alertRadius") != 0)return ret;
    if(path_num > 0){
        if(check_int(path_num,2,32,pre,"alertPath") != 0)return ret;
        if(check_alertPath(alertPath,level+1) != 0)return ret;
    }
    printf("%s\n",pre);
    return 0;
}


// ---------------------------------- add ---------------------------------------

// 添加 alertPath
static void add_alertPath(PathPointList_t *path_list,cJSON *alertPath)
{
    int i;
    int point_num =  0;
    if(alertPath)point_num = cJSON_GetArraySize(alertPath);
    if(point_num > 0){
        for(i=0;i<point_num;i++){
            int point_bits = 0;
            cJSON *point = cJSON_GetArrayItem(alertPath,i);
            PositionOffsetLLV_t *alert_point = calloc(1,sizeof(PositionOffsetLLV_t));
            long lng = cJSON_GetObjectItem(point,"lng")->valueint;
            long lat = cJSON_GetObjectItem(point,"lat")->valueint;
            cJSON *bits = cJSON_GetObjectItem(point,"bits");
            if(bits)point_bits = bits->valueint;
            PositionOffsetLL_PR point_type = get_point_type((int)lng,(int)lat,point_bits);
            alert_point->offsetLL.present = PositionOffsetLL_PR_position_LatLon;
            alert_point->offsetLL.choice.position_LatLon.lon = lng;
            alert_point->offsetLL.choice.position_LatLon.lat = lat;
            set_point(alert_point,lng,lat,point_type);
            ASN_SET_ADD(&path_list->list,alert_point);
        }
    }else{
        for(i = 0;i<2;i++)
        {
            PositionOffsetLLV_t* alert_point;
            alert_point  =calloc(1,sizeof(PositionOffsetLLV_t));
            alert_point->offsetLL.present = PositionOffsetLL_PR_position_LL1;
            alert_point->offsetLL.choice.position_LL1.lat = 0;
            alert_point->offsetLL.choice.position_LL1.lon = 0;
            ASN_SET_ADD(&path_list->list,alert_point);
        }
    }

}

// 从文件读取 rsi 的 json 数据， 并用asn编码保存到文件
void encode_rsi(cJSON *json, char *uper_file)
{
    MessageFrame_t *msgframe = NULL;

    char *pre = "——————————";
    // 检查 json 数据是否符合要求
    printf("%s check rsi json %s\n",pre,pre);
    int ret = check_rsi_json(json);
    if(ret == 0)printf("%s check rsi json \e[32;40mOK\e[0m %s\n",pre,pre);
    else{printf("%s check rsi json \e[31;40mFAIL\e[0m %s\n",pre,pre);return;}

    int rsu_id = 0;
    float radius = 0;
    int lng = cJSON_GetObjectItem(json,"lng")->valueint;
    int lat = cJSON_GetObjectItem(json,"lat")->valueint;
    int alertType = cJSON_GetObjectItem(json,"alertType")->valueint;
    cJSON *alertRadius = cJSON_GetObjectItem(json,"alertRadius");
    cJSON *alertPath = cJSON_GetObjectItem(json,"alertPath");
    if(alertRadius)radius = alertRadius->valuedouble;

    msgframe = (MessageFrame_t*)malloc(sizeof(MessageFrame_t));
    memset(msgframe,0,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_rsiFrame;

    RSI_t *rsi = &msgframe->choice.rsiFrame;
    rsi->msgCnt = 0;
    rsi->id.buf = malloc(8);
    memcpy(rsi->id.buf,&rsu_id,sizeof(rsu_id));
    rsi->id.size = 8;
    rsi->rsiId = 0;
    rsi->refPos.lat = lat;       // 纬度
    rsi->refPos.Long = lng;      // 经度
    rsi->alertType = alertType;  // 警告类型
	float tmp = radius/ALERTRADIUS_RESOLUTION;
    rsi->alertRadius = tmp; //国标分辨率是 10 cm,json文件里边是m,要转换
    add_alertPath(&rsi->alertPath,alertPath);

    encode(uper_file,msgframe);
    cJSON_Delete(json);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}



// ---------------------------------- print ---------------------------------------


// 打印 alertPath
static void print_alertPath(PathPointList_t *alertpath,int level)
{
    int i;
    long lng,lat;
    char pre[PRE_SIZE] = {0};
    char str[100] = {0};
    get_pre(pre,"alertPath",level);

    int count = alertpath->list.count;
    for(i=0;i<count;i++){
        PositionOffsetLLV_t *point = alertpath->list.array[i];
        get_point(point,&lng,&lat);
        get_type_str(point->offsetLL.present,str);

        printf("%s[%d] : lng=%ld,lat=%ld   (%s)\n",pre,i,lng,lat,str);
    }
}

// 打印 rsi 数据
void print_rsi(MessageFrame_t *msg)
{
    int level = 0;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"rsi",level);

    RSI_t rsi = msg->choice.rsiFrame;
	int count= rsi.alertPath.list.count;
    printf("%s : lng=%ld,lat=%ld,alertType=%ld,alertRadius=%.1fm(%ld),alertPath[%d]\n",pre,
           rsi.refPos.Long,rsi.refPos.lat,rsi.alertType,rsi.alertRadius*ALERTRADIUS_RESOLUTION,rsi.alertRadius,count);

    print_alertPath(&rsi.alertPath,level+1);

    printf("%s\n",pre);
}




