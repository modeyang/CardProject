#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include "ExceptionCheck.h"
#include "Markup.h"
#include "TimeUtil.h"
#include "XmlUtil.h"
#include "../Card.h"
#include "../BHGX_CardLib.h"
#include "../public/debug.h"
#include "../public/liberr.h"
#include "../tinyxml/headers/tinyxml.h"
#include "../Encry/DESEncry.h"

using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")


#define FORBIDDEN_FLAG		0
#define WARINNING_FLAG		1

#pragma warning (disable : 4996)

CExceptionCheck::CExceptionCheck(std::map<int, std::map<int, std::string> > logConfig)
{
	m_bNormal = TRUE;
	mapLogConfig = logConfig;
	m_dbPath = mapLogConfig[1][1];
}

CExceptionCheck::CExceptionCheck(char *logXml)
{
	m_bNormal = TRUE;
	if (logXml == NULL || strlen(logXml) < 10) {
		m_bNormal = FALSE;
		return;
	} 
	
	try{
		CXmlUtil::paserLogXml(logXml, mapLogConfig);
		m_dbPath = mapLogConfig[1][1];
	} catch (exception& e) {
		cout << e.what() << endl;
		m_bNormal = FALSE;
	} 

}

CExceptionCheck::~CExceptionCheck(void)
{
}


int CExceptionCheck::parseExceptionXml(char *filePath, std::vector<db_check_info> &vecRecord)
{
	CMarkup xml;

	if (ENCRYPT == 1) {
		CDESEncry encry;
		if (!encry.DesryFile(filePath)){
			return 1;
		}
		xml.SetDoc(encry.GetDescryContent());
	} else {
		xml.Load(filePath);
	}

	
	if (!xml.FindElem("members")) {
		return -1;
	}
	xml.IntoElem();
	while (xml.FindElem("member")){
		db_check_info record;
		xml.IntoElem();
		if (xml.FindElem("name")) {
			record.name = xml.GetData();
		}
		if (xml.FindElem("idCard")) {
			record.Idcard = xml.GetData();
		}
		if (xml.FindElem("cardState")) {
			record.cardState = atoi((char*)xml.GetData().c_str());
		}
		if (xml.FindElem("cardNo")) {
			record.cardNo = xml.GetData();
		}
		if (xml.FindElem("cardSerialNo")) {
			record.cardSerialNo = xml.GetData();
		}
		vecRecord.push_back(record);
		xml.OutOfElem();
	}
	xml.OutOfElem();
	return 0;
}

int CExceptionCheck::isExceptionCard(std::vector<db_check_info> &vecRecord)
{
	for (int i=0; i<vecRecord.size(); i++) {
		db_check_info &record = vecRecord[i];
		if (m_strCardNO == record.cardNo && 
			m_strCardSeq == record.cardSerialNo) {
			return i;
		}
	}
	return -1;
}

int CExceptionCheck::geneForbiddenLog(db_check_info &record) 
{
	std::map<int, std::string> &configMap = mapLogConfig[1];
	std::map<int, std::string> &contentMap = mapLogConfig[2];
	CMarkup xml;

	char timeStr[64];
	std::string strFilePath(configMap[4]);
	strFilePath += strcat(CTimeUtil::getCurrentDay(timeStr), ".xml");

	if (fileIsExisted((char*)strFilePath.c_str())) {
		xml.Load(strFilePath.c_str());
		xml.ResetMainPos();
		xml.FindElem("members");
		xml.IntoElem();
		xml.AddElem("member");
		xml.IntoElem();
		xml.AddElem("name", record.name);
		xml.AddElem("idCard", record.Idcard);
		xml.AddElem("cardSerialNo", record.cardSerialNo);
		xml.AddElem("cardNo", record.cardNo);
		xml.OutOfElem();
		xml.OutOfElem();
		xml.Save(strFilePath.c_str());
	} else {
		xml.SetDoc("<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n");
		xml.AddElem("members");
		xml.IntoElem();
		xml.AddElem("member");
		xml.IntoElem();
		xml.AddElem("name", record.name);
		xml.AddElem("idCard", record.Idcard);
		xml.AddElem("cardSerialNo", record.cardSerialNo);
		xml.AddElem("cardNo", record.cardNo);
		xml.OutOfElem();
		xml.OutOfElem();
		xml.Save(strFilePath.c_str());
	}
	return 0;
}


bool CExceptionCheck::isForbidden()
{
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("FORBIDDEN_FLAG", szQuery);
	if (n != 0){
		return false;
	}
	std::string flagStr;
	CXmlUtil::GetQueryInfoForOne(szQuery, flagStr);
	int status = atoi(flagStr.c_str());
	return (status == 1) ;
}

int CExceptionCheck::writeForbiddenFlag(int flag)
{
	CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n");
	xml.AddElem("SEGMENTS");
	xml.AddAttrib("PROGRAMID", "001");
	xml.IntoElem();
	xml.AddElem("SEGMENT");
	xml.AddAttrib("ID", 4);
	xml.IntoElem();
	xml.AddElem("COLUMN");
	xml.AddAttrib("ID", 58);
	xml.AddAttrib("VALUE", flag);
	xml.OutOfElem();
	xml.OutOfElem();
	char *writeBuffer = (char*)xml.GetDoc().c_str();
	int status = iWriteInfo(writeBuffer);
	return status;
}


