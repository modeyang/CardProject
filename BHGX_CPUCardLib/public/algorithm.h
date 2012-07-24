/**
 *
 */
#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#ifndef NULL
#define NULL 0
#endif    

#ifdef __cplusplus
extern "C" {
#endif
	void DbgMem(unsigned char *mem, int len);

	int BinToHexstr(unsigned char *HexStr,unsigned char *Bin,int  BinLen);

	int HexstrToBin(unsigned char *bin, unsigned char *asc,int len);

	int trimRightF(unsigned char *bin, int len); 

#ifdef __cplusplus
}
#endif

#endif	//_ALGORITHM_H_