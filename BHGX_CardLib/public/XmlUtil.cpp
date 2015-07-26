#include "XmlUtil.h"
#include "Markup.h"
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

int CXmlUtil::GetQueryInfos(char *QueryXML, std::map<std::string, std::string> &mapQueryInfo)
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
			std::string szSource = Colum->Attribute("SOURCE");
			//Colum->Attribute("ID", &id);
			mapQueryInfo[szSource] = Colum->Attribute("VALUE");
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


void CXmlUtil::parseHISXml(const char *szReader, std::map<std::string, ColumInfo> &mapColumInfo)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(szReader);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	while(Segment){
		Colum = Segment->FirstChildElement();
		while (Colum){
			ColumInfo info;
			info.ID = atoi(Colum->Attribute("ID"));
			info.strSource = Colum->Attribute("SOURCE");
			info.strValue = Colum->Attribute("VALUE");
			mapColumInfo[info.strSource] = info;
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
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

int  CXmlUtil::CheckCardXMLValid(std::string &pszCardXml)
{
	std::string strCardXML = pszCardXml.substr(0, pszCardXml.find(">"));
	strCardXML = strlwr((char*)strCardXML.c_str());
	int pos = strCardXML.find("gb2312");

	std::string &dstXml = pszCardXml;
	if (pos < 0){
		dstXml.replace(0, dstXml.find(">")+1, 
			"<?xml version=\"1.0\" encoding=\"gb2312\" ?>");
	}

	CMarkup xml;
	xml.SetDoc(dstXml.c_str());
	if (!xml.FindElem("SEGMENTS")){
		return -1;
	}
	xml.IntoElem();
	if (! xml.FindElem("SEGMENT")){
		return -1;
	}

	xml.IntoElem();
	if (! xml.FindElem("COLUMN")) {
		return -1;
	}

	if (!xml.FindAttrib("ID") ||
		!xml.FindAttrib("VALUE")) {
			return -1;
	}
	xml.OutOfElem();
	xml.OutOfElem();
	return 0;
}