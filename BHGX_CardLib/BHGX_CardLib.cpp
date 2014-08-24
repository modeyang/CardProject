#include <vector>
#include <stdio.h>
#include <time.h>
#include <string>
#include <map>

#include "BHGX_CardLib.h"
#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"
#include "public/debug.h"
#include "public/liberr.h"

#include "Card.h"
#include "CPUCard.h"
#include "M1Card.h"
#include "adapter.h"
#include "public/algorithm.h"

#include "resource.h"
#include "BHGX_Printer.h"
#include "public/ConvertUtil.h"
#include "WebServiceAssist.h"
#include "tinyxml/headers/tinyxml.h"
#include "public/Markup.h"
#include "Encry/DESEncry.h"
#include "public/Authority.h"
#include "StringUtil.h"

#include "public/TimeUtil.h"
#include "public/XmlUtil.h"
#include "public/ExceptionCheck.h"

using namespace std;
#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#define DBGCore(format, ...) LogWithTime(0, format)

#define SAFE_DELETE(a)  if (a != NULL) { delete(a);a = NULL;}
#define SAFE_DELETE_C(a)  if (a != NULL) { free(a);a = NULL;}

#define NR_MASK(nr) (1 << nr)
#define NOT_NR_MASK(nr) ~(1 << nr)

#define SETBIT(byte, nr) byte |= NR_MASK(nr)
#define CLRBIT(byte, nr) byte &= NOT_NR_MASK(nr)

#define SAFEARRAY_DELETE(a)  if (a != NULL) { delete [] a ;a = NULL;}

#define ISSCANCARD { if (iScanCard() != 0) return CardScanErr;}
											
#define ISGWCARD(a) ((a[0]) == '1')

#define SCANCARD_XML(xml, ret)					\
	if (iScanCard() != 0) {						\
		CreateResponXML(3, "寻卡失败", ret);	\
		strcpy(xml, ret);						\
		return CardScanErr;						\
	}											\



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


struct CreateKeyInfoS
{
	int offset;
	int length;
	unsigned char ID[9];
	unsigned char token[6];
	unsigned char key[16];
};

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

//source 与 value的默认对应值
std::map<string, QueryColum> g_sourceValueMap;
std::map<string, int> g_segMap;
std::map<string, int> g_columnMap;


CBHGX_Printer *m_pBHPrinter = NULL;
BOOL g_bPreLoad = FALSE;
BOOL g_bCardOpen = FALSE;

static CardOps *g_CardOps = NULL;

/**
 * 全局的数据结构
 */
struct XmlProgramS *g_XmlListHead = NULL;
struct XmlProgramS *g_CPUXmlListHead = NULL;
struct XmlProgramS *g_M1XmlListHead = NULL;

//M1卡时会将一部分在第2块的数据保存在第五块区域，当只访问第二扇区时
bool g_OnlySecond = false;

typedef  std::map<std::string, struct XmlColumnS *> XmlColumnMapT;
XmlColumnMapT XmlColumnMap; 

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
CpuCallocForColmn(struct XmlColumnS *result);

static void 
M1CallocForColmn(struct XmlColumnS *result);

static struct XmlColumnS  *
CloneColmn(struct XmlColumnS *ColmnElement, int mode);

static struct XmlSegmentS *
CloneSegment(struct XmlSegmentS *SegmentElement, int mode);

int 
FindColumIDByColumName(struct XmlSegmentS *list, const char *name);

int 
FindSegIDByColumName(struct XmlSegmentS *list ,const char *name);

struct XmlSegmentS *
FindSegmentByID(struct XmlSegmentS *listHead, int ID);

struct XmlSegmentS *
FindSegmentByColumName(struct XmlSegmentS *list ,const char *name);

struct XmlSegmentS * 
getSegmentByColumName(struct XmlSegmentS *list ,const char *name);

struct XmlColumnS* 
FindColumByColumName(struct XmlSegmentS *list, const char *name);

struct XmlColumnS* 
getColumByColumName(struct XmlSegmentS *list, const char *name);

struct XmlColumnS *
FindColumnByID(struct XmlColumnS *listHead, int ID);

//M1
static int 
CombineColValue(struct XmlColumnS *ColumnElement, 
				std::pair<int,int> pairCol, 
				char sep, struct XmlSegmentS *Segment,
				char *buff);

//M1
static int 
InsertColumnBySplite(struct XmlSegmentS *SegmentElement, 
					 std::map<int,std::string> &mapSplite, 
					 struct XmlSegmentS *pSeg, 
					 std::pair<int,int> pairCol);


static int 
M1ConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length);

static struct XmlSegmentS* 
M1ConvertXmltoList(char *xml);

static int 
CpuConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length);

static struct XmlSegmentS* 
CpuConvertXmltoList(char *xml);

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
iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length);

static struct CreateKeyInfoS* 
CreateCardKeyInfo(unsigned char *seed);

static int 
CheckCardXMLValid(std::string &pszCardXml);

//**********************************
//以下函数会注册到CardOps结构体中
extern "C" adapter CpuAdapter;

//CPU卡驱动函数列表
//加载xml，初始化链表
static int InitCpuGlobalList();

static int InitionCpuGList(char *xml);

CardOps CpuCardOps;

//M1卡驱动函数列表
extern "C" adapter M1Adapter;

//加载xml，初始化链表
static int InitM1GlobalList();

static int InitionM1GList(char *xml);

CardOps M1CardOps;

static int InitCardOps() 
{
	InitM1CardOps();
	InitCpuCardOps();
	CpuCardOps.iInitGList = InitCpuGlobalList,
	CpuCardOps.iConvertXmlByList = CpuConvertXmlByList,
	CpuCardOps.iConvertXmltoList = CpuConvertXmltoList,
	CpuCardOps.iCallocForColmn =  CpuCallocForColmn,
	CpuCardOps.cardAdapter = &CpuAdapter,

	M1CardOps.iInitGList = InitM1GlobalList;
	M1CardOps.iConvertXmlByList = M1ConvertXmlByList;
	M1CardOps.iConvertXmltoList = M1ConvertXmltoList;
	M1CardOps.iCallocForColmn =  M1CallocForColmn;
	M1CardOps.cardAdapter = &M1Adapter;

	return 0;
}
//*************************************

static BOOL isWriteable(int cpuIndex) 
{
	return cpuIndex < CPU_WRITE_DOWN;
}


/**
 * CloneColmn - 克隆XmlColmn元素的数据结构
 * @ ClomnElement 被克隆的元素
 */

static void CpuCallocForColmn(struct XmlColumnS *result) 
{
	int length = result->CheckInfo.CpuInfo.ColumnByte; 
	int padding = 0;
	struct XmlSegmentS *pSeg = result->parent;
	
	if (pSeg->datatype == eRecType) {
		// rec file dataformat: TLV type:length:value
		padding = 2;
	}

	if (result->CheckInfo.CpuInfo.itemtype != eAnsType) {
		length *= 2; 
	}
	length += padding + 1;
	result->Value = (char*)malloc(length);
	memset(result->Value, 0, length);
}

static void M1CallocForColmn(struct XmlColumnS *result) 
{
	int len = result->CheckInfo.M1Info.ColumnBit;
	result->Value = (char*)malloc(len + 1);
	memset(result->Value, 0, len + 1);
}

// mode 为1时表示分配内存
static struct XmlColumnS *CloneColmn(struct XmlColumnS *ColmnElement, int mode)
{
	struct XmlColumnS *result = NULL;

	if(ColmnElement == NULL)
		return NULL;

