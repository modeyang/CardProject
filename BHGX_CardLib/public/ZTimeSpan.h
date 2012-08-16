
#ifndef _TIME_SPAN_H_78R789493289148932819452839KFLDKL
#define _TIME_SPAN_H_78R789493289148932819452839KFLDKL


class CZTimeSpan
{
public:

	//构造函数
	CZTimeSpan();

	//拷贝构造函数
	CZTimeSpan(const CZTimeSpan &timespan);

	//构造函书
	CZTimeSpan(long nDays, long nSecs);

	//构造函数，参数依次为时间间隔的日、时、分、秒值
	CZTimeSpan(long lDays,	//时间间隔日值 0-24800(约)
		int nHours,			//时间间隔时值 0-23
		int nMins,			//时间间隔分值 0-59
		int nSecs);			//时间间隔秒值 0-59

	//析构函数
	~CZTimeSpan();
	
	//获取时间的日计数
	long GetDays() const;

	//获取时间的时计数
	long GetHours() const;
	
	//获取时间间隔的总时数
	long GetTotalHours() const;

	//获取时间的分计数
	long GetMinutes() const;

	//获取时间间隔的总分数
	long GetTotalMinutes() const;

	//获取时间的秒计数
	long GetSeconds() const;

	//获取时间间隔的总秒数
	long GetTotalSeconds() const;
	
	// == 运算符重载 
	bool operator == (const CZTimeSpan &span) const ;

	// != 运算符重载 
	bool operator != (const CZTimeSpan &span) const ;

	// < 运算符重载 
	bool operator < (const CZTimeSpan &span) const ;

	// > 运算符重载 
	bool operator > (const CZTimeSpan &span) const ;

	// <= 运算符重载 
	bool operator <= (const CZTimeSpan &span) const ;

	// >= 运算符重载 
	bool operator >= (const CZTimeSpan &span) const ;

	// = 运算符重载 
	CZTimeSpan& operator = (const CZTimeSpan &timespan);

	//获取时间间隔的秒值
	long GetTimeSpanSeconds() const;

	//设置时间间隔
	bool SetTimeSpan(long nDays, long nSeconds);

protected:
	//时间间隔的秒计数
	long m_nSeconds;

	//时间间隔的日计数
	long m_nDays;

	//设置时间间隔，参数依次为时间间隔的日、时、分、秒值
	bool _SetTimeSpan(long lDays, int nHours, int nMins, int nSecs);

	
};


#endif

