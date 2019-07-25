#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include "encode_map.h"
#include "common.h"





//---------------------- check -----------------------------

/*
 * 获取不同 pointBits 类型的不同经纬度范围
 * pointBits 类型有 ： 24,28,32,36,44,48,64 , 代表用多少位数据来保存经纬度2个值
 * 例如 24 代表 12位数据保存经度，另外12位保持纬度
 * 注意：前6种类型，代表相对于参考点的经纬度偏移值， 而 64 不同，它代表的就是经纬度值
 */
static void get_point_max(int *lon_max,int *lat_max,int pointBits)
{
    int temp;
    if(pointBits >= 64){
        *lon_max = LON_MAX;
        *lat_max = LAT_MAX;
    }else{
        temp = pow(2,pointBits/2-1) - 1;
        *lon_max = temp;
        *lat_max = temp;
    }
}

static int check_pointBits(int pointBits)
{
    int ret = -1;
    switch (pointBits) {
        case 24:
        case 28:
        case 32:
        case 36:
        case 44:
        case 48:
        case 64:
            ret = 0;
            break;
    }
    return ret;
}

// 检查文件中的 json 数据是否符合 lanes 中的 points 要求
static int check_points(cJSON *points,int pointBits,int level)
{
    int ret = -1,i,lon_max,lat_max,lon_int,lat_int;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"point",level);

    get_point_max(&lon_max,&lat_max,pointBits);
    printf("%s range == lat:+-%d , lon:+-%d\n",pre,lon_max,lat_max);

    int count =  cJSON_GetArraySize(points);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        cJSON *point = cJSON_GetArrayItem(points,i);
        cJSON *lon = cJSON_GetObjectItem(point,"lon");
        cJSON *lat = cJSON_GetObjectItem(point,"lat");
        if(!lon){printf("%s error : has no lon\n",pre);return ret;}
        if(!lat){printf("%s error : has no lat\n",pre);return ret;}
        lon_int = lon->valueint;
        lat_int = lat->valueint;
        if(check_int(lon_int,-lon_max,lon_max,pre,"lon") !=0 )return ret;
        if(check_int(lat_int,-lat_max,lat_max,pre,"lat") !=0 )return ret;
        sprintf(log+strlen(log),"lon=%d,lat=%d",lon_int,lat_int);
        printf("%s\n",log);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 lanes 中的 connectsTo 要求
static int check_connectsTo(cJSON *connectsTo,int level)
{
    int ret = -1,i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"connectsTo",level);

    int count =  cJSON_GetArraySize(connectsTo);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        cJSON *connect = cJSON_GetArrayItem(connectsTo,i);
        cJSON *remoteIntersection = cJSON_GetObjectItem(connect,"remoteIntersection");
        cJSON *connectingLane = cJSON_GetObjectItem(connect,"connectingLane");
        cJSON *phaseId = cJSON_GetObjectItem(connect,"phaseId");

        if(remoteIntersection == NULL){printf("%s error : has no remoteIntersection\n",pre);return ret;}
        sprintf(log+strlen(log),"remoteIntersection=%d,",remoteIntersection->valueint);
        if(connectingLane)sprintf(log+strlen(log),"*connectingLane=%d,",connectingLane->valueint);
        if(phaseId)sprintf(log+strlen(log),"*phaseId=%d,",phaseId->valueint);
        printf("%s\n",log);

        if(check_int(remoteIntersection->valueint,NODEID_MIN,NODEID_MAX,pre,"remoteIntersection") != 0)return ret;
        if(connectingLane){
            if(check_int(connectingLane->valueint,LANEID_MIN,LANEID_MAX,pre,"connectingLane") != 0)return ret;
        }
        if(phaseId){
            if(check_int(phaseId->valueint,PHASEID_MIN,PHASEID_MAX,pre,"phaseId") != 0)return ret;
        }
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 lanes 要求
static int check_lanes(cJSON *lanes,int level)
{
    int ret = -1,i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"lane",level);

    int count =  cJSON_GetArraySize(lanes);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        int points_count = 0;
        int connectsTo_count = 0;
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        cJSON *laneID = cJSON_GetObjectItem(lane,"laneID");
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        cJSON *pointBits = cJSON_GetObjectItem(lane,"pointBits");
        cJSON *connectsTo = cJSON_GetObjectItem(lane,"connectsTo");
        if(laneID == NULL){printf("%s error : has no laneID\n",pre);return ret;}
        if(pointBits == NULL){printf("%s error : has no pointBits\n",pre);return ret;}
        sprintf(log+strlen(log),"laneID=%d,pointBits=%d,",laneID->valueint,pointBits->valueint);
        if(points){
            points_count = cJSON_GetArraySize(points);
            sprintf(log+strlen(log),"*points[%d],",points_count);
        }
        if(connectsTo){
            connectsTo_count = cJSON_GetArraySize(connectsTo);
            sprintf(log+strlen(log),"*connectsTo[%d],",connectsTo_count);
        }
        printf("%s\n",log);

        if(check_int(laneID->valueint,LANEID_MIN,LANEID_MAX,pre,"laneID") != 0)return ret;
        if( check_pointBits(pointBits->valueint) != 0){
            printf("%s pointBits invalid : value = [%d]. must be [24,28,32,36,44,48,64]\n",pre,pointBits->valueint);
            return ret;
        }
        if(points_count > 0){
            // 国标 points 个数: 2 - 31 , optional ， 可以没有
            if(check_int(points_count,2,31,pre,"points count") != 0)return ret;
            if(check_points(points,pointBits->valueint,level+1) != 0)return ret;
        }
        if(connectsTo_count > 0){
            // 国标 connectsTo 个数: 1 - 8 , optional ， 可以没有
            if(check_int(connectsTo_count,1,8,pre,"connectsTo count") != 0)return ret;
            if(check_connectsTo(connectsTo,level+1) != 0)return ret;
        }
        printf("%s[%d]\n",pre,i);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 speedlimits 要求