	result = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
	if(result == NULL) {
		DBGCore( "Memeory Shrink, Malloc Memory Failure\n");
		return NULL;
	}
	
	memcpy(result, ColmnElement, sizeof(struct XmlColumnS));
	result->parent = ColmnElement->parent;
	result->Value = NULL;
	result->Next = NULL;

	if (mode) {
		g_CardOps->iCallocForColmn(result);
	}
	

	return result;
}

/**
 * CloneSegment - 克隆XmlSegmentS元素数据结构
 * @SegmentElement 被克隆的元素
 * @mode 1->开辟value内存
 */
static struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement, int mode)
{
	struct XmlColumnS *OrigColumnElement = NULL;	// 元SegmentElegment对应元素
	struct XmlColumnS *TempColumnElement = NULL;	// Result对应的临时元素
	struct XmlSegmentS *result = NULL;

	if(SegmentElement != NULL){
		result = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		if (result == NULL) {
			return NULL;
		}

		memcpy(result, SegmentElement, sizeof(struct XmlSegmentS));
		result->ColumnHeader = NULL;
		result->ColumnTailer = NULL;
		result->Next = NULL;

		// 复制下边的链表结构
		for(OrigColumnElement = SegmentElement->ColumnHeader; OrigColumnElement; 
			OrigColumnElement = OrigColumnElement->Next){
			TempColumnElement = CloneColmn(OrigColumnElement, mode);

			// 将新生成的元素加入到链表中
			if(result->ColumnHeader){
				result->ColumnTailer->Next = TempColumnElement;
				result->ColumnTailer = TempColumnElement;

			} else {
				result->ColumnTailer = TempColumnElement;
				result->ColumnHeader = TempColumnElement;
			}
		}
	}
	return result;
}


struct XmlSegmentS *FindSegmentByID(struct XmlSegmentS *listHead, int ID)
{
	struct XmlSegmentS *result = NULL;

	result = listHead;
	while(result){
		if(result->ID == ID)
			break;

		result = result->Next;
	}

	return result;
}


int FindSegIDByColumName(struct XmlSegmentS *list ,const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;

	while (result) {
		resultCol = result->ColumnHeader;
		while (resultCol) {
			if (strcmp(resultCol->Source, name) == 0) {
				return result->ID;
			}
			resultCol = resultCol->Next;
		}
		result = result->Next;
	}
	return -1;
}

struct XmlSegmentS * FindSegmentByColumName(struct XmlSegmentS *list ,const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;

	while (result) {
		resultCol = result->ColumnHeader;
		while (resultCol) {
			if (strcmp(resultCol->Source, name) == 0) {
				return result;
			}
			resultCol = resultCol->Next;
		}
		result = result->Next;
	}
	return NULL;
}

struct XmlSegmentS * getSegmentByColumName(struct XmlSegmentS *list ,const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;

	while (result) {
		resultCol = result->ColumnHeader;
		while (resultCol) {
			if (strcmp(resultCol->Source, name) == 0) {
				return CloneSegment(result, 1);
			}
			resultCol = resultCol->Next;
		}
		result = result->Next;
	}
	return NULL;
}

int FindColumIDByColumName(struct XmlSegmentS *list, const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;

	while (result) {
		resultCol = result->ColumnHeader;
		while (resultCol) {
			if (strcmp(resultCol->Source, name) == 0) {
				return resultCol->ID;
			}
			resultCol = resultCol->Next;
		}
		result = result->Next;
	}
	return -1;
}

struct XmlColumnS* FindColumByColumName(struct XmlSegmentS *list, const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;
	struct XmlColumnS *tmp = NULL;
	while (result) {
		tmp = result->ColumnHeader;
		while (tmp) {
			if (strcmp(tmp->Source, name) == 0) {
				return tmp;
			}
			tmp = tmp->Next;
		}
		result = result->Next;
	}
	return NULL;
}

struct XmlColumnS* getColumByColumName(struct XmlSegmentS *list, const char *name) 
{
	struct XmlSegmentS *result = list;
	struct XmlColumnS *resultCol = NULL;
	struct XmlColumnS *tmp = NULL;
	while (result) {
		tmp = result->ColumnHeader;
		while (tmp) {
			if (strcmp(tmp->Source, name) == 0) {
				resultCol = CloneColmn(tmp, 1);
				goto done;
			}
			tmp = tmp->Next;
		}
		result = result->Next;
	}

done:
	return resultCol;
}



/**
*
*/
struct XmlColumnS *FindColumnByID(struct XmlColumnS *listHead, int ID)
{
	struct XmlColumnS *result = NULL;

	result = listHead;
	while(result){
		if(result->ID == ID)
			break;

		result = result->Next;
	}

	return result;
}


/**
 *
 */
static struct XmlSegmentS *GetXmlSegmentByFlag(int flag)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;
	struct XmlSegmentS *XmlListHead = g_XmlListHead->SegHeader;

	for(SegmentElement=XmlListHead; SegmentElement; SegmentElement = SegmentElement->Next){
		// 表明这个位置被设置
		int nReadFlag = flag & 0x1;

		if(nReadFlag > 0) {
			TempSegmentElement = CloneSegment(SegmentElement, 1);

			// 将新生成的链表加入
			if(result){
				CurrSegmentElement->Next = TempSegmentElement;
				CurrSegmentElement = TempSegmentElement;
			} else {
				CurrSegmentElement = TempSegmentElement;
				result = CurrSegmentElement;
			}
		}
		flag = flag >> 1;
	}

	return result;
}

/**
 *
 */

