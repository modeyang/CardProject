#include <vector>
#include <stdio.h>
#include <time.h>
#include <string>
#include <map>

#include "BHGX_CardLib.h"
//#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
//#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"
#include "ns_pipeClient_1/NHOneCardServiceSoapBinding.nsmap"
#include "ns_pipeClient_1/soapNHOneCardServiceSoapBindingProxy.h"
#include "public/debug.h"
#include "public/liberr.h"
#include "tinyxml/headers/tinyxml.h"
#include "adapter.h"
#include "public/algorithm.h"
#include "resource.h"
#include "BHGX_Printer.h"
#include "public/ConvertUtil.h"
#include "WebServiceAssist.h"
#include "public/Markup.h"
#include "Encry/DESEncry.h"
#include "public/Authority.h"
#include "StringUtil.h"


using namespace std;
#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")


#define TIMEOUT		15000
#define ASSERT_INIT(a)      \
	if (a != TRUE){         \
	return -1;				\
	}                       \


#define ASSERT_OPEN(a)      \
	if (!a)                 \
	return CardNotOpen;		\

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
};


CBHGX_Printer *m_pBHPrinter = NULL;
BOOL g_bPreLoad = FALSE;
BOOL g_bCardOpen = FALSE;


/**
 * 全局的数据结构
 */
struct XmlSegmentS *XmlListHead = NULL;
bool bTelRW = false;

typedef  std::map<std::string, struct XmlColumnS *> XmlColumnMapT;
XmlColumnMapT XmlColumnMap; 

static struct XmlColumnS  
*CloneColmn(struct XmlColumnS *ColmnElement);

static struct XmlSegmentS 
*CloneSegment(struct XmlSegmentS *SegmentElement);

static struct XmlSegmentS *
GetXmlSegmentByFlag(int flag);

static struct XmlSegmentS *
FindSegmentByID(struct XmlSegmentS *listHead, int ID);

static struct XmlColumnS *
FindColumnByID(struct XmlColumnS *listHead, int ID);

static int 
CombineColValue(struct XmlColumnS *ColumnElement, 
				std::pair<int,int> pairCol, 
				char sep, struct XmlSegmentS *Segment);

static int 
InsertColumnBySplite(struct XmlSegmentS *SegmentElement, 
					 std::map<int,std::string> &mapSplite, 
					 struct XmlSegmentS *pSeg, 
					 std::pair<int,int> pairCol);

static int 
iConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length);

static struct XmlSegmentS* 
ConvertXmltoList(char *xml);

static int 
InitionGList(char *xml);

static void 
DestroyList(struct XmlSegmentS *listHead);

static void 
ReadConfigFromReg(char *name);

static int 
iGetKeyValue(const unsigned char *seed,  unsigned char *key);

static int 
_iQueryInfo(const char *name, char *xml, int &nLen, QueryColum &stQuery);

static int 
iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length);

static struct CreateKeyInfoS* 
CreateCardKeyInfo(unsigned char *seed);

static int 
CheckCardXMLValid(std::string &pszCardXml);


/**
 * CloneColmn - 克隆XmlColmn元素的数据结构
 * @ ClomnElement 被克隆的元素
 */
static struct XmlColumnS *CloneColmn(struct XmlColumnS *ColmnElement)
{
	struct XmlColumnS *result = NULL;

	if(ColmnElement == NULL)
		return NULL;

	result = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
	if(result == NULL) 
	{
		return NULL;
	}
	
	memcpy(result, ColmnElement, sizeof(struct XmlColumnS));
	result->Next = NULL;

	return result;
}

/**
 * CloneSegment - 克隆XmlSegmentS元素数据结构
 * @SegmentElement 被克隆的元素
 */
static struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement)
{

	struct XmlColumnS *OrigColumnElement = NULL;	// 元SegmentElegment对应元素
	struct XmlColumnS *CurrColumnElement = NULL;	// 当前Result对应的当前的元素
	struct XmlColumnS *TempColumnElement = NULL;	// Result对应的临时元素
	struct XmlSegmentS *result = NULL;

	if(SegmentElement != NULL)
	{
		result = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(result, SegmentElement, sizeof(struct XmlSegmentS));
		result->Column = NULL;
		result->Next = NULL;

		// 复制下边的链表结构
		for(OrigColumnElement = SegmentElement->Column; OrigColumnElement; OrigColumnElement = OrigColumnElement->Next)
		{
			TempColumnElement = CloneColmn(OrigColumnElement);

			// 将新生成的元素加入到链表中
			if(result->Column)	// 已经有元素插入链表，这种情况比较常见，在前边
			{
				CurrColumnElement->Next = TempColumnElement;
				CurrColumnElement = TempColumnElement;
			}
			else				// 第一个元素插入链表
			{
				CurrColumnElement = TempColumnElement;
				result->Column = CurrColumnElement;
			}
		}
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
	int nFlag = 1;

	for(SegmentElement=XmlListHead; SegmentElement; SegmentElement = SegmentElement->Next)
	{
		// 表明这个位置被设置
		int nReadFlag = nFlag & flag;
		int nSegFlag = (1 << (SegmentElement->ID-1)) & nFlag;
		if(nReadFlag > 0 && nSegFlag > 0)
		{
			TempSegmentElement = CloneSegment(SegmentElement);

			// 将新生成的链表加入
			if(result)
			{
				CurrSegmentElement->Next = TempSegmentElement;
				CurrSegmentElement = TempSegmentElement;
			}
			else 
			{
				CurrSegmentElement = TempSegmentElement;
				result = CurrSegmentElement;
			}
		}
		nFlag = nFlag<<1;
	}

	return result;
}

/**
 *
 */

//将两个字段的内容合并成一个字段
static int CombineColValue(struct XmlColumnS *ColumnElement, std::pair<int,int> pairCol, 
						   char sep, struct XmlSegmentS *Segment)
{
	if (ColumnElement->ID == pairCol.first)
	{
		struct XmlSegmentS *pSegTel = Segment;
		struct XmlSegmentS *pSegPri = NULL;
		while (pSegTel->Next != NULL)
		{
			pSegPri = pSegTel;
			pSegTel = pSegTel->Next;
		}

		if (pSegTel->ID == 5)
		{
			struct XmlColumnS *pTel = pSegTel->Column;
			struct XmlColumnS *pPri = NULL;
			while (pTel->Next != NULL && pTel->ID != pairCol.second)
			{
				pPri = pTel;
				pTel = pTel->Next;
			}

			if (pTel->ID == pairCol.second)
			{
				if (strlen(pTel->Value) > 0)
				{
					if (sep != 0)
						sprintf_s(ColumnElement->Value, 100, "%s%c%s", ColumnElement->Value, sep,pTel->Value);
					else
						sprintf_s(ColumnElement->Value, 100, "%s%s", ColumnElement->Value,pTel->Value);
				}

				if (bTelRW)
				{
					pPri->Next = pTel->Next;
					free(pTel);
				}

			}

		}
	}
	return 0;
}


static int iConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length)
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

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	for(SegmentElement = listHead; SegmentElement; SegmentElement = SegmentElement->Next)
	{
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", SegmentElement->ID);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);

		int nNameSeg = SegmentElement->ID;

		for(ColumnElement = SegmentElement->Column; ColumnElement; ColumnElement = ColumnElement->Next)
		{
			memset(buf, 0, 10);
			sprintf_s(buf, 10, "%d", ColumnElement->ID);

			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", buf);

			if (nNameSeg == 2)
			{
				std::string szName;
				struct XmlSegmentS *pSegPri, *pCur = NULL;

				switch (ColumnElement->ID)
				{
				case 22:
					CombineColValue(ColumnElement, std::make_pair(22,75), '/', SegmentElement);
					break;
				case 25:
					CombineColValue(ColumnElement, std::make_pair(25,76), '/', SegmentElement);
					pCur = SegmentElement;
					while (pCur->Next != NULL)
					{
						pSegPri = pCur;
						pCur = pCur->Next;
					}

					if (0 == IsAllTheSameFlag((unsigned char*)&pCur->Column->Value, 18, 0))
					{
						pSegPri->Next = pCur->Next;
						free(pCur);
					}
					break;
				case 24:
					CombineColValue(ColumnElement, std::make_pair(24,77),0, SegmentElement);
					if (Is_IntName(ColumnElement->Value))
					{
						szName = CConvertUtil::uf_gbk_int_covers(ColumnElement->Value,"togbk");
						memcpy(ColumnElement->Value, szName.c_str(), szName.size());
						ColumnElement->Value[szName.size()] = 0;
					}
					break;
				case 9:
					CombineColValue(ColumnElement, std::make_pair(9,78),0, SegmentElement);
					if (Is_IntName(ColumnElement->Value))
					{
						szName = CConvertUtil::uf_gbk_int_covers(ColumnElement->Value,"togbk");
						memcpy(ColumnElement->Value, szName.c_str(), szName.size());
						ColumnElement->Value[szName.size()] = 0;
					}
					break;
				case 10://add by yanggx5-28 为解决身份证末尾为字母的错误
					pBuf = ColumnElement->Value;
					while (*(pBuf) != 0)
						pBuf++;
					pBuf--;
					if (*pBuf > '9' || *pBuf < '0')
						*pBuf = 'X';
					break;
				default:
					break;

				}

			}

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


static struct XmlSegmentS *FindSegmentByID(struct XmlSegmentS *listHead, int ID)
{
	struct XmlSegmentS *result = NULL;

	result = listHead;
	while(result)
	{
		if(result->ID == ID)
			break;

		result = result->Next;
	}
	
	return result;
}


/**
 *
 */
static struct XmlColumnS *FindColumnByID(struct XmlColumnS *listHead, int ID)
{
	struct XmlColumnS *result = NULL;

	result = listHead;
	while(result)
	{
		if(result->ID == ID)
			break;

		result = result->Next;
	}

	return result;
}


static int InsertColumnBySplite(struct XmlSegmentS *SegmentElement, 
								std::map<int,std::string> &mapSplite, 
								struct XmlSegmentS *pSeg, 
								std::pair<int,int> pairCol)
{
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlColumnS  *pColNext = pSeg->Column;
	struct XmlColumnS *ColumnElement, *TempColumnElement = NULL;
	std::map<int, std::string>::iterator iter = mapSplite.find(pairCol.first);
	if (iter != mapSplite.end())
	{
		ColumnElement = FindColumnByID(SegmentElement->Column, pairCol.second);
		if (NULL != ColumnElement)
		{
			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memcpy(TempColumnElement, ColumnElement, sizeof(struct XmlColumnS));
			TempColumnElement->Next = NULL;
			strcpy_s(TempColumnElement->Value, 100, iter->second.c_str());
			if (pSeg->Column == NULL)
			{
				pSeg->Column = TempColumnElement;
				pColNext = pSeg->Column;
			}
			else
			{
				if (pColNext != NULL)
				{
					while (pColNext->Next != NULL)
					{
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
static struct XmlSegmentS* ConvertXmltoList(char *xml)
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
	while(Segment)
	{
		ID = atoi(Segment->Attribute("ID"));
		SegmentElement = FindSegmentByID(XmlListHead, ID);
		if (NULL == SegmentElement)
			break;

		TempSegmentElement = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(TempSegmentElement, SegmentElement, sizeof(struct XmlSegmentS));
		TempSegmentElement->Column = NULL;
		TempSegmentElement->Next = NULL;

		// 加入新的元素
		if(result)	// 以后加入
		{
			CurrSegmentElement->Next = TempSegmentElement;
			CurrSegmentElement = TempSegmentElement;
		}
		else		//第一次加入
		{
			CurrSegmentElement = TempSegmentElement;
			result = CurrSegmentElement;
		}
		
		Colum = Segment->FirstChildElement();
		while(Colum)
		{
			int nColumnID = atoi(Colum->Attribute("ID"));
			ColumnElement = FindColumnByID(SegmentElement->Column, nColumnID);

			if (NULL == ColumnElement)
				break;

			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memcpy(TempColumnElement, ColumnElement, sizeof(struct XmlColumnS));
			TempColumnElement->Next = NULL;

			//add 1020
			char strCheckValue[100];
			memset(strCheckValue, 0, sizeof(strCheckValue));
			CheckSpace(Colum->Attribute("VALUE"), 
				strlen(Colum->Attribute("VALUE")), strCheckValue);

			int nBit = TempColumnElement->ColumnBit/4;
			if(strlen(Colum->Attribute("VALUE")) == 0 || 
				std::string(Colum->Attribute("VALUE")) == string(" ")) 
			{
				memset(strCheckValue, 'f', TempColumnElement->ColumnBit%4 ? nBit+1:nBit);
			}
			else
			{
				if (ID == 2)
				{
					if (nColumnID == 9 || nColumnID == 24)
					{
						if (Is_GbkName(strCheckValue))
						{
							std::string szName = CConvertUtil::uf_gbk_int_covers(strCheckValue, "toint");
							strcpy(strCheckValue, szName.c_str());
						}

						if (nColumnID == 24 && strlen(strCheckValue) > 32)
						{
							mapSplite[nColumnID] = std::string(strCheckValue).substr(32, strlen(strCheckValue));
							strCheckValue[32] = 0;
						}
						else if (nColumnID == 9 && strlen(strCheckValue) > 64)
						{
							mapSplite[nColumnID] = std::string(strCheckValue).substr(64, strlen(strCheckValue));
							strCheckValue[64] = 0;
						}
					}

					//addby yanggx 1124
					if (nColumnID == 22)
					{
						std::vector<std::string> vecPhone;
						vecPhone = split(std::string(strCheckValue), "/");
						strcpy_s(strCheckValue, sizeof(strCheckValue), vecPhone[0].c_str());
						if (vecPhone.size() > 1)
						{
							mapSplite[22] = vecPhone[1];
						}
						else
						{
							mapSplite[22] = "fffffffffff";
						}

					}

					//addby yanggx 1124
					if (nColumnID == 25)
					{
						std::vector<std::string> vecPhone;
						vecPhone = split(std::string(strCheckValue), "/");
						strcpy_s(strCheckValue, sizeof(strCheckValue), vecPhone[0].c_str());
						if (vecPhone.size() > 1)
						{
							mapSplite[25] = vecPhone[1];
						}
						else
						{
							mapSplite[25] = "fffffffffff";
						}

					}

				}

			}

			// 赋值
			strcpy_s(TempColumnElement->Value, 100,  strCheckValue);

			// 加入链表
			if(CurrSegmentElement->Column) // 已经加入过了
			{
				CurrColumnElement->Next = TempColumnElement;
				CurrColumnElement = TempColumnElement;

			}
			else // 第一次加入
			{
				CurrColumnElement = TempColumnElement;
				CurrSegmentElement->Column = CurrColumnElement;
			}

			// 向后迭代
			Colum = Colum->NextSiblingElement();
		}

		// 向后迭代
		Segment = Segment->NextSiblingElement();
	}

	SegmentElement = FindSegmentByID(XmlListHead, 5);
	if (NULL != SegmentElement)
	{
		struct XmlSegmentS *pTel = result;
		while (pTel->Next != NULL)
		{
			pTel = pTel->Next;
		}

		struct XmlSegmentS *pSeg = pTel;
		if (pTel->ID != 5)
		{
			pSeg = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
			memcpy(pSeg, SegmentElement, sizeof(struct XmlSegmentS));
			pSeg->ID = 5;
			pSeg->Column = NULL;
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

/**
 *
 */
static int InitionGList(char *xml)
{
	struct XmlSegmentS	*CurrSegmentElement = NULL;
	struct XmlSegmentS	*TempSegmentElement = NULL;
	struct XmlColumnS	*CurrColumnElement = NULL;
	struct XmlColumnS	*TempColumnElement = NULL;

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
		TempSegmentElement = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memset(TempSegmentElement, 0, sizeof(struct XmlSegmentS));
	
		// 插入链表
		if(XmlListHead) 
		{
			CurrSegmentElement->Next = TempSegmentElement;
			CurrSegmentElement = TempSegmentElement;
		}
		else 
		{
			CurrSegmentElement = TempSegmentElement;
			XmlListHead = CurrSegmentElement;
		}

		// 对元素进行赋值
		CurrSegmentElement->ID =  atoi(Segment->Attribute("ID"));

		// 插入链表中元素
		Colum = Segment->FirstChildElement();
		while(Colum) 
		{
			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memset(TempColumnElement, 0, sizeof(struct XmlColumnS));

			// 插入元素
			if(CurrSegmentElement->Column)
			{
				CurrColumnElement->Next = TempColumnElement;
				CurrColumnElement = TempColumnElement;
			}
			else
			{
				CurrColumnElement = TempColumnElement;
				CurrSegmentElement->Column = CurrColumnElement;
			}
				
			// 对元素进行赋值
			CurrColumnElement->ID = atoi(Colum->Attribute("ID"));
			strcpy_s(CurrColumnElement->Source ,50 ,Colum->Attribute("SOURCE"));
			strcpy_s(CurrColumnElement->Target , 50, Colum->Attribute("TARGET"));
			strcpy_s(CurrColumnElement->Value , 100, Colum->Attribute("DEFAULT"));
			CurrColumnElement->Offset = atoi(Colum->Attribute("OFFSET"));
			CurrColumnElement->ColumnBit = atoi(Colum->Attribute("COLUMNBIT"));
			CurrColumnElement->Mask = atoi(Colum->Attribute("MASK"));
			

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
static void DestroyList(struct XmlSegmentS *listHead)
{
	struct XmlSegmentS	*CurrSegmentElement = NULL;
	struct XmlSegmentS	*TempSegmentElement = NULL;
	struct XmlColumnS	*CurrColumnElement	= NULL;
	struct XmlColumnS	*TempColumnElement	= NULL;

	CurrSegmentElement = listHead;
	while(CurrSegmentElement)
	{
		CurrColumnElement = CurrSegmentElement->Column;
		while(CurrColumnElement)
		{
			TempColumnElement = CurrColumnElement;
			CurrColumnElement = CurrColumnElement->Next;

			free(TempColumnElement);
		}
		
		TempSegmentElement = CurrSegmentElement;
		CurrSegmentElement = CurrSegmentElement->Next;

		free(TempSegmentElement);
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
	if (ERROR_SUCCESS != RegOpenKeyEx(RootKey, (LPCSTR)(MainKey), (DWORD)strlen(MainKey), KEY_READ | KEY_WRITE, &hKey))
	{
		if (ERROR_SUCCESS != RegCreateKeyEx(RootKey, MainKey, 0, MainKey, 0,
			KEY_READ | KEY_WRITE, NULL, &hKey, &dwDesc))
		{
			RegCloseKey(hKey);
			return;
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "Config", NULL, dwType, (PBYTE)CONFIG, (DWORD)strlen(CONFIG)))
		{
			RegCloseKey(hKey);
			return;
		}
	}

	dwLen = 256;
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, "Config", NULL, &dwType, (PBYTE)szValue,&dwLen))
	{
		RegCloseKey(hKey);
		return;
	}
	szValue[dwLen] = 0;
	RegCloseKey(hKey);
	strcpy_s(name, NAME_MAX_LEN, szValue);
}

/**
* 得到KEY值
*/
static int iGetKeyValue(const unsigned char *seed,  unsigned char *key)
{
	unsigned char keyA[6] = {0x43,0x97,0x04,0x47,0x20,0x47};
	unsigned char control[4] = {0x08,0x77,0x8F,0x69};
	unsigned char keyB[6];

	// 获得KEYB值
	memset(keyB, 0, 6);
	iGetKeyBySeed(seed, keyB);

	// 写入到返回值
	memcpy(key, keyA, 6);
	memcpy(key+6, control, 4);
	memcpy(key+10, keyB, 6);

	return 0;
}

static int  _iQueryInfo(const char *name, char *xml, int &nLen, QueryColum &stQuery)
{
	struct RWRequestS	*RequestList = NULL;
	RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
	memset(RequestList, 0, sizeof(struct RWRequestS));
	RequestList->offset = 0;
	RequestList->length = 0;
	unsigned char buf[160];
	XmlSegmentS *pSeg = XmlListHead;
	int nRet = -1;
	while(pSeg)
	{
		int nSeg = pSeg->ID;
		XmlColumnS *pColum = pSeg->Column;
		while(pColum)
		{
			if (strcmp(name, pColum->Source) == 0)
			{
				RequestList->offset = pColum->Offset;; 
				RequestList->length = pColum->ColumnBit;
				RequestList->value = buf;
				memset(buf, 0, 160);

				if (RequestList->length > 0)
				{
					nRet = iReadCard(RequestList);
					if (nRet != 0)
					{
						return nRet;
					}
					if (std::string(pColum->Source) == std::string("NAME") || 
						std::string(pColum->Source) == std::string("LINKMANNAME"))
					{
						std::string szName;
						char strName[160];
						memset(strName, 0, sizeof(strName));
						int nLen = 0;
						memcpy(strName, buf, strlen((char*)buf) +1);
						if (Is_IntName(strName))
						{
							szName = CConvertUtil::uf_gbk_int_covers(strName, "togbk");
						}
						else
						{
							szName = strName;
						}
						memcpy(buf, szName.c_str(), szName.size());
						buf[szName.size()] = 0;
					}
					memcpy(xml, buf, strlen((char*)buf));
					xml[strlen((char*)buf)] = 0;
					stQuery.nSegID = nSeg;
					stQuery.nID = pColum->ID;
					stQuery.szSource = pColum->Source;
					stQuery.szValue = xml;
					nLen = (int)strlen(xml);
					free(RequestList);
					return 0;
				}
			}
			pColum = pColum->Next;
		}
		pSeg = pSeg->Next;
	}
	free(RequestList);
	return nRet;
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
	for (size_t i=0; i<v.size(); ++i)
	{
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
			Cloumn->SetAttribute("Source", stQuery.szSource.c_str());
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

	xml.OutOfElem();
	xml.OutOfElem();
	return 0;
}



static struct CreateKeyInfoS* CreateCardKeyInfo(unsigned char *seed)
{
	char OldKey[20];
	struct CreateKeyInfoS *res = NULL;

	res = (struct CreateKeyInfoS *)malloc(sizeof(struct CreateKeyInfoS));
	memset(res, 0, sizeof(struct CreateKeyInfoS));
	memset(OldKey, 0, 20);

    // 获得原始密码
	//char OldKeyXML[1024];
	int nLen = 0;
	//res->offset = 792;
	//res->length = 72;
	//iQueryInfo("MEDICARECERTIFICATENO", OldKeyXML);
	//_iGetKeySeed(OldKeyXML, nLen);

	iGetKeyBySeed((unsigned char *)OldKey, res->token);
	LogPrinter("%s\n", OldKey);

	int length = 18;
	Str2Bcd((char*)seed, res->ID, &length);
	iGetKeyValue(seed, res->key);
	return res;
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

	if (g_bPreLoad == TRUE){
		if (authUCard(szSystem)) {
			return CardUAuthFailed;
		}
		return 0;
	}

	// 在资源文件里边提取XML文件并且初始化他
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CardLib.dll");
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_XML2),"XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes)
	{	
		LogPrinter("加载xml文件错误\n");
		return CardInitErr;
	}

	// 如果已经分配了链表
	if(XmlListHead)
		return CardXmlErr;

	// 初始化全局列表
	InitionGList(pvRes);

	if (authUCard(szSystem)) {
		return CardUAuthFailed;
	}

	// 对设备进行初始化
	g_bPreLoad = (initCoreDevice(szSystem)==0);
	g_bCardOpen = g_bPreLoad;
	return g_bPreLoad==TRUE ? CardProcSuccess:CardInitErr;
}

int __stdcall iCardDeinit()
{
	DestroyList(XmlListHead);
	XmlListHead = NULL;
	g_bPreLoad = FALSE;
	g_bCardOpen = FALSE;
	return closeCoreDevice();
}

int __stdcall iCardOpen()
{
   ASSERT_INIT(g_bPreLoad);
   int ret =  opendev();
   if (!ret)
	   g_bCardOpen = TRUE;
   return ret;

}

int __stdcall iCardClose()
{
	ASSERT_OPEN(g_bCardOpen);
	int ret =  closedev();
	if (!ret)
		g_bCardOpen = FALSE;
	return ret;
}


int __stdcall iScanCard()
{
	ASSERT_OPEN(g_bCardOpen);
	return iCoreFindCard();
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

	if (iScanCard() != 0)
		return CardScanErr;

	char strCardNo[20];
	memset(strCardNo, 0, sizeof(strCardNo));
	int nLen = 0;
	QueryColum stQuery;
	int nRet = _iQueryInfo("CARDNO", strCardNo, nLen, stQuery);
	if (nRet != 0)
	{
		return 0;
	}

	if (strCardNo[0] == '0')
	{
		for (int i=0; i<nLen; ++i)
		{
			if (strCardNo[i] != '0')
			{
				return CardIsNotEmpty;
			}
		}
	}
	else
	{
		for (int i=0; i<nLen; ++i)
		{
			if (strCardNo[i] != 'f' || strCardNo[i] != 'F')
			{
				return CardIsNotEmpty;
			}
		}
	}



	return 0;
}

/**
 *
 */

int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int res;

	if (iScanCard() != 0)
		return CardScanErr;

	// 根据Flag产生List
	if (!(flag & 0x10) && (flag >> 1 & 0x1))
	{
		bTelRW = true;
		flag = flag | 0x10;
	}
	list = GetXmlSegmentByFlag(flag);
	if (list == NULL)
	{
		return CardXmlErr;
	}

	// 获取读写链表
	RequestList = CreateRWRequest(list, 0);

	// 设备的真实读取
	res = iReadCard(RequestList);

	// 销毁读写请求链表
	DestroyRWRequest(RequestList, 0);

	// 通过链表产生XML字符串
	iConvertXmlByList(list, xml, &length);
	//
	DestroyList(list);

	if (res != 0)
	{
		return CardReadErr;
	}
	return res;
}

DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name)
{
	ASSERT_OPEN(g_bCardOpen);
	if (iScanCard() != 0)
		return CardScanErr;
	int nret = -1;
	if (flag > 0)
	{
		nret = iReadInfo(flag, xml);
	}

	if (flag<=0 && strcmp(name, "") != 0)
	{
		nret = iQueryInfo(name, xml);
	}
	return nret;
}


/**
 *
 */
int __stdcall iQueryInfo(char *name, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	if (iScanCard() != 0)
		return CardScanErr;

	std::vector<std::string> vecQuery;
	vecQuery = split(name, "|");
	struct RWRequestS	*RequestList = NULL;
	char  readbuf[100];
	int nLen = 0;
	RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
	std::vector<QueryColum> vecResult;
	for (size_t i=0; i<vecQuery.size(); ++i)
	{
		std::string &strColum = vecQuery[i];

		QueryColum stQuery;
		if (0 == _iQueryInfo(strColum.c_str(), (char*)readbuf,nLen, stQuery))
		{
			vecResult.push_back(stQuery);
		}
	}
	if (vecQuery.size() == 0)
	{
		return CardReadErr;
	}
	iCreateXmlByVector(vecResult, xml, &nLen);
	return 0;
}

/**
 *
 */
int __stdcall iWriteInfo(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);

	std::string strxml = std::string(xml);
	if (CheckCardXMLValid(strxml) < 0)
	{
		LogPrinter("CardXML:Check Error\n");
		return CardXmlErr;
	}
	struct XmlSegmentS *XmlList;
	struct RWRequestS	*RequestList = NULL;
	unsigned int res = 0;

	if (iScanCard() != 0)
		return CardScanErr;

	// 产生List
	XmlList = ConvertXmltoList((char*)(strxml.c_str()));

	if (XmlList == NULL)
	{
		// 销毁XML链表
		DestroyList(XmlList);
		return CardXmlErr;
	}

	// 产生读写链表
	RequestList = CreateRWRequest(XmlList, 1);

	// 对设备进行真实的写
	res = iWriteCard(RequestList);

	// 销毁读写链表
	DestroyRWRequest(RequestList, 0);

	// 销毁XML链表
	DestroyList(XmlList);
    
	return res;
}


int __stdcall iPatchCard(
						char *pszCardDataXml,
						char *pszCardCoverDataXml,
						char *pszPrinterType,
						char *pszCardCoverXml 
						)
{
	ASSERT_OPEN(g_bCardOpen);
	
	if (m_pBHPrinter == NULL)
	{
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0)
		{
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	m_pBHPrinter->FeedCard();

	Sleep(1000);
	int nTimeOut = 1000;
	while (iScanCard() != 0)
	{
		if (nTimeOut >= TIMEOUT)
		{
			m_pBHPrinter->DeInitPrinter();
			SAFE_DELETE(m_pBHPrinter);
			return FeedCardError;
		}
		Sleep(1000);
		nTimeOut += 1000;
	}

	int nret = iCreateCard(pszCardDataXml);
	if (nret != 0)
	{
		m_pBHPrinter->DeInitPrinter();
		SAFE_DELETE(m_pBHPrinter);
		return CardCreateErr;
	}
   
	m_pBHPrinter->BackToPrintHeader();
	nret = iPrintCard(pszPrinterType, pszCardCoverDataXml, pszCardCoverXml);
	if (nret != 0)
	{
		SAFE_DELETE(m_pBHPrinter);
		return CardCoverPrintErr;
	}
	SAFE_DELETE(m_pBHPrinter);

	return 0;
}

//获取打印机列表名字
int __stdcall iGetPrinterList(char *PrinterXML)
{
	if (PrinterXML == NULL)
	{
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
	if (m_pBHPrinter == NULL)
	{
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0)
		{
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	if (m_pBHPrinter->InitPrinter(pszCardCoverDataXml, pszCardCoverXml)!= 0)
	{
		SAFE_DELETE(m_pBHPrinter);
		return CardXmlErr;
	}

	if (!m_pBHPrinter->StartPrint())
	{
		m_pBHPrinter->DeInitPrinter();
		nPrint = -1;
	}
	
	SAFE_DELETE(m_pBHPrinter);
	return nPrint;
}


int __stdcall iCreateCard(char *pszCardDataXml)
{
	ASSERT_OPEN(g_bCardOpen);

	if (iScanCard() != 0)
		return CardScanErr;

	if (CheckCardXMLValid(std::string(pszCardDataXml)) < 0)
	{
		LogPrinter("CardXML:Check Error\n");
		return CardXmlErr;
	}

	XmlSegmentS *seg = ConvertXmltoList(pszCardDataXml);
	seg = FindSegmentByID(seg, 2);
	int nRet = 0;
	if (seg != NULL)
	{
		unsigned char KeyB[6];
		XmlColumnS *stColumn = FindColumnByID(seg->Column, 1);
		if (seg->Column->Value[0] == '0')
		{
			stColumn = FindColumnByID(seg->Column, 7);
		}
		iGetKeyBySeed((unsigned char *)stColumn->Value, KeyB);

		nRet = iWriteInfo(pszCardDataXml);
		LogPrinter("回写数据：%d\n", nRet);

		nRet = InitPwd(KeyB);
		LogPrinter( "重置密码结果%d\n", nRet);

	}
	else
	{
		return CardCreateErr;
	}
	return nRet != 0? CardCreateErr:0;
}

int __stdcall iFormatCard()
{
	ASSERT_OPEN(g_bCardOpen);
	if (iScanCard() != 0)
		return CardScanErr;

	return aFormatCard(0xff);
}


int __stdcall iCardCtlCard(int cmd, void *data)
{
	ASSERT_OPEN(g_bCardOpen);
	return iCtlCard(cmd, data);
}

DLL_EXPORT int __stdcall iCheckMsgForNH(char *pszCardCheckWSDL,char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen);
	std::string strCheckWSDL = pszCardCheckWSDL;
	std::string strServerURL = pszCardServerURL;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	if (iScanCard() != 0)
	{
		CreateResponXML(3, "寻卡失败", strResult);
		strcpy(pszXml, strResult);
		return CardScanErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	std::string strCardNO;
	int n = iQueryInfo("CARDNO", szQuery);
	if (n != 0)
	{
		CreateResponXML(3, "获取卡号失败", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfo(szQuery, strCardNO);
	
	int nCheckCode = CardProcSuccess;

	bool bSuccessed = true;
	NHOneCardServiceSoapBindingProxy  proxy;
	proxy.soap_endpoint = strServerURL.c_str();

	char *strCheckParams = new char[1024];
	memset(strCheckParams, 0, 1024);
	CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);
	ns1__nh_USCOREpipe pCheck;
	pCheck.ns1__parms = strCheckParams;

	ns1__nh_USCOREpipeResponse pReturn;
	pReturn.ns1__out = new char[1024];

	proxy.nh_USCOREpipe(&pCheck, &pReturn);
	if(proxy.error) {   
		bSuccessed = false;
		CreateResponXML(3, "与服务器连接失败",  strResult);
		strcpy(pszXml, strResult);
	} else {
		std::string strRetCode, strStatus;
		if (pReturn.ns1__out == NULL) {
			CreateResponXML(1, "返回值为空", strResult);
			strcpy(pszXml, strResult);
			bSuccessed = false;
			goto done;
		}
			
		std::string strXML = pReturn.ns1__out;
		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			bSuccessed = false;
			CreateResponXML(1, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		} else{
			nCheckCode = atoi(strStatus.c_str());
			strCheckDesc.clear();
			if (GetCardStatus(nCheckCode, strCheckDesc) == 0){
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			}
		}
	}

done:
	delete [] strCheckParams;
	delete [] pReturn.ns1__out;

	if (bSuccessed){
		char szRead[4096];
		memset(szRead, 0, sizeof(szRead));
		iReadInfo(2, szRead);
		strcpy(pszXml, szRead);
	}

	proxy.destroy();
	return bSuccessed ? 0 : CardCheckError;
}

DLL_EXPORT int __stdcall iReadConfigMsg(char *pszConfigXML,char *pszReadXML)
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


DLL_EXPORT int __stdcall iRegMsgForNH(char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen);
	std::string strServerURL = pszCardServerURL;
	std::string strXML;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	if (iScanCard() != 0)
	{
		CreateResponXML(3, "寻卡失败", strResult);
		strcpy(pszXml, strResult);
		return CardScanErr;
	}

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	std::string strCardNO;
	int n = iQueryInfo("CARDNO", szQuery);
	if (n != 0)
	{
		CreateResponXML(3, "获取卡号失败", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfo(szQuery, strCardNO);

	bool bSuccessed = true;
	NHOneCardServiceSoapBindingProxy  proxy(strServerURL.c_str(), SOAP_C_UTFSTRING);

	char* strRegParams = new char[1024];
	memset(strRegParams, 0, 1024);
	CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 
	ns1__nh_USCOREpipe pCheck;
	pCheck.ns1__parms = strRegParams;

	ns1__nh_USCOREpipeResponse pReturn;
	pReturn.ns1__out = new char[4096];

	proxy.nh_USCOREpipe(&pCheck, &pReturn);
	if(proxy.error)   
	{   
		bSuccessed = false;
		CreateResponXML(3, "与服务器连接失败", strResult); 
		strcpy(pszXml, strResult);
	} 
	else
	{
		std::string strRetCode, strStatus;
		if (pReturn.ns1__out == NULL) {
			CreateResponXML(1, "返回值为空", strResult);
			strcpy(pszXml, strResult);
			bSuccessed = false;
			goto done;
		}
		strXML = pReturn.ns1__out;

		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) 
		{
			bSuccessed = false;
			CreateResponXML(3, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		}
		else
		{
			if (strStatus.size() > 0 && CheckCardXMLValid(strStatus) == 0)
			{
				FormatWriteInfo(strStatus.c_str(), strResult);
				int nState = iWriteInfo(strResult);
				if (nState != 0)
				{
					bSuccessed = false;
					memset(strResult, 0, sizeof(strResult));
					CreateResponXML(2, "卡回写失败", strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
	}

done:
	delete [] strRegParams;
	delete [] pReturn.ns1__out;

	if (bSuccessed)
	{
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2, strResult);
		strcpy(pszXml, strResult);
	}

	proxy.destroy();
	return bSuccessed ? 0 : CardRegError;
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
	if (iScanCard() != 0)
	{
		CreateResponXML(3, "寻卡失败", strResult);
		strcpy(pszXml, strResult);
		return 3;
	}

	int n = iQueryInfo("MEDICARECERTIFICATENO", szQuery);
	if (n != 0)
	{
		CreateResponXML(3, "获取参合号失败", strResult);
		strcpy(pszXml, strResult);
		return 3;
	}
	std::string strMedicalID;
	GetQueryInfo(szQuery, strMedicalID);

	std::string strCardNO;
	n = iQueryInfo("CARDNO", szQuery);
	if (n != 0)
	{
		CreateResponXML(3, "获取卡号失败", strResult);
		strcpy(pszXml, strResult);
		return 3;
	}
	GetQueryInfo(szQuery, strCardNO);

	bool bSuccessed = true;
	NHOneCardServiceSoapBindingProxy  proxy(strServerURL.c_str(), SOAP_C_UTFSTRING);
	proxy.soap_endpoint = strServerURL.c_str();

	if (IsMedicalID(strMedicalID))
	{
		char *strCheckParams = new char[1024];
		memset(strCheckParams, 0, 1024);
		CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);

		ns1__nh_USCOREpipe pCheck;
		pCheck.ns1__parms = strCheckParams;

		ns1__nh_USCOREpipeResponse pReturn;
		pReturn.ns1__out = new char[1024];

		proxy.nh_USCOREpipe(&pCheck, &pReturn);
		if (proxy.error)
		{   
			bSuccessed = false;
			CreateResponXML(3, "与服务器连接失败", strResult);
			strcpy(pszXml, strResult);
		}
		else
		{
			std::string strRetCode, strStatus;
			if (pReturn.ns1__out == NULL) {
				CreateResponXML(1, "返回值为空", strResult);
				strcpy(pszXml, strResult);
				bSuccessed = false;
				goto done;
			}
			strXML = pReturn.ns1__out;
			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) 
			{
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			}
			else
			{
				int nCardStatus = atoi(strStatus.c_str());
				strCheckDesc.clear();
				if (GetCardStatus(nCardStatus, strCheckDesc) == 0)
				{
					bSuccessed = false;
					CreateResponXML(1, strCheckDesc.c_str(), strResult);
					strcpy(pszXml, strResult);
				}
			}
		}

	done:
		delete [] strCheckParams;
		delete [] pReturn.ns1__out;

		if (!bSuccessed)
		{
			proxy.destroy();
			return 1;
		}
	}

	if (bSuccessed)
	{
		char* strRegParams = new char[1024];
		memset(strRegParams, 0, 1024);
		CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

		ns1__nh_USCOREpipe pCheck;
		pCheck.ns1__parms = strRegParams;

		ns1__nh_USCOREpipeResponse pReturn;
		pReturn.ns1__out = new char[4096];

		proxy.nh_USCOREpipe(&pCheck, &pReturn);

		if (proxy.error)
		{   
			bSuccessed = false;
			CreateResponXML(3, "与服务器连接失败", strResult);
			strcpy(pszXml, strResult);
		} 
		else
		{
			std::string strRetCode, strStatus;
			if (pReturn.ns1__out == NULL) {
				CreateResponXML(1, "返回值为空", strResult);
				strcpy(pszXml, strResult);
				bSuccessed = false;
				goto done1;
			}
			strXML = pReturn.ns1__out;

			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) 
			{
				bSuccessed = false;
				CreateResponXML(3, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			}
			else
			{
				if (strStatus.size() > 0 && CheckCardXMLValid(strStatus) == 0)
				{
					FormatWriteInfo(strStatus.c_str(), strResult);
					int nState = iWriteInfo(strResult);
					if (nState != 0)
					{
						bSuccessed = false;
						memset(strResult, 0, sizeof(strResult));
						CreateResponXML(2, "卡回写失败", strResult);
						strcpy(pszXml, strResult);
					}
				}
			}
		}
	done1:
		delete [] strRegParams;
		delete [] pReturn.ns1__out;
	}

	if (bSuccessed)
	{
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2, strResult);
		strcpy(pszXml, strResult);
	} 
	proxy.destroy();
	return bSuccessed==true ? 0 : 2;
}


int __stdcall iEncryFile(char *filename)
{
	CDESEncry encry;
	bool bSuccess = encry.EncryFile(filename);
	return bSuccess ? 0 : EncryFileError;
}


int __stdcall iCreateLicense(char *filename, char *timeStr, int maxCounts)
{
	if (timeStr == NULL && maxCounts == 0) {
		return InitFullLicense(filename);
	}else if (maxCounts > 0){
		return InitCountLicense(filename, maxCounts);
	} else {
		return InitTimeLicense(filename, timeStr);
	}
	return 0;
}

int __stdcall iCheckLicense(char *filename,int type, int counts)
{
	int status = 0;
	if (type == 0) {
		status = CheckTimeLicense(filename);
		status = (status == 0 ? 0 : CardAuthExpired);
	} else if (type == 1){
		status = CheckFullLicense(filename);
		status = (status == 0 ? 0 : CardNoAuthority);
	} else if (type == 2) {
		status = CheckCounts(filename, counts);
		status = (status == 0 ? 0 : CardNoAuthority);
	}

	return status;
}