static int check_speedLimits(cJSON *speedLimits,int level)
{
    int ret = -1,i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"speedLimit",level);

    int count = cJSON_GetArraySize(speedLimits);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        cJSON *limit = cJSON_GetArrayItem(speedLimits,i);
        cJSON *type = cJSON_GetObjectItem(limit,"type");
        cJSON *speed = cJSON_GetObjectItem(limit,"speed");
        if(!type) {printf("%s error : no type\n",pre);return ret;}
        if(!speed){printf("%s error : no speed\n",pre);return ret;}
        int type_int = type->valueint;
        double speed_double = speed->valuedouble;
        double tmp = (speed_double/SPEED_RESOLUTION);   // 需要搞个中间值，要不算出来的 speed_int 不对
        int speed_int = tmp;
        sprintf(log+strlen(log),"type=%d,speed=%.2lf(%d)",type_int,speed_double,speed_int);
        printf("%s\n",log);

        if( check_int(type_int,SPEEDTYPE_MIN,SPEEDTYPE_MAX,pre,"type") != 0 )return ret;
        if( check_double(speed_double,SPEED_MIN,SPEED_MAX,pre,"speed") !=0 )return ret;
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 links 要求
static int check_links(cJSON *links,int level)
{
    int ret = -1,i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"link",level);

    int count =  cJSON_GetArraySize(links);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        int lanes_count = 0;
        int speedLimits_count = 0;
        cJSON *link = cJSON_GetArrayItem(links,i);
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId");
        cJSON *laneWidth = cJSON_GetObjectItem(link,"laneWidth");
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        cJSON *speedLimits = cJSON_GetObjectItem(link,"speedLimits");
        if(upstreamNodeId == NULL){printf("%s error : has no upstreamNodeId\n",pre);return ret;}
        if(laneWidth == NULL){printf("%s error : has no laneWidth\n",pre);return ret;}
        if(lanes == NULL){printf("%s error : has no lanes\n",pre);return ret;}
        lanes_count = cJSON_GetArraySize(lanes);
        sprintf(log+strlen(log),"upstreamNodeId=%d,laneWidth=%d,lanes[%d],",upstreamNodeId->valueint,laneWidth->valueint,lanes_count);
        if(speedLimits){
            speedLimits_count = cJSON_GetArraySize(speedLimits);
            sprintf(log+strlen(log),"*speedLimits[%d]",speedLimits_count);
        }
        printf("%s\n",log);

        if(check_int(upstreamNodeId->valueint,NODEID_MIN,NODEID_MAX,pre,"upstreamNodeId") !=0)return ret;
        if(check_int(laneWidth->valueint,0,LANEWIDTH_MAX,pre,"laneWidth") !=0)return ret;
        // 国标 lanes 个数: 1 - 32
        if(check_int(lanes_count,1,32,pre,"lanes") !=0)return ret;
        if(check_lanes(lanes,level+1) != 0)return ret;
        if(speedLimits_count > 0){
            // 国标 speedLimits 个数: 1 - 9 , optional ， 可以没有
            if(check_int(speedLimits_count,1,9,pre,"speedLimits count"));
            if(check_speedLimits(speedLimits,level+1) != 0)return ret;
        }
        printf("%s[%d]\n",pre,i);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 nodes 要求
