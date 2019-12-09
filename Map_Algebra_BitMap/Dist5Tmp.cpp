#include "pch.h"
#include "Dist5Tmp.h"
#include <math.h>

int Dist5Tmp::GetSize()
{
	return this->templateSize;
}

float Dist5Tmp::GetDist(int i)
{
	return m_DisTmp[i];
}

int Dist5Tmp::GetOffX(int i)
{
	return i%5 -2;
}

int Dist5Tmp::GetOffY(int i)
{
	return i/5 - 2;
}

Dist5Tmp::Dist5Tmp()
{
	templateSize = 25;
	m_DisTmp = new float[templateSize];
	for (int i = 0; i < 25; i++)
	{
		int offx = GetOffX(i);
		int offy = GetOffY(i);
		m_DisTmp[i] = sqrt(double(offx*offx+offy*offy));  //��������֮ǰת��Ϊ����������������������һ��ҲҪע�ⲻ�ܳ�����Χ
	}
}


Dist5Tmp::~Dist5Tmp()
{
	delete[] m_DisTmp;
}
