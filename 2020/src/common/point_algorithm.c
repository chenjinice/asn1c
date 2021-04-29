#include <unistd.h>
#include <stdio.h>
#include <math.h>


#include "point_algorithm.h"




// 度数转弧度
double degreeToRadian(double degree)
{
    return degree*M_PI/180;
}


// 经纬度点转为地心坐标系XYZ
void WGS84To3D(double lng,double lat,MyPoint3D *p)
{
    double r_lng = degreeToRadian(lng);
    double r_lat = degreeToRadian(lat);
    p->x = cos(r_lat)*cos(r_lng)*EARTH_RADIUS;
    p->y = cos(r_lat)*sin(r_lng)*EARTH_RADIUS;
    p->z = sin(r_lat)*EARTH_RADIUS;
}


// 2维点p绕点reference(参考点)逆时针旋转degree度数后的坐标值
void rotate2D(MyPoint2D *reference,MyPoint2D *p,double degree)
{
    double radian = degreeToRadian(degree);
    double cos_value = cos(degree);
    double sin_value = sin(degree);
    double dx = p->x - reference->x;
    double dy = p->y - reference->y;
    double x = cos_value*dx - sin_value*dy + reference->x;
    double y = sin_value*dx + cos_value*dy + reference->y;
    p->x = x;
    p->y = y;
}


// 3维点p绕轴refrence 旋转(貌似是顺时针)degree度后的坐标值
void rotate3D(MyPoint3D *reference,MyPoint3D *p,double degree)
{
        double radian = degreeToRadian(degree);
        double x2 = reference->x*reference->x;
        double y2 = reference->y*reference->y;
        double z2 = reference->z*reference->z;
        double d2 = x2+y2+z2;
        double d  = sqrt(d2);
        double sina = sin(radian);
        double cosa = cos(radian);
        double x = (x2+(y2+z2)*cosa)/d2*p->x + (reference->x*reference->y*(1-cosa)/d2-reference->z*sina/d)*p->y
                  + (reference->x*reference->z*(1-cosa)/d2+reference->y*sina/d)*p->z;
        double y = (reference->y*reference->x*(1-cosa)/d2+reference->z*sina/d)*p->x + (y2+(x2+z2)*cosa)/d2*p->y
                  + (reference->y*reference->z*(1-cosa)/d2-reference->x*sina/d)*p->z;
        double z = (reference->z*reference->x*(1-cosa)/d2 - reference->y*sina/d)*p->x
                  + (reference->z*reference->y*(1-cosa)/d2+reference->x*sina/d)*p->y + (z2+(x2+y2)*cosa)/d2*p->z;
        p->x = x;
        p->y = y;
        p->z = z;
}


// 两个经纬度点之间求距离(Haversine公式)
double getDistance(double lng1,double lat1 ,double lng2,double lat2){
    double r_lng1 = degreeToRadian(lng1);
    double r_lat1 = degreeToRadian(lat1);
    double r_lng2 = degreeToRadian(lng2);
    double r_lat2 = degreeToRadian(lat2);

    double d_lng = r_lng2 - r_lng1;
    double d_lat = r_lat2 - r_lat1;

    double tmp = sin(d_lat/2)*sin(d_lat/2) + cos(r_lat1)*cos(r_lat2)*sin(d_lng/2)*sin(d_lng/2);
    double value = 2*asin(sqrt(tmp))*EARTH_RADIUS;
    return value;
}


// 经纬度p点到线段ab的最小距离计算-面积法, 效率不如矢量法
double minDistanceArea(MyPointWGS84 *p ,MyPointWGS84 *a,MyPointWGS84 *b){
    double pa = getDistance(p->lng,p->lat,a->lng,a->lat);
    double pb = getDistance(p->lng,p->lat,b->lng,b->lat);
    double ab = getDistance(a->lng,a->lat,b->lng,b->lat);

    double pa_pow = pa*pa;
    double pb_pow = pb*pb;
    double ab_pow = ab*ab;

    if(pa_pow+ab_pow <= pb_pow)return pa;
    if(pb_pow+ab_pow <= pa_pow)return pb;

    double half = (pa+pb+ab)/2;
    double s = sqrt(half*(half-pa)*(half-pb)*(half-ab));
    return (2*s/ab);
}


// 经纬度点p到线段ab的最短距离计算-矢量法
// flag会被设置为： 0(p到ab有垂线),-1(p在向量ab后方)，-2(p在向量ab前方)
double minDistance(MyPointWGS84 *p ,MyPointWGS84 *a,MyPointWGS84 *b,int *flag){
    if(flag!=NULL)*flag = 0;
    MyPoint3D op = {0} , oa = {0}, ob = {0};
    WGS84To3D(p->lng,p->lat,&op);
    WGS84To3D(a->lng,a->lat,&oa);
    WGS84To3D(b->lng,b->lat,&ob);

    // ap与ab向量乘积为负数，说明角pab为钝角，p在点a的外侧，最短距离为ap的长度
    double cross = (op.x-oa.x)*(ob.x-oa.x) + (op.y-oa.y)*(ob.y-oa.y) + (op.z-oa.z)*(ob.z-oa.z);
    if(cross <= 0){
        if( (cross < 0) && (flag!=NULL) )*flag = -1;
        return sqrt( (op.x-oa.x)*(op.x-oa.x) +(op.y-oa.y)*(op.y-oa.y) + (op.z-oa.z)*(op.z-oa.z) );
    }

    // ap与ab向量乘积大于ab的平方，说明ap在ab上的投影超过ab长度，则p在b的外侧，最短距离为bp的长度
    double ab = (oa.x-ob.x)*(oa.x-ob.x) + (oa.y-ob.y)*(oa.y-ob.y) + (oa.z-ob.z)*(oa.z-ob.z);
    if(cross >= ab){
        if( (cross > ab) && (flag!=NULL) )*flag = -2;
        return sqrt( (op.x-ob.x)*(op.x-ob.x) +(op.y-ob.y)*(op.y-ob.y) + (op.z-ob.z)*(op.z-ob.z) );
    }

    // 求ap到ab上垂线长度
    double r = cross/ab;
    double dx = oa.x + r*(ob.x-oa.x);
    double dy = oa.y + r*(ob.y-oa.y);
    double dz = oa.z + r*(ob.z-oa.z);
    return sqrt( (op.x-dx)*(op.x-dx) + (op.y-dy)*(op.y-dy) + (op.z-dz)*(op.z-dz) );
}


// 根据p1的位置和方位角，求p2在p1的哪个位置, pos.x为左右距离，pos.y为前后距离,
// 返回值为两点之间线段距离(这里没用Haversine公式，因为耗时多些，且短距离两者差别不大，上百公里才相差一米)
double getRelativePos(MyPointWGS84 *p1 , MyPointWGS84 *p2,double p1_degree,MyPoint2D *pos){
    p2->lng -= p1->lng;
    p1->lng = 0;
    MyPoint3D a = {0} , b = {0} , c = {0};
    WGS84To3D(p1->lng,p1->lat,&a);
    WGS84To3D(p2->lng,p2->lat,&b);

    c.x = b.x-a.x;
    c.y = b.y-a.y;
    c.z = b.z-a.z;
    MyPoint3D ref = {fabs(c.x),0,0};
    rotate3D(&ref,&c,p1_degree);
    pos->x = c.y;   // 左右距离
    pos->y = sqrt(c.x*c.x+c.z*c.z)*(c.z/fabs(c.z)); // 前后距离
    // 返回两点之间线段
    return sqrt(c.x*c.x+c.z*c.z+c.y*c.y);
}

