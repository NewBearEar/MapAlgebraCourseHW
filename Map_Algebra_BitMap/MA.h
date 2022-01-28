#pragma once
#ifndef MA_H
#include "Dist5Tmp.h"
#include "DistOctTmp.h"
#include "Dist13Tmp.h"
#include <vector>
#include <utility>
#include "MyPoint.h"
#include "DDALine.h"
using namespace std;
int BmpReverse(const char* InBmpName, const char* OutBmpName);
int Bmp8bitTo32bit(const char* InBmpName, const char* OutBmpName);
int BmpOverlay(const char* InBmpName1, const char*InBmpName2, const char*OutBmpName);
int mean33Smooth(const char* InBmpName, const char*OutBmpName);  //3×3平滑
int Bmp8BitDistTrans(const char*InBmpName, const char*OutDistBmp, const char*OutLocBmp,DistanceTemplate* pdisTmp);
int getVoronoiBoundary(const char*InBmpName, const char*OutBmpName);  //getBdyFromLoc从分配场获取边界
int getBufferFromDis(float bufferWidth, const char*InBmpName, const char*OutBmpName);//从距离场获得缓冲区边界
int getDelaunay(const char*InBmpName, const char*OutLocBmp,const char*OutVoronoiBmp,const char*OutDelauneyBmp, DistanceTemplate* pdisTmp);  //获取delaunay三角网
int getBufferSetBlack(float bufferWidth, const char * InBmpName, const char * InLocBmpName, const char * OutBmpName);//获取缓冲区并将外部设置为黑色
int adhesionTrans(float bufferWidth,const char*InBmpName, const char*OutsideDistBmp, const char*OutsideLocBmp,const char*InsideDistBmp, const char*InsideLocBmp, const char * OutsideBmp, const char * InsideBmp, DistanceTemplate* pdisTmp); //粘连变换
int getBufferSetWhite(float bufferWidth, const char * InBmpName, const char * InLocBmpName, const char * OutBmpName);  //获取缓冲区并将内部设置为白色
int Tif2Bmp(const char* InName, const char* OutName);
int Bmp2Tif(const char* InName, const char* OutName);
int TifOverlay(const char* InTifName1, const char* InTifName2, const char* OutTifName);
int TifReverse(const char* InTifName, const char* OutTifName);
int getTifBoundary(const char* InTifName, const char* OutTifName);
int TifDistTrans(const char* InTifName, const char* OutDistTif, const char* OutLocTif, DistanceTemplate* pdisTmp);
int getTifDelaunay(const char* InTifName, const char* OutLocTif, const char* OutVoronoiTif, const char* OutDelaunayTif, DistanceTemplate* pdisTmp);

#endif // !1

