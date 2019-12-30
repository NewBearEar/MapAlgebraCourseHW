#include<stdio.h>
#include<Windows.h>
#include"pch.h"
#include"MA.h"
#include"BitMap.h"
#define Float_MAX FLT_MAX
#define Float_MIN 0.000001
#define Float_BARRIER 10000000.0
int BmpReverse(const char* InBmpName, const char* OutBmpName)   //��ɫ����
{
	BitMap bmp;
	FILE* file;
	errno_t err;
	err= fopen_s(&file,InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	//windows�涨һ��ɨ������ռ���ֽ���������4�ı���(����longΪ��λ),�������0���
	//�������Լ������
	/*�����fread�������BMP�ļ���Ϣ��ͼƬ��Ϣ���������� �ڴ���� ��ԭ��
	**�������Ϣ���ܴ�Ų����ڴ���Ԥ�ڵĵط�����#pragma pack(1)������Զ������������
	**���ݲ���ķ�ʽҲ����*/
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //�л���
	//���ݷ�ɫ
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			//if (lineBuf[j] == 0xFF) continue;  //255����ɫ����֤��ɫ��ɫ����
			bmp.BmpMtx[i][j] = 255 - lineBuf[j];
		}
	}
	//����λͼ
	FILE* filew;
	//errno_t err;
	//err = 
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp.ClrTab, 1024, 1, filew);
	//д��λͼ
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(bmp.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//����ɨβ,�������Զ����
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;

	return 0;
}

int Bmp8bitTo32bit(const char* InBmpName, const char* OutBmpName)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	BitMap bmp8, bmp32;
	BitMap* pbmp= &bmp8;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//8bit���ݶ�ȡ
	
	fread(&(pbmp->bmpHead), sizeof(BITMAPFILEHEADER), 1, file);  //���м����bmpHeadֱ�Ӷ�ȡ������ٸ�ֵ���ܸ���
	fread(&(pbmp->bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	if (pbmp->bmpInfo.biBitCount != 8)
	{
		printf("Input bitmap is not 8bits");
		return -1;
	}
	/************************************************************/
	//����ʹ��unsigned int�ĵ�ɫ����и�ֵ
	//unsigned int aColorsTab[256];
	//fread(aColorsTab, sizeof(unsigned int), 256, file);
	/************************************************************/
	//ʹ��unsigned char�ķ���
	fread(pbmp->ClrTab, sizeof(unsigned int), 256, file);  
	//256������1024������ͼ������ƽ�ƣ���Ϊд��unsigned int(һ��ʼдunsigned�Ҵ�����char)
	//fread(pbmp->ClrTab, sizeof(unsigned char), 1024, file);//����ʹ��
	/************************************************************/
	pbmp->BmpWidth = pbmp->bmpInfo.biWidth;
	pbmp->BmpHeight = pbmp->bmpInfo.biHeight;
	pbmp->MtxWidth = (pbmp->BmpWidth + 3) / 4 * 4; //8bitλͼ4��һ�飬���㲹ȫ
	pbmp->MtxHeight = pbmp->BmpHeight;
	pbmp->creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[pbmp->MtxWidth];  //�л���
	//��ȡ8bit
	for (int i = 0; i < pbmp->MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), pbmp->MtxWidth, file);
		for (int j = 0; j < pbmp->MtxWidth; j++)
		{
			//��ȡ���л���浽bmp����
			pbmp->BmpMtx[i][j] = lineBuf[j];
		}
	}
	//32λͼ����,��ȫ��������
	bmp32.BmpWidth = pbmp->BmpWidth;
	bmp32.BmpHeight = pbmp->BmpHeight;
	bmp32.MtxWidth = pbmp->MtxWidth;
	bmp32.MtxHeight = pbmp->MtxHeight;
	bmp32.creatBmp32bitMtx();  //����32λ����

	bmp32.bmpHead = pbmp->bmpHead;
	bmp32.bmpInfo = pbmp->bmpInfo;
	bmp32.bmpHead.bfOffBits = 54;
	bmp32.bmpHead.bfSize = sizeof(unsigned int)*(bmp32.BmpWidth*bmp32.BmpHeight) + 54;  //��ʵ�ʿ�ȼ���λͼsize
	bmp32.bmpInfo.biBitCount = 32;
	//���ݴ���
	for (int i = 0; i < pbmp->MtxHeight; i++)
	{
		for (int j = 0; j < pbmp->MtxWidth; j++)
		{
			/*int kk=pbmp->BmpMtx[i][j];
			int jj = pbmp->ClrTab[pbmp->BmpMtx[i][j]];
			//if (jj != 0) {
				int x = i;
				int y = j;

				printf("%d",jj);
			}//�����ɫֵ�Ĳ���*/
			//bmp32.Bmp32Mtx[i][j] = pbmp->ClrTab[pbmp->BmpMtx[i][j]]; //char���鲻��ֱ��������ֵ��ӦClrTab��RGBֵ
			//���ڵ�ɫ���RGBQUAD�����ĸ��ֽ�
			//��ˣ�Ӧ�öԵ�ɫ���е�ÿ����ɫ��4���ֽ����ݽ�������ϲ���һ��int��
			//����˵�ĳ�����Ϊunsigned int��ClrTab����
			/************************************************************/
			//ʹ��unsigned char�ķ���
			unsigned int ARGBint = 0;  //ע��int��ʾ��ɫ��˳��Alpha��R��G��B
			unsigned char Rchar, Gchar, Bchar; //��Ϊ��ɫ����ĸ��ֽں�Ϊ�㣬����ֻ��Ҫ�洢RGB
			Bchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4];   //ע����ɫ���ֽ�˳��ΪB��G��R
			Gchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4 + 1];
			Rchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4 + 2];
			//ARGBint = Rchar * pow(2, 16) + Gchar * pow(2, 8) + Bchar * pow(2, 0);
			ARGBint = Rchar << 16 | Gchar << 8 | Bchar;
			bmp32.Bmp32Mtx[i][j] = ARGBint;
			/************************************************************/
			//����ʹ��unsigned int�ĵ�ɫ����и�ֵ
			//bmp32.Bmp32Mtx[i][j] = aColorsTab[pbmp->BmpMtx[i][j]];  //���صߵ�RGB˳�������BMP�ĵ���
			/************************************************************/
			
		}
		//printf("\n");
	}

	//д���µ�32λλͼ�ļ�
	FILE* filew;
	fopen_s(&filew, OutBmpName, "wb");
	/*if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}*/
	fwrite(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fwrite(bmp32.Bmp32Mtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);  
		//ע��32λ���ض���4�ı����������bmpwidth���ô�����mtxwidth,����ͼ���ƫ��
	}
	//ɨβ����
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;

	return 0;
}

