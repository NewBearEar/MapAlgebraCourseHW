#include<stdio.h>
#include<Windows.h>
#include"pch.h"
#include"MA.h"
#include"BitMap.h"
#include<time.h>
#include<gdal_priv.h>
#include<gdal.h>

#define Float_MAX 10000000.0
#define Float_MIN 0.000001
#define Float_BARRIER FLT_MAX

int BmpReverse(const char* InBmpName, const char* OutBmpName)   //反色函数
{
	BitMap bmp;
	FILE* file;
	errno_t err;
	err= fopen_s(&file,InBmpName, "rb");
	if (file == NULL){
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
	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			//if (lineBuf[j] == 0xFF) continue;  //255不反色，保证白色底色不变
			bmp.BmpMtx[i][j] = 255 - lineBuf[j];
		}
	}
	//创建位图
	FILE* filew;
	//errno_t err;
	//err = 
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++){
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
	if (file == NULL){
		printf("Input file is not found!");
		return -1;
	}
	//8bit数据读取
	
	fread(&(pbmp->bmpHead), sizeof(BITMAPFILEHEADER), 1, file);  //用中间变量bmpHead直接读取，最后再赋值可能更好
	fread(&(pbmp->bmpInfo), sizeof(BITMAPINFOHEADER), 1, file);
	if (pbmp->bmpInfo.biBitCount != 8){
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
	for (int i = 0; i < pbmp->MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), pbmp->MtxWidth, file);
		for (int j = 0; j < pbmp->MtxWidth; j++){
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
	for (int i = 0; i < pbmp->MtxHeight; i++){
		for (int j = 0; j < pbmp->MtxWidth; j++){
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
			unsigned int ARGBint = 0;  //int表示颜色的顺序，Alpha，R、G、B
			unsigned char Rchar, Gchar, Bchar; //因为颜色表第四个字节恒为零，所以只需要存储RGB
			Bchar = pbmp->ClrTab[pbmp->BmpMtx[i][j] * 4];   //注意颜色表字节顺序为B、G、R、A
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
	for (int i = 0; i < bmp32.MtxHeight; i++){
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
	if (file == NULL){
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
	//数据读取
	for (int i = 0; i < bmp1.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp1.MtxWidth, file);
		for (int j = 0; j < bmp1.MtxWidth; j++)
		{
			bmp1.BmpMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);

	//读取BMP2数据并保存
	err = fopen_s(&file, InBmpName2, "rb");
	if (file == NULL){
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

	for (int i = 0; i < bmp2.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp2.MtxWidth, file);
		for (int j = 0; j < bmp2.MtxWidth; j++){
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
	for (int i = 0; i < (sizeof(bmp1.ClrTab) / sizeof(unsigned char)); i++){
		bmp.ClrTab[i] = bmp1.ClrTab[i];
	}
	bmp.BmpWidth = bmp1.bmpInfo.biWidth;
	bmp.BmpHeight = bmp1.bmpInfo.biHeight;
	bmp.MtxWidth = bmp1.MtxWidth; //4个一组，不足补全
	bmp.MtxHeight = bmp1.MtxHeight;
	bmp.creatBmp8bitMtx();

	FILE* filew;
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL){
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

int Tif2Bmp(const char* InName, const char* OutName) {
	//应付硕士作业，Tif转bmp，复用代码
	GDALAllRegister();
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(InName, GA_ReadOnly);
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);

	GDALDriver* pdrv = GetGDALDriverManager()->GetDriverByName("BMP");
	const char* outFiletest = OutName;
	GDALDataset* dst = pdrv->CreateCopy(outFiletest, poDataset, 0, NULL, NULL, NULL);
	GDALClose(dst);
	return 0;
}

int Bmp2Tif(const char* InName, const char* OutName) {
	GDALAllRegister();
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(InName, GA_ReadOnly);
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);

	GDALDriver* pdrv = GetGDALDriverManager()->GetDriverByName("GTIFF");
	const char* outFiletest = OutName;
	GDALDataset* dst = pdrv->CreateCopy(outFiletest, poDataset, 0, NULL, NULL, NULL);

	GDALClose(dst);
	return 0;
}

int TifReverse(const char* InTifName, const char* OutTifName) {
	const char* tmpBmpName = "./results1/tempBmp.bmp";
	const char* tmpBmpOutName = "./results1/tempOutBmp.bmp";
	Tif2Bmp(InTifName, tmpBmpName);
	BmpReverse(tmpBmpName, tmpBmpOutName);
	Bmp2Tif(tmpBmpOutName, OutTifName);
	return 0;
}

int TifOverlay(const char* InTifName1, const char* InTifName2, const char* OutTifName)
{
	const char* tmpBmpName1 = "./results1/tempBmp1.bmp";
	const char* tmpBmpName2 = "./results1/tempBmp2.bmp";
	
	const char* tmpBmpOutName = "./results1/tempOutBmp.bmp";
	Tif2Bmp(InTifName1, tmpBmpName1);
	Tif2Bmp(InTifName2, tmpBmpName2);

	BmpOverlay(tmpBmpName1, tmpBmpName2, tmpBmpOutName);
	Bmp2Tif(tmpBmpOutName,OutTifName);
	return 0;
}

int mean33Smooth(const char* InBmpName, const char*OutBmpName)
{
	BitMap bmp,bmpmean;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL){
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
	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
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
	for (int i = 0; i < 1024; i++){
		bmpmean.ClrTab[i] = bmp.ClrTab[i];
	}
	for (int i = 0; i < bmp.BmpHeight; i++){
		for (int j = 0; j < bmp.BmpWidth; j++){
			
			float total = 0;
			smoothCount = 0;
			for (int k = 0; k < 3 ; k++){
				for (int m = 0; m < 3; m++){
					int CX = j + (m - 1);
					int CY = i + (k - 1);  //偏移量和循环变量决定了模板情况
					//检验位置的合法性
					if ((CY < 0) || (CY > bmp.BmpHeight - 1)) continue;
					if ((CX < 0) || (CX > bmp.BmpWidth - 1))continue;
					//计算距离最小值
					total += bmp.BmpMtx[CY][CX];
					smoothCount++;
				}
			}
			bmpmean.BmpMtx[i][j] = unsigned char(floor(total/smoothCount));
		}
	}

	//创建位图
	FILE* filew;
	//errno_t err;
	//err = 
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmpmean.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmpmean.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmpmean.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(bmpmean.BmpMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//清理扫尾,类析构自动清除
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;

	return 0;
}

int TifDistTrans(const char* InTifName, const char* OutDistTif, const char* OutLocTif, DistanceTemplate* pdisTmp) {
	const char* tmpBmpName1 = "./results1/tempBmp1.bmp";
	const char* tmpBmpName2 = "./results1/tempBmp2.bmp";

	const char* tmpBmpOutName1 = "./results1/tempOutBmp1.bmp";
	const char* tmpBmpOutName2 = "./results1/tempOutBmp2.bmp";
	Tif2Bmp(InTifName, tmpBmpName1);
	//Tif2Bmp(InTifName2, tmpBmpName2);

	Bmp8BitDistTrans(tmpBmpName1, tmpBmpOutName1, tmpBmpOutName2,pdisTmp);

	Bmp2Tif(tmpBmpOutName1, OutDistTif);
	Bmp2Tif(tmpBmpOutName2, OutLocTif);

	return 0;
}

int Bmp8BitDistTrans(const char*InBmpName, const char*OutDistBmp, const char*OutLocBmp, DistanceTemplate* pdisTmp)
{
	BitMap bmp,bmp2,bmploc;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL){
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

	bmp2.bmpHead = bmp.bmpHead;
	bmp2.bmpInfo = bmp.bmpInfo;
	for (int i = 0; i < 1024; i++){
		bmp2.ClrTab[i] = bmp.ClrTab[i];
	}

	//1.分配场和距离场空间的建立与分配
	unsigned char** LocMtx = NULL;
	LocMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++){
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
	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			bmp.BmpMtx[i][j] = lineBuf[j];
			LocMtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);
	unsigned char tmploc = 255;
	float tmpdist = 0;
	//3.根据分配场初始化距离场矩阵
	for (int i = 0; i < bmp.MtxHeight; i++){
		for (int j = 0; j < bmp.MtxWidth; j++){
			if (LocMtx[i][j] == 0xff) DistMtx[i][j] = Float_MAX;
			//加入障碍部分，为黑色0x00 和 0x01
			else if (LocMtx[i][j] == 0x00 || LocMtx[i][j] == 0x01) DistMtx[i][j] = Float_BARRIER;
			else{
				DistMtx[i][j] = 0;  //非加权赋值
				//加权赋值
				/*
				if (tmploc != LocMtx[i][j]) {
					tmploc = LocMtx[i][j];
					//srand((unsigned)time(NULL));
					//tmpdist = float(rand() % 20);//加权扩展，100以内随机赋权值
					tmpdist = tmpdist+1;  //每个目标权重加2
				}
				DistMtx[i][j] = tmpdist;
				*/
			}
		}
	}
	//4.根据模板扫描计算
	
	//First 左下->右上扫描  //所谓左下->右上，不只是改变了模板，其遍历顺序也影响具体结果值!!!
	for (int i = bmp.BmpHeight-1; i >=0; i--){
		for (int j = 0; j < bmp.BmpWidth; j++){
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;  //初始化最小距离
			int locx = j;
			int	locy = i;
			for (int k = 0; k < pdisTmp->GetSize() / 2 + 1; k++){

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
				if ((DistMtx[CY][CX] + templateDis) < dismin){
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
	for (int i = 0; i <bmp.BmpHeight; i++){
		for (int j = bmp.BmpWidth-1; j >=0; j--){
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;
			for (int k = pdisTmp->GetSize()/2; k < pdisTmp->GetSize(); k++){
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
				if ((DistMtx[CY][CX] + templateDis) < dismin){
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
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	BitMap bmp32;
	bmp32.bmpHead = bmp.bmpHead;
	bmp32.bmpInfo = bmp.bmpInfo;
	bmp32.bmpHead.bfOffBits = 54;
	bmp32.bmpHead.bfSize = sizeof(float)*(bmp.BmpHeight*bmp.BmpWidth) + 54;
	bmp32.bmpInfo.biBitCount = 32;
	fwrite(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	
	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(DistMtx[i], sizeof(float), bmp.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}
	fclose(filew);
	
	
	//尝试输出分配矩阵，8位矩阵
	filew;
	fopen_s(&filew, OutLocBmp, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}

	fwrite(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(&(bmp2.ClrTab), sizeof(unsigned int), 256, filew);
	
	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(LocMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);//8位图补齐4倍数
	}
	

	//扫尾清理
	fclose(filew);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp.BmpHeight; i++){
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

int getTifBoundary(const char* InTifName, const char* OutTifName) {
	const char* tmpBmpName = "./results1/tempBmp.bmp";
	const char* tmpBmpOutName = "./results1/tempOutBmp.bmp";
	Tif2Bmp(InTifName, tmpBmpName);
	getVoronoiBoundary(tmpBmpName, tmpBmpOutName);
	Bmp2Tif(tmpBmpOutName, OutTifName);
	return 0;
}

int getVoronoiBoundary(const char * InBmpName, const char * OutBmpName)
{
	BitMap bmp;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL){
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

	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			bmp.BmpMtx[i][j] = lineBuf[j];
		}
	}

	//边界矩阵
	unsigned char** boundMtx = NULL;
	boundMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++){
		boundMtx[i] = new unsigned char[bmp.MtxWidth];
	}
	//分配场边界提取运算
	for (int i = 0; i < bmp.MtxHeight; i++){
		for (int j = 0; j < bmp.MtxWidth; j++){
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
			}else{
				boundMtx[i][j] = bmp.BmpMtx[i][j];
			}

		}
	}


	//创建位图
	FILE* filew;
	//errno_t err;
	//err = 
	err=fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(boundMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}
	//清理扫尾,类析构自动清除
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp.BmpHeight; i++){
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
	if (file == NULL){
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
	//注意分配场为float
	float * lineBuf = new float[bmp32.MtxWidth];  //行缓存

	for (int i = 0; i < bmp32.MtxHeight; i++){
		fread(lineBuf, sizeof(float), bmp32.MtxWidth, file);
		for (int j = 0; j < bmp32.MtxWidth; j++){
			bmp32.Bmp32Mtx[i][j] = lineBuf[j];
		}
	}

	//缓冲区矩阵
	unsigned int** bufferMtx = NULL;
	unsigned int** tmpBufferMtx = NULL;
	bufferMtx = new unsigned int*[bmp32.MtxHeight];
	tmpBufferMtx = new unsigned int*[bmp32.MtxHeight];
	for (int i = 0; i < bmp32.MtxHeight; i++){
		bufferMtx[i] = new unsigned int[bmp32.MtxWidth];
		tmpBufferMtx[i] = new unsigned int[bmp32.MtxWidth];
	}
	//缓冲区运算
	for (int i = 0; i < bmp32.MtxHeight; i++){
		for (int j = 0; j < bmp32.MtxWidth; j++){
			if (bmp32.Bmp32Mtx[i][j] < bufferWidth){
				/*if (bmp32.Bmp32Mtx[i][j] != 0)
					cout << "?";   *///????????
				bufferMtx[i][j] = 0;
			}else{
				bufferMtx[i][j] = 0xffffffff;
			}
		}
	}

	//分配场边界提取运算
	for (int i = 0; i < bmp32.MtxHeight; i++){
		for (int j = 0; j < bmp32.MtxWidth; j++){
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
			if ((CYT < 0) || (CYD > bmp32.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp32.BmpWidth - 1))continue;
			//if (bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYT][j] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYD][j]
			//	&& bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXR] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXL])
			if (bufferMtx[i][j] == bufferMtx[CYT][j] && bufferMtx[i][j] == bufferMtx[CYD][j]
				&& bufferMtx[i][j] == bufferMtx[i][CXR] && bufferMtx[i][j] == bufferMtx[i][CXL])
			{
				tmpBufferMtx[i][j] = 0xffffffff;
			}else{
				tmpBufferMtx[i][j] = bufferMtx[i][j];
			}

		}
	}


	//创建位图
	FILE* filew;
	fopen_s(&filew, OutBmpName, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	bmp32.bmpHead.bfOffBits = 54;
	bmp32.bmpHead.bfSize = sizeof(unsigned int)*(bmp32.BmpHeight*bmp32.BmpWidth) + 54;
	bmp32.bmpInfo.biBitCount = 32;
	fwrite(&(bmp32.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp32.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);

	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp32.MtxHeight; i++){
		fwrite(tmpBufferMtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}
	//清理扫尾,类析构自动清除
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp32.BmpHeight; i++){
		delete[] bufferMtx[i];
		bufferMtx[i] = NULL;
	}
	return 0;

}

int getDelaunay(const char * InBmpName, const char*OutLocBmp, const char*OutVoronoiBmp, const char * OutDelaunayBmp, DistanceTemplate * pdisTmp)
{
	BitMap bmp, bmp2, bmploc;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL){
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

	bmp2.bmpHead = bmp.bmpHead;
	bmp2.bmpInfo = bmp.bmpInfo;
	for (int i = 0; i < 1024; i++){
		bmp2.ClrTab[i] = bmp.ClrTab[i];
	}
	bmp2.BmpWidth = bmp.BmpWidth;
	bmp2.BmpHeight = bmp.BmpHeight;
	bmp2.MtxWidth = bmp.MtxWidth; //4个一组，不足补全
	bmp2.MtxHeight = bmp.MtxHeight;
	bmp2.creatBmp8bitMtx();
	//1.分配场和距离场空间的建立与分配
	unsigned char** LocMtx = NULL;
	LocMtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++){
		LocMtx[i] = new unsigned char[bmp.MtxWidth];
	}
	float** DistMtx = NULL;
	DistMtx = new float*[bmp.MtxHeight];
	for (int i = 0; i < bmp.MtxHeight; i++){
		DistMtx[i] = new float[bmp.MtxWidth];
	}
	vector<MyPoint> sourcePoint; //记录所有点
	vector<pair<MyPoint, MyPoint>> DelaunayVertexs;  //记录三角网顶点对
	MyPoint tmpPt1, tmpPt2;
	//2.读取位图数据并复制到分配场
	unsigned char* lineBuf = new unsigned char[bmp.MtxWidth];  //行缓存
	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(lineBuf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			bmp.BmpMtx[i][j] = lineBuf[j];
			LocMtx[i][j] = lineBuf[j];
			if (lineBuf[j] != 255){
				tmpPt1.x = j;
				tmpPt1.y = i;
				tmpPt1.loc = lineBuf[j];
				//加入判断，面数据只取一个点
				int flag = 0;
				for (vector<MyPoint>::iterator iter = sourcePoint.begin(); iter != sourcePoint.end(); iter++) {
					if (tmpPt1.loc == iter->loc) {
						flag = 1;
						break;
					}
				}
				if (flag == 0 && tmpPt1.loc != 0x00) {
					sourcePoint.push_back(tmpPt1);
				}
				
			}
		}
	}
	fclose(file);

	//3.根据分配场初始化距离场矩阵
	for (int i = 0; i < bmp.MtxHeight; i++){
		for (int j = 0; j < bmp.MtxWidth; j++){
			if (LocMtx[i][j] == 0xff) DistMtx[i][j] = Float_MAX;
			//加入障碍部分，为黑色
			else if (LocMtx[i][j] == 0x00) DistMtx[i][j] = Float_BARRIER;
			else{
				DistMtx[i][j] = 0;
			}
		}
	}
	//4.根据模板扫描计算

	//First 左下->右上扫描  //所谓左下->右上，不只是改变了模板，其遍历顺序也影响具体结果值!!!
	for (int i = bmp.BmpHeight - 1; i >= 0; i--){
		for (int j = 0; j < bmp.BmpWidth; j++){
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;  //初始化最小距离
			int locx = j;
			int	locy = i;
			for (int k = 0; k < pdisTmp->GetSize() / 2 + 1; k++){

				int offx = pdisTmp->GetOffX(k);
				int offy = pdisTmp->GetOffY(k);  //offy为负值

				float templateDis = pdisTmp->GetDist(k);
				//计算距离矩阵的偏移
				int CX = j + offx;
				int CY = i - offy;  //偏移量和循环变量决定了模板情况
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
					locx = CX;
					locy = CY;
				}
			}
			DistMtx[i][j] = dismin;	//得出的最小距离赋值给模板中心位置
			LocMtx[i][j] = LocMtx[locy][locx];	//将最近的目标Loc值作为模板中心位置Loc值
		}
	}

	//Second 扫描
	for (int i = 0; i < bmp.BmpHeight; i++){
		for (int j = bmp.BmpWidth - 1; j >= 0; j--){
			if (fabs(DistMtx[i][j]) < Float_MIN) continue;
			//加入障碍部分，为黑色
			if (fabs(fabs(DistMtx[i][j] - Float_BARRIER)) < Float_MIN) continue;
			float dismin = Float_MAX;
			for (int k = pdisTmp->GetSize() / 2; k < pdisTmp->GetSize(); k++)
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
				if ((DistMtx[CY][CX] + templateDis) < dismin){
					dismin = DistMtx[CY][CX] + templateDis;
					LocMtx[i][j] = LocMtx[CY][CX];	//将最近的目标Loc值作为模板中心位置Loc值
					bmp2.BmpMtx[i][j] = LocMtx[i][j];
				}
			}
			DistMtx[i][j] = dismin;	//得出的最小距离赋值给模板中心位置

		}
	}

	//尝试输出分配矩阵，8位矩阵
	FILE * filew;
	fopen_s(&filew, OutLocBmp, "wb");
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}

	fwrite(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(&(bmp2.ClrTab), sizeof(unsigned int), 256, filew);

	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(LocMtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);//8位图补齐4倍数
	}
	fclose(filew);
	cout << "分配场输出完成"<<endl;
	//**********************************************/
	//边界矩阵
	unsigned char** boundMtx = NULL;
	boundMtx = new unsigned char*[bmp2.MtxHeight];
	for (int i = 0; i < bmp2.MtxHeight; i++){
		boundMtx[i] = new unsigned char[bmp2.MtxWidth];
	}
	//分配场边界提取运算
	for (int i = 0; i < bmp2.MtxHeight; i++){
		for (int j = 0; j < bmp2.MtxWidth; j++){
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
			if ((CYT < 0) || (CYD > bmp2.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp2.BmpWidth - 1))continue;
			if (bmp2.BmpMtx[i][j] == bmp2.BmpMtx[CYT][j] && bmp2.BmpMtx[i][j] == bmp2.BmpMtx[CYD][j]
				&& bmp2.BmpMtx[i][j] == bmp2.BmpMtx[i][CXR] && bmp2.BmpMtx[i][j] == bmp2.BmpMtx[i][CXL])
			{
				boundMtx[i][j] = 0xff;
			}else{
				boundMtx[i][j] = bmp2.BmpMtx[i][j];
				//记录顶点对
				if (bmp2.BmpMtx[i][j] != bmp2.BmpMtx[CYT][j]){
					vector<MyPoint>::iterator iter = sourcePoint.begin();
					for (; iter != sourcePoint.end(); iter++){
						if (bmp2.BmpMtx[i][j] == iter->loc){
							tmpPt1 = *iter;
						}
						if (bmp2.BmpMtx[CYT][j] == iter->loc){
							tmpPt2 = *iter;
						}
					}
					DelaunayVertexs.push_back(pair<MyPoint, MyPoint>(tmpPt1, tmpPt2));
				}else if (bmp2.BmpMtx[i][j] != bmp2.BmpMtx[CYD][j]){
					vector<MyPoint>::iterator iter = sourcePoint.begin();
					for (; iter != sourcePoint.end(); iter++){
						if (bmp2.BmpMtx[i][j] == iter->loc){
							tmpPt1 = *iter;
						}
						if (bmp2.BmpMtx[CYD][j] == iter->loc){
							tmpPt2 = *iter;
						}
					}
					DelaunayVertexs.push_back(pair<MyPoint, MyPoint>(tmpPt1, tmpPt2));
				}else if (bmp2.BmpMtx[i][j] != bmp2.BmpMtx[i][CXR]){
					vector<MyPoint>::iterator iter = sourcePoint.begin();
					for (; iter != sourcePoint.end(); iter++){
						if (bmp2.BmpMtx[i][j] == iter->loc){
							tmpPt1 = *iter;
						}
						if (bmp2.BmpMtx[i][CXR] == iter->loc){
							tmpPt2 = *iter;
						}
					}
					DelaunayVertexs.push_back(pair<MyPoint, MyPoint>(tmpPt1, tmpPt2));
				}else if(bmp2.BmpMtx[i][j] != bmp2.BmpMtx[i][CXL]){
					vector<MyPoint>::iterator iter = sourcePoint.begin();
					for (; iter != sourcePoint.end(); iter++){
						if (bmp2.BmpMtx[i][j] == iter->loc){
							tmpPt1 = *iter;
						}
						if (bmp2.BmpMtx[i][CXL] == iter->loc){
							tmpPt2 = *iter;
						}
					}
					DelaunayVertexs.push_back(pair<MyPoint, MyPoint>(tmpPt1, tmpPt2));
				}
			}

		}
	}


	//创建位图
	filew;
	//errno_t err;
	//err = 
	err = fopen_s(&filew, OutVoronoiBmp, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp2.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp2.MtxHeight; i++)
	{
		fwrite(boundMtx[i], sizeof(unsigned char), bmp2.MtxWidth, filew);
	}
	//扫尾清理
	fclose(filew);
	cout << "边界输出完成" << endl;
	//*******************************************/

	//绘制三角网
	vector<pair<MyPoint, MyPoint>>::iterator iter = DelaunayVertexs.begin();
	for (; iter != DelaunayVertexs.end(); iter++){
		DDALine dda(&(iter->first), &(iter->second));
		dda.draw(boundMtx);
	}

	//创建位图
	filew;
	//errno_t err;
	//err = 
	err = fopen_s(&filew, OutDelaunayBmp, "wb");
	if (filew == NULL)
	{
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp2.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp2.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp2.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp2.MtxHeight; i++)
	{
		fwrite(boundMtx[i], sizeof(unsigned char), bmp2.MtxWidth, filew);
	}

	//扫尾清理
	fclose(filew);
	cout << "三角网输出完成" << endl;
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

int getTifDelaunay(const char* InTifName, const char* OutLocTif, const char* OutVoronoiTif, const char* OutDelaunayTif, DistanceTemplate* pdisTmp) {
	const char* tmpBmpInName = "./results1/tempInBmp.bmp";
	const char* tmpBmpLocName = "./results1/tempLocBmp.bmp";
	const char* tmpBmpVorName = "./results1/tempVorBmp.bmp";
	const char* tmpBmpDenName = "./results1/tempDenBmp.bmp";
	Tif2Bmp(InTifName, tmpBmpInName);

	getDelaunay(tmpBmpInName, tmpBmpLocName, tmpBmpVorName, tmpBmpDenName, pdisTmp);
	
	Bmp2Tif(tmpBmpLocName, OutLocTif);
	Bmp2Tif(tmpBmpVorName, OutVoronoiTif);
	Bmp2Tif(tmpBmpDenName, OutDelaunayTif);
	return 0;
}

int getBufferSetBlack(float bufferWidth, const char * InBmpName, const char * InLocBmpName, const char * OutBmpName)
{
	//获取缓冲区并将外部设置为黑色

	//缓冲区不输出32位图
	BitMap bmp32,bmp,tmp8Bmp;
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
	float * lineBuf = new float[bmp32.MtxWidth];  //行缓存

	for (int i = 0; i < bmp32.MtxHeight; i++){
		fread(lineBuf, sizeof(float), bmp32.MtxWidth, file);
		for (int j = 0; j < bmp32.MtxWidth; j++){
			bmp32.Bmp32Mtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);
	file;
	err;
	err = fopen_s(&file, InLocBmpName, "rb");
	if (file == NULL){
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
	unsigned char * line8Buf = new unsigned char[bmp.MtxWidth];  //行缓存

	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(line8Buf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			bmp.BmpMtx[i][j] = line8Buf[j];
		}
	}



	//缓冲区矩阵
	unsigned int** tmpBufferMtx = NULL;
	tmpBufferMtx = new unsigned int*[bmp32.MtxHeight];

	unsigned int** bufferMtx = NULL;
	unsigned char** buffer8Mtx = NULL;
	unsigned char** tmpBuffer8Mtx = NULL;
	bufferMtx = new unsigned int*[bmp32.MtxHeight];
	buffer8Mtx = new unsigned char*[bmp.MtxHeight];
	tmpBuffer8Mtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp32.MtxHeight; i++){
		bufferMtx[i] = new unsigned int[bmp32.MtxWidth];
		tmpBufferMtx[i] = new unsigned int[bmp32.MtxWidth];
	}
	for (int i = 0; i < bmp.MtxHeight; i++){
		buffer8Mtx[i] = new unsigned char[bmp.MtxWidth];
		tmpBuffer8Mtx[i] = new unsigned char[bmp.MtxWidth];
	}
	//缓冲区运算
	for (int i = 0; i < bmp32.MtxHeight; i++){
		for (int j = 0; j < bmp32.MtxWidth; j++){
			if (bmp32.Bmp32Mtx[i][j] < bufferWidth){
				bufferMtx[i][j] = 0xfff;
				buffer8Mtx[i][j] = bmp.BmpMtx[i][j];
			}else{
				bufferMtx[i][j] = 0xffffffff;
				buffer8Mtx[i][j] = 0x00;  //黑色
			}
		}
	}

	//分配场边界提取运算
	for (int i = 0; i < bmp32.MtxHeight; i++){
		for (int j = 0; j < bmp32.MtxWidth; j++){
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
			if ((CYT < 0) || (CYD > bmp32.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp32.BmpWidth - 1))continue;
			//这里if该用bufferMtx，但鉴于没有输出，暂不改了
			if (bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYT][j] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[CYD][j]
				&& bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXR] && bmp32.Bmp32Mtx[i][j] == bmp32.Bmp32Mtx[i][CXL])
			{
				tmpBufferMtx[i][j] = 0xffffffff;
			}else{
				tmpBufferMtx[i][j] = bufferMtx[i][j];
			}

		}
	}
	for (int i = 0; i < bmp.MtxHeight; i++){
		for (int j = 0; j < bmp.MtxWidth; j++){
			//四个位置
			int CXL = j - 1;  //左边
			int CXR = j + 1;  //右
			int CYD = i + 1;  //下
			int CYT = i - 1;  //上
			//检验位置的合法性
			if ((CYT < 0) || (CYD > bmp.BmpHeight - 1)) continue;
			if ((CXL < 0) || (CXR > bmp.BmpWidth - 1))continue;
			if (buffer8Mtx[i][j] == buffer8Mtx[CYT][j] && buffer8Mtx[i][j] == buffer8Mtx[CYD][j]
				&& buffer8Mtx[i][j] == buffer8Mtx[i][CXR] && buffer8Mtx[i][j] == buffer8Mtx[i][CXL])
			{
				if (buffer8Mtx[i][j] != 0x00)
					tmpBuffer8Mtx[i][j] = 0xff;
				else
					tmpBuffer8Mtx[i][j] = 0x00;
			}else{
				tmpBuffer8Mtx[i][j] = buffer8Mtx[i][j];
			}

		}
	}

	//创建位图
	FILE* //创建位图
		filew;
	//errno_t err;
	//err = 
	err = fopen_s(&filew, OutBmpName, "wb");
	
	if (filew == NULL){
		printf("Output file is not found!");
		return -1;
	}
	fwrite(&(bmp.bmpHead), sizeof(BITMAPFILEHEADER), 1, filew);
	fwrite(&(bmp.bmpInfo), sizeof(BITMAPINFOHEADER), 1, filew);
	fwrite(bmp.ClrTab, 1024, 1, filew);
	//写入位图
	for (int i = 0; i < bmp.MtxHeight; i++){
		fwrite(tmpBuffer8Mtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);  //
	}

	/*
	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fwrite(bufferMtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}*/

	//清理扫尾,类析构自动清除
	fclose(filew);
	fclose(file);
	delete[] lineBuf;
	lineBuf = NULL;
	for (int i = 0; i < bmp32.BmpHeight; i++){
		delete[] bufferMtx[i];
		bufferMtx[i] = NULL;
	}
	return 0;

}

int adhesionTrans(float bufferWidth,const char*InBmpName, const char*OutsideDistBmp, const char*OutsideLocBmp, const char*InsideDistBmp, const char*InsideLocBmp, const char * OutsideBmp, const char * InsideBmp, DistanceTemplate* pdisTmp)
{
	//加壳变换
	Bmp8BitDistTrans(InBmpName, OutsideDistBmp, OutsideLocBmp, pdisTmp);  //外距变换
	//获取外距变换缓冲区
	getBufferSetBlack(bufferWidth, OutsideDistBmp, OutsideLocBmp, OutsideBmp);
	//蜕皮变换
	Bmp8BitDistTrans(OutsideBmp, InsideDistBmp, InsideLocBmp, pdisTmp);  //内距变换
	//获取内距变换缓冲区
	getBufferSetWhite(bufferWidth, InsideDistBmp, InsideLocBmp, InsideBmp);  //最终结果
	return 0;
}


int getBufferSetWhite(float bufferWidth, const char * InBmpName, const char * InLocBmpName, const char * OutBmpName)
{
	//获取缓冲区并将内部设置为白色
	BitMap bmp32, bmp, tmp8Bmp;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, InBmpName, "rb");
	if (file == NULL){
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
	float * lineBuf = new float[bmp32.MtxWidth];  //行缓存

	for (int i = 0; i < bmp32.MtxHeight; i++){
		fread(lineBuf, sizeof(float), bmp32.MtxWidth, file);
		for (int j = 0; j < bmp32.MtxWidth; j++){
			bmp32.Bmp32Mtx[i][j] = lineBuf[j];
		}
	}
	fclose(file);
	file;
	err;
	err = fopen_s(&file, InLocBmpName, "rb");
	if (file == NULL){
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
	unsigned char * line8Buf = new unsigned char[bmp.MtxWidth];  //行缓存

	for (int i = 0; i < bmp.MtxHeight; i++){
		fread(line8Buf, sizeof(unsigned char), bmp.MtxWidth, file);
		for (int j = 0; j < bmp.MtxWidth; j++){
			bmp.BmpMtx[i][j] = line8Buf[j];
		}
	}



	//缓冲区矩阵
	unsigned int** tmpBufferMtx = NULL;
	tmpBufferMtx = new unsigned int*[bmp32.MtxHeight];
	unsigned int** bufferMtx = NULL;
	unsigned char** buffer8Mtx = NULL;
	unsigned char** tmpBuffer8Mtx = NULL;
	bufferMtx = new unsigned int*[bmp32.MtxHeight];
	buffer8Mtx = new unsigned char*[bmp.MtxHeight];
	tmpBuffer8Mtx = new unsigned char*[bmp.MtxHeight];
	for (int i = 0; i < bmp32.MtxHeight; i++){
		bufferMtx[i] = new unsigned int[bmp32.MtxWidth];
		tmpBufferMtx[i] = new unsigned int[bmp32.MtxWidth];
	}
	for (int i = 0; i < bmp.MtxHeight; i++){
		buffer8Mtx[i] = new unsigned char[bmp.MtxWidth];
		tmpBuffer8Mtx[i] = new unsigned char[bmp.MtxWidth];
	}
	//缓冲区运算
	for (int i = 0; i < bmp32.MtxHeight; i++){
		for (int j = 0; j < bmp32.MtxWidth; j++){
			if (bmp32.Bmp32Mtx[i][j] < bufferWidth){
				bufferMtx[i][j] = 0xfff;
				buffer8Mtx[i][j] = 0xff;
			}else{
				bufferMtx[i][j] = 0xffffffff;
				if (bmp.BmpMtx[i][j] == 0x00)
					buffer8Mtx[i][j] = 0xff;  //白色
				else
					buffer8Mtx[i][j] = bmp.BmpMtx[i][j];
			}
		}
	}
	/*
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
			//这里if该用bufferMtx，但鉴于没有输出，暂不改了
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
			if (buffer8Mtx[i][j] == buffer8Mtx[CYT][j] && buffer8Mtx[i][j] == buffer8Mtx[CYD][j]
				&& buffer8Mtx[i][j] == buffer8Mtx[i][CXR] && buffer8Mtx[i][j] == buffer8Mtx[i][CXL])
			{
				if (buffer8Mtx[i][j] != 0x00)
					tmpBuffer8Mtx[i][j] = 0xff;
				else
					tmpBuffer8Mtx[i][j] = 0x00;
			}
			else
			{
				tmpBuffer8Mtx[i][j] = buffer8Mtx[i][j];
			}

		}
	}
	*/
	//创建位图
	FILE* //创建位图
		filew;
	//errno_t err;
	//err = 
	err = fopen_s(&filew, OutBmpName, "wb");
	
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
		fwrite(buffer8Mtx[i], sizeof(unsigned char), bmp.MtxWidth, filew);
	}

	/*
	//用距离矩阵写入位图,float和int位数相同时
	for (int i = 0; i < bmp32.MtxHeight; i++)
	{
		fwrite(bufferMtx[i], sizeof(unsigned int), bmp32.BmpWidth, filew);//不必补齐,int已经是4的倍数
	}*/

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
