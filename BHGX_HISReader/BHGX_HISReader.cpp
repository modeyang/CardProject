
#include <stdlib.h>
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "../BHGX_CardLib/public/liberr.h"
#include "tinyxml/headers/tinyxml.h"
#include "resource.h"
#include "BHGX_HISReader.h"
#include "Markup.h"
#include <map>
#include <vector>
#include <windows.h>
#include "TimeUtil.h"
#include "XmlUtil.h"

using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#ifdef _DEBUG
#pragma comment(lib, "../Debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../Release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)

#define LICENSEFILE	"北航冠新HIS.license"

#define SAFE_DELETE(a)	\
if (a != NULL)			\
{						\
	delete a;			\
	a = NULL;			\
}						\

struct HISReader 
{
	int			ID;
	std::string strValue;
	std::string strSource;
	std::string strDesc;
};


std::map<int, HISReader> mapHISReader;
std::map<int, std::map<int, std::string> > mapLogConfig; 
int g_rwFlag = 0;
char g_processName[20];

bool IsMedicalID(const std::string &strID)
{
	for (size_t i=0; i<strID.size(); ++i){
		char ID = strID[i];
		if (ID != 0x30){
			return true;
		}
	}
	return false;
}


void fillHISMapInfo(std::map<int, std::string> &mapAll, std::map<int, HISReader> &mapReader)
{	
	if (mapAll.size() > 0){
		std::map<int, HISReader>::iterator iter = mapReader.begin();
		for (; iter != mapHISReader.end(); ++iter){
			int nID = iter->first;
			HISReader &stHIS = iter->second;
			stHIS.strValue = mapAll[nID];
		}
	}
}

void createHISReaderXML(const std::map<int, HISReader> &mapReader, char *strHISReader)
{
	TiXmlDocument *XmlDoc = NULL;
	TiXmlElement *RootElement = NULL;
	TiXmlElement *Segment = NULL;
	TiXmlElement *Column = NULL;
	TiXmlDeclaration HeadDec;
	TiXmlPrinter Printer;

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");
	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID", 2);
	
	std::map<int, HISReader>::const_iterator iter = mapReader.begin();
	for (; iter != mapReader.end(); ++iter) {
		const HISReader &stHIS = iter->second;
		Column = new TiXmlElement("COLUMN");
		Column->SetAttribute("ID", iter->first);
		//Column->SetAttribute("DESC", stHIS.strDesc.c_str());
		Column->SetAttribute("VALUE", stHIS.strValue.c_str());
		Segment->LinkEndChild(Column);
	}
	
	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(strHISReader, Printer.CStr());
}

void geneHISLog(const char *pszContent, std::map<int, std::string> &mapXmlInfo)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	XmlDoc.Parse(pszContent);
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
	std::map<int, std::string> contentMap = mapLogConfig[2];
	contentMap[-1] = mapXmlInfo[2];
	contentMap[0] = mapXmlInfo[5];
	contentMap[8] = mapXmlInfo[1];
	contentMap[9] = mapXmlInfo[10];
	contentMap[10] = mapXmlInfo[9];
	contentMap[11] = timeStr;
	if (g_rwFlag == 0)  {
		contentMap[12] = "0";
	} else {
		contentMap[12] = "1";
	}
	contentMap[13] = g_processName;
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

	std::map<int, std::string> configMap = mapLogConfig[1];
	std::string strFilePath(configMap[1]);
	//strFilePath += strcat((char*)contentMap[2].c_str(), "_");
	strFilePath += strcat(CTimeUtil::getCurrentDay(timeStr), ".log");
	FILE *fp = fopen(strFilePath.c_str(), "a+");
	fwrite(Printer.CStr(), strlen(Printer.CStr()), 1, fp);
	fclose(fp);
}

int _getHISInfo(const char *szReader, char *strHISReader, 
				std::map<int, HISReader> &mapInfo, bool bLog)
{
	std::map<int, std::string> mapXmlInfo;
	CXmlUtil::parseHISXml(szReader, mapXmlInfo);

	fillHISMapInfo(mapXmlInfo, mapInfo);
	createHISReaderXML(mapInfo, strHISReader);
	if (bLog) {
		geneHISLog(strHISReader, mapXmlInfo);
	}
	return 0;
}

