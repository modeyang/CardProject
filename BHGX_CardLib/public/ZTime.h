
#ifndef	_TIME_H_785432543
#define	_TIME_H_785432543


#include "ZTimeSpan.h"
#include <string>

using   namespace   std; 


class CZTime
{
public:
	//构造函数
	CZTime();

	//拷贝构造函书
	CZTime(const CZTime &time);

	//构造函数
	CZTime(
		int nYear,	//公元纪年的年 1900-2100
		int nMonth,		//公元纪年的月 1-12
		int nDay,		//公元纪年的日 1-31
		int nHour,		//时0-23
		int nMin,		//分0-59
		int nSec);		//秒0-59
	
	//析构函数
	~CZTime();

	//获取系统当前时间
	static CZTime GetZCurrentTime();


	// < 运算符重载 
	bool operator < (const CZTime &time) const;

	// == 运算符重载 
	bool operator == (const CZTime &time) const;

	// != 运算符重载 
	bool operator != (const CZTime &time) const;

	// <= 运算符重载 
	bool operator <= (const CZTime &time) const;

	// >= 运算符重载 
	bool operator >= (const CZTime &time) const;

	// > 运算符重载 
	bool operator > (const CZTime &time) const;

	// + 运算符重载 
	CZTime operator + (const CZTimeSpan &timeSpan) const;

	// - 运算符重载 
	CZTime operator - (const CZTimeSpan &timeSpan) const;

	// - 运算符重载 
	CZTimeSpan operator - (const CZTime &time) const;

	// = 运算符重载 
	CZTime& operator = (const CZTime &time);	

	//获取相对1900年1月1日的时日增量
	long GetTotalDays() const;

	//获取相对00：00：00 的时秒增量
	long GetTotalSeconds() const;

protected:

	//时间值相对1900年1月1日的日增量
	long m_nDays;

	//时间值相对00：00：00 的秒增量
	long m_nSeconds;

	//设置时间值
	bool _SetDate(
		int nYear,				//公元纪年的年 1900-2100
		int nMonth,				//公元纪年的月 1-12
		int nDay,				//公元纪年的日 1-31
		int nHour,				//时0-23
		int nMin,				//分0-59
		int nSec);				//秒0-59

	//设置时间值
	bool _SetDate(
		int nDays,				//相对1900年1月1日的日增量
		int nSeconds);			//相对00：00：00 的秒增量

	//获取日期 
	bool _GetDate(
		int &nYear,				//公元纪年的年
		int &nMonth,			//公元纪年的月
		int &nDay				//公元纪年的日
		) const;	

	//获取时间
	bool _GetTime(
		int &nHour,				//时
		int &nMin,				//分
		int &nSec				//秒
		) const;

	//根据格式字符获取格式值
	string _GetValue(char cFormat) const;

	//根据星期几获取星期全名
	string _GetDayofWeekFullName(int nDayofWeek) const;

	//根据几月获取月缩写
	string _GetMonthAbbrevName(int nMonth) const;

	//根据几月获取月全名
	string _GetMonthFullName(int nMonth)const;

};



#endif





