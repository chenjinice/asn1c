#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include "encode_map.h"
#include "common.h"


//---------------------- check -----------------------------
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
            printf("------pointBits error : invalid pointBits [%d] . should be one in [24,28,32,36,44,48,64]\n",pointBits);
            return ret;
            break;
    }
    return 0;
}

static int check_points(cJSON *points,int pointBits)
{
    int ret = -1,i,lon_max,lat_max,lon_int,lat_int;
    int point_num =  cJSON_GetArraySize(points);
    if( get_point_max(&lon_max,&lat_max,pointBits) != 0)return ret;
    printf("------point num = %d  [lon : +-%d , lat : +-%d]\n",point_num,lon_max,lat_max);
    if(point_num < 2){printf("------point error : num < 2\n");return ret;}
    for(i=0;i<point_num;i++){
        cJSON *point = cJSON_GetArrayItem(points,i);
        cJSON *lon = cJSON_GetObjectItem(point,"lon");
        cJSON *lat = cJSON_GetObjectItem(point,"lat");
        lon_int = lon->valueint;
        lat_int = lat->valueint;
        if(!lon){printf("--------point[%d] error : has no lon\n",i);return ret;}
        if(!lat){printf("--------point[%d] error : has no lat\n",i);return ret;}
        if(abs(lon_int) > lon_max){printf("--------point[%d] lon error : |%d| > %d \n",i,lon_int,lon_max);return ret;}
        if(abs(lat_int) > lat_max){printf("--------point[%d] lat error : |%d| > %d \n",i,lat_int,lat_max);return ret;}
        printf("--------point[%d]:lon = %d , lat = %d\n",i,lon_int,lat_int);
    }
    return 0;
}

static int check_lanes(cJSON *lanes)
{
    int ret = -1,i;
    int lane_num =  cJSON_GetArraySize(lanes);
    for(i=0;i<lane_num;i++){
        printf("----lane[%d] :\n",i);
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        cJSON *laneID = cJSON_GetObjectItem(lane,"laneID");
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        cJSON *pointBits = cJSON_GetObjectItem(lane,"pointBits");
        if(laneID == NULL){printf("------lane error : has no laneID");return ret;}
        if(pointBits == NULL){printf("------lane error : has no pointBits\n");return ret;}
        printf("------laneID = %d , pointBits = %d\n",laneID->valueint,pointBits->valueint);
        if(points){
            if(check_points(points,pointBits->valueint) != 0)return ret;
        }
    }
    return 0;
}

static int check_links(cJSON *links){
    int ret = -1,i;
    int link_num =  cJSON_GetArraySize(links);
    printf("--link num = %d \n",link_num);
    if(link_num < 1){printf("--link error : num < 1\n");return ret;}
    for(i=0;i<link_num;i++){
        printf("--link[%d] : \n",i);
        cJSON *link = cJSON_GetArrayItem(links,i);
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId");
        cJSON *laneWidth = cJSON_GetObjectItem(link,"laneWidth");
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        if(upstreamNodeId == NULL){printf("----link error : has no upstreamNodeId\n");return ret;}
        if(laneWidth == NULL){printf("----link error : has no laneWidth\n");return ret;}
        if(lanes == NULL){printf("----link error : has no lanes\n");return ret;}
        int lane_num =  cJSON_GetArraySize(lanes);
        if(lane_num < 1){printf("----lane error : num < 1\n");return ret;}
        printf("----upstreamNodeId = %d , laneWidth = %d , lane_num = %d\n",upstreamNodeId->valueint,laneWidth->valueint,lane_num);
        if(check_lanes(lanes) != 0)return ret;
    }
    return 0;
}

static int check_map_json(cJSON *json){
    int ret = -1,i;
    if(json == NULL)return ret;

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    if(nodes == NULL){printf("error : no node\n");return ret;}
    int node_num = cJSON_GetArraySize(nodes);
    printf("node num = %d\n",node_num);
    if(node_num < 1){printf("node error : num < 1\n");return ret;}

    for(i=0;i<node_num;i++){
        printf("node[%d] : \n",i);
        cJSON *node = cJSON_GetArrayItem(nodes,i);
        cJSON *id = cJSON_GetObjectItem(node,"id");
        cJSON *lon = cJSON_GetObjectItem(node,"lon");
        cJSON *lat = cJSON_GetObjectItem(node,"lat");
        cJSON *links = cJSON_GetObjectItem(node,"links");
        if(id == NULL){printf("--node error : has no id\n");return ret;}
        if(lon == NULL){printf("--node error : has no lon\n");return ret;}
        if(lat == NULL){printf("--node error : has no lat\n");return ret;}
        printf("--id = %d , lon = %d , lat = %d\n",id->valueint,lon->valueint,lat->valueint);
        // links
        if(links){
            if(check_links(links) != 0)return ret;
        }
    }

    return 0;
}


//---------------------- add -----------------------------
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
        map_link->upstreamNodeId.id = upstreamNodeId;
        map_link->laneWidth = laneWidth;
        add_lanes(&map_link->lanes,lanes);

        ASN_SET_ADD(&linklist->list,map_link);
    }
}

void encode_map(char *json_file, char *uper_file)
{
    int i;
    MessageFrame_t *msgframe = NULL;
    cJSON *json = read_json(json_file);
    if(!json)return;

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
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}



//---------------------- print map -----------------------------
static void print_points(PointList_t *pointlist)
{
    if(!pointlist)return;

    int i;
    long lon,lat;
    char * type = "";
    int point_num = pointlist->list.count;
    printf("------point num = %d \n",point_num);
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
        printf("------point[%d] : [%s] lon = %ld , lat = %ld  \n",i,type,lon,lat);
    }
}

static void print_lanes(LaneList_t *lanelist)
{
    int i;
    int lane_num = lanelist->list.count;
    for(i=0;i<lane_num;i++){
        printf("----lane[%d] : \n",i);
        Lane_t *lane = lanelist->list.array[i];
        printf("------laneID = %ld\n",lane->laneID);
        print_points(lane->points);
    }
}

static void print_links(LinkList_t *linklist)
{
    if(!linklist)return;

    int i;
    int link_num = linklist->list.count;
    printf("--link num = %d\n",link_num);
    for(i=0;i<link_num;i++){
        printf("--link[%d] :\n",i);
        Link_t *link = linklist->list.array[i];
        printf("----upstreamNodeId = %ld , laneWidth = %ld\n",link->upstreamNodeId.id,link->laneWidth);
        printf("----lane num = %d\n",link->lanes.list.count);
        print_lanes(&link->lanes);
    }
}

void print_map(MessageFrame_t *msg)
{
    int i;
    MAP_t map = msg->choice.mapFrame;
    int node_num = map.nodes.list.count;
    printf("node num = %d\n",node_num);
    for(i=0;i<node_num;i++){
        printf("node[%d] :\n",i);
        Node_t *node = map.nodes.list.array[i];
        LinkList_t *linklist = node->inLinks;
        printf("--id = %ld , lon = %ld , lat= %ld\n",node->id.id,node->refPos.Long,node->refPos.lat);
        print_links(linklist);
    }
}



