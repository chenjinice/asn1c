#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "rsi_about.h"
#include "msg_common.h"
#include "common.h"


static long s_lng = 0;
static long s_lat = 0;


// ---------------------------------- check ---------------------------------------


// 检查json 文件 alertPath
static int checkAlertPath(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count=0;
    char *pre = getPreSuf(level,keyname);
    char *str = NULL;
    if(json){
        if(jsonArrayRange(json,2,32,pre,keyname)!=0)return ret; // 国标 alertPath 个数: 2 - 32 , optional ， 可以没有
        count = cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        char *key_lng = "lng" ,*key_lat = "lat";
        cJSON *point = cJSON_GetArrayItem(json,i);
        cJSON *lng = cJSON_GetObjectItem(point,key_lng);
        cJSON *lat = cJSON_GetObjectItem(point,key_lat);
        if(jsonIntRange(lng,-LNG_MAX,LNG_MAX,pre,key_lng)!=0)return ret;
        if(jsonIntRange(lat,-LAT_MAX,LAT_MAX,pre,key_lat)!=0)return ret;
        long lng_int = lng->valueint,lat_int = lat->valueint,lng_offset ,lat_offset;
        getOffsetLL(s_lng,s_lat,lng_int,lat_int,&lng_offset,&lat_offset,&str);
        mylog("%s[%d/%d] : lng=%d,lat=%d (%d,%d) (%s)\n",pre,i+1,count,lng_int,lat_int,lng_offset,lat_offset,str);
    }
    return 0;
}

// 检查 rsi 的 json 文件数据是否符合要求
static int rsiJsonCheck(cJSON *json)
{
    int ret = -1,level = 0 , radius_int = 0;
    char *pre = getPreSuf(level,"rsi");

    float radius = 0;
    char *key_pos = "refPos" ,*key_type ="alertType" ,*key_radius = "alertRadius" ,*key_path = "alertPath";
    char *key_des = "description";
    cJSON *refPos           = cJSON_GetObjectItem(json,key_pos);
    cJSON *alertType        = cJSON_GetObjectItem(json,key_type);
    cJSON *alertRadius      = cJSON_GetObjectItem(json,key_radius);
    cJSON *alertPath        = cJSON_GetObjectItem(json,key_path);
    cJSON *description      = cJSON_GetObjectItem(json,key_des);
    if(jsonIntRange(alertType,0,U16_MAX,pre,key_type)!=0)return ret;
    if(jsonDoubleRange(alertRadius,0,ALERTRADIUS_MAX,pre,key_radius)!=0)return ret;
    radius = alertRadius->valuedouble;
	float tmp = radius/ALERTRADIUS_RESOLUTION;
	radius_int = tmp;
    mylog("%s : alertType=%d,alertRadius=%.1fm(%d)",pre,alertType->valueint,radius,radius_int);
    if(description){
        mylog(",description=%s",description->valuestring);
        if(jsonStrLenRange(description,1,256,pre,key_des)!=0)return ret;
    }
    mylog("\n");
    if(refPosJsonCheck(refPos,level+1,key_pos,&s_lng,&s_lat)!=0)return ret;
    if(alertPath){
        if(checkAlertPath(alertPath,level+1,key_path)!=0)return ret;
    }
    mylog("%s\n",pre);
    return 0;
}





// ---------------------------------- add ---------------------------------------

// 添加 alertPath
static void addAlertPath(PathPointList_t *list,cJSON *json)
{
    int i , count =  0;
    if(json)count = cJSON_GetArraySize(json);
    if(count > 0){
        for(i=0;i<count;i++){
            cJSON *point = cJSON_GetArrayItem(json,i);
            PositionOffsetLLV_t *alert_point = (PositionOffsetLLV_t *)calloc(1,sizeof(PositionOffsetLLV_t));
            int lng = cJSON_GetObjectItem(point,"lng")->valueint;
            int lat = cJSON_GetObjectItem(point,"lat")->valueint;
            long lng_offset,lat_offset;
            PositionOffsetLL_PR point_type = getOffsetLL(s_lng,s_lat,lng,lat,&lng_offset,&lat_offset,NULL);
            setOffsetLL(alert_point,lng_offset,lat_offset,point_type);
            ASN_SET_ADD(&list->list,alert_point);
        }
    }else{
        for(i = 0;i<2;i++)
        {
            PositionOffsetLLV_t* alert_point;
            alert_point  = (PositionOffsetLLV_t *)calloc(1,sizeof(PositionOffsetLLV_t));
            alert_point->offsetLL.present = PositionOffsetLL_PR_position_LL1;
            alert_point->offsetLL.choice.position_LL1.lat = 0;
            alert_point->offsetLL.choice.position_LL1.lon = 0;
            ASN_SET_ADD(&list->list,alert_point);
        }
    }

}

