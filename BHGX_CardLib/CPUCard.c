#include <stdio.h>
#include <malloc.h>
#include "CPUCard.h"
#include "device.h"
#include "adapter.h"
#include "Card.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"


#define ISAPTSCANCARD {if (apt_ScanCard() != 0) return CardScanErr;}

#pragma warning (disable : 4996)

#define IFD_OK	0				//正常
#define IFD_ICC_Type_Error	1	//卡片类型不对
#define IFD_NO_ICC	2			//无卡
#define IFD_ICC_Clash	3		//多卡片冲突
#define ICC_NO_Response	4		//卡片无应答
#define IFD_Error	5			//接口设备故障
#define IFD_Bad_Command	6		//不支持该命令
#define IFD_Length_Error	7	//命令长度错误
#define IFD_Parameter_Error	8	//命令参数错误
#define IFD_CheckSum_Error	9	//信息校验和出错
#define IFD_RD_CONNECT_ERROR 10	//


/************************************************************************/
/* 密钥ID 定义                                                                     */
/************************************************************************/
#define KEY_RK_DDF1			0
#define KEY_RK_DF01			1
#define KEY_RK_DF02			2
#define KEY_RK_DF03			3

#define KEY_UK_DDF1			4
#define KEY_UK_DF01			5
#define KEY_UK_DF02_1		6
#define KEY_UK_DF02_2		7
#define KEY_UK_DF02_3		8
#define KEY_UK_DF03_1		9
#define KEY_UK_DF03_2		10

#define PADDING     2


extern  struct RecFolder g_recIndex[30];
extern  struct CardDevice *Instance;

//Bin文件读写权限
static int g_BinAccessMap[BIN_START] = {0};

//判断定长文件是否读过标记
static BOOL g_SureFill[BIN_START] = {0};

//CPU初始化后的工作
static int	CpuLastInit(void*);

//为链表分配内存
static int	CpuCallocForList(struct RWRequestS*);

//mode 0:reader 1:writer
static struct RWRequestS  *_CreateReadList(struct RWRequestS *, int mode);

static int _iReadCard(struct RWRequestS *);

static void ListParseContent(struct RWRequestS *list);

// 真实的向设备进行写入工作
static int _iWriteCard(struct RWRequestS *list);

// 向真实写代理传输数据
static void ParseWriteContent(struct RWRequestS *list);

static void* SpliteSegments(struct RWRequestS *list);

static BOOL IsSameFile(struct RWRequestS *oldReq, struct RWRequestS *newReq);

static int GetFloderKeyID(char *folder)
{
	if (strcmp (folder, "DDF1") == 0)
		return KEY_RK_DDF1;
	else if (strcmp (folder, "DF01") == 0)
		return KEY_RK_DF01;
	else if (strcmp (folder, "DF02") == 0) 
		return KEY_RK_DF02;
	else if (strcmp (folder, "DF03") == 0)
		return KEY_RK_DF03;
	return -1;
}

//mode为擦写标志 0为擦除
static int GetUpdateKeyID(int SegID,int mode)
{
	int update_id_16K = 10;
	if (get_card_type() == eCPU16Card) {
		update_id_16K = 11;
		if (SegID <= 2) {
			return KEY_UK_DDF1;
		}
	}
	if (SegID > 2 && SegID < 5)
		return KEY_UK_DDF1;
	else if (SegID < 9) 
		return KEY_UK_DF01;
	else if (SegID < update_id_16K)
		return KEY_UK_DF02_1;
	else if (SegID < 11)
		return KEY_UK_DF02_2;
	else if (SegID < 13)
		return KEY_UK_DF02_3;
	else if (SegID < BIN_START){
		if (mode == 0)
			return KEY_UK_DF03_2;
		return KEY_UK_DF03_1;
	} else {
		return KEY_UK_DF03_1;
	}
	return -1;
}

