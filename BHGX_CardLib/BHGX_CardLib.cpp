#include <vector>
#include <stdio.h>
#include <time.h>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>

#include "Card.h"
#include "adapter.h"
#include "M1Card.h"
#include "CPUCard.h"
#include "CPUAdapter.h"
#include "BHGX_CardLib.h"
#include "SegmentHelper.h"
#include "resource.h"
#include "BHGX_Printer.h"
#include "StringUtil.h"
#include "WebServiceAssist.h"
#include "public/debug.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/Markup.h"
#include "public/Authority.h"
#include "public/TimeUtil.h"
#include "public/XmlUtil.h"
#include "public/ExceptionCheck.h"
#include "public/ConvertUtil.h"
#include "public/LogHelper.h"
#include "Encry/DESEncry.h"
#include "tinyxml/headers/tinyxml.h"
using namespace std;

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#define ISSCANCARD {							\
	if (iScanCard() != 0)						\
		return CardScanErr;						\
}												\
											
#define ISGWCARD(a) ((a[0]) == '1')

#define SCANCARD_XML(xml)											\
	if (xml == NULL) return CardInputParamError;					\
	if (iScanCard() != 0) {											\
		CXmlUtil::CreateResponXML(CardReadErr, "寻卡失败", xml);	\
		return CardScanErr;											\
	}	

#if (CPU_16K)
#define CPU_CAN_WRITE_SECTION	0
#define CPU_BIN_SECTION			12
#else 
#define CPU_CAN_WRITE_SECTION	3
#define CPU_BIN_SECTION			11
#endif


#define TIMEOUT		15000
#define SEGBASE		100
#define ASSERT_INIT(a)					\
	if (a != TRUE){						\
		return CardInitErr;				\
	}									\


#define ASSERT_OPEN(a)					\
	if (a != TRUE)   {					\
		return CardNotOpen;				\
	}									\

struct QueryColum
{
	int nSegID;
	int nID;
	std::string szSource;
	std::string szValue;
	QueryColum()
		:nSegID(-1)
		,nID(-1)
		,szSource("unknown")
		,szValue("unknown")
	{}
	QueryColum(int segId, int id, char *source, char *value)
		:nSegID(segId)
		,nID(id)
		,szSource(source)
		,szValue(value)
	{}

	QueryColum(const QueryColum &item)
		:nID(item.nID),
		nSegID(item.nSegID),
		szSource(item.szSource),
		szValue(item.szValue)
	{}

	QueryColum & operator=(const QueryColum &item) {
		if (this != &item) {
			this->nID = item.nID;
			this->nSegID = item.nSegID;
			this->szSource = item.szSource;
			this->szValue = item.szValue;
		}
		return *this;
	}
	bool operator < (struct QueryColum &item) {
		return nID < item.nID;
	}
};


//cpu 与 M1相互转化时的对应关系
struct dataItem 
{
	int nId;	
	int nCpuId;
	std::string source;    //M1
	std::string value;
	dataItem ()
		:nId(-1)
		,nCpuId(-1)
	{}
	dataItem(const dataItem &item)
		:nId(item.nId),
		nCpuId(item.nCpuId),
		source(item.source),
		value(item.value)
	{}

	dataItem & operator=(const dataItem &item) {
		if (this != &item) {
			this->nId = item.nId;
			this->nCpuId = item.nCpuId;
			this->source = item.source;
			this->value = item.value;
		}
		return *this;
	}
	bool operator < (struct dataItem &item) {
		return nId < item.nId;
	}
};


//source 与 value的默认对应值
std::map<string, QueryColum> g_sourceValueMap;
std::map<string, int> g_segMap;
std::string g_SamID = "";


CBHGX_Printer *m_pBHPrinter = NULL;
BOOL g_bPreLoad = FALSE;
BOOL g_bCardOpen = FALSE;
BOOL g_bAuthed   = TRUE;

static CardOps *g_CardOps = NULL;
static CardOps *g_CpuCardOps = NULL;
static CardOps *g_M1CardOps = NULL;
static CSegmentHelper *g_SegHelper = NULL;

/**
 * 全局的数据结构
 */
struct XmlProgramS *g_XmlListHead = NULL;

//将CPU的ID号映射到M1中，其中将健康档案号存放在0, 
//CPU卡中发卡机构名称,发卡机构代码分别存放在2,5中
int cpuInM1Index[29] = {839,108, 103, -1, -1, 104, -1, 840,
						-1, 211, 215, 214, 212, -1, -1, -1,
						-1, 213, -1, 735, -1, -1,416, -1,
						625, 627, -1, -1, -1,};
#define  CPU_WRITE_DOWN  16

char *M1Reserver[29] = {
	"","","","000000","000000","","","",
	"","","","","","","","",
	"","","","","","","","",
	"","","","","",
};

char *M1SourceReserver[29] = {
	"HEALTHRECORDNO","CARDNO","RESERVE1","PASSWORD","STAGENO",
	"RESERVE2","ORGANIZATIONNO","MEDICARECERTIFICATENO",
	"LASTUPDATEDATE","NAME","IDNUMBER","BIRTHDAY","GENDER",
	"HOUSEHOLDERRELATION","PEOPLETYPE","HOUSEHOLDTYPE",
	"CIVILAFFAIRSFLAG","NATIONALITY","HEALTHSTATE","CIVILSTATE",
	"MARRYRELATIVESFLAG","FIRSTPARTAKEDATE","PHONENO","RESERVE4",
	"LINKMANNAME","LINKMANPHONENO","CHRONICCODE1","CHRONICCODE2","RESERVE5",
};

static void 
DestroyList(struct XmlSegmentS *listHead, int mode);

static void 
ReadConfigFromReg(char *name);

static int
QueryItem(CardType type, const char *name, char *xml,
		   int &nLen, QueryColum &stQuery);

static int
QueryItem(CardType type, const char *name, char *xml, int &nLen);

static int 
ParseValueQuery(char *source, std::string &szResult);

static struct XmlColumnS * 
M1QueryItem(const char *name, char *xml, int &nLen);

static struct XmlColumnS * 
CpuQueryItem(const char *name, char *xml, int &nLen);

static int 
iCreateScanXml(int card_info, char *xml);

static int 
iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length);


//*************************************

static BOOL isWriteable(int cpuIndex) 
{
	return cpuIndex < CPU_WRITE_DOWN;
}

static int M1ConvertXmlByArray(const std::vector<struct dataItem> &vecItem, int segId, char *xml, int *length) 
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;
	char buf[10];

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	memset(buf, 0, 10);
	sprintf_s(buf, 10, "%d", segId);

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID",buf);

	for (size_t i=1; i<vecItem.size(); i++) {
		struct dataItem  item = vecItem[i];
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", item.nId);

		Cloumn = new TiXmlElement("COLUMN");
		Cloumn->SetAttribute("ID", buf);
		//Cloumn->SetAttribute("SOURCE", item.source.c_str());
		Cloumn->SetAttribute("VALUE", item.value.c_str());

		Segment->LinkEndChild(Cloumn); 
	}
	RootElement->LinkEndChild(Segment);
	if (vecItem[1].value.size() > 0 ) {
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", 5);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);
		Cloumn = new TiXmlElement("COLUMN");

		sprintf_s(buf, 10, "%d", 1);
		Cloumn->SetAttribute("ID", buf);
		Cloumn->SetAttribute("VALUE", vecItem[0].value.c_str());
		Segment->LinkEndChild(Cloumn); 

		RootElement->LinkEndChild(Segment);
	}

	XmlDoc->LinkEndChild(RootElement);
	  
	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);
	return 0;

}


