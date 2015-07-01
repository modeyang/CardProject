#include <iostream>
#include "SQLiteHelper.h"
#include <exception>

using namespace std;

CSQLiteHelper::CSQLiteHelper()
:CDBHelper()
{

}

CSQLiteHelper::~CSQLiteHelper()
{
	closeDB();
}

int CSQLiteHelper::openDB(char *path)
{
	int last = sqlite3_open(path, &m_SqliteDB);
	if(SQLITE_OK != last) {
		return -1;
	}
	return 0;
}

void CSQLiteHelper::closeDB()
{
	if (m_SqliteDB != NULL) {
		sqlite3_close(m_SqliteDB);
	}
}

int CSQLiteHelper::query(char *sql)
{
	int row, col;
	char **result;
	int queryStatus = rawQuery(sql, &row, &col, &result);
	if (row == 0 || queryStatus != 0) {
		return -1;
	}

	// get query info
	std::vector<std::string> vecColName;
	for (int i=0; i<col; i++) {
		vecColName.push_back(result[i]);
	}

	std::vector<std::string> vecColValue;
	for (int j=0; j<col; j++) {
		vecColValue.push_back(std::string(result[1 * col + j]));
	}

	db_check_info info;
	for (int i=0; i<vecColName.size(); i++) {
		
		if (strcmp(vecColName[i].c_str(), "name") == 0) {
			info.name = std::string(vecColValue[i]);

		} else if (strcmp(vecColName[i].c_str(), "ATR") == 0) {
			info.ATR = std::string(vecColValue[i]);

		}else if (strcmp(vecColName[i].c_str(), "cardState") == 0) {
			info.cardState = atoi(vecColValue[i].c_str());

		}else if (strcmp(vecColName[i].c_str(), "cardNo") == 0) {
			info.cardNo = std::string(vecColValue[i]);

		}else if (strcmp(vecColName[i].c_str(), "cardSerialNo") == 0) {
			info.cardSerialNo = std::string(vecColValue[i]);

		} else if (strcmp(vecColName[i].c_str(), "cityCode") == 0) {
			info.cityCode = atoi(vecColValue[i].c_str());

		} else if (strcmp(vecColName[i].c_str(), "checkState") == 0){
			info.checkState = atoi(vecColValue[i].c_str());

		}else if (strcmp(vecColName[i].c_str(), "Idcard") == 0) {
			info.Idcard = std::string(vecColValue[i]);

		}
	}
	m_vecCheckInfo.push_back(info);
	return m_vecCheckInfo.size();
}

int CSQLiteHelper::rawQuery(char *sql, int *row, int *column, char ***result)
{
	int res = sqlite3_get_table(m_SqliteDB, sql, result,row,column,0);
	if (res != SQLITE_OK){  
		return -1;  
	}  
	return 0;
}

int CSQLiteHelper::queryFromCallback(char *sql, callback pfunc_callback, char  **errMsg)
{
	int res = sqlite3_exec(m_SqliteDB, sql, pfunc_callback, 0 , errMsg);
	if (res != SQLITE_OK){  
		return -1;  
	}  
	return 0;
}


CSQLServerHelper::CSQLServerHelper()
:CDBHelper()
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
	if(m_pRecordset != NULL){
		m_pRecordset->Close();
		m_pConnection->Close();
	}

	::CoUninitialize(); //ÊÍ·Å»·¾³
}

int CSQLServerHelper::connect(char *addr)
{
	try {
		::CoInitialize(NULL);
		m_pConnection.CreateInstance(__uuidof(Connection));
		m_pRecordset.CreateInstance(__uuidof(Recordset)); 
		 
		char conn_str[200];
		memset(conn_str, 0, sizeof(conn_str));
		sprintf_s(conn_str, sizeof(conn_str), 
			"Provider=SQLOLEDB.1;Password=BHGX@greatsoft.net ; Persist Security Info=True; \
			User ID=sa;Initial Catalog=bhgx_healthcard ;Data Source=%s", addr);

		_bstr_t bs_conn_str(conn_str);
		HRESULT hr = m_pConnection->Open(bs_conn_str, "","", adModeUnknown);  
		if (hr != S_OK) {
			cout<<"Can not connect to the specified database!"<<endl;
			return -1;
		}
	} catch (_com_error e) {
		cout<<e.Description()<<endl;
	}
	return -1;
}

int CSQLServerHelper::query(char *sql)
{
	_bstr_t bstrSQL(sql);
	 m_pRecordset->Open(bstrSQL, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);

	 while (!m_pRecordset->EndOfFile)
	 {
		 _variant_t vstr, vuint;
		db_check_info info;
		vstr = m_pRecordset->GetCollect("cardSerialNo");
		info.cardSerialNo = (_bstr_t)vstr.bstrVal;

		vstr = m_pRecordset->GetCollect("ATR");
		info.ATR = (_bstr_t)vstr.bstrVal;

		vstr = m_pRecordset->GetCollect("name");
		info.name = (_bstr_t)vstr.bstrVal;

		vstr = m_pRecordset->GetCollect("cardNo");
		info.cardNo = (_bstr_t)vstr.bstrVal;

		vstr = m_pRecordset->GetCollect("Idcard");
		info.Idcard = (_bstr_t)vstr.bstrVal;

		vuint = m_pRecordset->GetCollect("cityCode");
		info.cityCode = vuint.intVal; 

		vuint = m_pRecordset->GetCollect("checkState");
		info.checkState = vuint.intVal;

		vuint = m_pRecordset->GetCollect("cardState");
		info.cardState = vuint.intVal;

		m_vecCheckInfo.push_back(info);
		m_pRecordset->MoveNext();  
	 }
	 m_pRecordset->Close();

	 return m_vecCheckInfo.size();
}

int CSQLServerHelper::insert_log(db_log_info & log_info)
{
	char insert_str[1024 * 12];
	memset(insert_str, 0, sizeof(insert_str));
	sprintf_s(insert_str, sizeof(insert_str), "insert into card_log(ISSUEUINT, CARDCODE, SAMID, \
		CARDNO, IDNUMBER, NAME, Time, Log) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", 
		log_info.issueUnit, log_info.cardCode, log_info.SAMID, log_info.cardNO,
		log_info.IDNumber, log_info.Name, log_info.Time, log_info.Log);

	try{
		m_pConnection->Execute(_bstr_t(insert_str) ,NULL, adExecuteNoRecords);
	} catch (exception& e) {
		cout<<e.what()<<endl;
		return -1;
	}
	return 0;
}
