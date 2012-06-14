#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include "public/device.h"
#include "public/liberr.h"
#include "adapter.h"
#include "UtilProcess.h"

#pragma warning (disable : 4996)

#define  USERCARD	0
#define  SAMCARD	1

#define CMD_BEEP	1
#define CMD_LED		2

struct RecFolder g_recIndex[30]= {
	{"", "", ""},
};

//每个字段的最大记录条数
static int g_RecMap[] = {0, 10, 5, 1, 6, 4, 9, 3, 4, 15, 1};

static struct RWRequestS  *_CreateReadList(struct RWRequestS *);

static int _iReadCard(struct RWRequestS *);

static void ListParseContent(struct RWRequestS *list);

// 真实的向设备进行写入工作
static int _iWriteCard(struct RWRequestS *list);

// 向真实写代理传输数据
static void ParseWriteContent(struct RWRequestS *list);

static int CheckActionValide();


//处理事件链表
extern struct XmlProgramS *g_XmlListHead;
static struct CardDevice *Instance = NULL;

int __stdcall initCoreDevice(const char *System)
{
	//unsigned char rndString[8];
	//unsigned char KeySys[16];
	//unsigned char KeyDev[16];
	unsigned char resp[260];
	unsigned char len = 0;
	unsigned short para = 30;
	unsigned char *presp = resp;
	int status = 0;
	if(!Instance) 
		Instance = getCardDevice(System);

	//进行终端与与设备的认证
	if (!Instance)
		return -1;
	//memset(rndString, 0, sizeof(rndString));
	//memset(KeyDev, 0, sizeof(KeyDev));
	//memset(KeySys, 0, sizeof(KeySys));
	//Instance->iGetRandom(0, 8, rndString);
	//status = Instance->iDevAuthSys(KeySys);
	//status |= Instance->iSysAuthDev(KeySys, KeyDev);
	Instance->iIOCtl(CMD_LED, &para, 2);
	Instance->iIOCtl(CMD_BEEP, &para, 2);
	status = Instance->ICCSet(SAMCARD, &len, presp);
	return (status == 0 ? 0 : -1) ;  
}

int __stdcall closeCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret) 
		Instance = NULL;

	return ret;
}

//寻卡操作，做相应的卡权限设置
int __stdcall iCoreFindCard(void)
{
	// 探测卡，如果没有卡，自动退出
	unsigned char ret = 0;// = (unsigned char)Instance->ICCSet(
	return ret==0 ? 0 : CardScanErr ;
}


int __stdcall DestroyCPUData()
{
	return 0;
}

int __stdcall iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res;

	// 创建真实的读写链表
	AgentList = _CreateReadList(list);

	// 真实进行设备读写
	res = _iReadCard(AgentList);

	// 外部列表进行解析
	ListParseContent(list);

	// 删除读写列表
	DestroyRequest(AgentList, 1);

	return res;

}

int __stdcall iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	// 创建真实的读写链表
	AgentList = _CreateReadList(list);

	// 向真实写代理传输数据
	ParseWriteContent(list);

	// 真实的向设备进行写入工作
	res = _iWriteCard(AgentList);

	// 删除读写列表
	DestroyRequest(AgentList, 1);
	return res;
}

static struct RWRequestS  *_CreateReadList(struct RWRequestS *ReqList)
{
	struct RWRequestS *tmp = NULL, *current = NULL;
	struct RWRequestS *NCurrent;
	struct RWRequestS *ReadList = NULL;

	current = ReqList;
	while(current)
	{
		tmp = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(tmp, 0, sizeof(struct RWRequestS));

		tmp->offset = current->offset;
		tmp->length = current->length;
		tmp->datatype = current->datatype;
		tmp->nID = current->nID;
		tmp->mode = current->mode;

		//加入链表
		if(ReadList)
		{
			NCurrent ->Next = tmp;
			NCurrent = tmp;
		}
		else 
		{
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
		if (current->datatype = 1) {
			current->length += g_RecMap[current->nID] * 2;
			current->value = (BYTE*)malloc(current->length);
		} else 
		{
			current->value = (BYTE*)malloc(current->length);
		}
		memset(current->value, 0, current->length);
		current = current->Next;
	}
	return ReadList;
}


static int _iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	struct RWRequestS *Rw = NULL;
	int status = 0;
	if (Instance)
	{
		if (!CheckActionValide())
			return -1;

		while (pReq)
		{
			if (memcmp (g_recIndex[pReq->nID].section , "", 10) != 0)
				Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID].section);

