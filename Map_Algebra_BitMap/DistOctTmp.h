#pragma once
#include "DistanceTemplate.h"
#include <math.h>
#include <stdio.h>
#include <float.h>
//°Ë±ßÐÎOctagonÄ£°å
class DistOctTmp :
	public DistanceTemplate
{
private:
	int templateSize;
	float* m_DisTmp;
public:
	int GetSize();
	float GetDist(int i);
	int GetOffX(int i);
	int GetOffY(int i);
public:
	DistOctTmp();
	~DistOctTmp();
};

