#pragma once

#ifndef _CARD_LIB_ADAPTER_H_
#define _CARD_LIB_ADAPTER_H_


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
struct XmlColumnS
{
	int		ID;
	char	Source[50];
	char	Target[50];
	char	Value[100];
	char	Mask		:1;
	char	Type		:3;
	char	Check		:1;
	char	IsWrite		:1;
	char	INullLable	:1;
	char	WriteBack	:1;

	int		Offset;
	int		ColumnBit;

	struct XmlColumnS	*Next;
};

/**
* @ID
* @TARGET
*/
struct XmlSegmentS
{
	int		ID;
	char	Target[50];

	struct XmlColumnS	*Column;
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

	struct XmlSegmentS		*Segment;
	struct XmlProgramS		*Next;
};
struct RWRequestS
{
	int	mode;	// 读写请求标志

	int offset;	// 绝对地址偏移量
	int	length;	// 该元素的长度
	unsigned char *value;

	struct RWRequestS *agent;	// 真实的读写代理

	void *pri;	

	// 下一个元素
	struct RWRequestS	*Next;
};


#ifdef  __cplusplus
extern "C" {
#endif

	int __stdcall initCoreDevice(const char *System);
	int __stdcall closeCoreDevice(void);

	int __stdcall opendev(void);
	int __stdcall closedev(void);

	int __stdcall iCoreFindCard(void);
	int __stdcall authUCard(const char *System);

	struct RWRequestS * __stdcall CreateRWRequest(struct XmlSegmentS *listHead, int mode);
	void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag);

	int __stdcall iCtlCard(int cmd, void *data);

	int __stdcall iReadCard(struct RWRequestS *list);
	int __stdcall iWriteCard(struct RWRequestS *list);
	int __stdcall InitPwd(unsigned char *newKeyB);

	int __stdcall aFormatCard(unsigned char cFlag);

	int __stdcall IsAllTheSameFlag(const unsigned char *szBuf, int nLen, unsigned char cflag);

#ifdef  __cplusplus
};
#endif

#endif	// _CARD_LIB_ADAPTER_H_