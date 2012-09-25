// CardProcess.h : CCardProcess 的声明

#pragma once
#include "resource.h"       // 主符号

#include "BHGX_CardActiveX.h"
#include <string>
#include <map>
using namespace std;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif



// CCardProcess

class ATL_NO_VTABLE CCardProcess :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCardProcess, &CLSID_CardProcess>,
	public ISupportErrorInfo,
	public IObjectWithSiteImpl<CCardProcess>,
	public IDispatchImpl<ICardProcess, &IID_ICardProcess, &LIBID_BHGX_CardActiveXLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCardProcess()
	{
		strcpy_s(m_strErr, sizeof(m_strErr), "卡操作成功");
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CARDPROCESS)


BEGIN_COM_MAP(CCardProcess)
	COM_INTERFACE_ENTRY(ICardProcess)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
public:
	STDMETHOD(iATLGetCardVersion)(BSTR* szVersion);
	STDMETHOD(iATLCardInit)(LONG* nRet);
	STDMETHOD(iATLReadInfo)(LONG nFlag, BSTR* szReadXML);
	STDMETHOD(iATLWriteInfo)(BSTR szXML, LONG* nRet);
	STDMETHOD(iATLQueryInfo)(BSTR szQuerySource, BSTR* szResult);
	STDMETHOD(iATLerr)(BSTR* szError);
	STDMETHOD(iATLFormatCard)(LONG* nRet);
	STDMETHOD(iATLCreateCard)(BSTR szCardXML, LONG* nRet);
	STDMETHOD(iATLPrintCard)(BSTR pszPrinterType, BSTR pszCardCoverDataXml, BSTR pszCardCoverXml, LONG* nRet);
	STDMETHOD(iATLPatchCard)(BSTR szCardXML, BSTR pszCardCoverDataXml, BSTR pszPrinterType, BSTR pszCardCoverXml, LONG* nRet);
	STDMETHOD(iATLCreateCardData)(BSTR szCreateData, BSTR szLicense);
	STDMETHOD(iATLScanCard)(LONG* nRet);
	STDMETHOD(iATLCardClose)(LONG* nRet);
	STDMETHOD(iATLReadCardMessageForNH)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL, BSTR* pszXml);
	STDMETHOD(iATLReadHISInfo)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML);
	STDMETHOD(iATLReadInfoForXJ)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML);
	STDMETHOD(iATLCardIsEmpty)(LONG* bEmpty);
	STDMETHOD(iATLCheckMsgForNH)(BSTR bstrCheckWSDL, BSTR bstrServerURL, BSTR* strCheckRet);
	STDMETHOD(iATLReadConfigMsg)(BSTR bstrConfigInfo, BSTR* bstrReadXML);
	STDMETHOD(iATLRegMsgForNH)(BSTR bstrServerURL, BSTR* bstrReadXML);
	STDMETHOD(iATLEncryFile)(BSTR bstrfilename, LONG* nProCode);
	STDMETHOD(iATLGetPrinterList)(BSTR* bstrPrinterXML);
	STDMETHOD(iATLReadOnlyHIS)(BSTR* bstrHISInfo);
	STDMETHOD(iATLCardOpen)(LONG* Ret);
	STDMETHOD(iATLCardDeinit)(LONG* Ret);

protected:
	void GetErrInfo(int nProcRet, LONG &nRet);

	char *GetErrInfo(int nProcRet);

	void CreateResponXML(int nID, const char *szResult, char *RetXML);

protected:
	char m_strErr[1024];
public:
	STDMETHOD(iATLCreateLicense)(BSTR timeFMT, LONG* ret);
};

OBJECT_ENTRY_AUTO(__uuidof(CardProcess), CCardProcess)
