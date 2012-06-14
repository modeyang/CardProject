#include <string>
#include <windows.h>

#if !defined(_SQ_TEXTCONVERT_H_INCLUDED_)
#define _SQ_TEXTCONVERT_H_INCLUDED_

//宽字节字符串转换为多字节字符串
// CodePage:[in]编码
// lpcwszText:[in]宽字节字符串
// strVal:[out]多字节字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WcharToChar(UINT CodePage,const wchar_t *lpcwszText,std::string &strVal)
{
	CHAR *lpszVal( NULL );
	int nLength(0);
	size_t dwSize(0);

	nLength = ::WideCharToMultiByte( CodePage,0,lpcwszText,-1,NULL,0,NULL,NULL);
	if( nLength <= 0 )
		return FALSE;

	dwSize = nLength * sizeof(CHAR);
	lpszVal = (CHAR *)malloc( dwSize );
	if( NULL == lpszVal )
		return FALSE;

	memset( lpszVal,0x0,dwSize);
	nLength = WideCharToMultiByte( CodePage, 0, lpcwszText,-1,lpszVal, nLength, NULL, NULL);
	if( nLength <= 0 )
	{
		free(lpszVal);
		lpszVal = NULL;
		return FALSE;
	}

	strVal.assign(lpszVal);
	free(lpszVal);
	lpszVal = NULL;
	return TRUE;
}

//多字节字符串转换为宽字节字符串
// CodePage:[in]编码
// lpcwszText:[in]多字节字符串
// lppszVal:[out]宽字节字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WcharFromChar(UINT CodePage,const char *lpcszText,std::wstring &wstrVal)
{
	WCHAR *lpwszVal( NULL );
	int nLength(0);
	size_t dwSize(0);
	nLength = ::MultiByteToWideChar( CodePage, 0, lpcszText, -1 , NULL, 0); 
	if( nLength <= 0 )
		return FALSE;

	dwSize = nLength * sizeof(WCHAR);
	lpwszVal = (WCHAR *)malloc( dwSize );
	if( NULL == lpwszVal )
		return FALSE;

	memset(lpwszVal,0x0,dwSize);
	nLength = ::MultiByteToWideChar( CodePage, 0, lpcszText, -1 , lpwszVal, nLength ); 
	if( nLength <= 0 )
	{
		free(lpwszVal);
		lpwszVal = NULL;
		return FALSE;
	}
	wstrVal.assign( lpwszVal );
	free(lpwszVal);
	lpwszVal = NULL;
	return TRUE;
}

//宽字节字符串转换为多字节字符串
// lpcwszText:[in]宽字节字符串
// strVal:[out]多字节字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WToA(const wchar_t *lpcwszText,std::string &strVal)
{
	return WcharToChar(CP_ACP,lpcwszText,strVal);
}

//多字节字符串转换为宽字节字符串
// lpcwszText:[in]多字节字符串
// lppszVal:[out]宽字节字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WFromA(const char *lpcszText,std::wstring &wstrVal )
{
	return WcharFromChar(CP_ACP,lpcszText,wstrVal);
}

//宽字节字符串转换为UTF-8字符串
// lpcwszText:[in]宽字节字符串
// strVal:[out]UTF-8字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WToUTF8(const wchar_t *lpcwszText,std::string &strVal)
{
	return WcharToChar(CP_UTF8,lpcwszText,strVal);
}


//UTF-8字符串转换为宽字节字符串
// lpcwszText:[in]UTF-8字符串
// lppszVal:[out]宽字节字符串
//返回值:TRUE表示成功,其它表示失败
inline BOOL WFromUTF8(const char *lpcszText,std::wstring &wstrVal )
{
	return WcharFromChar(CP_UTF8,lpcszText,wstrVal);
}