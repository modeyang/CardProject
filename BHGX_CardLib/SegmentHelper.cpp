#include "SegmentHelper.h"
#include "public/Markup.h"

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

CSegmentHelper::CSegmentHelper(void)
:m_XmlListHead(NULL)
{

}

CSegmentHelper::CSegmentHelper(struct XmlProgramS *xmlHeader, struct CardOps *ops)
:m_XmlListHead(xmlHeader),
 m_CardOps(ops)
{

}

CSegmentHelper::CSegmentHelper(struct CardOps *cardOps)
:m_CardOps(cardOps)
{
	m_XmlListHead = m_CardOps->programXmlList;
}


CSegmentHelper::~CSegmentHelper()
{

}

void CSegmentHelper::setXmlProgramS(struct XmlProgramS *xmlHeader)
{
	m_XmlListHead = xmlHeader;
}

void CSegmentHelper::setCardOps(struct CardOps * ops)
{
	m_CardOps = ops;
}

// mode 为1时表示分配内存
struct XmlColumnS * 
CSegmentHelper::CloneColmn(struct XmlColumnS *ColmnElement, int mode)
{
	struct XmlColumnS *result = NULL;

	if(ColmnElement == NULL)
		return NULL;

	result = (struct XmlColumnS *)malloc(sizeof(struct XmlColumnS));
	if(result == NULL) {
		return NULL;
	}

	memcpy(result, ColmnElement, sizeof(struct XmlColumnS));
	result->parent = ColmnElement->parent;
	result->Value = NULL;
	result->Next = NULL;

	if (mode) {
		m_CardOps->iCallocForColmn(result);
	}


	return result;
}

/**
* CloneSegment - 克隆XmlSegmentS元素数据结构
* @SegmentElement 被克隆的元素
* @mode 1->开辟value内存
*/
struct XmlSegmentS * 
CSegmentHelper::CloneSegment(struct XmlSegmentS *SegmentElement, int mode)
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


struct XmlSegmentS * CSegmentHelper::FindSegmentByID(struct XmlSegmentS *listHead, int ID)
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


int CSegmentHelper::FindSegIDByColumName(struct XmlSegmentS *list ,const char *name) 
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

struct XmlSegmentS * CSegmentHelper::FindSegmentByColumName(struct XmlSegmentS *list ,const char *name) 
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

struct XmlSegmentS * CSegmentHelper::getSegmentByColumName(struct XmlSegmentS *list ,const char *name) 
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

int CSegmentHelper::FindColumIDByColumName(struct XmlSegmentS *list, const char *name) 
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

struct XmlColumnS* CSegmentHelper::FindColumByColumName(struct XmlSegmentS *list, const char *name) 
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

struct XmlColumnS* CSegmentHelper::getColumByColumName(struct XmlSegmentS *list, const char *name) 
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
struct XmlColumnS * CSegmentHelper::FindColumnByID(struct XmlColumnS *listHead, int ID)
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
struct XmlSegmentS * CSegmentHelper::GetXmlSegmentByFlag(int flag)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;
	struct XmlSegmentS *XmlListHead = m_XmlListHead->SegHeader;

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