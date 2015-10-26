
#include "BHGX_Printer.h"
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <Windows.h>
#include <winspool.h>
#include "tinyxml/headers/tinyxml.h"
#include "public/debug.h"
#include "Card.h"

using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")
#pragma comment(lib, "winspool.lib")

#define MAX_RESULT 256
#pragma warning (disable : 4244)
#pragma warning (disable : 4996)

CBHGX_Printer::CBHGX_Printer()
:m_bInit(false)
,m_bALL(false)
{
}

CBHGX_Printer::~CBHGX_Printer()
{
	if (m_iPrinter.iFreePrinter != NULL) {
		m_iPrinter.iFreePrinter();
		FreeLibrary(m_iPrinter.hInstLibrary);
		
	}
}

void GetPrintType(const char *strName, char *strType)
{
	std::string szName(strName);
	size_t nPos = szName.find(" ");
	if (nPos == -1){
		strcpy(strType, strName);
	}else{
		szName = szName.substr(szName.find(" ")+1, szName.length());
		szName = szName.substr(0, szName.find(" "));
		strcpy(strType, szName.c_str());
	}

}

int	 CBHGX_Printer::Init(char *pszPrinter)
{
	char szDLL[100] ;
	memset(szDLL, 0, sizeof(szDLL));
	bool bLoad = false;
	HINSTANCE hInst = NULL;
	char strType[50];
	memset(strType, 0, sizeof(strType));

	if (strlen(pszPrinter) == 0){
		GetDefaultPrinterName(m_strPrinter);
	}else{
		m_strPrinter = pszPrinter;
	}
	

	GetPrintType((char*)m_strPrinter.c_str(), strType);
	strType[strlen(strType)] = 0;

	sprintf_s(szDLL, "%s\\BHGX_PRINT_%s.dll", DRIVER_PATH, strType);
	hInst = LoadLibrary(szDLL);
	if (hInst != NULL){
		bLoad = true;
	}

	std::string strDefaultPrinter;
	GetDefaultPrinterName(strDefaultPrinter);
	if (m_strPrinter != strDefaultPrinter){
		SetDefaultPrinter(pszPrinter);
	}

	if (!bLoad){
		memset(szDLL, 0, sizeof(szDLL));
		sprintf_s(szDLL, "%s\\BHGX_PRINT_ALL.dll", DRIVER_PATH);
		hInst = LoadLibrary(szDLL);
		if (hInst != NULL){
			bLoad = true;
		}
	}

	LOG_INFO("调用打印的DLL为:%s", szDLL);

	if (bLoad){
		m_iPrinter.hInstLibrary = hInst;
		m_iPrinter.iProbePrinter = (ProbePrinter)GetProcAddress(hInst, "iProbePrinter");
		LOG_INFO("iProbePrinter: %d, status: %d", m_iPrinter.iProbePrinter, m_iPrinter.iProbePrinter());

		//if (m_iPrinter.iProbePrinter != NULL && m_iPrinter.iProbePrinter())
		//{
			m_iPrinter.iFeedInCard = (FeedCardToM1)GetProcAddress(hInst,"iFeedCardToM1");
			m_iPrinter.iBackCardToPrintHeader = (BackCardFromM1)GetProcAddress(hInst,"iBackCardFromM1");
			m_iPrinter.iInitGraphics = (InitGraphics)GetProcAddress(hInst, "iInitGraphics");
			m_iPrinter.iDrawText = (PrintDrawText)GetProcAddress(hInst, "iPrintText");
			m_iPrinter.iPrintGraphics = (PrintGraphics)GetProcAddress(hInst,"iFlushGraphics");
			m_iPrinter.iCloseGraphics = (CloseGraphics)GetProcAddress(hInst,"iCloseGraphics");
			m_iPrinter.iOutCard = (OutCard)GetProcAddress(hInst, "iOutCard");
			m_iPrinter.iFreePrinter = (FreePrinter)GetProcAddress(hInst, "iFreePrinter");
			m_iPrinter.iCheckPrinterStatus = (iCheckStatus)GetProcAddress(hInst, "iCheckPrinterStatus");

			m_bInit = true;
			LOG_INFO("加载动态库成功");
			return 0;
		//}
	}
	LOG_ERROR("加载动态库失败");
	return -1;
}