static int  CpuLastInit(void *data)
{
	struct CardDevice *device;
	BYTE para[20], presp[20];
	BYTE status = 0, len = 0;

	device = (struct CardDevice*)data;

	//进行终端与与设备的认证
	if (!Instance)
		return -1;

	device->iIOCtl(CMD_LED, &para, 2);
	device->iIOCtl(CMD_BEEP, &para, 2);
	status = device->ICCSet(CARDSEAT_PSAM1, &len, presp);
	return (status == 0 ? 0 : -1) ;  
}

static int	CpuCallocForList(struct RWRequestS* list)
{
	return 0;
}


int  CpuReadCard(struct RWRequestS *list, void *apt)
{
	struct RWRequestS *AgentList = NULL;
	int res;

	// 创建真实的读写链表
	AgentList = _CreateReadList(list, 1);
	if (AgentList == NULL) {
		return -1;
	}

	// 真实进行设备读写
	res = _iReadCard(AgentList);

	// 外部列表进行解析
	ListParseContent(list);

	// 删除读写列表
	apt_DestroyRWRequest(AgentList, 1);

	return res;

}

int  CpuWriteCard(struct RWRequestS *list,  void *apt)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	SpliteSegments(list);

	// 创建真实的读写链表
	AgentList = _CreateReadList(list, 1);
	if (AgentList == NULL) {
		return -1;
	}

	// 向真实写代理传输数据
	ParseWriteContent(list);

	// 真实的向设备进行写入工作
	res = _iWriteCard(AgentList);

	// 删除读写列表
	apt_DestroyRWRequest(AgentList, 1);
	return res;
}

static struct RWRequestS  *_CreateReadList(struct RWRequestS *ReqList, int mode)
{
	struct RWRequestS *tmp = NULL, *current = NULL;
	struct RWRequestS *NCurrent;
	struct RWRequestS *ReadList = NULL;

	LOG_INFO("create agent for card read/write request");
	current = ReqList;
	while(current)
	{
		tmp = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memcpy(tmp, current, sizeof(struct RWRequestS));
		tmp->Next = NULL;

		//加入链表
		if(ReadList){
			NCurrent->Next = tmp;
			NCurrent = tmp;
		}else {
			NCurrent = tmp;
			ReadList = NCurrent;
		}

		tmp = current->Next;
		while(tmp && (tmp->offset == (current->offset + current->length)))
		{
			// 设置真正进行读写的代理
			current->agent = NCurrent;
			NCurrent->length += tmp->length;
			current = current->Next ;
			tmp = current->Next;
		}

		current->agent = NCurrent;
		current = current->Next;
	}

	// 分配内存
	current = ReadList;
	while(current)
	{
		if (current->datatype == eRecType) {
			current->length += get_seg_counts(current->nID) * 2;
			current->value = (BYTE*)malloc(current->length + 1);
		} else {
			current->value = (BYTE*)malloc(current->length + 1);
		}
		memset(current->value, 0, current->length+1);
		current = current->Next;
	}
	return ReadList;
}

#define		END_OFFSET		0
#define     START_POS_1		1893
#define		START_POS_2		3267
#define		CPU_8K_OFFSET	254
static int _iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	int status = 0;
	int UCardFlag = 0;
	unsigned char card_type = (unsigned char)get_card_type();
	unsigned char sam_seat = (unsigned char)get_samSeat();
	if (Instance){
		while (pReq){
			status = 0;
			if (strlen((char*)(g_recIndex[pReq->nID-1].section)) > 0) {
				status = Instance->iSelectFile(0, g_recIndex[pReq->nID-1].section);
				UCardFlag = GetFloderKeyID((char*)g_recIndex[pReq->nID-1].section);
				status |= Instance->iUCardAuthSys(card_type, sam_seat, UCardFlag);
				LOG_INFO("文件名:%s, KeyID:%d, iUCardAuthSys:%d", g_recIndex[pReq->nID-1].section, UCardFlag, status);
			}

			if (strlen((char*)(g_recIndex[pReq->nID-1].subSection)) > 0) {
				status = Instance->iSelectFile(0, g_recIndex[pReq->nID-1].subSection);
				UCardFlag = GetFloderKeyID((char*)g_recIndex[pReq->nID-1].subSection);
				status |= Instance->iUCardAuthSys(card_type, sam_seat, UCardFlag);
				LOG_INFO("文件名:%s, KeyID:%d, iUCardAuthSys:%d", g_recIndex[pReq->nID-1].subSection, UCardFlag, status);
			}

			if (status) {
				LOG_ERROR("读验证失败");
				goto done;
			}
			
			switch (pReq->datatype)
			{
			case eSureType:
			case eCycType:
			case eRecType:
				status |= Instance->iReadRec(card_type, g_recIndex[pReq->nID-1].fileName,
					pReq->value, pReq->length, 0xff, get_seg_counts(pReq->nID));
				LOG_INFO("文件名:%s, iReadRec:%d", g_recIndex[pReq->nID-1].fileName, status);
				break;
			case eBinType:
				status |= Instance->iReadBin(card_type, g_recIndex[pReq->nID-1].fileName,
					pReq->value, pReq->length - END_OFFSET, pReq->offset);
				LOG_INFO("文件名:%s, iReadBin:%d", g_recIndex[pReq->nID-1].fileName, status);
				break;
			}
			
			pReq = pReq->Next;
		}
	}

