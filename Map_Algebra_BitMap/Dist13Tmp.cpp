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
		m_DisTmp[i] = sqrt(double(offx*offx + offy * offy));  //��������֮ǰת��Ϊ����������������������һ��ҲҪע�ⲻ�ܳ�����Χ
	}
}


Dist13Tmp::~Dist13Tmp()
{
	delete[] m_DisTmp;
}


int Dist13Tmp::GetSize()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return this->templateSize;
}


float Dist13Tmp::GetDist(int i)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return m_DisTmp[i];
}


int Dist13Tmp::GetOffX(int i)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return i % 13 - 6;
}


int Dist13Tmp::GetOffY(int i)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return i / 13 - 6;
}
