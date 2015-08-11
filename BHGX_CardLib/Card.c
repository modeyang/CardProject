#include "Card.h"
#include <stdio.h>
#include <memory.h>

CardType g_CardType;

static CardOps *cardOpsArray[ARRAY_MAX] = {NULL};

struct CardType2SAMSeat gSAMCardType[SEAT_NUMS] = { {0, eM1Card}};

struct CardXmlListHeaders gXmlLists[ARRAY_MAX];
static int gCurPos = -1;


void  CardUnregisterOps(int type) 
{
	assert(type < ARRAY_MAX && type < -1);
	if (type == -1) {
		memset(cardOpsArray, 0, sizeof(ARRAY_MAX));
	} else {
		cardOpsArray[type] = NULL;
	}
}

void  CardRegisterOps(int type, CardOps *ops)
{
	assert(type < ARRAY_MAX && type > 0);
	cardOpsArray[type] = ops;
}

CardOps*  GetCardOps(int type)
{
	assert(type < ARRAY_MAX && type > 0);
	return cardOpsArray[type];
}

int IsAllTheSameFlag(const unsigned char *szBuf, int nLen, unsigned char cflag)
{
	int i=0;
	for (; i<nLen; ++i){
		if (szBuf[i] != cflag){
			return -1;
		}
	}
	return 0;
}

int	get_seg_counts(int sec)
{
	int g_16RecMap[BIN_START] = {0, 10, 5, 1, 6, 4, 9, 3, 4, 15, 1, 2, 2, 2, 2};
	int g_32RecMap[BIN_START] = {0, 10, 5, 1, 6, 4, 9, 3, 4, 15, 1, 3, 10, 3, 5};
	if (sec < 0 || sec >= BIN_START) {
		return -1;
	}
	if (g_CardType == eCPU16Card) {
		return g_16RecMap[sec];
	}
	return g_32RecMap[sec];
}

int	get_can_write_seg()
{
	return g_CardType == eCPU16Card ? 0 : 3;
}

int get_bin_start_seg()
{
	return g_CardType == eCPU16Card ? 12 : 11;
}

void set_card_type(CardType type)
{
	g_CardType = type;
}

CardType get_card_type()
{
	return g_CardType;
}

void set_card_sam(int sam_seat, CardType eType)
{
	struct CardType2SAMSeat card_sam_seat;
	card_sam_seat.card_type = eType;
	card_sam_seat.sam_seat_id = sam_seat;
	gSAMCardType[sam_seat] = card_sam_seat;
}

int get_samSeat()
{
	int i;
	CardType SAMType;
	if (g_CardType == eM1Card) {
		return 1;
	} else if (g_CardType == eCPU16Card) {
		SAMType = eCPUSAM16;
	} else if (g_CardType == eCPU32Card) {
		SAMType = eCPUSAM32;
	}

	for (i=0; i<sizeof(gSAMCardType); i++) {
		if (SAMType == gSAMCardType[i].card_type)
			return i;
	}
	return -1;
}

int get_read_flag()
{
	int flag = 2;
	if (g_CardType == eCPU16Card) {
		flag += 1 + (1 << 7) + (1 << 10);
	} else if (g_CardType == eCPU32Card) {
		flag += 1 + (1 << 7);
	}
	return flag;
}

void set_card_xmlList(CardType eType, struct XmlProgramS *xmlListHeader)
{
	struct CardXmlListHeaders header;
	if (get_card_xmlList(eType) != NULL) {
		return;
	}

	gCurPos ++;
	header.eType = eType;
	header.xmlListHeader = xmlListHeader;
	gXmlLists[gCurPos] = header;
}

struct XmlProgramS *get_card_xmlList(CardType eType)
{
	int index = 0;
	for (; index <= gCurPos; index++) {
		if (gXmlLists[index].eType == eType) {
			return gXmlLists[index].xmlListHeader;
		}
	}
	return NULL;
}

/**
*
*/
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

void clean_up()
{
	int index = 0;
	for (; index <= gCurPos; index++) {
		DestroyList(gXmlLists[index].xmlListHeader->SegHeader, 0);
		SAFE_DELETE_C(gXmlLists[index].xmlListHeader);
		gCurPos --;
	}
}