int	 CBHGX_Printer::FeedCard()
{
	if (m_iPrinter.iFeedInCard != NULL){
		return m_iPrinter.iFeedInCard();
	}
	return -1;
}

int CBHGX_Printer::BackToPrintHeader()
{
	if (m_iPrinter.iBackCardToPrintHeader != NULL) {
		return m_iPrinter.iBackCardToPrintHeader();
	}
	return -1;
}

int  CBHGX_Printer::CheckStatus()
{
	if (m_iPrinter.iCheckPrinterStatus != NULL) {
		return m_iPrinter.iCheckPrinterStatus();
	}
	return 0;
}

int	 CBHGX_Printer::GetPrinterList(std::vector<std::string> &vecPrinter)
{
	DWORD Flags = PRINTER_ENUM_NAME; //local   printers
	DWORD cbBuf;
	DWORD pcReturned;

	DWORD Level = 1;
	TCHAR Name[500];
	LPPRINTER_INFO_1 pPrinterEnum = NULL;

	memset(Name,0,sizeof(TCHAR)*500);
	::EnumPrinters(Flags, "", Level, NULL, 0, &cbBuf, &pcReturned);
	pPrinterEnum = (LPPRINTER_INFO_1)LocalAlloc(LPTR, cbBuf+4);

	if (NULL == pPrinterEnum){
		LOG_ERROR("获取打印机失败, NULL");
		return  0;
	}

	LOG_INFO("pPrinterEnum : %d", pPrinterEnum);
	if (!EnumPrinters(
		Flags,    //   DWORD   Flags,   printer   object   types  
		Name,    //   LPTSTR   Name,   name   of   printer   object  
		Level,    //   DWORD   Level,   information   level  
		(LPBYTE)pPrinterEnum,    //   LPBYTE   pPrinterEnum,   printer   information   buffer  
		cbBuf,    //   DWORD   cbBuf,   size   of   printer   information   buffer
		&cbBuf, //   LPDWORD   pcbNeeded,   bytes   received   or   required  
		&pcReturned)    //   LPDWORD   pcReturned   number   of   printers   enumerated  
		)
	{
		LOG_ERROR("获取打印机失败");
		return 0;
	}
	for (unsigned int i=0; i<pcReturned; i++)
	{
		PRINTER_INFO_1A printer = pPrinterEnum[i];
		vecPrinter.push_back(printer.pName);
		LOG_INFO("found printer name: %s", printer.pName);
	}
	LocalFree(pPrinterEnum);
	return (int)vecPrinter.size();
}

int CBHGX_Printer::InitPrinter(char *CardCoverDataXml,char *pszXZQHXML)
{
	if (CreatePrintInfo(pszXZQHXML) != 0){
		return -1;
	}

	if (CreatePrintData(CardCoverDataXml) != 0){
		return -1;
	}
	return 0;
}


int  CBHGX_Printer::DeInitPrinter()
{
	m_bInit = false;
	if (m_iPrinter.iOutCard != NULL) {
		return m_iPrinter.iOutCard();
	}
	return 0;
}

