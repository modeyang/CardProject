// BHGX_CPUCardLib.cpp : 定义 DLL 应用程序的入口点。
//
#include "stdafx.h"
#include "adapter.h"
#include "public/Markup.h"
#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"
#include "public/liberr.h"
#include "UtilProcess.h"
#include "resource.h"


#pragma warning (disable : 4996)
static BOOL g_CardInit = FALSE;
struct XmlProgramS *g_XmlListHead = NULL;

#define ASSERT_INIT(a)\
	if (a != TRUE)\
{\
	return -1;\
}\

// 得到卡版本号
int __stdcall iGetCardVersion(char *pszVersion)
{
	strcpy(pszVersion, "Version 1.0.0.1");
	return 0;
}

// 动态链接库初始化函数
int __stdcall iCardInit(char *xml)
{
	if (g_CardInit)
		return 0;

	// 在资源文件里边提取XML文件并且初始化他
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CPUCardLib.dll");
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_XML1),"XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes) 
	{	
		return CardInitErr;
	}
  
	char szSystem[256];
	memset(szSystem, 0, sizeof(szSystem));
	ReadConfigFromReg(szSystem);

	// 对设备进行初始化
	g_CardInit = (initCoreDevice(szSystem)==0);

	if (g_CardInit)
	{
		CreateCPUData(pvRes);
	}
	return 0;
}

int __stdcall iCardDeinit()
{
	DestroyList(g_XmlListHead->SegHeader, 0);
	free(g_XmlListHead);
	g_CardInit = FALSE;
	g_XmlListHead = NULL;
	return closeCoreDevice();
}


int __stdcall iCardClose()
{
	return 0;
}

int __stdcall iCardOpen()
{
	return 0;
}

int __stdcall iCardIsEmpty()
{
	return 0;
}

// 读取flag标志，输出xml
int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_INIT(g_CardInit);
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int res;

	list = GetXmlSegmentByFlag(flag);
	if (list == NULL)
	{
		return CardXmlErr;
	}

	// 获取读写链表
	RequestList = CreateRequest(list, 0);

	// 设备的真实读取
	res = iReadCard(RequestList);

	// 销毁读写请求链表
	DestroyRequest(RequestList, 0);

	// 通过链表产生XML字符串
	iConvertXmlByList(list, xml, &length);

	DestroyList(list, 1);

	return res!=0 ? CardReadErr : 0;
}

int __stdcall iReadAnyInfo(int flag, char *xml, char *name)
{
	return 0;
}

// 读取name信息，单个标志情况，
int __stdcall iQueryInfo(char *name, char *xml)
{
	return 0;
}

// 写入xml内容
int __stdcall iWriteInfo(char *xml)
{
	//ASSERT_INIT(g_CardInit);

	if (CheckCardXMLValid(xml) < 0)
	{
		printf("CardXML:Check Error\n");
		return CardXmlErr;
	}
	struct XmlSegmentS *XmlList;
	struct RWRequestS	*RequestList = NULL;
	unsigned int res = 0;

	// 产生List
	XmlList = ConvertXmltoList(xml);

	if (XmlList == NULL)
	{
		return CardXmlErr;
	}

	// 产生读写链表
	RequestList = CreateRequest(XmlList, 0);

	// 对设备进行真实的写
	res = iWriteCard(RequestList);

	// 销毁读写链表
	DestroyRequest(RequestList, 0);

	// 销毁XML链表
	DestroyList(XmlList, 1);

	return res;
}

int __stdcall iScanCard()
{
	return 0;
}

//错误信息
char* __stdcall err(int errcode)
{
	return _err(errcode);
}


//制卡
int __stdcall iCreateCard(char *pszCardDataXml)
{
	return 0;
}

int __stdcall iFormatCard()
{
	return 0;
}

int __stdcall iCardCtlCard(int cmd, void *data)
{
	return 0;
}

//卡校验
int __stdcall iCheckMsgForNH(
				char *pszCardCheckWSDL, 
				char *pszCardServerURL, 
				char* pszXml
				)
{
	return 0;
}

//1|2|3|4 对应 姓名|身份证号|出生日期|性别
int __stdcall iReadConfigMsg(
				char *pszConfigXML,
				char *pszReadXML
				)
{
	return 0;
}

//卡注册
int __stdcall iRegMsgForNH(
			  char *pszCardServerURL, 
			  char* pszXml
			  )
{
	return 0;
}

int __stdcall iReadCardMessageForNH(
				char *pszCardCheckWSDL, 
				char *pszCardServerURL, 
				char* pszXml
				)
{
	return 0;
}

int __stdcall iEncryFile(char *filename)
{
	return 0;
}

//制卡并打印
int __stdcall iPatchCard(
				 char *pszCardDataXml,
				 char *pszCardCoverDataXml,
				 char *pszPrinterType,
				 char *pszCardCoverXml 
				 )
{
	return 0;
}

//获取打印机列表名字
int __stdcall iGetPrinterList(char *PrinterXML)
{
	return 0;
}

//打印卡封面
int __stdcall iPrintCard(
				 char *pszPrinterType,
				 char *pszCardCoverDataXml,
				 char *pszCardCoverXml
				 )
{
	return 0;
}

