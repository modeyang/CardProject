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

	int Str2Bcd(const char *str, unsigned  char *bcd, int *len);
	int Bcd2Str(char *str, const unsigned char *bcd, int len);
	int Bcd2Ch(char *str, const unsigned char *bcd, int nOffSet);
	int Bcd2StrOffSet(char *str ,const unsigned char *bcd, int len, int nOffset);
	int StrCh2Bcd(unsigned char ch, unsigned  char *bcd, int rem);

	int iCleanStr(char *str);
	int CleanName(char *name);

	int iGetKeyBySeed(const unsigned char *seed, unsigned char *key);
#ifdef __cplusplus
}
#endif

#endif	//_ALGORITHM_H_