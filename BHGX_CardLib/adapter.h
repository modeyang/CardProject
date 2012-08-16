#pragma once

#ifndef _CARD_LIB_ADAPTER_H_
#define _CARD_LIB_ADAPTER_H_

    
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

	struct RWRequestS * __stdcall CreateRWRequest(struct XmlSegmentS *listHead, int mode);
	void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag);

	int __stdcall iCtlCard(int cmd, void *data);

	int __stdcall iReadCard(struct RWRequestS *list);
	int __stdcall iWriteCard(struct RWRequestS *list);

	int __stdcall aChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
		const unsigned char * pNewKeyB,const unsigned char * poldPin ,
		unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag);

	int __stdcall aFormatCard(unsigned char *pControl, unsigned char* szFormat, int nBlk ,unsigned char *keyB);

	int __stdcall IsAllTheSameFlag(unsigned char *szBuf, int nLen, unsigned char cflag);

	int __stdcall aGetControlBuff(unsigned char *pControl, int nSecr);
	int __stdcall GetWriteWord(const unsigned char *pControl);

	int __stdcall repairKeyB(unsigned char *ctrlword);
	int  __stdcall repairKeyForFault(unsigned char *ctrlword);

#ifdef  __cplusplus
};
#endif

#endif	// _CARD_LIB_ADAPTER_H_