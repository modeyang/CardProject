#ifndef _CARD_LIB_DEBUG_H_
#define _CARD_LIB_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

	void LogWithTime(int level, char *format, ...);

	void LogMessage(char *formate, ...);

	void LogPrinter(char *formate, ...);

	int  SetDbgLevel(int level);

	int	 GetDbgLevel(void);

	int pTime(void);

	//   
	#define DBG(level, format, ...)					\
		do {										\
			if(level < GetDbgLevel())			\
			{										\
			LogMessage(format,##__VA_ARGS__);		\
			}\
			else \
			{\
			LogPrinter(format,##__VA_ARGS__);\
			}\
		}while(0)\

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_DEBUG_H_