//将两个字段的内容合并成一个字段，从第5块合并数据到
static int CombineColValue(struct XmlColumnS *ColumnElement, 
						   std::pair<int,int> pairCol, 
						   char sep, struct XmlSegmentS *Segment,
						   char *buff)
{
	if (sep == 0 && strlen(ColumnElement->Value)/5 <= ColumnElement->CheckInfo.M1Info.ColumnBit/64) {
		strcpy(buff, ColumnElement->Value);
		return 0;
	}
	if (ColumnElement->ID == pairCol.first)
	{
		struct XmlSegmentS *pSegTel = Segment;
		struct XmlSegmentS *pSegPri = NULL;
		while (pSegTel->Next != NULL){
			pSegPri = pSegTel;
			pSegTel = pSegTel->Next;
		}

		if (pSegTel->ID == 5){
			struct XmlColumnS *pTel = pSegTel->ColumnHeader;
			struct XmlColumnS *pPri = NULL;
			while (pTel->Next != NULL && pTel->ID != pairCol.second){
				pPri = pTel;
				pTel = pTel->Next;
			}

			if (pTel->ID == pairCol.second){
				if (strlen(pTel->Value) > 0){
					if (sep != 0)
						sprintf_s(buff, 100, "%s%c%s", ColumnElement->Value, sep,pTel->Value);
					else
						sprintf_s(buff, 100, "%s%s", ColumnElement->Value,pTel->Value);
				} else {
					strcpy(buff, ColumnElement->Value);
				}

				pPri->Next = pTel->Next;
				free(pTel);
			}
		}
	}
	return 0;
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
static int M1ConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlColumnS *ColumnElement = NULL;

	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;

	char buf[10];
	char *pBuf = NULL;
	char ColmnBuf[100];
	ZeroMemory(ColmnBuf, sizeof(ColmnBuf));

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	for(SegmentElement = listHead; SegmentElement; 
		SegmentElement = SegmentElement->Next){
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", SegmentElement->ID);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);

		int nNameSeg = SegmentElement->ID;

		for(ColumnElement = SegmentElement->ColumnHeader; ColumnElement; 
			ColumnElement = ColumnElement->Next){
			memset(buf, 0, 10);
			sprintf_s(buf, 10, "%d", ColumnElement->ID);

			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", buf);
			Cloumn->SetAttribute("SOURCE", ColumnElement->Source);

			if (nNameSeg == 2){
				std::string szName;
				struct XmlSegmentS *pSegPri, *pCur = NULL;

				switch (ColumnElement->ID){
				case 9:
					CombineColValue(ColumnElement, std::make_pair(9,78),0, SegmentElement, ColmnBuf);
					if (Is_IntName(ColmnBuf)){
						szName = CConvertUtil::uf_gbk_int_covers(ColmnBuf,"togbk");
						memcpy(ColmnBuf, szName.c_str(), szName.size());
						ColmnBuf[szName.size()] = 0;
					}
					break;
				case 10://add by yanggx5-28 为解决身份证末尾为字母的错误
					pBuf = ColumnElement->Value;
					while (*(pBuf) != 0)
						pBuf++;
					pBuf--;
					if (*pBuf > '9' || *pBuf < '0')
						*pBuf = 'X';

					strcpy(ColmnBuf, ColumnElement->Value);
					break;
				case 22:
					CombineColValue(ColumnElement, std::make_pair(22,75), '/', SegmentElement, ColmnBuf);
					break;
				case 24:
					CombineColValue(ColumnElement, std::make_pair(24,77),0, SegmentElement, ColmnBuf);
					if (Is_IntName(ColmnBuf)){
						szName = CConvertUtil::uf_gbk_int_covers(ColmnBuf,"togbk");
						memcpy(ColmnBuf, szName.c_str(), szName.size());
						ColmnBuf[szName.size()] = 0;
					}
					break;

				case 25:
					CombineColValue(ColumnElement, std::make_pair(25,76), '/', SegmentElement, ColmnBuf);
					pCur = SegmentElement;
					while (pCur->Next != NULL){
						pSegPri = pCur;
						pCur = pCur->Next;
					}

					if (g_OnlySecond ||
						IsAllTheSameFlag((unsigned char*)&pCur->ColumnHeader->Value,
						18, '0') == 0){
						pSegPri->Next = pCur->Next;
						SAFE_DELETE_C(pCur);
					}
					break; 
				default:
					strcpy(ColmnBuf, ColumnElement->Value);
					break;

				}
			} else {
				strcpy(ColmnBuf, ColumnElement->Value);
			}
			Cloumn->SetAttribute("VALUE", ColmnBuf);
			
			Segment->LinkEndChild(Cloumn);   
		}

		RootElement->LinkEndChild(Segment);
	}
	XmlDoc->LinkEndChild(RootElement);

	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return 0;
}

static int InsertColumnBySplite(struct XmlSegmentS *SegmentElement, 
								std::map<int,std::string> &mapSplite, 
								struct XmlSegmentS *pSeg, 
								std::pair<int,int> pairCol)
{
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlColumnS  *pColNext = pSeg->ColumnHeader;
	struct XmlColumnS *ColumnElement, *TempColumnElement = NULL;
	std::map<int, std::string>::iterator iter = mapSplite.find(pairCol.first);
	if (iter != mapSplite.end()){
		ColumnElement = FindColumnByID(SegmentElement->ColumnHeader, pairCol.second);
		if (NULL != ColumnElement){

			std::string value = iter->second;
			int len = value.length();
			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memcpy(TempColumnElement, ColumnElement, sizeof(struct XmlColumnS));
			TempColumnElement->Value = (char*)malloc(len + 1);
			TempColumnElement->Value[len] = 0;
			TempColumnElement->Next = NULL;
			strcpy_s(TempColumnElement->Value, len+1, value.c_str());
			if (pSeg->ColumnHeader == NULL){

				pSeg->ColumnHeader = TempColumnElement;
				pColNext = pSeg->ColumnHeader;
			} else{
				if (pColNext != NULL){
					while (pColNext->Next != NULL){
						pColNext = pColNext->Next;
					}
				}
				pColNext->Next = TempColumnElement;
			}
		}
	}
	return 0;
}
/**
 *
 */
static struct XmlSegmentS* M1ConvertXmltoList(char *xml)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;

	struct XmlColumnS  *ColumnElement = NULL;
	struct XmlColumnS  *CurrColumnElement = NULL;
	struct XmlColumnS  *TempColumnElement = NULL;
	struct XmlColumnS  *pAddtionElement = NULL;
	struct XmlColumnS  *pAddtionElement2 = NULL;
	struct XmlSegmentS *XmlListHead = g_XmlListHead->SegHeader;

	TiXmlDocument *XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;

	std::map<int, std::string> mapSplite;

	int ID;

	// 解析XML语句
	XmlDoc = new TiXmlDocument();
	XmlDoc->Parse(xml);
	RootElement = XmlDoc->RootElement();

	Segment = RootElement->FirstChildElement();
	while(Segment){
		ID = atoi(Segment->Attribute("ID"));
		SegmentElement = FindSegmentByID(XmlListHead, ID);
		if (NULL == SegmentElement)
			break;

		TempSegmentElement = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(TempSegmentElement, SegmentElement, sizeof(struct XmlSegmentS));
		TempSegmentElement->ColumnHeader = NULL;
		TempSegmentElement->ColumnTailer = NULL;
		TempSegmentElement->Next = NULL;

		// 加入新的元素
		if(result){
			CurrSegmentElement->Next = TempSegmentElement;
			CurrSegmentElement = TempSegmentElement;
		} else {
			CurrSegmentElement = TempSegmentElement;
			result = CurrSegmentElement;
		}
		
		Colum = Segment->FirstChildElement();
		while(Colum){
			int nColumnID = atoi(Colum->Attribute("ID"));
			ColumnElement = FindColumnByID(SegmentElement->ColumnHeader, nColumnID);

			if (NULL == ColumnElement)
				break;

			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memcpy(TempColumnElement, ColumnElement, sizeof(struct XmlColumnS));
			TempColumnElement->Value = (char*)malloc(TempColumnElement->CheckInfo.M1Info.ColumnBit +1); 
			TempColumnElement->Next = NULL;

			//add 1020
			char strCheckValue[100];
			memset(strCheckValue, 0, sizeof(strCheckValue));
			CheckSpace(Colum->Attribute("VALUE"), 
				strlen(Colum->Attribute("VALUE")), strCheckValue);

			int nBit = TempColumnElement->CheckInfo.M1Info.ColumnBit/4;
			if(strlen(Colum->Attribute("VALUE")) == 0 || 
			   std::string(Colum->Attribute("VALUE")) == string(" ")) {
				memset(strCheckValue, 'f', TempColumnElement->CheckInfo.M1Info.ColumnBit%4 ? nBit+1:nBit);
			} else {
				if (ID == 2) {
					if (nColumnID == 9 || nColumnID == 24) {
						if (Is_GbkName(strCheckValue)) {
							std::string szName = CConvertUtil::uf_gbk_int_covers(strCheckValue, "toint");
							strcpy(strCheckValue, szName.c_str());
						}

						if (nColumnID == 24 && strlen(strCheckValue) > 32){
							mapSplite[nColumnID] = std::string(strCheckValue).substr(32, strlen(strCheckValue));
							strCheckValue[32] = 0;
						} else if (nColumnID == 9 && strlen(strCheckValue) > 64) {
							mapSplite[nColumnID] = std::string(strCheckValue).substr(64, strlen(strCheckValue));
							strCheckValue[64] = 0;
						}
					}

					//addby yanggx 1124
					if (nColumnID == 22){
						std::vector<std::string> vecPhone;
						vecPhone = split(std::string(strCheckValue), "/");
						strcpy_s(strCheckValue, sizeof(strCheckValue), vecPhone[0].c_str());
						if (vecPhone.size() > 1){
							mapSplite[22] = vecPhone[1];
						} else {
							mapSplite[22] = "fffffffffff";
						}

					}

					//addby yanggx 1124
					if (nColumnID == 25){
						std::vector<std::string> vecPhone;
						vecPhone = split(std::string(strCheckValue), "/");
						strcpy_s(strCheckValue, sizeof(strCheckValue), vecPhone[0].c_str());
						if (vecPhone.size() > 1) {
							mapSplite[25] = vecPhone[1];
						} else {
							mapSplite[25] = "fffffffffff";
						}
					}
				}
			}

			// 赋值
			strcpy(TempColumnElement->Value, strCheckValue);

			// 加入链表  // 已经加入过了
			if(CurrSegmentElement->ColumnHeader) {
				CurrColumnElement->Next = TempColumnElement;
				CurrColumnElement = TempColumnElement;
			} else {
				CurrColumnElement = TempColumnElement;
				CurrSegmentElement->ColumnHeader = CurrColumnElement;
			}

			// 向后迭代
			Colum = Colum->NextSiblingElement();
		}

		// 向后迭代
		Segment = Segment->NextSiblingElement();
	}

	SegmentElement = FindSegmentByID(XmlListHead, 5);
	if (NULL != SegmentElement) {
		struct XmlSegmentS *pTel = result;
		while (pTel->Next != NULL){
			pTel = pTel->Next;
		}

		struct XmlSegmentS *pSeg = pTel;
		if (pTel->ID != 5){
			pSeg = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
			memcpy(pSeg, SegmentElement, sizeof(struct XmlSegmentS));
			pSeg->ID = 5;
			pSeg->ColumnHeader = NULL;
			pSeg->ColumnTailer = NULL;
			pSeg->Next = NULL;
			pTel->Next = pSeg;
		}

		//addby yanggx 0228
		InsertColumnBySplite(SegmentElement, mapSplite, pSeg, std::make_pair(22,75));
		InsertColumnBySplite(SegmentElement, mapSplite, pSeg, std::make_pair(25,76));
		InsertColumnBySplite(SegmentElement, mapSplite, pSeg, std::make_pair(24,77));
		InsertColumnBySplite(SegmentElement, mapSplite, pSeg, std::make_pair(9,78));
	}

	// 返回结果
	return result;
}