static int GetCpuReadFlag(const std::map<int, dataItem> &mapInfo) 
{
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();
	int offset = 0, flag = 0;
	for (; iter != mapInfo.end(); iter++)
	{
		int id = iter->first;
		if (id < 16 && id != 7) {
			continue;
		} else if (id < 21) {
			offset = 3;
		} else if (id < 25) {
			offset = 4;
		} else if (id < 34) {
			offset = 5;
		} else if (id < 37) {
			offset = 6;
		} else if (id < 41) {
			offset = 7;
		} else if (id < 56) {
			offset = 8;
		} 
		SETBIT(flag, offset);
	}
	return flag;
}

static void CpuConvertRetPos(std::vector<QueryColum> &vec) 
{
	std::map<string, int>::iterator iter = g_segMap.begin();
	for (size_t i=0; i<vec.size(); i++) 
	{
		QueryColum &query = vec[i];
		iter = g_segMap.find(query.szSource);
		if (iter != g_segMap.end()) {
			query.nSegID = iter->second / SEGBASE;
			query.nID = iter->second % SEGBASE;
		}
	}
}

static void xml2Map(char *src, std::map<int, dataItem> &mapInfo, CardType type, bool isRead = false)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;

	XmlDoc.Parse(src);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return;
	}
	Segment = RootElement->FirstChildElement();

	while (Segment)
	{
		int nSeg = atoi(Segment->Attribute("ID"));
		Colum = Segment->FirstChildElement();
		while (Colum) {
			int nColumID = atoi(Colum->Attribute("ID"));
			std::string strColum = Colum->Attribute("VALUE");
			struct dataItem item;
			item.nId = nColumID;
			item.value = strColum;
			if (isRead) {
				item.source = Colum->Attribute("SOURCE");
			}
			
			if (type == eCPUCard) {
				mapInfo[nColumID] = item;
			} else {
				mapInfo[nColumID + nSeg * SEGBASE] = item;
			}
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
}

static int ParseValueQuery(char *source, std::string &szResult)
{
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	int n = iQueryInfo(source,  szQuery);
	if (n != 0){
		return n;
	}
	CXmlUtil::GetQueryInfoForOne(szQuery, szResult);
	return 0;
}

void M12CpuMap(const std::map<int, dataItem> &mapInfo, 
			   std::map<int, dataItem> &mapCpuInfo)
{
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();
	for (; iter != mapInfo.end(); iter++) {
		int id = iter->first;
		dataItem item = iter->second;
		int SegId = 0, ColmnId = 0;
		SegId = id / SEGBASE;
		ColmnId = id % SEGBASE;
		if (SegId == 5) {
			if (cpuInM1Index[0] != -1) {
				item.nCpuId = cpuInM1Index[0] % SEGBASE;
				mapCpuInfo[item.nCpuId] = item;
			} 

		} else if (SegId == 2){
			if (cpuInM1Index[ColmnId] != -1) {
				item.nCpuId = cpuInM1Index[ColmnId] % SEGBASE;
				mapCpuInfo[item.nCpuId] = item;
			}
		}
	}
}

static int Cpu2M1Xml(char *src, char *dest, int *length) 
{
	std::map<int, dataItem> mapCpuInfo;
	xml2Map(src, mapCpuInfo, eM1Card, false);
	std::vector<struct dataItem> vecData;
	for (int i=0; i<29; i++) {
		struct dataItem item;
		item.nId = i;
		std::map<int, dataItem>::iterator iter = mapCpuInfo.end();
		int cpuId = cpuInM1Index[i];
		item.nCpuId = cpuId;
		if (cpuId == -1) {
			item.value = M1Reserver[i];
			item.source = M1SourceReserver[i];
		} else {
			iter = mapCpuInfo.find(cpuId);
			if (iter != mapCpuInfo.end()) {
				item.value = iter->second.value;
				item.source = iter->second.source;
			} else {
				item.value = M1Reserver[i];
				item.source = M1SourceReserver[i];
			}
		}
		vecData.push_back(item);
	}
	return M1ConvertXmlByArray(vecData, 2, dest, length);
}

static int fillCpuXml(char *xml, const std::map<int, dataItem> &mapInfo) 
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	TiXmlPrinter Printer;
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();

	XmlDoc.Parse(xml);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return -1;
	}
	Segment = RootElement->FirstChildElement();

	while (Segment)
	{
		int nSeg = atoi(Segment->Attribute("ID"));
		Colum = Segment->FirstChildElement();
		while (Colum)
		{
			int nColumID = atoi(Colum->Attribute("ID"));
			iter = mapInfo.find(nColumID);
			if (iter != mapInfo.end()) {
				const dataItem &item = iter->second;
				Colum->SetAttribute("VALUE", item.value.c_str());
			}
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
	XmlDoc.Accept(&Printer);
	strcpy(xml, Printer.CStr());
	return 0;
}

/**
 *
 */
static void DestroyList(struct XmlSegmentS *listHead, int mode)
{
	struct XmlSegmentS	*CurrSegmentElement = NULL;
	struct XmlSegmentS	*TempSegmentElement = NULL;
	struct XmlColumnS	*CurrColumnElement	= NULL;
	struct XmlColumnS	*TempColumnElement	= NULL;

	CurrSegmentElement = listHead;
	while(CurrSegmentElement)
	{
		CurrColumnElement = CurrSegmentElement->ColumnHeader;
		while(CurrColumnElement)
		{
			TempColumnElement = CurrColumnElement;
			CurrColumnElement = CurrColumnElement->Next;
			if (mode) {
				SAFE_DELETE_C(TempColumnElement->Value);
			}
			SAFE_DELETE_C(TempColumnElement);
		}
		
		TempSegmentElement = CurrSegmentElement;
		CurrSegmentElement = CurrSegmentElement->Next;

		SAFE_DELETE_C(TempSegmentElement);
	}

	return;
}



static void ReadConfigFromReg(char *name)
{
	HKEY RootKey;
	HKEY hKey;
	char szValue[256];
	DWORD dwLen;

	RootKey = HKEY_CURRENT_USER;
	DWORD dwDesc;
	DWORD dwType = REG_SZ;
	if (ERROR_SUCCESS != RegOpenKeyEx(RootKey, (LPCSTR)(MainKey), 
		(DWORD)strlen(MainKey), KEY_READ | KEY_WRITE, &hKey))
	{
		if (ERROR_SUCCESS != RegCreateKeyEx(RootKey, MainKey, 0, MainKey, 0,
			KEY_READ | KEY_WRITE, NULL, &hKey, &dwDesc)){
			RegCloseKey(hKey);
			return;
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "Config", NULL, dwType, 
										  (PBYTE)CONFIG, (DWORD)strlen(CONFIG)))
		{
			RegCloseKey(hKey);
			return;
		}
	}

	dwLen = 256;
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, "Config", NULL, 
										&dwType, (PBYTE)szValue,&dwLen))
	{
		RegCloseKey(hKey);
		return;
	}
	szValue[dwLen] = 0;
	RegCloseKey(hKey);
	strcpy_s(name, NAME_MAX_LEN, szValue);
}


