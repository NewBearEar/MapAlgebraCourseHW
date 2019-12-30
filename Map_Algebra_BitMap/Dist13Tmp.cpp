#include "pch.h"
#include "Dist13Tmp.h"


Dist13Tmp::Dist13Tmp()
{
	templateSize = 13*13;
	m_DisTmp = new float[templateSize];
	for (int i = 0; i < templateSize; i++)
	{
		int offx = GetOffX(i);
		int offy = GetOffY(i);
		m_DisTmp[i] = sqrt(double(offx*offx + offy * offy));  //整数开方之前转换为浮点数，浮点数距离运算一定也要注意不能超出范围
	}
}


Dist13Tmp::~Dist13Tmp()
{
	delete[] m_DisTmp;
}


int Dist13Tmp::GetSize()
{
	// TODO: 在此处添加实现代码.
	return this->templateSize;
}


float Dist13Tmp::GetDist(int i)
{
	// TODO: 在此处添加实现代码.
	return m_DisTmp[i];
}


int Dist13Tmp::GetOffX(int i)
{
	// TODO: 在此处添加实现代码.
	return i % 13 - 6;
}


int Dist13Tmp::GetOffY(int i)
{
	// TODO: 在此处添加实现代码.
	return i / 13 - 6;
}
