#include <map>
#include <vector>
#include "ExceptionCheck.h"
#include "Markup.h"
#include "TimeUtil.h"
#include "XmlUtil.h"
#include "../Card.h"
#include "../BHGX_CardLib.h"
#include "../public/liberr.h"
#include "../tinyxml/headers/tinyxml.h"
#include "../Encry/DESEncry.h"

using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#if (CPU_M1 || CPU_8K)
	#define CARDNO			1
	#define CARDSEQ			10
#else
	#define CARDNO			8
	#define CARDSEQ			10
#endif

#define FORBIDDEN_FLAG		0
#define WARINNING_FLAG		1

#pragma warning (disable : 4996)

CExceptionCheck::CExceptionCheck(std::map<int, std::map<int, std::string> > logConfig)
{
	mapLogConfig = logConfig;
}

CExceptionCheck::CExceptionCheck(char *logXml)
{
	CXmlUtil::paserLogXml(logXml, mapLogConfig);
}

CExceptionCheck::~CExceptionCheck(void)
{
}


int CExceptionCheck::parseExceptionXml(char *filePath, std::vector<excepRecord> &vecRecord)
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
		excepRecord record;
		xml.IntoElem();
		if (xml.FindElem("name")) {
			record.Name = xml.GetData();
		}
		if (xml.FindElem("idCard")) {
			record.ID = xml.GetData();
		}
		if (xml.FindElem("cardState")) {
			record.cardState = atoi((char*)xml.GetData().c_str());
		}
		if (xml.FindElem("cardNo")) {
			record.cardNO = xml.GetData();
		}
		if (xml.FindElem("cardSerialNo")) {
			record.cardSeq = xml.GetData();
		}
		vecRecord.push_back(record);
		xml.OutOfElem();
	}
	xml.OutOfElem();
	return 0;
}

int CExceptionCheck::isExceptionCard(std::vector<excepRecord> &vecRecord)
{
	for (int i=0; i<vecRecord.size(); i++) {
		excepRecord &record = vecRecord[i];
		if (m_strCardNO == record.cardNO && 
			m_strCardSeq == record.cardSeq) {
			return i;
		}
	}
	return -1;
}

