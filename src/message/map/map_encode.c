#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>


#include "map_about.h"
#include "msg_common.h"
#include "common.h"


static long s_lng = 0;
static long s_lat = 0;


//---------------------- add -----------------------------


// 给 lane 添加 points
static void addPoints(PointList_t *list,cJSON *json)
{
    int i , count;
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        cJSON *point = cJSON_GetArrayItem(json,i);
        RoadPoint_t *road_point = (RoadPoint_t *)calloc(1,sizeof(RoadPoint_t));
        int lng = cJSON_GetObjectItem(point,"lng")->valueint;
        int lat = cJSON_GetObjectItem(point,"lat")->valueint;
        long lng_offset,lat_offset;
        PositionOffsetLL_PR point_type = getOffsetLL(s_lng,s_lat,lng,lat,&lng_offset,&lat_offset,NULL);
        setOffsetLL(&road_point->posOffset,lng_offset,lat_offset,point_type);
        ASN_SET_ADD(&list->list,road_point);
    }
}

// 给 lane 添加 connectsTo
static void addConnectsTo(ConnectsToList_t *list,cJSON *json)
{
    int i,count;
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        cJSON *con = cJSON_GetArrayItem(json,i);
        Connection_t *map_con = (Connection_t *)calloc(1,sizeof(Connection_t));
        cJSON *remoteIntersection = cJSON_GetObjectItem(con,"remoteIntersection");
        cJSON *connectingLane = cJSON_GetObjectItem(con,"connectingLane");
        cJSON *phaseId = cJSON_GetObjectItem(con,"phaseId");
        addNodeRefId(&map_con->remoteIntersection,remoteIntersection);
        if(connectingLane){
            map_con->connectingLane = (ConnectingLane_t *)calloc(1,sizeof(ConnectingLane_t));
            map_con->connectingLane->lane = connectingLane->valueint;
        }
        if(phaseId){
            map_con->phaseId = (PhaseID_t *)calloc(1,sizeof(PhaseID_t));
            *map_con->phaseId = phaseId->valueint;
        }
        ASN_SET_ADD(&list->list,map_con);
    }
}

// 给 link 添加 lanes
static void addLanes(LaneList_t *list, cJSON *json)
{
    int i ,count;
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        cJSON *lane = cJSON_GetArrayItem(json,i);
        Lane_t *map_lane = (Lane_t *)calloc(1,sizeof(Lane_t));
        int id = cJSON_GetObjectItem(lane,"laneID")->valueint;
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        cJSON *connectsTo = cJSON_GetObjectItem(lane,"connectsTo");
        cJSON *maneuvers = cJSON_GetObjectItem(lane,"maneuvers");
        map_lane->laneID = id;
        if(points){
            PointList_t *pointlist = (PointList_t *)calloc(1,sizeof(PointList_t));
            map_lane->points = pointlist;
            addPoints(pointlist,points);
        }
        if(connectsTo){
            ConnectsToList_t *connectlist = (ConnectsToList_t *)calloc(1,sizeof(ConnectsToList_t));
            map_lane->connectsTo = connectlist;
            addConnectsTo(connectlist,connectsTo);
        }
        if(maneuvers){
            int maneuvers_int = maneuvers->valueint;
            int b_size = 2;
            AllowedManeuvers_t * lane_maneuvers = (AllowedManeuvers_t *)calloc(1,sizeof(AllowedManeuvers_t));
            lane_maneuvers->buf = (uint8_t *)calloc(b_size,sizeof(uint8_t));
            lane_maneuvers->size = b_size;
            lane_maneuvers->bits_unused = 4;
            memcpy(lane_maneuvers->buf,&maneuvers_int,b_size);
            map_lane->maneuvers = lane_maneuvers;
        }
        ASN_SET_ADD(&list->list,map_lane);
    }
}

// 给 links 添加 speedLimits
static void addSpeedLimits(SpeedLimitList_t *list,cJSON *json)
{
    int i, count;
    count = cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        RegulatorySpeedLimit_t *map_speed = (RegulatorySpeedLimit_t *)calloc(1,sizeof(RegulatorySpeedLimit_t));
        cJSON *limit = cJSON_GetArrayItem(json,i);
        int type = cJSON_GetObjectItem(limit,"type")->valueint;
        double speed_double = cJSON_GetObjectItem(limit,"speed")->valuedouble;
        double tmp = (speed_double/SPEED_RESOLUTION);
        // 直接 int speed = (speed_double/SPEED_RESOLUTION)的话，值有点不对，用一个 double tmp来转一下
        int speed = tmp;
        map_speed->type = type;
        map_speed->speed = speed;
        ASN_SET_ADD(&list->list,map_speed);
    }
}

