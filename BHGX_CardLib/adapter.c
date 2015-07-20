#include <stdio.h>
#include "adapter.h"
#include "device.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"
#include "Card.h"

#pragma warning (disable : 4020)
#pragma warning (disable : 4996)

#define DBGADAP(format, ...)		\
	LogWithTime(0, format)			\	

struct CardDevice *Instance = NULL;

static CardType g_PreCardType = -1;


/**
 * 函数： apt_InitCoreDevice
 * 参数：
 *
 * 返回值：
 * 成功： 0
 * 失败：非零
 */
int __stdcall apt_InitCoreDevice(const char *System)
{
	if(!Instance) 
		Instance = getCardDevice(System);

	return (Instance == NULL?-1:0);
}

/**
 * 函数：apt_CloseCoreDevice
 * 参数：
 *
 * 返回值：
 * 成功： 0
 * 失败： 非零
 */
int __stdcall apt_CloseCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret)  {
		Instance = NULL;
		g_PreCardType = -1;
	}
	return ret;
}

int __stdcall apt_OpenDev(void)
{
	int ret = 0;
	if (Instance) {
		Instance->iProbe();
		ret = Instance->iOpen();
	}
	return (ret == 0 ? 0 : CardNotOpen);
}
int __stdcall apt_CloseDev(void)
{
	int ret = 0;
	if (Instance) {
		ret = Instance->iClose();
	}
	return ret;
}

/**
 * 函数：iFindCard
 * 参数：
 *
 * 返回值：
 * 成功： 非零
 * 失败：0
 */
int __stdcall apt_ScanCard(char *card_info)
{
	unsigned char cardType = 0, psamType=0;
	int status = 0;
	unsigned short para = 30;

	char resp[260];
	char attr[260];
	memset(resp, 0, sizeof(resp));
	memset(attr, 0, sizeof(attr));


	// 探测卡，如果没有卡，自动退出
	if (!Instance) {
		return CardInitErr;
	}

	//status = Instance->iIOCtl( CMD_BEEP, &para , 2);
	//status = Instance->iIOCtl( CMD_LED, &para , 2);
	status = Instance->ICCSet( CARDSEAT_RF, &cardType , attr);

	if (cardType == eCPUCard) {
		status = Instance->ICCSet(CARDSEAT_PSAM1, &psamType, resp);
	}
	
	if (status) {
		return CardScanErr; 
	}

	sprintf(card_info, "%d|%s|%s", (int)cardType, resp, attr);
	memcpy(card_info, card_info, strlen(card_info) + 1);
	card_info[strlen(card_info)] = 0;
	//*card_type = cardType;

	//加载xml文件
	apt_InitGList(cardType);
	return 0;
}

/**
 *
 */
int __stdcall apt_IOCtl(int cmd, void *data)
{
	// 初始化结果为失败
	unsigned char result = 0;

	// 如果设置了这些函数
	if(Instance && Instance->iIOCtl) {
		result = (unsigned char)Instance->iIOCtl(cmd, data, 0);
	}

	return result;
}

/**
*
*/
struct RWRequestS*  apt_CreateRWRequest(struct XmlSegmentS *listHead, int mode, CardType type)
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
			if (TempRequest == NULL) {
				return NULL;
			}

			memset(TempRequest, 0, sizeof(struct RWRequestS));

			// 对节点进行赋值
			TempRequest->mode = mode;
			TempRequest->value = ColumnElement->Value;
			TempRequest->pri = (void *) ColumnElement;
			TempRequest->offset = ColumnElement->Offset;

			//为不同的卡创建读写链表
			if (type == eCPUCard) {
				TempRequest->length = ColumnElement->CheckInfo.CpuInfo.ColumnByte;
				TempRequest->nID = SegmentElement->ID;
				TempRequest->nColumID = ColumnElement->ID;
				TempRequest->datatype = SegmentElement->datatype;
				TempRequest->itemtype = ColumnElement->CheckInfo.CpuInfo.itemtype;
			} else {
				TempRequest->length = ColumnElement->CheckInfo.M1Info.ColumnBit;
			}

			// 加入链表
			if(result){
				CurrRequest->Next = TempRequest;
				CurrRequest = TempRequest;
			} else {
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


/**
*
*/
void  apt_DestroyRWRequest(struct RWRequestS *list, int flag)
{
	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;

	CurrRequest = list;
	while(CurrRequest)
	{
		TempRequest = CurrRequest;
		CurrRequest = CurrRequest->Next;

		if(flag){
			free(TempRequest->value);
		}
		free(TempRequest);
	}

	return;
}


