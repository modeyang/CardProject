#include "XmlUtil.h"
#include "../tinyxml/headers/tinyxml.h"

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#pragma warning (disable : 4996)

CXmlUtil::CXmlUtil(void)
{
}

CXmlUtil::~CXmlUtil(void)
{
}


int CXmlUtil::GetQueryInfoForOne(char *QueryXML, std::string &szCardNO)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(QueryXML);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment){
		Colum = Segment->FirstChildElement();
		if (Colum){
			szCardNO = Colum->Attribute("VALUE");
			return 0;
		}
	}
	return -1;;
}

int CXmlUtil::GetQueryInfos(char *QueryXML, std::map<int, std::string> &mapQueryInfo)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(QueryXML);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	while(Segment){
		Colum = Segment->FirstChildElement();
		while (Colum){
			int id;
			Colum->Attribute("ID", &id);
			mapQueryInfo[id] = Colum->Attribute("VALUE");
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
	return 0;
}

void CXmlUtil::CreateResponXML(int nID, const char *szResult, char *RetXML)
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


void CXmlUtil::parseHISXml(const char *szReader, std::map<int, std::string> &mapAll)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(szReader);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment){
		Colum = Segment->FirstChildElement();
		std::string strValue;
		while (Colum){
			int nID = atoi(Colum->Attribute("ID"));
			strValue = Colum->Attribute("VALUE");
			mapAll[nID] = strValue;
			Colum = Colum->NextSiblingElement();
		}
	}
	Segment = Segment->NextSiblingElement();
	if (Segment) {
		Colum = Segment->FirstChildElement();
		mapAll[39] = Colum->Attribute("VALUE");
	}
}


int CXmlUtil::paserLogXml(char *pszLogXml, std::map<int, std::map<int, std::string> > &mapLogConfig)
{
	std::map<int, std::string> mapConfig;
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(pszLogXml);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	while (Segment) {
		int SegID = atoi(Segment->Attribute("ID"));
		std::string desc = Segment->Attribute("DESC");
		Colum = Segment->FirstChildElement();
		while (Colum) {
			int ColumID = atoi(Colum->Attribute("ID"));
			mapConfig[ColumID] = Colum->Attribute("VALUE");
			Colum = Colum->NextSiblingElement();
		}
		mapLogConfig[SegID] = mapConfig;
		Segment = Segment->NextSiblingElement();
	}
	return 0;
}