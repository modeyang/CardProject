#include <stdio.h>
#include <string.h>
#include "adapter.h"
#include "device.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"
#include <windows.h>
#pragma warning (disable : 4020)
#pragma warning (disable : 4996)


#define DEFAULT_CONTROL	0		//默认配置 KeyA读 KeyB写
#define KEYA_CONTROL	1		//KeyA读写
#define KEYB_CONTROL	2		//KeyB读写

#define KEY_LEN			16
#define BLK_LEN			16

#define KEYA			0
#define KEYB			1

#define DEFAULT			0
#define NHCARD			1
#define GWCARD			2

static unsigned char defKey[6] = 
{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static unsigned char defKeyA[0x6] = 
{0x43, 0x97, 0x04, 0x47, 0x20, 0x47};

static unsigned char defCtrl[0x4]=
{0x08,0x77,0x8f,0x69};

static unsigned char initCtrl[0x4]=
{0xff, 0x07, 0x80, 0x69};

#define ISGWCARD(cardno)   (cardno == 1) 

static struct CardDevice *Instance = NULL;

static int GetControlBuff(unsigned char *pControl, int nSecr);
static int GetWriteWord(const unsigned char *pControl);
static int iGetKeySeed(int type, unsigned char *seed);
static int repairKeyForFault(unsigned char *ctrlword);
static int repairKeyB(unsigned char *ctrlword);
static int repairKeyBAllF(unsigned char *ctrlword);
static int ChangePwdEx(const unsigned char * pNewKeyA ,
					   const unsigned char * ctrlword,
					   const unsigned char * pNewKeyB,
					   const unsigned char * poldPin ,
				       unsigned char nsector,
					   unsigned char keyA1B0,
					   unsigned char changeflag);


static int _FormatCard(unsigned char *pControl, 
					   unsigned char* szFormat, 
					   int nBlk, 
					   unsigned char *keyB);


int __stdcall IsAllTheSameFlag(const unsigned char *szBuf, int nLen, unsigned char cflag)
{
	int i=0;
	for (; i<nLen; ++i)
	{
		if (szBuf[i] != cflag)
		{
			return -1;
		}
	}
	return 0;
}

/**
 *
 */
static int str2bcd(const char *str, unsigned  char *bcd, int len)
{
	int i, j;
	
	for(i = 0; i < len; i++) 
	{		
		j = i >> 1;	
		
		if(i & 1) 
		{
			bcd[j] |= (str[i-1] - 48) << 4;
		}
		else
		{
			bcd[j]=str[i+1] -48;
		}
	}
	
	return i;
}

/**
 *
 */
static int bcd2str(const unsigned char *bcd, char *str, int len)
{	
	int i, j;

	for(i=0; i < len; i++) 
	{
		j = i >> 1;
		if(i & 1) 
		{
			str[i] |= (bcd[j] & 0x0f) + 48;
		}	
		else 
		{
			str[i] = (bcd[j] >> 4) + 48;
		}
	}

	return i;
}

//获取写控制， 初始化的控制位时返回1，其他情况为0
static int GetWriteWord(const unsigned char *pControl)
{
	int nRet = 0;
	int i = 0;

	for (; i<4; ++i)
	{
		if (initCtrl[i] != pControl[i])
		{
			return DEFAULT_CONTROL;
		}
	}
	return KEYA_CONTROL;
}

static int GetControlBuff(unsigned char *pControl, int nSecr)
{
	//439704472047
	unsigned char keyA[6];
	unsigned char bRead = 0;
	int BlkNr = 0;
	memset(keyA, 0, sizeof(keyA));

	/* 如果没有读卡设备接入*/
	if(!Instance) 
		return CardInitErr;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	BlkNr = nSecr * 4 + 3;
	memcpy(keyA, defKeyA, sizeof(keyA));
	bRead = Instance->iRead(keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	if (bRead != 0)
	{
		if (iCoreFindCard() != 0)
			return CardScanErr;

		memset(keyA, 0xff, sizeof(keyA));
		bRead = Instance->iRead(keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	}
	return bRead;
}


#define FAILE_RETRY  2
int __stdcall aFormatCard(unsigned char cFlag)
{
	unsigned char seed[32];
	unsigned char keyB[6];
	unsigned char newKeyA[0x6];
	unsigned char newKeyB[0x6];
	unsigned char changeflag = 2;
	unsigned char ctrlWork[0x4]={0x08,0x77,0x8f,0x69};
	unsigned char szFormat[KEY_LEN];
	int nLen=0, i;
	int faile_retry = 0;
	int nRet = 0;
	memset(newKeyA, 0xff, 6);
	memset(newKeyB, 0xff, 6);
	memset(seed, 0, sizeof(seed));

	nLen = iGetKeySeed(DEFAULT, seed);
	if (nRet == -1 || nLen == 0 ||
		IsAllTheSameFlag(seed, nLen/2, 0x30)== 0 ||
		IsAllTheSameFlag(seed, nLen/2, 0x3f)== 0 )
	{
		memset(keyB, 0xff, 6);
	}
	else
	{
		iGetKeyBySeed(seed, keyB);
	}

	LogPrinter("开始格式化数据:");
	GetControlBuff(ctrlWork, 0);
	memset(szFormat, cFlag, KEY_LEN);
	while (faile_retry < FAILE_RETRY)
	{
		for (i=4; i<64; ++i)
		{
			if ((i+1) % 4 == 0)
			{
				continue;
			}
			nRet = _FormatCard(ctrlWork, szFormat, i, keyB);
			if (nRet)
			{
				faile_retry++;
				break;
			}
		}
		if (nRet)
		{
			nRet = repairKeyB(ctrlWork);
			if (nRet)
				nRet = repairKeyForFault(ctrlWork);
			if (!nRet)
			{
				faile_retry = FAILE_RETRY-1;
				LogPrinter("\n重新格式化数据:");
			}
		}
		else
		{
			break;
		}
	}

	if (nRet)
		goto done;

	LogPrinter("格式化密码:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(newKeyA, ctrlWork, newKeyB, keyB, i, 0, changeflag);
		if (nRet != 0)
			break;
	}
	LogPrinter("%d\n", nRet);

done:
	return nRet==0?CardProcSuccess:CardFormatErr;
}


static int _FormatCard(unsigned char *pControl, unsigned char* szFormat, int nBlk, unsigned char *keyB)
{
	int nOffset = 128*nBlk;
	unsigned char bool_test = 0;

	if(!Instance)
		return CardInitErr;

	if (GetWriteWord(pControl) == DEFAULT_CONTROL)
	{
		bool_test = Instance->iWrite(keyB, szFormat, DEFAULT_CONTROL, 128, nOffset);
	}
	else
	{
		bool_test = Instance->iWrite(defKeyA, szFormat,  KEYA_CONTROL ,128, nOffset);
	}
	LogPrinter("%d", bool_test);
	return  bool_test==0 ? 0:-1;
}

static int ChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
					   const unsigned char * pNewKeyB,const unsigned char * poldPin ,
					   unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag)
{
	unsigned char  nRet = 0;
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	if (GetWriteWord(ctrlword) == DEFAULT_CONTROL)
	{
		nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, poldPin, nsector, keyA1B0, changeflag);
	}
	else
	{
		if (0 == IsAllTheSameFlag(pNewKeyA, 6, 0xff))
		{
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, nsector, 1, changeflag);
		}
		else
		{
			memset(KeyA, 0xff, 6);
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, nsector, 1, changeflag);
		}
	}

	return nRet ==0 ? 0:-1;
}
static int iGetKeySeed(int type, unsigned char *seed)
{
	unsigned char tmp[32];

	//没有寻到卡
	if(!Instance || !Instance->iRead)
		return -1;

	//读取seed
	memset(tmp, 0, 32);
	Instance->iRead(defKeyA, tmp, 56, 640);
	if (type == GWCARD) {
		bcd2str(tmp, seed, 14);
		goto done;
	} else if (type == NHCARD) {
		memset(tmp, 0, sizeof(tmp));
		Instance->iRead(defKeyA, tmp, 72, 792);
		bcd2str(tmp, seed, 18);
		goto done;
	} else {
		if (tmp[0]>>4 == 0)
		{
			memset(tmp, 0, sizeof(tmp));
			Instance->iRead(defKeyA, tmp, 72, 792);
			bcd2str(tmp, seed, 18);
		}
		else
		{
			bcd2str(tmp, seed, 14);
		}
	}

done:
	return strlen(seed);
}


