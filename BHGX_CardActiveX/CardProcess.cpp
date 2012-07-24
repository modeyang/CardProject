// CardProcess.cpp : CCardProcess 的实现

#include "stdafx.h"   
#include "CardProcess.h"
#include "../BHGX_HISReader/BHGX_HISReader.h"
#include "../BHGX_CardLib/src/BHGX_CardLib.h"
#include "tinyxml/headers/tinyxml.h"
#include <comutil.h>
#include <comdef.h>
#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#pragma comment(lib, "../debug/BHGX_HISReader.lib")
#else
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#pragma comment(lib, "../release/BHGX_HISReader.lib")
#endif

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#pragma comment(lib, "comsupp.lib")    

#define SAFE_DELETE(a)\
	if (a != NULL)\
{\
	delete a;\
	a = NULL;\
}\

std::map<std::string, std::string> m_mapCodeDesc;

// CCardProcess

STDMETHODIMP CCardProcess::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{     
		&IID_ICardProcess
	};

	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


void SaveXML(char *xml, int len)
{
	FILE *fp;
	fp = fopen("c:\\ATLCreate.xml", "wb");
	if (fp != NULL)
	{
		fwrite(xml, len, sizeof(char), fp);
	}
	fclose(fp);
}

void CCardProcess::CreateResponXML(int nID, const char *szResult, char *RetXML)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlElement *Segment;
	TiXmlDeclaration HeadDec;
	TiXmlPrinter Printer;

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("CardProcess");
	Segment = new TiXmlElement("ReturnInfo");
	Segment->SetAttribute("ID", nID);
	Segment->SetAttribute("Desc", szResult);

	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(RetXML, Printer.CStr());

}

void CCardProcess::GetErrInfo(int nProcRet, LONG &nRet)
{
	if (nProcRet != 0)
	{
		//CreateResponXML(-1, (const char*)err(nProcRet), m_strErr);
		strcpy(m_strErr, err(nProcRet));
	}
	nRet = nProcRet;
}

char *CCardProcess::GetErrInfo(int nProcRet)
{
	if (nProcRet !=0 )
	{
		strcpy(m_strErr, err(nProcRet));
		return m_strErr;
	}
	return NULL;
}

