#pragma once
#ifndef _VIRT_CARD_H_
#define _VIRT_CARD_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef  __cplusplus
extern "C" {
#endif
	// 探测自己设备
	DLL_EXPORT int __stdcall bProbe(void);

	// 打开设备
	DLL_EXPORT int __stdcall iOpen(void);

	// 关闭设备
	DLL_EXPORT int __stdcall iClose(void);

	// 卡扫描数据
	DLL_EXPORT int __stdcall iScanCard(void);

	// 设备控制数据
	DLL_EXPORT int __stdcall iIOCtl(int cmd, void *);

	// 读设备数据
	//DLL_EXPORT int iRead(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall iRead(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

	// 写设备数据
	//DLL_EXPORT int iWrite(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall iWrite(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

#ifdef  __cplusplus
};
#endif

#endif	// _VIRT_CARD_H_
