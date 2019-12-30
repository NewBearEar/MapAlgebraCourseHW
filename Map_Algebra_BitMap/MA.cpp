#include<stdio.h>
#include<Windows.h>
#include"pch.h"
#include"MA.h"
#include"BitMap.h"
#define Float_MAX FLT_MAX
#define Float_MIN 0.000001
#define Float_BARRIER 10000000.0
int BmpReverse(const char* InBmpName, const char* OutBmpName)   //反色函数
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
	//数据读取
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	//windows规定一个扫描行所占的字节数必须是4的倍数(即以long为单位),不足的以0填充
	//下面是自己的理解
	/*如果用fread整体读入BMP文件信息和图片信息，可能由于 内存对齐 的原因，
	**读入的信息可能存放不到内存里预期的地方，用#pragma pack(1)解决可以对齐产生的问题
	**数据补足的方式也可以*/
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //行缓存
	//数据反色
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			//if (lineBuf[j] == 0xFF) continue;  //255不反色，保证白色底色不变
			bmp.BmpMtx[i][j] = 255 - lineBuf[j];
		}
	}
	//创建位图
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
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(bmp.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//清理扫尾,类析构自动清除
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;

	return 0;
}

int Bmp8bitTo32bit(const char* InBmpName, const char* OutBmpName)
{
	// TODO: 在此处添加实现代码.
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
	//8bit数据读取
	
	fread(&(pbmp->bmpHead), sizeof(BITMAPFILEHEADER), 1, file);  //用中间变量bmpHead直接读取，最后再赋值可能更好
	fread(&(pbmp->bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	if (pbmp->bmpInfo.biBitCount != 8)
	{
		printf("Input bitmap is not 8bits");
		return -1;
	}
	/************************************************************/
	//尝试使用unsigned int的调色板进行赋值
	//unsigned int aColorsTab[256];
	//fread(aColorsTab, sizeof(unsigned int), 256, file);
	/************************************************************/
	//使用unsigned char的方法
	fread(pbmp->ClrTab, sizeof(unsigned int), 256, file);  
	//256而不是1024，否则图像整体平移，因为写的unsigned int(一开始写unsigned我错当成了char)
	//fread(pbmp->ClrTab, sizeof(unsigned char), 1024, file);//或者使用
	/************************************************************/
	pbmp->BmpWidth = pbmp->bmpInfo.biWidth;
	pbmp->BmpHeight = pbmp->bmpInfo.biHeight;
	pbmp->MtxWidth = (pbmp->BmpWidth + 3) / 4 * 4; //8bit位图4个一组，不足补全
	pbmp->MtxHeight = pbmp->BmpHeight;
	pbmp->creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[pbmp->MtxWidth];  //行缓存
	//读取8bit
	for (int i = 0; i < pbmp->MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), pbmp->MtxWidth, file);
		for (int j = 0; j < pbmp->MtxWidth; j++)
		{
			//读取的行缓存存到bmp类中
			pbmp->BmpMtx[i][j] = lineBuf[j];
		}
	}
	//32位图创建,补全对象数据
	bmp32.BmpWidth = pbmp->BmpWidth;
	bmp32.BmpHeight = pbmp->BmpHeight;
	bmp32.MtxWidth = pbmp->MtxWidth;
	bmp32.MtxHeight = pbmp->MtxHeight;
	bmp32.creatBmp32bitMtx();  //创建32位矩阵

	bmp32.bmpHead = pbmp->bmpHead;
	bmp32.bmpInfo = pbmp->bmpInfo;
	bmp32.bmpHead.bfOffBits = 54;
	bmp32.bmpHead.bfSize = sizeof(unsigned int)*(bmp32.BmpWidth*bmp32.BmpHeight) + 54;  //用实际宽度计算位图size
	bmp32.bmpInfo.biBitCount = 32;
	//数据处理
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
			}//输出颜色值的测试*/
			//bmp32.Bmp32Mtx[i][j] = pbmp->ClrTab[pbmp->BmpMtx[i][j]]; //char数组不能直接用索引值对应ClrTab的RGB值
			//由于调色板的RGBQUAD含有四个字节
			//因此，应该对调色板中的每个颜色的4个字节数据进行运算合并到一个int中
			//或者说改成声明为unsigned int的ClrTab数组
			/************************************************************/
			//使用unsigned char的方法
			unsigned int ARGBint = 0;  //注意int表示颜色的顺序，Alpha，R、G、B
			unsigned char Rchar, Gchar, Bchar; //因为颜色表第四个字节恒为零，所以只需要存储RGB
			Bchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4];   //注意颜色表字节顺序为B、G、R
			Gchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4 + 1];
			Rchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4 + 2];
			//ARGBint = Rchar * pow(2, 16) + Gchar * pow(2, 8) + Bchar * pow(2, 0);
			ARGBint = Rchar << 16 | Gchar << 8 | Bchar;
			bmp32.Bmp32Mtx[i][j] = ARGBint;
			/************************************************************/
			//尝试使用unsigned int的调色板进行赋值
			//bmp32.Bmp32Mtx[i][j] = aColorsTab[pbmp->BmpMtx[i][j]];  //不必颠倒RGB顺序可能与BMP的倒叙
			/************************************************************/
			
		}
		//printf("\n");
	}

	//写入新的32位位图文件
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
		//注意32位不必对齐4的倍数，因此用bmpwidth不用处理后的mtxwidth,否则图像会偏移
	}
	//扫尾清理
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
	//读取BMP1数据并保存
	err = fopen_s(&file, InBmpName1, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//数据读取
	fread(&(bmp1.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp1.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp1.ClrTab, 1024, 1, file);
	bmp1.BmpWidth = bmp1.bmpInfo.biWidth;
	bmp1.BmpHeight = bmp1.bmpInfo.biHeight;
	bmp1.MtxWidth = (bmp1.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	bmp1.MtxHeight = bmp1.BmpHeight;
	bmp1.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp1.MtxWidth];  //行缓存分配空间
	//数据反色
	for (int i = 0; i < bmp1.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp1.MtxWidth, file);
		for (int j = 0; j < bmp1.MtxWidth; j++)
		{
			bmp1.BmpMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//读取BMP2数据并保存
	err = fopen_s(&file, InBmpName2, "rb");
	if (file == NULL)
	{
		printf("Input file is not found!");
		return -1;
	}
	//数据读取
	fread(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp2.ClrTab, 1024, 1, file);
	bmp2.BmpWidth = bmp2.bmpInfo.biWidth;
	bmp2.BmpHeight = bmp2.bmpInfo.biHeight;
	bmp2.MtxWidth = (bmp2.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	bmp2.MtxHeight = bmp2.BmpHeight;
	bmp2.creatBmp8bitMtx();
	//数据反色
	for (int i = 0; i < bmp2.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp2.MtxWidth, file);
		for (int j = 0; j < bmp2.MtxWidth; j++)
		{
			bmp2.BmpMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//创建结果位图

	if (bmp1.bmpInfo.biBitCount != bmp2.bmpInfo.biBitCount &&
		bmp1.MtxHeight != bmp2.MtxHeight &&
		bmp1.MtxWidth != bmp2.MtxWidth) //判断两张图片是否结构相同
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
	bmp.MtxWidth = bmp1.MtxWidth; //4个一组，不足补全
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
//写入位图
for (int i = 0; i < bmp.MtxHeight; i++)
{
	for (int j = 0; j < bmp.MtxWidth; j++)  //叠置核心运算
	{
		bmp.BmpMtx[i][j] = (bmp1.BmpMtx[i][j] + bmp2.BmpMtx[i][j]) % 256 + 1;  //相加并除256取余（防止超限）叠加,%256+1保证叠加白色时颜色不变
	}
	fwrite(bmp.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
}
//扫尾清理
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
	//数据读取
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //行缓存
	//读取数据
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			bmp.BmpMtx[i][j] = lineBuf[j];
		}
	}
	//3×3均值平滑运算
	float smoothTmp[3][3] = {1}; //平滑模板
	int smoothCount=0; //平滑计数
	bmpmean.bmpHead = bmp.bmpHead;
	bmpmean.bmpInfo = bmp.bmpInfo;
	bmpmean.BmpWidth = bmp.BmpWidth;
	bmpmean.BmpHeight = bmp.BmpHeight;
	bmpmean.MtxWidth = bmp.MtxWidth; //4个一组，不足补全
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
					int CY = i + k;  //偏移量和循环变量决定了模板情况
					//检验位置的合法性
					if ((CY < 0) || (CY > bmp.BmpHeight - 1)) continue;
					if ((CX < 0) || (CX > bmp.BmpWidth - 1))continue;
					//计算距离最小值
					total += bmp.BmpMtx[CY][CX];
					smoothCount++;
				}
			}
			bmpmean.BmpMtx[i][j] = int(total/smoothCount);
		}
	}

	//创建位图
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
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(bmpmean.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//清理扫尾,类析构自动清除
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
	//数据读取
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, sizeof(unsigned int), 256, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	//1.分配场和距离场空间的建立与分配
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
	//2.读取位图数据并复制到分配场
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //行缓存
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

	//3.根据分配场初始化距离场矩阵
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			if (LocMtx[i][j] == 0xff) DistMtx[i][j] = Float_MAX;
			//加入障碍部分，为黑色
			else if (LocMtx[i][j] == 0x00) DistMtx[i][j] = Float_BARRIER;
			else
			{
				DistMtx[i][j] = 0;
			}
		}
	}
	//4.根据模板扫描计算
	
	//First 左下->右上扫描  //所谓左下->右上，不只是改变了模板，其遍历顺序也影响具体结果值!!!
	for (int i = bmp.BmpHeight-1; i >=0; i--)
	{
		for (int j = 0; j < bmp.BmpWidth; j++)
		{
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;  //初始化最小距离
			int locx = j;
			int	locy = i;
			for (int k = 0; k < pdisTmp->GetSize() / 2 + 1; k++)
			{

				int offx = pdisTmp->GetOffX(k);
				int offy = pdisTmp->GetOffY(k);  //offy为负值
				
				float templateDis = pdisTmp->GetDist(k);
				//计算距离矩阵的偏移
				int CX = j + offx;
				int CY = i - offy;  //偏移量和循环变量决定了模板情况
				//检验位置的合法性
				if ((CY < 0) || (CY > bmp.BmpHeight-1)) continue;
				if ((CX < 0) || (CX > bmp.BmpWidth-1))continue;
				if (fabs((fabs(DistMtx[CY][CX])- Float_MAX)) < Float_MIN)continue;//跳过无穷大的距离矩阵值的计算，加快速度；
				if (fabs(fabs(templateDis - Float_MAX)) < Float_MIN)continue;//跳过模板无穷大的部分
				if (fabs((fabs(DistMtx[CY][CX]) - Float_BARRIER)) < Float_MIN)continue;//跳过障碍计算
				//计算距离最小值
				if ((DistMtx[CY][CX] + templateDis) < dismin)
				{
					dismin = DistMtx[CY][CX] +templateDis;
					locx = CX;
					locy = CY;
				}
			}	
			DistMtx[i][j] = dismin;	//得出的最小距离赋值给模板中心位置
			LocMtx[i][j] = LocMtx[locy][locx];	//将最近的目标Loc值作为模板中心位置Loc值
		}
	}
	
	//Second 扫描
	for (int i = 0; i <bmp.BmpHeight; i++)
	{
		for (int j = bmp.BmpWidth-1; j >=0; j--)
		{
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;
			for (int k = pdisTmp->GetSize()/2; k < pdisTmp->GetSize(); k++)
			{
				int offx = pdisTmp->GetOffX(k);
				int offy = pdisTmp->GetOffY(k);  //offy为正值
				float templateDis = pdisTmp->GetDist(k);
				//计算距离矩阵的偏移
				int CX = j + offx;
				int CY = i - offy;  //注意,偏移量和循环变量决定了模板情况
				//检验位置的合法性
				if ((CY < 0) || (CY > bmp.BmpHeight - 1)) continue;
				if ((CX < 0) || (CX > bmp.BmpWidth - 1))continue;
				if (fabs((fabs(DistMtx[CY][CX]) - Float_MAX)) < Float_MIN)continue;//跳过无穷大的距离矩阵值的计算，加快速度；
				if (fabs(fabs(templateDis - Float_MAX)) < Float_MIN)continue;//跳过模板无穷大的部分
				if (fabs((fabs(DistMtx[CY][CX]) - Float_BARRIER)) < Float_MIN)continue;//跳过障碍计算

				//计算距离最小值
				if ((DistMtx[CY][CX] + templateDis) < dismin)
				{
					dismin = DistMtx[CY][CX] + templateDis;
					LocMtx[i][j] = LocMtx[CY][CX];	//将最近的目标Loc值作为模板中心位置Loc值
				}
			}
			DistMtx[i][j] = dismin;	//得出的最小距离赋值给模板中心位置

		}
	}
	
	//存储数据,输出距离矩阵，float类型，32位矩阵
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
	
	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(DistMtx[i], sizeof(unsigned int), bmp.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}
	
	
	
	//尝试输出分配矩阵，8位矩阵
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
		fwrite(LocMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);//8位图补齐4倍数
	}
	

	//扫尾清理
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
	//数据读取
	fread(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	fread(bmp.ClrTab, 1024, 1, file);
	bmp.BmpWidth = bmp.bmpInfo.biWidth;
	bmp.BmpHeight = bmp.bmpInfo.biHeight;
	bmp.MtxWidth = (bmp.BmpWidth + 3) / 4 * 4; //4个一组，不足补全
	bmp.MtxHeight = bmp.BmpHeight;
	bmp.creatBmp8bitMtx();
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //行缓存

	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			bmp.BmpMtx[i][j] = lineBuf[j];
		}
	}

	//边界矩阵
	unsigned char** boundMtx = NULL;
	boundMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		boundMtx[i] = new unsigned char[bmp.MtxWidth];
	}
	//分配场边界提取运算
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		for (int j = 0; j < bmp.MtxWidth; j++)
		{
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
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


	//创建位图
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
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++)
	{
		fwrite(boundMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//清理扫尾,类析构自动清除
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
	//数据读取
	fread(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, file);
	fread(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);

	bmp32.BmpWidth = bmp32.bmpInfo.biWidth;
	bmp32.BmpHeight = bmp32.bmpInfo.biHeight;
	bmp32.MtxWidth = bmp32.BmpWidth; 
	bmp32.MtxHeight = bmp32.BmpHeight;
	bmp32.creatBmp32bitMtx();
	unsigned int * lineBuf = new unsigned int[bmp32.MtxWidth];  //行缓存

	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fread(lineBuf, sizeof(unsigned int), bmp32.MtxWidth, file);
		for (int j = 0; j < bmp32.MtxWidth; j++)
		{
			bmp32.Bmp32Mtx[i][j] = lineBuf[j];
		}
	}

	//缓冲区矩阵
	unsigned int** bufferMtx = NULL;
	bufferMtx = new unsigned int*[bmp32.MtxHeight];
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		bufferMtx[i] = new unsigned int[bmp32.MtxWidth];
	}
	//缓冲区运算
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

	//分配场边界提取运算
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		for (int j = 0; j < bmp32.MtxWidth; j++)
		{
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
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


	//创建位图
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

	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fwrite(bufferMtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}
	//清理扫尾,类析构自动清除
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