STDMETHODIMP CCardProcess::iATLGetCardVersion(BSTR* szVersion)
{
	int ret = 0;
	char szTempVersion[100];
	ret = iGetCardVersion(szTempVersion);
	_bstr_t bstr(szTempVersion);
	*szVersion=bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLerr(BSTR* szError)
{
	// TODO: 在此添加实现代码
	_bstr_t bstr(m_strErr);
	*szError = bstr.Detach();
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLCardInit( LONG* nRet)
{
	// TODO: 在此添加实现代码
	int ret = iCardInit();
	GetErrInfo(ret, (*nRet));
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLCardClose(LONG* nRet)
{
	// TODO: 在此添加实现代码
	int ret = iCardClose();
	GetErrInfo(ret, (*nRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCardOpen(LONG* Ret)
{
	// TODO: 在此添加实现代码
	int ret = iCardOpen();
	GetErrInfo(ret, (*Ret));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCardDeinit(LONG* Ret)
{
	// TODO: 在此添加实现代码
	int ret = iCardDeinit();
	GetErrInfo(ret, (*Ret));
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLReadInfo(LONG nFlag, BSTR* szReadXML)
{
	// TODO: 在此添加实现代码
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	int ret = 0;
	ret = iReadInfo(nFlag, strXML);
	if (ret != 0)
	{
		memset(strXML, 0 ,sizeof(strXML));
		CreateResponXML(-1, GetErrInfo(ret), strXML);
	}
	_bstr_t bstr(strXML);
	*szReadXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLWriteInfo(BSTR szXML, LONG* nRet)
{
	// TODO: 在此添加实现代码
	_bstr_t bstr(szXML);
	char *strInXML = (char*)bstr;
	int ret = 0;
	ret = iWriteInfo(strInXML);
	GetErrInfo(ret, (*nRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLQueryInfo(BSTR szQuerySource, BSTR* szResult)
{
	// TODO: 在此添加实现代码
	LONG n;
	char strXML[8092];
	memset(strXML, 0, sizeof(strXML));
	_bstr_t bstr(szQuerySource, true);
	int ret = 0;
	char *strINXML = (char*)bstr;
	ret = iQueryInfo(strINXML, strXML);
	if (ret != 0)
	{
		strcpy(strXML, GetErrInfo(ret));
	}
	bstr = strXML;
	*szResult = bstr.Detach();
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLFormatCard(LONG* nRet)
{
	// TODO: 在此添加实现代码
	int ret = 0;
	ret = iFormatCard();
	GetErrInfo(ret, (*nRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCreateCard(BSTR szCardXML, LONG* nRet)
{
	// TODO: 在此添加实现代码
	_bstr_t bstr(szCardXML);
	char *strINXML = (char*)bstr;
	if (strINXML != NULL)
	{
		int ret = iCreateCard(strINXML);
		GetErrInfo(ret, (*nRet));
	}
	else
	{
		*nRet = -1;
	}

	return S_OK;
}

STDMETHODIMP CCardProcess::iATLPrintCard(BSTR pszPrinterType, BSTR pszCardCoverDataXml, 
										 BSTR pszCardCoverXml, LONG* nRet)
{
	// TODO: 在此添加实现代码
	_bstr_t Typebstr(pszPrinterType);
	_bstr_t CardDatabstr(pszCardCoverDataXml);
	_bstr_t CardCoverbstr(pszCardCoverXml);
	char *szType = (char*)Typebstr;
	char *szCardData = (char*)CardDatabstr;
	char *szCover = (char*)CardCoverbstr;
	if (szType != NULL && szCardData != NULL && szCover != NULL)
	{
		int ret = iPrintCard(szType, szCardData, szCover);
		GetErrInfo(ret, (*nRet));
	}
	else
	{
		*nRet = -1;
	}

	return S_OK;
}

STDMETHODIMP CCardProcess::iATLPatchCard(BSTR szCardXML, BSTR pszCardCoverDataXml, 
										 BSTR pszPrinterType, BSTR pszCardCoverXml, 
										 LONG* nRet)
{
	// TODO: 在此添加实现代码
	_bstr_t CreateXML(szCardXML);
	_bstr_t Typebstr(pszPrinterType);
	_bstr_t CardDatabstr(pszCardCoverDataXml);
	_bstr_t CardCoverbstr(pszCardCoverXml);
	char *strINXML = (char*)CreateXML;
	char *szType = (char*)Typebstr;
	char *szCardData = (char*)CardDatabstr;
	char *szCover = (char*)CardCoverbstr;
	if (szType != NULL && szCardData != NULL && szCover != NULL && strINXML != NULL)
	{
		int ret = iPatchCard(strINXML, szCardData, szType, szCover);
		GetErrInfo(ret, (*nRet));
	}
	else
	{
		*nRet = -7;
	}
	
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLCreateCardData(BSTR szCreateData, BSTR szLicense)
{
	// TODO: 在此添加实现代码

	return S_OK;
}

STDMETHODIMP CCardProcess::iATLScanCard(LONG* nRet)
{
	// TODO: 在此添加实现代码
	int Ret = iScanCard();
	GetErrInfo(Ret, (*nRet));
	return S_OK;
}



STDMETHODIMP CCardProcess::iATLReadCardMessageForNH(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL, BSTR* pszXml)
{
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardRewritePackageWSDL);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	int ret = iReadCardMessageForNH(strCheckWSDL, strRewriteWSDL, strResult);
	_bstr_t bstr = strResult;
	*pszXml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadHISInfo(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardRewritePackageWSDL);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	int ret = 0;
	ret = iReadHISInfo(strCheckWSDL, strRewriteWSDL, strXML);
	_bstr_t bstr(strXML);
	*szXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadOnlyHIS(BSTR* bstrHISInfo)
{
	// TODO: 在此添加实现代码
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	int ret = 0;
	ret = iReadOnlyHIS(strXML);
	_bstr_t bstr(strXML);
	*bstrHISInfo = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadInfoForXJ(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardRewritePackageWSDL);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	int ret = 0;
	ret = iReadInfoForXJ(strCheckWSDL, strRewriteWSDL, strXML);
	_bstr_t bstr(strXML);
	*szXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCardIsEmpty(LONG* bEmpty)
{
	// TODO: 在此添加实现代码
	int ret = iCardIsEmpty();
	GetErrInfo(ret, (*bEmpty));

	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCheckMsgForNH(BSTR bstrCheckWSDL, BSTR bstrServerURL,  BSTR* strCheckRet)
{
	// TODO: 在此添加实现代码
	_bstr_t _bstrCheckWSDL(bstrCheckWSDL);  
	_bstr_t bstrRewriteWSDL(bstrServerURL);
	char* strCheckWSDL = (char*)_bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	iCheckMsgForNH(strCheckWSDL, strRewriteWSDL, szRead);
	_bstr_t bstr(szRead);
	*strCheckRet = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadConfigMsg(BSTR bstrConfigInfo, BSTR* bstrReadXML)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(bstrConfigInfo);  
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	iReadConfigMsg(strCheckWSDL, strXML);
	_bstr_t bstr(strXML);
	*bstrReadXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLRegMsgForNH(BSTR bstrServerURL, BSTR* bstrReadXML)
{
	// TODO: 在此添加实现代码 
	_bstr_t bstrRewriteWSDL(bstrServerURL);
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	char strXML[8092];
	memset(strXML, 0,sizeof(strXML));
	iRegMsgForNH(strRewriteWSDL, strXML);
	_bstr_t bstr(strXML);
	*bstrReadXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLEncryFile(BSTR bstrfilename, LONG* nProCode)
{
	// TODO: 在此添加实现代码
	_bstr_t bstr(bstrfilename);
	char *strfilename = (char*)bstr;
	*nProCode = iEncryFile(strfilename);
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLGetPrinterList(BSTR* bstrPrinterXML)
{
	// TODO: 在此添加实现代码
	char strXML[2048];
	memset(strXML, 0,sizeof(strXML));
	iGetPrinterList(strXML);
	_bstr_t bstr(strXML);
	*bstrPrinterXML = bstr.Detach();
	return S_OK;
}