int CExceptionCheck::geneForbiddenLog(excepRecord &record) 
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
		xml.AddElem("name", record.Name);
		xml.AddElem("idCard", record.ID);
		xml.AddElem("cardSerialNo", record.cardSeq);
		xml.AddElem("cardNo", record.cardNO);
		xml.OutOfElem();
		xml.OutOfElem();
		xml.Save(strFilePath.c_str());
	} else {
		xml.SetDoc("<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n");
		xml.AddElem("members");
		xml.IntoElem();
		xml.AddElem("member");
		xml.IntoElem();
		xml.AddElem("name", record.Name);
		xml.AddElem("idCard", record.ID);
		xml.AddElem("cardSerialNo", record.cardSeq);
		xml.AddElem("cardNo", record.cardNO);
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
	if (isForbidden()) {
		return CardForbidden;
	}
	std::map<int, std::string> &configMap = mapLogConfig[1];
	std::vector<excepRecord> vecForbiddenRecord;
	if (parseExceptionXml((char*)configMap[2].c_str(), vecForbiddenRecord) == 1) {
		return DescryFileError;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("CARDNO|CARDSEQ", szQuery);
	if (n != 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::map<int, std::string> mapQueryInfo;
	CXmlUtil::GetQueryInfos(szQuery, mapQueryInfo);
	m_strCardNO = mapQueryInfo[CARDNO];
	m_strCardSeq = mapQueryInfo[CARDSEQ];
	int index = isExceptionCard(vecForbiddenRecord);
	if (index != -1) {
		excepRecord &record = vecForbiddenRecord[index];
		geneForbiddenLog(record);
		writeForbiddenFlag(1);
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
	std::vector<excepRecord> vecWarnningRecord;
	std::string strFilePath(configMap[3]);
	strFilePath += strIDNumber.substr(0, 4);
	strFilePath += "_greylist.xml";
	if (parseExceptionXml((char*)strFilePath.c_str(), vecWarnningRecord) == 1) {
		return DescryFileError;
	}

	if (isExceptionCard(vecWarnningRecord) != -1) {
		CXmlUtil::CreateResponXML(CardWarnning, err(CardWarnning), xml);
		return CardWarnning;
	}
	return CardProcSuccess;
}


CDBExceptionCheck::CDBExceptionCheck(std::map<int, std::map<int, std::string> > logConfig)
:CExceptionCheck(logConfig){
	initDBHelper();
}

CDBExceptionCheck::CDBExceptionCheck(char *logXml)
:CExceptionCheck(logXml){
	initDBHelper();
}

CDBExceptionCheck::~CDBExceptionCheck(void)
{
	if (m_dbHelper != NULL) {
		m_dbHelper->closeDB();
		m_dbHelper = NULL;
	}
}

int CDBExceptionCheck::initDBHelper()
{
	m_dbHelper = NULL;
	if (fileIsExisted((char*)mapLogConfig[1][2].c_str())) {
		m_dbHelper = new CSQLiteHelper();
		m_dbHelper->openDB((char*)mapLogConfig[1][2].c_str());
	}
	return 0;
}

int CDBExceptionCheck::filterForbidden(char *xml)
{
	if (isForbidden()) {
		return CardForbidden;
	}
	if (m_dbHelper == NULL) {
		CXmlUtil::CreateResponXML(CardDBFileNotFound, err(CardDBFileNotFound), xml);
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
	std::map<int, std::string> mapQueryInfo;
	CXmlUtil::GetQueryInfos(szQuery, mapQueryInfo);
	m_strCardNO = mapQueryInfo[CARDNO];
	m_strCardSeq = mapQueryInfo[CARDSEQ];
	int status = isExceptionCard(FORBIDDEN_FLAG);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardForbidden, err(CardForbidden), xml);
		return CardForbidden;
	}
	return CardProcSuccess;
}

int CDBExceptionCheck::filterWarnning(char *xml)
{
	if (m_dbHelper == NULL) {
		CXmlUtil::CreateResponXML(CardDBFileNotFound, err(CardDBFileNotFound), xml);
		return CardDBFileNotFound;
	}

	int status = isExceptionCard(WARINNING_FLAG);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardWarnning, err(CardWarnning), xml);
		return CardWarnning;
	}
	return CardProcSuccess;
}

int CDBExceptionCheck::isExceptionCard(int checkFlag)
{
	int queryStatus = 0;
	char sql[200];

	memset(sql, 0, sizeof(sql));
	sprintf_s(sql, sizeof(sql), "select * from card_checks where cardNo='%s' \
				and cardSerialNo='%s' and checkState=%s", 
				m_strCardNO.c_str(), m_strCardSeq.c_str(), checkFlag);

	int row, col;
	char **result;
	queryStatus = m_dbHelper->rawQuery(sql, &row, &col, &result);
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

	// forbidden card, generate log and write flag
	if (checkFlag == FORBIDDEN_FLAG) {
		excepRecord record;
		for (int i=0; i<vecColName.size(); i++) {
			if (strcmp(vecColName[i].c_str(), "name") == 0) {
				record.Name = std::string(vecColValue[i]);

			} else if (strcmp(vecColName[i].c_str(), "idCard") == 0) {
				record.ID = std::string(vecColValue[i]);

			}else if (strcmp(vecColName[i].c_str(), "cardState") == 0) {
				record.cardState = atoi(vecColValue[i].c_str());

			}else if (strcmp(vecColName[i].c_str(), "cardNo") == 0) {
				record.cardNO = std::string(vecColValue[i]);

			}else if (strcmp(vecColName[i].c_str(), "cardSerialNo") == 0) {
				record.cardSeq = std::string(vecColValue[i]);

			}
		}
		geneForbiddenLog(record);
		writeForbiddenFlag(1);
	}
	return 0;

}
