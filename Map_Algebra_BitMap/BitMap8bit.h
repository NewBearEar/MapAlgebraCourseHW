#pragma once
#include "BitMap.h"
class BitMap8bit :
	public BitMap
{
public:
	BitMap8bit();
	virtual ~BitMap8bit();
	virtual BITMAPTYPE getType();
};

