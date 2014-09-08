#ifndef CPUCARD_H
#define CPUCARD_H
#include "Card.h"

#ifdef  __cplusplus
extern "C" {
#endif

//对于记录文件的描述
struct RecFolder{
	BYTE	section[10];      //DDF
	BYTE    subSection[10];   //DF
	BYTE	fileName[10];     //EF, ED....
};

adapter* __stdcall InitCpuAdapter();

int __stdcall FormatCpuCard(char c);

#ifdef  __cplusplus
}
#endif
#endif