static int  CpuConvertXmlByList(struct XmlSegmentS *listHead, 
								char *xml, 
								int *length)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlColumnS *ColumnElement = NULL;

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

	for(SegmentElement = listHead; SegmentElement; 
		SegmentElement = SegmentElement->Next){
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", SegmentElement->ID);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);

		int nNameSeg = SegmentElement->ID;

		for(ColumnElement = SegmentElement->ColumnHeader; ColumnElement; 
			ColumnElement = ColumnElement->Next){
			memset(buf, 0, 10);
			sprintf_s(buf, 10, "%d", ColumnElement->ID);

			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", buf);
			//Cloumn->SetAttribute("SOURCE", ColumnElement->Source);

			Cloumn->SetAttribute("VALUE", ColumnElement->Value);

			Segment->LinkEndChild(Cloumn);   
		}
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
	std::string strXML;
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	int n = iQueryInfo(source,  szQuery);
	if (n != 0){
		return n;
	}
	GetQueryInfoForOne(szQuery, szResult);
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
			//item.source = M1SourceReserver[i];
		} else {
			iter = mapCpuInfo.find(cpuId);
			if (iter != mapCpuInfo.end()) {
				item.value = iter->second.value;
				//item.source = iter->second.source;
			} else {
				item.value = M1Reserver[i];
				//item.source = M1SourceReserver[i];
			}
		}
		vecData.push_back(item);
	}
	return M1ConvertXmlByArray(vecData, 2, dest, length);
}

static int M12CpuXml(char *xml, const std::map<int, dataItem> &mapInfo) 
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


static struct XmlSegmentS*  CpuConvertXmltoList(char *xml)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;

	struct XmlColumnS  *ColumnElement = NULL;
	struct XmlColumnS  *CurrColumnElement = NULL;
	struct XmlColumnS  *TempColumnElement = NULL;

	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	char *HexString = NULL;
	char *tmpString = NULL;
	int ElemLen = 0, padding = 0;
	int realLen = 0;

	struct XmlSegmentS *XmlListHead = g_XmlListHead->SegHeader;

	// 解析XML语句
	XmlDoc.Parse(xml);
	RootElement = XmlDoc.RootElement();

	Segment = RootElement->FirstChildElement();
	while(Segment){
		int ID = atoi(Segment->Attribute("ID"));
		SegmentElement = FindSegmentByID(XmlListHead, ID);
		if (NULL == SegmentElement)
			break;

		TempSegmentElement = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(TempSegmentElement, SegmentElement, sizeof(struct XmlSegmentS));
		TempSegmentElement->ColumnHeader = NULL;
		TempSegmentElement->ColumnTailer = NULL;
		TempSegmentElement->Next = NULL;

		// 加入新的元素
		if(result){
			CurrSegmentElement->Next = TempSegmentElement;
			CurrSegmentElement = TempSegmentElement;
		} else{
			CurrSegmentElement = TempSegmentElement;
			result = CurrSegmentElement;
		}

		padding = 0;
		Colum = Segment->FirstChildElement();
		while(Colum){
			int nColumnID = atoi(Colum->Attribute("ID"));
			ColumnElement = FindColumnByID(SegmentElement->ColumnHeader, nColumnID);

			if (NULL == ColumnElement) {
				Colum = Colum->NextSiblingElement();
				continue;  //search for the next column node
			}

			TempColumnElement = CloneColmn(ColumnElement, 1);
			if (TempColumnElement == NULL) {
				Colum = Colum->NextSiblingElement();
				continue;  //search for the next column node
			}

			//为了节省空间
			if (TempSegmentElement->datatype == eRecType) {
				padding = 2; //记录文件需要填充2个字节
			}
			std::string strColum = Colum->Attribute("VALUE");
			ElemLen = (int)strColum.length();

			//考虑每个字段字符转化当不为Ans类型时，需要转化
			CpuInfo info = TempColumnElement->CheckInfo.CpuInfo;

			//进行数据转换ans cn b类型
			if (info.itemtype != eAnsType) {
				tmpString = NULL;
				char tmpArray[64];
				memset(tmpArray, 0, sizeof(tmpArray));
				HexString = TempColumnElement->Value;
				memset(TempColumnElement->Value, 0xFF, info.ColumnByte * 2);
				ElemLen = ElemLen < info.ColumnByte * 2 ? ElemLen : info.ColumnByte * 2;

				if (ElemLen > sizeof(tmpArray)) {
					tmpString = (char*)malloc(info.ColumnByte * 2 + 1);
					memset(tmpString, 0, info.ColumnByte * 2 + 1);
				} else {
					tmpString = &tmpArray[0];
				}

				if (ElemLen % 2) { //不是2的倍数时，补充‘f’
					tmpString[ElemLen] = 'f';
				}
				memcpy(tmpString, strColum.c_str(), ElemLen);

				//将小数点转换为'a'
				if (info.itemtype == eCnType) {
					size_t pos = strColum.find('.');
					if (pos != -1) {
						*(tmpString + pos) = 'a';
					}
				}


				HexstrToBin((BYTE*)HexString+padding, (BYTE*)tmpString, ElemLen);

				if (ElemLen > sizeof(tmpArray)) {
					SAFE_DELETE_C(tmpString);
				}
			} else {
				HexString = TempColumnElement->Value;
				realLen = info.ColumnByte < ElemLen ? info.ColumnByte : ElemLen;
				memcpy(HexString+padding, strColum.c_str(), realLen);
			}

			// 加入链表
			if(CurrSegmentElement->ColumnHeader){
				CurrSegmentElement->ColumnTailer->Next = TempColumnElement;
				CurrSegmentElement->ColumnTailer = TempColumnElement;
			} else{
				CurrSegmentElement->ColumnHeader = TempColumnElement;
				CurrSegmentElement->ColumnTailer = TempColumnElement;
			}

			// 向后迭代
			Colum = Colum->NextSiblingElement();
		}

		// 向后迭代
		Segment = Segment->NextSiblingElement();
	}

	// 返回结果
	return result;
}
/**
 *
 */