static int check_nodes(cJSON *nodes,int level)
{
    int ret = -1,i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"node",level);

    int count = cJSON_GetArraySize(nodes);
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        int links_count = 0;
        cJSON *node = cJSON_GetArrayItem(nodes,i);
        cJSON *id = cJSON_GetObjectItem(node,"id");
        cJSON *lon = cJSON_GetObjectItem(node,"lon");
        cJSON *lat = cJSON_GetObjectItem(node,"lat");
        cJSON *links = cJSON_GetObjectItem(node,"links");
        if(id == NULL){printf("%s error : has no id\n",pre);return ret;}
        if(lon == NULL){printf("%s error : has no lon\n",pre);return ret;}
        if(lat == NULL){printf("%s error : has no lat\n",pre);return ret;}
        sprintf(log+strlen(log),"id=%d,lon=%d,lat=%d,",id->valueint,lon->valueint,lat->valueint);
        if(links){
            links_count =  cJSON_GetArraySize(links);
            sprintf(log+strlen(log),"*inLinks[%d]",links_count);
        }
        printf("%s\n",log);

        if(check_int(id->valueint,NODEID_MIN,NODEID_MAX,pre,"id") != 0)return ret;
        if(check_int(lon->valueint,-LON_MAX,LON_MAX,pre,"lon") != 0)return ret;
        if(check_int(lat->valueint,-LAT_MAX,LAT_MAX,pre,"lat") != 0)return ret;
        if(links_count > 0){
            // 国标 links 个数: 1 - 32 , optional ， 可以没有
            if(check_int(links_count,1,32,pre,"links count") !=0)return ret;
            if(check_links(links,level+1) != 0)return ret;
        }
        printf("%s[%d]\n",pre,i);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 要求
static int check_map_json(cJSON *json)
{
    int ret = -1,level = 0;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};

    get_pre(pre,"map",level);
    sprintf(log,"%s : ",pre);

    int nodes_count = 0 ;
    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    if(nodes == NULL){printf("%s error : no nodes\n",pre);return ret;}
    nodes_count = cJSON_GetArraySize(nodes);
    sprintf(log+strlen(log),"nodes[%d]",nodes_count);
    printf("%s \n",log);

    // 国标 nodes 个数: 1 - 32
    if(check_int(nodes_count,1,32,pre,"nodes count") !=0)return ret;
    if(check_nodes(nodes,level+1) != 0)return ret;

    printf("%s\n",pre);
    return 0;
}



//---------------------- add -----------------------------

// 给 lane 添加 points
static void add_points(PointList_t *pointlist,cJSON *points,int pointBits)
{
    int i;
    int point_num =  cJSON_GetArraySize(points);
    for(i=0;i<point_num;i++){
        cJSON *point = cJSON_GetArrayItem(points,i);
        RoadPoint_t *road_point = calloc(1,sizeof(RoadPoint_t));
        long lon = cJSON_GetObjectItem(point,"lon")->valueint;
        long lat = cJSON_GetObjectItem(point,"lat")->valueint;
        road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LatLon;
        road_point->posOffset.offsetLL.choice.position_LatLon.lon = lon;
        road_point->posOffset.offsetLL.choice.position_LatLon.lat = lat;

        switch (pointBits) {
            case 24:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL1;
                road_point->posOffset.offsetLL.choice.position_LL1.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL1.lat = lat;
                break;
            case 28:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL2;
                road_point->posOffset.offsetLL.choice.position_LL2.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL2.lat = lat;
                break;
            case 32:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL3;
                road_point->posOffset.offsetLL.choice.position_LL3.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL3.lat = lat;
                break;
            case 36:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL4;
                road_point->posOffset.offsetLL.choice.position_LL4.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL4.lat = lat;
                break;
            case 44:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL5;
                road_point->posOffset.offsetLL.choice.position_LL5.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL5.lat = lat;
                break;
            case 48:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LL6;
                road_point->posOffset.offsetLL.choice.position_LL6.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LL6.lat = lat;
                break;
            case 64:
                road_point->posOffset.offsetLL.present = PositionOffsetLL_PR_position_LatLon;
                road_point->posOffset.offsetLL.choice.position_LatLon.lon = lon;
                road_point->posOffset.offsetLL.choice.position_LatLon.lat = lat;
                break;
        }

        ASN_SET_ADD(&pointlist->list,road_point);
    }
}

