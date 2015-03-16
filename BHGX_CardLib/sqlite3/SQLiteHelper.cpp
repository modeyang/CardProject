#include <iostream>
#include "SQLiteHelper.h"

using namespace std;

CSQLiteHelper::CSQLiteHelper()
{

}

CSQLiteHelper::~CSQLiteHelper()
{
	closeDB();
}

int CSQLiteHelper::openDB(char *path)
{
	int last = sqlite3_open(path,&db);
	if(SQLITE_OK != last) {
		return -1;
	}
	return 0;
}

void CSQLiteHelper::closeDB()
{
	if (db != NULL) {
		sqlite3_close(db);
	}
}

int CSQLiteHelper::execSQL(char *sql)
{
	char *errMsg;
	int res = sqlite3_exec(db,sql,0,0, &errMsg);
	if (res != SQLITE_OK)  
	{  
		std::cout << "执行创建table的SQL 出错." << errMsg << std::endl;  
		return -1;  
	}
	return 0;
}

int CSQLiteHelper::rawQuery(char *sql, int *row, int *column, char ***result)
{
	int res = sqlite3_get_table(db,sql, result,row,column,0);
	if (res != SQLITE_OK){  
		return -1;  
	}  
	return 0;
}

int CSQLiteHelper::queryFromCallback(char *sql, callback pfunc_callback, char  **errMsg)
{
	int res = sqlite3_exec(db, sql, pfunc_callback, 0 , errMsg);
	if (res != SQLITE_OK){  
		return -1;  
	}  
	return 0;
}

