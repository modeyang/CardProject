// BHGX_PRINT_ALL.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include "BHGX_PRINT_ALL.h"
#include <string>
#include <vector>
#include <Windows.h>
#include <winspool.h>
#include <map>
using namespace std;

#pragma comment(lib, "winspool.lib")

#pragma warning (disable : 4996)

#define PRINT_TRUE		1
#define PRINT_FALSE		0

#define SAFE_DELETE(a)\
	if (a != NULL)\
{\
	free(a);\
	a = NULL;\
}\

#define  ASSERT_VALID_HANDLE(a) \
	if (a == NULL)\
		return 0;\

LOGFONT	m_Font;
HANDLE  m_PrintHandle = NULL;
std::string m_strPrinter("");

//std::map<std::string, HANDLE> g_mapPrinter;

struct FontPrinter 
{
	std::string strFont;
	long nFontSize;
	long nFontStyle;
	long nFontColor;
};

struct InfoPrinter 
{
	std::string strFont;
	long nFontSize;
	long nFontStyle;
	long nFontColor;
	long nPosX;
	long nPosY;
	std::string strContent;
};
static FontPrinter g_Font;
static std::vector<InfoPrinter> g_vecInfo;

LOGFONT GetFontHandle(char *szFont, long nFontSize)
{
	LOGFONT lg;
	HFONT hf = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::GetObject(hf, sizeof(LOGFONT), &lg);
	lg.lfHeight = nFontSize;
	strcpy(lg.lfFaceName, szFont);
	return lg;
}

LOGPEN GetPenHandle(long nPenStyle,long nPenColor)
{
	HPEN hp = (HPEN)::GetStockObject(WHITE_PEN);
	LOGPEN lp;
	::GetObject(hp, sizeof(LOGPEN), &lp);
	lp.lopnStyle = PS_SOLID;
	lp.lopnColor = (COLORREF)nPenColor;
	return lp;

}

int GetAllPrinterName(std::vector<std::string> &vecPrinter)
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

	if (NULL == pPrinterEnum)
	{
		return  0;
	}

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
		printf("获取打印机失败\n");
		return 0;
	}
	printf("获取打印机列表%d\n", pcReturned);
	for (unsigned int i=0; i<pcReturned; i++)
	{
		PRINTER_INFO_1A printer = pPrinterEnum[i];
		vecPrinter.push_back(printer.pName);
		printf("%s\n",printer.pName);
	}
	printf("*******************\n");
	return (int)vecPrinter.size();
}


/// 获取打印作业的状态
BOOL GetJobs(HANDLE hPrinter,
			 JOB_INFO_2 **ppJobInfo,
			 int *pcJobs,           
			 DWORD *pStatus)        

{

	DWORD       cByteNeeded, nReturned, cByteUsed;
	JOB_INFO_2          *pJobStorage = NULL;
	PRINTER_INFO_2       *pPrinterInfo = NULL;

	if (!GetPrinter(hPrinter, 2, NULL, 0, &cByteNeeded))
	{
		DWORD dwCode = GetLastError();
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			return FALSE;
		}
	}

	pPrinterInfo = (PRINTER_INFO_2 *)malloc(cByteNeeded);
	if (!(pPrinterInfo))
		return FALSE;


	if (!GetPrinter(hPrinter,
		2,
		(LPBYTE)pPrinterInfo,
		cByteNeeded,
		&cByteUsed))
	{

		SAFE_DELETE(pPrinterInfo);
		pPrinterInfo = NULL;
		return FALSE;
	}

	cByteUsed = 0;
	cByteNeeded = 0;
	nReturned = 0;
	if (!EnumJobs(hPrinter,
		0,
		pPrinterInfo->cJobs,
		2,
		NULL,
		0,
		(LPDWORD)&cByteNeeded,
		(LPDWORD)&nReturned))
	{
		DWORD dwErrCode = GetLastError();
		if (dwErrCode != ERROR_INSUFFICIENT_BUFFER)
		{
			SAFE_DELETE(pPrinterInfo);
			return FALSE;
		}
	}

	pJobStorage = (JOB_INFO_2 *)malloc(cByteNeeded);
	if (!pJobStorage)
	{
		SAFE_DELETE(pPrinterInfo);
		return FALSE;
	}

	ZeroMemory(pJobStorage, cByteNeeded);

	if (!EnumJobs(hPrinter,
		0,
		pPrinterInfo->cJobs,
		2,
		(LPBYTE)pJobStorage,
		cByteNeeded,
		(LPDWORD)&cByteUsed,
		(LPDWORD)&nReturned))
	{
		SAFE_DELETE(pPrinterInfo);
		SAFE_DELETE(pJobStorage);
		return FALSE;
	}

	*pcJobs = nReturned;
	*pStatus = pPrinterInfo->Status;
	*ppJobInfo = pJobStorage;
	SAFE_DELETE(pPrinterInfo);
	return TRUE;
}



