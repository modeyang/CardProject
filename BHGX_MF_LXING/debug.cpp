#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

#define DBGFILE		"C:\\log.txt"

static int DbgLevel = 7;

#if 1
/**
*
*/
void _LogMessage(char *formate, ...)
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
#endif
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