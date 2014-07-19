#pragma once

class CTimeUtil
{
public:
	CTimeUtil(void);
	~CTimeUtil(void);

	static char *getCurrentTime(char *timeStr);

	static char *getCurrentDay(char *timeStr);
};
