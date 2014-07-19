#include "stdafx.h"
#include <stdio.h>

#include "ConstDesc.h"
#include "BHGX_MF_P330i.h"
#include "debug.h"


struct ZBRHandler 
{
	HINSTANCE	hInstLibrary;


	ZBRGetHandle		getHandle;
	ZBRCloseHandle		closeHandle;
	ZBRPRNEjectCard		ejectCard;
	ZBRPRNSendCmd		sendCmd;
	ZBRPRNSendCmdEx		sendCmdEx;
	ZBRPRNGetPrinterStatus	getPrinterStatus;

	//Sets the magnetic encoding direction. 0 = forward ,1 = reverse
	ZBRPRNSetEncodingDir	setEncodingDir;
	ZBRPRNResetMagEncoder	resetMagEncoder;
	ZBRPRNSetMagEncodingStd	setEncodingStd;

	ZBRPRNReadMag			readMag;
	ZBRPRNReadMagByTrk		readMagByTrk;
	ZBRPRNWriteMag			WriteMag;
	ZBRPRNWriteMagByTrk		WriteMagByTrk;
	ZBRPRNWriteMagPassThru	WriteMagPassThru;
};

ZBRHandler g_ZBRHandler;

static int getFunc() 
{
	HINSTANCE hInstance = LoadLibrary("ZBRPrinter.dll");
	if (hInstance == NULL){
		return -1;
	}

	g_ZBRHandler.getHandle = (ZBRGetHandle)GetProcAddress(hInstance, "ZBRGetHandle");
	g_ZBRHandler.closeHandle = (ZBRCloseHandle)GetProcAddress(hInstance, "ZBRCloseHandle");
	g_ZBRHandler.sendCmdEx = (ZBRPRNSendCmdEx)GetProcAddress(hInstance, "ZBRPRNSendCmdEx");
	g_ZBRHandler.sendCmd = (ZBRPRNSendCmd)GetProcAddress(hInstance, "ZBRPRNSendCmd");
	g_ZBRHandler.ejectCard = (ZBRPRNEjectCard)GetProcAddress(hInstance, "ZBRPRNEjectCard");
	g_ZBRHandler.getPrinterStatus = (ZBRPRNGetPrinterStatus)GetProcAddress(hInstance, "ZBRPRNGetPrinterStatus");

	g_ZBRHandler.setEncodingDir = (ZBRPRNSetEncodingDir)GetProcAddress(hInstance, "ZBRPRNSetEncodingDir");
	g_ZBRHandler.setEncodingStd = (ZBRPRNSetMagEncodingStd)GetProcAddress(hInstance, "ZBRPRNSetMagEncodingStd");
	g_ZBRHandler.resetMagEncoder = (ZBRPRNResetMagEncoder)GetProcAddress(hInstance, "ZBRPRNResetMagEncoder");

	g_ZBRHandler.WriteMagByTrk = (ZBRPRNWriteMagByTrk)GetProcAddress(hInstance, "ZBRPRNWriteMagByTrk");
	g_ZBRHandler.readMagByTrk = (ZBRPRNReadMagByTrk)GetProcAddress(hInstance, "ZBRPRNReadMagByTrk");
	g_ZBRHandler.WriteMagPassThru = (ZBRPRNWriteMagPassThru)GetProcAddress(hInstance, "ZBRPRNWriteMagPassThru");
	g_ZBRHandler.WriteMag = (ZBRPRNWriteMag)GetProcAddress(hInstance, "ZBRPRNWriteMag");
	g_ZBRHandler.readMag = (ZBRPRNReadMag)GetProcAddress(hInstance, "ZBRPRNReadMag");

	g_ZBRHandler.hInstLibrary = hInstance;
	return 0;
}



static HANDLE mGHandle = NULL;
static int printerType = 0;

#define NR_MASK(nr) (1 << nr)
#define NOT_NR_MASK(nr) ~(1 << nr)

#define SETBIT(byte, nr) byte |= NR_MASK(nr)
#define CLRBIT(byte, nr) byte &= NOT_NR_MASK(nr)

