#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
//#include "stdafx.h"

#include "debug.h"

#define DBGFILE		"c:\\log.txt"

static int DbgLevel = 7;

/**
 *
 */
void LogMessage(char *formate, ...)
{
	FILE *fd;
	va_list ap;

	fopen_s(&fd, DBGFILE, "a+b");

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