static int QueryItem(CardType type, const char *name, char *xml, int &nLen, QueryColum &stQuery)
{
	struct XmlColumnS *pColum = NULL;
	if (type == eCPUCard) {
		pColum = CpuQueryItem(name, xml, nLen);
	} else {
		pColum = M1QueryItem(name, xml, nLen);
	}
	if (pColum == NULL) {
		std::map<string, QueryColum>::iterator iter = 
			g_sourceValueMap.find(name);
		if (iter != g_sourceValueMap.end()) {
			stQuery = iter->second;
		} else {
			stQuery = QueryColum();
		}
		return 0;
	}
	stQuery.nSegID = pColum->parent->ID;
	stQuery.nID = pColum->ID;
	stQuery.szSource = pColum->Source;
	stQuery.szValue = xml;
	if (type == eCPUCard) {
		SAFE_DELETE_C(pColum->Value);
		SAFE_DELETE_C(pColum);
	}
	return 0;
}

static int QueryItem(CardType type, const char *name, char *xml, int &len) 
{
	if (type == eCPUCard) {
		CpuQueryItem(name, xml, len);
	} else {
		M1QueryItem(name, xml, len);
	}
	return 0;
}

static struct XmlColumnS* CpuQueryItem(const char *name, char *xml, int &nLen)
{
	struct RWRequestS	*RequestList = NULL;
	struct XmlColumnS * queryItem = NULL;
	struct XmlColumnS *pBinColum = NULL;
	int status = -1;

	//find segId from column name
	struct XmlSegmentS *pFindSeg = g_SegHelper->getSegmentByColumName(g_XmlListHead->SegHeader, name);
	if (pFindSeg == NULL) {
		nLen = -1;
		return NULL;   
	}

	//bin file can read by random , while others must read by one section and extract own name. 
	if (pFindSeg->datatype == eBinType) {
		pBinColum = g_SegHelper->FindColumByColumName(pFindSeg, name);
		assert(pBinColum);

		RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(RequestList, 0, sizeof(struct RWRequestS));
		RequestList->datatype = pFindSeg->datatype;
		RequestList->itemtype = pBinColum->CheckInfo.CpuInfo.itemtype;
		RequestList->length = pBinColum->CheckInfo.CpuInfo.ColumnByte;
		RequestList->nColumID = pBinColum->ID;
		RequestList->nID = pFindSeg->ID;
		RequestList->offset = pBinColum->Offset;
		RequestList->value = (BYTE*)pBinColum->Value;
	} else {
		assert(pFindSeg);

		// 获取读写链表
		RequestList = apt_CreateRWRequest(pFindSeg , 0, g_CardOps->cardAdapter->type);
	}

	// 设备的真实读取
	int res = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	XmlSegmentS *pSeg = pFindSeg;
	
	while(pSeg)
	{
		int nSeg = pSeg->ID;
		XmlColumnS *pColum = pSeg->ColumnHeader;
		while(pColum)
		{
			if (strcmp(name, pColum->Source) == 0) {
				nLen = strlen(pColum->Value);
				memcpy(xml, pColum->Value, nLen);

				queryItem = g_SegHelper->CloneColmn(pColum, 1);
				memcpy(queryItem->Value, pColum->Value, nLen);
				goto done;
			}
			pColum = pColum->Next;
		}
		pSeg = pSeg->Next;
	}

done:
	// 销毁读写请求链表
	apt_DestroyRWRequest(RequestList, 0);
	DestroyList(pFindSeg, 1);
	return queryItem;
}


static struct XmlColumnS*   M1QueryItem(const char *name, char *xml, int &nLen)
{
	struct RWRequestS	*RequestList = NULL;
	struct XmlColumnS   *queryItem = NULL;
	RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
	memset(RequestList, 0, sizeof(struct RWRequestS));
	unsigned char buf[160];
	XmlSegmentS *pSeg = g_XmlListHead->SegHeader;
	int nRet = -1;

	while(pSeg)
	{
		int nSeg = pSeg->ID;
		XmlColumnS *pColum = pSeg->ColumnHeader;
		while(pColum)
		{
			if (strcmp(name, pColum->Source) == 0){
				RequestList->offset = pColum->Offset;
				RequestList->length = pColum->CheckInfo.M1Info.ColumnBit;
				RequestList->value = buf;
				memset(buf, 0, 160);

				if (RequestList->length > 0){
					nRet = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);
					if (nRet != 0){
						return NULL;
					}
					if (std::string(pColum->Source) == std::string("NAME") || 
						std::string(pColum->Source) == std::string("LINKMANNAME")){
						std::string szName;
						char strName[160];
						memset(strName, 0, sizeof(strName));
						int nLen = 0;
						memcpy(strName, buf, strlen((char*)buf) +1);
						if (Is_IntName(strName)){
							szName = CConvertUtil::uf_gbk_int_covers(strName, "togbk");
						} else {
							szName = strName;
						}
						memcpy(buf, szName.c_str(), szName.size());
						buf[szName.size()] = 0;
					}
					memcpy(xml, buf, strlen((char*)buf));
					xml[strlen((char*)buf)] = 0;
					nLen = (int)strlen(xml);
					queryItem = pColum;
					goto done;
				}
			}
			pColum = pColum->Next;
		}
		pSeg = pSeg->Next;
	}

done:
	SAFE_DELETE_C(RequestList);
	return queryItem;
}

static int iCreateScanXml(char * card_info, char *xml)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment, *Segment1, *Segment2;
	TiXmlPrinter Printer;

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID",0);
	Segment->SetAttribute("SOURCE", "CARDCATEGORY");

	std::vector<std::string> vec_info;
	vec_info = split(std::string(card_info), "|");
	int card_type = atoi(vec_info[0].c_str());
	if (card_type == eCPUCard) {
		Segment->SetAttribute("VALUE", "CPU");
	} else if (card_type == eM1Card) {
		Segment->SetAttribute("VALUE", "M1");
	} else {
		Segment->SetAttribute("VALUE", "UNKNOWN");
	}

	RootElement->LinkEndChild(Segment);

	if (card_type == eCPUCard) {
		if (vec_info.size() > 2) {
			Segment1 = new TiXmlElement("SEGMENT");
			Segment1->SetAttribute("ID",1);
			Segment1->SetAttribute("SOURCE", "PSAM");
			Segment1->SetAttribute("VALUE", vec_info[1].c_str());
			RootElement->LinkEndChild(Segment1);
			// get sam ID
			g_SamID = vec_info[1];

			Segment2 = new TiXmlElement("SEGMENT");
			Segment2->SetAttribute("ID",1);
			Segment2->SetAttribute("SOURCE", "ATR");
			Segment2->SetAttribute("VALUE", vec_info[2].c_str());
			RootElement->LinkEndChild(Segment2);
		}

	}
	XmlDoc->LinkEndChild(RootElement);

	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	int length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), length);
	return 0;
}

/**
*
*/
static int iCreateXmlByVector(const vector<QueryColum>&  vecQuery, char *xml, int *length)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;

	std::map<int,std::vector<QueryColum> > mapSeg;
	for (size_t i=0; i<vecQuery.size(); ++i){
		QueryColum stQuery = vecQuery[i];
		mapSeg[stQuery.nSegID].push_back(stQuery);
	}

	int result = 0;
	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");
	std::map<int,std::vector<QueryColum> >::iterator iter = mapSeg.begin();
	for (; iter!=mapSeg.end(); ++iter)
	{
		int nSeg = iter->first;
		std::vector<QueryColum> &vecQuery = iter->second;
		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",nSeg);

		for(size_t i = 0; i < vecQuery.size(); i++)
		{
			QueryColum &stQuery = vecQuery[i];
			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", stQuery.nID);
			Cloumn->SetAttribute("SOURCE", stQuery.szSource.c_str());
			Cloumn->SetAttribute("VALUE", stQuery.szValue.c_str());

			Segment->LinkEndChild(Cloumn);
		}
		RootElement->LinkEndChild(Segment);
	}

	XmlDoc->LinkEndChild(RootElement);

	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return result;
}


