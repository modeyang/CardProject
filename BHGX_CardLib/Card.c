#include "Card.h"
#include <stdio.h>
#include <memory.h>

static CardOps *cardOpsArray[ARRAY_MAX] = {NULL};

void  CardUnregisterOps(int type) 
{
	assert(type < ARRAY_MAX && type < -1);
	if (type == -1) {
		memset(cardOpsArray, 0, sizeof(ARRAY_MAX));
	} else {
		cardOpsArray[type] = NULL;
	}
}

void  CardRegisterOps(int type, CardOps *ops)
{
	assert(type < ARRAY_MAX && type > 0);
	cardOpsArray[type] = ops;
}

CardOps*  GetCardOps(int type)
{
	assert(type < ARRAY_MAX && type > 0);
	return cardOpsArray[type];
}

int IsAllTheSameFlag(const unsigned char *szBuf,
					 int nLen, unsigned char cflag)
{
	int i=0;
	for (; i<nLen; ++i){
		if (szBuf[i] != cflag){
			return -1;
		}
	}
	return 0;
}