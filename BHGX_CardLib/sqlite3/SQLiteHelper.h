#pragma once
#include "sqlite3.h"

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
	char** rawQuery(char *sql,int *row,int *column,char **result);
	int queryFromCallback(char *sql, callback pfunc_callback, char **errMsg);

};
