#include "pch.h"
#include "BitMap.h"


BitMap::BitMap()
{
	BmpMtx = NULL;
	Bmp32Mtx = NULL;
	BmpHeight = 0;
	BmpWidth = 0;
	MtxWidth = 0;
	MtxHeight = 0;
}


BitMap::~BitMap()
{
	if (BmpMtx != NULL)
	{
		for (int i = 0; i < MtxHeight; i++)
		{
			delete[] BmpMtx[i];
			BmpMtx[i] = NULL;
		}
		delete[] BmpMtx;
		BmpMtx = NULL;
	}

	if (Bmp32Mtx != NULL)
	{
		for (int i = 0; i < MtxHeight; i++)
		{
			delete[] Bmp32Mtx[i];
			Bmp32Mtx[i] = NULL;
		}
		delete[] Bmp32Mtx;
		Bmp32Mtx = NULL;
	}
}

void BitMap::creatBmp8bitMtx()
{
	if (BmpMtx != NULL || BmpHeight == 0 || BmpWidth == 0) 
	{
		return;
	}
	BmpMtx = new unsigned char*[BmpHeight];
	for (int i = 0; i < BmpHeight; i++)
	{
		BmpMtx[i] = new unsigned char[MtxWidth];
		for (int j = 0; j < MtxWidth; j++)  //MtxWidth保证所有原有以及内存对齐的部分都初始化
		{
			BmpMtx[i][j] = 0xFF;
		}
	}
}


void BitMap::creatBmp32bitMtx()
{
	// TODO: 在此处添加实现代码.
	if (Bmp32Mtx != NULL || BmpHeight == 0 || BmpWidth == 0)
	{
		return;
	}
	Bmp32Mtx = new unsigned int*[BmpHeight];
	for (int i = 0; i < BmpHeight; i++)
	{
		Bmp32Mtx[i] = new unsigned int[MtxWidth];
		for (int j = 0; j < MtxWidth; j++)  
		{
			Bmp32Mtx[i][j] = 0xFF;
		}
	}
}



