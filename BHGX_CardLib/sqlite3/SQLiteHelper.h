#pragma once
#include "sqlite3.h"
#import "c:\program files\common files\system\ado\msado15.dll"  
no_namespace rename("EOF", "EndOfFile")   

typedef int (*callback)(void *NotUsed, int argc, char **argv, char **azColName);

class CSQLiteHelper
{
public:
	CSQLiteHelper(void);

	~CSQLiteHelper(void);

	sqlite3		*db;

	int openDB(char *path);
	void closeDB();

	int execSQL(char *sql);
	int rawQuery(char *sql,int *row,int *column,char ***result);
	int queryFromCallback(char *sql, callback pfunc_callback, char **errMsg);

};

struct db_check_info 
{
};


class CSQLServerHelper 
{
public:
	CSQLServerHelper();
	~CSQLServerHelper();

	int openDB(char *path);
	void closeDB();

	int execSQL(char *sql);
protected:
	int connect(char *addr);

	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;
};