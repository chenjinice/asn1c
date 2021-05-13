#ifndef __ENCODE_RSI_H__
#define __ENCODE_RSI_H__


#include <iostream>
#include <vector>
#include <stdint.h>
#include "cJSON.h"
#include "MessageFrame.h"


using namespace std;


// WGS84经纬度点
class PosWGS84{
public:
    double lng          = 0;               // 真实经度,度数
    double lat          = 0;               // 真实纬度,度数
    double elev         = 0;               // 高度，基本没用到,单位:m
};


// -------------------rsi 相关 -----------------------------
// 影响路径
class LocalRefPath
{
public:
    double                  radius              = 0;        // 影响路径的半径
    vector<PosWGS84>        points;                         // 路径上的经纬度点，如果只有一个，则是影响一个圆圈
};

// rsi的交通事件
class LocalRtes
{
public:
    void show() const{
        printf("--rtes:id=%d,type=%d,des=%s,%d paths,pos(%.07lf,%.07lf)\n",rte_id,type,des.c_str(),(int)paths.size(),event_pos.lng,event_pos.lat);
        for(size_t i=0;i<paths.size();i++){
            printf("----path[%d]:radius=%.02lf\n",(int)i,paths[i].radius);
            const LocalRefPath &p = paths[i];
            for(size_t m=0;m<p.points.size();m++){
                printf("------pos[%d]:lng=%.07lf,lat=%.07lf\n",(int)m,p.points[m].lng,p.points[m].lat);
            }
        }
    }
    int                     rte_id              = 0;
    int                     type                = 0;        // GB/T 29100-2012
    int                     source              = 0;        // 信息来源
    string                  des;                            // description
    PosWGS84                event_pos;
    vector<LocalRefPath>    paths;                          // ReferencePath,影响路径
};

// rsi的交通标志
class LocalRtss
{
public:
    void show() const{
        printf("--rtes:id=%d,type=%d,des=%s,%d paths,pos(%.07lf,%.07lf)\n",rts_id,type,des.c_str(),(int)paths.size(),sign_pos.lng,sign_pos.lat);
        for(size_t i=0;i<paths.size();i++){
            printf("----path[%d]:radius=%.02lf\n",(int)i,paths[i].radius);
            const LocalRefPath &p = paths[i];
            for(size_t m=0;m<p.points.size();m++){
                printf("------pos[%d]:lng=%.07lf,lat=%.07lf\n",(int)m,p.points[m].lng,p.points[m].lat);
            }
        }
    }
    int                     rts_id              = 0;
    int                     type                = 0;        // GB 5768.2-2009
    string                  des;                            // description
    PosWGS84                sign_pos;
    vector<LocalRefPath>    paths;                          // ReferencePath,影响路径
};

// rsi
class LocalRsi
{
public:
    void  show() const{
        printf("rsi:id=%s,lng=%.07lf,lat=%.07lf,%d rtes ,%d rtss\n",id.c_str(),pos.lng,pos.lat,(int)rtes.size(),(int)rtss.size());
        for (size_t i=0;i<rtes.size();i++){
            rtes[i].show();
        }
        for (size_t i=0;i<rtss.size();i++){
            rtss[i].show();
        }
    }

    int                  msg_count          = 0;        // 0 ~ 127 , 可用于统计丢包情况
    timeval              tv                 = {0};      // 时间结构体

    string               id;                            // rsu id
    PosWGS84             pos;                           // 参考点位置
    vector<LocalRtes>    rtes;                          // 交通事件
    vector<LocalRtss>    rtss;                          // 交通标志
};

//----------------------------------------------------------


void rsiPrint(MessageFrame_t *msg);
void rsiEncode(cJSON *json,char *uper_file);


#endif

