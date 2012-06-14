
#ifndef _CARDDATA_ALGORITHM_H
#define _CARDDATA_ALGORITHM_H



#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void DbgMem(unsigned char *mem, int len);

	int Str2Bcd(const char *str, unsigned  char *bcd, int *len);
	int Bcd2Str(char *str, const unsigned char *bcd, int len);

	int iGetKeyBySeed(const  unsigned char *seed, unsigned char *key);
#ifdef __cplusplus
}
#endif

#endif	//_ALGORITHM_H_