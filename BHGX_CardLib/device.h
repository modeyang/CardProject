#pragma once

#ifndef ACC_DEVICE_H
#define ACC_DEVICE_H


#include <windows.h>
     
/**
 *
 */
enum {
	CMD_CHANGCARD,
	CMD_SET_READKEY,
	CMD_GET_READKEY,
	CMD_SET_WRITEKEY,
	CMD_GET_WRITEKEY,
	CMD_MAX
};

typedef int (__stdcall *DllProbe)(void);
typedef int (__stdcall *DllOpen)(void);
typedef int (__stdcall *DllClose)(void);

typedef BOOL (__stdcall *DllAuthUDev)(void);
typedef int (__stdcall *DllScanCard)(void);
typedef int (__stdcall *DLLIOCtl)(int, void *);
typedef int (__stdcall *DllRead)(const unsigned char *, unsigned char *, int, int);
typedef int (__stdcall *DllWrite)(const unsigned char *, unsigned char *, int ,int, int);
typedef unsigned char (__stdcall *DLLChangePwdEx)(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
												  const unsigned char * pNewKeyB,const unsigned char * poldPin ,
												  unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag);



/**
 * strurct 
 *
 * @name			卡设备名称
 * @hInstLibrary	打开动态库句柄
 *
 * @iProbe			探测设备函数
 * @iOpen			打开设备函数
 * @iClose			关闭设备函数
 * @iScanCard		扫描卡函数
 * @iRead			设备读函数
 * @iWrite			设备写函数
 */
struct CardDevice
{
	HINSTANCE	hInstLibrary;

	DllProbe	iProbe;
	DllOpen		iOpen;
	DllClose	iClose;
	DllScanCard	iScanCard;
	DLLIOCtl	iIOCtl;
	DllRead		iRead;
	DllWrite	iWrite;
	DLLChangePwdEx  iChangePwdEx;
};


struct CardAuth 
{
	HINSTANCE	hAuthLibrary;
	DllAuthUDev iAuthUDev;
};
/*
 * 成功： 返回抽象卡设备
 * 失败： NULL
 */
struct CardDevice* getCardDevice(const char *System);

/**
 * Func: putCardDevice
 * @device 输入的卡设备
 *
 *Return:
 * 成功： 0
 * 失败： 非零
 */
int putCardDevice(struct CardDevice *device);


//U盾验证
int authUDev(const char *System);


#endif	//ACC_DEVICE_H