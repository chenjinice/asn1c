#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include "encode_map.h"
#include "common.h"


#define NODEID_MAX  65535
#define NODEID_MIN  0
#define LANEID_MAX  255
#define LANEID_MIN  0

#define LIST_MAX    32
#define LIST_MIN    1

#define POINTS_MAX  32
#define POINTS_MIN  2

#define SPEEDLIMIT_MAX 9
#define SPEEDLIMIT_MIN 1
#define SPEEDTYPE_MAX  SpeedLimitType_vehiclesWithTrailersNightMaxSpeed
#define SPEEDTYPE_MIN  SpeedLimitType_unknown
#define SPEED_MAX  163.82   // 8191*0.02
#define SPEED_MIN  0.0
#define SPEED_RESOLUTION  0.02

#define CONNECT_MAX 8
#define CONNECT_MIN 1

//---------------------- check -----------------------------



// 检查数量是否符合要求
static int check_num(int num ,int max,int min,char *log)
{
    if( (num < min) || (num > max) ){
        printf("%s error : value = %d, must be (%d .. %d)\n",log,num,min,max);
        return -1;
    }else{
        return 0;
    }
}

/*
 * 获取不同 pointBits 类型的不同经纬度范围
 * pointBits 类型有 ： 24,28,32,36,44,48,64 , 代表用多少位数据来保存经纬度2个值
 * 例如 24 代表 12位数据保存经度，另外12位保持纬度
 * 注意：前6种类型，代表相对于参考点的经纬度偏移值， 而 64 不同，它代表的就是经纬度值
 */
