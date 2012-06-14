#pragma once
#ifndef _CPUVIRT_CARD_H_
#define _CPUVIRT_CARD_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef  __cplusplus
extern "C" {
#endif

	DLL_EXPORT int __stdcall iOpen(void);

	DLL_EXPORT int __stdcall bProbe(void);

	DLL_EXPORT int __stdcall iClose(void);

	DLL_EXPORT int __stdcall iIOCtl(int cmd, void *data,int datasize);

	DLL_EXPORT int __stdcall ICCSet(unsigned char iCardSeat, 
		unsigned char *pCardType, 
		unsigned char *pATR);

	DLL_EXPORT int __stdcall iGetRandom(unsigned char iCardType,
		unsigned char iRndLen,
		unsigned char *pOutRnd);


	DLL_EXPORT int __stdcall iSelectFile(unsigned char iCardType,
		unsigned char *pInFID);

	DLL_EXPORT int __stdcall iReadBin (     unsigned char iCardType,
		unsigned char *pFID, 
		unsigned char *pBuf, 
		int iSizeInBytes,
		int iOffsetInBytes);



	DLL_EXPORT int __stdcall iWriteBin(
		unsigned char iCardType,
		unsigned char *pFID, 
		unsigned char *pBuf, 
		int iSizeInBytes,
		int iOffsetInBytes);

	// 读设备数据
	//DLL_EXPORT int iRead(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall iReadRec(
		unsigned char iCardType,
		unsigned char *pFID, 
		unsigned char *pBuf, 
		unsigned long buflen,
		int iRecNo,
		int iRecCount);

	// 写设备数据
	//DLL_EXPORT int iWrite(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall  iWriteRec(
		unsigned char iCardType,
		unsigned char *pFID, 
		unsigned char *pBuf, 
		unsigned long buflen,
		int iRecNo,
		int iRecCount);

#ifdef  __cplusplus
};
#endif


#endif	// _VIRT_CARD_H_
