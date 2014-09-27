// CardProcess.cpp : CCardProcess 的实现

#include "stdafx.h"   
#include "CardProcess.h"
#include "../BHGX_HISReader/BHGX_HISReader.h"
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "../BHGX_HospitalProcess/BHGX_HospitalProcess.h"
#include "tinyxml/headers/tinyxml.h"
#include <comutil.h>
#include <comdef.h>
#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#pragma comment(lib, "../debug/BHGX_HISReader.lib")
#pragma comment(lib, "../debug/BHGX_HospitalProcess.lib")
#else
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#pragma comment(lib, "../release/BHGX_HISReader.lib")
#pragma comment(lib, "../release/BHGX_HospitalProcess.lib")
#endif

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#pragma comment(lib, "comsupp.lib")    

#pragma warning (disable : 4996)

#define SAFE_DELETE(a)		\
	if (a != NULL)			\
{							\
	delete a;				\
	a = NULL;				\
}							\


#define BUFFSIZE	8096
char g_ReadBuff[BUFFSIZE];

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
	if (nProcRet != 0){
		strcpy(m_strErr, err(nProcRet));
	}
	nRet = nProcRet;
}

char *CCardProcess::GetErrInfo(int nProcRet)
{
	if (nProcRet !=0 ){
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
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadInfo(nFlag, g_ReadBuff);
	if (ret != 0){
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
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
	memset(g_ReadBuff, 0, sizeof(g_ReadBuff));
	_bstr_t bstr(szQuerySource, true);
	int ret = 0;
	char *strINXML = (char*)bstr;
	ret = iQueryInfo(strINXML, g_ReadBuff);
	if (ret != 0){
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	bstr = g_ReadBuff;
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
	if (strINXML != NULL){
		int ret = iCreateCard(strINXML);
		GetErrInfo(ret, (*nRet));
	}else{
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
	if (szType != NULL && szCardData != NULL && szCover != NULL){
		int ret = iPrintCard(szType, szCardData, szCover);
		GetErrInfo(ret, (*nRet));
	}else{
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
	if (szType != NULL && szCardData != NULL && 
		szCover != NULL && strINXML != NULL){
		int ret = iPatchCard(strINXML, szCardData, szType, szCover);
		GetErrInfo(ret, (*nRet));
	}else{
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
	int ret = iReadCardMessageForNH(strCheckWSDL, strRewriteWSDL, g_ReadBuff);
	if (ret != 0) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr = g_ReadBuff;
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
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadHISInfo(strCheckWSDL, strRewriteWSDL, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*szXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadOnlyHIS(BSTR* bstrHISInfo)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadOnlyHIS(g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
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
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = iReadInfoForXJ(strCheckWSDL, strRewriteWSDL, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
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
	memset(g_ReadBuff, 0, sizeof(g_ReadBuff));
	iCheckMsgForNH(strCheckWSDL, strRewriteWSDL, g_ReadBuff);
	_bstr_t bstr(g_ReadBuff);
	*strCheckRet = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadConfigMsg(BSTR bstrConfigInfo, BSTR* bstrReadXML)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(bstrConfigInfo);  
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = iReadConfigMsg(strCheckWSDL, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*bstrReadXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLRegMsgForNH(BSTR bstrServerURL, BSTR* bstrReadXML)
{
	// TODO: 在此添加实现代码 
	_bstr_t bstrRewriteWSDL(bstrServerURL);
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = iRegMsgForNH(strRewriteWSDL, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
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
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	iGetPrinterList(g_ReadBuff);
	_bstr_t bstr(g_ReadBuff);
	*bstrPrinterXML = bstr.Detach();
	return S_OK;
}


#define LICENSEFILE	"北航冠新.license"

STDMETHODIMP CCardProcess::iATLCreateLicense(BSTR timeFMT, LONG* ret)
{
	// TODO: 在此添加实现代码
	_bstr_t bstr(timeFMT);
	char *strTime = (char*)bstr;
	int nRet = iCreateLicense(LICENSEFILE, strTime);
	GetErrInfo(nRet, (*ret));
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLFormatHospInfo(LONG* pRet)
{
	int ret = 0;
	ret = iFormatCard();
	GetErrInfo(ret, (*pRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLWriteHospInfo(BSTR xml, LONG* pRet)
{
	_bstr_t bstr(xml);
	char *strInXML = (char*)bstr;
	int ret = iWriteHospInfo(strInXML);
	GetErrInfo(ret, (*pRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadClinicInfo(BSTR pszCode, BSTR* readXML)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	int ret = iReadClinicInfo((char*)bszCode, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*readXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadMedicalInfo(BSTR pszCode, BSTR* readXML)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	int ret = iReadMedicalInfo((char*)bszCode, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*readXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadFeeInfo(BSTR pszCode, BSTR* readXML)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	int ret = iReadFeeInfo((char*)bszCode, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*readXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadOnlyHISLog(BSTR logConfXml, BSTR* bstrHISInfo)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	_bstr_t bszLogConfXml(logConfXml);
	ret = iReadOnlyHISLog((char*)bszLogConfXml, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*bstrHISInfo = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadInfoForXJLog(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardServerURL);
	_bstr_t bsLog(pszLogXml);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadInfoForXJLog(strCheckWSDL, strRewriteWSDL, (char*)bsLog, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadHISInfoLog(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardServerURL);
	_bstr_t bsLog(pszLogXml);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadHISInfoLog(strCheckWSDL, strRewriteWSDL,(char*)bsLog, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}



STDMETHODIMP CCardProcess::iATLReadMedicalInfoLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadMedicalInfoLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadFeeInfoLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadFeeInfoLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLWriteHospInfoLog(BSTR xml, BSTR pszLogXml, LONG* pRet)
{
	_bstr_t bstr(xml);
	_bstr_t bsLog(pszLogXml);
	char *strInXML = (char*)bstr;
	int ret = iWriteHospInfoLog(strInXML, (char*)bsLog);
	GetErrInfo(ret, (*pRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadClinicInfoLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadClinicInfoLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadOnlyHISLocal(BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	_bstr_t bszLogConfXml(pszLogXml);
	ret = iReadOnlyHISLocal((char*)bszLogConfXml, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadInfoForXJLocal(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardServerURL);
	_bstr_t bsLog(pszLogXml);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadInfoForXJLocal((char*)bsLog, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadHISInfoLocal(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml)
{
	// TODO: 在此添加实现代码
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardServerURL);
	_bstr_t bsLog(pszLogXml);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	int ret = 0;
	ret = iReadHISInfoLocal((char*)bsLog, g_ReadBuff);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadMedicalInfoLocal(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadMedicalInfoLocal((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadFeeInfoLocal(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadFeeInfoLocal((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadClinicInfoLocal(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	memset(g_ReadBuff, 0,sizeof(g_ReadBuff));
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadClinicInfoLocal((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		memset(g_ReadBuff, 0 ,sizeof(g_ReadBuff));
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLWriteHospInfoLocal(BSTR xml, BSTR pszLogXml, LONG* pRet)
{
	_bstr_t bstr(xml);
	_bstr_t bsLog(pszLogXml);
	char *strInXML = (char*)bstr;
	int ret = iWriteHospInfoLocal(strInXML, (char*)bsLog);
	GetErrInfo(ret, (*pRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLWriteHospInfoOnlyLog(BSTR xml, BSTR pszLogXml, LONG* pRet)
{
	_bstr_t bstr(xml);
	_bstr_t bsLog(pszLogXml);
	char *strInXML = (char*)bstr;
	int ret = iWriteHospInfoOnlyLog(strInXML, (char*)bsLog);
	GetErrInfo(ret, (*pRet));
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadClinicInfoOnlyLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadClinicInfoOnlyLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadFeeInfoOnlyLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadFeeInfoOnlyLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;;
}

STDMETHODIMP CCardProcess::iATLReadMedicalInfoOnlyLog(BSTR pszCode, BSTR pszLogXml, BSTR* xml)
{
	_bstr_t bszCode(pszCode);
	_bstr_t bsLog(pszLogXml);
	int ret = iReadMedicalInfoOnlyLog((char*)bszCode, g_ReadBuff, (char*)bsLog);
	if (ret) {;
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*xml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLRegMsgForNHLog(BSTR bstrServerURL, BSTR pszLogXml, BSTR* bstrReadXML)
{
	_bstr_t bstrRewriteWSDL(bstrServerURL);
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	_bstr_t bsLog(pszLogXml);
	int ret = iRegMsgForNHLog(strRewriteWSDL,(char*)bsLog, g_ReadBuff);
	if (ret) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*bstrReadXML = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadCardMessageForNHLog(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL, BSTR pszLogXml, BSTR* pszXml)
{
	_bstr_t bstrCheckWSDL(pszCardCheckWSDL);  
	_bstr_t bstrRewriteWSDL(pszCardRewritePackageWSDL);
	_bstr_t bsLog(pszLogXml);
	char* strCheckWSDL = (char*)bstrCheckWSDL;
	char* strRewriteWSDL = (char*)bstrRewriteWSDL;
	int ret = iReadCardMessageForNHLog(strCheckWSDL, strRewriteWSDL, (char*)bsLog, g_ReadBuff);
	if (ret != 0) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr = g_ReadBuff;
	*pszXml = bstr.Detach();
	return S_OK;
}


STDMETHODIMP CCardProcess::iATLReadCardMessageForNHLocal(BSTR pszLogXml, BSTR* pszXml)
{
	_bstr_t bsLog(pszLogXml);
	int ret = iReadCardMessageForNHLocal( (char*)bsLog, g_ReadBuff);
	if (ret != 0) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr = g_ReadBuff;
	*pszXml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLCheckMsgForNHLocal(BSTR pszLogXml, BSTR* pszXml)
{
	_bstr_t bsLog(pszLogXml);
	int ret = iCheckMsgForNHLocal((char*)bsLog, g_ReadBuff);
	if (ret != 0) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*pszXml = bstr.Detach();
	return S_OK;
}

STDMETHODIMP CCardProcess::iATLReadOnlyCardMessageForNH(BSTR pszLogXml, BSTR* pszXml)
{
	_bstr_t bsLog(pszLogXml);
	int ret = iReadOnlyCardMessageForNH((char*)bsLog, g_ReadBuff);
	if (ret != 0) {
		CreateResponXML(-1, GetErrInfo(ret), g_ReadBuff);
	}
	_bstr_t bstr(g_ReadBuff);
	*pszXml = bstr.Detach();
	return S_OK;
}
