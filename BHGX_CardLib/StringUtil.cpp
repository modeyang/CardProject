#include "StringUtil.h"
#include <stdio.h>
#include <Windows.h>


typedef basic_string<char>::size_type S_T;
static const S_T npos = -1;

/**
*
*/
void trim(string & str)
{
	string::size_type pos = str.find_last_not_of(' ');
	if(pos != string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}


//delimit为一个字符，严格分割
vector<string> split(const string& src, string delimit, string null_subst)  
{  
	vector<string> v; 
	if( src.empty() || delimit.empty() ) 
		return v;  

	S_T deli_len = delimit.size();  
	long index = npos, last_search_position = 0;  
	while( (index=(long)src.find(delimit,last_search_position))!=npos )  
	{  

		if(index==last_search_position)  
			v.push_back(null_subst);  
		else 
		{
			string tmp = src.substr(last_search_position, index-last_search_position);
			trim(tmp);
			v.push_back(tmp);
		}

		last_search_position = (long)(index + deli_len);  
	}  
	string last_one = src.substr(last_search_position);  
	v.push_back( last_one.empty()? null_subst:last_one );  

	return v;  
}


//gb2312转utf8 
void Gb2312ToUtf8(char* pstrOut, int dwOutLen, const char* pstrIn, int dwInLen) 
{ 
#ifdef WIN32 
	int i = MultiByteToWideChar(CP_ACP, 0, pstrIn, -1, NULL, 0); 
	wchar_t * strSrc = new wchar_t[i+1]; 
	MultiByteToWideChar(CP_ACP, 0, pstrIn, -1, strSrc, i); 

	i = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, NULL, 0, NULL, NULL); 
	if (i >= dwOutLen) 
	{ 
		i = dwOutLen - 1; 
	} 
	WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, pstrOut, i, NULL, NULL); 
	delete strSrc; 
#else 
	iconv_t iConv = iconv_open("utf-8", "gb2312"); 
	iconv(iConv, (char **)&pstrIn, (size_t *)&dwInLen, (char **)&pstrOut, (size_t *)&dwOutLen); 
	iconv_close(iConv); 
#endif 
} 

//utf8转gb2312 
void Utf8ToGb2312(char* pstrOut, int dwOutLen, const char* pstrIn, int dwInLen) 
{ 
	if (NULL == pstrOut) 
	{ 
		return ; 
	} 
#ifdef WIN32 
	int i = MultiByteToWideChar(CP_UTF8, 0, pstrIn, -1, NULL, 0); 
	wchar_t * strSrc = new wchar_t[i+1]; 
	MultiByteToWideChar(CP_UTF8, 0, pstrIn, -1, strSrc, i); 

	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL); 
	if (i >= dwOutLen) 
	{ 
		i = dwOutLen - 1; 
	} 
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, pstrOut, i, NULL, NULL); 
	delete strSrc; 
#else 
	iconv_t iConv = iconv_open("gb2312", "utf-8"); 
	iconv(iConv, (char **)&pstrIn, (size_t *)&dwInLen, (char **)&pstrOut, (size_t *)&dwOutLen); 
	iconv_close(iConv); 
#endif 
}
//gb2312转Unicode 
void Gb2312ToUnicode(wchar_t* pstrOut, int dwOutLen, const char* pstrIn, int dwInLen) 
{ 
	if (NULL == pstrOut) 
	{ 
		return ; 
	} 
#ifdef WIN32 
	int i = MultiByteToWideChar(CP_ACP, 0, pstrIn, -1, NULL, 0); 
	if (i >= dwOutLen) 
	{ 
		i = dwOutLen - 1; 
	} 
	MultiByteToWideChar(CP_ACP, 0, pstrIn, -1, pstrOut, i); 
#else 
	iconv_t iConv = iconv_open("unicode", "gb2312"); 
	iconv(iConv, (char**)pstrIn, &dwInLen, (char**)pstrOut, &dwOutLen); 
	iconv_close(iConv); 
#endif 
} 

//Unicode转Gb2312 
void UnicodeToGb2312(char* pstrOut, int dwOutLen, const wchar_t* pstrIn, int dwInLen) 
{ 
	if (NULL == pstrOut) 
	{ 
		return ; 
	} 
#ifdef WIN32 
	int i = WideCharToMultiByte(CP_ACP, 0, pstrIn, -1, NULL, 0, NULL, NULL); 
	if (i >= dwOutLen) 
	{ 
		i = dwOutLen - 1; 
	} 
	WideCharToMultiByte(CP_ACP, 0, pstrIn, -1, pstrOut, i, 0, 0); 
#else 
	iconv_t iConv = iconv_open("gb2312", "unicode"); 
	iconv(iConv, (char**)pstrIn, &dwInLen, (char**)pstrOut, &dwOutLen); 
	iconv_close(iConv); 
#endif 
} 

//utf8转Unicode 
void Utf8ToUnicode(wchar_t* pstrOut, int dwOutLen, const char* pstrIn, int dwInLen) 
{ 
	if (NULL == pstrOut) 
	{ 
		return ; 
	} 
#ifdef WIN32 
	int i = MultiByteToWideChar(CP_UTF8, 0, pstrIn, -1, NULL, 0); 
	if (i >= dwOutLen) 
	{ 
		i = dwOutLen - 1; 
	} 
	MultiByteToWideChar(CP_UTF8, 0, pstrIn, -1, pstrOut, i); 
#else 
	iconv_t iConv = iconv_open("unicode", "utf-8"); 
	iconv(iConv, (char**)pstrIn, &dwInLen, (char**)pstrOut, &dwOutLen); 
	iconv_close(iConv); 
#endif 
}