// 给 lane 添加 connectsTo
static void add_connectsTo(ConnectsToList_t *list,cJSON *connectsTo)
{
    int i;
    int count =  cJSON_GetArraySize(connectsTo);

    for(i=0;i<count;i++){
        cJSON *connect = cJSON_GetArrayItem(connectsTo,i);
        Connection_t *map_con = calloc(1,sizeof(Connection_t));
        cJSON *remoteIntersection = cJSON_GetObjectItem(connect,"remoteIntersection");
        cJSON *connectingLane = cJSON_GetObjectItem(connect,"connectingLane");
        cJSON *phaseId = cJSON_GetObjectItem(connect,"phaseId");
        map_con->remoteIntersection.id = remoteIntersection->valueint;
        if(connectingLane){
            map_con->connectingLane = calloc(1,sizeof(ConnectingLane_t));
            map_con->connectingLane->lane = connectingLane->valueint;
        }
        if(phaseId){
            map_con->phaseId = calloc(1,sizeof(PhaseID_t));
            *map_con->phaseId = phaseId->valueint;
        }

        ASN_SET_ADD(&list->list,map_con);
    }
}

// 给 link 添加 lanes
static void add_lanes(LaneList_t *lanelist, cJSON *lanes)
{
    int i;
    int count =  cJSON_GetArraySize(lanes);

    for(i=0;i<count;i++){
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        Lane_t *map_lane = calloc(1,sizeof(Lane_t));
        long id = cJSON_GetObjectItem(lane,"laneID")->valueint;
        int pointBits = cJSON_GetObjectItem(lane,"pointBits")->valueint;
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        cJSON *connectsTo = cJSON_GetObjectItem(lane,"connectsTo");
        map_lane->laneID = id;
        if( points && (cJSON_GetArraySize(points) > 1) ){
            PointList_t *pointlist = calloc(1,sizeof(PointList_t));
            map_lane->points = pointlist;
            add_points(pointlist,points,pointBits);
        }
        if( connectsTo && (cJSON_GetArraySize(connectsTo) > 0 ) ){
            ConnectsToList_t *connectlist = calloc(1,sizeof(ConnectsToList_t));
            map_lane->connectsTo = connectlist;
            add_connectsTo(connectlist,connectsTo);
        }

        ASN_SET_ADD(&lanelist->list,map_lane);
    }
}

// 给 links 添加 speedLimits
static void add_speedLimits(SpeedLimitList_t *speedlist,cJSON *speedlimits)
{
    int i;
    int count = cJSON_GetArraySize(speedlimits);

    for(i=0;i<count;i++){
        RegulatorySpeedLimit_t *map_speed = calloc(1,sizeof(RegulatorySpeedLimit_t));
        cJSON *limit = cJSON_GetArrayItem(speedlimits,i);
        int type = cJSON_GetObjectItem(limit,"type")->valueint;
        double speed_double = cJSON_GetObjectItem(limit,"speed")->valuedouble;
        double tmp = (speed_double/SPEED_RESOLUTION);
        // 直接 int speed = (speed_double/SPEED_RESOLUTION)的话，值有点不对，用一个 double tmp来转一下
        int speed = tmp;
        map_speed->type = type;
        map_speed->speed = speed;

        ASN_SET_ADD(&speedlist->list,map_speed);
    }
}

