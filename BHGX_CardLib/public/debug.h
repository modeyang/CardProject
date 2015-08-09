#ifndef _CARD_LIB_DEBUG_H_
#define _CARD_LIB_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

	void LogWithTime(int level, char *func, int line, char *format, ...);

	void LogMessage(char *prelog, char *formate, ...);

	void LogPrinter(char *prelog, char *formate, ...);

	int  SetDbgLevel(int level);

	int	 GetDbgLevel(void);

	   
	# define LOG_DEBUG(format, ...)		LogWithTime(0, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)
	# define LOG_INFO(format, ...)		LogWithTime(1, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)
	# define LOG_ERROR(format, ...)		LogWithTime(2, __FUNCTION__,__LINE__, format, ##__VA_ARGS__)

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_DEBUG_H_