static int repairKeyForFault(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];  //变成原来的KeyB
	unsigned char curKeyB[0x6];//当前错误的keyB
	unsigned char NHKyeB[6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;
	memset(curKeyB, 0x75, sizeof(curKeyB));

	//没有寻到卡
	if(!Instance || !Instance->iRead)
		return -1;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	//读出卡号,得出旧的KeyB
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, oldKeyB);

	//如果是农合卡，直接失败
	if (!ISGWCARD(type)) {
		LogPrinter("此卡为农合卡，直接失败\n");
		return -1;
	}

	//得到以农合号为依据的keyB
	memset(seed, 0, sizeof(seed));
	memset(NHKyeB, 0, sizeof(NHKyeB));
	iGetKeySeed(NHCARD, seed);
	iGetKeyBySeed(seed, NHKyeB);

	LogPrinter("[6-4新疆错误修补密码]:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlword, oldKeyB, curKeyB, i, 0, changeflag);
		if (nRet) {
			if (iCoreFindCard() != 0)
				break;
			nRet = ChangePwdEx(defKeyA, ctrlword, oldKeyB, NHKyeB, i, 0, changeflag);
		}
		LogPrinter(" %d", nRet);
		if (nRet)
			break;
	}
	LogPrinter("\n");
	return nRet;

}

