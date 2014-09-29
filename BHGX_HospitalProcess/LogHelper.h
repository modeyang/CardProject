#pragma once
#include <string>
#include <map>
#include "XmlUtil.h"
using namespace std;

class CLogHelper
{
public:
	CLogHelper(char *LogXml);
	~CLogHelper(void);

	void setLogParams(int rwFlag, char * processName);
	void setCardInfo(char *cardXml);
	void geneHISLog();

private:
	void getDefaultMap(std::map<std::string, std::string> &mapScValue);
private:
	std::map<int, std::map<int, std::string> > m_MapLogConfig; 
	std::map<std::string, ColumInfo> m_MapReaderInfo;
	int			m_rwFlag;
	std::string m_ProcessName;
	char		*m_pCardInfo;
};
