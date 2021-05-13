#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>

#include "map_about.h"
#include "convert_common.h"
#include "common.h"


static long s_lng = 0;
static long s_lat = 0;


//---------------------- check -----------------------------


// 检查 json 数据是否符合 lanes 中的 points 要求
static int pointsJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count=0;
    char *pre = getPreSuf(level,keyname);
    char *str = nullptr;
    if(json){
        if(jsonArrayRange(json,2,31,pre,keyname)!=0)return ret; // 国标 points 个数: 2 - 31 , optional ， 可以没有
        count = cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        char *key_lng = (char *)"lng" ,*key_lat = (char *)"lat";
        cJSON *point = cJSON_GetArrayItem(json,i);
        cJSON *lng = cJSON_GetObjectItem(point,key_lng);
        cJSON *lat = cJSON_GetObjectItem(point,key_lat);
        if(jsonIntRange(lng,-LNG_MAX,LNG_MAX,pre,key_lng)!=0)return ret;
        if(jsonIntRange(lat,-LAT_MAX,LAT_MAX,pre,key_lat)!=0)return ret;
        long lng_int = lng->valueint,lat_int = lat->valueint,lng_offset ,lat_offset;
        getOffsetLL(s_lng,s_lat,lng_int,lat_int,&lng_offset,&lat_offset,&str);
        mylog("%s*[%d/%d] : lng=%d,lat=%d (%d,%d) (%s)\n",pre,i+1,count,lng_int,lat_int,lng_offset,lat_offset,str);
    }
    return 0;
}

// 检查 json 数据是否符合 lanes 中的 connectsTo 要求
static int connectsToJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(json){
        if(jsonArrayRange(json,1,8,pre,keyname)!=0)return ret;  // 国标 connectsTo 个数: 1 - 8 , optional ， 可以没有
        count =  cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        char *key_remote = (char *)"remoteIntersection",*key_con =(char *)"connectingLane",*key_phase = (char *)"phaseId";
        cJSON *con = cJSON_GetArrayItem(json,i);
        cJSON *remoteIntersection = cJSON_GetObjectItem(con,key_remote);
        cJSON *connectingLane = cJSON_GetObjectItem(con,key_con);
        cJSON *phaseId = cJSON_GetObjectItem(con,key_phase);
        mylog("%s[%d/%d] : ",pre,i+1,count,remoteIntersection->valueint);
        if(connectingLane)mylog("connectingLane*=%d,",connectingLane->valueint);
        if(phaseId)mylog("phaseId*=%d",phaseId->valueint);
        mylog("\n");

        if(nodeRefIDJsonCheck(remoteIntersection,level+1,key_remote)!=0)return ret;
        if(connectingLane){
            if(jsonIntRange(connectingLane,LANEID_MIN,LANEID_MAX,pre,key_con) != 0)return ret;
        }
        if(phaseId){
            if(jsonIntRange(phaseId,PHASEID_MIN,PHASEID_MAX,pre,key_phase) != 0)return ret;
        }
    }
    return 0;
}

// 检查 json 数据是否符合 list 中的 lanes 要求
static int lanesJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(jsonArrayRange(json,1,32,pre,keyname)!=0)return ret;  // 国标 lanes 个数: 1 - 32
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        char *key_laneid = (char *)"laneID" ,*key_points = (char *)"points" ,*key_con = (char *)"connectsTo" ;
        char *key_mane = (char *)"maneuvers",*key_width = (char *)"laneWidth";
        cJSON *lane = cJSON_GetArrayItem(json,i);
        cJSON *laneID = cJSON_GetObjectItem(lane,key_laneid);
        cJSON *points = cJSON_GetObjectItem(lane,key_points);
        cJSON *connectsTo = cJSON_GetObjectItem(lane,key_con);
        cJSON *maneuvers = cJSON_GetObjectItem(lane,key_mane);
        cJSON *laneWidth = cJSON_GetObjectItem(lane,key_width);
        if(jsonIntRange(laneID,LANEID_MIN,LANEID_MAX,pre,key_laneid)!=0)return ret;
        if(laneWidth){
            if(jsonIntRange(laneWidth,0,LANEWIDTH_MAX,pre,key_width)!=0)return ret;
        }
        if(maneuvers){
            if(jsonIntRange(maneuvers,0,4095,pre,key_mane)!=0)return ret;
        }
        mylog("%s[%d/%d] : laneID=%d",pre,i+1,count,laneID->valueint);
        if(laneWidth)mylog(",laneWidth*=%d",laneWidth->valueint);
        if(maneuvers)mylog(",maneuvers*=%d",maneuvers->valueint);
        mylog("\n");
        if(connectsTo){
            if(connectsToJsonCheck(connectsTo,level+1,key_con)!=0)return ret;
        }
        if(points){
            if(pointsJsonCheck(points,level+1,key_points)!=0)return ret;
        }
        mylog("%s[%d/%d]\n",pre,i+1,count);
    }
    return 0;
}