static int InitCpuGlobalList() 
{
	if (g_CPUXmlListHead) {
		return 0;
	}

	// 在资源文件里边提取XML文件并且初始化他
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CardLib.dll");
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_XML4),"XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes){	
		LogPrinter("加载xml文件错误\n");
		DBGCore( "加载xml文件错误\n");
		return CardInitErr;
	}

	// 如果已经分配了链表
	if(g_CPUXmlListHead)
		return -2;

	// 初始化全局列表
	InitionCpuGList(pvRes);
	return 0;
}

extern "C" {
	struct RecFolder g_recIndex[30];
}

static void InitGlobalMap()
{
	g_sourceValueMap.insert(std::make_pair("STAGENO", QueryColum(2, 4, "STAGENO", "000000")));
	g_segMap["CARDNO"] = 201;
	g_segMap["MEDICARECERTIFICATENO"] = 207;
}

#define  INSERT_SEGS(list, seg, id, name)											\
	memcpy(g_recIndex[id-1].section, g_recIndex[seg->ID-1].section, 10);            \
	memcpy(g_recIndex[id-1].subSection, g_recIndex[seg->ID-1].subSection, 10);      \
	memcpy(g_recIndex[id-1].fileName, name, strlen(name));							\
	seg->ID = id;																	\
	list->SegTailer->Next = seg;													\
	list->SegTailer = seg;															\


static int InitionCpuGList(char *xmlstr)
{
	if (g_CPUXmlListHead == NULL){
		g_CPUXmlListHead = (struct XmlProgramS*)malloc(sizeof(struct XmlProgramS));
		g_CPUXmlListHead->SegHeader = g_CPUXmlListHead->SegTailer = NULL;
		InitGlobalMap();
	}

	struct ColCell 
	{
		std::string Source;
		int itemtype;
		int nByte;
	};
	struct XmlSegmentS *pTmp = NULL;
	CMarkup xml;
	int nSegID = 0;
	xml.SetDoc(xmlstr);
	if (!xml.FindElem("PROGRAMS")){
		return -1;
	}
	xml.IntoElem();
	if (! xml.FindElem("PROGRAM")){
		return -1;
	}
	std::string AppContext = xml.GetAttrib("Context");

	xml.IntoElem();
	while (xml.FindElem("SEGMENT")) {
		std::string szTmp;
		int RecFlag = atoi(xml.GetAttrib("REC").c_str());
		struct XmlSegmentS *pSeg = (struct XmlSegmentS*)malloc(sizeof(struct XmlSegmentS));
		pSeg->ColumnHeader = pSeg->ColumnTailer = NULL;
		pSeg->Next = NULL;
		pSeg->datatype = (eFileType)RecFlag;
		pSeg->offset = 0;
		pSeg->ID = atoi(xml.GetAttrib("ID").c_str());
		nSegID = pSeg->ID;
		szTmp = xml.GetAttrib("SECTION").c_str();
		memcpy(g_recIndex[nSegID-1].section, szTmp.c_str(),szTmp.size());

		szTmp = xml.GetAttrib("SUBSECTION").c_str();
		memcpy(g_recIndex[nSegID-1].subSection, szTmp.c_str(), szTmp.size());
		memcpy(pSeg->Target, xml.GetAttrib("SOURCE").c_str(),sizeof(pSeg->Target));

		std::string cFileType = xml.GetAttrib("FILE");

		xml.IntoElem();
		int nIDCounts = 0, subflag=0;
		int nOffset = 0;
		XmlColumnS *pColTmp = NULL;
		while (xml.FindElem("COLUMN")) {
			++nIDCounts;
			XmlColumnS *pColumnS = (XmlColumnS*)malloc(sizeof(XmlColumnS));
			pColumnS->Next = NULL;
			pColumnS->parent = pSeg;
			strcpy(pColumnS->Source, xml.GetAttrib("SOURCE").c_str());
			if (nSegID < 11) 
				pColumnS->ID = atoi(xml.GetAttrib("ID").c_str());
			else
				pColumnS->ID = nIDCounts;

			pColumnS->CheckInfo.CpuInfo.ColumnByte = atoi(xml.GetAttrib("OCCUPYBYTE").c_str());
			pColumnS->Offset = nOffset;
			pColumnS->CheckInfo.CpuInfo.itemtype = (eItemType)atoi(xml.GetAttrib("TYPE").c_str());
			nOffset += pColumnS->CheckInfo.CpuInfo.ColumnByte;
			pColumnS->Value = NULL;

			//插入到Segment中
			if (pSeg->ColumnHeader == NULL) {
				pSeg->ColumnHeader = pColumnS;
				pSeg->ColumnTailer = pSeg->ColumnHeader;

			}else {
				pSeg->ColumnTailer->Next = pColumnS;
				pSeg->ColumnTailer = pColumnS;
			}

			//发现有子列，将子列加入到链表中
			subflag = atoi(xml.GetAttrib("SUB").c_str());
			if (subflag == 1) {
				xml.IntoElem();
				while (xml.FindElem("SUBCOL"))
				{
					int counts = atoi(xml.GetAttrib("COUNT").c_str());
					std::vector<ColCell*> vecCell;
					xml.IntoElem();
					while (xml.FindElem("SUBCOLUMN"))
					{
						ColCell *pCell = new ColCell;
						pCell->nByte = atoi(xml.GetAttrib("OCCUPYBYTE").c_str());
						pCell->itemtype = atoi(xml.GetAttrib("TYPE").c_str());
						pCell->Source = xml.GetAttrib("SOURCE");
						vecCell.push_back(pCell);
					}
					xml.OutOfElem();

					char SourceName[30];
					memset(SourceName, 0, sizeof(SourceName));
					for (int Index = 0; Index < counts; ++Index)
					{
						for (size_t i=0; i<vecCell.size(); ++i)
						{
							ColCell *pCell = vecCell[i];
							++nIDCounts;
							XmlColumnS *pColumnS = (XmlColumnS*)malloc(sizeof(XmlColumnS));
							pColumnS->Next = NULL;
							sprintf(SourceName, "%s%d", pCell->Source.c_str(), Index+1);
							strcpy(pColumnS->Source, SourceName);
							pColumnS->ID = nIDCounts;
							pColumnS->CheckInfo.CpuInfo.ColumnByte = pCell->nByte;
							pColumnS->Offset = nOffset;
							pColumnS->CheckInfo.CpuInfo.itemtype = (eItemType)pCell->itemtype;
							pColumnS->parent = pSeg;
							nOffset += pColumnS->CheckInfo.CpuInfo.ColumnByte;
							pColumnS->Value = NULL;

							pSeg->ColumnTailer->Next = pColumnS;
							pSeg->ColumnTailer = pColumnS;
						}
					}

					for (size_t i=0; i<vecCell.size(); ++i)
					{
						ColCell *pCell = vecCell[i];
						delete pCell;
					}
					vecCell.clear();
				}
				xml.OutOfElem();
			}
		}
		xml.OutOfElem();

		if (g_CPUXmlListHead->SegHeader == NULL) {
			g_CPUXmlListHead->SegHeader = pSeg;
			g_CPUXmlListHead->SegTailer = pSeg;
			memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
		} else {

			if (cFileType == std::string("EE01--03")) {
				INSERT_SEGS(g_CPUXmlListHead, pSeg, nSegID, "EE01");
				pTmp = CloneSegment(pSeg, 0);
				nSegID++;
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "EE02")
					nSegID++;
				pTmp = CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "EE03")
			} else if (cFileType == std::string("ED01--05")) {

				INSERT_SEGS(g_CPUXmlListHead, pSeg, nSegID, "ED01")
					pTmp = CloneSegment(pSeg, 0);
				nSegID++;
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "ED02")
					nSegID++;
				pTmp = CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "ED03")
					nSegID++;
				pTmp = CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "ED04")
					nSegID++;
				pTmp = CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CPUXmlListHead, pTmp, nSegID, "ED05")
			} else {

				memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
				g_CPUXmlListHead->SegTailer->Next = pSeg;
				g_CPUXmlListHead->SegTailer = pSeg;
			}
		}

	}
	xml.OutOfElem();
	xml.OutOfElem();

	return 0;
}

