/**
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algorithm.h"

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


int StrCh2Bcd(unsigned char ch, unsigned  char *bcd, int rem)
{
	if((ch >= '0') && (ch <= '9')) 
	{
		ch = ch - '0';
	}
	else if((ch >= 'a') && (ch <= 'f'))
	{
		ch = ch - 'a' + 0x0a;
	}
	else if (ch == 0x2e) //添加'.'
	{
		ch = 0xfe;
	}
	else if (ch == 'x' || ch == 'X')
	{
		ch = (ch == 'x')? 0 : 1;
		ch += 0xd;
	}
	else
	{
		ch = ch - 'A' + 0x0a;
	}

	if(rem)
	{
		*bcd &= 0xf0;
		*bcd |= ch;
	}
	else
	{
		*bcd = ch << 4 | 0xf; 
	}
	return rem;
}

/**
 *
 */
int Str2Bcd(const char *str, unsigned  char *bcd, int *len)
{
	unsigned char low, high;
	int rem;
	int slen;
	int i, j;

	// 检验输入的值
	if((str == NULL) || (bcd == NULL) || (len == 0))
		return -1;

	// 校验长度问题
	slen = *len;
	rem = slen%2;
	slen -= rem;
	if(slen == 0 && rem == 0)
		return -1;

	for(i=0, j=0; i<slen/2; i++,j+=2)
	{
		high = str[j];
		low  = str[j + 1];
		if((high >= '0') && (high <= '9')) 
		{
			high = high - '0';
		}
		else if((high >= 'a') && (high <= 'z'))
		{
			high = high - 'a' + 0x0a;
		}
		else {
			high = high - 'A' + 0x0a;
		}

		if((low >= '0') && (low <= '9')) 
		{
			low = low - '0';
		}
		else if((low >= 'a') && (low <= 'z'))
		{
			low = low - 'a' + 0x0a;
		}
		else {
			low = low - 'A' + 0x0a;
		}

		bcd[i] = high << 4 | low;
	}

	if(rem)
	{
		high = str[j];

		if((high >= '0') && (high <= '9')) 
		{
			high = high - '0';
		}
		else if((high >= 'a') && (high <= 'z'))
		{
			high = high - 'a' + 0x0a;
		}
		else {
			high = high - 'A' + 0x0a;
		}

		bcd[i] = high << 4 | 0xf; 
	}


	//返回长度值
	if(len) {
		 *len=(slen/2 + rem);
	}
	 return 0;
}

int Bcd2Ch(char *str, const unsigned char *bcd, int nOffSet)
{
	if((str==NULL) || (bcd==NULL))
		return -1;
	if(nOffSet == 0)
	{
		str[0]= (bcd[0]>>4) > 9 ? (bcd[0]>>4)-10+'A' : (bcd[0]>>4)+'0';
	}
	else
	{
		str[0]=(bcd[0] & 0x0F) >9 ? (bcd[0] & 0x0F)-10+'A' : (bcd[0] & 0x0F)+'0';
	}
	return nOffSet;
}
/**
 *
 */
int Bcd2Str(char *str, const unsigned char *bcd, int len)
{	
	int i,j;

	// 验证传输数据的正确性
	if((str==NULL) || (bcd==NULL) || (len==0))
		return -1;

	for(i=0,j=0; i<len; i++,j+=2)
	{
		str[j]= (bcd[i]>>4) > 9 ? (bcd[i]>>4)-10+'A' : (bcd[i]>>4)+'0';
		str[j+1]=(bcd[i] & 0x0F) >9 ? (bcd[i] & 0x0F)-10+'A' : (bcd[i] & 0x0F)+'0';
	}
	str[j]='\0';

	return 0;
}


