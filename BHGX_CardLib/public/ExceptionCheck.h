#pragma once
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <io.h>
#include "../sqlite3/SQLiteHelper.h"

using namespace std;


class CExceptionCheck
{
public:
	CExceptionCheck(std::map<int, std::map<int, std::string> > logConfig);
	CExceptionCheck(char *logXml);

	~CExceptionCheck(void);

	int filterForbidden(char *xml);
	int filterWarnning(char *xml);

protected:
	int  writeForbiddenFlag(int flag);
	bool isForbidden();
	int  geneForbiddenLog(db_check_info &record);
	int  isExceptionCard(std::vector<db_check_info> &vecRecord);
	int  parseExceptionXml(char *filePath, std::vector<db_check_info> &vecRecord);

	bool fileIsExisted(char *filename) {
		return _access(filename, 0) == 0;
	}

protected:
	std::string m_strCardNO;
	std::string m_strCardSeq;
	std::string m_dbPath;
	std::map<int, std::map<int, std::string> > mapLogConfig;
};

class CDBExceptionCheck: CExceptionCheck
{
public :
	CDBExceptionCheck(std::map<int, std::map<int, std::string> > logConfig);
	CDBExceptionCheck(char *logXml);
	~CDBExceptionCheck(void);

	int filterForbidden(char *xml);
	int filterWarnning(char *xml);
protected:
	int initDBHelper();
	int isExceptionCard(int checkFlag);

protected:
	CDBHelper *m_dbHelper;
};