done:
	return status;
}

static void ListParseContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = list;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	BYTE tmpBuff[4096];
	eFileType eType;
	int len = 0, i = 0;
	BYTE padding[2];
	struct RWRequestS *preAgent = NULL;

	while (CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;
		eType = CurrRequest->datatype;
		if (CurrRequest->offset == Agent->offset
			&& preAgent != Agent){
			bcd = Agent->value;
			preAgent = Agent;
		} 

		memset(tmpBuff, 0, sizeof(tmpBuff));
		if (eType == eRecType) {//记录文件
			memcpy(padding, bcd, sizeof(padding));
			bcd += sizeof(padding);
			len = padding[1];
			memcpy(tmpBuff, bcd, len > CurrRequest->length ? CurrRequest->length : len);
			bcd += CurrRequest->length;

		} else { //二进制文件
			memcpy(tmpBuff , bcd, CurrRequest->length);
			bcd += CurrRequest->length;
		}

		//进行转化
		if (CurrRequest->itemtype != eAnsType) {
			BinToHexstr(CurrRequest->value, tmpBuff, CurrRequest->length);
			if (CurrRequest->datatype != eSureType) {
				trimRightF(CurrRequest->value, CurrRequest->length * 2);
			}
			clearFF(CurrRequest->value, CurrRequest->length * 2);

			//将压缩数字中的'a'转回'.'  this is a bad way
			if (CurrRequest->itemtype == eCnType) {
				for (i=0; i<CurrRequest->length * 2 &&
					CurrRequest->value[i] != 0; i++) {
					if (CurrRequest->value[i] == 'a') {
						CurrRequest->value[i] = '.';
					}
				}
			}
		} else {
			memcpy(CurrRequest->value, tmpBuff, CurrRequest->length);
			clearFF(CurrRequest->value, CurrRequest->length);
		}

		CurrRequest = CurrRequest->Next;
	}
}

// 向真实写代理传输数据
static void ParseWriteContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = list;
	struct RWRequestS *prevRequest = NULL;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	int nByteLen = 0;
	eFileType datatype ;
	BOOL bAgent = TRUE;

	while(CurrRequest)
	{
		Agent= CurrRequest->agent;
		datatype = CurrRequest->datatype;
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;

		//fix 8k时，如果首文件记录为空，则会出现bcd值偏移，出现错误
		if ((CurrRequest->offset - Agent->offset) == 0){
			if (prevRequest == NULL ||
				prevRequest->nID != CurrRequest->nID ||
				CurrRequest->nID >= BIN_START ||
				CurrRequest->datatype == eSureType) {
				bcd = Agent->value;
			}
		}
		
		if (datatype == eRecType) { //记录文件
			memcpy(bcd, CurrRequest->value, CurrRequest->length + PADDING);
			*bcd++ = (BYTE)CurrRequest->nColumID;
			*bcd++ =(BYTE)CurrRequest->length;
			bcd += CurrRequest->length;

		} else {
			memcpy(bcd, CurrRequest->value, CurrRequest->length);
			bcd += CurrRequest->length;
		}
		prevRequest = CurrRequest;
		CurrRequest = CurrRequest->Next;
	}

}