static int InitM1GlobalList() 
{
	if (g_M1XmlListHead) {
		return 0;
	}

	// 在资源文件里边提取XML文件并且初始化他
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CardLib.dll");
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_XML2),"XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes){	
		LogPrinter("加载xml文件错误\n");
		DBGCore( "加载xml文件错误\n");
		return CardInitErr;
	}

	// 如果已经分配了链表
	if(g_M1XmlListHead)
		return -2;

	// 初始化全局列表
	InitionM1GList(pvRes);
	return 0;
}
static int InitionM1GList(char *xml)
{
	if (g_M1XmlListHead == NULL){
		g_M1XmlListHead = (struct XmlProgramS*)malloc(sizeof(struct XmlProgramS));
		g_M1XmlListHead->SegHeader = g_M1XmlListHead->SegTailer = NULL;
	}

	TiXmlDocument *XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Program;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;

	XmlDoc = new TiXmlDocument();
	XmlDoc->Parse(xml);
	RootElement = XmlDoc->RootElement();

	Program = RootElement->FirstChildElement();
	Segment = Program->FirstChildElement();
	while(Segment) 
	{
		struct XmlSegmentS *pSeg = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memset(pSeg, 0, sizeof(struct XmlSegmentS));
		pSeg->ColumnHeader = pSeg->ColumnTailer = NULL;

		// 插入链表
		if(g_M1XmlListHead->SegHeader) {
			g_M1XmlListHead->SegTailer->Next = pSeg;
			g_M1XmlListHead->SegTailer = pSeg;
		}  else {
			g_M1XmlListHead->SegHeader = g_M1XmlListHead->SegTailer = pSeg;
		}

		// 对元素进行赋值
		pSeg->ID =  atoi(Segment->Attribute("ID"));

		// 插入链表中元素
		Colum = Segment->FirstChildElement();
		while(Colum) 
		{
			struct XmlColumnS *pColmn = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memset(pColmn, 0, sizeof(struct XmlColumnS));
			pColmn->Next = NULL;
				
			// 插入元素
			if(pSeg->ColumnHeader) {
				pSeg->ColumnTailer->Next = pColmn;
				pSeg->ColumnTailer = pColmn;
			} else{
				pSeg->ColumnHeader = pColmn;
				pSeg->ColumnTailer = pColmn;
			}

			// 对元素进行赋值
			pColmn->parent = pSeg;
			pColmn->ID = atoi(Colum->Attribute("ID"));
			strcpy_s(pColmn->Source ,30 ,Colum->Attribute("SOURCE"));
			pColmn->Offset = atoi(Colum->Attribute("OFFSET"));
			pColmn->CheckInfo.M1Info.ColumnBit = atoi(Colum->Attribute("COLUMNBIT"));
			pColmn->CheckInfo.M1Info.Mask = atoi(Colum->Attribute("MASK"));
			
			// 迭代下一个元素
			Colum = Colum->NextSiblingElement();
		}
		
		// 迭代下一个元素
		Segment = Segment->NextSiblingElement();
	}

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


static int QueryItem(CardType type, const char *name, char *xml, 
					  int &nLen, QueryColum &stQuery)
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
	struct XmlSegmentS *pFindSeg = getSegmentByColumName(g_XmlListHead->SegHeader, name);
	if (pFindSeg == NULL) {
		nLen = -1;
		return NULL;   
	}

	//bin file can read by random , while others must read by one section and extract own name. 
	if (pFindSeg->datatype == eBinType) {
		pBinColum = FindColumByColumName(pFindSeg, name);
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

				queryItem = CloneColmn(pColum, 1);
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

/**
*
*/
static int iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;
	std::map<int,std::vector<QueryColum> > mapSeg;
	for (size_t i=0; i<v.size(); ++i){
		QueryColum stQuery = v[i];
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
	std::map<int,std::vector<QueryColum> >::iterator iter= mapSeg.begin();
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

static int CheckCardXMLValid(std::string &pszCardXml)
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

/**
 *
 */
int __stdcall iGetCardVersion(char *pszVersion)
{
	int res = 0;

	memcpy(pszVersion, "1.0", sizeof("1.0"));

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
	char szSystem[NAME_MAX_LEN];
	ZeroMemory(szSystem, sizeof(szSystem));

	ReadConfigFromReg(szSystem);

	// 对设备进行初始化
	g_bPreLoad = (apt_InitCoreDevice(szSystem)==0);
	g_bCardOpen = g_bPreLoad;

	InitCardOps();
	return g_bPreLoad==TRUE ? 0:-1;
}

int __stdcall iCardDeinit()
{
	if (g_CPUXmlListHead) {
		DestroyList(g_CPUXmlListHead->SegHeader, 0);
	}

	if (g_M1XmlListHead) {
		DestroyList(g_M1XmlListHead->SegHeader, 0);
	}

	g_XmlListHead = NULL;
	g_CPUXmlListHead = NULL;
	g_M1XmlListHead = NULL;
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


int __stdcall iScanCard()
{
	ASSERT_OPEN(g_bCardOpen);
	return apt_ScanCard();
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
	QueryItem(g_CardOps->cardAdapter->type, "CARDNO", strCardNo, nLen);

	int status = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, '0');
	strlwr(strCardNo);
	int stat = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, 'f');
	return (status & stat) == 0 ? 0 : CardIsNotEmpty;
}

/**
 *
 */

static int _iReadInfo(int flag, char *xml) 
{
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int status = CardProcSuccess;
	
	list = GetXmlSegmentByFlag(flag);
	if (list == NULL) {
		CreateResponXML(CardXmlErr, err(CardXmlErr), xml);
		return CardXmlErr;
	}

	// 获取读写链表
	RequestList = apt_CreateRWRequest(list, 0, g_CardOps->cardAdapter->type);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		CreateResponXML(status, err(status), xml);
		goto done;
	}

	// 设备的真实读取
	status = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	// 通过链表产生XML字符串
	g_CardOps->iConvertXmlByList(list, xml, &length);

	// 销毁读写请求链表
	apt_DestroyRWRequest(RequestList, 0);

done:
	DestroyList(list, 1);
	return status;
}

int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	if (xml == NULL) {
		return CardReadErr;
	}
	
	ISSCANCARD;
	int bNHInfoRead = 0;
	if (g_CardOps->cardAdapter->type == eM1Card ) {

		// 根据Flag产生List,健康档案号在第五区
		if ((flag & 0x2) && !(flag & 0x10)){
			g_OnlySecond = true;
			flag = flag | 0x10;
		}
	} else {
		if ((flag & 0x2) == 0x2) {
			bNHInfoRead = 1;
			flag = 0x1 | 0x2 | 0x8 | 32 | 64 | 128;
		}
		
	}

	char readxml[1024*12];
	ZeroMemory(readxml, sizeof(readxml));

	int status = _iReadInfo(flag, readxml);

	//convert cpu xml to m1 xml;
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
	return status == 0 ? CardProcSuccess : CardReadErr;
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
		return CardXmlErr;
	}

	// 产生读写链表
	RequestList = apt_CreateRWRequest(XmlList, 0, eCPUCard);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		goto done;
	}

	// 对设备进行真实的写
	status = g_CardOps->cardAdapter->iWriteCard(RequestList, g_CardOps->cardAdapter);

	// 销毁读写链表
	apt_DestroyRWRequest(RequestList, 0);

	// 销毁XML链表