// 给 map 添加 links
static void add_links(LinkList_t *linklist,cJSON *links)
{
    int i;
    int count =  cJSON_GetArraySize(links);

    for(i=0;i<count;i++){
        cJSON *link = cJSON_GetArrayItem(links,i);
        Link_t *map_link = calloc(1,sizeof(Link_t));
        long upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId")->valueint;
        long laneWidth = cJSON_GetObjectItem(link,"laneWidth")->valueint;
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        cJSON *speedlimits = cJSON_GetObjectItem(link,"speedLimits");
        map_link->upstreamNodeId.id = upstreamNodeId;
        map_link->laneWidth = laneWidth;
        add_lanes(&map_link->lanes,lanes);
        if( speedlimits && (cJSON_GetArraySize(speedlimits) > 0) ){
            SpeedLimitList_t *speedlist = calloc(1,sizeof(SpeedLimitList_t));
            map_link->speedLimits = speedlist;
            add_speedLimits(speedlist,speedlimits);
        }

        ASN_SET_ADD(&linklist->list,map_link);
    }
}

// 读取文件中的json数据，asn编码并保存到文件
void encode_map(char *json_file, char *uper_file)
{
    int i;
    MessageFrame_t *msgframe = NULL;
    cJSON *json = read_json(json_file);
    if(!json)return;

    char *pre = "——————————";
    // 检查 json 文件是否符合 map 数据的要求
    printf("%s check map json start %s\n",pre,pre);
    int ret = check_map_json(json);
    if(ret == 0)printf("%s check map json \e[32;40mOK\e[0m %s\n",pre,pre);
    else{printf("%s check map json \e[31;40mFAIL\e[0m %s\n",pre,pre);return;}

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    int node_num = cJSON_GetArraySize(nodes);

    msgframe = (MessageFrame_t*)calloc(1,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_mapFrame;
    MAP_t *map = &msgframe->choice.mapFrame;
    map->msgCnt = 0;

    for(i=0;i<node_num;i++){
        Node_t *map_node = calloc(1,sizeof(Node_t));
        cJSON *node = cJSON_GetArrayItem(nodes,i);
        long id = cJSON_GetObjectItem(node,"id")->valueint;
        long lon = cJSON_GetObjectItem(node,"lon")->valueint;
        long lat = cJSON_GetObjectItem(node,"lat")->valueint;
        cJSON *links = cJSON_GetObjectItem(node,"links");
        map_node->id.id = id;
        map_node->refPos.Long = lon;
        map_node->refPos.lat = lat;
        if( links && (cJSON_GetArraySize(links) > 0) ){
            LinkList_t *inLinks = calloc(1,sizeof(LinkList_t));
            map_node->inLinks = inLinks;
            add_links(inLinks,links);
        }

        ASN_SET_ADD(&map->nodes.list,map_node);
    }

    encode(uper_file,msgframe);

    cJSON_Delete(json);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}



//---------------------- print map -----------------------------

// 打印 lanes 中的 points
static void print_points(PointList_t *pointlist,int level)
{
    if(!pointlist)return;

    int i;
    long lon,lat;
    char * type = "";
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"point",level);


    int count = pointlist->list.count;
    for(i=0;i<count;i++){
        RoadPoint_t *point = pointlist->list.array[i];

        switch (point->posOffset.offsetLL.present) {
            case PositionOffsetLL_PR_position_LL1:
                lon = point->posOffset.offsetLL.choice.position_LL1.lon;
                lat = point->posOffset.offsetLL.choice.position_LL1.lat;
                type = "LL1:24";
                break;
            case PositionOffsetLL_PR_position_LL2:
                lon = point->posOffset.offsetLL.choice.position_LL2.lon;
                lat = point->posOffset.offsetLL.choice.position_LL2.lat;
                type = "LL2:28";
                break;
            case PositionOffsetLL_PR_position_LL3:
                lon = point->posOffset.offsetLL.choice.position_LL3.lon;
                lat = point->posOffset.offsetLL.choice.position_LL3.lat;
                type = "LL3:32";
                break;
            case PositionOffsetLL_PR_position_LL4:
                lon = point->posOffset.offsetLL.choice.position_LL4.lon;
                lat = point->posOffset.offsetLL.choice.position_LL4.lat;
                type = "LL4:36";
                break;
            case PositionOffsetLL_PR_position_LL5:
                lon = point->posOffset.offsetLL.choice.position_LL5.lon;
                lat = point->posOffset.offsetLL.choice.position_LL5.lat;
                type = "LL5:44";
                break;
            case PositionOffsetLL_PR_position_LL6:
                lon = point->posOffset.offsetLL.choice.position_LL6.lon;
                lat = point->posOffset.offsetLL.choice.position_LL6.lat;
                type = "LL6:48";
                break;
            case PositionOffsetLL_PR_position_LatLon:
                lon = point->posOffset.offsetLL.choice.position_LatLon.lon;
                lat = point->posOffset.offsetLL.choice.position_LatLon.lat;
                type = "LL_LatLon:64";
                break;
            default :
                lon = 0;
                lat = 0;
                type = " ?? ";
                break;
        }
        printf("%s[%d] : lon=%ld,lat=%ld  (%s)\n",pre,i,lon,lat,type);
    }
}

