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
	unsigned char ClrTab[1024]; //һ��ʼд��char����ClrTab��ȡ��ֵ��Ϊ��ֵ��
	//unsigned int ClrTab[256]//������ﶨ���unsigned int�Ϳ���ʹ����ʦ�ķ�����ֵλͼRGB��
	//RGBQUAD aColors[1024];
	int BmpWidth, BmpHeight;
	int MtxWidth, MtxHeight;
	unsigned char** BmpMtx;  //8λͼ�ľ���
	unsigned int** Bmp32Mtx;   //32λͼ����
public:
	BitMap();
	~BitMap();
public:
	void creatBmp8bitMtx();
	void creatBmp32bitMtx();
	
	virtual BITMAPTYPE getType();
};

