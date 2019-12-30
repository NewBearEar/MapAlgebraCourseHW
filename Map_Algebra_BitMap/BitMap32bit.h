#pragma once
#include "BitMap.h"
class BitMap32bit :
	public BitMap
{
public:
	BitMap32bit();
	~BitMap32bit();
	virtual BITMAPTYPE getType();
};

