#ifndef __COMMON_H__
#define __COMMON_H__


#include "cJSON.h"
#include "MessageFrame.h"

#define U8_MAX     255
#define U16_MAX    65535

#define BUFF_SIZE 65535
#define LOG_SIZE    300
#define PRE_SIZE    40

#define LON_MAX     (180*1e7)
#define LAT_MAX     (90*1e7)

#define LANEWIDTH_MAX 32767

#define NODEID_MAX  65535
#define NODEID_MIN  0
#define LANEID_MAX  255
#define LANEID_MIN  0
#define PHASEID_MAX 255
#define PHASEID_MIN 0

#define SPEEDTYPE_MAX  SpeedLimitType_vehiclesWithTrailersNightMaxSpeed
#define SPEEDTYPE_MIN  SpeedLimitType_unknown
#define SPEED_MAX  163.82   // 8191*0.02
#define SPEED_MIN  0.0
#define SPEED_RESOLUTION  0.02

#define DEFAULT_UPSTREAMID 0
#define DEFAULT_LANEWIDTH  350


void get_pre(char *pre,char *name,int level);
int check_int(int num ,int min,int max,char *pre,char *name);
int check_double(double num ,double min,double max,char *pre,char *name);

// encode & decode
int get_file_size(char *path);
int read_file(char *path,uint8_t **buffer);
void write_file(char *path, uint8_t *buffer, int length);
cJSON *read_json(char *path);
void encode(char *path, MessageFrame_t *msg);
MessageFrame_t *decode(char *path);

// roadPoint
PositionOffsetLL_PR get_point_type(int lon, int lat, int bits);
void get_type_str(PositionOffsetLL_PR type,char *str);
void set_roadpoint(RoadPoint_t *point ,long lon,long lat,PositionOffsetLL_PR type);
void get_roadpoint(RoadPoint_t *point ,long *lon,long *lat);

#endif



