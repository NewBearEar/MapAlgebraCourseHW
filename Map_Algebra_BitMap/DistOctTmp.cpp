#include "pch.h"
#include "DistOctTmp.h"
#define Float_MAX FLT_MAX
#define Float_MIN 0.000001
int DistOctTmp::GetSize()
{
	return this->templateSize;
}

float DistOctTmp::GetDist(int i)
{
	return m_DisTmp[i];
}

int DistOctTmp::GetOffX(int i)
{
	return i % 7 - 3;
}

int DistOctTmp::GetOffY(int i)
{
	return i / 7 - 3;
}



DistOctTmp::DistOctTmp()
{
	templateSize = 49;
	m_DisTmp = new float[templateSize];
	int mandis = 0;
	for (int i = 0; i < 49; i++)
	{
		int offx = GetOffX(i);
		int offy = GetOffY(i);
		mandis = abs(offx) + abs(offy);

		if (mandis == 0)
		{
			m_DisTmp[i] = 0;
		}
		else if (mandis == 1)
		{
			m_DisTmp[i] = 1.0;
		}
		else if (mandis > 1 && mandis <= 3)
		{
			if (abs(offx) == 3 || abs(offy) == 3)
			{
				m_DisTmp[i] = 3.0;
			}
			else
			{
				m_DisTmp[i] = 2.0;
			}
			
		}
		else if (mandis == 4)
		{
			m_DisTmp[i] = 3.0;
		}
		else
		{
			m_DisTmp[i] = Float_MAX;
		}
		
	}
}


DistOctTmp::~DistOctTmp()
{
	delete[] m_DisTmp;
}
