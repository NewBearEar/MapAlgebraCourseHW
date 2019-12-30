#pragma once
#include<iostream>
#include<string>
#include<stdio.h>
#include<Windows.h>

typedef enum
{
	BIT8, BIT32
}BITMAPTYPE;

class BitMap
{
public:
	unsigned k;
	BITMAPFILEHEADER bmpHead;
	BITMAPINFOHEADER bmpInfo;
	unsigned char ClrTab[1024]; //一开始写的char导致ClrTab读取的值变为负值。
	//unsigned int ClrTab[256]//如果这里定义成unsigned int就可以使用老师的方法赋值位图RGB了
	//RGBQUAD aColors[1024];
	int BmpWidth, BmpHeight;
	int MtxWidth, MtxHeight;
	unsigned char** BmpMtx;  //8位图的矩阵
	unsigned int** Bmp32Mtx;   //32位图矩阵
public:
	BitMap();
	~BitMap();
public:
	void creatBmp8bitMtx();
	void creatBmp32bitMtx();
	
	virtual BITMAPTYPE getType();
};

