#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <time.h>
#include "debug.h"
#include <stdio.h>

#pragma warning (disable : 4996)
#define DBGFILE		"C:\\BHGX_CardLib.log"

static int DbgLevel = 0;
char log_buff[1024];

void LogWithTime(int level,  char *func, int line,  char *format, ...)
{
	time_t t = time(0);
	char timeStr[64]; 
	FILE *fd;
	va_list ap;

	memset(log_buff,0, sizeof(log_buff)); 
	strftime( timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",localtime(&t) );
	if (level == 0) {
		strcpy(log_buff, "[DEBUG]");
	} else if (level == 1) {
		strcpy(log_buff, "[INFO]");
	} else if (level == 2) {
		strcpy(log_buff, "[ERROR]");
	} else {
		strcpy(log_buff, "[DEBUG]");
	}

	sprintf_s(log_buff, sizeof(log_buff), "%s %s %s:%d,", log_buff, timeStr, func, line); 
	if (level > GetDbgLevel()) {
		fopen_s(&fd, DBGFILE, "ab+");

		va_start(ap, format);
		vprintf(log_buff, ap);
		vprintf(format, ap);
		vprintf("\r\n", ap);

		vfprintf(fd, log_buff, ap);
		vfprintf(fd, format, ap);
		vfprintf(fd, "\r\n", ap);
		va_end(ap);

		fflush(fd);

		fclose(fd);
	} else {
		va_start(ap, format);
		vprintf(log_buff, ap);
		vprintf(format, ap);
		vprintf("\r\n", ap);
		va_end(ap);
	}
}

/**
 *
 */
void LogMessage(char *prelog, char *formate, ...)
{
	FILE *fd;
	va_list ap;

	LogPrinter(prelog, formate);
	fopen_s(&fd, DBGFILE, "ab+");

	va_start(ap, formate);
	vfprintf(fd, prelog, ap);
	vfprintf(fd, formate, ap);
	vfprintf(fd, "\r\n", ap);
	va_end(ap);

	fflush(fd);

	fclose(fd);

	return;
}

void LogPrinter(char *prelog, char *formate, ...)
{
	va_list ap;
	va_start(ap, formate);
	vprintf(prelog, ap);
	vprintf(formate, ap);
	vprintf("\r\n", ap);
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

