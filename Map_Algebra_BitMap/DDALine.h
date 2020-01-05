#pragma once
#include "MyPoint.h"
#include <math.h>
#include "BitMap.h"
class DDALine
{
public:
	DDALine(MyPoint *ptOrg,MyPoint *ptEnd);
	~DDALine();
	MyPoint ptOrg, ptEnd;
	void draw(unsigned char** bmpMtx);
};