int BmpOverlay(const char * InBmpName1, const char * InBmpName2, const char * OutBmpName)
{
	BitMap bmp1,bmp2,bmp;
	FILE* file;
	errno_t err;
	//��ȡBMP1���ݲ�����
	err = fopen_s(&file, InBmpName1, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp1.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp1.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp1.ClrTab, 1024, 1, file);
	bmp1.BmpWidth = bmp1.bmpInfo.biWidth;
	bmp1.BmpHeight = bmp1.bmpInfo.biHeight;
	bmp1.MtxWidth = (bmp1.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	bmp1.MtxHeight = bmp1.BmpHeight;
	bmp1.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp1.MtxWidth];  //�л������ռ�
	//���ݷ�ɫ
	for (int i = 0; i < bmp1.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp1.MtxWidth, file);
		for (int j = 0; j < bmp1.MtxWidth; j++)
		{
			bmp1.BmpMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//��ȡBMP2���ݲ�����
	err = fopen_s(&file, InBmpName2, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp2.ClrTab, 1024, 1, file);
	bmp2.BmpWidth = bmp2.bmpInfo.biWidth;
	bmp2.BmpHeight = bmp2.bmpInfo.biHeight;
	bmp2.MtxWidth = (bmp2.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	bmp2.MtxHeight = bmp2.BmpHeight;
	bmp2.creatBmp8bitMtx();
	//���ݷ�ɫ
	for (int i = 0; i < bmp2.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp2.MtxWidth, file);
		for (int j = 0; j < bmp2.MtxWidth; j++)
		{
			bmp2.BmpMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//�������λͼ

	if (bmp1.bmpInfo.biBitCount != bmp2.bmpInfo.biBitCount &&
		bmp1.MtxHeight != bmp2.MtxHeight &&
		bmp1.MtxWidth != bmp2.MtxWidth) //�ж�����ͼƬ�Ƿ�ṹ��ͬ
	{
		printf("Two Bitmaps have different width , height or bit depth");
		return -2;
	}
	bmp.bmpHead = bmp1.bmpHead;
	bmp.bmpInfo = bmp1.bmpInfo;
	for (int i = 0; i < (sizeof(bmp1.ClrTab) / sizeof(unsigned char)); i++)
	{
		bmp.ClrTab[i] = bmp1.ClrTab[i];
	}
	bmp.BmpWidth = bmp1.bmpInfo.biWidth;
	bmp.BmpHeight = bmp1.bmpInfo.biHeight;
	bmp.MtxWidth = bmp1.MtxWidth; //4��һ�飬���㲹ȫ
	bmp.MtxHeight = bmp1.MtxHeight;
	bmp.creatBmp8bitMtx();

	FILE* filew;
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
fwrite(bmp.ClrTab, 1024, 1, filew);
//д��λͼ
for (int i = 0; i < bmp.MtxHeight; i++)
{
	for (int j = 0; j < bmp.MtxWidth; j++)  //���ú�������
	{
		bmp.BmpMtx[i][j] = (bmp1.BmpMtx[i][j] + bmp2.BmpMtx[i][j]) % 256 + 1;  //��Ӳ���256ȡ�ࣨ��ֹ���ޣ�����,%256+1��֤���Ӱ�ɫʱ��ɫ����
	}
	fwrite(bmp.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
}
//ɨβ����
fclose(filew);
delete[] lineBuf;
lineBuf = NULL;

return 0;
}

int mean33Smooth(const char* InBmpName, const char*OutBmpName)
{
	BitMap bmp,bmpmean;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //�л���
	//��ȡ����
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			bmp.BmpMtx[i][j] = lineBuf[j];
		}
	}
	//3��3��ֵƽ������
	float smoothTmp[3][3] = {1}; //ƽ��ģ��
	int smoothCount=0; //ƽ������
	bmpmean.bmpHead = bmp.bmpHead;
	bmpmean.bmpInfo = bmp.bmpInfo;
	bmpmean.BmpWidth = bmp.BmpWidth;
	bmpmean.BmpHeight = bmp.BmpHeight;
	bmpmean.MtxWidth = bmp.MtxWidth; //4��һ�飬���㲹ȫ
	bmpmean.MtxHeight = bmp.MtxHeight;
	bmpmean.creatBmp8bitMtx();
	for (int i = 0; i < bmp.BmpHeight; i++)
	{
		bmpmean.ClrTab[i] = bmp.ClrTab[i];
	}
	for (int i = 0; i < bmp.BmpHeight; i++)
	{
		for (int j = 0; j < bmp.BmpWidth; j++)
		{
			
			float total = 0;
			smoothCount = 0;
			for (int k = 0; k < 3 ; k++)
			{
				for (int m = 0; m < 3; m++)
				{
					int CX = j + m;
					int CY = i + k;  //ƫ������ѭ������������ģ�����
					//����λ�õĺϷ���
					if ((CY < 0) || (CY > bmp.BmpHeight - 1)) continue;
					if ((CX < 0) || (CX > bmp.BmpWidth - 1))continue;
					//���������Сֵ
					total += bmp.BmpMtx[CY][CX];
					smoothCount++;
				}
			}
			bmpmean.BmpMtx[i][j] = int(total/smoothCount);
		}
	}

	//����λͼ
	FILE* filew;
	//errno_t err;
	//err = 
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmpmean.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmpmean.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmpmean.ClrTab, 1024, 1, filew);
	//д��λͼ
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(bmpmean.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//����ɨβ,�������Զ����
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;

	return 0;
}

int Bmp8BitDistTrans(const char*InBmpName, const char*OutDistBmp, const char*OutLocBmp, DistanceTemplate* pdisTmp)
{
	BitMap bmp,bmp2,bmploc;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, sizeof(unsigned int), 256, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	//1.���䳡�;��볡�ռ�Ľ��������
	unsigned char** LocMtx = NULL;
	LocMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		LocMtx[i] = new unsigned char[bmp.MtxWidth];
	}
	float** DistMtx = NULL;
	DistMtx = new float*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		DistMtx[i] = new float[bmp.MtxWidth];
	}
	//2.��ȡλͼ���ݲ����Ƶ����䳡
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //�л���
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			bmp.BmpMtx[i][j] = lineBuf[j];
			LocMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//3.���ݷ��䳡��ʼ�����볡����
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			if (LocMtx[i][j] == 0xff) DistMtx[i][j] = Float_MAX;
			//�����ϰ����֣�Ϊ��ɫ
			else if (LocMtx[i][j] == 0x00) DistMtx[i][j] = Float_BARRIER;
			else
			{
				DistMtx[i][j] = 0;
			}
		}
	}
	//4.����ģ��ɨ�����
	
	//First ����->����ɨ��  //��ν����->���ϣ���ֻ�Ǹı���ģ�壬�����˳��ҲӰ�������ֵ!!!
	for (int i = bmp.BmpHeight-1; i >=0; i--)
	{
		for (int j = 0; j < bmp.BmpWidth; j++)
		{
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//�����ϰ����֣�Ϊ��ɫ
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;  //��ʼ����С����
			int locx = j;
			int	locy = i;
			for (int k = 0; k < pdisTmp->GetSize() / 2 + 1; k++)
			{

				int offx = pdisTmp->GetOffX(k);
				int offy = pdisTmp->GetOffY(k);  //offyΪ��ֵ
				
				float templateDis = pdisTmp->GetDist(k);
				//�����������ƫ��
				int CX = j + offx;
				int CY = i - offy;  //ƫ������ѭ������������ģ�����
				//����λ�õĺϷ���
				if ((CY < 0) || (CY > bmp.BmpHeight-1)) continue;
				if ((CX < 0) || (CX > bmp.BmpWidth-1))continue;
				if (fabs((fabs(DistMtx[CY][CX])- Float_MAX)) < Float_MIN)continue;//���������ľ������ֵ�ļ��㣬�ӿ��ٶȣ�
				if (fabs(fabs(templateDis - Float_MAX)) < Float_MIN)continue;//����ģ�������Ĳ���
				if (fabs((fabs(DistMtx[CY][CX]) - Float_BARRIER)) < Float_MIN)continue;//�����ϰ�����
				//���������Сֵ
				if ((DistMtx[CY][CX] + templateDis) < dismin)
				{
					dismin = DistMtx[CY][CX] +templateDis;
					locx = CX;
					locy = CY;
				}
			}	
			DistMtx[i][j] = dismin;	//�ó�����С���븳ֵ��ģ������λ��
			LocMtx[i][j] = LocMtx[locy][locx];	//�������Ŀ��Locֵ��Ϊģ������λ��Locֵ
		}
	}
	
	//Second ɨ��
	for (int i = 0; i <bmp.BmpHeight; i++)
	{
		for (int j = bmp.BmpWidth-1; j >=0; j--)
		{
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//�����ϰ����֣�Ϊ��ɫ
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;
			for (int k = pdisTmp->GetSize()/2; k < pdisTmp->GetSize(); k++)
			{
				int offx = pdisTmp->GetOffX(k);
				int offy = pdisTmp->GetOffY(k);  //offyΪ��ֵ
				float templateDis = pdisTmp->GetDist(k);
				//�����������ƫ��
				int CX = j + offx;
				int CY = i - offy;  //ע��,ƫ������ѭ������������ģ�����
				//����λ�õĺϷ���
				if ((CY < 0) || (CY > bmp.BmpHeight - 1)) continue;
				if ((CX < 0) || (CX > bmp.BmpWidth - 1))continue;
				if (fabs((fabs(DistMtx[CY][CX]) - Float_MAX)) < Float_MIN)continue;//���������ľ������ֵ�ļ��㣬�ӿ��ٶȣ�
				if (fabs(fabs(templateDis - Float_MAX)) < Float_MIN)continue;//����ģ�������Ĳ���
				if (fabs((fabs(DistMtx[CY][CX]) - Float_BARRIER)) < Float_MIN)continue;//�����ϰ�����

				//���������Сֵ
				if ((DistMtx[CY][CX] + templateDis) < dismin)
				{
					dismin = DistMtx[CY][CX] + templateDis;
					LocMtx[i][j] = LocMtx[CY][CX];	//�������Ŀ��Locֵ��Ϊģ������λ��Locֵ
				}
			}
			DistMtx[i][j] = dismin;	//�ó�����С���븳ֵ��ģ������λ��

		}
	}
	
	//�洢����,����������float���ͣ�32λ����
	FILE* filew;
	fopen_s(&filew, OutDistBmp, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	bmp.bmpHead.bfOffBits = 54;
	bmp.bmpHead.bfSize = sizeof(unsigned int)*(bmp.BmpHeight*bmp.BmpWidth) + 54;
	bmp.bmpInfo.biBitCount = 32;
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	
	//�þ������д��λͼ,float��intλ����ͬʱ
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(DistMtx[i], sizeof(unsigned int), bmp.BmpWidth, filew);//���ز���,int�Ѿ���4�ı���
	}
	
	
	
	//��������������8λ����
	filew;
	fopen_s(&filew, OutLocBmp, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	bmp2.bmpHead = bmp.bmpHead;
	bmp2.bmpInfo = bmp.bmpInfo;
	for (int i = 0; i < 1024; i++)
	{
		bmp2.ClrTab[i] = bmp.ClrTab[i];
	}
	fwrite(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(&(bmp2.ClrTab), sizeof(unsigned int), 256, filew);
	
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(LocMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);//8λͼ����4����
	}
	

	//ɨβ����
	fclose(filew);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp.BmpHeight; i++)
	{
		delete[] LocMtx[i];
		LocMtx[i] = NULL;
		delete[] DistMtx[i];
		DistMtx[i] = NULL;
	}
	delete[] LocMtx;
	LocMtx = NULL;
	delete[] DistMtx;
	DistMtx = NULL;

	return 0;
}