int CBHGX_Printer::StartPrint()
{
	int nRet = -1;
	if (m_bInit)
	{
		//m_iPrinter.iBackCardToPrintHeader();
		nRet = m_iPrinter.iInitGraphics(m_strPrinter.c_str());
		LOG_INFO("iInitGraphics : %s, status: %d", m_strPrinter.c_str(), nRet);
		//if (nRet != 0){
		//	return -1;
		//}
		LOG_INFO("vecPrintSeg size: %d", m_vecPrintSeg.size());
		for (size_t i=0; i<m_vecPrintSeg.size(); i++)
		{
			PrintSegMent &seg = m_vecPrintSeg[i];
			
			if (seg.bPrint){

				m_iPrinter.iDrawText(seg.SegPrintInfo.xPos, seg.SegPrintInfo.yPos,
					seg.SegPrintInfo.strTarget.c_str(), seg.SegPrintInfo.strFontFace.c_str(),
					seg.SegPrintInfo.nFontHeight, seg.SegPrintInfo.lFontStyle, seg.SegPrintInfo.lColor);

				LOG_INFO("%d, %d, %s, %s, %d, %d, %d", seg.SegPrintInfo.xPos, seg.SegPrintInfo.yPos,
					seg.SegPrintInfo.strTarget.c_str(), seg.SegPrintInfo.strFontFace.c_str(),
					seg.SegPrintInfo.nFontHeight, seg.SegPrintInfo.lFontStyle, seg.SegPrintInfo.lColor);

				for (size_t j=0; j<seg.vecPrintColumn.size(); ++j)
				{
					PrintColumn &stColum = seg.vecPrintColumn[j];
					m_iPrinter.iDrawText(stColum.ColumnPrintInfo.xPos, stColum.ColumnPrintInfo.yPos,
						stColum.strSource.c_str(), stColum.ColumnPrintInfo.strFontFace.c_str(),
						stColum.ColumnPrintInfo.nFontHeight, stColum.ColumnPrintInfo.lFontStyle,
						stColum.ColumnPrintInfo.lColor);

					LOG_INFO("%d, %d, %s, %s, %d, %d, %d", stColum.ColumnPrintInfo.xPos, stColum.ColumnPrintInfo.yPos,
						stColum.strSource.c_str(), stColum.ColumnPrintInfo.strFontFace.c_str(),
						stColum.ColumnPrintInfo.nFontHeight, stColum.ColumnPrintInfo.lFontStyle,
						stColum.ColumnPrintInfo.lColor);
				}
			}
		}
		nRet = m_iPrinter.iPrintGraphics();
		LOG_INFO("iPrintGraphics status: %d",nRet); 
		nRet = m_iPrinter.iCloseGraphics();
		LOG_INFO("iCloseGraphics status: %d",nRet); 
	}
	return nRet;
}

