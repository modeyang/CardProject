#include "TimeUtil.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#pragma warning (disable : 4996)

CTimeUtil::CTimeUtil(void)
{
}

CTimeUtil::~CTimeUtil(void)
{
}

char * CTimeUtil::getCurrentTime(char *timeStr)
{
	time_t t = time(0); 
	char tmp[64]; 
	strftime( tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&t) ); 
	strcpy(timeStr, tmp);
	return timeStr;
}

char * CTimeUtil::getCurrentDay(char *timeStr)
{
	time_t t = time(0); 
	char tmp[64]; 
	strftime( tmp, sizeof(tmp), "%Y-%m-%d",localtime(&t) ); 
	strcpy(timeStr, tmp);
	return timeStr;
}
