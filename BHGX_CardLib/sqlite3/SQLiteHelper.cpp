
#include "SQLiteHelper.h"
#include <exception>
#include <sstream>
#include "../public/liberr.h"
#include "../public/debug.h"

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
	m_dbPath = path;
	int last = sqlite3_open(path, &m_SqliteDB);
	if(SQLITE_OK != last) {
		return CardDBConnectError;
	}
	return CardProcSuccess;
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
		return CardSQLError;
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
	m_pConnection = NULL;
}

CSQLServerHelper::~CSQLServerHelper()
{
}

int CSQLServerHelper::openDB(char *path)
{
	m_dbPath = path;
	return this->connect(path);
}

void CSQLServerHelper::closeDB()
{

	if (m_pConnection != NULL && m_pConnection->GetState() == adStateOpen) {
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
			"Provider=SQLOLEDB.1;Password=BHGX@greatsoft.net; Persist Security Info=True; \
			User ID=sa;Initial Catalog=bhgx_healthcard ;Data Source=%s", addr);

		LOG_DEBUG(conn_str);
		_bstr_t bs_conn_str(conn_str);
		HRESULT hr = m_pConnection->Open(bs_conn_str, "","", adConnectUnspecified);  
		if (hr != S_OK) {
			LOG_ERROR("Can not connect to the specified database <%s>!", addr);
			return CardDBConnectError;
		}
	} catch (_com_error &e) {
		LOG_ERROR(e.Description());
		return CardDBConnectError;
	}
	return CardProcSuccess;
}

int CSQLServerHelper::reConnect()
{
	if (m_pConnection != NULL && m_pConnection->GetState() == adStateOpen) {
		return CardProcSuccess;
	}
	closeDB();
	return this->connect((char*)m_dbPath.c_str());
}


int CSQLServerHelper::query(char *sql)
{
	_bstr_t bstrSQL(sql);
	_RecordsetPtr record_set;
	LOG_INFO(sql);
	record_set.CreateInstance(__uuidof(Recordset)); 
	record_set->Open(bstrSQL, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);

	 while (!record_set->EndOfFile)
	 {
		 _variant_t vstr, vuint;
		db_check_info info;
		vstr = record_set->GetCollect("cardSerialNo");
		info.cardSerialNo = (_bstr_t)vstr.bstrVal;

		vstr = record_set->GetCollect("ATR");
		info.ATR = (_bstr_t)vstr.bstrVal;

		vstr = record_set->GetCollect("name");
		info.name = (_bstr_t)vstr.bstrVal;

		vstr = record_set->GetCollect("cardNo");
		info.cardNo = (_bstr_t)vstr.bstrVal;

		vstr = record_set->GetCollect("Idcard");
		info.Idcard = (_bstr_t)vstr.bstrVal;

		vuint = record_set->GetCollect("cityCode");
		info.cityCode = vuint.intVal; 

		vuint = record_set->GetCollect("checkState");
		info.checkState = vuint.intVal;

		vuint = record_set->GetCollect("cardState");
		info.cardState = vuint.intVal;

		m_vecCheckInfo.push_back(info);
		record_set->MoveNext();  
	 }
	
	 LOG_INFO("数据查询结果行数:%d", int(m_vecCheckInfo.size()));
	 return m_vecCheckInfo.size();
}

int CSQLServerHelper::insert_log(db_log_info & log_info)
{
	std::ostringstream ostr;
	ostr << "insert into card_log (ISSUEUNIT, CARDCODE, SAMID, CARDNO, IDNUMBER, NAME, Time, Log) VALUES ( '" 
		<< log_info.issueUnit << "','" << log_info.cardCode << "','" << log_info.SAMID << "','"
		<< log_info.cardNO << "','" << log_info.IDNumber << "','" << log_info.Name << "','" 
		<< log_info.Time << "','" << log_info.Log << "');";
	
	_variant_t RecordsAffected;
	_RecordsetPtr record_set;
	record_set.CreateInstance(__uuidof(Recordset)); 
	try{
		LOG_DEBUG((char*)ostr.str().c_str());
		record_set = m_pConnection->Execute(_bstr_t(ostr.str().c_str()) , &RecordsAffected, adCmdText);
	} catch (_com_error &e) {
		LOG_ERROR(e.Description());
		return CardDBInsertError;
	}
	int lines = (int)RecordsAffected.lVal;
	LOG_INFO("影响的行数: %d", 1);
	return CardProcSuccess;
}