static int repairKeyBAllF(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];
	unsigned char newKeyB[6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;
	memset(oldKeyB, 0xFF, sizeof(oldKeyB));

	//没有寻到卡
	if(!Instance || !Instance->iRead)
		return -1;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	//读出卡号,得出旧的KeyB
	memset(newKeyB, 0, sizeof(newKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, newKeyB);

	if (!ISGWCARD(type)) {

		//得到以农合号为依据的keyB
		memset(newKeyB, 0, sizeof(newKeyB));
		memset(seed, 0, sizeof(seed));
		iGetKeySeed(NHCARD, seed);
		iGetKeyBySeed(seed, newKeyB);
	}

	LogPrinter("修补密码:");
	for (i=0; i<BLK_LEN; ++i){
		nRet = ChangePwdEx(defKeyA, ctrlword, newKeyB, oldKeyB, i, 0, changeflag);
		LogPrinter("%d", nRet);
		if (nRet)
			break;
	}

	return nRet;
}

static int repairKeyB(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];
	unsigned char newKeyB[0x6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;

	//没有寻到卡
	if(!Instance || !Instance->iRead)
		return -1;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	//读出卡号,得出旧的KeyB
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, oldKeyB);

	if (ISGWCARD(type)) {
		LogPrinter("如果是公卫卡，直接失败\n");
		return -1;
	}

	//得到以农合号为依据的keyB
	memset(newKeyB, 0, sizeof(newKeyB));
	memset(seed, 0, sizeof(seed));
	iGetKeySeed(NHCARD, seed);
	iGetKeyBySeed(seed, newKeyB);

	LogPrinter("修补密码:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlword, newKeyB, oldKeyB, i, 0, changeflag);
		LogPrinter("%d", nRet);
		if (nRet)
			break;
	}

	return nRet;
}



/**
 * 函数： initCoreDevice
 * 参数：
 *
 * 返回值：
 * 成功： 0
 * 失败：非零
 */
int __stdcall initCoreDevice(const char *System)
{
	if(!Instance) 
		Instance = getCardDevice(System);

	return (Instance != NULL ? 0:-1);
}

/**
 * 函数：closeCoreDevice
 * 参数：
 *
 * 返回值：
 * 成功： 0
 * 失败： 非零
 */
int __stdcall closeCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret) 
		Instance = NULL;

	return ret;
}

