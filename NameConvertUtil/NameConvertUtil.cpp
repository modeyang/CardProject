// NameConvertUtil.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "NameConvertUtil.h"
#include "ConvertUtil.h"
#include <string>

using namespace std;

int __stdcall Name2Code(const char *strName, char *strCode, int *nLen)
{
	if (strCode == NULL)
	{
		return -1;
	}
	std::string szCode = CConvertUtil::uf_gbk_int_covers(strName, "toint");
	strcpy(strCode, szCode.c_str());
	*nLen = szCode.length();
	return 0;
}

int __stdcall Code2Name(const char *strCode, char *strName, int *nLen)
{
	if (strName == NULL)
	{
		return -1;
	}
	std::string szCode = CConvertUtil::uf_gbk_int_covers(strCode, "togbk");
	strcpy(strName, szCode.c_str());
	*nLen = szCode.length();
	return 0;
}