int getVoronoiBoundary(const char * InBmpName, const char * OutBmpName)
{
	BitMap bmp;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4��һ�飬���㲹ȫ
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //�л���

	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			bmp.BmpMtx[i][j] = lineBuf[j];
		}
	}

	//�߽����
	unsigned char** boundMtx = NULL;
	boundMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		boundMtx[i] = new unsigned char[bmp.MtxWidth];
	}
	//���䳡�߽���ȡ����
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			//�ĸ�λ��
			int CXL = j - 1;  //���
			int CXR = j + 1;  //��
			int CYD = i + 1;  //��
			int CYT = i - 1;  //��
			//����λ�õĺϷ���
			if ((CYT < 0) || (CYD > bmp.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp.BmpWidth - 1))continue;
			if (bmp.BmpMtx[i][j] == bmp.BmpMtx[CYT][j] && bmp.BmpMtx[i][j] == bmp.BmpMtx[CYD][j]
				&& bmp.BmpMtx[i][j] == bmp.BmpMtx[i][CXR] && bmp.BmpMtx[i][j] == bmp.BmpMtx[i][CXL])
			{
				boundMtx[i][j] = 0xff;
			}
			else
			{
				boundMtx[i][j] = bmp.BmpMtx[i][j];
			}

		}
	}


	//����λͼ
	FILE* filew;
	//errno_t err;
	//err = 
	err=fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp.ClrTab, 1024, 1, filew);
	//д��λͼ
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(boundMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//����ɨβ,�������Զ����
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp.BmpHeight; i++)
	{
		delete[] boundMtx[i];
		boundMtx[i] = NULL;
	}
	return 0;

}