/**
 *
 */
int __stdcall iGetCardVersion(char *pszVersion)
{
	int res = 0;

	memcpy(pszVersion, "2.0.1.0", sizeof("2.0.1.0"));

	return res;
}

int __stdcall iCardCompany(char *szCompanyXml)
{
	return 0;
}

/**
 *
 */
int __stdcall iCardInit(char *xml)
{
	char path[MAX_PATH] = {};
	GetSystemDirectory(path, MAX_PATH);

	// 对设备进行初始化
	g_bPreLoad = (apt_InitCoreDevice(path)==0);
	g_bCardOpen = g_bPreLoad;

	return g_bPreLoad==TRUE ? 0:-1;
}

int __stdcall iCardDeinit()
{
	if (g_CpuCardOps) {
		DestroyList(g_CpuCardOps->programXmlList->SegHeader, 0);
		CPUClear();
		g_CpuCardOps = NULL;
	}

	if (g_M1CardOps) {
		DestroyList(g_M1CardOps->programXmlList->SegHeader, 0);
		M1clear();
		g_M1CardOps = NULL;
	}
	//SAFE_DELETE(g_SegHelper);

	g_XmlListHead = NULL;
	g_bPreLoad = FALSE;
	g_bCardOpen = FALSE;
	return apt_CloseCoreDevice();
}

int __stdcall iCardOpen()
{
   ASSERT_INIT(g_bPreLoad);
   if (g_bCardOpen) {
	   return 0;
   }

   int ret =  apt_OpenDev();
   if (!ret) {
	   g_bCardOpen = TRUE;
   }
   return ret;
}

int __stdcall iCardClose()
{
	ASSERT_OPEN(g_bCardOpen);
	int ret =  apt_CloseDev();
	if (!ret) {
		g_bCardOpen = FALSE;
	}
	return ret;
}


int __stdcall iScanCard(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	char card_info[512];
	memset(card_info, 0, sizeof(card_info));
	int status = apt_ScanCard(card_info);
	if (status != CardProcSuccess) {
		if (xml != NULL) CXmlUtil::CreateResponXML(status, err(status), xml);
		LOG_ERROR(err(status));
		return status;
	}

	LOG_DEBUG(card_info);

	// scancard success
	if (xml != NULL) {
		iCreateScanXml(card_info, xml);
	} else {
		std::vector<std::string> vec_info;
		vec_info = split(std::string(card_info), "|");
		int card_type = atoi(vec_info[0].c_str());
		if (card_type == eCPUCard) {
			// get sam ID
			g_SamID = vec_info[1];
		}
	}
	return CardProcSuccess;
}
/**
*
*/
char* __stdcall err(int errcode)
{
	if(errcode < 0)
		errcode = -errcode;

	return _err(errcode);
}

int __stdcall iCardIsEmpty()
{
	ASSERT_OPEN(g_bCardOpen);

	ISSCANCARD;

	char strCardNo[20];
	memset(strCardNo, 0, sizeof(strCardNo));
	int nLen = 0;
	int status = 0;
	if (g_CardOps->cardAdapter->type == eM1Card) {
		status = QueryItem(g_CardOps->cardAdapter->type, "CARDNO", strCardNo, nLen);
	} else {
		status = QueryItem(g_CardOps->cardAdapter->type, "CARDSEQ", strCardNo, nLen);
	}

	if (status != CardProcSuccess) {
		return status;
	}

	status = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, '0');
	strlwr(strCardNo);
	int stat = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, 'f');
	return (status & stat) == 0 ? 0 : CardIsNotEmpty;
}

/**
*
*/

static int _iReadQuery(int flag, std::vector<QueryColum> &vecQuery) 
{
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int status = CardProcSuccess;

	list = g_SegHelper->GetXmlSegmentByFlag(flag);
	if (list == NULL) {
		return CardXmlErr;
	}

	// 获取读写链表
	RequestList = apt_CreateRWRequest(list, 0, g_CardOps->cardAdapter->type);
	if (RequestList == NULL) {
		status = CardXmlErr;
		goto done;
	}

	// 设备的真实读取
	status = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	// get query column 
	for (int i=0; i<vecQuery.size(); i++)
	{
		QueryColum &queryItem = vecQuery[i];
		struct XmlColumnS* xmlColumn = g_SegHelper->FindColumByColumName(list, queryItem.szSource.c_str());
		if (xmlColumn != NULL) {
			queryItem.nID = xmlColumn->ID;
			queryItem.nSegID = xmlColumn->parent->ID;
			queryItem.szSource = xmlColumn->Source;
			if (g_CardOps->cardAdapter->type == eM1Card) {
				queryItem.szValue = CConvertUtil::ConvertZHName(queryItem.szSource.c_str(), xmlColumn->Value);
			} else {
				queryItem.szValue = xmlColumn->Value;
			}
		}
	}

	// 销毁读写请求链表
	apt_DestroyRWRequest(RequestList, 0);

done:
	DestroyList(list, 1);
	return status;
}

	

/**
 *
 */

static int _iReadInfo(int flag, char *xml, int del_flag=-1) 
{
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int status = CardProcSuccess;
	
	list = g_SegHelper->GetXmlSegmentByFlag(flag);
	if (list == NULL) {
		CXmlUtil::CreateResponXML(CardXmlErr, err(CardXmlErr), xml);
		return CardXmlErr;
	}

	// 获取读写链表
	RequestList = apt_CreateRWRequest(list, 0, g_CardOps->cardAdapter->type);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		CXmlUtil::CreateResponXML(status, err(status), xml);
		LOG_ERROR(err(status));
		goto done;
	}

	// 设备的真实读取
	status = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	// 通过链表产生XML字符串
	g_CardOps->iConvertXmlByList(list, xml, &length, del_flag);

	// 销毁读写请求链表
	apt_DestroyRWRequest(RequestList, 0);

done:
	DestroyList(list, 1);
	return status;
}

int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_OPEN(g_bCardOpen)
	SCANCARD_XML(xml)

	int del_flag    = -1;
	int bNHInfoRead = 0;
	if (g_CardOps->cardAdapter->type == eM1Card ) {

		// 根据Flag产生List,健康档案号在第五区
		if ((flag & 0x2) && !(flag & 0x10)){
			flag = flag | 0x10;
			del_flag = 5;
		}
	} 
#if (CPU_M1 || CPU_8K)
	else {
		if ((flag & 0x2) == 0x2) {
			bNHInfoRead = 1;
			flag = 0x1 | 0x2 | 0x8 | 32 | 64 | 128;
		}
	}
#endif

	char readxml[MAX_BUFF_SIZE];
	ZeroMemory(readxml, sizeof(readxml));

	int status = _iReadInfo(flag, readxml, del_flag);

	//convert cpu xml to m1 xml;
	if(status != 0) {
		strcpy(xml, readxml);
		return CardReadErr;
	}

	if (g_CardOps->cardAdapter->type == eM1Card) {
		strcpy(xml, readxml);
	} else if (g_CardOps->cardAdapter->type == eCPUCard) {
		if (bNHInfoRead == 1) {
			int length = 0;
			Cpu2M1Xml(readxml, xml, &length);
		} else {
			strcpy(xml, readxml);
		}
	} else {}
	return status == CardProcSuccess ? CardProcSuccess : CardReadErr;
}

DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	int status = -1;
	if (flag > 0){
		status = iReadInfo(flag, xml);
	}

	if (flag<=0 && strcmp(name, "") != 0){
		status = iQueryInfo(name, xml);
	}
	return status;
}


/**
 *
 */
static int _iWriteInfo(char *xml) 
{
	struct XmlSegmentS *XmlList;
	struct RWRequestS	*RequestList = NULL;
	int status = CardProcSuccess;
		// 产生List
	XmlList = g_CardOps->iConvertXmltoList(xml);

	if (XmlList == NULL){
		LOG_ERROR(err(CardXmlErr));
		return CardXmlErr;
	}

	// 产生读写链表
	RequestList = apt_CreateRWRequest(XmlList, 0, eCPUCard);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		LOG_ERROR(err(CardMallocFailed));
		goto done;
	}

	// 对设备进行真实的写
	status = g_CardOps->cardAdapter->iWriteCard(RequestList, g_CardOps->cardAdapter);

	// 销毁读写链表
	apt_DestroyRWRequest(RequestList, 0);

done:
	// 销毁XML链表
	DestroyList(XmlList, 1);
	return status;
}


//返回是否CPU可回写， vecFlag 为记录文件  vecBin为bin文件
static int checkCpuWriteXml(char *xmlStr, 
							std::vector<int> &vecFlag,
							std::vector<int> &vecBin)
{
	CMarkup xml;

	xml.SetDoc(xmlStr);
	if (!xml.FindElem("SEGMENTS")){
		return -1;
	}
	xml.IntoElem();
	while (xml.FindElem("SEGMENT")){
		int id = atoi(xml.GetAttrib("ID").c_str());
		if (id <= CPU_CAN_WRITE_SECTION) {
			return 0;
		} else if (id < CPU_BIN_SECTION) {
			vecFlag.push_back(id);
		} else {
			vecBin.push_back(id);
		}
	}
	xml.OutOfElem();
	return 1;
}

static int is_can_write(char *xmlStr, 
						std::map<int, int> &mapRec, 
						std::map<int, int> &mapBin)
{
	CMarkup xml;
	
	xml.SetDoc(xmlStr);
	if (!xml.FindElem("SEGMENTS")){
		return -1;
	}
	xml.IntoElem();
	while (xml.FindElem("SEGMENT")){
		std::map<int, int> sec_counts;

		int id = atoi(xml.GetAttrib("ID").c_str());
		if (id <= CPU_CAN_WRITE_SECTION) {
			return 0;
		} 
		xml.IntoElem();
		int col_counts = 0;
		while (xml.FindElem("COLUMN")){
			col_counts ++;
		};
		if (id < CPU_BIN_SECTION) {
			mapRec.insert(std::make_pair(id, col_counts));
		} else {
			mapBin.insert(std::make_pair(id, col_counts));
		}
		xml.OutOfElem();
	} 
	
	xml.OutOfElem();
	return 1;

}