			//if (memcmp(g_recIndex[pReq->nID].subSection, "", 10) != 0)
			//	Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID].subSection);

			if (pReq->datatype)
			{
				status = Instance->iReadRec(USERCARD, g_recIndex[pReq->nID].fileName,pReq->value,
					                       pReq->length, 0xff, g_RecMap[Rw->nID]);
			}
			else
			{
				Instance->iReadBin(USERCARD, g_recIndex[pReq->nID].fileName,pReq->value, pReq->length, pReq->offset);
			}
			pReq = pReq->Next;
		}
	}

	return 0;
}

static void ListParseContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS	  *pCur = list;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	int datatype = 0;
	BYTE len = 0;

	while(pCur)
	{
		CurrRequest = pCur->agent;
		datatype = pCur->datatype;
		while (CurrRequest)
		{
			ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
			if (datatype) {//记录文件
				bcd = Agent->value;
				bcd++;
				len = *(BYTE*)bcd;
				memcpy(CurrRequest->value, bcd+1, len > CurrRequest->length ? CurrRequest->length : len);
				bcd += len + 1;
			
			} else { //二进制文件

				bcd = (Agent->value) + (CurrRequest->offset - Agent->offset);

				memcpy(CurrRequest->value , bcd, CurrRequest->length);
			}
			Agent = CurrRequest->agent;


			CurrRequest = CurrRequest->Next;
		}
		pCur = pCur->Next;
	}
}

// 向真实写代理传输数据
static void ParseWriteContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS   *pCur = list;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	int nByteLen = 0, datatype = 0;
	
	while (pCur)
	{
		CurrRequest = pCur->agent;
		datatype = pCur->datatype;
		while(CurrRequest)
		{
			ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
			Agent = CurrRequest->agent;

			// 过滤为空的代码,多余的代码
			if(!strlen((char*)CurrRequest->value))
			{
				memset(CurrRequest->value, 0xff, CurrRequest->length);
			}

			if ((CurrRequest->offset - Agent->offset) == 0)
			{
				bcd = Agent->value;
			}

			if (datatype) { //记录文件
				*bcd = (BYTE)CurrRequest->nID;
				*(bcd+1) = (BYTE)strlen((const char *)CurrRequest->value);
				memcpy(bcd+2, CurrRequest->value, CurrRequest->length);
				bcd += CurrRequest->length + 2;

			} else {
				memcpy(bcd, CurrRequest->value, CurrRequest->length);
				bcd += CurrRequest->length;
			}

			CurrRequest = CurrRequest->Next;
		}
		pCur = pCur->Next;
	}


}

// 真实的向设备进行写入工作
static int _iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	struct RWRequestS *Rw = NULL;
	if (Instance)
	{
		if (!CheckActionValide())
			return -1;

		while (pReq)
		{
			//Instance->iSelectFile(USERCARD, pReq->appContext);
			//Instance->iSelectFile(USERCARD, pReq->section);
			//Instance->iSelectFile(USERCARD, pReq->subSection);
			if (pReq->datatype)
			{
				//Instance->iReadRec(USERCARD, , Rw->value, Rw->length, 0xff, g_RecMap[Rw->nID]);
			}
			else
			{
				//Instance->iReadBin(USERCARD, pReq->fileName, Rw->value, Rw->length, Rw->offset);
			}
			pReq = pReq->Next;
		}
	}

	return 0;
}

static int CheckActionValide()
{
	unsigned char len = 0;
	unsigned char resp[260];
	int status = 0;
	if (Instance)
	{
		//status = Instance->iUCardAuthSys();
		//status |= Instance->iSysAuthUCard("", "", "");
		status = Instance->ICCSet(USERCARD, &len , resp);
		status |= Instance->iGetRandom(USERCARD , 8 , resp);

		status |= Instance->iGetRandom(SAMCARD , 4 , resp);

		return (status==0);
	}
	return FALSE;
}

