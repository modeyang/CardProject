

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Wed Jan 04 13:46:25 2012
 */
/* Compiler settings for .\BHGX_CardActiveX.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __BHGX_CardActiveX_h__
#define __BHGX_CardActiveX_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICardProcess_FWD_DEFINED__
#define __ICardProcess_FWD_DEFINED__
typedef interface ICardProcess ICardProcess;
#endif 	/* __ICardProcess_FWD_DEFINED__ */


#ifndef __CardProcess_FWD_DEFINED__
#define __CardProcess_FWD_DEFINED__

#ifdef __cplusplus
typedef class CardProcess CardProcess;
#else
typedef struct CardProcess CardProcess;
#endif /* __cplusplus */

#endif 	/* __CardProcess_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICardProcess_INTERFACE_DEFINED__
#define __ICardProcess_INTERFACE_DEFINED__

/* interface ICardProcess */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICardProcess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02D30C94-A1B9-4663-99A9-C9F856ED3137")
    ICardProcess : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLGetCardVersion( 
            /* [retval][out] */ BSTR *szVersion) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCardInit( 
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadInfo( 
            LONG nFlag,
            /* [retval][out] */ BSTR *szReadXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLWriteInfo( 
            BSTR szXML,
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLQueryInfo( 
            BSTR szQuerySource,
            /* [retval][out] */ BSTR *szResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLerr( 
            /* [retval][out] */ BSTR *szError) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLFormatCard( 
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCreateCard( 
            BSTR szCardXML,
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLPrintCard( 
            BSTR pszPrinterType,
            BSTR pszCardCoverDataXml,
            BSTR pszCardCoverXml,
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLPatchCard( 
            BSTR szCardXML,
            BSTR pszCardCoverDataXml,
            BSTR pszPrinterType,
            BSTR pszCardCoverXml,
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCreateCardData( 
            BSTR szCreateData,
            BSTR szLicense) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLScanCard( 
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCardClose( 
            /* [retval][out] */ LONG *nRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadCardMessageForNH( 
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *pszXml) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadHISInfo( 
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *szXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadInfoForXJ( 
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *szXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCardIsEmpty( 
            /* [retval][out] */ LONG *bEmpty) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLCheckMsgForNH( 
            BSTR bstrCheckWSDL,
            BSTR bstrServerURL,
            /* [retval][out] */ BSTR *strCheckRet) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadConfigMsg( 
            BSTR bstrConfigInfo,
            /* [retval][out] */ BSTR *bstrReadXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLRegMsgForNH( 
            BSTR bstrServerURL,
            /* [retval][out] */ BSTR *bstrReadXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLEncryFile( 
            BSTR bstrfilename,
            /* [retval][out] */ LONG *nProCode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLGetPrinterList( 
            /* [retval][out] */ BSTR *bstrPrinterXML) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE iATLReadOnlyHIS( 
            /* [retval][out] */ BSTR *bstrHISInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICardProcessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICardProcess * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICardProcess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICardProcess * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICardProcess * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICardProcess * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICardProcess * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICardProcess * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLGetCardVersion )( 
            ICardProcess * This,
            /* [retval][out] */ BSTR *szVersion);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCardInit )( 
            ICardProcess * This,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadInfo )( 
            ICardProcess * This,
            LONG nFlag,
            /* [retval][out] */ BSTR *szReadXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLWriteInfo )( 
            ICardProcess * This,
            BSTR szXML,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLQueryInfo )( 
            ICardProcess * This,
            BSTR szQuerySource,
            /* [retval][out] */ BSTR *szResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLerr )( 
            ICardProcess * This,
            /* [retval][out] */ BSTR *szError);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLFormatCard )( 
            ICardProcess * This,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCreateCard )( 
            ICardProcess * This,
            BSTR szCardXML,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLPrintCard )( 
            ICardProcess * This,
            BSTR pszPrinterType,
            BSTR pszCardCoverDataXml,
            BSTR pszCardCoverXml,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLPatchCard )( 
            ICardProcess * This,
            BSTR szCardXML,
            BSTR pszCardCoverDataXml,
            BSTR pszPrinterType,
            BSTR pszCardCoverXml,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCreateCardData )( 
            ICardProcess * This,
            BSTR szCreateData,
            BSTR szLicense);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLScanCard )( 
            ICardProcess * This,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCardClose )( 
            ICardProcess * This,
            /* [retval][out] */ LONG *nRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadCardMessageForNH )( 
            ICardProcess * This,
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *pszXml);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadHISInfo )( 
            ICardProcess * This,
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *szXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadInfoForXJ )( 
            ICardProcess * This,
            BSTR pszCardCheckXML,
            BSTR pszCardRewritePackageXML,
            /* [retval][out] */ BSTR *szXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCardIsEmpty )( 
            ICardProcess * This,
            /* [retval][out] */ LONG *bEmpty);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLCheckMsgForNH )( 
            ICardProcess * This,
            BSTR bstrCheckWSDL,
            BSTR bstrServerURL,
            /* [retval][out] */ BSTR *strCheckRet);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadConfigMsg )( 
            ICardProcess * This,
            BSTR bstrConfigInfo,
            /* [retval][out] */ BSTR *bstrReadXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLRegMsgForNH )( 
            ICardProcess * This,
            BSTR bstrServerURL,
            /* [retval][out] */ BSTR *bstrReadXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLEncryFile )( 
            ICardProcess * This,
            BSTR bstrfilename,
            /* [retval][out] */ LONG *nProCode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLGetPrinterList )( 
            ICardProcess * This,
            /* [retval][out] */ BSTR *bstrPrinterXML);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *iATLReadOnlyHIS )( 
            ICardProcess * This,
            /* [retval][out] */ BSTR *bstrHISInfo);
        
        END_INTERFACE
    } ICardProcessVtbl;

    interface ICardProcess
    {
        CONST_VTBL struct ICardProcessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICardProcess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICardProcess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICardProcess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICardProcess_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICardProcess_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICardProcess_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICardProcess_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICardProcess_iATLGetCardVersion(This,szVersion)	\
    (This)->lpVtbl -> iATLGetCardVersion(This,szVersion)

#define ICardProcess_iATLCardInit(This,nRet)	\
    (This)->lpVtbl -> iATLCardInit(This,nRet)

#define ICardProcess_iATLReadInfo(This,nFlag,szReadXML)	\
    (This)->lpVtbl -> iATLReadInfo(This,nFlag,szReadXML)

#define ICardProcess_iATLWriteInfo(This,szXML,nRet)	\
    (This)->lpVtbl -> iATLWriteInfo(This,szXML,nRet)

#define ICardProcess_iATLQueryInfo(This,szQuerySource,szResult)	\
    (This)->lpVtbl -> iATLQueryInfo(This,szQuerySource,szResult)

#define ICardProcess_iATLerr(This,szError)	\
    (This)->lpVtbl -> iATLerr(This,szError)

#define ICardProcess_iATLFormatCard(This,nRet)	\
    (This)->lpVtbl -> iATLFormatCard(This,nRet)

#define ICardProcess_iATLCreateCard(This,szCardXML,nRet)	\
    (This)->lpVtbl -> iATLCreateCard(This,szCardXML,nRet)

#define ICardProcess_iATLPrintCard(This,pszPrinterType,pszCardCoverDataXml,pszCardCoverXml,nRet)	\
    (This)->lpVtbl -> iATLPrintCard(This,pszPrinterType,pszCardCoverDataXml,pszCardCoverXml,nRet)

#define ICardProcess_iATLPatchCard(This,szCardXML,pszCardCoverDataXml,pszPrinterType,pszCardCoverXml,nRet)	\
    (This)->lpVtbl -> iATLPatchCard(This,szCardXML,pszCardCoverDataXml,pszPrinterType,pszCardCoverXml,nRet)

#define ICardProcess_iATLCreateCardData(This,szCreateData,szLicense)	\
    (This)->lpVtbl -> iATLCreateCardData(This,szCreateData,szLicense)

#define ICardProcess_iATLScanCard(This,nRet)	\
    (This)->lpVtbl -> iATLScanCard(This,nRet)

#define ICardProcess_iATLCardClose(This,nRet)	\
    (This)->lpVtbl -> iATLCardClose(This,nRet)

#define ICardProcess_iATLReadCardMessageForNH(This,pszCardCheckXML,pszCardRewritePackageXML,pszXml)	\
    (This)->lpVtbl -> iATLReadCardMessageForNH(This,pszCardCheckXML,pszCardRewritePackageXML,pszXml)

#define ICardProcess_iATLReadHISInfo(This,pszCardCheckXML,pszCardRewritePackageXML,szXML)	\
    (This)->lpVtbl -> iATLReadHISInfo(This,pszCardCheckXML,pszCardRewritePackageXML,szXML)

#define ICardProcess_iATLReadInfoForXJ(This,pszCardCheckXML,pszCardRewritePackageXML,szXML)	\
    (This)->lpVtbl -> iATLReadInfoForXJ(This,pszCardCheckXML,pszCardRewritePackageXML,szXML)

#define ICardProcess_iATLCardIsEmpty(This,bEmpty)	\
    (This)->lpVtbl -> iATLCardIsEmpty(This,bEmpty)

#define ICardProcess_iATLCheckMsgForNH(This,bstrCheckWSDL,bstrServerURL,strCheckRet)	\
    (This)->lpVtbl -> iATLCheckMsgForNH(This,bstrCheckWSDL,bstrServerURL,strCheckRet)

#define ICardProcess_iATLReadConfigMsg(This,bstrConfigInfo,bstrReadXML)	\
    (This)->lpVtbl -> iATLReadConfigMsg(This,bstrConfigInfo,bstrReadXML)

#define ICardProcess_iATLRegMsgForNH(This,bstrServerURL,bstrReadXML)	\
    (This)->lpVtbl -> iATLRegMsgForNH(This,bstrServerURL,bstrReadXML)

#define ICardProcess_iATLEncryFile(This,bstrfilename,nProCode)	\
    (This)->lpVtbl -> iATLEncryFile(This,bstrfilename,nProCode)

#define ICardProcess_iATLGetPrinterList(This,bstrPrinterXML)	\
    (This)->lpVtbl -> iATLGetPrinterList(This,bstrPrinterXML)

#define ICardProcess_iATLReadOnlyHIS(This,bstrHISInfo)	\
    (This)->lpVtbl -> iATLReadOnlyHIS(This,bstrHISInfo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLGetCardVersion_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ BSTR *szVersion);


void __RPC_STUB ICardProcess_iATLGetCardVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCardInit_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLCardInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadInfo_Proxy( 
    ICardProcess * This,
    LONG nFlag,
    /* [retval][out] */ BSTR *szReadXML);


void __RPC_STUB ICardProcess_iATLReadInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLWriteInfo_Proxy( 
    ICardProcess * This,
    BSTR szXML,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLWriteInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLQueryInfo_Proxy( 
    ICardProcess * This,
    BSTR szQuerySource,
    /* [retval][out] */ BSTR *szResult);


void __RPC_STUB ICardProcess_iATLQueryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLerr_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ BSTR *szError);


void __RPC_STUB ICardProcess_iATLerr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLFormatCard_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLFormatCard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCreateCard_Proxy( 
    ICardProcess * This,
    BSTR szCardXML,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLCreateCard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLPrintCard_Proxy( 
    ICardProcess * This,
    BSTR pszPrinterType,
    BSTR pszCardCoverDataXml,
    BSTR pszCardCoverXml,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLPrintCard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLPatchCard_Proxy( 
    ICardProcess * This,
    BSTR szCardXML,
    BSTR pszCardCoverDataXml,
    BSTR pszPrinterType,
    BSTR pszCardCoverXml,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLPatchCard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCreateCardData_Proxy( 
    ICardProcess * This,
    BSTR szCreateData,
    BSTR szLicense);


void __RPC_STUB ICardProcess_iATLCreateCardData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLScanCard_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLScanCard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCardClose_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ LONG *nRet);


void __RPC_STUB ICardProcess_iATLCardClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadCardMessageForNH_Proxy( 
    ICardProcess * This,
    BSTR pszCardCheckXML,
    BSTR pszCardRewritePackageXML,
    /* [retval][out] */ BSTR *pszXml);


void __RPC_STUB ICardProcess_iATLReadCardMessageForNH_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadHISInfo_Proxy( 
    ICardProcess * This,
    BSTR pszCardCheckXML,
    BSTR pszCardRewritePackageXML,
    /* [retval][out] */ BSTR *szXML);


void __RPC_STUB ICardProcess_iATLReadHISInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadInfoForXJ_Proxy( 
    ICardProcess * This,
    BSTR pszCardCheckXML,
    BSTR pszCardRewritePackageXML,
    /* [retval][out] */ BSTR *szXML);


void __RPC_STUB ICardProcess_iATLReadInfoForXJ_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCardIsEmpty_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ LONG *bEmpty);


void __RPC_STUB ICardProcess_iATLCardIsEmpty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLCheckMsgForNH_Proxy( 
    ICardProcess * This,
    BSTR bstrCheckWSDL,
    BSTR bstrServerURL,
    /* [retval][out] */ BSTR *strCheckRet);


void __RPC_STUB ICardProcess_iATLCheckMsgForNH_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadConfigMsg_Proxy( 
    ICardProcess * This,
    BSTR bstrConfigInfo,
    /* [retval][out] */ BSTR *bstrReadXML);


void __RPC_STUB ICardProcess_iATLReadConfigMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLRegMsgForNH_Proxy( 
    ICardProcess * This,
    BSTR bstrServerURL,
    /* [retval][out] */ BSTR *bstrReadXML);


void __RPC_STUB ICardProcess_iATLRegMsgForNH_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLEncryFile_Proxy( 
    ICardProcess * This,
    BSTR bstrfilename,
    /* [retval][out] */ LONG *nProCode);


void __RPC_STUB ICardProcess_iATLEncryFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLGetPrinterList_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ BSTR *bstrPrinterXML);


void __RPC_STUB ICardProcess_iATLGetPrinterList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICardProcess_iATLReadOnlyHIS_Proxy( 
    ICardProcess * This,
    /* [retval][out] */ BSTR *bstrHISInfo);


void __RPC_STUB ICardProcess_iATLReadOnlyHIS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICardProcess_INTERFACE_DEFINED__ */



#ifndef __BHGX_CardActiveXLib_LIBRARY_DEFINED__
#define __BHGX_CardActiveXLib_LIBRARY_DEFINED__

/* library BHGX_CardActiveXLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_BHGX_CardActiveXLib;

EXTERN_C const CLSID CLSID_CardProcess;

#ifdef __cplusplus

class DECLSPEC_UUID("8610611D-8094-461B-B83A-881CCB340D3E")
CardProcess;
#endif
#endif /* __BHGX_CardActiveXLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


