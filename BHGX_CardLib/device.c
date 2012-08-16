#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "device.h"
#include "public/debug.h"
#pragma warning (disable : 4996)
#pragma warning (disable : 4020)
#pragma warning (disable : 4013)

#define MAX_RESULT 256


/**
 *
 */
static char **EnumDLL( __in const char *directory, __out int *count)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	TCHAR tStringArray[MAX_RESULT][MAX_PATH];
	TCHAR **result;
	TCHAR Pattern[MAX_PATH];
	int i = 0, j = 0;

	// 开始查找
	lstrcpy(Pattern, directory);
	lstrcat(Pattern,"*.dll");
	hFind = FindFirstFile(Pattern, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		*count = 0;
		return NULL;
	}
	else 
	{
		do 
		{
			lstrcpy(tStringArray[i++], FindFileData.cFileName);
		} while(FindNextFile(hFind, &FindFileData) != 0);
	}

	// 查找结束
	FindClose(hFind);

	// 复制到结果中
	result = (char **)malloc(i, sizeof(char *));
	for (j = 0; j < i; j++)
	{
		result[j] = (char *)malloc(MAX_PATH, sizeof(char));
		lstrcpy(result[j], tStringArray[j]);
	}

	*count = i;
	return result;
}


/**
 *
 */
static struct CardDevice *_InitDevice(HINSTANCE hInstLibrary)
{
	struct CardDevice *result = NULL;
	
	// 分配CardDevice数据结构空间
	result = (struct CardDevice *)malloc(sizeof(struct CardDevice));

	// 初始化句柄函数
	result->hInstLibrary = hInstLibrary;

	// 初始化设备结构的回调函数
	result->iProbe = (DllProbe)GetProcAddress(hInstLibrary, "bProbe");
	result->iOpen = (DllOpen)GetProcAddress(hInstLibrary, "iOpen");
	result->iClose = (DllClose)GetProcAddress(hInstLibrary, "iClose");
	result->iScanCard = (DllScanCard)GetProcAddress(hInstLibrary,"iScanCard");
	result->iIOCtl = (DLLIOCtl)GetProcAddress(hInstLibrary, "iIOCtl");
	result->iRead = (DllRead)GetProcAddress(hInstLibrary, "iRead");
	result->iWrite = (DllWrite)GetProcAddress(hInstLibrary, "iWrite");
	result->iChangePwdEx = (DLLChangePwdEx)GetProcAddress(hInstLibrary, "iChangePwdEx");
	return result;
}


/**
 * 函数: getCardDevice 
 * 参数： 
 *
 * 返回值:
 * 成功： 返回抽象卡设备
 * 失败： NULL
 */
struct CardDevice *getCardDevice(const char *System)
{
	DllProbe FunProbe;
	struct CardDevice *result = NULL;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char Pattern[MAX_PATH];
	int nProbe = 0;

	// 开始查找
	strcpy(Pattern, System);
	strcat(Pattern, "BHGX_MF_*.dll");
	hFind = FindFirstFile(Pattern, &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		HINSTANCE hInstLibrary;
		hInstLibrary = LoadLibrary(FindFileData.cFileName);
		if (hInstLibrary != NULL)
		{
			FunProbe = (DllProbe)GetProcAddress(hInstLibrary, "bProbe");
			printf("%s:Probe函数地址:%ll\n", FindFileData.cFileName, &FunProbe);
			nProbe = FunProbe();
			printf("bProbe:%d\n",nProbe);
			DBG(0, "%s bProbe:%d\n", FindFileData.cFileName,nProbe);
			if((FunProbe != NULL) && nProbe)
			{
				result = _InitDevice(hInstLibrary);
				break;
			}
			FreeLibrary(hInstLibrary);
		}
		if (0 == FindNextFile(hFind, &FindFileData))
			break;
	}
	FindClose(hFind);

	///**
	// * 如果设备找到，就打开并且嗡鸣一下
	// */
	if(result)
	{
		int ret = result->iOpen();
	}
	return result;
}


/**
 * 函数：putCardDevice
 * 参数：
 * @device: 卡设备
 *
 * 返回值：
 * 成功： 0
 * 失败： 非零
 */
int putCardDevice(struct CardDevice *device)
{
	int result = 0;

	if(device != NULL)
	{
		device->iClose();
		FreeLibrary(device->hInstLibrary);
		free(device);
	}

	return result;
}