#include "ConvertUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

CConvertUtil::CConvertUtil(void)
{

}

CConvertUtil::~CConvertUtil()
{
}


//把int转换成string
std::string CConvertUtil::uf_inttostr(long strAscii)
{
	char hold_str[30];

	// Convert first character of pair to a char
	//hold_str = std::string((char)strAscii);    // asc to char
	if (strAscii <128)
	{
		sprintf(hold_str, "%c", strAscii);
	}
	else
	{
		// Add characters to string after converting
		// the integer's high byte to char
		// asc is from char to asc
		char char2 = strAscii & 0xff;
		char char1 = (strAscii - ((unsigned char)char2))/255;
		sprintf(hold_str, "%c%c",char2, char1);
	}
	return std::string(hold_str);
}

//把中文string转换成int
std::string CConvertUtil::uf_strtoint(const char *str, int nstrlen)
{
	long strasc;
	unsigned char char1, char2;
	std::string ls_result;
	char szInt[30];
	if (nstrlen == 1)
	{
		return std::string(str);
	}
	else
	{
		char2 = (unsigned char)str[0];
		char1 = (unsigned char)str[1];
		strasc = char1*256 + char2;
		sprintf(szInt, "%d", strasc);
	}
	return std::string(szInt);
}

std::string CConvertUtil::zh2num(const char* strsz)
{
	std::string ls_strasc,ls_str,ls_temp;
	long ll_temp,ll_cou;
	int gbk_Len = strlen(strsz);
	ls_str = strsz;
	while (ls_str.size() > 0)
	{
		if (ls_str[0] < 0)
		{
			ls_temp = ls_str.substr(0, 2);
			ll_temp = 2;
		}
		else
		{
			ls_temp = ls_str.substr(0, 1);
			ll_temp = 1;
		}

		//转换成ASC
		ls_temp = uf_strtoint(ls_temp.c_str(), (int)ll_temp); 
		//定制5位
		ls_temp = ls_temp +"fffff"; //先拼接fffff ,再截取左5位
		ls_temp = ls_temp.substr(0, 5);
		ls_strasc = ls_strasc +ls_temp;

		//去掉已处理过的
		ls_str = ls_str.substr(ll_temp, ls_str.size());
	}
	return ls_strasc;
}

std::string CConvertUtil::num2zh(const char* strNum)
{
	std::string ls_strasc,ls_str,ls_temp;
	long ll_temp,ll_cou;
	int gbk_Len = strlen(strNum);
	ls_str = strNum;
	ll_cou = ls_str.size()/5;
	for (int i=0; i<ll_cou; ++i)
	{
		ls_temp = ls_str.substr(0, 5);
		int nFPos = ls_temp.find('f');
		if (nFPos > 0)
		{
			ls_temp  = ls_temp.substr(0, nFPos);
		}
		if (ls_temp.size() == 1)
		{
			ll_temp = char(ls_temp[0]);
		}
		else
		{
			ll_temp = atoi(ls_temp.c_str());
		}
		ls_temp = uf_inttostr((short)ll_temp);
		ls_strasc = ls_strasc + ls_temp;
		ls_str = ls_str.substr(5, ls_str.size());
	}
	return ls_strasc;
}

std::string CConvertUtil::uf_gbk_int_covers(const char* gbk_or_int, std::string covers)
{
	std::string ls_strasc,ls_str,ls_temp;
	long ll_temp,ll_cou;
	int gbk_Len = strlen((const char*)gbk_or_int);
	ls_str = (const char*)gbk_or_int;
	if (covers == "toint")
	{
		while (ls_str.size() > 0)
		{
			unsigned char cStart = (unsigned char)ls_str[0];
			if (cStart > 128)
			{
				ls_temp = ls_str.substr(0, 2);
				ll_temp = 2;
			}
			else
			{
				ls_temp = ls_str.substr(0, 1);
				ll_temp = 1;
			}

			//转换成ASC
			ls_temp = uf_strtoint(ls_temp.c_str(), (int)ll_temp); 
			//定制5位
			ls_temp = ls_temp +"FFFFF"; //先拼接fffff ,再截取左5位
			ls_temp = ls_temp.substr(0, 5);
			ls_strasc = ls_strasc +ls_temp;

			//去掉已处理过的
			ls_str = ls_str.substr(ll_temp, ls_str.size());
		}
	}
	else if (covers == "togbk")
	{
		ll_cou = ls_str.size()/5;
		for (int i=0; i<ll_cou; ++i)
		{
			ls_temp = ls_str.substr(0, 5);
			int nFPos = ls_temp.find('F');
			if (nFPos > 0)
			{
				ls_temp  = ls_temp.substr(0, nFPos);
			}

			int nePos = ls_temp.find('E');
			if (nePos == 0)
			{
				ls_temp = '.';
			}

			if (ls_temp.size() == 1)
			{
				ll_temp = char(ls_temp[0]);
			}
			else
			{
				ll_temp = atoi(ls_temp.c_str());
			}
			ls_temp = uf_inttostr(ll_temp);
			ls_strasc = ls_strasc + ls_temp;
			ls_str = ls_str.substr(5, ls_str.size());
		}
	}
	else
	{
		return "";
	}
	return ls_strasc;
}

wchar_t *CConvertUtil::ConvertMutliToUnicode(char *strMutli)
{
	DWORD dwSize = strlen(strMutli);
	DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, strMutli, -1, NULL, 0);
	wchar_t *pwText;
	pwText = new wchar_t[dwNum];
	MultiByteToWideChar (CP_ACP, 0, strMutli, dwSize, pwText, dwNum);
	return pwText;
}