#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "encode_map.h"
#include "common.h"



static int check_points(cJSON *points)
{
    int ret = -1,i;
    int point_num =  cJSON_GetArraySize(points);
    printf("------point num=%d\n",point_num);
    if(point_num < 2){printf("------point num < 2");return ret;}
    for(i=0;i<point_num;i++){
        cJSON *point = cJSON_GetArrayItem(points,i);
        cJSON *lon = cJSON_GetObjectItem(point,"lon");
        cJSON *lat = cJSON_GetObjectItem(point,"lat");
        if(!lon){printf("--------point[%d] has no lon\n",i);return ret;}
        if(!lat){printf("--------point[%d] has no lat\n",i);return ret;}
        printf("--------point[%d]:lon=%d,lat=%d\n",i,lon->valueint,lat->valueint);
    }
    return 0;
}

static int check_lanes(cJSON *lanes)
{
    int ret = -1,i;
    int lane_num =  cJSON_GetArraySize(lanes);
    for(i=0;i<lane_num;i++){
        printf("----lane[%d] :\n",i);
        int point_num = 0;
        cJSON *lane = cJSON_GetArrayItem(lanes,i);
        cJSON *laneID = cJSON_GetObjectItem(lane,"laneID");
        cJSON *points = cJSON_GetObjectItem(lane,"points");
        if(laneID == NULL){printf("------lane has no laneId");return ret;}
        printf("------laneId=%d\n",laneID->valueint);
        if(points){
            if(check_points(points) != 0)return ret;
        }
    }
    return 0;
}

static int check_links(cJSON *links){
    int ret = -1,i;
    int link_num =  cJSON_GetArraySize(links);
    for(i=0;i<link_num;i++){
        printf("--link[%d] : \n",i);
        cJSON *link = cJSON_GetArrayItem(links,i);
        cJSON *upstreamNodeId = cJSON_GetObjectItem(link,"upstreamNodeId");
        cJSON *laneWidth = cJSON_GetObjectItem(link,"laneWidth");
        cJSON *lanes = cJSON_GetObjectItem(link,"lanes");
        if(upstreamNodeId == NULL){printf("----link has no upstreamNodeId\n");return ret;}
        if(laneWidth == NULL){printf("----link has no laneWidth\n");return ret;}
        if(lanes == NULL){printf("----link has no lanes\n");return ret;}
        int lane_num =  cJSON_GetArraySize(lanes);
        if(lane_num < 1){printf("----lane num < 1\n");return ret;}
        printf("----upstreamNodeId=%d,laneWidth=%d,lane_num=%d\n",upstreamNodeId->valueint,laneWidth->valueint,lane_num);
        if(check_lanes(lanes) != 0)return ret;
    }
    return 0;
}

static int check_map_json(cJSON *json){
    int ret = -1,i;
    if(json == NULL)return ret;

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    if(nodes == NULL){printf("no node\n");return ret;}
    int node_num =  cJSON_GetArraySize(nodes);
    printf("node num=%d\n",node_num);
    if(node_num < 1){printf("node num < 1\n");return ret;}

    for(i=0;i<node_num;i++){
        printf("node[%d] : \n",i);
        cJSON *node = cJSON_GetArrayItem(nodes,i);
        cJSON *id = cJSON_GetObjectItem(node,"id");
        cJSON *lon = cJSON_GetObjectItem(node,"lon");
        cJSON *lat = cJSON_GetObjectItem(node,"lat");
        cJSON *links = cJSON_GetObjectItem(node,"links");
        if(id == NULL){printf("--node has no id\n");return ret;}
        if(lon == NULL){printf("--node has no lon\n");return ret;}
        if(lat == NULL){printf("--node has no lat\n");return ret;}
        if(links == NULL){printf("--node has no links\n");return ret;}
        int link_num =  cJSON_GetArraySize(links);
        if(link_num < 1){printf("--link num < 1\n");return ret;}
        printf("--id=%d,lon=%d,lat=%d,link_num=%d\n",id->valueint,lon->valueint,lat->valueint,link_num);
        if(check_links(links) != 0)return ret;
    }

    return 0;
}




void encode_map(char *json_file, char *uper_file)
{
    int i,ii,iii,iiii;
    MessageFrame_t *msgframe = NULL;
    cJSON *json = read_json(json_file);
    printf("———————— check map json start ————————\n");
    int ret = check_map_json(json);
    printf("———————— check map json end : ret = %d ————————\n",ret);
    if(ret != 0)return;

    cJSON *nodes = cJSON_GetObjectItem(json,"nodes");
    int node_num =  cJSON_GetArraySize(nodes);

    msgframe = (MessageFrame_t*)calloc(1,sizeof(MessageFrame_t));
    msgframe->present = MessageFrame_PR_mapFrame;
    MAP_t *map = &msgframe->choice.mapFrame;
    map->msgCnt = 0;

    for(i=0;i<1;i++){
        Node_t *node = calloc(1,sizeof(Node_t));
        node->id.id = i;
        node->refPos.Long = 1111;
        node->refPos.lat = 2222;
        ASN_SET_ADD(&map->nodes.list,node);
    }

    encode(uper_file,msgframe);
}




