#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "rsi_about.h"
#include "convert_common.h"
#include "common.h"


const int res_pos = 1e7;


cJSON *getItem(cJSON *json , string key,bool required = true){
    cJSON *tmp = cJSON_GetObjectItem(json,key.c_str());

    if(tmp == nullptr && required){
        myerr("no %s\n",key.c_str());
    }
    return tmp;
}


// ---------------------------------- json to local ---------------------------------------

bool posToLocal(cJSON *json,PosWGS84 &pos){
    if(!json)return false;
    cJSON *lng = getItem(json,"lng");
    cJSON *lat = getItem(json,"lat");
    if(lng == nullptr)return false;
    if(lat == nullptr)return false;

    pos.lng = lng->valuedouble;
    pos.lat = lat->valuedouble;
    return true;
}

bool pointsToLocal(cJSON *json,vector<PosWGS84> &v){
    bool ret            = false;
    if(!json)return ret;
    int count           = cJSON_GetArraySize(json);
    for(int i=0;i<count;i++){
        PosWGS84 l_point;
        cJSON * point   = cJSON_GetArrayItem(json,i);

        if( !posToLocal(point,l_point) )return ret;
        v.push_back(l_point);
    }
    return true;
}

bool refPathsToLocal(cJSON *json,vector<LocalRefPath> &v){
    bool ret                = false;
    if(!json)return ret;
    int count               = cJSON_GetArraySize(json);
    for(int i=0;i<count;i++){
        LocalRefPath l_path;
        cJSON * path        = cJSON_GetArrayItem(json,i);
        cJSON * activePath  = getItem(path,"activePath");
        cJSON * pathRadius  = getItem(path,"pathRadius");

        if(!activePath)return ret;
        l_path.radius       = pathRadius->valuedouble;

        if( (!pathRadius) || (!pointsToLocal(activePath,l_path.points)) )return ret;
        v.push_back(l_path);
    }
    return true;
}

bool rtesToLocal(cJSON *json,vector<LocalRtes> &v){
    bool ret            = false;
    if(!json)return ret;
    int count           = cJSON_GetArraySize(json);
    for(int i=0;i<count;i++){
        LocalRtes  l_rte;
        cJSON *rte      = cJSON_GetArrayItem(json,i);
        cJSON *type     = getItem(rte,"eventType");
        cJSON *des      = getItem(rte,"description",false);
        cJSON *eventPos = getItem(rte,"eventPos",false);
        cJSON *paths    = getItem(rte,"referencePaths",false);

        if(!type)return ret;

        l_rte.type      = type->valueint;
        if(des)l_rte.des= des->valuestring;
        if( eventPos && (!posToLocal(eventPos,l_rte.event_pos)) )return ret;
        if( paths    && (!refPathsToLocal(paths,l_rte.paths))   )return ret;
        v.push_back(l_rte);
    }
    return true;
}

bool rtssToLocal(cJSON *json,vector<LocalRtss> &v){
    bool ret            = false;
    if(!json)return ret;
    int count           = cJSON_GetArraySize(json);
    for(int i=0;i<count;i++){
        LocalRtss  l_rts;
        cJSON *rts      = cJSON_GetArrayItem(json,i);
        cJSON *type     = getItem(rts,"signType");
        cJSON *des      = getItem(rts,"description",false);
        cJSON *signPos  = getItem(rts,"signPos",false);
        cJSON *paths    = getItem(rts,"referencePaths",false);

        if(!type)return ret;

        l_rts.type      = type->valueint;
        if(des)l_rts.des= des->valuestring;
        if( signPos && (!posToLocal(signPos,l_rts.sign_pos)) )return ret;
        if( paths   && (!refPathsToLocal(paths,l_rts.paths)) )return ret;
        v.push_back(l_rts);
    }
    return true;
}

//  rsi的json转本地
static bool rsiToLocal(cJSON *json,LocalRsi &rsi)
{
    bool    ret             = false;
    cJSON *refPos           = getItem(json,"refPos");
    cJSON *rtss             = getItem(json,"rtss",false);
    cJSON *rtes             = getItem(json,"rtes",false);

    if( !posToLocal(refPos,rsi.pos) )return ret;
    if( rtss && (!rtssToLocal(rtss,rsi.rtss)) )return ret;
    if( rtes && (!rtesToLocal(rtes,rsi.rtes)) )return ret;

    return true;
}





// ---------------------------------- add ---------------------------------------

void setRefPos(const PosWGS84 &l,Position3D_t &p)
{
    p.Long = l.lng *res_pos;
    p.lat  = l.lat *res_pos;
}

void setId(const string &l,OCTET_STRING &id)
{
    int len     = 8;
    id.buf      = new uint8_t[len]();
    id.size     = len;

    int data_len = l.length();
    int copy_len = data_len > len ? len : data_len;
    memcpy(id.buf,l.data(),copy_len);
}

void setoffsetPos(const PosWGS84 &l,PositionOffsetLLV &offset)
{
    offset.offsetLL.present = PositionOffsetLL_PR_position_LatLon;
    offset.offsetLL.choice.position_LatLon.lon = l.lng;
    offset.offsetLL.choice.position_LatLon.lat = l.lat;
}

void setRtss(const vector<LocalRtss> &v,RTSList *rtss)
{
    int count = v.size();
    for(int i=0;i<count;i++){
        const LocalRtss &l  = v[i];
        RTSData *rts        = new RTSData();
        rts->rtsId          = 0;
        rts->signType       = l.type;
        rts->signPos;

        ASN_SET_ADD(&rtss->list,rts);
    }
}

// 从文件读取 rsi 的 json 数据， 并用asn编码保存到文件
void rsiEncode(cJSON *json, char *uper_file)
{
    // json 转本地
    LocalRsi l;
    if( !rsiToLocal(json,l) ){ myerr("rsiToLocal error\n");return; }
    cJSON_Delete(json);

    l.show();

    MessageFrame_t * msgframe   = new MessageFrame_t();
    msgframe->present           = MessageFrame_PR_rsiFrame;
    RoadSideInformation_t *rsi  = &msgframe->choice.rsiFrame;

    setId(l.id,rsi->id);
    setRefPos(l.pos,rsi->refPos);

    if(l.rtss.size() > 0){
        RTSList *rtss           = new RTSList();
        setRtss(l.rtss,rsi->rtss);
    }

    encode(uper_file,msgframe);
    ASN_STRUCT_FREE(asn_DEF_MessageFrame, msgframe);
}


