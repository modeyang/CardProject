/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algorithm.h"

#pragma warning (disable : 4996)
/**
 *
 */

void DbgMem(unsigned char *mem, int len)
{
	int i = 0;
	for(i=0; i<len; i++)
		printf("%02x ",mem[i]);

	printf("\n");
	
	return;
}

int BinToHexstr(unsigned char *HexStr,unsigned char *Bin,int  BinLen)
{
	char Temp1[3];
	char *result;
	int ret, i;
	int lens;
	lens=BinLen;
	if (lens<=0) 
		return 0;

	result=(char *)malloc((lens*2+2)*sizeof( char));
	if (!result) return 0;
	memset(result,0,sizeof(result));
	for (i=0;i<lens;i++)
	{
		memset(Temp1,0,sizeof(Temp1));
		ret=sprintf(Temp1,"%X",Bin[i]);
		if (strlen(Temp1)==1) {
			Temp1[1]=Temp1[0];
			Temp1[0]='0';
		}
		strcat(result,Temp1);
	}
	strcpy((char *)HexStr,result);

	free(result);
	return 2*i;
}

int HexstrToBin(unsigned char *bin, unsigned char *asc,int len)
{
	char ucChar;

	//len=len/2+len%2;
	//npDest=(char *)bin;
	while(len--){
		ucChar=(*asc<='9'&&*asc>='0')?*asc-'0':*asc-'A'+10;
		ucChar<<=4;
		asc++;
		ucChar|=((*asc<='9'&&*asc>='0')?*asc-'0':*asc-'A'+10)&0x0F;
		asc++;
		len--;
		*bin++=ucChar;
	}
	return len;
}

int trimRightF(unsigned char *bin, int len)
{
	int i=0;
	for (; i<len; ++i)
	{
		if (bin[i] == 'f' || bin[i] == 'F')
			bin[i] = 0;
	}
	return i;
}