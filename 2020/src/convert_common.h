#ifndef __MSG_COMMON_H__
#define __MSG_COMMON_H__


#include "cJSON.h"
#include "MessageFrame.h"

#define U8_MAX     255
#define U16_MAX    65535

#define LOG_SIZE    300
#define PRE_SIZE    40

#define LNG_MAX     (180*10000000)
#define LAT_MAX     (90*10000000)

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

#define ALERTRADIUS_MAX  102.4  // 1024*0.1
#define ALERTRADIUS_RESOLUTION	0.1

#define DEFAULT_LANEWIDTH  350


typedef struct _MyPoint{
    int lng;
    int lat;
}MyPoint;


// check

int jsonStrLenRange(cJSON *json,int min,int max,char *pre,char *keyname);
int jsonArrayRange(cJSON *json, int min, int max, char *pre, char *keyname);
int jsonIntRange(cJSON *json, int min, int max, char *pre, char *keyname);
int jsonDoubleRange(cJSON *json, double min, double max, char *pre,char *keyname);
int jsonExists(cJSON *json , char *pre,char *keyname);

// common check
int refPosJsonCheck(cJSON *json, int level, char *keyname, long *lng_value, long *lat_value);
int nodeRefIDJsonCheck(cJSON *json,int level,char *keyname);

// common add
void addRefPos(Position3D_t *pos, cJSON *json, long *lng_value, long *lat_value);
void addNodeRefId(NodeReferenceID_t *nodeId,cJSON *json);
IA5String_t *addIA5String(cJSON *json);

// common print
void printNodeRefId(NodeReferenceID_t *nodeid ,int level,char *name);

// point
PositionOffsetLL_PR getOffsetLL(long ref_lng, long ref_lat, long lng, long lat, long *lng_offset, long *lat_offset, char **log);
void setOffsetLL(PositionOffsetLLV_t *point , long lng, long lat, PositionOffsetLL_PR type);
char *getPoint(PositionOffsetLLV_t *point , long *lng, long *lat);

#endif



