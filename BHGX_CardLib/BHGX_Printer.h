#ifndef _BHGX_PRINTER_H
#define _BHGX_PRINTER_H
#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

//寻找打印机
typedef int (__stdcall *ProbePrinter)();

//释放打印机
typedef int (__stdcall *FreePrinter)();

//进卡
typedef int (__stdcall *FeedCardToM1)();

//出卡
typedef int (__stdcall *BackCardFromM1)();

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

typedef int (__stdcall *iCheckStatus)();


struct Printer
{
	HINSTANCE	hInstLibrary;

	ProbePrinter iProbePrinter;
	FreePrinter  iFreePrinter;
	FeedCardToM1 iFeedInCard;
	BackCardFromM1 iBackCardToPrintHeader;
	InitGraphics iInitGraphics;
	PrintDrawText iDrawText;
	PrintGraphics iPrintGraphics;
	CloseGraphics iCloseGraphics;
	IsPrinterReady iIsPrinterReady;
	iCheckStatus	   iCheckPrinterStatus;
	OutCard iOutCard;
	Printer()
		:iFeedInCard(NULL)
		,iProbePrinter(NULL)
		,iFreePrinter(NULL)
		,iBackCardToPrintHeader(NULL)
		,iInitGraphics(NULL)
		,iDrawText(NULL)
		,iPrintGraphics(NULL)
		,iCloseGraphics(NULL)
		,iIsPrinterReady(NULL)
		,iOutCard(NULL)
		,iCheckPrinterStatus(NULL)
	{

	}
};

struct  BH_PrintInfo
{
	int xPos;
	int yPos;
	int nHeight;
	int nWidth;
	long lColor;
	long lFontStyle;
	int nFontHeight;
	int nCtrl;
	std::string strTarget;
	std::string strFontFace;

	BH_PrintInfo()
		:xPos(0)
		,yPos(0)
		,nHeight(0)
		,nWidth(0)
		,lColor(0xffffffff)
		,lFontStyle(0)
		,nFontHeight(0)
		,nCtrl(0)
	{

	}

	BH_PrintInfo(const BH_PrintInfo &node)
		:xPos(node.xPos)
		,yPos(node.yPos)
		,nHeight(node.nHeight)
		,nWidth(node.nWidth)
		,nFontHeight(node.nFontHeight)
		,lColor(node.lColor)
		,lFontStyle(node.lFontStyle)
		,strFontFace(node.strFontFace)
		,strTarget(node.strTarget)
		,nCtrl(node.nCtrl)
	{

	}


	BH_PrintInfo &operator=(const BH_PrintInfo &node)
	{
		if (&node != this)
		{
			xPos = node.xPos;
			yPos = node.yPos;
			nHeight = node.nHeight;
			nWidth = node.nWidth;
			lColor = node.lColor;
			lFontStyle = node.lFontStyle;
			nFontHeight = node.nFontHeight;
			strTarget = node.strTarget;
			strFontFace = node.strFontFace;
			nCtrl = node.nCtrl;
		}
		return *this;
	}
};

struct BackupLine
{
	int ypos;
	int nFontHeight;

	BackupLine()
		:ypos(0)
		,nFontHeight(0)
	{

	}

	BackupLine(const BackupLine &node)
		:ypos(node.ypos)
		,nFontHeight(node.nFontHeight)
	{

	}

	BackupLine &operator=(const BackupLine &node)
	{
		if (this != &node)
		{
			ypos = node.ypos;
			nFontHeight = node.nFontHeight;
		}
		return *this;
	}
};


struct PrintColumn 
{
	int nID;
	std::string strSource;
	BH_PrintInfo ColumnPrintInfo;

};

typedef std::vector<PrintColumn> vecColumn;

struct  PrintSegMent
{
	int				nID;
	std::string		strContent;
	BH_PrintInfo	SegPrintInfo;
	bool			bPrint;
	vecColumn		vecPrintColumn;
	PrintSegMent()
		:nID(0)
		,bPrint(FALSE)
	{

	}

	PrintSegMent(const PrintSegMent &node)
		:nID(node.nID)
		,strContent(node.strContent)
		,SegPrintInfo(node.SegPrintInfo)
		,vecPrintColumn(node.vecPrintColumn)
		,bPrint(false)
	{

	}

	PrintSegMent &operator=(const PrintSegMent &node)
	{
		if (&node != this)
		{
			nID = node.nID;
			strContent = node.strContent;
			SegPrintInfo = node.SegPrintInfo;
			vecPrintColumn = node.vecPrintColumn;
			bPrint = node.bPrint;
		}
		return *this;
	}
};

typedef std::map<int, PrintSegMent> mapSegment;
typedef std::vector<PrintSegMent> vecSegment;

class CBHGX_Printer
{
public:
	CBHGX_Printer(void);
	~CBHGX_Printer(void);
	int	 Init(char *pszPrinter);
	int  InitPrinter(char *CardCoverDataXml,char *pszXZQHXML);
	int  DeInitPrinter();
	int  StartPrint();
	int	 FeedCard();
	int  BackToPrintHeader();
	int  CheckStatus();
	static int	GetPrinterList(std::vector<std::string> &vecPrinter);
protected:
	int CreatePrintData(char *pszCardXml);
	int CreatePrintInfo(char *szPrintXML);
	int QuerySegment(int nID);
	int QueryColumn(PrintSegMent &segment, int nID);
	int GetDefaultPrinterName(std::string &strDefaultPrinter);
	

	int Cm2Pos(float fPos)
	{
		return (int)(fPos/2.54*300);
	}
protected:
	Printer m_iPrinter;
	std::string m_strPrinter;
	bool m_bInit;
	bool m_bALL;
	vecSegment m_vecPrintSeg;
	std::map<int, BackupLine> m_mapBackup;
};

#endif

