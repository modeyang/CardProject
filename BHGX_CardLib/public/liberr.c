#include <stdio.h>
#include <stdlib.h>
#include "liberr.h"



char *_err(int errcode)
{
	if(errcode < 0)
		errcode = -errcode;
	if (errcode > sizeof(errString)/sizeof(char*)) {
		char errInfo[10];
		return itoa(errcode, errInfo, 10);
	}

	return errString[errcode];
}