int __stdcall opendev(void)
{
	int ret = 0;
	if (Instance)
		ret = Instance->iOpen();
	return ret;
}
int __stdcall closedev(void)
{
	int ret = 0;
	if (Instance)
		ret = Instance->iClose();
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
int __stdcall iCoreFindCard(void)
{

	// 探测卡，如果没有卡，自动退出
	unsigned char ret = (unsigned char)Instance->iScanCard();
	LogPrinter("ScanCard: %d\n", ret);
	if(ret != 0)
		return CardScanErr; 
	return 0;
}

int __stdcall authUCard(const char *System)
{
	return authUDev(System);
}
/**
 *
 */
int __stdcall iCtlCard(int cmd, void *data)
{
	// 初始化结果为失败
	unsigned char result = 0;

	// 如果设置了这些函数
	if(Instance && Instance->iIOCtl) 
	{
		result = (unsigned char)Instance->iIOCtl(cmd, data);
	}

	return result;
}

/**
 *
 */
struct RWRequestS* __stdcall CreateRWRequest(struct XmlSegmentS *listHead, int mode)
{
	struct XmlSegmentS	*SegmentElement = NULL;
	struct XmlColumnS	*ColumnElement = NULL;

	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;
	struct RWRequestS	*result = NULL;

	SegmentElement = listHead;
	while(SegmentElement)
	{
		ColumnElement = SegmentElement->Column;
		while(ColumnElement)
		{
			// 产生新的节点
			TempRequest = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
			memset(TempRequest, 0, sizeof(struct RWRequestS));
			
			// 对节点进行赋值
			TempRequest->mode = mode;
			TempRequest->offset = ColumnElement->Offset;
			TempRequest->length = ColumnElement->ColumnBit;
			TempRequest->value = ColumnElement->Value;
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


/**
 *
 */
void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag)
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

	return;
}

/**

 *
 */
static struct RWRequestS  *_CreateReadList(struct RWRequestS *TmpReadRequest)
{
	struct RWRequestS *tmp = NULL, *current = NULL;
	struct RWRequestS *NCurrent;
	struct RWRequestS *ReadList = NULL;

	current = TmpReadRequest;
	while(current)
	{
		tmp = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(tmp, 0, sizeof(struct RWRequestS));

		tmp->offset = current->offset;
		tmp->length = current->length;
		
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
		current->value = (unsigned char *) malloc((current->length + 7)/8);

		current = current->Next;
	}

	return ReadList;
}

/**
 *
 */
#define ID_OFFSET	1280
static void ListParseContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	unsigned char *bcd = NULL;
	int nOffSet = 0;

	CurrRequest = list;
	while(CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;
		bcd = (Agent->value) + (CurrRequest->offset -Agent->offset)/8;
		memset(CurrRequest->value, 0, 100);
		// 过滤为空的代码: 如果为0xff,则跳过这个列
		if(bcd[0] == 0xff)
		{
			goto Next;
		}

		nOffSet = (CurrRequest->offset -Agent->offset)/4%2;

		//add by yanggx	20110619
		////BCD转化成字符串 有问题， 将单个4位的BCD编码过滤掉了.
		if(CurrRequest->length == 4)
		{
			if (nOffSet == 0)
			{
				nOffSet = CurrRequest->offset/4%2;
			}
			Bcd2Ch(CurrRequest->value, bcd, nOffSet);	
		}
		else
		{
			//add by yanggx 20110706
			Bcd2StrOffSet(CurrRequest->value, bcd, (CurrRequest->length)/4, nOffSet);
			if (CurrRequest->offset == ID_OFFSET)
			{
				if (CurrRequest->value[(CurrRequest->length)/4-1] == 'D'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'd')
				{
					CurrRequest->value[(CurrRequest->length)/4-1] = 'x';

				}else if (CurrRequest->value[(CurrRequest->length)/4-1] == 'e'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'E')
				{
					CurrRequest->value[(CurrRequest->length)/4-1] = 'X';
				}

			}
		}

		// 256是为了识别名字，名字处理有点小小的不同

		if(CurrRequest->length == 256 || CurrRequest->length == 128) {
			CleanName((char *)CurrRequest->value);
		}
		else {
			iCleanStr((char *)CurrRequest->value);
		}
		Next:
		// 向后迭代
		CurrRequest = CurrRequest->Next;
	}
	return;
}

/**
 *
 */
static int _iReadCard(struct RWRequestS *list)
{
	//439704472047
	unsigned char bRead = 0;

	struct RWRequestS *CurrRequest= NULL;
	struct ReadRequest *CurrList = NULL;
	
	/* 如果没有读卡设备接入*/
	if(!Instance) 
		return CardInitErr;

	CurrRequest = list;
	while(CurrRequest)
	{
		memset(CurrRequest->value, 0, (CurrRequest->length + 7)/8);
		bRead = Instance->iRead(defKeyA, CurrRequest->value, CurrRequest->length, CurrRequest->offset);

		// 向后迭代
		CurrRequest = CurrRequest->Next;
	}

	return bRead==0 ? 0:CardReadErr;
}


int __stdcall iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res;

	// 对链表进行排序

	// 创建真实的读写链表
	AgentList = _CreateReadList(list);

	// 真实进行设备读写
	res = _iReadCard(AgentList);

	// 外部列表进行解析
	ListParseContent(list);

	// 删除读写列表
	DestroyRWRequest(AgentList, 1);;

	return res;
}



