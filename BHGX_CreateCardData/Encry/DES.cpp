// DES.cpp: implementation of the DES class.
//
//////////////////////////////////////////////////////////////////////

#include <memory.h>
#include "DES.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DES::DES()
{

}

DES::~DES()
{

}



bool DES::Des_Go(char *Out, char *In, long datalen, const char *Key, int keylen, bool Type)
{
if( !( Out && In && Key && (datalen=(datalen+7)&0xfffffff8) ) ) 
		return false;
	SetKey(Key, keylen);
	if( !Is3DES ) {   // 1次DES
		for(long i=0,j=datalen>>3; i<j; ++i,Out+=8,In+=8)
			SDES(Out, In, &SubKey[0], Type);
	}
	else{   // 3次DES 加密:加(key0)-解(key1)-加(key0) 解密::解(key0)-加(key1)-解(key0)
		for(long i=0,j=datalen>>3; i<j; ++i,Out+=8,In+=8) {
			SDES(Out, In,  &SubKey[0], Type);
			SDES(Out, Out, &SubKey[1], !Type);
			SDES(Out, Out, &SubKey[0], Type);
		}
	}
	return true;
 }

void DES::Des_one(char Out[], char In[], const char Key[], bool type)
{

	SetKey(Key, 8);
	SDES(Out,In,&SubKey[0],type);
}

