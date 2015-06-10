#pragma once
// "c:\program files\common files\system\ado\msado15.dll"  
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF", "EndOfFile")   
#include <string>
#include <vector>
#include "sqlite3.h"
//using namespace std;

typedef int (*callback)(void *NotUsed, int argc, char **argv, char **azColName);

struct db_check_info 
{
	std::string cardSerialNo;
	std::string ATR;
	int cityCode;
	int checkState;
	std::string name;
	std::string cardNo;
	std::string Idcard;
	int cardState;
};


class CDBHelper
{
public:
	CDBHelper(void){}
	virtual ~CDBHelper() {};
	virtual int openDB(char *path) {return 0;};
	virtual void closeDB() {};
	virtual int query(char *sql) {return 0;};

	std::string m_dbType;
	std::vector<db_check_info> m_vecCheckInfo;
};

class CSQLiteHelper: public CDBHelper
{
public:
	CSQLiteHelper(void);

	~CSQLiteHelper(void);

	int openDB(char *path);
	void closeDB();

	int query(char *sql);

protected:
	int rawQuery(char *sql,int *row,int *column,char ***result);
	int queryFromCallback(char *sql, callback pfunc_callback, char **errMsg);

	sqlite3		*m_SqliteDB;

};


class CSQLServerHelper : public CDBHelper
{
public:
	CSQLServerHelper();
	~CSQLServerHelper();

	int openDB(char *path);
	void closeDB();

	int query(char *sql);
protected:
	int connect(char *addr);

	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;

};