// 从文件读取 rsi 的 json 数据， 并用asn编码保存到文件
void rsiEncode(cJSON *json, char *uper_file)
{
    MessageFrame_t *msgframe = NULL;

    // 检查 json 数据是否符合要求
    mylog("---check rsi json start---\n");
    int ret = rsiJsonCheck(json);
    mylog("---check rsi json end---\n");
    if(ret == 0)myok("check json ok\n");
    else{myerr("check json error\n");return;}

    int rsu_id = 0;
    float radius = 0;
    msgframe = (MessageFrame_t*)calloc(1,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_rsiFrame;
    RSI_t *rsi = &msgframe->choice.rsiFrame;
    rsi->msgCnt = 0;
    rsi->id.buf = (uint8_t *)calloc(8,sizeof(uint8_t));
    memcpy(rsi->id.buf,&rsu_id,sizeof(rsu_id));
    rsi->id.size = 8;
    rsi->rsiId = 0;

    cJSON *refPos           = cJSON_GetObjectItem(json,"refPos");
    int alertType           = cJSON_GetObjectItem(json,"alertType")->valueint;
    cJSON *alertRadius      = cJSON_GetObjectItem(json,"alertRadius");
    cJSON *alertPath        = cJSON_GetObjectItem(json,"alertPath");
    cJSON *description      = cJSON_GetObjectItem(json,"description");
    if(alertRadius)radius   = alertRadius->valuedouble;
    rsi->alertType = alertType;  // 警告类型
	float tmp = radius/ALERTRADIUS_RESOLUTION;
    rsi->alertRadius = tmp; //国标分辨率是 10 cm,json文件里边是m,要转换
    rsi->description = addIA5String(description);

    addRefPos(&rsi->refPos,refPos,&s_lng,&s_lat);
    addAlertPath(&rsi->alertPath,alertPath);

    encode(uper_file,msgframe);
    cJSON_Delete(json);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}



// ---------------------------------- print ---------------------------------------


// 打印 alertPath
static void printAlertPath(PathPointList_t *alertpath,int level,int ref_lng,int ref_lat)
{
    int i ,count = 0;
    char *pre = getPreSuf(level,"alertPath");
    count = alertpath->list.count;
    for(i=0;i<count;i++){
        long lng,lat;
        PositionOffsetLLV_t *point = alertpath->list.array[i];
        char *str = getPoint(point,&lng,&lat);
        if(point->offsetLL.present == PositionOffsetLL_PR_position_LatLon){
            mylog("%s[%d/%d] : lng=%ld,lat=%ld (%s)\n",pre,i+1,count,lng,lat,str);
        }else{
            mylog("%s[%d/%d] : lng=%ld,lat=%ld (%ld,%ld) (%s)\n",pre,i+1,count,ref_lng+lng,ref_lat+lat,lng,lat,str);
        }
    }
}

// 打印 rsi 数据
void rsiPrint(MessageFrame_t *msg)
{
    int level = 0;
    char *pre = getPreSuf(level,"rsi");

    RSI_t rsi = msg->choice.rsiFrame;
	int count= rsi.alertPath.list.count;
    mylog("%s : lng=%ld,lat=%ld,alertType=%ld,alertRadius=%.1fm(%ld),alertPath{%d}",pre,
           rsi.refPos.Long,rsi.refPos.lat,rsi.alertType,rsi.alertRadius*ALERTRADIUS_RESOLUTION,rsi.alertRadius,count);
    if(rsi.description)mylog(",%s",rsi.description->buf);
    mylog("\n");
    printAlertPath(&rsi.alertPath,level+1,rsi.refPos.Long,rsi.refPos.lat);

    mylog("%s\n",pre);
}




