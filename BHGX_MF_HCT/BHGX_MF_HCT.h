#pragma once

#ifndef _DEVICE_BHGX_MIFARE_H_
#define _DEVICE_BHGX_MIFARE_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef  __cplusplus
extern "C" {
#endif
	// 探测自己设备
	DLL_EXPORT  int __stdcall bProbe(void);

	// 打开设备
	DLL_EXPORT int __stdcall iOpen(void);

	// 关闭设备
	DLL_EXPORT int __stdcall iClose(void);

	// 卡扫描数据
	DLL_EXPORT int __stdcall iScanCard(void);

	// 设备控制数据
	DLL_EXPORT int __stdcall iIOCtl(int cmd, void *);

	// 读设备数据
	DLL_EXPORT int __stdcall iRead(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

	// 写设备数据
	DLL_EXPORT int __stdcall iWrite(
		const unsigned char *key, 
		unsigned char *buf,
		int nCtrlWord,
		int iSizeInBits, 
		int iOffsetInBits
		);



	//修改用户权限，必须输入新keya 和 keyb 的值。
	DLL_EXPORT int  __stdcall iChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
		const unsigned char * pNewKeyB,const unsigned char * poldPin,unsigned char nsector
		,unsigned char keyA1B0,unsigned char changeflag);



#ifdef  __cplusplus
};
#endif

#endif	// _DEVICE_MIFARE_H