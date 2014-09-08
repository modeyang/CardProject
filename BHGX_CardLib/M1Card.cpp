#include "M1Card.h"
#include "device.h"
#include "StringUtil.h"
#include "adapter.h"
#include "resource.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"
#include "SegmentHelper.h"
#include "public/ConvertUtil.h"
#include "Card.h"
#include <map>
#include <vector>

using namespace std;

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

#define DBGADAP(format, ...)  LogWithTime(0, format)

static unsigned char defKey[6] = 
{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static unsigned char defKeyA[0x6] = 
{0x43, 0x97, 0x04, 0x47, 0x20, 0x47};

static unsigned char defCtrl[0x4]=
{0x08,0x77,0x8f,0x69};

static unsigned char initCtrl[0x4]=
{0xff, 0x07, 0x80, 0x69};

#define ISGWCARD(cardno)   (cardno == 1) 
#define ISAPTSCANCARD {if (apt_ScanCard() != 0) return CardScanErr;}

#define SAFE_DELETE(a)  if (a != NULL) { delete(a);a = NULL;}
#define SAFE_DELETE_C(a)  if (a != NULL) { free(a);a = NULL;}

//M1卡时会将一部分在第2块的数据保存在第五块区域，当只访问第二扇区时
bool g_OnlySecond = false;
struct CardDevice *Instance;

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

//M1卡初始化后的工作
static int	M1LastInit(void*);

//为链表分配内存
static int	M1CallocForList(struct RWRequestS*);

//cardops
static int InitionM1GList(char *xml);


adapter M1Adapter;
CardOps g_CardOps;
static struct XmlProgramS *g_XmlListHead = NULL;
static CSegmentHelper *g_SegHelper = NULL;

static int InitM1GlobalList() 
{
	if (g_XmlListHead) {
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
		return CardInitErr;
	}

	// 如果已经分配了链表
	if(g_XmlListHead)
		return -2;

	// 初始化全局列表
	InitionM1GList(pvRes);
	return 0;
}

static int InitionM1GList(char *xml)
{
	if (g_XmlListHead == NULL){
		g_XmlListHead = (struct XmlProgramS*)malloc(sizeof(struct XmlProgramS));
		g_XmlListHead->SegHeader = g_XmlListHead->SegTailer = NULL;
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
		if(g_XmlListHead->SegHeader) {
			g_XmlListHead->SegTailer->Next = pSeg;
			g_XmlListHead->SegTailer = pSeg;
		}  else {
			g_XmlListHead->SegHeader = g_XmlListHead->SegTailer = pSeg;
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
static int str2bcd(const unsigned char *str, unsigned  char *bcd, int len)
{
	int i, j;

	for(i = 0; i < len; i++) 
	{		
		j = i >> 1;	

		if(i & 1) {
			bcd[j] |= (str[i-1] - 48) << 4;
		} else {
			bcd[j]=str[i+1] -48;
		}
	}

	return i;
}

/**
*
*/
static int bcd2str(const unsigned char *bcd, unsigned char *str, int len)
{	
	int i, j;

	for(i=0; i < len; i++) 
	{
		j = i >> 1;
		if(i & 1) {
			str[i] |= (bcd[j] & 0x0f) + 48;
		}else {
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
		if (initCtrl[i] != pControl[i]){
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
	ISAPTSCANCARD;

	BlkNr = nSecr * 4 + 3;
	memcpy(keyA, defKeyA, sizeof(keyA));
	bRead = Instance->iReadBin(CARDSEAT_M1, keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	if (bRead != 0){
		ISAPTSCANCARD;

		memset(keyA, 0xff, sizeof(keyA));
		bRead = Instance->iReadBin(CARDSEAT_M1, keyA, pControl, 4*8, BlkNr * 128 + 6*8);
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
		IsAllTheSameFlag(seed, nLen/2, 0x3f)== 0 ){
		memset(keyB, 0xff, 6);
	}else{
		iGetKeyBySeed(seed, keyB);
	}

	LogPrinter("开始格式化数据:");
	GetControlBuff(ctrlWork, 0);
	memset(szFormat, cFlag, KEY_LEN);
	while (faile_retry < FAILE_RETRY)
	{
		for (i=4; i<64; ++i)
		{
			if ((i+1) % 4 == 0){
				continue;
			}
			nRet = _FormatCard(ctrlWork, szFormat, i, keyB);
			if (nRet){
				faile_retry++;
				LogPrinter( "格式化失败，需要修补密码\n");
				break;
			}
		}

		if (nRet){
			nRet = repairKeyB(ctrlWork);
			if (nRet) {
				nRet = repairKeyForFault(ctrlWork);
			}

			if (!nRet)
			{
				faile_retry = FAILE_RETRY-1;
				DBGADAP( "修补密码成功，重新格式化数据\n");
				LogPrinter("\n重新格式化数据:");
			}
		}else{
			break;
		}
	}

	if (nRet) {
		goto done;
	}

	LogPrinter("格式化密码:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(newKeyA, ctrlWork, newKeyB, keyB, i, 0, changeflag);
		if (nRet != 0) {
			break;
		}
	}
	DBGADAP( "格式化密码结果:%d\n", nRet);
	LogPrinter("%d\n", nRet);

done:
	return nRet==0?CardProcSuccess:CardFormatErr;
}


static int _FormatCard(unsigned char *pControl, unsigned char* szFormat,
					   int nBlk, unsigned char *keyB)
{
	int nOffset = 128*nBlk;
	unsigned char bool_test = 0;

	if(!Instance)
		return CardInitErr;

	if (GetWriteWord(pControl) == DEFAULT_CONTROL)
	{
		bool_test = Instance->iWriteBin(CARDSEAT_M1, keyB, szFormat, DEFAULT_CONTROL, 128, nOffset);
	}else{
		bool_test = Instance->iWriteBin(CARDSEAT_M1, defKeyA, szFormat,  KEYA_CONTROL ,128, nOffset);
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
		nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, poldPin, 
			nsector, keyA1B0, changeflag);
	}
	else
	{
		if (0 == IsAllTheSameFlag(pNewKeyA, 6, 0xff))
		{
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, 
				nsector, 1, changeflag);
		}
		else
		{
			memset(KeyA, 0xff, 6);
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, 
				nsector, 1, changeflag);
		}
	}
	return nRet ==0 ? 0:-1;
}

static int iGetKeySeed(int type, unsigned char *seed)
{
	unsigned char tmp[32];

	//没有寻到卡
	if(!Instance || !Instance->iReadBin)
		return -1;

	//读取seed
	memset(tmp, 0, 32);
	Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 56, 640);
	if (type == GWCARD) {
		bcd2str(tmp, seed, 14);
		goto done;
	} else if (type == NHCARD) {
		memset(tmp, 0, sizeof(tmp));
		Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 72, 792);
		bcd2str(tmp, seed, 18);
		goto done;
	} else {
		if (tmp[0]>>4 == 0){
			memset(tmp, 0, sizeof(tmp));
			Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 72, 792);
			bcd2str(tmp, seed, 18);
		} else {
			bcd2str(tmp, seed, 14);
		}
	}

done:
	return strlen((char*)seed);
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
	if(!Instance || !Instance->iReadBin)
		return -1;

	ISAPTSCANCARD;

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
			ISAPTSCANCARD;
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
	if(!Instance || !Instance->iReadBin)
		return -1;
	ISAPTSCANCARD;

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
	DBGADAP("修补密码:%d\n", nRet);

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
	if(!Instance || !Instance->iReadBin)
		return -1;
	ISAPTSCANCARD;

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
	DBGADAP("修补密码:%d\n", nRet);

	return nRet;
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
		//memset(CurrRequest->value, 0, 100);

		// 过滤为空的代码: 如果为0xff,则跳过这个列
		if(bcd[0] == 0xff){
			goto Next;
		}

		nOffSet = (CurrRequest->offset - Agent->offset)/4%2;

		//add by yanggx	20110619
		////BCD转化成字符串 有问题， 将单个4位的BCD编码过滤掉了.
		if(CurrRequest->length == 4){
			if (nOffSet == 0){
				nOffSet = CurrRequest->offset/4%2;
			}
			Bcd2Ch(CurrRequest->value, bcd, nOffSet);	
		} else {
			//add by yanggx 20110706
			Bcd2StrOffSet(CurrRequest->value, bcd, (CurrRequest->length)/4, nOffSet);
			if (CurrRequest->offset == ID_OFFSET) {
				if (CurrRequest->value[(CurrRequest->length)/4-1] == 'D'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'd'){
					CurrRequest->value[(CurrRequest->length)/4-1] = 'x';

				}else if (CurrRequest->value[(CurrRequest->length)/4-1] == 'e'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'E') {

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
int _iReadCard(struct RWRequestS *list)
{
	unsigned char bRead = 0;

	struct RWRequestS *CurrRequest= NULL;

	/* 如果没有读卡设备接入*/
	if(!Instance) 
		return CardInitErr;

	//memset(defKeyA, 0xff, sizeof(defKeyA));
	CurrRequest = list;
	while(CurrRequest)
	{
		bRead = Instance->iReadBin(CARDSEAT_M1, defKeyA, CurrRequest->value, CurrRequest->length, 
			CurrRequest->offset);

		// 向后迭代
		CurrRequest = CurrRequest->Next;
	}

	return bRead==0 ? 0:CardReadErr;
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
		if(!strlen((char*)CurrRequest->value)){
			memset(CurrRequest->value, 0xff, (CurrRequest->length)/8);
		}

		//comment by yanggx	110612  按字符存取，将每个Agent的相关字符记录下来
		nByteLen = CurrRequest->length/4;
		if ((CurrRequest->offset - Agent->offset) == 0){
			bcd = Agent->value;

			//add by yanggx 20110616 判断开始位是否有数据
			nBitPos = CurrRequest->offset/4%2;
			if(nBitPos == 1){
				memset(bcd, 0xff, 1);
			}
			nAgentReLen = nByteLen + nBitPos;
		} else {
			bcd = Agent->value + nAgentReLen/2;
			nAgentReLen += nByteLen;
		}

		// 这里的长度应该是字符串的长度，不应为空间的长度 
		len = (int)strlen((char*)CurrRequest->value);
		nPos = (nAgentReLen - nByteLen)%2;

		for(i=0; i<len; ++i){

			//将字符转为BCD编码，保存在bcd中
			StrCh2Bcd(CurrRequest->value[i], bcd+(i+nPos)/2, (i+nPos)%2);
		}

		//add by yanggx 20110615
		//如果字符长度没有字节长度的4倍，后面加0xf
		nByteLen = CurrRequest->length/4;
		if (nByteLen > len) {
			for(; i<nByteLen; ++i)
			{
				StrCh2Bcd('f', bcd+(i+nPos)/2, (i+nPos)%2);
			}
		}
		CurrRequest = CurrRequest->Next;
	}

	return;
}

int _iWriteCard(struct RWRequestS *list)
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
			IsAllTheSameFlag(seed, 14, 0x30) == 0){
			memcpy(Key, keyNewB, sizeof(keyNewB));
		} else {
			//默认读写形式
			if (nWriteControl == DEFAULT_CONTROL){
				iGetKeyBySeed(seed, keyNewB);
				memcpy(Key, keyNewB, sizeof(keyNewB));
			} else {
				//KeyA有全部读写权限
				nWriteControl = KEYA_CONTROL;
				memcpy(Key, defKeyA, sizeof(defKeyA));
			}
		}

		ISAPTSCANCARD;

		CurrRequest = list;
		while(CurrRequest)
		{
			bool_test = Instance->iWriteBin(CARDSEAT_M1, Key, CurrRequest->value, nWriteControl , 
				CurrRequest->length, CurrRequest->offset);
			CurrRequest = CurrRequest->Next;
			if (bool_test) {
				LogPrinter("写卡失败，需要修补密码\n");
				faile_retry++;
				break;
			}
		}

		//对于keyb错误的写操作，修补密码
		if (bool_test) {
			bool_test = repairKeyBAllF(pControl);
			if (bool_test) {
				//修补农合卡KeyB错误
				bool_test = repairKeyB(pControl);
			}
			if (bool_test) {
				//修补新疆密码为0x75的错误
				bool_test = repairKeyForFault(pControl);
			}
			if (!bool_test) {
				DBGADAP( "修补密码成功，重新写卡\n");
				LogPrinter("修补密码成功，重新写卡\n");
				faile_retry = FAILE_RETRY-1;
			}
		} else{
			break;
		}
	}
	return bool_test==0 ? 0:CardWriteErr;
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
		memset(current->value, 0, (current->length + 7)/8);

		current = current->Next;
	}

	return ReadList;
}



int  M1ReadCard(struct RWRequestS *list, void *apt)
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
	apt_DestroyRWRequest(AgentList, 1);;

	return res;
}

