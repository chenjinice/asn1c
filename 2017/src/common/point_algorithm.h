#ifndef __POINT_ALGORITHM_H__
#define __POINT_ALGORITHM_H__


#define EARTH_RADIUS 6371000    // 地球半径 m


// 二维坐标点
typedef struct _MyPoint2D{
    double x;
    double y;
}MyPoint2D;


// 三维坐标点
typedef struct _MyPoint3D{
    double x;
    double y;
    double z;
}MyPoint3D;


// WGS84经纬度点
typedef struct _MyPointWGS84{
    double lng;
    double lat;
    double alt;  // 高度，基本没用到
}MyPointWGS84;


// 度数转弧度
double degreeToRadian(double degree);

// 经纬度点转为地心坐标系XYZ
void WGS84To3D(double lng,double lat,MyPoint3D *p);

// 2维点p绕点reference(参考点)逆时针旋转degree度数后的坐标值
void rotate2D(MyPoint2D *reference,MyPoint2D *p,double degree);

// 3维点p绕轴refrence 旋转(貌似是顺时针)degree度后的坐标值
void rotate3D(MyPoint3D *reference,MyPoint3D *p,double degree);

// 两个经纬度点之间求距离(Haversine公式)
double getDistance(double lng1,double lat1 ,double lng2,double lat2);

// 经纬度点p到线段ab的最短距离计算-矢量法
// flag会被设置为： 0(p到ab有垂线),-1(p在向量ab后方)，-2(p在向量ab前方)
double minDistance(MyPointWGS84 *p ,MyPointWGS84 *a,MyPointWGS84 *b,int *flag);

// 经纬度p点到线段ab的最小距离计算-面积法, 效率不如矢量法
double minDistanceArea(MyPointWGS84 *p ,MyPointWGS84 *a,MyPointWGS84 *b);

// 根据p1的位置和方位角，求p2在p1的哪个位置, pos.x为左右距离，pos.y为前后距离,
// 返回值为两点之间线段距离(这里没用Haversine公式，因为耗时多些，且短距离两者差别不大，上百公里才相差一米)
double getRelativePos(MyPointWGS84 *p1 , MyPointWGS84 *p2,double p1_degree,MyPoint2D *pos);




#endif