// 0 为非黑名单  
int CExceptionCheck::filterForbidden(char *xml) 
{
	std::map<int, std::string> &configMap = mapLogConfig[1];
	std::vector<db_check_info> vecForbiddenRecord;
	int status = parseExceptionXml((char*)configMap[2].c_str(), vecForbiddenRecord);
	if (status == 1) {
		return DescryFileError;
	} else if (status == -1) {
		return CardXmlErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("CARDNO|CARDSEQ", szQuery);
	if (n != 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::map<std::string, std::string> mapQueryInfo;
	CXmlUtil::GetQueryInfos(szQuery, mapQueryInfo);
	m_strCardNO = mapQueryInfo["CARDNO"];
	m_strCardSeq = mapQueryInfo["CARDSEQ"];
	int index = isExceptionCard(vecForbiddenRecord);
	if (index != -1) {
		CXmlUtil::CreateResponXML(CardForbidden, err(CardForbidden), xml);
		return CardForbidden;
	}
	return CardProcSuccess;
}

// 0 为非灰名单
int CExceptionCheck::filterWarnning(char *xml)
{
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("IDNUMBER", szQuery);
	if (n != 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::string strIDNumber;
	CXmlUtil::GetQueryInfoForOne(szQuery, strIDNumber);

	std::map<int, std::string> &configMap = mapLogConfig[1];
	std::vector<db_check_info> vecWarnningRecord;
	std::string strFilePath(configMap[3]);
	strFilePath += strIDNumber.substr(0, 4);
	strFilePath += "_greylist.xml";
	int status = parseExceptionXml((char*)strFilePath.c_str(), vecWarnningRecord);
	if (status == 1) {
		return DescryFileError;
	} else if (status == -1) {
		return CardXmlErr;
	}

	if (isExceptionCard(vecWarnningRecord) != -1) {
		CXmlUtil::CreateResponXML(CardWarnning, err(CardWarnning), xml);
		return CardWarnning;
	}
	return CardProcSuccess;
}


CDBExceptionCheck::CDBExceptionCheck(std::map<int, std::map<int, std::string> > logConfig)
:CExceptionCheck(logConfig){
}

CDBExceptionCheck::CDBExceptionCheck(char *logXml)
:CExceptionCheck(logXml){
}

CDBExceptionCheck::~CDBExceptionCheck(void)
{
	if (m_dbHelper != NULL) {
		m_dbHelper->closeDB();
		delete m_dbHelper;
		m_dbHelper = NULL;
	}
}

int CDBExceptionCheck::initDBHelper()
{
	m_dbHelper = NULL;
	string &path = m_dbPath;
	if (CHECK_TYPE == 1) {
		m_dbHelper = new CSQLServerHelper();
	} else if (CHECK_TYPE == 0 && fileIsExisted((char*)path.c_str())) {
		m_dbHelper = new CSQLiteHelper();
	} else {
		return CardDBConnectError;
	}
	return m_dbHelper->openDB((char*)path.c_str());
}

int CDBExceptionCheck::filterForbidden(char *xml)
{
	if (m_dbHelper == NULL) {
		CXmlUtil::CreateResponXML(CardDBFileNotFound, err(CardDBFileNotFound), xml);
		LOG_ERROR(err(CardDBFileNotFound));
		return CardDBFileNotFound;
	}

	std::map<int, std::string> &configMap = mapLogConfig[1];

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("CARDNO|CARDSEQ", szQuery);
	if (n != 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::map<std::string, std::string> mapQueryInfo;
	CXmlUtil::GetQueryInfos(szQuery, mapQueryInfo);
	m_strCardNO = mapQueryInfo["CARDNO"];
	m_strCardSeq = mapQueryInfo["CARDSEQ"];
	LOG_INFO("cardno:%s, cardseq:%s", m_strCardNO.c_str(), m_strCardSeq.c_str());
	int status = isExceptionCard(FORBIDDEN_FLAG);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardForbidden, err(CardForbidden), xml);
		LOG_ERROR(err(CardForbidden));
		return CardForbidden;
	}
	return CardProcSuccess;
}

int CDBExceptionCheck::filterWarnning(char *xml)
{
	if (m_dbHelper == NULL) {
		CXmlUtil::CreateResponXML(CardDBFileNotFound, err(CardDBFileNotFound), xml);
		LOG_ERROR(err(CardDBFileNotFound));
		return CardDBFileNotFound;
	}

	int status = isExceptionCard(WARINNING_FLAG);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardWarnning, err(CardWarnning), xml);
		LOG_ERROR(err(CardWarnning));
		return CardWarnning;
	}
	return CardProcSuccess;
}

int CDBExceptionCheck::isExceptionCard(int checkFlag)
{
	char sql[200];
	memset(sql, 0, sizeof(sql));
	sprintf_s(sql, sizeof(sql), "select * from card_checks where cardNo='%s' \
				and cardSerialNo='%s' and checkState=%d", 
				m_strCardNO.c_str(), m_strCardSeq.c_str(), checkFlag);
	
	int status = m_dbHelper->query(sql);
	return status <= 0 ? 0 : status;
}