int Bcd2StrOffSet(char *str ,const unsigned char *bcd, int len, int nOffset)
{
	int i,j,offset;
	int nSpareLen = len%2;

	// 验证传输数据的正确性
	if((str==NULL) || (bcd==NULL) || (len==0))
		return -1;

	if (nOffset)
	{
		for(i=0,j=0; i<len/2; i++,j+=2)
		{
			offset = i;
			str[j]=(bcd[offset] & 0x0F) >9 ? (bcd[offset] & 0x0F)-10+'A' : (bcd[offset] & 0x0F)+'0';
			str[j+1]= (bcd[offset+1]>>4) > 9 ? (bcd[offset+1]>>4)-10+'A' : (bcd[offset+1]>>4)+'0';
		}
	}
	else
	{
		for(i=0,j=0; i<len/2; i++,j+=2)
		{
			str[j]= (bcd[i]>>4) > 9 ? (bcd[i]>>4)-10+'A' : (bcd[i]>>4)+'0';
			str[j+1]=(bcd[i] & 0x0F) >9 ? (bcd[i] & 0x0F)-10+'A' : (bcd[i] & 0x0F)+'0';
		}
	}
	if (nSpareLen != 0)
	{
		Bcd2Ch(str+j, bcd+i, nOffset);
	}
	str[j+1]=0;

	return 0;
}

int iCleanStr(char *str)
{
	int i;
	int len = (int)strlen(str);

	for(i=0; i< len; i++) {
		if(str[i] == 'F') {
			str[i] = 0;
			break;
		}
	}

	return 0;
}

/**
 * 处理名字汉字码，名字编码是5的倍数：
 * 中间如果没有什么编码就有F填充，如果在
 * 5的倍数上还是F的话表示这里就是名字的结束
 */
int CleanName(char *name)
{
	int i;
	int len = (int)strlen(name);

	for(i=0; i < len; i++) {
		if(name[i] == 'F' && (!(i%5))) {
			name[i] = 0;
			break;
		}
	}

	return 0;
}

/**
 *
 */
static int IsNumber(char c)
{
	return (c >= '0') && (c <= '9');
}

/**
 *
 */
static int Number(char c) 
{
	return c - '0';
}

/**
 *
 */
static int ShiftLeft(char *string, int count)
{
	int index = 0;
	char c = string[index];
	for(index = 1; index  < (int)strlen(string); index ++)
	{
		string[index -1] = string[index];
	}

	string[index -1] = c;

	return 0;
}

/**
 *
 */
static int ShiftExchange(char *string, int block)
{
	int len = (int)strlen(string);
	int index = 0;
	char tmp;

	len -= 1;
	tmp = string[block -1];
	string[block -1] = string[len - block + 1];
	string[len-block +1] = tmp;

	return 0;
}

/**
 *
 */
static int ConvertKeyB(const char *seed, char *keyB)
{
	char *str = NULL;
	int length = 0;
	int index = 0;
	int round = 0;

	str = _strdup(seed);
	length = (int)strlen(str);

	// 1:循环累加
	for(index =0; index < length; index ++)
	{
		if(IsNumber(str[index]))
			round += Number(str[index]);
	}
	
	// 2:循环左移
	for(index = 0; index < round; index ++)
	{
		ShiftLeft(str, index);
	}

	// 3: 英文字符26字母对称法还原，数字字符9减得余数还原
	for(index = 0; index < length; index ++)
	{
		if(IsNumber(str[index]))
		{	
			str[index] = '9' - str[index] + '0';
		}
		else
		{
			str[index] = 219 - str[index];
		}
	}

	// 交叉移位
	for(index = 1; index <= (length/2); index ++)
	{
		if(index%2)
		{
			ShiftExchange(str,index);
		}
	}

	// // 取前12位作为KEYB
	memset(keyB, 0, 16);
	memcpy(keyB, str, 12);
	free(str);

	return 0;
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
			bcd[j] = str[i] - 48;
			
		}
		else
		{
			bcd[j] |= (str[i] - 48) << 4;
		}
	}
	
	return i;
}

int iGetKeyBySeed(const unsigned char *_seed, unsigned char *key)
{
	unsigned char tkey[16];
	int length;
	memset(tkey, 0, sizeof(tkey));

	ConvertKeyB(_seed, tkey);

	//输出数据
	length = 12;
	Str2Bcd(tkey, key, &length);

	//下边为调试代码
	printf("[iGetKeyBySeed tkey]:");
	printf("%s\n", tkey);
	return 0;
}