int __stdcall iWriteInfo(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	int len = strlen(xml) + 1;
	int status = CardProcSuccess;
	
	std::string xmlStr(xml);
	if (len == 1 || CXmlUtil::CheckCardXMLValid(xmlStr) < 0){
		LOG_DEBUG("CardXML:Check Error\n");
		return CardXmlErr;
	}
	
	ISSCANCARD
	isCardAuth(7);
	if (g_CardOps->cardAdapter->type == eM1Card) {
		status =  _iWriteInfo((char*)xmlStr.c_str());
		goto done;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {

		std::map<int, dataItem> mapInfo, mapCpuInfo;
		int flag = 0, isCpuWrite = 0, isRec = 1, isNeedRead = 0;

		std::map<int, int> mapRecFlag;
		std::map<int, int> mapBinFlag;
		isCpuWrite = is_can_write((char*)xmlStr.c_str(), mapRecFlag, mapBinFlag);
		if (isCpuWrite == 1) {
			if (mapBinFlag.size() > 0) {
				isRec = 0;
			}

			if (isRec) {   //记录文件，记录要回写的块
				std::map<int, int>::const_iterator iter = mapRecFlag.begin();
				for (; iter != mapRecFlag.end(); iter++) {
					if (iter->first > CPU_CAN_WRITE_SECTION) {
						SETBIT(flag, (iter->first - 1));
					}

					if (iter->second != get_sec_counts(iter->first)) {
						isNeedRead = 1;
					}
				}
			}
		} 
#if (CPU_M1 || CPU_8K)
		else {

			xml2Map((char*)xmlStr.c_str(), mapInfo, eM1Card, false);

			if (mapInfo.size() > 0) {
				M12CpuMap(mapInfo, mapCpuInfo);
			}
			flag = GetCpuReadFlag(mapCpuInfo);

			//没有找到可以写的字段
			if (flag == 1 || flag == 0) {
				return CardProcSuccess;
			}
		}
#endif

		if (isRec && isNeedRead) {
			xml2Map((char*)xmlStr.c_str(), mapCpuInfo, eCPUCard, false);

			char convertXml[MAX_BUFF_SIZE];
			ZeroMemory(convertXml, sizeof(convertXml));
			status = _iReadInfo(flag, convertXml);
			if (status) {
				return status;
			}

			if (mapCpuInfo.size() > 0) {
				fillCpuXml(convertXml, mapCpuInfo);
			}
			ISSCANCARD;

			status =  _iWriteInfo(convertXml);
		} else {  //cpu bin write
			status =  _iWriteInfo((char*)xmlStr.c_str());
		}
	} 

done:
	return status == CardProcSuccess? CardProcSuccess : CardWriteErr ;
}


/**
*
*/
int __stdcall iQueryInfo(char *name, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	int status = CardProcSuccess;
	char readbuf[256];
	int	nLen = 0;
	bool bReadQuery = false;
	std::vector<std::string> vecQuery;
	std::set<int> segIdColl;
	std::vector<QueryColum> vecResult;
	struct RWRequestS	*RequestList = NULL;

	// need optinum in cpu item query
	vecQuery = split(name, "|");
	for (int i=0; i<vecQuery.size(); i++) {
		int index = g_SegHelper->FindSegIDByColumName(g_XmlListHead->SegHeader, vecQuery[i].c_str());
		if (index > 0) {
			segIdColl.insert(index);
			QueryColum query;
			query.szSource = vecQuery[i];
			vecResult.push_back(query);
		}
	}

	if (vecResult.size() == 0){
		CXmlUtil::CreateResponXML(CardReadErr, err(CardReadErr), xml);
		return CardInputParamError;
	}

	if (segIdColl.size() < vecQuery.size()) {
		bReadQuery = true;
	}

	if (!bReadQuery) {
		for (size_t i=0; i<vecResult.size(); ++i){
			QueryColum & stQuery = vecResult[i];

			ZeroMemory(readbuf, sizeof(readbuf));
			QueryItem(g_CardOps->cardAdapter->type, 
						stQuery.szSource.c_str(), 
						readbuf, nLen, stQuery);
		}

	} else {
		std::set<int>::iterator iter;
		int readFlag = 0;
		for (iter=segIdColl.begin(); iter!=segIdColl.end(); iter++) {
			readFlag += 1 << (*iter - 1);
		}
		status = _iReadQuery(readFlag, vecResult);
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		CpuConvertRetPos(vecResult);
	}
	iCreateXmlByVector(vecResult, xml, &nLen);
	return status == CardProcSuccess ? CardProcSuccess : CardReadErr;
}


int __stdcall iPatchCard(
						char *pszCardDataXml,
						char *pszCardCoverDataXml,
						char *pszPrinterType,
						char *pszCardCoverXml 
						)
{
	ASSERT_OPEN(g_bCardOpen);
	
	if (m_pBHPrinter == NULL){
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0){
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	m_pBHPrinter->FeedCard();

	Sleep(1000);
	int nTimeOut = 1000;
	int type = 0;
	while (iScanCard() != 0)
	{
		if (nTimeOut >= TIMEOUT){;
			m_pBHPrinter->DeInitPrinter();
			SAFE_DELETE(m_pBHPrinter);
			return FeedCardError;
		}
		Sleep(1000);
		nTimeOut += 1000;
	}

	int nret = iCreateCard(pszCardDataXml);
	if (nret != 0){
		m_pBHPrinter->DeInitPrinter();
		SAFE_DELETE(m_pBHPrinter);
		return CardCreateErr;
	}
   
	m_pBHPrinter->BackToPrintHeader();
	nret = iPrintCard(pszPrinterType, pszCardCoverDataXml, pszCardCoverXml);
	if (nret != 0){
		SAFE_DELETE(m_pBHPrinter);
		return CardCoverPrintErr;
	}
	SAFE_DELETE(m_pBHPrinter);

	return 0;
}

//获取打印机列表名字
int __stdcall iGetPrinterList(char *PrinterXML)
{
	if (PrinterXML == NULL || strlen(PrinterXML) == 0){
		return -1;
	}
	std::vector<std::string> vecPrinter;
	CBHGX_Printer::GetPrinterList(vecPrinter);
	int nSize = (int)vecPrinter.size();
	TiXmlDocument *XmlDoc = new TiXmlDocument;
	TiXmlElement *RootElement = NULL;
	TiXmlDeclaration HeadDec;
	TiXmlElement *PrintSeg = NULL;
	TiXmlPrinter Printer;

	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\"?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("PrinterList");
	RootElement->SetAttribute("NUM", (int)vecPrinter.size());
	for (size_t i=0; i<vecPrinter.size(); ++i)
	{
		PrintSeg = new TiXmlElement("PrinterName");
		PrintSeg->SetAttribute("NAME", vecPrinter[i].c_str());
		RootElement->LinkEndChild(PrintSeg);
	}

	XmlDoc->LinkEndChild(RootElement);
	XmlDoc->Accept(&Printer);
	strcpy(PrinterXML, Printer.CStr());
	return nSize;
}

int __stdcall iPrintCard(
						char *pszPrinterType,
						char *pszCardCoverDataXml,
						char *pszCardCoverXml 
						)
{
	int nPrint = 0;
	if (m_pBHPrinter == NULL){
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0) {
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	if (m_pBHPrinter->InitPrinter(pszCardCoverDataXml, pszCardCoverXml)!= 0){
		SAFE_DELETE(m_pBHPrinter);
		return CardXmlErr;
	}

	if (!m_pBHPrinter->StartPrint()){
		m_pBHPrinter->DeInitPrinter();
		nPrint = -1;
	}
	
	SAFE_DELETE(m_pBHPrinter);
	return nPrint;
}


int __stdcall iCreateCard(char *pszCardDataXml)
{
	ASSERT_OPEN(g_bCardOpen);
	int result = 0;
	ISSCANCARD;

	std::string xmlStr(pszCardDataXml);
	if (CXmlUtil::CheckCardXMLValid(xmlStr) < 0){
		LOG_DEBUG("CardXML:Check Error");
		return CardXmlErr;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		LOG_DEBUG("CPU暂不支持");
		return CardNoSupport;
	}

	//M1制卡
	XmlSegmentS *seg = g_CardOps->iConvertXmltoList((char*)xmlStr.c_str());
	seg = g_SegHelper->FindSegmentByID(seg, 2);
	int nRet = 0;
	if (seg != NULL){
		unsigned char KeyB[6];
		XmlColumnS *stColumn = g_SegHelper->FindColumnByID(seg->ColumnHeader, 1);
		if (seg->ColumnHeader->Value[0] == '0'){
			stColumn = g_SegHelper->FindColumnByID(seg->ColumnHeader, 7);
		}
		iGetKeyBySeed((unsigned char *)stColumn->Value, KeyB);

		nRet = iWriteInfo((char*)xmlStr.c_str());
		LOG_DEBUG("回写数据：%d", nRet);

		nRet = InitPwd(KeyB);
		LOG_DEBUG( "重置密码结果%d", nRet);

	} else {
		return CardCreateErr;
	}
	return nRet != 0? CardCreateErr:0;
}

int __stdcall iFormatCard()
{
	int status = 0;
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		status = FormatCpuCard(0xff);
	} else {
		status = aFormatCard(0xff);
	}
	return status == 0 ? CardProcSuccess : CardFormatErr;
}


int __stdcall iCardCtlCard(int cmd, void *data)
{
	ASSERT_OPEN(g_bCardOpen);
	return apt_IOCtl(cmd, data);
}

int __stdcall iCheckMsgForNH(char *pszCardCheckWSDL,char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)
	//SCANCARD_XML(pszXml)

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	std::string strCardNO;
	if (iQueryInfo("CARDNO", szQuery) != 0){
		CXmlUtil::CreateResponXML(CardReadErr, "获取卡号失败", pszXml);
		return CardReadErr;
	}
	CXmlUtil::GetQueryInfoForOne(szQuery, strCardNO);
	
	int status = CardProcSuccess;
	WebServiceUtil checkUtil(pszCardCheckWSDL, pszCardServerURL);
	status = checkUtil.NHCheckValid(strCardNO, pszXml);
	
	isCardAuth();
	if (status == CardProcSuccess){
		int flag = 2;
		if (g_CardOps->cardAdapter->type == eCPUCard) {
			if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY | CPU_ONLY) == 1) {
				flag += 1 + (1 << 7);
			} else if (CPU_16K == 1) {
				flag += 1 + (1 << 7) + (1 << 10);
			}
		}
		status = iReadInfo(flag, pszXml);
	}
	return status;
}

int __stdcall iReadConfigMsg(char *pszConfigXML,char *pszReadXML)
{
	ASSERT_OPEN(g_bCardOpen);
	std::string strQuery("");
	std::vector<std::string> vecQuery;
	vecQuery = split(std::string(pszConfigXML), "|");
	for (size_t i=0; i<vecQuery.size(); ++i)
	{
		int nCode = atoi(vecQuery[i].c_str());
		switch (nCode)
		{
		case 1:
			strQuery += "NAME|";
			break;
		case 2:
			strQuery += "IDNUMBER|";
			break;
		case 3:
			strQuery += "BIRTHDAY|";
			break;
		case 4:
			strQuery += "GENDER|";
			break;
		default:
			strQuery = "";
			break;
		}
	}
	strQuery = strQuery.substr(0, strQuery.size()-1);
	
	return iQueryInfo((char*)strQuery.c_str(), pszReadXML);
}



