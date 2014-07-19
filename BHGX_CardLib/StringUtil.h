#pragma once
#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include <vector>
#include <string>

using namespace std;

#define SPACE			0x20
#define ASSCI_1			0x30
#define ASSCI_9			0x39
#define NAME_MAX_LEN	256

void 
trim(string & str);

vector<string> 
split(const string& src, 
	  string delimit, 
	  string null_subst="");

void Utf8ToGb2312(char* pstrOut, int dwOutLen, const char* pstrIn, int dwInLen);

bool inline Is_GbkName(char *szValue)
{
	int i = 0;
	while (szValue[i] != 0)
	{
		if(szValue[i] > ASSCI_1 && szValue[i] < ASSCI_9)
		{
			return false;
		}
		++i;
	}
	return true;
}

bool inline Is_IntName(char *szValue)
{
	int i=0;
	while (szValue[i] != 0)
	{
		if (szValue[i] < 0)
		{
			return false;
		}
		++i;
	}
	return true;
}




int inline CheckSpace(const char *szCheck, int nLen, char *strValue)
{
	int newlen = 0; 
	for (int i=0; i<nLen; ++i)
	{
		if (szCheck[i] != SPACE)
		{
			strValue[newlen] = szCheck[i];
			++newlen;
		}
	}
	strValue[newlen] = 0;
	return newlen;
}


#endif
