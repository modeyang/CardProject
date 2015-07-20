#ifndef _CARD_H
#define _CARD_H
#include <assert.h>
#include <stdio.h>


#ifdef  __cplusplus
extern "C" {
#endif

#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)

#define CPU_ONLY	1
#define CPU_M1		0
#define CPU_8K		0
#define CPU_8K_ONLY	0
#define CPU_16K		0

// for cpu test
#define CPU_8K_TEST 0

// for cpu test all, combination with CPU_8K_ONLY flag
#define CPU_8K_TEST_ALL 0

#define ENCRYPT		0

#define	CARDSEAT_RF		0	//0：非接用户卡 
#define CARDSEAT_PSAM1	1	//1：SAM卡编号1
#define CARDSEAT_M1		2	//2：M1

#define LOG_STORE		1   // 0 file， 1 db 
#define CHECK_TYPE		1	// 0 sqlite3,  1 sqlserver

#define CMD_BEEP	1
#define CMD_LED		2

typedef unsigned char BYTE;

#define MAX_BUFF_SIZE   (1024 * 24)


#define SAFE_DELETE(a)  if (a != NULL) { delete(a);a = NULL;}
#define SAFE_DELETE_C(a)  if (a != NULL) { free(a);a = NULL;}

#define NR_MASK(nr) (1 << nr)
#define NOT_NR_MASK(nr) ~(1 << nr)

#define SETBIT(byte, nr) byte |= NR_MASK(nr)
#define CLRBIT(byte, nr) byte &= NOT_NR_MASK(nr)

#define SAFEARRAY_DELETE(a)  if (a != NULL) { delete [] a ;a = NULL;}

//CPU每个字段的类型
typedef enum eItemType
{
	eAnsType,
	eCnType,
	eBType,
}eItemType;

//CPU卡文件类型
typedef enum eFileType
{
	eBinType = 0,   
	eRecType,       
	eCycType,            //循环文件，利用appendRec
	eSureType            //定长文件,利用SignRec
}eFileType;



/**
* @ID 
* @MASK 
* @TYPE 
* @CHECK 
* @SOURCE 
* @TARGET
* @DEFAULT 
* @ISWRITE 
* @OFFSET 
* @COLUMNBIT 
* @INULLABLE= 
* @WRITEBACK
*/
typedef struct M1Type {
	char	Mask		:1;
	char	Type		:3;
	char	Check		:1;
	char	IsWrite		:1;
	char	INullLable	:1;
	char	WriteBack	:1;
	int		ColumnBit;
}M1Type;

typedef struct CpuInfo {
	eItemType	itemtype;
	int			ColumnByte;
}CpuInfo;

struct XmlColumnS
{
	int		ID;
	char	Source[32];

	union {
		M1Type   M1Info;
		CpuInfo  CpuInfo;
	}CheckInfo;

	int		Offset;
	char	*Value;
	struct XmlSegmentS  *parent;
	struct XmlColumnS	*Next;
};


/**
* @ID
* @TARGET
*/
struct XmlSegmentS
{
	int		ID;
	char	Target[30];

	//cpu
	int					offset;
	eFileType			datatype;

	struct XmlColumnS	*ColumnHeader;
	struct XmlColumnS   *ColumnTailer;
	struct XmlSegmentS	*Next;
};



/**
*@ID 
*@TARGET
*/
struct XmlProgramS
{
	int		ID;
	char	Target[50];

	struct XmlSegmentS		*SegHeader;
	struct XmlSegmentS		*SegTailer;
	struct XmlProgramS		*Next;
};
struct RWRequestS
{
	int	mode;	// 读写请求标志

	int offset;	// 绝对地址偏移量
	//int startID;	//开始记录块ID
	//int recCounts;  //记录条数
	int	length;	// 该元素的长度
	unsigned char *value;

	//cpu
	int nID;    // 编号
	int nColumID;  
	eFileType	datatype;
	eItemType   itemtype;

	struct RWRequestS *agent;	// 真实的读写代理

	void *pri;	

	// 下一个元素
	struct RWRequestS	*Next;
};

//初始化后的工作
typedef int					(*LastInit) (void*);

//加载xml，初始化链表
typedef int					(*InitGlobalList)();

//为链表分配内存
typedef int					(*CallocForList)(struct RWRequestS*);

typedef int					(*ConvertXmlByList) (struct XmlSegmentS *listHead, 
												 char *xml, 
												 int *length, 
												 int del_flag);

typedef struct XmlSegmentS* (*ConvertXmltoList) (char *xml);

typedef int                 (*ReadCard)(struct RWRequestS *list, void *apt);
typedef int                 (*WriteCard)(struct RWRequestS *list,  void *apt);

typedef void				(*CallocForColmn)(struct XmlColumnS *);

typedef enum CardType
{
	eM1Card = 0,
	eCPUCard,
	eCARDSEAT_PSAM1,
}CardType;

#define ARRAY_MAX 10

typedef struct adapter{
	CardType			type;
	LastInit			iLastInit;
	CallocForList		iCallocForList;
	ReadCard			iReadCard;
	WriteCard			iWriteCard;

	void				*parent;
} adapter;


typedef struct CardOps 
{
	InitGlobalList		iInitGList;
	ConvertXmlByList	iConvertXmlByList;
	ConvertXmltoList	iConvertXmltoList;
	CallocForColmn      iCallocForColmn;

	struct XmlProgramS *programXmlList;
	adapter		       *cardAdapter;

	void * SegmentHelper;
}CardOps;

void      CardUnregisterOps(int type);

void      CardRegisterOps(int type, CardOps *ops);

CardOps*  GetCardOps(int type);

int       IsAllTheSameFlag(const unsigned char *szBuf, int nLen, unsigned char cflag);

#ifdef  __cplusplus
};
#endif


#endif