done:
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
		if (id <= 3) {
			return 0;
		} else if (id < 11) {
			vecFlag.push_back(id);
		} else {
			vecBin.push_back(id);
		}
	}
	xml.OutOfElem();
	return 1;
}

static int WriteFile(char *filename,const char *xml)
{
	FILE *handle;

	fopen_s(&handle, filename, "w");
	fwrite(xml, strlen(xml), 1, handle);
	fclose(handle);

	return 0;
}

int __stdcall iWriteInfo(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	int len = strlen(xml) + 1;
	int status = 0;
	
	std::string xmlStr(xml);
	if (len == 1 || CheckCardXMLValid(xmlStr) < 0){
		LogPrinter("CardXML:Check Error\n");
		return CardXmlErr;
	}
	//WriteFile("234.xml", xmlStr.c_str());
	
	ISSCANCARD;

	if (g_CardOps->cardAdapter->type == eM1Card) {
		status =  _iWriteInfo((char*)xmlStr.c_str());
		goto done;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {

		std::map<int, dataItem> mapInfo, mapCpuInfo;
		int flag = 0, isCpuWrite = 0, isRec = 1;

		//add verify cpu write xml, return vector segflag
		std::vector<int> vecRecFlag;
		std::vector<int> vecBinFlag;
		isCpuWrite = checkCpuWriteXml((char*)xmlStr.c_str(), vecRecFlag, vecBinFlag);
		if (isCpuWrite == 1) {
			if (vecBinFlag.size() > 0) {
				isRec = 0;
			}

			if (isRec) {   //记录文件，记录要回写的块
				for (size_t i=0; i<vecRecFlag.size(); i++) {
					if (vecRecFlag[i] > 3) {
						SETBIT(flag, (vecRecFlag[i]-1));
					}	
				}
				xml2Map((char*)xmlStr.c_str(), mapCpuInfo, eCPUCard, false);
			}
		} else {
			if (vecRecFlag.size() > 0 || vecBinFlag.size() > 0) {

				cout<<"CPU卡无法回写除2以外的M1数据"<<endl;
				DBGCore("CPU卡无法回写除2以外的M1数据");
				return CardWriteErr;
			}

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

		//status =  _iWriteInfo(xml);

		if (isRec) {
			char convertXml[2048];
			ZeroMemory(convertXml, sizeof(convertXml));
			status = _iReadInfo(flag, convertXml);
			if (status) {
				return CardReadErr;
			}

			if (mapCpuInfo.size() > 0) {
				M12CpuXml(convertXml, mapCpuInfo);
			}
			ISSCANCARD;

			status =  _iWriteInfo(convertXml);
		} else {  //cpu bin write
			status =  _iWriteInfo((char*)xmlStr.c_str());
		}
	} 

done:
	return status == 0 ? 0 : CardWriteErr;
}


/**
*
*/
int __stdcall iQueryInfo(char *name, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	char readbuf[256];
	int	nLen = 0;
	std::vector<std::string> vecQuery;
	std::vector<QueryColum> vecResult;
	struct RWRequestS	*RequestList = NULL;

	vecQuery = split(name, "|");
	for (size_t i=0; i<vecQuery.size(); ++i){
		std::string &strColum = vecQuery[i];

		QueryColum stQuery;
		ZeroMemory(readbuf, sizeof(readbuf));
		if (QueryItem(g_CardOps->cardAdapter->type, 
			strColum.c_str(), readbuf, nLen, stQuery) == 0){
			vecResult.push_back(stQuery);
		}
	}
	if (vecResult.size() == 0){
		return CardReadErr;
	}
	if (g_CardOps->cardAdapter->type == eCPUCard) {
		CpuConvertRetPos(vecResult);
	}
	iCreateXmlByVector(vecResult, xml, &nLen);
	return 0;
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
		if (nTimeOut >= TIMEOUT){
			cout<<"寻卡失败"<<endl;
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
	if (CheckCardXMLValid(xmlStr) < 0){
		LogPrinter("CardXML:Check Error\n");
		//DBGCore( "CardXML Check Error\n");
		return CardXmlErr;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		LogPrinter("CPU暂不支持\n");
		return CardNoSupport;
	}

	//M1制卡
	XmlSegmentS *seg = g_CardOps->iConvertXmltoList((char*)xmlStr.c_str());
	seg = FindSegmentByID(seg, 2);
	int nRet = 0;
	if (seg != NULL){
		unsigned char KeyB[6];
		XmlColumnS *stColumn = FindColumnByID(seg->ColumnHeader, 1);
		if (seg->ColumnHeader->Value[0] == '0'){
			stColumn = FindColumnByID(seg->ColumnHeader, 7);
		}
		iGetKeyBySeed((unsigned char *)stColumn->Value, KeyB);

		nRet = iWriteInfo((char*)xmlStr.c_str());
		DBGCore( "写卡数据结果:%d\n", nRet);
		LogPrinter("回写数据：%d\n", nRet);

		nRet = InitPwd(KeyB);
		DBGCore( "重置密码结果%d\n", nRet);
		LogPrinter( "重置密码结果%d\n", nRet);

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
	ASSERT_OPEN(g_bCardOpen);
	std::string strCheckWSDL = pszCardCheckWSDL;
	std::string strServerURL = pszCardServerURL;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	std::string strCardNO;
	if (iQueryInfo("CARDNO", szQuery) != 0){
		CreateResponXML(3, "获取卡号失败", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfoForOne(szQuery, strCardNO);
	
	int nCheckCode = CardProcSuccess;

	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	char *strCheckParams = (char*)malloc(sizeof(char)*2048);
	memset(strCheckParams, 0, 1024*2);
	CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strCheckParams;

	_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

	pReturn.nh_USCOREpipeResult = (char*)malloc(sizeof(char)*1024);

	m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);
	if(m_CardObj.soap->error) {   
		bSuccessed = false;
		CreateResponXML(3, "与服务器连接失败", strResult);
		strcpy(pszXml, strResult);
	} else {
		std::string strRetCode, strStatus;
		std::string strXML = pReturn.nh_USCOREpipeResult;
		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			bSuccessed = false;
			CreateResponXML(1, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		} else {
			nCheckCode = atoi(strStatus.c_str());
			strCheckDesc.clear();
			if (GetCardStatus(nCheckCode, strCheckDesc) == 0){
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			}
		}
	}
	SAFE_DELETE_C(strCheckParams);
	SAFE_DELETE_C(pReturn.nh_USCOREpipeResult);

	if (bSuccessed){
		char szRead[4096];
		memset(szRead, 0, sizeof(szRead));
		iReadInfo(2, szRead);
		strcpy(pszXml, szRead);
	}

	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed ? 0 : CardCheckError;
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



int __stdcall iRegMsgForNH(char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen);
	if (g_CardOps->cardAdapter->type != eM1Card) {
		return CardNoSupport;
	}
	std::string strServerURL = pszCardServerURL;

	std::string strXML;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	std::string strCardNO;
	int n = iQueryInfo("CARDNO", szQuery);
	if (n != 0){
		CreateResponXML(3, "获取卡号失败", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfoForOne(szQuery, strCardNO);
	
	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	char* strRegParams = new char[1024];
	memset(strRegParams, 0, 1024);
	CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strRegParams;

	_ns1__nh_USCOREpipeResponse pReturn;

	pReturn.nh_USCOREpipeResult = new char[4096];

	m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);

	if(m_CardObj.soap->error) {   
		bSuccessed = false;
		CreateResponXML(3, "与服务器连接失败", strResult); 
		strcpy(pszXml, strResult);
	} else {
		std::string strRetCode, strStatus;
		strXML = pReturn.nh_USCOREpipeResult;

		GetCheckState(strXML, strRetCode, strStatus);
		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			bSuccessed = false;
			CreateResponXML(3, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		} else{
			if (strStatus.size() > 0 && CheckCardXMLValid(strStatus) == 0){ 
				FormatWriteInfo(strStatus.c_str(), strResult);
				int nState = iWriteInfo(strResult);
				if (nState != 0){
					bSuccessed = false;
					memset(strResult, 0, sizeof(strResult));
					CreateResponXML(2, "卡回写失败", strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
	}
	delete [] strRegParams;
	delete [] pReturn.nh_USCOREpipeResult;

	if (bSuccessed){
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2 , strResult);
		strcpy(pszXml, strResult);
	}
	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed ? 0 : CardRegError;
}

std::map<int, std::map<int, std::string> > mapLogConfig; 
int g_rwFlag = 0;
char g_processName[20];


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

int __stdcall iRegMsgForNHLog(char *pszCardServerURL, char* pszLogXml, char* pszXml)
{
	int status = iRegMsgForNH(pszCardServerURL, pszLogXml);
	if (status != CardProcSuccess) {
		return CardRegError;
	}
	CXmlUtil::paserLogXml(pszLogXml, mapLogConfig);

	std::map<int, std::string> mapReaderInfo;
	CXmlUtil::parseHISXml(pszXml, mapReaderInfo);

	g_rwFlag = 0;
	strcpy(g_processName, "iRegMsgForNHLog");
	geneHISLog(pszXml, mapReaderInfo);
	return CardProcSuccess;
}


int __stdcall iReadCardMessageForNHLocal(char* pszLogXml, char* pszXml)
{
	std::string strMedicalID;
	int n = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (n != 0) {
		CreateResponXML(3, "获取参合号失败", pszXml);
		return 3;
	}
	return iCheckMsgForNHLocal(pszLogXml, pszXml);
}

//卡校验 黑名单校验
int __stdcall iCheckMsgForNHLocal(char* pszLogXml, char* pszXml)
{
	int status = iCheckException(pszLogXml, pszXml);
	if (status != CardProcSuccess) {
		return status;
	}

	if (CardProcSuccess != iReadInfo(2, pszXml)) {
		return CardReadErr;
	}
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
	CXmlUtil::paserLogXml(pszLogXml, mapLogConfig);

	std::map<int, std::string> mapReaderInfo;
	CXmlUtil::parseHISXml(pszXml, mapReaderInfo);

	g_rwFlag = 0;
	strcpy(g_processName, "iReadCardMessageForNHLog");
	geneHISLog(pszXml, mapReaderInfo);
	return CardProcSuccess;
}

int __stdcall iReadCardMessageForNH(char *pszCardCheckWSDL, char *pszCardServerURL, char* pszXml)
{
	std::string strCheckWSDL = pszCardCheckWSDL;
	std::string strServerURL = pszCardServerURL;

	std::string strXML;
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	std::string strMedicalID;
	int n = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (n != 0) {
		CreateResponXML(3, "获取参合号失败", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	if (strMedicalID.size() == 0) {
		CreateResponXML(3, "参合号为空", strResult);
		strcpy(pszXml, strResult);
		return CardMedicalFailed;
	}

	std::string strCardNO;
	n = ParseValueQuery("CARDNO", strCardNO);
	if (n != 0 || strCardNO.size() == 0){
		CreateResponXML(3, "获取卡号失败或者卡号为空", strResult);
		strcpy(pszXml, strResult);
		return 3;
	}

	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	if (IsMedicalID(strMedicalID)){
		char *strCheckParams = new char[1024];
		memset(strCheckParams, 0, 1024);
		CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);

		_ns1__nh_USCOREpipe pCheck;
		pCheck.parms = strCheckParams;

		_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

		pReturn.nh_USCOREpipeResult = new char[1024];

		m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);
		if(m_CardObj.soap->error) {   
			bSuccessed = false;
			CreateResponXML(3, "与服务器连接失败", strResult); 
			strcpy(pszXml, strResult);
		} else {
			std::string strRetCode, strStatus;
			strXML = pReturn.nh_USCOREpipeResult;
			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			} else{
				int nCardStatus = atoi(strStatus.c_str());
				strCheckDesc.clear();

				if (GetCardStatus(nCardStatus, strCheckDesc) == 0){
					bSuccessed = false;
					CreateResponXML(1, strCheckDesc.c_str(), strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
		SAFEARRAY_DELETE(strCheckParams);
		SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);

		if (!bSuccessed){
			soap_end(m_CardObj.soap);   
			soap_done(m_CardObj.soap); 
			return 1;
		}
	}

	if (bSuccessed && g_CardOps->cardAdapter->type == eM1Card){
		char* strRegParams = new char[1024];
		memset(strRegParams, 0, 1024);
		CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

		_ns1__nh_USCOREpipe pCheck;
		pCheck.parms = strRegParams;

		_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

		pReturn.nh_USCOREpipeResult = new char[4096];

		m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);

		if(m_CardObj.soap->error){   
			bSuccessed = false;
			CreateResponXML(3, "与服务器连接失败", strResult); 
			strcpy(pszXml, strResult);
		}  else {

			std::string strRetCode, strStatus;
			strXML = pReturn.nh_USCOREpipeResult;

			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
				bSuccessed = false;
				CreateResponXML(3, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			} else {
				FormatWriteInfo(strStatus.c_str(), strResult);
				int nState = iWriteInfo(strResult);
				if (nState != 0){
					bSuccessed = false;
					memset(strResult, 0, sizeof(strResult));
					CreateResponXML(2, "卡回写失败", strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
		SAFEARRAY_DELETE(strRegParams);
		SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);
	}

	if (bSuccessed){
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2, strResult);
		strcpy(pszXml, strResult);
	}
	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed==true ? 0 : 2;
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
	CExceptionCheck check(pszLogXml);
	int status = check.filterForbidden(pszXml);
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
		g_CardOps = &CpuCardOps;
		InitCpuGlobalList();
		g_XmlListHead = g_CPUXmlListHead;
	} else {
		g_CardOps = &M1CardOps;
		InitM1GlobalList();
		g_XmlListHead = g_M1XmlListHead;
	}
	return 0;
}
