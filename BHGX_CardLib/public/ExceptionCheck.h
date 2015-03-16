#pragma once
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <io.h>
#include "../sqlite3/SQLiteHelper.h"

using namespace std;

struct excepRecord
{
	std::string Name;
	std::string ID;
	int			cardState;
	std::string cardNO;
	std::string cardSeq;
};

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
	int  geneForbiddenLog(excepRecord &record);
	int  isExceptionCard(std::vector<excepRecord> &vecRecord);
	int  parseExceptionXml(char *filePath, std::vector<excepRecord> &vecRecord);

	bool fileIsExisted(char *filename) {
		return _access(filename, 0) == 0;
	}

protected:
	std::string m_strCardNO;
	std::string m_strCardSeq;
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

	static int forbidden_query(void *NotUsed, int argc, char **argv, char **azColName);
	static int warnning_query(void *NotUsed, int argc, char **argv, char **azColName);
protected:
	int m_forbidden_flag;
	int m_warnning_flag;
	CSQLiteHelper *m_dbHelper;
};
