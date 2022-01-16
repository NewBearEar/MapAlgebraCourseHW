// Map_Algebra_BitMap.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
#include"MA.h"
#include <gdal_priv.h>
#include<gdal.h>

using namespace std;
//反色，叠置，均值平滑,8位图转32位,8位图距离变换（带障碍）,v图边界,缓冲区，内距变换(中轴),粘连变换,Delaunay三角网
typedef enum
{
	BMPREVERSE,BMPOVERLAY,MEANSMOOTH,BMP8BIT_TO_32BIT,BMP8BIT_DISTTRANS,VORONOIBOUND, BUFFER,AXIS, ADHESION_TRANS,DELAUNAY,
	TIFFREVERSE,TIFFOVERLAY,TIFFBOUND,TIFF_DISTTRANS
}MA_ALGORITHM;
int main()
{
    cout << "Hello World!\n"; 	

	/*
	const char* pszFile;
	pszFile = "./data/gym_raster.tif";
	const char* pszOut = "./data/test3.bmp";
	Tif2Bmp(pszFile, pszOut);
	*/
	/*
	const char* pszFile;
	GDALAllRegister();
	pszFile = "./data/test1.bmp";
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(pszFile, GA_ReadOnly);
	GDALRasterBand* poBand = poDataset->GetRasterBand(1);

	GDALDriver* pdrv = GetGDALDriverManager()->GetDriverByName("BMP");
	const char* outFiletest = "./data/test1.bmp";
	GDALDataset* dst =  pdrv->CreateCopy(outFiletest, poDataset, 0, NULL, NULL, NULL);
	GDALClose(dst);

	double trans[6];
	CPLErr aaa = poDataset->GetGeoTransform(trans);
	
	int xsize = poBand->GetXSize();
	int ysize = poBand->GetYSize();

	cout << aaa << endl;
	cout << xsize << endl;
	cout << ysize << endl;
	*/


	//选择算法
	cout << "请选择算法：" << endl;
	cout << "0-反色运算" << endl << "1-叠置运算" << endl << "2-均值平滑" << endl << "3-8位转32位" << endl;
	cout << "4-8位图距离变换" << endl << "5-V图边界提取" << endl << "6-缓冲区" << endl<<"7-中轴提取"<<endl;
	cout << "8-粘连变换" << endl<< "9-delaunay三角网提取" << endl;
	cout << "10-TIFF反色运算" << endl;
	cout << "11-TIFF叠置运算" << endl;
	cout << "12-TIFF边界提取" << endl;
	cout << "13-TIFF距离变换" << endl;
	
	char * InputBmpName1 = new char[1024];
	char * InputBmpName2 = new char[1024];
	char * OutputBmpName1 = new char[1024];
	char * OutputBmpName2 = new char[1024];
	char * OutputBmpName3 = new char[1024];
	char * OutputBmpName4 = new char[1024];
	char * OutputBmpName5 = new char[1024];
	char * OutputBmpName6 = new char[1024];
	//测试障碍距离变换
	DistanceTemplate* distemp = new Dist5Tmp();  //5×5模板
		//DistanceTemplate* distemp = new DistOctTmp();//八边形模板
	//DistanceTemplate* distemp = new Dist13Tmp();//13×13模板
	float bufferWidth = 0; //缓冲区宽度
	int algorChoose = -1;
	cin >> algorChoose; //输入
	switch (algorChoose)
	{
	case BMPREVERSE:
		cout << "源文件的路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		BmpReverse(InputBmpName1, OutputBmpName1); //调用反色函数
		break;
	case TIFFREVERSE:
		// 输入输出tiff
		cout << "源文件的路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		TifReverse(InputBmpName1, OutputBmpName1); //调用反色函数
		break;
	case BMPOVERLAY:
		cout << "待叠加的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "叠加的文件路径：" << endl;
		cin >> InputBmpName2;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		BmpOverlay(InputBmpName1, InputBmpName2, OutputBmpName1); //调用叠加函数
		break;
	case TIFFOVERLAY:
		//变量名没改，但是要用TIFF
		cout << "待叠加的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "叠加的文件路径：" << endl;
		cin >> InputBmpName2;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		TifOverlay(InputBmpName1, InputBmpName2, OutputBmpName1); //调用叠加函数
		break;
	case MEANSMOOTH:
		cout << "待平滑的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		mean33Smooth(InputBmpName1, OutputBmpName1); //调用平滑
		break;
	case BMP8BIT_TO_32BIT:
		cout << "待转换的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		Bmp8bitTo32bit(InputBmpName1, OutputBmpName1); //调用8位转32位
		break;
	case BMP8BIT_DISTTRANS:
		
		cout << "待距离变换的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出距离场文件的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "输出分配场文件的路径：" << endl;
		cin >> OutputBmpName2;
		Bmp8BitDistTrans(InputBmpName1, OutputBmpName1, OutputBmpName2, distemp); //调用距离变换
		break;
	case TIFF_DISTTRANS:

		cout << "待距离变换的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出距离场文件的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "输出分配场文件的路径：" << endl;
		cin >> OutputBmpName2;
		TifDistTrans(InputBmpName1, OutputBmpName1, OutputBmpName2, distemp); //调用距离变换
		break;
	case VORONOIBOUND:
		cout << "分配场的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		getVoronoiBoundary(InputBmpName1, OutputBmpName1); //获取v图
		break;
	case TIFFBOUND:
		cout << "输入的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		getTifBoundary(InputBmpName1, OutputBmpName1); //获取边界
		break;
		
	case BUFFER:
		cout << "距离场的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出文件的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "缓冲区宽度：" << endl;
		cin >> bufferWidth;
		getBufferFromDis(bufferWidth, InputBmpName1, OutputBmpName1);//缓冲区
		break;
	case AXIS:
		cout << "待提取中轴的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出距离场文件的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "输出分配场文件的路径：" << endl;
		cin >> OutputBmpName2;
		cout << "输出中轴的路径：" << endl;
		cin >> OutputBmpName3;
		Bmp8BitDistTrans(InputBmpName1, OutputBmpName1, OutputBmpName2, distemp); //调用距离变换
		getVoronoiBoundary(OutputBmpName2, OutputBmpName3); //获取v图
		break;
	case DELAUNAY:
		cout << "待提取三角网的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出分配场的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "输出V图的路径：" << endl;
		cin >> OutputBmpName2;
		cout << "输出Delaunay的路径：" << endl;
		cin >> OutputBmpName3;
		getDelaunay(InputBmpName1, OutputBmpName1,OutputBmpName2,OutputBmpName3, distemp);  
		break;
	case ADHESION_TRANS:
		cout << "待粘连变换的文件路径：" << endl;
		cin >> InputBmpName1;
		cout << "输出外距变换距离场的路径：" << endl;
		cin >> OutputBmpName1;
		cout << "输出外距变换分配场的路径：" << endl;
		cin >> OutputBmpName2;
		cout << "输出内距变换距离场的路径：" << endl;
		cin >> OutputBmpName3;
		cout << "输出内距变换分配场的路径：" << endl;
		cin >> OutputBmpName4;
		cout << "输出外距变换的路径：" << endl;
		cin >> OutputBmpName5;
		cout << "输出内距变换（最终结果）的路径：" << endl;
		cin >> OutputBmpName6;
		cout << "缓冲区宽度：" << endl;
		cin >> bufferWidth;
		adhesionTrans(bufferWidth,InputBmpName1, OutputBmpName1, OutputBmpName2, OutputBmpName3, OutputBmpName4, OutputBmpName5, OutputBmpName6, distemp);
		break;
	default:
		break;
	}
	cout << "完成！" << endl;

	/*
	stringstream sstr;  //字符串流，可用于char*到string ，也可用string的data（）方法
	sstr.clear();
	//输入
	char * InputBmpName = new char[1024];
	string sInputBmpName = "E:\\学习\\地图代数实验1\\8bitTest.bmp";
	sstr << sInputBmpName;
	sstr >> InputBmpName;

	char * InputBmpName2 = new char[1024];
	string sInputBmpName2 = "E:\\学习\\地图代数实验1\\test.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sInputBmpName2;
	sstr >> InputBmpName2;

	char * InputBmpName3 = new char[1024];
	string sInputBmpName3 = "E:\\学习\\地图代数实验3\\distTranBarrier3.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sInputBmpName3;
	sstr >> InputBmpName3;

	char * InputBmpName4 = new char[1024];
	string sInputBmpName4 = "E:\\学习\\地图代数实验3\\distTransOctangonDistribute.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sInputBmpName4;
	sstr >> InputBmpName4;


	char * InputBmpName5 = new char[1024];
	string sInputBmpName5 = "E:\\学习\\地图代数实验1\\write8bitDistTransHalf.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sInputBmpName5;
	sstr >> InputBmpName5;
	*/
	
	//输出反色
	/*
	char * OutputBmpName = new char[1024];
	string sOutputBmpName = "E:\\学习\\地图代数实验1\\write1.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName;
	sstr >> OutputBmpName;
	//BmpReverse(InputBmpName, OutputBmpName); //调用反色函数
	*/

	/*
	//输出32bit
	char * OutputBmpName2 = new char[1024];
	string sOutputBmpName2 = "E:\\学习\\地图代数实验1\\write32bitCharTest.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName2;
	sstr >> OutputBmpName2;
	Bmp8bitTo32bit(InputBmpName, OutputBmpName2);
	*/

	/*
	//输出叠加结果
	char * OutputBmpName3 = new char[1024];
	string sOutputBmpName3 = "E:\\学习\\地图代数实验1\\writeOverlay(255).bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName3;
	sstr >> OutputBmpName3;
	BmpOverlay(InputBmpName,InputBmpName2, OutputBmpName3); //调用叠加函数
	*/

	/*
	//测试障碍距离变换
	DistanceTemplate* distemp = new Dist5Tmp();  //5×5模板
	//DistanceTemplate* distemp = new DistOctTmp();//八边形模板
	char * OutputBmpName4 = new char[1024];
	string sOutputBmpName4 = "E:\\学习\\地图代数实验3\\distTranBarrierDistMtxDest3.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName4;
	sstr >> OutputBmpName4;
	Bmp8BitDistTrans(InputBmpName3, OutputBmpName4,OutputBmpName4,distemp); //调用距离变换
	*/
	/*
	char * OutputBmpName5 = new char[1024];
	string sOutputBmpName5 = "E:\\学习\\地图代数实验3\\VoronoiBoundary.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName5;
	sstr >> OutputBmpName5;
	getVoronoiBoundary(InputBmpName4,OutputBmpName5);
	*/
	/*
	char * OutputBmpName6 = new char[1024];
	string sOutputBmpName6 = "E:\\学习\\地图代数实验3\\Buffer.bmp";
	sstr.clear();	//多次类型转换前需要clear(),包含数据流时不可被赋值
	sstr << sOutputBmpName6;
	sstr >> OutputBmpName6;
	getBufferFromDis(10,InputBmpName5, OutputBmpName6);
	*/

	//cout << InputBmpName << endl;

	/*
	std::cout << "Hello World!\n";
	char* InputBmpName = new char[1024];

	std::stringstream sstr;
	sstr.clear();
	char * InputBmpName = new char[1024];
	std::string sInputBmpName = "E:\\学习\\地图代数实验1\\test.bmp";
	sstr << sInputBmpName;
	sstr >> InputBmpName;
	std::cout << InputBmpName << std::endl;
	*/

}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
