
#include <vector>
#include <exception>
#include "LogHelper.h"
#include "XmlUtil.h"
#include "TimeUtil.h"
#include "../Card.h"
#include "../sqlite3/SQLiteHelper.h"
#include "../tinyxml/headers/tinyxml.h"

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

using namespace std;

#define LOGPATH  "d:/config/"

CLogHelper::CLogHelper(char *LogXml)
{
	CXmlUtil::paserLogXml(LogXml, m_MapLogConfig);
	m_dbPath = m_MapLogConfig[1][1];
	initDBHelper();
}

CLogHelper::~CLogHelper(void)
{
	if (m_dbHelper != NULL) {
		m_dbHelper->closeDB();
		delete m_dbHelper;
		m_dbHelper = NULL;
	}
}

void CLogHelper::setLogParams(int rwFlag, char * processName)
{
	m_rwFlag = rwFlag;
	m_ProcessName = processName;
}

void CLogHelper::setCardInfo(char *cardXml)
{
	m_pCardInfo = cardXml;
	CXmlUtil::parseHISXml(cardXml, m_MapReaderInfo);
}

void CLogHelper::getDefaultMap(std::map<std::string, std::string> &mapScValue)
{
	std::map<std::string, ColumInfo>::iterator iter = m_MapReaderInfo.begin();
	for (; iter!=m_MapReaderInfo.end(); iter++) {
		ColumInfo &info = iter->second;
		mapScValue[iter->first] = info.strValue; 
	}
}

int CLogHelper::initDBHelper()
{
	m_dbHelper = NULL;
	string &path = m_dbPath;
	if (CHECK_TYPE == 1) {
		m_dbHelper = new CSQLServerHelper();
	} else if (CHECK_TYPE == 0 && fileIsExisted((char*)path.c_str())) {
		m_dbHelper = new CSQLiteHelper();
	}
	m_dbHelper->openDB((char*)path.c_str());
	return 0;
}

void CLogHelper::geneLogInFile()
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	XmlDoc.Parse(m_pCardInfo);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();

	std::vector<TiXmlElement*> vtcSeg;
	while (Segment) {
		vtcSeg.push_back(Segment);
		Segment = Segment->NextSiblingElement();
	}

	TiXmlDocument *XmlDocLog = NULL;
	TiXmlElement *LogElement = NULL;
	TiXmlElement *InfoSegment = NULL;
	TiXmlElement *LogSegment = NULL;
	TiXmlPrinter Printer;
	TiXmlDeclaration HeadDec;

	// 创建XML文档
	XmlDocLog = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDocLog->LinkEndChild(&HeadDec);

	LogElement = new TiXmlElement("LogInfo");
	LogElement->SetAttribute("PROGRAMID", "001");

	InfoSegment = new TiXmlElement("Info");
	InfoSegment->SetAttribute("ID", 1);
	InfoSegment->SetAttribute("DESC", "base info");

	LogSegment = new TiXmlElement("SEGMENT");
	LogSegment->SetAttribute("ID", 1);

	//insert base info column
	char timeStr[64];
	CTimeUtil::getCurrentTime(timeStr);
	std::map<int, std::string> contentMap;
	if (m_MapLogConfig.size() > 1) {
		contentMap = m_MapLogConfig[2];
	}

	std::map<std::string, std::string> mapScValue;
	getDefaultMap(mapScValue);
	contentMap[-1] = mapScValue["ISSUEUNIT"];
	contentMap[0] = mapScValue["CARDCODE"];
	// PSAM info
	contentMap[1] = "";

	contentMap[2] = mapScValue["CARDNO"];
	contentMap[3] = mapScValue["IDNUMBER"];
	contentMap[4] = mapScValue["NAME"];
	contentMap[5] = timeStr;
	if (m_rwFlag == 0)  {
		contentMap[6] = "0";
	} else {
		contentMap[6] = "1";
	}
	contentMap[7] = m_ProcessName;
	std::map<int, std::string>::iterator mapIter = contentMap.begin();
	for (; mapIter != contentMap.end(); mapIter++) {
		TiXmlElement *pColumn = new TiXmlElement("COLUMN");
		pColumn->SetAttribute("ID", mapIter->first + 2);
		pColumn->SetAttribute("VALUE", mapIter->second.c_str());
		LogSegment->LinkEndChild(pColumn);
	}
	InfoSegment->LinkEndChild(LogSegment);
	LogElement->LinkEndChild(InfoSegment);

	TiXmlElement *pCtInfoSegment = new TiXmlElement("Info");
	pCtInfoSegment->SetAttribute("ID", 2);
	pCtInfoSegment->SetAttribute("VALUE", "content");

	for (int i=0; i<vtcSeg.size(); i++) {
		pCtInfoSegment->InsertEndChild(*vtcSeg[i]);
	}
	LogElement->LinkEndChild(pCtInfoSegment);

	XmlDocLog->LinkEndChild(LogElement);
	XmlDocLog->Accept(&Printer);

	std::string strFilePath(string(LOGPATH) + mapScValue["CARDCODE"]);
	strFilePath += strcat(CTimeUtil::getCurrentDay(timeStr), ".log");
	FILE *fp = fopen(strFilePath.c_str(), "a+");
	fwrite(Printer.CStr(), strlen(Printer.CStr()), 1, fp);
	fclose(fp);
}

void CLogHelper::geneLogInDB()
{
	std::map<std::string, std::string> mapScValue;
	getDefaultMap(mapScValue);

	db_log_info log_info;
	log_info.issueUnit = mapScValue["ISSUEUNIT"];
	log_info.cardCode = mapScValue["CARDCODE"];
	log_info.cardNO = mapScValue["CARDNO"];
	log_info.IDNumber = mapScValue["IDNUMBER"];
	log_info.Name = mapScValue["NAME"];
	log_info.Log = m_pCardInfo;
	log_info.SAMID = "";

	//insert base info column
	char timeStr[64];
	CTimeUtil::getCurrentTime(timeStr);
	log_info.Time = timeStr;
	
	m_dbHelper->insert_log(log_info);
}


void CLogHelper::geneHISLog()
{
	if (LOG_STORE == 0) {
		geneLogInFile();
	} else if (LOG_STORE == 1) {
		geneLogInDB();
	}
	
}