int M1WriteCard(struct RWRequestS *list,  void *apt)
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
	apt_DestroyRWRequest(AgentList, 1);

	return res;
}

//M1卡初始化后的工作
static int	M1LastInit(void* data)
{
	return 0;
}

//为链表分配内存
static int	M1CallocForList(struct RWRequestS* request)
{
	return 0;
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

					if (IsAllTheSameFlag((unsigned char*)&pCur->ColumnHeader->Value,18, '0') == 0){
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
		ColumnElement = g_SegHelper->FindColumnByID(SegmentElement->ColumnHeader, pairCol.second);
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
		SegmentElement = g_SegHelper->FindSegmentByID(XmlListHead, ID);
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
			ColumnElement = g_SegHelper->FindColumnByID(SegmentElement->ColumnHeader, nColumnID);

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

	SegmentElement = g_SegHelper->FindSegmentByID(XmlListHead, 5);
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

static void M1CallocForColmn(struct XmlColumnS *result) 
{
	int len = result->CheckInfo.M1Info.ColumnBit;
	result->Value = (char*)malloc(len + 1);
	memset(result->Value, 0, len + 1);
}

static int __stdcall InitM1Adapter()
{
	M1Adapter.type = eM1Card;
	M1Adapter.iLastInit = M1LastInit;
	M1Adapter.iCallocForList = M1CallocForList;
	M1Adapter.iReadCard = M1ReadCard;
	M1Adapter.iWriteCard = M1WriteCard;
	return 0;
}

CardOps * __stdcall InitM1CardOps()
{
	InitM1Adapter();
	g_CardOps.iInitGList = InitM1GlobalList;
	g_CardOps.iConvertXmlByList = M1ConvertXmlByList;
	g_CardOps.iConvertXmltoList = M1ConvertXmltoList;
	g_CardOps.iCallocForColmn =  M1CallocForColmn;
	g_CardOps.cardAdapter = &M1Adapter;

	g_CardOps.iInitGList();
	g_CardOps.programXmlList = g_XmlListHead;

	g_SegHelper = new CSegmentHelper(g_XmlListHead, &g_CardOps); 
	return &g_CardOps;
}

void __stdcall M1clear()
{
	SAFE_DELETE(g_SegHelper);
	SAFE_DELETE_C(g_XmlListHead);
}
