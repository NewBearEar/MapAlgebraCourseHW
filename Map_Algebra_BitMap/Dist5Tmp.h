#pragma once
#include "DistanceTemplate.h"
class Dist5Tmp :
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
	Dist5Tmp();
	virtual ~Dist5Tmp();
};