int CBHGX_Printer::CreatePrintData(char *pszCardXml)
{
	TiXmlDocument XmlDoc;
	TiXmlElement *RootElement = NULL;
	TiXmlElement *Segment = NULL;
	TiXmlElement *Cloumn = NULL;
	XmlDoc.Parse(pszCardXml);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return -1;
	}
	Segment = RootElement->FirstChildElement();
	if (Segment != NULL){
		int nSegID = atoi(Segment->Attribute("ID"));
		if (nSegID < 0){
			return -1;
		}
		Cloumn = Segment->FirstChildElement();
		while (Cloumn != NULL)
		{
			int nColumID = atoi(Cloumn->Attribute("ID"));
			nColumID = QuerySegment(nColumID);
			if (nColumID < 0){
				continue;
			}
			PrintSegMent &stSegment = m_vecPrintSeg[nColumID];
			stSegment.bPrint = true;
			std::string szContent = Cloumn->Attribute("VALUE");
			if (stSegment.vecPrintColumn.size() > 1){

				int CtrlPos = stSegment.vecPrintColumn[0].ColumnPrintInfo.nCtrl;
				if (szContent.length() > 2*CtrlPos){

					stSegment.vecPrintColumn[0].strSource = szContent.substr(0, 2*CtrlPos);
					stSegment.vecPrintColumn[1].strSource = szContent.substr(2*CtrlPos, szContent.size());
					BackupLine &line = m_mapBackup[stSegment.nID];
					stSegment.vecPrintColumn[0].ColumnPrintInfo.yPos = line.ypos;
					stSegment.vecPrintColumn[0].ColumnPrintInfo.nFontHeight = line.nFontHeight;
					stSegment.vecPrintColumn[1].ColumnPrintInfo.nFontHeight = line.nFontHeight;
				} else{
					stSegment.vecPrintColumn[0].strSource = szContent;
					stSegment.vecPrintColumn.pop_back();
					//stSegment.vecPrintColumn.erase(++stSegment.vecPrintColumn.begin());
				}
			}else{
				stSegment.vecPrintColumn[0].strSource = szContent;
			}
			Cloumn = Cloumn->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
	return 0;

}


int CBHGX_Printer::CreatePrintInfo(char *szPrintXML)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement = NULL;
	TiXmlElement  *Program = NULL;
	TiXmlElement  *Segment = NULL;
	TiXmlElement  *Colum = NULL;
	TiXmlElement  *SubCol = NULL;

	XmlDoc.Parse(szPrintXML);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return -1;
	}
	Program = RootElement->FirstChildElement();
	int nProgramID = atoi(Program->Attribute("ID"));
	std::string strTarget = Program->Attribute("TARGET");
	Segment = Program->FirstChildElement();
	while (Segment)
	{
		PrintSegMent stSegment;
		stSegment.nID = atoi(Segment->Attribute("ID"));
		stSegment.SegPrintInfo.strTarget = Segment->Attribute("TARGET");
		stSegment.SegPrintInfo.lColor = atoi(Segment->Attribute("COLOR"));
		stSegment.SegPrintInfo.xPos = Cm2Pos(atof(Segment->Attribute("X")));
		stSegment.SegPrintInfo.yPos = Cm2Pos(atof(Segment->Attribute("Y")));
		stSegment.SegPrintInfo.nHeight = Cm2Pos(atof(Segment->Attribute("HEIGHT")));
		stSegment.SegPrintInfo.nWidth = Cm2Pos(atof(Segment->Attribute("WIDTH")));
		stSegment.SegPrintInfo.strFontFace = Segment->Attribute("FONT.FACE");
		stSegment.SegPrintInfo.nFontHeight = abs(atoi(Segment->Attribute("FONT.HEITHT")));
		stSegment.bPrint = false;
		Colum = Segment->FirstChildElement();
		while (Colum)
		{
			PrintColumn stColumn;
			stColumn.nID = atoi(Colum->Attribute("ID"));
			stColumn.strSource = Colum->Attribute("SOURCE");
			stColumn.ColumnPrintInfo.strTarget = Colum->Attribute("TARGET");
			stColumn.ColumnPrintInfo.lColor = atoi(Colum->Attribute("COLOR"));
			stColumn.ColumnPrintInfo.xPos = Cm2Pos(atof(Colum->Attribute("X")));
			stColumn.ColumnPrintInfo.yPos = Cm2Pos(atof(Colum->Attribute("Y")));
			stColumn.ColumnPrintInfo.nHeight = Cm2Pos(atof(Colum->Attribute("HEIGHT")));
			stColumn.ColumnPrintInfo.nWidth = Cm2Pos(atof(Colum->Attribute("WIDTH")));
			stColumn.ColumnPrintInfo.strFontFace = Colum->Attribute("FONT.FACE");
			stColumn.ColumnPrintInfo.nFontHeight = abs(atoi(Colum->Attribute("FONT.HEITHT")));

			//add by yanggx 2-28 增加打印换行问题
			stColumn.ColumnPrintInfo.nCtrl = atoi(Colum->Attribute("CTRL"));
			stSegment.vecPrintColumn.push_back(stColumn);
			if (stColumn.ColumnPrintInfo.nCtrl > 0)
			{
				PrintColumn SubPrintCol = stColumn;
				SubCol = Colum->FirstChildElement();
				SubPrintCol.ColumnPrintInfo.xPos = Cm2Pos(atof(SubCol->Attribute("X2")));
				SubPrintCol.ColumnPrintInfo.yPos = Cm2Pos(atof(SubCol->Attribute("Y2")));
				stSegment.vecPrintColumn.push_back(SubPrintCol);
				BackupLine line;
				line.ypos = Cm2Pos(atof(SubCol->Attribute("Y1")));
				line.nFontHeight = abs(atoi(SubCol->Attribute("FONT.HEITHT")));
				m_mapBackup[stSegment.nID] = line;

			}
			Colum = Colum->NextSiblingElement();
			
		}
		Segment = Segment->NextSiblingElement();
		m_vecPrintSeg.push_back(stSegment);
	}
	return 0;
}

int CBHGX_Printer::QuerySegment(int nID)
{
	for (size_t i=0; i<m_vecPrintSeg.size(); ++i)
	{
		PrintSegMent &Seg = m_vecPrintSeg[i];
		if (nID == Seg.nID) {
			return (int)i;
		}
	}
	return -1;
}

int CBHGX_Printer::QueryColumn(PrintSegMent &segment, int nID)
{
	for (size_t i=0; i<segment.vecPrintColumn.size(); ++i) {
		
		PrintColumn &Column = segment.vecPrintColumn[i];
		if (nID == Column.nID){
			return (int)i;
		}
	}
	return -1;
}

int CBHGX_Printer::GetDefaultPrinterName(std::string &strDefaultPrinter)
{
	DWORD len = 0;
	::GetDefaultPrinter(NULL, &len);
	char szPrinter[100];
	GetDefaultPrinter(szPrinter, &len);
	szPrinter[len] = 0;
	strDefaultPrinter = szPrinter;
	return 0;
}

