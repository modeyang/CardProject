#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <time.h>
#include "debug.h"
#include <stdio.h>

#pragma warning (disable : 4996)
#define DBGFILE		"c:\\log.txt"

static int DbgLevel = 7;
char tmp[256];

void LogWithTime(int level, char *format, ...)
{
	time_t at;
	at = time(0);
	memset(tmp,0, sizeof(tmp));
	strftime( tmp, sizeof(tmp), "%Y-%m-%d %X",localtime(&at));
	sprintf_s(tmp, sizeof(tmp), "%s %s:%d:%s\n", tmp, __FILE__,__LINE__, format); 
	if (DbgLevel > GetDbgLevel()) {
		LogPrinter(tmp);
	} else {
		LogMessage(tmp);
	}
}

/**
 *
 */
void LogMessage(char *formate, ...)
{
	FILE *fd;
	va_list ap;

	fopen_s(&fd, DBGFILE, "a+");

	va_start(ap, formate);
	vfprintf(fd, formate, ap);
	va_end(ap);

	fflush(fd);

	fclose(fd);

	return;
}

void LogPrinter(char *formate, ...)
{
	va_list ap;
	va_start(ap, formate);
	vprintf(formate, ap);
	va_end(ap);
}

/**
 *
 */
int SetDbgLevel(int level)
{
	DbgLevel = level;

	return DbgLevel;
}

/**
 *
 */
int	GetDbgLevel(void)
{
	return DbgLevel;
}


/**
 *
 */
int pTime(void)
{
	SYSTEMTIME s;
	GetLocalTime(&s);

	DBG(0, "%d:%d",s.wSecond, s.wMilliseconds);
	return 0;
}