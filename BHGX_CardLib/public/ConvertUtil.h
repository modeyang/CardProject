#pragma once
#include <string>
using namespace std;

class CConvertUtil
{
public:
	CConvertUtil(void);

	~CConvertUtil(void);

	static std::string uf_gbk_int_covers(const char* gbk_or_int, std::string covers);

	static std::string zh2num(const char* strsz);

	static std::string num2zh(const char* strNum);

	static wchar_t *ConvertMutliToUnicode(char *strMutli);

	static std::string ConvertZHName(const char *, const char *);

protected:
	static std::string uf_inttostr(long strAscii);

	static std::string uf_strtoint(const char *str, int nstrlen);
};
