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

//---------------------- print map -----------------------------

// 打印 lanes 中的 points
static void printPoints(PointList_t *list,int level,long ref_lng,long ref_lat)
{
    int i ,count;
    char *pre = getPreSuf(level,"points");
    count = list->list.count;
    for(i=0;i<count;i++){
        long lng,lat;
        RoadPoint_t *point = list->list.array[i];
        char *str = getPoint(&point->posOffset,&lng,&lat);
        mylog("%s*[%d/%d] : lng=%ld,lat=%ld (%ld,%ld) (%s)\n",pre,i+1,count,ref_lng+lng,ref_lat+lat,lng,lat,str);
    }
}

// 打印 links 中的 connectsTo
static void printConnectsTo(ConnectsToList_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"connectsTo");
    count = list->list.count;
    for(i=0;i<count;i++){
        Connection_t *con = list->list.array[i];
        mylog("%s*[%d/%d] : ",pre,i+1,count);
        if(con->connectingLane)mylog("connectingLane*=%ld,",con->connectingLane->lane);
        if(con->phaseId)mylog("phaseId*=%ld",*con->phaseId);
        mylog("\n");

        printNodeRefId(&con->remoteIntersection,level+1,"remoteIntersection");
    }
}

// 打印 links 中的 lanes
static void printLanes(LaneList_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"lanes");
    count = list->list.count;
    for(i=0;i<count;i++){
        Lane_t *lane = list->list.array[i];
        int point_count = 0,connect_count = 0, maneuvers_int = 0;
        if(lane->points)point_count = lane->points->list.count;
        if(lane->connectsTo)connect_count = lane->connectsTo->list.count;
        if(lane->maneuvers)memcpy(&maneuvers_int,lane->maneuvers->buf,lane->maneuvers->size);
        mylog("%s[%d/%d] : laneID=%ld,points*{%d},connectsTo*{%d}",
               pre,i+1,count,lane->laneID,point_count,connect_count,maneuvers_int);
        if(lane->maneuvers)mylog(",maneuvers*=%d",maneuvers_int);
        mylog("\n");

        if(lane->connectsTo)printConnectsTo(lane->connectsTo,level+1);
        if(lane->points)printPoints(lane->points,level+1,s_lng,s_lat);
        mylog("%s[%d/%d]\n",pre,i+1,count);
    }
}

// 打印 map 中的 speedLimits
static void printSpeedLimits(SpeedLimitList_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"speedLimits");
    count = list->list.count;
    for(i=0;i<count;i++){
        RegulatorySpeedLimit_t *limit = list->list.array[i];
        mylog("%s*[%d/%d] : type=%ld,speed=%.2lfm/s(%ld)\n",pre,i+1,count,limit->type,limit->speed*SPEED_RESOLUTION,limit->speed);
    }
}

// 打印 links 中的 movements
static void printMovements(MovementList_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"movements");
    count = list->list.count;
    for(i=0;i<count;i++){
        Movement_t *move = list->list.array[i];
        mylog("%s*[%d/%d] : ",pre,i+1,count);
        if(move->phaseId)mylog("phaseId*=%ld",*move->phaseId);
        mylog("\n");

        printNodeRefId(&move->remoteIntersection,level+1,"remoteIntersection");
    }
}

// 打印 map 中的 links
static void printLinks(LinkList_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"links");
    count = list->list.count;
    for(i=0;i<count;i++){
        Link_t *link = list->list.array[i];
        int lane_count = link->lanes.list.count;
        int limit_count = 0,movement_count = 0;
        if(link->speedLimits)limit_count = link->speedLimits->list.count;
        if(link->movements)movement_count = link->movements->list.count;
        mylog("%s*[%d/%d] : laneWidth=%ld,lanes{%d},speedLimits*{%d},movements*{%d}\n",
               pre,i+1,count,link->laneWidth,lane_count,limit_count,movement_count);

        printNodeRefId(&link->upstreamNodeId,level+1,"upstreamNodeId");
        if(link->speedLimits)printSpeedLimits(link->speedLimits,level+1);
        if(link->movements)printMovements(link->movements,level+1);
        printLanes(&link->lanes,level+1);
        mylog("%s*[%d/%d]\n",pre,i+1,count);
    }
}

// 打印 map 中的 nodes
void printNodes(NodeListltev_t *list,int level)
{
    int i ,count;
    char *pre = getPreSuf(level,"nodes");
    count = list->list.count;
    for(i=0;i<count;i++){
        Node_t *node = list->list.array[i];
        LinkList_t *linklist = node->inLinks;
        int link_count = 0;
        if(linklist)link_count = linklist->list.count;
        s_lng = node->refPos.Long;
        s_lat = node->refPos.lat;
        mylog("%s[%d/%d] : lng=%ld,lat=%ld,inLinks*{%d}\n",
              pre,i+1,count,node->refPos.Long,node->refPos.lat,link_count);
        printNodeRefId(&node->id,level+1,"id");

        if(linklist)printLinks(linklist,level+1);
        mylog("%s[%d/%d]\n",pre,i+1,count);
    }
}

// 打印 map
void mapPrint(MessageFrame_t *msg)
{
    int level = 0;
    char *pre = getPreSuf(level,"map");

    mylog("%s : \n",pre);
    MAP_t map = msg->choice.mapFrame;
    mylog("nodes[%d]\n",map.nodes.list.count);

    printNodes(&map.nodes,level+1);
    mylog("%s\n",pre);
}



