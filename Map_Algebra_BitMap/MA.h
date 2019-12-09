#pragma once
#ifndef MA_H
#include "Dist5Tmp.h"
#include "DistOctTmp.h"

int BmpReverse(const char* InBmpName, const char* OutBmpName);
int Bmp8bitTo32bit(const char* InBmpName, const char* OutBmpName);
int BmpOverlay(const char* InBmpName1, const char*InBmpName2, const char*OutBmpName);
int Bmp8BitDistTrans(const char*InBmpName, const char*OutBmpName,DistanceTemplate* pdisTmp);
int getVoronoiBoundary(const char*InBmpName, const char*OutBmpName);  //getBdyFromLoc从分配场获取边界
int getBufferFromDis(float bufferWidth, const char*InBmpName, const char*OutBmpName);//从距离场获得缓冲区边界
#endif // !1

