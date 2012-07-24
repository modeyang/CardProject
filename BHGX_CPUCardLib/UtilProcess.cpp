#include "UtilProcess.h"
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include "public/Markup.h"
#include "./tinyxml/headers/tinyxml.h"
#include "public/algorithm.h"
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

using namespace std;
#pragma warning (disable : 4996)

extern struct XmlProgramS *g_XmlListHead;
extern "C" 
{
struct RecFolder g_recIndex[30];
}

#define  INSERT_SEGS(list, seg, id, name)											\
	memcpy(g_recIndex[id-1].section, g_recIndex[seg->ID-1].section, 10);            \
    memcpy(g_recIndex[id-1].subSection, g_recIndex[seg->ID-1].subSection, 10);      \
	memcpy(g_recIndex[id-1].fileName, name, strlen(name));							\
	seg->ID = id;																	\
	list->SegTailer->Next = seg;													\
	list->SegTailer = seg;															\

int __stdcall CreateCPUData(char *configXML) {

	if (g_XmlListHead == NULL){
		g_XmlListHead = (struct XmlProgramS*)malloc(sizeof(struct XmlProgramS));
		g_XmlListHead->SegHeader = g_XmlListHead->SegTailer = NULL;
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
	xml.SetDoc(configXML);
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
			strcpy(pColumnS->Source, xml.GetAttrib("SOURCE").c_str());
			if (nSegID < 11) 
				pColumnS->ID = atoi(xml.GetAttrib("ID").c_str());
			else
				pColumnS->ID = nIDCounts;

			pColumnS->ColumnByte = atoi(xml.GetAttrib("OCCUPYBYTE").c_str());
			pColumnS->Offset = nOffset;
			pColumnS->itemtype = (eItemType)atoi(xml.GetAttrib("TYPE").c_str());
			nOffset += pColumnS->ColumnByte;
			pColumnS->value = (BYTE*)malloc(sizeof(BYTE)*(pColumnS->ColumnByte+1));
			memset(pColumnS->value, 0, pColumnS->ColumnByte+1);

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
							sprintf(SourceName, "%s%d", pCell->Source.c_str(), i);
							strcpy(pColumnS->Source, SourceName);
							pColumnS->ID = nIDCounts;
							pColumnS->ColumnByte = pCell->nByte;
							pColumnS->Offset = nOffset;
							pColumnS->itemtype = (eItemType)pCell->itemtype;
							nOffset += pColumnS->ColumnByte;
							pColumnS->value = (BYTE*)malloc(sizeof(BYTE)*(pColumnS->ColumnByte+1));
							memset(pColumnS->value, 0, pColumnS->ColumnByte+1);

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

		if (g_XmlListHead->SegHeader == NULL) {
			g_XmlListHead->SegHeader = pSeg;
			g_XmlListHead->SegTailer = pSeg;
			memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
		} else {

			if (cFileType == std::string("EE01--03")) {
				INSERT_SEGS(g_XmlListHead, pSeg, nSegID, "EE01");
				pTmp = CloneSegment(pSeg);
				nSegID++;
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "EE02")
				nSegID++;
				pTmp = CloneSegment(pSeg);
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "EE03")
			} else if (cFileType == std::string("ED01--05")) {

				INSERT_SEGS(g_XmlListHead, pSeg, nSegID, "ED01")
				pTmp = CloneSegment(pSeg);
				nSegID++;
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "ED02")
				nSegID++;
				pTmp = CloneSegment(pSeg);
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "ED03")
				nSegID++;
				pTmp = CloneSegment(pSeg);
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "ED04")
				nSegID++;
				pTmp = CloneSegment(pSeg);
				INSERT_SEGS(g_XmlListHead, pTmp, nSegID, "ED05")
			} else {

				memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
				g_XmlListHead->SegTailer->Next = pSeg;
				g_XmlListHead->SegTailer = pSeg;
				}
		}

	}
	xml.OutOfElem();
	xml.OutOfElem();

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

static  int CheckSpace(const char *szCheck, int nLen, char *strValue)
{
	int newlen = 0; 
	for (int i=0; i<nLen; ++i)
	{
		if (szCheck[i] != 0x20)
		{
			strValue[newlen] = szCheck[i];
			++newlen;
		}
	}
	strValue[newlen] = 0;
	return newlen;
}

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
	if (result->itemtype != eAnsType) {
		result->value = (BYTE*)malloc(2*result->ColumnByte+3);
		memset(result->value, 0, 2*result->ColumnByte+3);
	}else { 
		result->value = (BYTE*)malloc(result->ColumnByte+1);
		memset(result->value, 0, result->ColumnByte+1);
	}
	
	result->Next = NULL;

	return result;
}