static void ParseWriteContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	unsigned char *bcd = NULL;
	int len = 0;
	int i=0;
	int nPos = 0, nBitPos = 0;
	int nByteLen = 0;
	int nAgentReLen = 0;

	CurrRequest = list;
	while(CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;

		// 过滤为空的代码,多余的代码
		if(!strlen(CurrRequest->value))
		{
			memset(CurrRequest->value, 0xff, (CurrRequest->length)/8);
		}

		//comment by yanggx	110612  按字符存取，将每个Agent的相关字符记录下来
		//Agent第一次
		nByteLen = CurrRequest->length/4;
		if ((CurrRequest->offset - Agent->offset) == 0)
		{
			bcd = Agent->value;

			//add by yanggx 20110616 判断开始位是否有数据
			nBitPos = CurrRequest->offset/4%2;
			if(nBitPos == 1)
			{
				memset(bcd, 0xff, 1);
			}
			nAgentReLen = nByteLen + nBitPos;
		}
		else
		{
			bcd = Agent->value + nAgentReLen/2;
			nAgentReLen += nByteLen;
		}

		// 这里的长度应该是字符串的长度，不应为空间的长度 
		len = (int)strlen(CurrRequest->value);
		nPos = (nAgentReLen - nByteLen)%2;

		for(i=0; i<len; ++i)
		{
			//将字符转为BCD编码，保存在bcd中
			StrCh2Bcd(CurrRequest->value[i], bcd+(i+nPos)/2, (i+nPos)%2);
		}

		//add by yanggx 20110615
		//如果字符长度没有字节长度的4倍，后面加0xf
		nByteLen = CurrRequest->length/4;
		if (nByteLen > len)
		{
			for(; i<nByteLen; ++i)
			{
				StrCh2Bcd('f', bcd+(i+nPos)/2, (i+nPos)%2);
			}
		}
		CurrRequest = CurrRequest->Next;
	}

	return;
}


static int _iWriteCard(struct RWRequestS *list)
{
	unsigned char seed[32];
	unsigned char  bool_test = -1;
	unsigned char keyNewB[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char Key[6];
	unsigned char pControl[4];
	int nWriteControl = DEFAULT_CONTROL;
	struct RWRequestS *CurrRequest= NULL;
	int faile_retry = 0;

	if(!Instance)
		return CardInitErr;

	//获取控制位，判断卡片读写形式
	GetControlBuff(pControl, 0);
	nWriteControl = GetWriteWord(pControl);
	
	while (faile_retry < FAILE_RETRY)
	{
		memset(seed, 0, 32);

		//根据卡号首位获取生成Keyb的种子
		iGetKeySeed(DEFAULT, seed);   

		//全FF/00的情况视为初始密码为全F
		if (IsAllTheSameFlag(seed, 14, 0x3f) == 0 ||
			IsAllTheSameFlag(seed, 14, 0x30) == 0)
		{
			//KeyA有全部读写权限
			memcpy(Key, keyNewB, sizeof(keyNewB));
		}
		else
		{
			//默认读写形式
			if (nWriteControl == DEFAULT_CONTROL)
			{
				iGetKeyBySeed(seed, keyNewB);
				memcpy(Key, keyNewB, sizeof(keyNewB));
			}
			else
			{
				//KeyA有全部读写权限
				nWriteControl = KEYA_CONTROL;
				memcpy(Key, defKeyA, sizeof(defKeyA));
			}
		}

		if (iCoreFindCard() != 0)
			return CardScanErr;

		CurrRequest = list;
		while(CurrRequest)
		{
			bool_test = Instance->iWrite(Key, CurrRequest->value, nWriteControl , 
										CurrRequest->length, CurrRequest->offset);
			CurrRequest = CurrRequest->Next;
			if (bool_test)
			{
				LogPrinter("写卡失败，需要修补密码\n");
				faile_retry++;
				break;
			}
		}

		//对于keyb错误的写操作，修补密码
		if (bool_test)
		{
			bool_test = repairKeyBAllF(pControl);
			if (bool_test) {
				//修补农合卡KeyB错误
				bool_test = repairKeyB(pControl);
			}
			if (bool_test) {
				//修补新疆密码为0x75的错误
				bool_test = repairKeyForFault(pControl);
			}
			if (!bool_test)
			{
				LogPrinter("修补密码成功，重新写卡\n");
				faile_retry = FAILE_RETRY-1;
			}
		}
		else
		{
			break;
		}
	}
	return bool_test==0 ? 0:CardWriteErr;
}


int __stdcall iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	// 对链表进行排序

	// 创建真实的读写链表
	AgentList = _CreateReadList(list);

	// 向真实写代理传输数据
	ParseWriteContent(list);

	// 真实的向设备进行写入工作
	res = _iWriteCard(AgentList);

	// 删除读写列表
	DestroyRWRequest(AgentList, 1);

	return res;
}

int __stdcall InitPwd(unsigned char *newKeyB)
{
	unsigned char oldKeyB[6], ctrlWord[6];
	unsigned char changeflag=2, i, nRet;
	memset(oldKeyB, 0xff, 6);
	memset(ctrlWord, 0, 6);

	LogPrinter("重置密码:");
	GetControlBuff(ctrlWord, 0);
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlWord, newKeyB, oldKeyB, i, 0, changeflag);

		LogPrinter("%d", nRet); 
		if (nRet != 0)
			break;
	}
	LogPrinter("\n");
	return nRet;
}

