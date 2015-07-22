#pragma once
#include <io.h>
#include <map>
#include <string>
#include "XmlUtil.h"
#include "../sqlite3/SQLiteHelper.h"

using namespace std;

class CLogHelper
{
public:
	CLogHelper(char *LogXml);
	~CLogHelper(void);

	void setLogParams(int rwFlag, char * processName);
	void setCardInfo(char *cardXml);
	void setSamID(std::string SamID) { m_SamID = SamID; }
	void setHospInfo(char *hospInfo) { m_pHospInfo = hospInfo; }
	void geneHISLog();

private:
	int  initDBHelper();
	void geneLogInFile();
	void geneLogInDB();
	void getDefaultMap(std::map<std::string, std::string> &mapScValue);

	bool fileIsExisted(char *filename) {
		return _access(filename, 0) == 0;
	}

	std::map<int, std::map<int, std::string> > m_MapLogConfig; 
	std::map<std::string, ColumInfo> m_MapReaderInfo;
	int			m_rwFlag;
	std::string m_ProcessName;
	std::string m_SamID;
	std::string m_dbPath;
	char		*m_pCardInfo;
	char		*m_pHospInfo;
	CDBHelper	*m_dbHelper;
;
};