/// 检测打印机的状态
/// hPrinter表示打印机Handle
BOOL CheckPrinterStatus(HANDLE hPrinter)
{
	BOOL bResult = PRINT_FALSE;
	if(NULL == hPrinter)
	{
		return bResult;
	}

	JOB_INFO_2  *pJobs;
	int         cJobs,i;
	DWORD       dwPrinterStatus;


	if (!GetJobs(hPrinter, &pJobs, &cJobs, &dwPrinterStatus))
	{
		return bResult;
	}

	if (dwPrinterStatus &
		(PRINTER_STATUS_ERROR |     //打印出错
		PRINTER_STATUS_PAPER_JAM |        //卡纸
		PRINTER_STATUS_PAPER_OUT |
		PRINTER_STATUS_PAPER_PROBLEM |   //打印纸出现问题
		PRINTER_STATUS_OUTPUT_BIN_FULL |  //打印输出已满
		PRINTER_STATUS_NOT_AVAILABLE |   //打印机不可用
		PRINTER_STATUS_NO_TONER |    //没有墨粉
		PRINTER_STATUS_OUT_OF_MEMORY |   //打印内存出错
		PRINTER_STATUS_OFFLINE |    //未联机
		PRINTER_STATUS_DOOR_OPEN))    //打印机的门是开的
	{
		SAFE_DELETE( pJobs );
		return bResult;
	}

	for (i = 0; i < cJobs; ++i)
	{
		/// 如果打印页正在打印
		if (pJobs[i].Status & JOB_STATUS_PRINTING)
		{

			if (pJobs[i].Status &
				(JOB_STATUS_ERROR |
				JOB_STATUS_OFFLINE |
				JOB_STATUS_PAPEROUT |
				JOB_STATUS_BLOCKED_DEVQ))
		 {
			 SAFE_DELETE( pJobs );
			 return bResult;
		 }
		}


		// 如果打印页已经打印
		if(pJobs[i].Status & JOB_STATUS_PRINTED)
		{
			SAFE_DELETE( pJobs );
			bResult = PRINT_TRUE;
			return bResult;
		}
		/// 如果已经删除打印作业
		else if(pJobs[i].Status & JOB_STATUS_DELETED)
		{
			SAFE_DELETE( pJobs );
			bResult = PRINT_TRUE;
			return bResult;
		}
#if (NTDDI_VERSION >= NTDDI_WINXP)
		else if(pJobs[i].Status & JOB_STATUS_COMPLETE)
		{
			SAFE_DELETE( pJobs );
			bResult = PRINT_TRUE;
			return bResult;
		}
#endif // (NTDDI_VERSION >= NTDDI_WINXP)
	}
	/// 如果不存在打印任务
	if(0 == cJobs)
	{
		SAFE_DELETE( pJobs );
		bResult = PRINT_TRUE;
		return bResult;
	}

	SAFE_DELETE( pJobs );
	bResult = PRINT_TRUE;
	return bResult;

}


int __stdcall iProbePrinter()
{
	//GetAllPrinterName(m_vecPrinter);
	return PRINT_TRUE;
}	

int __stdcall iFreePrinter()
{
	ASSERT_VALID_HANDLE(m_PrintHandle);

	ClosePrinter(m_PrintHandle);
	return PRINT_TRUE;
}