int __stdcall iRegMsgForNH(char *pszCardServerURL, char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)
	if (g_CardOps->cardAdapter->type != eM1Card) {
		return CardNoSupport;
	}
	SCANCARD_XML(pszXml)

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	std::string strCardNO;
	int status = iQueryInfo("CARDNO", szQuery);
	if (status != 0){
		CXmlUtil::CreateResponXML(CardReadErr, "获取卡号失败", pszXml);
		return CardReadErr;
	}
	CXmlUtil::GetQueryInfoForOne(szQuery, strCardNO);
	
	WebServiceUtil checkUtil("", pszCardServerURL);
	status = checkUtil.NHRegCard(strCardNO, pszXml);
	if (status == CardProcSuccess) {
		status = iWriteInfo(pszXml);
		if (status != CardProcSuccess) {
			CXmlUtil::CreateResponXML(CardWriteErr, "卡回写失败", pszXml);
		}
	}

	if (status == CardProcSuccess){
		int flag = 2;
		if (g_CardOps->cardAdapter->type == eCPUCard) {
			if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY | CPU_ONLY) == 1) {
				flag += 1 + (1 << 7);
			} else if (CPU_16K == 1) {
				flag += 1 + (1 << 7) + (1 << 10);
			}
		}
		status = iReadInfo(flag, pszXml);
	}
	return status;
}



int __stdcall iRegMsgForNHLog(char *pszCardServerURL, char* pszLogXml, char* pszXml)
{
	int status = iRegMsgForNH(pszCardServerURL, pszXml);
	if (status != CardProcSuccess) {
		CXmlUtil::CreateResponXML(status, err(status), pszXml);
		return status;
	}
	CLogHelper LogHelper(pszLogXml);
	LogHelper.setLogParams(0, "iRegMsgForNHLog");
	LogHelper.setCardInfo(pszXml);
	LogHelper.geneHISLog();
	return CardProcSuccess;
}


int __stdcall iReadCardMessageForNHLocal(char* pszLogXml, char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)

	int status = iCheckException(pszLogXml, pszXml);
	if (status != CardProcSuccess) {
		return status;
	}

	std::string strMedicalID;
	status = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardReadErr, "获取参合号失败", pszXml);
		return CardReadErr;
	}

	int flag = 2;
	if (g_CardOps->cardAdapter->type == eCPUCard) {
		if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY | CPU_ONLY) == 1) {
			flag += 1 + (1 << 7);
		} else if (CPU_16K == 1) {
			flag += 1 + (1 << 7) + (1 << 10);
		}
	}

	if (CardProcSuccess != iReadInfo(flag, pszXml)) {
		return CardReadErr;
	}
	CLogHelper LogHelper(pszLogXml);
	LogHelper.setLogParams(0, "iReadCardMessageForNHLocal");
	LogHelper.setCardInfo(pszXml);
	LogHelper.geneHISLog();
	return CardProcSuccess;
}

int __stdcall iReadCardMessageForBothNHLocal(
	char *pszCardCheckWSDL, 
	char *pszCardServerURL, 
	char* pszLogXml,
	char* pszXml
	)
{
	if (strlen(pszCardCheckWSDL)==0 || strlen(pszCardServerURL) == 0) {
		return iReadCardMessageForNHLocal(pszLogXml, pszXml);
	} else {
		return iReadCardMessageForNH(pszCardCheckWSDL, pszCardServerURL, pszXml);
	}
}


//卡校验 黑名单校验
int __stdcall iCheckMsgForNHLocal(char* pszLogXml, char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)
	//SCANCARD_XML(pszXml)

	int status = iCheckException(pszLogXml, pszXml);
	if (status != CardProcSuccess) {
		return status;
	}
	
	CXmlUtil::CreateResponXML(CardProcSuccess, "卡验证成功，正常卡", pszXml);
	return CardProcSuccess;
}

int __stdcall iReadCardMessageForNHLog(char *pszCardCheckWSDL, 
									char *pszCardServerURL, 
									char* pszLogXml,
									char* pszXml)
{
	int status = iReadCardMessageForNH(pszCardCheckWSDL, pszCardServerURL, pszXml);
	if (status != CardProcSuccess) {
		return CardCheckError;
	}
	CLogHelper LogHelper(pszLogXml);
	LogHelper.setLogParams(0, "iReadCardMessageForNHLog");
	LogHelper.setCardInfo(pszXml);
	LogHelper.setSamID(g_SamID);
	LogHelper.geneHISLog();
	return CardProcSuccess;
}

int __stdcall iReadOnlyCardMessageForNHLog(char *pszLogXml, char* pszXml)
{
	int status = iReadOnlyCardMessageForNH(pszXml);
	if (status != CardProcSuccess) {
		return CardReadErr;
	} 
	CLogHelper LogHelper(pszLogXml);
	LogHelper.setLogParams(0, "iReadOnlyCardMessageForNHLog");
	LogHelper.setCardInfo(pszXml);
	LogHelper.setSamID(g_SamID);
	LogHelper.geneHISLog();
	return CardProcSuccess;
}

int __stdcall iReadOnlyCardMessageForNH(char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)
	//SCANCARD_XML(pszXml)

	int flag = 2;
	if (g_CardOps->cardAdapter->type == eCPUCard) {
		if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY | CPU_ONLY) == 1) {
			flag += 1 + (1 << 7);
		} else if (CPU_16K == 1) {
			flag += 1 + (1 << 7) + (1 << 10);
		}
	}
	int status = iReadInfo(flag, pszXml);
	if (status != CardProcSuccess) {
		return CardReadErr;
	}
	isCardAuth();
	return CardProcSuccess;
}

int __stdcall iReadCardMessageForNH(char *pszCardCheckWSDL, char *pszCardServerURL, char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen)
	//SCANCARD_XML(pszXml)

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	WebServiceUtil checkUtil(pszCardCheckWSDL, pszCardServerURL);

	std::string strCardNO;
	int status = ParseValueQuery("CARDNO", strCardNO);
	if (status != 0 || strCardNO.size() == 0){
		CXmlUtil::CreateResponXML(CardReadErr, "获取卡号失败或者卡号为空", pszXml);
		return CardReadErr;
	}

	status = checkUtil.NHCheckValid(strCardNO, pszXml);
	if (status == CardProcSuccess){
		if (g_CardOps->cardAdapter->type == eM1Card) {
			status = checkUtil.NHRegCard(strCardNO, pszXml);
			if (status == CardProcSuccess) {
				status = iWriteInfo(pszXml);
				if (status != CardProcSuccess) {
					CXmlUtil::CreateResponXML(CardWriteErr, "卡回写失败", pszXml);
					goto done;
				}
			} else {
				goto done;
			}
		}
	} else {
		goto done;
	}

done:
	if (status != CardProcSuccess) {
		return status;
	}

	std::string strMedicalID;
	status = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (status != 0) {
		CXmlUtil::CreateResponXML(CardReadErr, "获取参合号失败", pszXml);
		return CardReadErr;
	}
	if (strMedicalID.size() == 0) {
		CXmlUtil::CreateResponXML(CardMedicalFailed, "参合号为空", pszXml);
		return CardMedicalFailed;
	}

	if (!checkUtil.IsMedicalID(strMedicalID)) {
		CXmlUtil::CreateResponXML(CardNotMedicalCard, "非农合卡", pszXml);
		return CardNotMedicalCard;
	}

	if (status == CardProcSuccess){
		int flag = 2;
		if (g_CardOps->cardAdapter->type == eCPUCard) {
			if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY | CPU_ONLY) == 1) {
				flag += 1 + (1 << 7);
			} else if (CPU_16K == 1) {
				flag += 1 + (1 << 7) + (1 << 10);
			}
		}
		status = iReadInfo(flag, pszXml);
	}
	return status;
}