static int get_point_max(int *lon_max,int *lat_max,int pointBits)
{
    int temp ,ret = -1;
    switch (pointBits) {
        case 24:
        case 28:
        case 32:
        case 36:
        case 44:
        case 48:
            temp = pow(2,pointBits/2-1) -1;
            *lon_max = temp;
            *lat_max = temp;
            break;
        case 64:
            *lon_max = 180*1e7;
            *lat_max = 90*1e7;
            break;
        default:
            printf("------pointBits error : invalid pointBits [%d] . must be one of [24,28,32,36,44,48,64]\n",pointBits);
            return ret;
            break;
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 lanes 中的 points 要求
static int check_points(cJSON *points,int pointBits)
{
    int ret = -1,i,lon_max,lat_max,lon_int,lat_int;
    int point_num =  cJSON_GetArraySize(points);
    if( get_point_max(&lon_max,&lat_max,pointBits) != 0)return ret;
    if(check_num(point_num,POINTS_MAX,POINTS_MIN,"------points num") !=0)return ret;
    printf("------points num = %d (lon:+-%d,lat:+-%d)\n",point_num,lon_max,lat_max);

    for(i=0;i<point_num;i++){
        cJSON *point = cJSON_GetArrayItem(points,i);
        cJSON *lon = cJSON_GetObjectItem(point,"lon");
        cJSON *lat = cJSON_GetObjectItem(point,"lat");
        lon_int = lon->valueint;
        lat_int = lat->valueint;
        if(!lon){printf("--------points error : has no lon\n");return ret;}
        if(!lat){printf("--------points error : has no lat\n");return ret;}
        if(check_num(lon_int,lon_max,-lon_max,"--------points lon") !=0 )return ret;
        if(check_num(lat_int,lat_max,-lat_max,"--------points lat") !=0 )return ret;
        printf("--------points[%d] : lon = %d , lat = %d\n",i,lon_int,lat_int);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 lanes 中的 connectsTo 要求
static int check_connectsTo(cJSON *connectsTo)
{
    int ret = -1,i;
    int connect_num =  cJSON_GetArraySize(connectsTo);
    if(check_num(connect_num,CONNECT_MAX,CONNECT_MIN,"------connectsTo num") !=0)return ret;
    printf("------connectsTo num = %d\n",connect_num);

    for(i=0;i<connect_num;i++){
        cJSON *connect = cJSON_GetArrayItem(connectsTo,i);
        cJSON *remoteIntersection = cJSON_GetObjectItem(connect,"remoteIntersection");
        cJSON *connectingLane = cJSON_GetObjectItem(connect,"connectingLane");
        cJSON *phaseId = cJSON_GetObjectItem(connect,"phaseId");
        if(!remoteIntersection){printf("--------connectsTo error : has no remoteIntersection\n");return ret;}
        if(check_num(remoteIntersection->valueint,NODEID_MAX,NODEID_MIN,"--------connectsTo remoteIntersection") != 0)return ret;
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 lanes 要求
static int check_lanes(cJSON *lanes)
{
    int ret = -1,i;
    int lane_num =  cJSON_GetArraySize(lanes);
    if(check_num(lane_num,LIST_MAX,LIST_MIN,"----lanes num") !=0)return ret;
    printf("----lanes num = %d\n",lane_num);

    for(i=0;i<lane_num;i++){
        printf("------lanes[%d] :\n",i);
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        cJSON *laneID = cJSON_GetObjectItem(lane,"laneID");
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        cJSON *pointBits = cJSON_GetObjectItem(lane,"pointBits");
        cJSON *connectsTo = cJSON_GetObjectItem(lane,"connectsTo");
        if(laneID == NULL){printf("------lanes error : has no laneID\n");return ret;}
        if(pointBits == NULL){printf("------lanes error : has no pointBits\n");return ret;}
        if(check_num(laneID->valueint,LANEID_MAX,LANEID_MIN,"------lanes id") != 0)return ret;
        printf("------laneID = %d , pointBits = %d\n",laneID->valueint,pointBits->valueint);
        if(points){
            if(check_points(points,pointBits->valueint) != 0)return ret;
        }
        if(connectsTo){
            if(check_connectsTo(connectsTo) != 0)return ret;
        }
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 speedlimits 要求
static int check_speedLimits(cJSON *speedLimits)
{
    int ret = -1,i;
    int speedlimit_num = cJSON_GetArraySize(speedLimits);
    if(check_num(speedlimit_num,SPEEDLIMIT_MAX,SPEEDLIMIT_MIN,"----speedLimits num") !=0)return ret;
    printf("----speedLimits num = %d\n",speedlimit_num);

    for(i=0;i<speedlimit_num;i++){
        cJSON *limit = cJSON_GetArrayItem(speedLimits,i);
        cJSON *type = cJSON_GetObjectItem(limit,"type");
        cJSON *speed = cJSON_GetObjectItem(limit,"speed");
        if(!type) {printf("------speedlimits error : no type\n");return ret;}
        if(!speed){printf("------speedlimits error : no speed\n");return ret;}
        int type_int = type->valueint;
        double speed_double = speed->valuedouble;
        double tmp = (speed_double/SPEED_RESOLUTION);   // 需要搞个中间值，要不算出来的 speed_int 不对
        int speed_int = tmp;
        if( check_num(type_int,SPEEDTYPE_MAX,SPEEDTYPE_MIN,"------speedlimits type") != 0)return ret;
        if( (speed_double < SPEED_MIN) || (speed_double > SPEED_MAX) ){
            printf("------speedlimits error : speed = %.2lf , must be (%.0lf .. %.2lf)\n",speed_double,SPEED_MIN,SPEED_MAX);
            return ret;
        }
        printf("------speedlimits[%d] : type = %d, speed = %.2lf [%d]\n",i,type_int,speed_double,speed_int);
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 中的 links 要求
static int check_links(cJSON *links){
    int ret = -1,i;
    int link_num =  cJSON_GetArraySize(links);
    if(check_num(link_num,LIST_MAX,LIST_MIN,"--links num") !=0)return ret;
    printf("--links num = %d\n",link_num);

    for(i=0;i<link_num;i++){
        printf("----links[%d] : \n",i);
        cJSON *link = cJSON_GetArrayItem(links,i);
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId");
        cJSON *laneWidth = cJSON_GetObjectItem(link,"laneWidth");
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        cJSON *speedLimits = cJSON_GetObjectItem(link,"speedLimits");
        if(upstreamNodeId == NULL){printf("----links error : has no upstreamNodeId\n");return ret;}
        if(laneWidth == NULL){printf("----links error : has no laneWidth\n");return ret;}
        if(lanes == NULL){printf("----links error : has no lanes\n");return ret;}
        if( check_num(upstreamNodeId->valueint,NODEID_MAX,NODEID_MIN,"--links upstreamNodeId") !=0 )return ret;
        printf("----upstreamNodeId = %d , laneWidth = %d\n",upstreamNodeId->valueint,laneWidth->valueint);
        if(check_lanes(lanes) != 0)return ret;
        if(speedLimits){
            if(check_speedLimits(speedLimits) != 0)return ret;
        }
    }
    return 0;
}

// 检查文件中的 json 数据是否符合 map 要求
static int check_map_json(cJSON *json){
    int ret = -1,i;
    if(json == NULL)return ret;

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    if(nodes == NULL){printf("error : no nodes\n");return ret;}
    int node_num = cJSON_GetArraySize(nodes);
    if(check_num(node_num,LIST_MAX,LIST_MIN,"nodes num") !=0)return ret;
    printf("nodes num = %d\n",node_num);

    for(i=0;i<node_num;i++){
        printf("--nodes[%d] : \n",i);
        cJSON *node = cJSON_GetArrayItem(nodes,i);
        cJSON *id = cJSON_GetObjectItem(node,"id");
        cJSON *lon = cJSON_GetObjectItem(node,"lon");
        cJSON *lat = cJSON_GetObjectItem(node,"lat");
        cJSON *links = cJSON_GetObjectItem(node,"links");
        if(id == NULL){printf("--nodes error : has no id\n");return ret;}
        if(lon == NULL){printf("--nodes error : has no lon\n");return ret;}
        if(lat == NULL){printf("--nodes error : has no lat\n");return ret;}
        if(check_num(id->valueint,NODEID_MAX,NODEID_MIN,"--nodes id") != 0)return ret;
        printf("--id = %d , lon = %d , lat = %d\n",id->valueint,lon->valueint,lat->valueint);

        if(links){
            if(check_links(links) != 0)return ret;
        }
    }

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

// 给 link 添加 lanes
static void add_lanes(LaneList_t *lanelist, cJSON *lanes)
{
    int i;
    int lane_num =  cJSON_GetArraySize(lanes);
    for(i=0;i<lane_num;i++){
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        Lane_t *map_lane = calloc(1,sizeof(Lane_t));
        long id = cJSON_GetObjectItem(lane,"laneID")->valueint;
        int pointBits = cJSON_GetObjectItem(lane,"pointBits")->valueint;
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        map_lane->laneID = id;
        if(points){
            PointList_t *pointlist = calloc(1,sizeof(PointList_t));
            map_lane->points = pointlist;
            add_points(pointlist,points,pointBits);
        }

        ASN_SET_ADD(&lanelist->list,map_lane);
    }
}

// 给 links 添加 speedLimits
static void add_speedLimits(SpeedLimitList_t *speedlist,cJSON *speedlimits)
{
    int i;
    int speed_num = cJSON_GetArraySize(speedlimits);

    for(i=0;i<speed_num;i++){
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
    int link_num =  cJSON_GetArraySize(links);

    for(i=0;i<link_num;i++){
        cJSON *link = cJSON_GetArrayItem(links,i);
        Link_t *map_link = calloc(1,sizeof(Link_t));
        long upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId")->valueint;
        long laneWidth = cJSON_GetObjectItem(link,"laneWidth")->valueint;
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        cJSON *speedlimits = cJSON_GetObjectItem(link,"speedLimits");
        map_link->upstreamNodeId.id = upstreamNodeId;
        map_link->laneWidth = laneWidth;
        add_lanes(&map_link->lanes,lanes);
        if(speedlimits){
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

    // 检查 json 文件是否符合 map 数据的要求
    printf("———————— check map json start ————————\n");
    int ret = check_map_json(json);
    if(ret == 0)printf("———————— check map json \e[32;40mOK\e[0m ————————\n");
    else{printf("———————— check map json \e[31;40mFAIL\e[0m ————————\n");return;}

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
        if(links){
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
static void print_points(PointList_t *pointlist)
{
    if(!pointlist)return;

    int i;
    long lon,lat;
    char * type = "";
    int point_num = pointlist->list.count;
    printf("------points num = %d \n",point_num);
    for(i=0;i<point_num;i++){
        RoadPoint_t *point = pointlist->list.array[i];

        switch (point->posOffset.offsetLL.present) {
            case PositionOffsetLL_PR_position_LL1:
                lon = point->posOffset.offsetLL.choice.position_LL1.lon;
                lat = point->posOffset.offsetLL.choice.position_LL1.lat;
                type = "LL1 : 24";
                break;
            case PositionOffsetLL_PR_position_LL2:
                lon = point->posOffset.offsetLL.choice.position_LL2.lon;
                lat = point->posOffset.offsetLL.choice.position_LL2.lat;
                type = "LL2 : 28";
                break;
            case PositionOffsetLL_PR_position_LL3:
                lon = point->posOffset.offsetLL.choice.position_LL3.lon;
                lat = point->posOffset.offsetLL.choice.position_LL3.lat;
                type = "LL3 : 32";
                break;
            case PositionOffsetLL_PR_position_LL4:
                lon = point->posOffset.offsetLL.choice.position_LL4.lon;
                lat = point->posOffset.offsetLL.choice.position_LL4.lat;
                type = "LL4 : 36";
                break;
            case PositionOffsetLL_PR_position_LL5:
                lon = point->posOffset.offsetLL.choice.position_LL5.lon;
                lat = point->posOffset.offsetLL.choice.position_LL5.lat;
                type = "LL5 : 44";
                break;
            case PositionOffsetLL_PR_position_LL6:
                lon = point->posOffset.offsetLL.choice.position_LL6.lon;
                lat = point->posOffset.offsetLL.choice.position_LL6.lat;
                type = "LL6 : 48";
                break;
            case PositionOffsetLL_PR_position_LatLon:
                lon = point->posOffset.offsetLL.choice.position_LatLon.lon;
                lat = point->posOffset.offsetLL.choice.position_LatLon.lat;
                type = "LL_LatLon : 64";
                break;
        }
        printf("--------points[%d] : [%s] lon = %ld , lat = %ld  \n",i,type,lon,lat);
    }
}

// 打印 links 中的 lanes
static void print_lanes(LaneList_t *lanelist)
{
    int i;
    int lane_num = lanelist->list.count;
    printf("----lanes num = %d\n",lane_num);

    for(i=0;i<lane_num;i++){
        printf("------lanes[%d] : \n",i);
        Lane_t *lane = lanelist->list.array[i];
        printf("------laneID = %ld\n",lane->laneID);
        print_points(lane->points);
    }
}


// 打印 map 中的 speedLimits
static void print_speedLimits(SpeedLimitList_t *speedlist)
{
    if(!speedlist)return;

    int i;
    int speed_num = speedlist->list.count;
    printf("----speedLimits num = %d\n",speed_num);

    for(i=0;i<speed_num;i++){
        RegulatorySpeedLimit_t *limit = speedlist->list.array[i];
        printf("------speedLimits[%d] : type = %ld , speed = %.2lf [%ld]\n",i,limit->type,limit->speed*SPEED_RESOLUTION,limit->speed);
    }
}


// 打印 map 中的 links
static void print_links(LinkList_t *linklist)
{
    if(!linklist)return;

    int i;
    int link_num = linklist->list.count;
    printf("--links num = %d\n",link_num);
    for(i=0;i<link_num;i++){
        printf("----links[%d] :\n",i);
        Link_t *link = linklist->list.array[i];
        printf("----upstreamNodeId = %ld , laneWidth = %ld\n",link->upstreamNodeId.id,link->laneWidth);
        print_lanes(&link->lanes);
        print_speedLimits(link->speedLimits);
    }
}


// 打印 map 数据
void print_map(MessageFrame_t *msg)
{
    int i;
    MAP_t map = msg->choice.mapFrame;
    int node_num = map.nodes.list.count;
    printf("nodes num = %d\n",node_num);
    for(i=0;i<node_num;i++){
        printf("--nodes[%d] :\n",i);
        Node_t *node = map.nodes.list.array[i];
        LinkList_t *linklist = node->inLinks;
        printf("--id = %ld , lon = %ld , lat= %ld\n",node->id.id,node->refPos.Long,node->refPos.lat);
        print_links(linklist);
    }
}