// 给 lane 添加 movements
static void addMovements(MovementList_t *list,cJSON *json)
{
    int i ,count;
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        cJSON *move = cJSON_GetArrayItem(json,i);
        Movement_t *map_move = (Movement_t *)calloc(1,sizeof(Movement_t));
        cJSON *remoteIntersection = cJSON_GetObjectItem(move,"remoteIntersection");
        cJSON *phaseId = cJSON_GetObjectItem(move,"phaseId");
        addNodeRefId(&map_move->remoteIntersection,remoteIntersection);
        if(phaseId){
            map_move->phaseId = (PhaseID_t *)calloc(1,sizeof(PhaseID_t));
            *map_move->phaseId = phaseId->valueint;
        }
        ASN_SET_ADD(&list->list,map_move);
    }
}

// 给 map 添加 links
static void addLinks(LinkList_t *list,cJSON *json)
{
    int i,count;
    count =  cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        int width = DEFAULT_LANEWIDTH;
        cJSON *link = cJSON_GetArrayItem(json,i);
        Link_t *map_link = (Link_t *)calloc(1,sizeof(Link_t));
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId");
        cJSON *laneWidth = cJSON_GetObjectItem(link,"laneWidth");
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        cJSON *speedlimits = cJSON_GetObjectItem(link,"speedLimits");
        cJSON *movements = cJSON_GetObjectItem(link,"movements");
        if(laneWidth)width = laneWidth->valueint;
        map_link->laneWidth = width;
        addNodeRefId(&map_link->upstreamNodeId,upstreamNodeId);
        addLanes(&map_link->lanes,lanes);
        if(speedlimits){
            SpeedLimitList_t *speedlist = (SpeedLimitList_t *)calloc(1,sizeof(SpeedLimitList_t));
            map_link->speedLimits = speedlist;
            addSpeedLimits(speedlist,speedlimits);
        }
        if(movements){
            MovementList_t *movelist = (MovementList_t *)calloc(1,sizeof(MovementList_t));
            map_link->movements = movelist;
            addMovements(movelist,movements);
        }
        ASN_SET_ADD(&list->list,map_link);
    }
}

// 给 map 添加 lane
static void addNodes(NodeListltev_t *list,cJSON *json)
{
    int i,count;
    count = cJSON_GetArraySize(json);
    for(i=0;i<count;i++){
        Node_t *map_node = (Node_t *)calloc(1,sizeof(Node_t));
        cJSON *node = cJSON_GetArrayItem(json,i);
        cJSON *id = cJSON_GetObjectItem(node,"id");
        cJSON *refPos = cJSON_GetObjectItem(node,"refPos");
        cJSON *links = cJSON_GetObjectItem(node,"links");
        addRefPos(&map_node->refPos,refPos,&s_lng,&s_lat);
        addNodeRefId(&map_node->id,id);
        if(links){
            LinkList_t *inLinks = (LinkList_t *)calloc(1,sizeof(LinkList_t));
            map_node->inLinks = inLinks;
            addLinks(map_node->inLinks,links);
        }
        ASN_SET_ADD(&list->list,map_node);
    }
}

// 读取文件中的json数据，asn编码并保存到文件
void mapEncode(cJSON *json, char *uper_file)
{
    MessageFrame_t *msgframe = NULL;

    // 检查 json 文件是否符合 map 数据的要求
    mylog("---check map json start---\n");
    int ret = mapJsonCheck(json);
    mylog("---check map json end---\n");
    if(ret == 0)myok("check json ok\n");
    else{myerr("check json error\n");return;}

    msgframe = (MessageFrame_t*)calloc(1,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_mapFrame;
    MAP_t *map = &msgframe->choice.mapFrame;
    map->msgCnt = 0;

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    addNodes(&map->nodes,nodes);

    encode(uper_file,msgframe);
    cJSON_Delete(json);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}




