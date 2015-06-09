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


CSQLServerHelper::CSQLServerHelper()
{

}

CSQLServerHelper::~CSQLServerHelper()
{

}

int CSQLServerHelper::openDB(char *path)
{
	return this->connect(path);
}

void CSQLServerHelper::closeDB()
{
	if(m_pRecordset!=NULL){
		m_pRecordset->Close();
		m_pConnection->Close();
	}

	::CoUninitialize(); //释放环境
}

int CSQLServerHelper::connect(char *addr)
{
	try {
		::CoInitialize(NULL);
		m_pConnection.CreateInstance(__uuidof(Connection));
		 
		char conn_str[200];
		memset(conn_str, 0, sizeof(conn_str));
		sprintf_s(conn_str, sizeof(conn_str), 
			"Provider=SQLOLEDB.1;Password=bhgx@greatsoft.net ; Persist Security Info=True; \
			User ID=sa;Initial Catalog=bhgx_healthcard ;Data Source=%s", addr);

		_bstr_t bs_conn_str(conn_str);
		HRESULT hr = m_pConnection->Open(bs_conn_str, "","", adModeUnknown);  
		if (hr != S_OK) {
			cout<<"Can not connect to the specified database!"<<endl;
			return -1;
		}
	} catch (_com_error e) {
		cout<<e.Description()<<endl;
		return -1;
	}
	
}

int CSQLServerHelper::execSQL(char *sql)
{

}
