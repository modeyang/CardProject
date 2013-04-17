/**
 *
 */
#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_
#include "../Card.h"

#ifndef NULL
#define NULL 0
#endif    

#ifdef __cplusplus
extern "C" {
#endif
	void DbgMem(unsigned char *mem, int len);

	int Str2Bcd(const unsigned char *str, unsigned  char *bcd, int *len);
	int Bcd2Str(unsigned char *str, const unsigned char *bcd, int len);
	int Bcd2Ch(unsigned char *str, const unsigned char *bcd, int nOffSet);
	int Bcd2StrOffSet(unsigned char *str ,const unsigned char *bcd, 
					  int len, int nOffset);
	int StrCh2Bcd(unsigned char ch, unsigned  char *bcd, int rem);

	int iCleanStr(char *str);
	int CleanName(char *name);

	int iGetKeyBySeed(const unsigned char *seed, unsigned char *key);

	int BinToHexstr(BYTE *HexStr, BYTE *Bin,int  BinLen);

	int HexstrToBin(BYTE *bin,  BYTE *asc,int len);

	int trimRightF(unsigned char *bin, int len); 

	int clearFF(unsigned char *bin, int len);

#ifdef __cplusplus
}
#endif

#endif	//_ALGORITHM_H_