#ifndef _UTIL_H
#define _UTIL_H
#pragma once

typedef unsigned char BYTE;

typedef int (*ReadCard)();
typedef int (*WriteCard)();
enum CardDataType
{
	eStart = 0,
	eProviderInfo,
	eCardHolderInfo,
	eAddr,
	eContacter,
	eSocialInfo,
	eCertificat,
	eClinical,
	eSpecialInfo,
	eAllergyInfo,
	eImmuneInfo,
	eHosindex,
	eServiceIndex,

	eSegmentHosServ,
};
#pragma  pack(push, 1)

//发卡机构信息
struct ProviderInfo 
{
	BYTE cardType;		//卡类型
	BYTE version[4];	//规范版本
	BYTE provider[30];
	BYTE provCode[11];
	BYTE provCert[180];
	int  sendTime;
	int  holdlast;
	BYTE cardno[18];
	BYTE safecode[3];
	BYTE chipseq[10];
};

//持卡人信息
struct CardHolderInfo
{
	BYTE name[30];
	BYTE sex;
	BYTE nationcode;
	int  birthday;
	BYTE identity[18];
	BYTE phone1[20];
	BYTE phone2[20];
	BYTE feecharge[3];
};

//连续2个地址
struct Addr
{
	BYTE addrType;
	BYTE addrInfo[100];
};

//3个联系人
struct Contacter
{
	BYTE name[30];
	BYTE relation;
	BYTE phone[20];
};

//职业、婚姻信息文件
struct SocialInfo
{
	BYTE degree;
	BYTE married;
	BYTE employecode[3];
};

//证件记录信息
struct Certificat
{
	BYTE type;
	BYTE certno[18];
	BYTE healthno[17];
	BYTE farmno[18];
};

//临床基本数据
struct Clinical
{
	BYTE ABOCode;
	BYTE RHCode;
	BYTE xiaoCode;
	BYTE heartCode;
	BYTE headCode;
	BYTE dianCode;
	BYTE bloodCode;
	BYTE sweetCode;
	BYTE greeneyeCode;
	BYTE touxiCode;
	BYTE organtransCode;
	BYTE organlackCode;
	BYTE loadCode;
	BYTE heartpaceCode;
	BYTE others[40];
};

struct SpecialInfo
{
	BYTE mentalflag;
};

//循环3条
struct AllergyInfo
{
	BYTE name[20];
	BYTE Desc[100];
};

//循环10条
struct ImmuneInfo
{
	BYTE name[20];
	int  time;
};

//定长5记录
struct Hosindex
{
	BYTE valideflag;
};

//定长5记录
struct ServiceIndex
{
	BYTE valideflag;
};

struct XmlColumnS
{
	int		ID;
	char	Source[30];

	int		Offset;
	int		ColumnByte;
	BYTE	*value;

	struct XmlColumnS	*Next;
};

struct XmlSegmentS
{
	int		ID;
	int		datatype;	//1:记录文件 0:二进制文件
	int		offset;
	BYTE	Target[30];

	struct XmlColumnS	*ColumnHeader;
	struct XmlColumnS   *ColumnTailer;
	struct XmlSegmentS	*Next;
};

struct XmlProgramS
{
	struct XmlSegmentS *SegHeader;
	struct XmlSegmentS *SegTailer;
};

struct SegmentHosServ
{
	int		ID;
	char	Source[30];

	struct XmlColumnS	*ColumnHeader;
	struct XmlColumnS   *ColumnTailer;
	struct SegmentHosServ	*Next;
};






struct RWRequestS
{
	int	mode;	// 读写请求标志]
	int nID;
	int	datatype;	//1:记录文件 0:二进制文件

	int offset;	// 绝对地址偏移量
	int	length;	// 该元素的长度
	unsigned char *value;

	struct RWRequestS *agent;	// 真实的读写代理

	void *pri;	

	// 对该请求的数据进行加解密
	//void encode(struct RWRequest *);
	//void decode(struct RWRequest *);

	// 下一个元素
	struct RWRequestS	*Next;
};

//对于记录文件的描述
struct RecFolder{
	BYTE	section[10];
	BYTE    subSection[10];
	BYTE	fileName[10];
};

#pragma pack(pop)

#ifdef  __cplusplus
extern "C" {
#endif

int __stdcall CreateCPUData(char *configXML);
char* ReadConfigFromReg();

struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement);

struct XmlSegmentS *GetXmlSegmentByFlag(int nID);

void DestroyList(struct XmlSegmentS *node);

struct RWRequestS* __stdcall CreateRequest(struct XmlSegmentS *listHead, int mode);
struct RWRequestS* __stdcall CreateRWRequest(struct XmlColumnS *listHead, int mode);

void __stdcall DestroyRequest(struct RWRequestS *list, int flag);
void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag);

int CheckCardXMLValid(char *pszCardXml);

int iConvertXmlByList(struct XmlSegmentS*, char *, int *);
struct XmlSegmentS* ConvertXmltoList(char *xml);

#ifdef  __cplusplus
};
#endif

#endif