int __stdcall iEncryFile(char *filename)
{
	CDESEncry encry;
	bool bSuccess = encry.EncryFile(filename);
	return bSuccess ? 0 : EncryFileError;
}


int __stdcall iCreateLicense(char *filename, char *timeStr)
{
	if (timeStr == NULL || strlen(timeStr) == 0) {
		return InitFullLicense(filename);
	}else {
		return InitTimeLicense(filename, timeStr);
	}
	return 0;
}

int __stdcall iCheckLicense(char *filename,int type)
{
	int status = 0;
	if (type == 0) {
		status = CheckTimeLicense(filename);
		status = (status == 0 ? 0 : CardAuthExpired);
	} else {
		status = CheckFullLicense(filename);
		status = (status == 0 ? 0 : CardNoAuthority);
	}
	return status;
}

int __stdcall iCheckException(char *pszLogXml,char *pszXml)
{
	CDBExceptionCheck check(pszLogXml);
	if (check.isNormal() == FALSE) {
		return CardXmlErr;
	}

	int status = check.initDBHelper();
	if (status != CardProcSuccess) {
		return status;
	}

	status = check.filterForbidden(pszXml);
	if (status != CardProcSuccess) {
		return status;
	} 
	status = check.filterWarnning(pszXml);
	if (status != CardProcSuccess) {
		return status;
	}
	return CardProcSuccess;
}

int __stdcall apt_InitGList(CardType eType)
{
	if (eType == eCPUCard) {
		g_CpuCardOps = InitCpuCardOps();
		g_CardOps = g_CpuCardOps;

#if (CPU_M1 || CPU_8K)
		g_sourceValueMap.insert(std::make_pair("STAGENO", QueryColum(2, 4, "STAGENO", "000000")));
		g_segMap["CARDNO"] = 201;
		g_segMap["MEDICARECERTIFICATENO"] = 207;
#endif

	} else {
		g_M1CardOps = InitM1CardOps();
		g_CardOps = g_M1CardOps;		
	}
	g_XmlListHead = g_CardOps->programXmlList;
	g_SegHelper = (CSegmentHelper*)g_CardOps->SegmentHelper;
	return 0;
}


bool __stdcall isCardAuth(int timeout)
{
	if ((CPU_8K_TEST | CPU_8K_TEST_ALL) == 0) {
		return true;
	}
	bool bAuthed = false;
	char readInfo[1024];
	memset(readInfo, 0, sizeof(readInfo));
	int status = iReadInfo(1, readInfo);
	if (status == CardProcSuccess) {
		CMarkup xml;
		xml.SetDoc(readInfo);
		if (!xml.FindElem("SEGMENTS")){
			goto done;
		}
		xml.IntoElem();
		if (xml.FindElem("SEGMENT")){
			xml.IntoElem();
			while (xml.FindElem("COLUMN")){
				int col_id = atoi(xml.GetAttrib("ID").c_str());
				if (col_id == 5) {
					bAuthed = (strcmp(xml.GetAttrib("VALUE").c_str(), "8D") == 0);
					goto done;
				}
			}
			xml.OutOfElem();
		}
		xml.OutOfElem();
	}

done:
	if (!bAuthed) {
		Sleep(timeout * 1000);
	}
	return bAuthed;
}

int __stdcall iReadAll(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;
	int flag = 0;

	isCardAuth(5);
	if (CPU_8K_TEST == 1) {
		flag = 1 + 2 + (1 << 3) + (1 << 4) + (1 << 7);
	} else {
		flag = 1 + 2 + (1 << 3) + (1 << 4) + (1 << 5) + (1 << 6) + (1 << 7) + (1 << 8) + (1 << 9) + (1 << 14) + (1 << 17);
	}
	return iReadInfo(flag, xml);
}


int __stdcall iRWRecycle(
				char *pszCardCorp,
				char *pszXinCorp,
				int   counts,
				char *xml,
				char *pszRetInfo
			)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;
	
	std::locale::global(std::locale(""));
	char read_buff[1024];
	char filename[256];
	char timeStr[64];
	bool bAuthed = isCardAuth(40);

	memset(filename, 0, sizeof(filename));
	CTimeUtil::getCurrentDay(timeStr);

	sprintf_s(filename, sizeof(filename), "c:\\config\\KS%s-XP%s-%s.log", pszCardCorp, pszXinCorp, timeStr);
	ofstream out(filename, ios_base::out|ios_base::app);
	
	CTimeUtil::getCurrentTime(timeStr);
	out << "循环读写执行开始，开始时间：" << timeStr << endl;
	out << "卡商名称:" << pszCardCorp << endl;
	out << "芯片厂商名称：" << pszXinCorp << endl;
	out << "计划循环读写次数：" << counts << endl;
	out << xml << endl;

	int chose_one = -1;
	if (!bAuthed) {
		srand(unsigned(time(0)));
		chose_one = rand() % counts;
	}
	
	int rflag, wflag;
	int rSuccess=0, wSuccess=0;
	for (int i=0; i< counts; i++) {
		wflag = iWriteInfo(xml);
		wSuccess += (wflag == CardProcSuccess) ? 1 : 0;

		rflag = iReadInfo(1 << 7, read_buff);
		rSuccess += (rflag == CardProcSuccess) ? 1 : 0;
		out << "第"<< i+1 << "次，读取"<< (rflag==CardProcSuccess ? "成功" : "失败");
		out << "，写入"<< (wflag==CardProcSuccess ? "成功" : "失败") <<endl;
		if (chose_one == i) {
			memset(xml, 0, strlen(xml));
		}
	}
	CTimeUtil::getCurrentTime(timeStr);
	out << "循环读写执行结束，结束时间：" << timeStr << endl;
	out.close();
	memset(timeStr, 0, sizeof(timeStr));
	sprintf_s(timeStr, sizeof(timeStr), "成功读取%d次，成功写入%d次", rSuccess, wSuccess);
	strcpy(pszRetInfo, timeStr);
	return 0;
}

int __stdcall iReadOnlybloodbank (char *xml)
{
	int flag = 1 << 10;
	return iReadInfo(flag, xml);
}

int __stdcall iWritebloodbank(char *xml)
{
	return iWriteInfo(xml);
}

int __stdcall iReadCardSEQ(char *xml)
{
	ASSERT_OPEN(g_bCardOpen)
	//SCANCARD_XML(xml)

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	std::string query;
	if (g_CardOps->cardAdapter->type == eM1Card) {
		query = "CARDNO";
	} else {
		query = "CARDSEQ";
	}
	
	if (iQueryInfo((char*)query.c_str(), szQuery) != 0){
		CXmlUtil::CreateResponXML(CardReadErr, "获取卡序列号失败", xml);
		return CardReadErr;
	}
	std::string strCardSEQ;
	CXmlUtil::GetQueryInfoForOne(szQuery, strCardSEQ);
	strcpy(xml, strCardSEQ.c_str());
	return CardProcSuccess;
}

int __stdcall iGeneLog(
					   char* pszLogXml,
					   int rwFlag,
					   char *funcName,
					   char *pszXml,
					   char *hospInfo
					   )
{
	CLogHelper LogHelper(pszLogXml);
	LogHelper.setLogParams(rwFlag, funcName);
	LogHelper.setCardInfo(pszXml);
	LogHelper.setSamID(g_SamID);
	LogHelper.setHospInfo(hospInfo);
	LogHelper.geneHISLog();
	return 0;
}