#pragma once
#include <string>
#include <map>
using namespace std;

class CLogHelper
{
public:
	CLogHelper(char *LogXml);
	~CLogHelper(void);

	void setLogParams(int rwFlag, char * processName);
	void setCardInfo(char *cardXml);
	void geneHISLog(char *pszContent);

private:
	std::map<int, std::map<int, std::string> > m_MapLogConfig; 
	std::map<int, std::string> m_MapReaderInfo;
	int m_rwFlag;
	std::string m_ProcessName;
};