// 检查 json 数据是否符合 list 中的 speedlimits 要求
static int speedLimitsJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(json){
        if(jsonArrayRange(json,1,9,pre,keyname)!=0)return ret; // 国标 speedLimits 个数: 1 - 9 , optional ， 可以没有
        count =  cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        char *key_type = (char *)"type" ,*key_speed = (char *)"speed";
        cJSON *limit = cJSON_GetArrayItem(json,i);
        cJSON *type = cJSON_GetObjectItem(limit,key_type);
        cJSON *speed = cJSON_GetObjectItem(limit,key_speed);
        if(jsonIntRange(type,SPEEDTYPE_MIN,SPEEDTYPE_MAX,pre,key_type)!=0)return ret;
        if(jsonDoubleRange(speed,SPEED_MIN,SPEED_MAX,pre,key_speed)!=0)return ret;

        int type_int = type->valueint;
        double speed_double = speed->valuedouble;
        double tmp = (speed_double/SPEED_RESOLUTION);   // 需要搞个中间值，要不算出来的 speed_int 不对
        int speed_int = tmp;
        mylog("%s*[%d/%d] : type=%d,speed=%.2lfm/s(%d)\n",pre,i+1,count,type_int,speed_double,speed_int);
    }
    return 0;
}

// 检查 json 数据是否符合 link 中的 movements 要求
static int movementsJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(json){
        if(jsonArrayRange(json,1,32,pre,keyname)!=0)return ret; // 国标 movements 个数: 1 - 32,  optional ， 可以没有
        count =  cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        char *key_remote = (char *)"remoteIntersection" , *key_phase = (char *)"phaseId";
        cJSON *move = cJSON_GetArrayItem(json,i);
        cJSON *remoteIntersection = cJSON_GetObjectItem(move,key_remote);
        cJSON *phaseId = cJSON_GetObjectItem(move,key_phase);
        if(phaseId){
            if(jsonIntRange(phaseId,PHASEID_MIN,PHASEID_MAX,pre,key_phase)!=0)return ret;
        }
        mylog("%s*[%d/%d] : ",pre,i+1,count);
        if(phaseId)mylog("phaseId*=%d",phaseId->valueint);
        mylog("\n");
        if(nodeRefIDJsonCheck(remoteIntersection,level+1,key_remote)!=0)return ret;
    }
    return 0;
}

// 检查 json 数据是否符合 node 中的 links 要求
static int linksJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(json){
        if(jsonArrayRange(json,1,32,pre,keyname)!=0)return ret; // 国标 links 个数: 1 - 32 , optional ， 可以没有
        count =  cJSON_GetArraySize(json);
    }
    for(i=0;i<count;i++){
        int width = DEFAULT_LANEWIDTH;
        char *key_up = (char *)"upstreamNodeId" , *key_width = (char *)"linkWidth" , *key_lanes = (char *)"lanes";
        char *key_move = (char *)"movements" , *key_limit = (char *)"speedLimits", *key_points = (char *)"points";
        cJSON *link = cJSON_GetArrayItem(json,i);
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,key_up);
        cJSON *linkWidth = cJSON_GetObjectItem(link,key_width);
        cJSON *lanes = cJSON_GetObjectItem(link,key_lanes);
        cJSON *speedLimits = cJSON_GetObjectItem(link,key_limit);
        cJSON *movements = cJSON_GetObjectItem(link,key_move);
        cJSON *points    = cJSON_GetObjectItem(link,key_points);
        if(linkWidth){
            if(jsonIntRange(linkWidth,0,LANEWIDTH_MAX,pre,key_width)!=0)return ret;
            width = linkWidth->valueint;
        }
        mylog("%s*[%d/%d] : linkWidth=%d\n",pre,i+1,count,width);

        if(upstreamNodeId){
            if(nodeRefIDJsonCheck(upstreamNodeId,level+1,key_up)!=0)return ret;
        }
        if(speedLimits){
            if(speedLimitsJsonCheck(speedLimits,level+1,key_limit) != 0)return ret;
        }
        if(movements){
            if(movementsJsonCheck(movements,level+1,key_move) != 0)return ret;
        }
        if(points){
            if(pointsJsonCheck(points,level+1,key_points) != 0)return ret;
        }
        if(lanesJsonCheck(lanes,level+1,key_lanes) != 0)return ret;
        mylog("%s*[%d/%d]\n",pre,i+1,count);
    }
    return 0;
}


// 检查 json 数据是否符合 nodes 要求
static int nodesJsonCheck(cJSON *json,int level,char *keyname)
{
    int ret = -1,i,count = 0;
    char *pre = getPreSuf(level,keyname);
    if(jsonArrayRange(json,1,32,pre,keyname)!=0)return ret; // 国标 nodes 个数: 1 - 32
    count = cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        char *key_id = (char *)"id",*key_pos = (char *)"refPos" , *key_links = (char *)"links";
        cJSON *node = cJSON_GetArrayItem(json,i);
        cJSON *id = cJSON_GetObjectItem(node,key_id);
        cJSON *refPos = cJSON_GetObjectItem(node,key_pos);
        cJSON *links = cJSON_GetObjectItem(node,key_links);
        mylog("%s[%d/%d] : \n",pre,i+1,count);
        if(refPosJsonCheck(refPos,level+1,key_pos,&s_lng,&s_lat)!=0)return ret;
        if(nodeRefIDJsonCheck(id,level+1,key_id)!=0)return ret;
        if(linksJsonCheck(links,level+1,key_links)!=0)return ret;
        mylog("%s[%d/%d]\n",pre,i+1,count);
    }
    return 0;
}


// 检查 json 数据是否符合 map 要求
int mapJsonCheck(cJSON *json)
{
    int ret = -1,level = 0;
    char *pre = getPreSuf(level,(char *)"map");
    mylog("%s : \n",pre);

    char *key = (char *)"nodes";
    cJSON *nodes = cJSON_GetObjectItem(json,key);
    if(nodesJsonCheck(nodes,level+1,key)!=0)return ret;
    mylog("%s\n",pre);
    return 0;
}