// 真实的向设备进行写入工作
static int _iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	struct RWRequestS *pOldReq = NULL;
	int status = 1;
	int UKey = 0;
	int mode = 0;
	char write_flag = 1;
	unsigned char card_type = (unsigned char)get_card_type();
	unsigned char sam_seat = (unsigned char)get_samSeat();
	if (Instance)
	{
		while (pReq)
		{
			status = 0;
			if (!IsSameFile(pReq, pOldReq)) {
				if (strlen((char*)(g_recIndex[pReq->nID-1].section)) > 0) {
					status = Instance->iSelectFile(0, g_recIndex[pReq->nID-1].section);
					UKey = GetFloderKeyID((char*)g_recIndex[pReq->nID-1].section);
					status |= Instance->iUCardAuthSys(card_type, sam_seat, UKey);
					LOG_INFO("文件名:%s, KeyID:%d, iUCardAuthSys:%d", g_recIndex[pReq->nID-1].section, UKey, status);

				}

				if (strlen((char*)(g_recIndex[pReq->nID-1].subSection)) > 0) {
					status |= Instance->iSelectFile(0, g_recIndex[pReq->nID-1].subSection);
					UKey = GetFloderKeyID((char*)g_recIndex[pReq->nID-1].subSection);
					status |= Instance->iUCardAuthSys(card_type, sam_seat, UKey);
					LOG_INFO("文件名:%s, KeyID:%d, iUCardAuthSys:%d", g_recIndex[pReq->nID-1].subSection, UKey, status);

				}
			}

			mode = *(BYTE*)(pReq->value);
			mode = (mode==0 ? 1 : 0);
			UKey = GetUpdateKeyID(pReq->nID, mode);
			status |= Instance->iUCardAuthSys(card_type, sam_seat, UKey);
			if (status) {
				LOG_ERROR("写验证失败, Update KeyID:%d, iUCardAuthSys:%d", UKey, status);
				goto done;
			}

			switch (pReq->datatype)
			{
			case eRecType:
				status |= Instance->iWriteRec(card_type, sam_seat, g_recIndex[pReq->nID-1].fileName, pReq->value,
					pReq->length , write_flag, get_seg_counts(pReq->nID));
				LOG_INFO("记录文件名:%s，内容长度:%d, iWriteRec:%d", g_recIndex[pReq->nID-1].fileName, pReq->length, status);
				break;
			case eBinType:
				status |= Instance->iWriteBin(card_type, g_recIndex[pReq->nID-1].fileName , pReq->value, 0, 
					pReq->length, pReq->offset);
				LOG_INFO("Bin文件名:%s，内容长度:%d, iWriteBin:%d", g_recIndex[pReq->nID-1].fileName, pReq->length, status);

				break;
			case eCycType:
				status |= Instance->iAppendRec(card_type, sam_seat, g_recIndex[pReq->nID-1].fileName, pReq->value, pReq->length);
				LOG_INFO("循环文件名:%s，内容长度:%d, iAppendRec:%d", g_recIndex[pReq->nID-1].fileName, pReq->length, status);

				break;
			case eSureType:

				status = Instance->iSignRec(card_type, sam_seat, g_recIndex[pReq->nID-1].fileName, pReq->nColumID, mode);
				LOG_INFO("标记文件名:%s，内容:%d, iSignRec:%d", g_recIndex[pReq->nID-1].fileName, mode, status);
				break;

			}
			pOldReq = pReq;
			pReq = pReq->Next;
		}
	}
done:
	return status;
}

//为定长数据和循环数据 分成多个Segment写
static void* SpliteSegments(struct RWRequestS *list)
{
	struct RWRequestS *cur = list;
	int span = 1;
	int pos = 0;
	if (cur->datatype == eSureType ||
		cur->datatype == eCycType) {
		if (cur->datatype == eCycType)
			span = get_seg_counts(list->nID);

		while (cur) {
			if (pos == span){
				pos = 0;
				cur->offset = 0;
			}
			pos++;
			cur = cur->Next;
		}
	}
	return list;
}

