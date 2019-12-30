#pragma once
#include "DistanceTemplate.h"
#include <math.h>
//13¡Á13µÄÄ£°å
class Dist13Tmp :
	public DistanceTemplate
{
private:
	int templateSize;
	float* m_DisTmp;
public:
	Dist13Tmp();
	~Dist13Tmp();
	int GetSize();
	float GetDist(int i);
	int GetOffX(int i);
	int GetOffY(int i);
};