int getHISInfo(const char *szReader, char *strHISReader, bool bLog)
{
	if (mapHISReader.size() == 0){
		mapHISReader[1].strDesc = "卡号";
		mapHISReader[7].strDesc = "个人参合号";
		mapHISReader[9].strDesc = "姓名";
		mapHISReader[10].strDesc = "身份证号";
		mapHISReader[12].strDesc = "性别";
		mapHISReader[11].strDesc = "出生日期";
		mapHISReader[14].strDesc = "人员属性";
		mapHISReader[13].strDesc = "与户主关系";
		mapHISReader[21].strDesc = "首次参合时间";
		mapHISReader[18].strDesc = "健康状况";
		mapHISReader[22].strDesc = "电话号码";
		mapHISReader[39].strDesc = "健康档案号";
	}

	return _getHISInfo(szReader, strHISReader, mapHISReader, bLog);
}

int getInfoForXJ(const char *szReader, char *strHISReader, bool bLog)
{
	if (mapHISReader.size() == 0){
		mapHISReader[7].strDesc = "医疗证号";
		mapHISReader[9].strDesc = "姓名";
		mapHISReader[10].strDesc = "身份证号";
		mapHISReader[12].strDesc = "性别";
		mapHISReader[11].strDesc = "出生日期";
		mapHISReader[17].strDesc = "民族";
		mapHISReader[39].strDesc = "健康档案号";
	}
	return _getHISInfo(szReader, strHISReader, mapHISReader, bLog);
	
}
// 0 为农合医疗卡
int _filterNHCard(char *xml)
{
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("MEDICARECERTIFICATENO", szQuery);
	if (n != 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::string strMedicalID;
	CXmlUtil::GetQueryInfoForOne(szQuery, strMedicalID);

	if (!IsMedicalID(strMedicalID)){
		CXmlUtil::CreateResponXML(CardReadErr, "参合号不存在", xml);
		return CardReadErr;
	}
	return CardProcSuccess;
}


int readHISBaseInfo(char *pszCardCheckWSDL, char *pszCardServerURL,char *pszLogXml, char *xml, bool bNet, bool bLocal)
{
	if (xml == NULL){
		return -1;
	}

	int nInit = iCardInit();
	if (nInit != 0){
		CXmlUtil::CreateResponXML(CardInitErr, err(CardInitErr), xml);
		return CardInitErr;
	}

	if (pszLogXml != NULL)
		CXmlUtil::paserLogXml(pszLogXml, mapLogConfig);

	if (bLocal) {
		int status = iCheckException(pszLogXml, xml);
		if (status != CardProcSuccess) {
			return status;
		}

		if (CardProcSuccess != iReadInfo(2, xml)) {
			return CardReadErr;
		}
	} else {
		if (CardProcSuccess != _filterNHCard(xml)) {
			return CardReadErr;
		}

		if (bNet) {
			if (CardProcSuccess != iCheckMsgForNH(pszCardCheckWSDL, pszCardServerURL, xml)){
				return CardCheckError;
			}
		} else {
			if (CardProcSuccess != iReadInfo(2, xml)) {
				return CardReadErr;
			}
		}
	}
	iCardDeinit();
	return CardProcSuccess;
}

int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, NULL, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	status = getHISInfo(szRead, xml, false);
	return status;
}

int __stdcall iReadOnlyHIS(char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo("", "", NULL, szRead, false, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	status = getHISInfo(szRead, xml, false);
	return status;
}

int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, NULL,  szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	return getInfoForXJ(szRead, xml, true);
}

int __stdcall iReadHISInfoLog(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, pszLogXml, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadHISInfoLog");
	return getHISInfo(szRead, xml, true);
}

int __stdcall iReadOnlyHISLog(char *pszLogXml, char *xml)
{

	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo("", "", pszLogXml,  szRead, false, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadOnlyHISLog");
	return getHISInfo(szRead, xml, true);
}


int __stdcall iReadInfoForXJLog(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml) 
{

	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, pszLogXml, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadInfoForXJLog");
	return getHISInfo(szRead, xml, true);
}

int __stdcall iReadHISInfoLocal(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, pszLogXml, szRead, true, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadHISInfoLocal");
	return getHISInfo(szRead, xml, true);
}

int __stdcall iReadInfoForXJLocal(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml) 
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo(pszCardCheckWSDL, pszCardServerURL, pszLogXml, szRead, true, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadInfoForXJLocal");
	return getInfoForXJ(szRead, xml, true);
}

int __stdcall iReadOnlyHISLocal(char *pszLogXml, char *xml)
{
	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	int status = readHISBaseInfo("" , "", pszLogXml, szRead, false, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	g_rwFlag = 0;
	strcpy(g_processName, "iReadOnlyHISLocal");
	return getHISInfo(szRead, xml, true);
}
