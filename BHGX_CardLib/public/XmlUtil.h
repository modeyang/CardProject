#pragma once

#include <string>
#include <map>
using namespace std;

class CXmlUtil
{
public:
	CXmlUtil(void);
	~CXmlUtil(void);

	static int  GetQueryInfoForOne(char *QueryXML, std::string &szCardNO);
	static int  GetQueryInfos(char *QueryXML, std::map<int, std::string> &mapQueryInfo);
	static void CreateResponXML(int nID, const char *szResult, char *RetXML);
	static void parseHISXml(const char *szReader, std::map<int, std::string> &mapAll);
	static int  paserLogXml(char *pszLogXml, std::map<int, std::map<int, std::string> > &mapLogConfig);

	static int CheckCardXMLValid(std::string &pszCardXml);

};