// 打印 links 中的 connectsTo
static void print_connectsTo(ConnectsToList_t *list,int level)
{
    if(!list)return;

    int i;
    char pre[PRE_SIZE] = {0};
    char log[LOG_SIZE] = {0};
    get_pre(pre,"connectsTo",level);

    int count = list->list.count;
    for(i=0;i<count;i++){
        sprintf(log,"%s[%d] : ",pre,i);
        Connection_t *connect = list->list.array[i];
        sprintf(log+strlen(log),"remoteIntersection=%ld,",connect->remoteIntersection.id);
        if(connect->connectingLane)sprintf(log+strlen(log),"*connectingLane=%ld,",connect->connectingLane->lane);
        if(connect->phaseId)sprintf(log+strlen(log),"*phaseId=%ld,",*connect->phaseId);

        printf("%s\n",log);
    }
}

// 打印 links 中的 lanes
static void print_lanes(LaneList_t *lanelist,int level)
{
    int i;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"lane",level);

    int count = lanelist->list.count;
    for(i=0;i<count;i++){
        Lane_t *lane = lanelist->list.array[i];
        int point_count = 0;
        int connect_count = 0;
        if(lane->points)point_count = lane->points->list.count;
        if(lane->connectsTo)connect_count = lane->connectsTo->list.count;
        printf("%s[%d] : laneID=%ld,*points[%d],*connectsTo[%d]\n",pre,i,lane->laneID,point_count,connect_count);

        print_points(lane->points,level+1);
        print_connectsTo(lane->connectsTo,level+1);
        printf("%s[%d]\n",pre,i);
    }
}

// 打印 map 中的 speedLimits
static void print_speedLimits(SpeedLimitList_t *speedlist,int level)
{
    if(!speedlist)return;

    int i;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"speedLimit",level);
    int count = speedlist->list.count;

    for(i=0;i<count;i++){
        RegulatorySpeedLimit_t *limit = speedlist->list.array[i];
        printf("%s[%d] : type=%ld,speed=%.2lf(%ld)\n",pre,i,limit->type,limit->speed*SPEED_RESOLUTION,limit->speed);
    }
}

// 打印 map 中的 links
static void print_links(LinkList_t *linklist,int level)
{
    if(!linklist)return;

    int i;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"link",level);

    int count = linklist->list.count;
    for(i=0;i<count;i++){
        Link_t *link = linklist->list.array[i];
        int lane_count = link->lanes.list.count;
        int limit_count = 0;
        if(link->speedLimits)limit_count = link->speedLimits->list.count;
        printf("%s[%d] : upstreamNodeId=%ld,laneWidth=%ld,lanes[%d],*speedLimits[%d]\n",pre,i,link->upstreamNodeId.id,link->laneWidth,lane_count,limit_count);

        print_lanes(&link->lanes,level+1);
        print_speedLimits(link->speedLimits,level+1);
        printf("%s[%d]\n",pre,i);
    }
}

// 打印 map 中的 nodes
void print_nodes(NodeListltev_t *nodelist,int level)
{
    int i;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"node",level);

    int count = nodelist->list.count;
    for(i=0;i<count;i++){
        Node_t *node = nodelist->list.array[i];
        LinkList_t *linklist = node->inLinks;
        int link_count = 0;
        if(linklist)link_count = linklist->list.count;
        printf("%s[%d] : id=%ld,lon=%ld,lat=%ld,*inLinks[%d]\n",pre,i,node->id.id,node->refPos.Long,node->refPos.lat,link_count);

        print_links(linklist,level+1);
        printf("%s[%d]\n",pre,i);
    }
}

// 打印 map
void print_map(MessageFrame_t *msg)
{
    int level = 0;
    char pre[PRE_SIZE] = {0};
    get_pre(pre,"map",level);

    MAP_t map = msg->choice.mapFrame;
    printf("%s : nodes[%d]\n",pre,map.nodes.list.count);

    print_nodes(&map.nodes,level+1);
    printf("%s\n",pre);
}