/**
* CloneSegment - 克隆XmlSegmentS元素数据结构
* @SegmentElement 被克隆的元素
*/
struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement)
{
	struct XmlColumnS *OrigColumnElement = NULL;	// 元SegmentElegment对应元素
	struct XmlColumnS *CurrColumnElement = NULL;	// 当前Result对应的当前的元素
	struct XmlColumnS *TempColumnElement = NULL;	// Result对应的临时元素
	struct XmlSegmentS *result = NULL;

	if(SegmentElement != NULL)
	{
		result = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(result, SegmentElement, sizeof(struct XmlSegmentS));
		result->ColumnHeader = NULL;
		result->ColumnTailer = NULL;
		result->Next = NULL;

		// 复制下边的链表结构
		for(OrigColumnElement = SegmentElement->ColumnHeader; OrigColumnElement; OrigColumnElement = OrigColumnElement->Next)
		{
			TempColumnElement = CloneColmn(OrigColumnElement);

			// 将新生成的元素加入到链表中
			if(result->ColumnHeader)	// 已经有元素插入链表，这种情况比较常见，在前边
			{
				result->ColumnTailer->Next = TempColumnElement;
				result->ColumnTailer = TempColumnElement;

			}
			else				// 第一个元素插入链表
			{
				CurrColumnElement = TempColumnElement;
				result->ColumnTailer = CurrColumnElement;
				result->ColumnHeader = CurrColumnElement;
			}
		}
	}

	return result;
}

#define MainKey "Software\\北航冠新\\CardProcess"
#define CONFIG  "C:\\WINDOWS\\system32\\"
int ReadConfigFromReg(char *reg)
{
	HKEY RootKey;
	HKEY hKey;
	DWORD dwLen;
	char szValue[256];
	memset(szValue, 0, sizeof(szValue));

	RootKey = HKEY_CURRENT_USER;
	DWORD dwDesc;
	DWORD dwType = REG_SZ;
	if (ERROR_SUCCESS != RegOpenKeyEx(RootKey, (LPCSTR)(MainKey), 
		(DWORD)strlen(MainKey), KEY_READ | KEY_WRITE, &hKey))
	{
		if (ERROR_SUCCESS != RegCreateKeyEx(RootKey, MainKey, 0, MainKey, 0,
			KEY_READ | KEY_WRITE, NULL, &hKey, &dwDesc))
		{
			RegCloseKey(hKey);
			return -1;
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "Config", NULL, dwType, 
			(PBYTE)CONFIG, (DWORD)strlen(CONFIG)))
		{
			RegCloseKey(hKey);
			return -1;
		}
	}

	dwLen = 256;
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, "Config", NULL, &dwType, 
		(PBYTE)szValue,&dwLen))
	{
		RegCloseKey(hKey);
		return -1;
	}
	szValue[dwLen] = 0;
	RegCloseKey(hKey);
	memcpy(reg, szValue, dwLen+1);


	return 0;
}


struct XmlSegmentS *GetXmlSegmentByFlag(int flag)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;
	XmlSegmentS *XmlListHead = g_XmlListHead->SegHeader;

	for(SegmentElement=XmlListHead; SegmentElement; SegmentElement = SegmentElement->Next)
	{
		// 表明这个位置被设置
		int nReadFlag = flag & 0x1;

		if(nReadFlag > 0)
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
		flag = flag >> 1;
	}

	return result;
}

void DestroyList(struct XmlSegmentS *listHead, int mode)
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
			if (mode)
               free(TempColumnElement->value);

			free(TempColumnElement);
		}

		TempSegmentElement = CurrSegmentElement;
		CurrSegmentElement = CurrSegmentElement->Next;

		free(TempSegmentElement);
	}

	return;
}


/**
*
*/
/**
*
*/
struct RWRequestS* __stdcall CreateRequest(struct XmlSegmentS *listHead, int mode)
{
	struct XmlSegmentS	*SegmentElement = NULL;
	struct XmlColumnS	*ColumnElement = NULL;

	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;
	struct RWRequestS	*result = NULL;

	SegmentElement = listHead;
	while(SegmentElement)
	{
		ColumnElement = SegmentElement->ColumnHeader;
		while(ColumnElement)
		{
			// 产生新的节点
			TempRequest = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
			memset(TempRequest, 0, sizeof(struct RWRequestS));

			// 对节点进行赋值
			TempRequest->mode = mode;
			TempRequest->offset = ColumnElement->Offset;
			TempRequest->length = ColumnElement->ColumnByte;
			TempRequest->nID = SegmentElement->ID;
			TempRequest->nColumID = ColumnElement->ID;
			TempRequest->datatype = SegmentElement->datatype;
			TempRequest->itemtype = ColumnElement->itemtype;
			TempRequest->value = ColumnElement->value;
			TempRequest->pri = (void *) ColumnElement;

			// 加入链表
			if(result)
			{
				CurrRequest->Next = TempRequest;
				CurrRequest = TempRequest;
			}
			else 
			{
				CurrRequest = TempRequest;
				result = CurrRequest;
			}

			// 向后迭代
			ColumnElement = ColumnElement->Next;
		}

		// 向后迭代
		SegmentElement = SegmentElement->Next;
	}
	return result;
}