int getBufferFromDis(float bufferWidth, const char * InBmpName, const char * OutBmpName)
{
	
	BitMap bmp32;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//���ݶ�ȡ
	fread(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);

	bmp32.BmpWidth = bmp32.bmpInfo.biWidth;
	bmp32.BmpHeight = bmp32.bmpInfo.biHeight;
	bmp32.MtxWidth = bmp32.BmpWidth; 
	bmp32.MtxHeight = bmp32.BmpHeight;
	bmp32.creatBmp32bitMtx();
	unsigned int * lineBuf = new unsigned int[bmp32.MtxWidth];  //�л���

	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned int), bmp32.MtxWidth, file);
		for (int j = 0; j < bmp32.MtxWidth; j++)
		{
			bmp32.Bmp32Mtx[i][j] = lineBuf[j];
		}
	}

	//����������
	unsigned int** bufferMtx = NULL;
	bufferMtx = new unsigned int*[bmp32.MtxHeight];
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		bufferMtx[i] = new unsigned int[bmp32.MtxWidth];
	}
	//����������
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		for (int j = 0; j < bmp32.MtxWidth; j++)
		{
			if (bmp32.Bmp32Mtx[i][j] <= bufferWidth)
			{
				bufferMtx[i][j] = 0xfff;
			}
			else
			{
				bufferMtx[i][j] = 0xffffff;
			}
		}
	}

	//���䳡�߽���ȡ����
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		for (int j = 0; j < bmp32.MtxWidth; j++)
		{
			//�ĸ�λ��
			int CXL = j - 1;  //���
			int CXR = j + 1;  //��
			int CYD = i + 1;  //��
			int CYT = i - 1;  //��
			//����λ�õĺϷ���
			if ((CYT < 0) || (CYD > bmp32.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp32.BmpWidth - 1))continue;
			if (bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYT][j] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYD][j]
				&& bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXR] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXL])
			{
				bufferMtx[i][j] = 0xffffff;
			}
			else
			{
				bufferMtx[i][j] = bufferMtx[i][j];
			}

		}
	}


	//����λͼ
	FILE* filew;
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	bmp32.bmpHead.bfOffBits = 54;
	bmp32.bmpHead.bfSize = sizeof(unsigned int)*(bmp32.BmpHeight*bmp32.BmpWidth) + 54;
	bmp32.bmpInfo.biBitCount = 32;
	fwrite(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);

	//�þ������д��λͼ,float��intλ����ͬʱ
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fwrite(bufferMtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);//���ز���,int�Ѿ���4�ı���
	}
	//����ɨβ,�������Զ����
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp32.BmpHeight; i++)
	{
		delete[] bufferMtx[i];
		bufferMtx[i] = NULL;
	}
	return 0;

}