int __stdcall iFeedCardToM1(void)
{
	return PRINT_TRUE;
}
int __stdcall iBackCardFromM1(void)
{
	return PRINT_TRUE;
}
int __stdcall iInitGraphics (char *pszPrinterName)
{
	std::vector<std::string> vecPrinter;
	std::string strName = pszPrinterName;
	HANDLE hPrinter;
	if (0 != ::OpenPrinter((LPSTR)strName.c_str(), &hPrinter, NULL))
	{
		if (PRINT_TRUE == CheckPrinterStatus(hPrinter))
		{
			m_PrintHandle = hPrinter;
			m_strPrinter = strName;
		}
	}
	return (m_PrintHandle != NULL);
}

int __stdcall iPrintText(long nPosx, long nPosy, char *szContent,
						 char *szFont, long nFontSize, 
						 long nFontStyle, long nFontColoer)
{
	//g_Font.strFont = szFont;
	//g_Font.nFontSize = nFontSize;
	//g_Font.nFontStyle = nFontStyle;
	//g_Font.nFontColor = nFontSize;

	InfoPrinter stInfo;
	stInfo.strFont = szFont;
	stInfo.nFontSize = nFontSize;
	stInfo.nFontStyle = nFontStyle;
	stInfo.nFontColor = nFontSize;
	stInfo.nPosX = nPosx;
	stInfo.nPosY = nPosy;
	stInfo.strContent = szContent;
	g_vecInfo.push_back(stInfo);
	return PRINT_TRUE;
}

int __stdcall iFlushGraphics ( void )
{
	/*std::map<std::string, HANDLE>::iterator iter = g_mapPrinter.begin();
	for (; iter != g_mapPrinter.end(); ++iter)
	{*/
		//std::string strPrinter = iter->first;
		DOCINFO   di;
		HDC hdc = CreateDC(NULL, m_strPrinter.c_str(), NULL, NULL);

		if (hdc != NULL)
		{
			/* Set job info */
			di.cbSize      = sizeof(di);
			di.lpszDocName = "Print Card";
			di.lpszOutput  = NULL;
			di.lpszDatatype = NULL;
			di.fwType = 0;

			/* Start job */
			if (StartDoc(hdc, &di) != SP_ERROR)
			{
				if (StartPage (hdc) > 0)
				{
					//m_strPrinter = strPrinter;
					//m_PrintHandle = iter->second;

					/* Set mapping mode for the printer */

					for (size_t i=0; i<g_vecInfo.size(); ++i)
					{
						InfoPrinter &stInfo = g_vecInfo[i];
						int nSize = -MulDiv( stInfo.nFontSize/*字体大小*/,::GetDeviceCaps(hdc, LOGPIXELSY),72);
						LOGFONT lg = GetFontHandle((LPSTR)stInfo.strFont.c_str(), nSize);
						HFONT hFont = ::CreateFontIndirect(&lg);
						HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

						LOGPEN lp = GetPenHandle(stInfo.nFontStyle, stInfo.nFontColor);
						HPEN hp = ::CreatePenIndirect(&lp);
						HPEN oldpen = (HPEN)SelectObject(hdc, hp);
						SetROP2(hdc, R2_COPYPEN);
						
						TextOut(hdc, stInfo.nPosX, stInfo.nPosY, stInfo.strContent.c_str(), stInfo.strContent.size());
						SelectObject(hdc, oldpen);
						DeleteObject(hp);
						SelectObject(hdc, oldFont);
						DeleteObject(hFont);
					}

				}
				EndPage(hdc);
			}
			EndDoc(hdc);
			DeleteDC(hdc);

		}

	//}

	return PRINT_TRUE;
}

int __stdcall iCloseGraphics ( void )
{
	return PRINT_TRUE;
}

int __stdcall iOutCard(void)
{
	ASSERT_VALID_HANDLE(m_PrintHandle);
	AbortPrinter(m_PrintHandle);
	return 0;
}

int __stdcall iCheckPrinterStatus()
{
	return CheckPrinterStatus(m_PrintHandle);
}