void __stdcall DestroyRequest(struct RWRequestS *list, int flag)
{
	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;

	CurrRequest = list;
	while(CurrRequest)
	{
		TempRequest = CurrRequest;
		CurrRequest = CurrRequest->Next;

		if(flag)
		{
			free(TempRequest->value);
		}
		free(TempRequest);
	}
}



int CheckCardXMLValid(char *pszCardXml)
{
	std::string strCardXML = pszCardXml;
	strCardXML = strCardXML.substr(0, strCardXML.find(">"));
	strCardXML = strlwr((char*)strCardXML.c_str());
	size_t pos = strCardXML.find("gb2312");

	strCardXML = pszCardXml;
	if (pos < 0)
	{
		strCardXML = CMarkup::UTF8ToGB2312(strCardXML.c_str());
		strCardXML.replace(0, strCardXML.find(">")+1, "<?xml version=\"1.0\" encoding=\"gb2312\" ?>");
	}
	return 0;
}


int iConvertXmlByList(struct XmlSegmentS *listHead, char *xml, int *length)
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

	for(SegmentElement = listHead; SegmentElement; SegmentElement = SegmentElement->Next)
	{
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", SegmentElement->ID);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);

		int nNameSeg = SegmentElement->ID;

		for(ColumnElement = SegmentElement->ColumnHeader; ColumnElement; ColumnElement = ColumnElement->Next)
		{
			memset(buf, 0, 10);
			sprintf_s(buf, 10, "%d", ColumnElement->ID);

			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", buf);

			Cloumn->SetAttribute("VALUE", (char*)ColumnElement->value);

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

/**
*
*/
struct XmlSegmentS* ConvertXmltoList(char *xml)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;

	struct XmlColumnS  *ColumnElement = NULL;
	struct XmlColumnS  *CurrColumnElement = NULL;
	struct XmlColumnS  *TempColumnElement = NULL;

	TiXmlDocument *XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	BYTE *HexString = NULL;
	BYTE *tmpString = NULL;
	int ElemLen = 0, padding = 0;
	BYTE tmpArray[200];

	struct XmlSegmentS *XmlListHead = g_XmlListHead->SegHeader;

	// 解析XML语句
	XmlDoc = new TiXmlDocument();
	XmlDoc->Parse(xml);
	RootElement = XmlDoc->RootElement();

	Segment = RootElement->FirstChildElement();
	while(Segment)
	{
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
        padding = 0;
		Colum = Segment->FirstChildElement();
		while(Colum)
		{
			int nColumnID = atoi(Colum->Attribute("ID"));
			ColumnElement = FindColumnByID(SegmentElement->ColumnHeader, nColumnID);

			if (NULL == ColumnElement)
				break;

			TempColumnElement = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
			memcpy(TempColumnElement, ColumnElement, sizeof(struct XmlColumnS));
			TempColumnElement->Next = NULL;

			//为了节省空间
			if (TempSegmentElement->datatype == eRecType) {
				padding = 2; //记录文件需要填充2个字节
			}
			std::string strColum = Colum->Attribute("VALUE");
			ElemLen = (int)strColum.length();

			//考虑每个字段字符转化当不为Ans类型时，需要转化
			TempColumnElement->value = (BYTE*)malloc(TempColumnElement->ColumnByte+padding);
			memset(TempColumnElement->value, 0, TempColumnElement->ColumnByte + padding);

			//进行数据转换ans cn b类型
			if (TempColumnElement->itemtype != eAnsType) {
				HexString = TempColumnElement->value;
				if (ElemLen % 2)  //当不为Ans时，不是2的倍数时，补充‘f’
					ElemLen++;

				if (ElemLen > sizeof(tmpArray)) {
					tmpString = (BYTE*)malloc(ElemLen + 1);
				} else {
					tmpString = tmpArray;
				}

				memset(tmpString, 0, ElemLen+1);
			    tmpString[ElemLen] = 0;
				tmpString[ElemLen-1] = 'f';
				memcpy(tmpString, strColum.c_str(), strColum.size());
				HexstrToBin(HexString+padding, tmpString, ElemLen);

				if (ElemLen > sizeof(tmpArray)) {
					free(tmpString);
				}
			} else {
				HexString = TempColumnElement->value;
				memcpy(HexString+padding, strColum.c_str(), ElemLen);
			}

			// 加入链表
			if(CurrSegmentElement->ColumnHeader)
			{
				CurrSegmentElement->ColumnTailer->Next = TempColumnElement;
				CurrSegmentElement->ColumnTailer = TempColumnElement;


			}
			else // 第一次加入
			{
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