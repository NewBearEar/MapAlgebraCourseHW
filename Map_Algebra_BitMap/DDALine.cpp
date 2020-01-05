#include "pch.h"
#include "DDALine.h"


DDALine::DDALine(MyPoint *ptOrg, MyPoint *ptEnd)
{
	this->ptOrg = *ptOrg;
	this->ptEnd = *ptEnd;
}


DDALine::~DDALine()
{
}

void DDALine::draw(unsigned char** bmpMtx)
{
	int n, k, dx, dy;
	float xinc, yinc, x, y;
	dx = ptEnd.x - ptOrg.x;
	dy = ptEnd.y - ptOrg.y;
	if (abs(dx) > abs(dy))
	{
		n = abs(dx);
	}
	else
	{
		n = abs(dy);
	}
	xinc = (float)dx / n;
	yinc = (float)dy / n;
	x = ptOrg.x;
	y = ptOrg.y;
	for (k = 1; k <= n; k++)
	{
		bmpMtx[int(floor(y))][int(floor(x))] = 0;//画一个黑色像素点
		//SetPixel(*pDC, x, y, RGB(255, 0, 0));
		x += xinc;
		y += yinc;
	}
}
