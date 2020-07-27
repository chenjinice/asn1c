#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>

#include "map_about.h"
#include "msg_common.h"
#include "common.h"
#include "point_algorithm.h"

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
        if(point->posOffset.offsetLL.present == PositionOffsetLL_PR_position_LatLon)
            mylog("*%s[%d/%d] : lng=%ld,lat=%ld (%s)\n",pre,i+1,count,lng,lat,str);
        else
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
        if(lane->maneuvers){
            int tmp = 0;
            memcpy(&tmp,lane->maneuvers->buf,lane->maneuvers->size);
            maneuvers_int |= byteReverse(tmp&0xFF);
            maneuvers_int |= byteReverse((tmp>>8)&0xFF) << 8;
        }
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
static void printNodes(NodeListltev_t *list,int level)
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

//
static void test_algorithm(NodeListltev_t *list)
{
    int i,j,k,m,node_id=-1,lane_id=-1;
    double min = 1e7;
    MyPointWGS84 p,a,b;

    p.lng = 121.1878775;p.lat = 31.2755822;

    for(i=0;i<list->list.count;i++){
        Node_t *node = list->list.array[i];
        LinkList_t *link_list = node->inLinks;

        for(j=0;j<link_list->list.count;j++){
            Link_t *link = link_list->list.array[j];
            LaneList_t *lane_list = &link->lanes;

            for(k=0;k<lane_list->list.count;k++){
                Lane_t *lane = lane_list->list.array[k];
                PointList_t *point_list = lane->points;

                for(m=0;m<point_list->list.count-1;m++){
                    long lng1,lat1,lng2,lat2;
                    int f;
                    double d;
                    RoadPoint_t *point1 = point_list->list.array[m];
                    RoadPoint_t *point2 = point_list->list.array[m+1];
                    getPoint(&point1->posOffset,&lng1,&lat1);
                    getPoint(&point2->posOffset,&lng2,&lat2);
                    a.lng = (lng1+node->refPos.Long)*1e-7;
                    a.lat = (lat1+node->refPos.lat)*1e-7;
                    b.lng = (lng2+node->refPos.Long)*1e-7;
                    b.lat = (lat2+node->refPos.lat)*1e-7;
                    d = minDistance(&p,&a,&b,&f);
                    if(d < min){
                        min = d;
                        node_id = node->id.id;
                        lane_id = lane->laneID;
                    }
                    //                    printf("node=%ld ,lane=%ld ,dist === %lf (%d)\n",node->id.id,lane->laneID,d,f);
                }
            }
        }
    }
    printf("node=%d ,lane=%d ,dist === %lf\n",node_id,lane_id,min);
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

    //    test_algorithm(&map.nodes);

    mylog("%s\n",pre);
}



