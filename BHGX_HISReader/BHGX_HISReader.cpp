// BHGX_HISReader.cpp : 定义 DLL 应用程序的入口点。
//

//#include "stdafx.h"
// Windows 头文件:
#include <windows.h>
#include "../BHGX_CardLib/src/BHGX_CardLib.h"
#include "../BHGX_CardLib/src/liberr.h"
#include "tinyxml/headers/tinyxml.h"
#include "resource.h"
#include "BHGX_HISReader.h"
#include <map>
using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#ifdef _DEBUG
#pragma comment(lib, "../Debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../Release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)

#define SAFE_DELETE(a)\
	if (a != NULL)\
{\
	delete a;\
	a = NULL;\
}\

struct HISReader 
{
	std::string strValue;
	std::string strSource;
	std::string strDesc;
};
std::map<int, HISReader> mapHISReader;

bool IsMedicalID(const std::string &strID)
{
	for (size_t i=0; i<strID.size(); ++i)
	{
		char ID = strID[i];
		if (ID != 0x30)
		{
			return true;
		}
	}
	return false;
}

int GetQueryInfo(char *QueryXML, std::string &szCardNO)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(QueryXML);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment)
	{
		Colum = Segment->FirstChildElement();
		if (Colum)
		{
			szCardNO = Colum->Attribute("VALUE");
			return 0;
		}
	}
	return -1;;
}

void CreateResponXML(int nID, const char *szResult, char *RetXML)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlElement *Segment;
	TiXmlDeclaration HeadDec;
	TiXmlPrinter Printer;

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("CardProcess");
	Segment = new TiXmlElement("ReturnInfo");
	Segment->SetAttribute("ID", nID);
	Segment->SetAttribute("Desc", szResult);

	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(RetXML, Printer.CStr());
}

void ParseReaderXML(const char *szReader, std::map<int, HISReader> &mapReader)
{
	std::map<int, std::string> mapAll;
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(szReader);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment)
	{
		Colum = Segment->FirstChildElement();
		std::string strValue;
		while (Colum)
		{
			int nID = atoi(Colum->Attribute("ID"));
			strValue = Colum->Attribute("VALUE");
			mapAll[nID] = strValue;
			Colum = Colum->NextSiblingElement();
		}
	}

	if (mapAll.size() > 0)
	{
		std::map<int, HISReader>::iterator iter = mapReader.begin();
		for (; iter != mapHISReader.end(); ++iter)
		{
			int nID = iter->first;
			HISReader &stHIS = iter->second;
			stHIS.strValue = mapAll[nID];
		}
	}
}

void CreateHISReaderXML(const std::map<int, HISReader> &mapReader, char *strHISReader)
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
	for (; iter != mapReader.end(); ++iter)
	{
		const HISReader &stHIS = iter->second;
		Column = new TiXmlElement("COLUMN");
		Column->SetAttribute("ID", iter->first);
		Column->SetAttribute("DESC", stHIS.strDesc.c_str());
		Column->SetAttribute("VALUE", stHIS.strValue.c_str());
		Segment->LinkEndChild(Column);
	}
	
	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(strHISReader, Printer.CStr());
}

int GetHISInfo(const char *szReader, char *strHISReader)
{
	if (mapHISReader.size() == 0)
	{
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
	}
	
	ParseReaderXML(szReader, mapHISReader);

	//if (!IsMedicalID(mapHISReader[7].strValue))
	//{
	//	CreateResponXML(-1, "参合号为空", strHISReader);
	//	return -1;
	//}
	CreateHISReaderXML(mapHISReader, strHISReader);

	return 0;
}

int GetInfoForXJ(const char *szReader, char *strHISReader)
{
	if (mapHISReader.size() == 0)
	{
		mapHISReader[7].strDesc = "医疗证号";
		mapHISReader[9].strDesc = "姓名";
		mapHISReader[10].strDesc = "身份证号";
		mapHISReader[12].strDesc = "性别";
		mapHISReader[11].strDesc = "出生日期";
		mapHISReader[17].strDesc = "民族";
	}

	ParseReaderXML(szReader, mapHISReader);

	//if (!IsMedicalID(mapHISReader[7].strValue))
	//{
	//	CreateResponXML(-1, "参合号为空", strHISReader);
	//	return -1;
	//}

	CreateHISReaderXML(mapHISReader, strHISReader);

	return 0;
}

int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	if (xml == NULL)
	{
		return -1;
	}

	mapHISReader.clear();

	int nInit = iCardInit();
	if (nInit != 0)
	{
		CreateResponXML(CardInitErr, err(CardInitErr), xml);
		return CardInitErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("MEDICARECERTIFICATENO", szQuery);
	if (n != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::string strMedicalID;
	GetQueryInfo(szQuery, strMedicalID);

	if (!IsMedicalID(strMedicalID))
	{
		CreateResponXML(CardReadErr, "参合号不存在", xml);
		return CardReadErr;
	}

	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	nInit = iReadInfo(2, szRead);
	if (nInit != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}

	if (CardProcSuccess != iCheckMsgForNH(pszCardCheckWSDL, pszCardServerURL, xml))
	{
		return CardCheckError;
	}

	memset(xml, 0, strlen(xml)+1);

	return GetHISInfo(szRead, xml);
}

int __stdcall iReadOnlyHIS(char *xml)
{
	if (xml == NULL)
	{
		return -1;
	}

	mapHISReader.clear();

	int nInit = iCardInit();
	if (nInit != 0)
	{
		CreateResponXML(CardInitErr, err(CardInitErr), xml);
		return CardInitErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("MEDICARECERTIFICATENO", szQuery);
	if (n != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::string strMedicalID;
	GetQueryInfo(szQuery, strMedicalID);

	if (!IsMedicalID(strMedicalID))
	{
		CreateResponXML(CardReadErr, "参合号不存在", xml);
		return CardReadErr;
	}

	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	nInit = iReadInfo(2, szRead);
	if (nInit != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}

	memset(xml, 0, strlen(xml)+1);

	return GetHISInfo(szRead, xml);
}

int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	if (xml == NULL)
	{
		return -1;
	}

	mapHISReader.clear();

	int nInit = iCardInit();
	if (nInit != 0)
	{
		CreateResponXML(CardInitErr, err(CardInitErr), xml);
		return CardInitErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	int n = iQueryInfo("MEDICARECERTIFICATENO", szQuery);
	if (n != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}
	std::string strMedicalID;
	GetQueryInfo(szQuery, strMedicalID);

	if (!IsMedicalID(strMedicalID))
	{
		CreateResponXML(CardReadErr, "参合号不存在", xml);
		return CardReadErr;
	}

	char szRead[8092];
	memset(szRead, 0, sizeof(szRead));
	nInit = iReadInfo(2, szRead);
	if (nInit != 0)
	{
		CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardReadErr;
	}

	if (CardProcSuccess != iCheckMsgForNH(pszCardCheckWSDL, pszCardServerURL, xml))
	{
		return CardCheckError;
	}

	memset(xml, 0, strlen(xml)+1);

	return GetInfoForXJ(szRead, xml);
}

