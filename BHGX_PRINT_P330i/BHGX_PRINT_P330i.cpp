// BHGX_PRINT_P330i.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "BHGX_PRINT_P330i.h"
#include "dcrf32/dcrf32.h"
#include <stdio.h>
#include <string>

using namespace std;
std::string g_PrinterName;

#pragma comment(lib, "dcrf32/dcrf32.lib")



//寻找打印机
typedef int (__stdcall *ProbePrinter)();

//释放打印机
typedef int (__stdcall *FreePrinter)();

//进卡
typedef int (__stdcall *FeedInCard_NonContract)();

//出卡
typedef int (__stdcall *BackCardToPrintHeader_NonContract)();

//初始化打印机
typedef int (__stdcall *InitGraphics)(const char *strPrinter);

//将文本输入打印缓冲(前两个参数为文本左上角坐标，
//第三个参数为打印内容，后面依次为打印字体，字体大小，字体风格(0)，字体颜色(0))
typedef int (__stdcall *PrintDrawText)(long, long, const char *, const char *, long, long, long);

//打印//如不能正常打印，说明打印的内容指定得有问题，或是打印的范围或坐标，超出了卡片范围
typedef int (__stdcall *PrintGraphics)();

//设备打印关闭
typedef int (__stdcall *CloseGraphics)();

//设备状态
typedef int (__stdcall *IsPrinterReady)(const char *strPrinter);

//出卡,打印成功的话,无需调用,直接出卡
typedef int (__stdcall *OutCard)();

struct Printer
{
	HINSTANCE	hInstLibrary;

	ProbePrinter		   iProbePrinter;
	FreePrinter			   iFreePrinter;
	FeedInCard_NonContract iFeedInCard;
	BackCardToPrintHeader_NonContract iBackCardToPrintHeader;
	InitGraphics iInitGraphics;
	PrintDrawText iDrawText;
	PrintGraphics iPrintGraphics;
	CloseGraphics iCloseGraphics;
	IsPrinterReady iIsPrinterReady;
	OutCard iOutCard;
	Printer()
		:iFeedInCard(NULL)
		,iFreePrinter(NULL)
		,iProbePrinter(NULL)
		,iBackCardToPrintHeader(NULL)
		,iInitGraphics(NULL)
		,iDrawText(NULL)
		,iPrintGraphics(NULL)
		,iCloseGraphics(NULL)
		,iIsPrinterReady(NULL)
		,iOutCard(NULL)
	{

	}
};

Printer m_iPrinter;

int __stdcall GetPrinterFunc()
{
	HINSTANCE hInstance = LoadLibrary("FOX_DC.dll");
	if (hInstance == NULL)
	{
		return -1;
	}
	m_iPrinter.iFeedInCard = (FeedInCard_NonContract)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_FeedInCard_NonContract");

	m_iPrinter.iBackCardToPrintHeader = (BackCardToPrintHeader_NonContract)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_BackCardToPrintHeader_NonContract");
	m_iPrinter.iInitGraphics = (InitGraphics)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_InitGraphics");

	m_iPrinter.iDrawText = (PrintDrawText)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_DrawText");
	m_iPrinter.iPrintGraphics = (PrintGraphics)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_PrintGraphics");
	m_iPrinter.iCloseGraphics = (CloseGraphics)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_CloseGraphics");

	m_iPrinter.iIsPrinterReady = (IsPrinterReady)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_IsPrinterReady");

	m_iPrinter.iOutCard = (OutCard)GetProcAddress(hInstance, "DC_USB_CardPrinter_OutCard");
	m_iPrinter.hInstLibrary = hInstance;

	return 0;
}

int __stdcall iProbePrinter()
{
	return (GetPrinterFunc()==0);
}	

int __stdcall iFreePrinter()
{
	FreeLibrary(m_iPrinter.hInstLibrary);
	return 0;
}

int __stdcall iFeedCardToM1(void)
{
	return m_iPrinter.iFeedInCard();
}
int __stdcall iBackCardFromM1(void)
{
	return m_iPrinter.iBackCardToPrintHeader();
}
int __stdcall iInitGraphics ( char *pszPrinterName )
{
	g_PrinterName = pszPrinterName;
	return m_iPrinter.iInitGraphics(pszPrinterName);
}
int __stdcall iPrintText(long nPosx, long nPosy, char *szContent,
						char *szFont, long nFontSize, 
						long nFontStyle, long nFontColoer)
{
	int nRet = m_iPrinter.iDrawText(nPosx, nPosy, szContent, szFont, nFontSize, nFontStyle, nFontColoer);
	return nRet;
}

int __stdcall iFlushGraphics ( void )
{
	return m_iPrinter.iPrintGraphics();
}

int __stdcall iCloseGraphics ( void )
{
	return m_iPrinter.iCloseGraphics();
}

int __stdcall iOutCard(void)
{
	return m_iPrinter.iOutCard();
}

int __stdcall iCheckPrinterStatus()
{
	return 1;//m_iPrinter.iIsPrinterReady(g_PrinterName.c_str());
}



