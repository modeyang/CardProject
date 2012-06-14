
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>

#include "BHGX_MF_CPUVCard.h"

#define DEFAULT_FILENAME  "C:\\BHGX_MF_CPUVCard.data"
#define VCard_MAX_SIZE 8*1024

static int GOffset = 0;
static FILE *Handle = NULL;
#define REC_POS 1024
#define BIN_POS 0



/**
 * 制卡需要的关键信息
 */
struct CreateKeyInfoS
{
	int offset;
	int length;
	unsigned char ID[9];
	unsigned char token[6];
	unsigned char key[16];
};


/**
 *
 */
static void DbgMem(unsigned char *mem, int size)
{
	int i = 0;
	for(i=0; i< size; i++)
		printf("%02x ", mem[i]);
	printf("\n");
}

/**
* 初始化卡操作
*/
static void InitBinCard(void)
{
	unsigned char buf[1024];

	// 在Buf中初始化为oxff
	memset(buf, 0xff, 1024);

	rewind(Handle);
	fseek(Handle, BIN_POS, SEEK_SET);
	fwrite(buf, 1024, 1, Handle);
}

static void InitRecCard()
{
	unsigned char buf[1024];

	// 在Buf中初始化为oxff
	memset(buf, 0xff, 1024);
	buf[0]=1;
	buf[1]=4;
	buf[5]=2;
	buf[6]=1;
	buf[8]=3;
	buf[9]=1;
	buf[11]=4;
	buf[12]=4;
	buf[16]=1;
	buf[17]=1;

	rewind(Handle);
	fseek(Handle, REC_POS, SEEK_SET);
	fwrite(buf, 1024, 1, Handle);
}

/**
 * 虚拟文件探测文件，这里始终返回为1
 */
int __stdcall bProbe(void)
{
	/* 设置探测结果 */
	int res = 1;

	return res;
}

/**
 *
 */
static FILE * iOpenFile(const char *filename)
{
	FILE *res = NULL;

	//如果这个文件不存在
	if(_access(filename, 6))
	{
		fopen_s(&res, filename, "ab+");
		fclose(res);
	}

	// 打开文件
	fopen_s(&res, filename, "rb+");
	if(res != NULL)
	{
		rewind(res);
	}

	InitBinCard();
	InitRecCard();

	return res;
}

/**
 *
 */
int __stdcall iOpen(void)
{
	//把打开文件向后推延
	
	return 0;
}

/**
 *
 */
int __stdcall iClose(void)
{
	if(Handle)
		fclose(Handle);

	Handle = 0;

	return 0;
}


/**
 *
 */
static int ChangeVirtualFile(char *filename)
{
	//如果已经打开文件，则关闭文件句柄
	if(Handle != NULL)
	{
		fclose(Handle);
	}

	Handle = iOpenFile(filename);
	GOffset = 0;

	return 0;
}


/**
 *
 */
int __stdcall iIOCtl(int cmd, void *data, int len)
{
	int result = 0;


	switch(cmd)
	{
	case 0:
		InitRecCard();
		break;

	case 1:
		InitBinCard();
		break;

	case 2:
	case 3:
		ChangeVirtualFile((char *)data);
		break;

	default:
		break;
	}

	return result;
}




int __stdcall ICCSet(unsigned char iCardSeat, 
					   unsigned char *pCardType, 
					   unsigned char *pATR)
{
	*pCardType = 1;
	memcpy(pATR, "123", 4);
	return 0;
}



int __stdcall iGetRandom(unsigned char iCardType,
						   unsigned char iRndLen,
						   unsigned char *pOutRnd)
{
	return 0;
}


int __stdcall iSelectFile(unsigned char iCardType,
							unsigned char *pInFID)
{
	return 0;
}

int __stdcall iReadBin (unsigned char iCardType,
					  unsigned char *pFID, 
					  unsigned char *pBuf, 
					  int iSizeInBytes,
					  int iOffsetInBytes)
{
	int result;

	//如果文件不存在
	if(Handle == NULL)
		ChangeVirtualFile(DEFAULT_FILENAME);

	// 把文件放置到开头
	rewind(Handle);

	result = fseek(Handle, iOffsetInBytes + GOffset, SEEK_SET);
	result = fread(pBuf,  1, iSizeInBytes,Handle);

	//DbgMem(buf, iSizeInBytes);

	return 0;
}



int __stdcall iWriteBin(
					  unsigned char iCardType,
					  unsigned char *pFID, 
					  unsigned char *pBuf, 
					  int iSizeInBytes,
					  int iOffsetInBytes)
{
	// 如果文件不存在
	if(Handle == NULL)
		ChangeVirtualFile(DEFAULT_FILENAME);  

	// 把文件放置到开头
	rewind(Handle);

	fseek(Handle, iOffsetInBytes + GOffset, SEEK_SET);
	fwrite(pBuf, 1,iSizeInBytes,Handle);

	return 0;
}


int __stdcall iReadRec(
						unsigned char iCardType,
						 unsigned char *pFID, 
						 unsigned char *pBuf, 
						 unsigned long buflen,
						 int iRecNo,
						 int iRecCount)
{	
	int result;

	//如果文件不存在
	if(Handle == NULL)
	  ChangeVirtualFile(DEFAULT_FILENAME);

	// 把文件放置到开头
	rewind(Handle);
	fseek(Handle, REC_POS, SEEK_SET);



	return 0;

}

// 写设备数据
int __stdcall  iWriteRec(
					   unsigned char iCardType,
					   unsigned char *pFID, 
					   unsigned char *pBuf, 
					   unsigned long buflen,
					   int iRecNo,
					   int iRecCount)
{
	// 如果文件不存在
	if(Handle == NULL)
		ChangeVirtualFile(DEFAULT_FILENAME);  

	// 把文件放置到开头
	rewind(Handle);

	return 0;
}