static int SetBits(unsigned char *bits, const unsigned char *value, int offset, int len )
{
	int si, sr, di, dr;
	int i, j, k=0;
	for(i = 0; i < len ;) 
	{
		si = (offset + i) / 8;
		sr = (offset + i) % 8;

		dr = offset%8;
		di = (i+dr) / 8;
		for (j=sr; j<8 && k<len; ++j, ++k)
		{	
			if(value[di] & NR_MASK((7-sr)))
				SETBIT(bits[si], (7-j));
			else 
				CLRBIT(bits[si], (7-j));
			++sr;
			sr %= 8;
		}
		i = k;
	}
	return i;
}


static int GetBits(const unsigned char *bits, unsigned char *value, int offset, int len)
{
	int si, sr, di, dr;
	int i, j, k=0;
	for(i = 0; i < len ;) 
	{
		si = (offset + i) / 8;
		sr = (offset + i) % 8;

		di = i / 8;
		dr = i % 8;
		for (j=sr; j<8 && k<len; ++j, ++k)
		{	
			if(bits[si] & NR_MASK((7-j)))
				SETBIT(value[di], (7-j));
			else 
				CLRBIT(value[di], (7-j));
		}
		i = k;
	}
	return i;
}

/**
*
*/
static int str2bcd(const char *str, unsigned  char *bcd, int len)
{
	int i, j;

	for(i = 0; i < len; i++) 
	{		
		j = i >> 1;	

		if(i & 1) 
		{
			bcd[j] |= (str[i] - 48) << 4;
		}
		else
		{
			bcd[j] = str[i] - 48;
		}
	}

	return i;
}

/**
*
*/
static int bcd2str(const unsigned char *bcd, char *str, int len)
{	
	int i, j;

	for(i=0; i < len; i++) 
	{
		j = i >> 1;
		if(i & 1) 
		{
			str[i] = (bcd[j] >> 4) + 48;
		}	
		else 
		{
			str[i] = (bcd[j] & 0x0f) + 48;
		}
	}

	return i;
}


/**
*
*/
static void dbgmem(unsigned char *begin, int len)
{
	int i = 0;
	for(i=0; i<len; i++)
		printf("%02x ",begin[i]);

	printf("\n");

	return;
}


/**
*
*/
int __stdcall bProbe(void)
{
	int err, res;
	res = getFunc();
	if (res == 0) {
		res = g_ZBRHandler.getHandle(&mGHandle, "Zebra P330i USB Card Printer", &printerType, &err);
		//res = g_ZBRHandler.getPrinterStatus(&err);
	}

	return err != 0;
}

/**
*
*/
int __stdcall iOpen(void)
{
	int res = 0, err=0;
	res = g_ZBRHandler.getPrinterStatus(&err);
	return res == 0;
}

/**
*
*/
int __stdcall iClose(void)
{
	int err;
	g_ZBRHandler.ejectCard(mGHandle, printerType, &err);
	g_ZBRHandler.closeHandle(mGHandle, &err);
	return 0;
}

/**
*
*/
int __stdcall iScanCard(void)
{
	int err;
	g_ZBRHandler.sendCmd(mGHandle, printerType, "V", &err);
	return err != 0;
}


/**
*
*/
int __stdcall iIOCtl(int cmd, void *data)
{
	int result = 0;

	switch(cmd)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		//SetKey((struct CreateKeyInfoS *)data);
		break;
	case 3:
		break;
	default:
		break;
	}

	return result;
}

/**
*
*/
int __stdcall iRead(const unsigned char *key,unsigned char *buf, int iSizeInBits, int iOffsetInBits)
{
	return 0;
}



/**
*
*/
int __stdcall iWrite(const unsigned char *key, unsigned char *buf,
					 int nCtrlWord,int iSizeInBits, int iOffsetInBits)
{

	int result = 0;
	return result;
}


//修改用户权限，必须输入新keya 和 keyb 的值。
int  __stdcall iChangePwdEx(
							const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
							const unsigned char * pNewKeyB,const unsigned char * poldPin,
							unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag
							)
{
	return 0;
}
