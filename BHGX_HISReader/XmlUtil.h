#pragma once
#include <string>
#include <map>
using namespace std;

struct ColumInfo 
{
	int			ID;
	std::string strValue;
	std::string strSource;
	std::string strDesc;
};

class CXmlUtil
{
public:
	CXmlUtil(void);
	~CXmlUtil(void);

	static int  GetQueryInfoForOne(char *QueryXML, std::string &szCardNO);
	static int  GetQueryInfos(char *QueryXML, std::map<int, std::string> &mapQueryInfo);
	static void CreateResponXML(int nID, const char *szResult, char *RetXML);
	static void parseHISXml(const char *szReader, std::map<std::string, ColumInfo> &mapAll);
	static int  paserLogXml(char *pszLogXml, std::map<int, std::map<int, std::string> > &mapLogConfig);
	static int  CheckCardXMLValid(std::string &xml);
};