static BOOL IsSameFile(struct RWRequestS *oldReq, struct RWRequestS *newReq)
{
	if (!newReq)
		return FALSE;

	return (oldReq->nID == newReq->nID &&
			oldReq->datatype == newReq->datatype);
}


adapter * __stdcall InitCpuAdapter()
{
	adapter * CpuAdapter = (adapter *)malloc(sizeof(adapter));
	CpuAdapter->type = eCPUCard;
	CpuAdapter->iLastInit = CpuLastInit;
	CpuAdapter->iCallocForList = CpuCallocForList;
	CpuAdapter->iReadCard = CpuReadCard;
	CpuAdapter->iWriteCard = CpuWriteCard;
	return CpuAdapter;
}


int __stdcall FormatCpuCard(char c)
{
	int status = 1;
	unsigned char send[512];
	unsigned char buff[START_POS_2];
	unsigned char readBuff[100];
	int length = 0;
	unsigned char card_type = (unsigned char)get_card_type();
	unsigned char sam_seat = (unsigned char)get_samSeat();
	memset(buff, c, sizeof(buff));
	memset(readBuff, 0, sizeof(readBuff));

	if (Instance)
	{
		status = 0;
		strcpy((char*)send  , "DDF1");
		status |= Instance->iSelectFile(0 , send);

		strcpy((char*)send  , "DF03");
		status |= Instance->iSelectFile(0 , send);

		status |= Instance->iUCardAuthSys(card_type, sam_seat, KEY_UK_DF03_1);
		if (status) {
			LOG_ERROR("格式化验证失败, Update KeyID:%d, iUCardAuthSys:%d", KEY_UK_DF03_1, status);
			return status;
		}

		if (get_card_type() == eCPU16Card) {
			strcpy((char*)send, "EE01");
			length = START_POS_1 - END_OFFSET - CPU_8K_OFFSET;
			status = Instance->iWriteBin(card_type, send, buff, 0, length, 0);

			strcpy((char*)send, "EE02");
			length = START_POS_1 - END_OFFSET - CPU_8K_OFFSET;
			status = Instance->iWriteBin(card_type, send, buff, 0, length, 0);

			strcpy((char*)send, "ED01");
			length = START_POS_2 - END_OFFSET - CPU_8K_OFFSET;
			status |= Instance->iWriteBin(card_type, send , buff, 0, length, 0);

			strcpy((char*)send, "ED02");
			length = START_POS_2 - END_OFFSET - CPU_8K_OFFSET;
			status |= Instance->iWriteBin(card_type, send , buff, 0, length, 0);

		} else {
			strcpy((char*)send, "ED01");
			status |= Instance->iWriteBin(card_type, send , buff, 0, START_POS_2 - END_OFFSET, 0);

			strcpy((char*)send, "ED02");
			status |= Instance->iWriteBin(card_type, send , buff, 0, START_POS_2 - END_OFFSET, 0);

			strcpy((char*)send, "ED03");
			status |= Instance->iWriteBin(card_type, send , buff, 0, START_POS_2 - END_OFFSET, 0);

			strcpy((char*)send, "ED04");
			status |= Instance->iWriteBin(card_type, send , buff, 0, START_POS_2 - END_OFFSET, 0);

			strcpy((char*)send, "ED05");
			status |= Instance->iWriteBin(card_type, send , buff, 0, START_POS_2 - END_OFFSET, 0);

			strcpy((char*)send, "EE01");
			length = START_POS_1 - END_OFFSET;
			status = Instance->iWriteBin(card_type, send, buff, 0, length, 0);

			strcpy((char*)send, "EE02");
			length = START_POS_1 - END_OFFSET;
			status |= Instance->iWriteBin(card_type, send, buff, 0, length, 0);

			strcpy((char*)send, "EE03");
			length = START_POS_1 - END_OFFSET;
			status |= Instance->iWriteBin(card_type, send, buff, 0, length, 0);
			LOG_INFO("格式化结果:%d", status);
		}
	}
	return status;
}
