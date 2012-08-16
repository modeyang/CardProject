
#include "ZTimeSpan.h"


#define SECOND_PERDAY 86400
#define SECOND_PERHOUR 3600


CZTimeSpan::CZTimeSpan()
{
	m_nSeconds = 0;
	m_nDays =0;
}


CZTimeSpan::CZTimeSpan(const CZTimeSpan &timespan)
{
	this->m_nDays = timespan.GetDays();
	this->m_nSeconds = timespan.GetTimeSpanSeconds();
}


CZTimeSpan::CZTimeSpan(long nDays, long nSecs)
{
	m_nDays = nDays;
	m_nSeconds = nSecs;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	构造函数
* 输入参数：	lDays	:	时间间隔的日
*				nHours	:	时间间隔的时
*				nMins	:	时间间隔的分
*				nSecs	:	时间间隔的秒
* 输出参数：	无
* 返 回 值：	
* 其它说明：	
*******************************************************************************/
CZTimeSpan::CZTimeSpan(long lDays, int nHours, int nMins, int nSecs)
{
	m_nSeconds = 0;
	m_nDays = 0;
	_SetTimeSpan(lDays, nHours, nMins, nSecs);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	析构函数
* 输入参数：	
* 输出参数：	
* 返 回 值：	
* 其它说明：	
*******************************************************************************/
CZTimeSpan::~CZTimeSpan()
{

}
/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间的日计数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	日计数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetDays() const
{
	return m_nDays;	
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间的时计数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	时计数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetHours() const
{
	long nHours = m_nSeconds / SECOND_PERHOUR;
	return nHours;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间间隔的总时数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	总时数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetTotalHours() const
{
	long nHours = m_nDays * 24 + m_nSeconds / SECOND_PERHOUR;
	return nHours;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间的分计数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	分计数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetMinutes() const
{
	long nSeconsMinusHour = m_nSeconds % SECOND_PERHOUR;
	long nMins = nSeconsMinusHour / 60;
	return nMins;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间间隔的总分数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	总分数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetTotalMinutes() const
{
	long nMins = m_nSeconds / 60 + m_nDays * 24 * 60;
	return nMins;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间的秒计数
* 输入参数：	
* 输出参数：	
* 返 回 值：	秒计数
*******************************************************************************/
long CZTimeSpan::GetSeconds() const
{
	long nSecs = m_nSeconds % 60;
	return nSecs;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间间隔的总秒数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	总秒数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetTotalSeconds() const
{
	long nSecs = m_nDays * SECOND_PERDAY + m_nSeconds;
	return nSecs;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	==
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	相等返回true，否则返回false
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::operator == (const CZTimeSpan &timespan) const
{
	return !( *this < timespan || timespan < *this);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	!=
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	不相等返回true，否则返回false
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::operator !=(const CZTimeSpan &timespan) const
{
	return ( *this < timespan || timespan < *this);	
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	<
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	小于返回true，否则返回false
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::operator < (const CZTimeSpan &timespan) const
{
	if(this->GetDays() < timespan.GetDays())
	{
		return true;
	}
	else if(this->GetDays() == timespan.GetDays()
		&& this->GetTimeSpanSeconds() < timespan.GetTimeSpanSeconds())
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	>
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	大于返回true，否则返回false
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::operator > (const CZTimeSpan &timespan) const
{
	return (timespan < *this);
}
/*******************************************************************************
* 函数名称：	
* 功能描述：	<=
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	小于相等返回true，否则返回false
*******************************************************************************/
bool CZTimeSpan::operator <= (const CZTimeSpan &timespan) const
{
	return !(timespan < *this);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	>=
* 输入参数：	timespan	:	进行比较的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	大于相等返回true，否则返回false
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::operator >= (const CZTimeSpan &timespan) const
{
	return !(*this < timespan);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	=
* 输入参数：	timespan	:	用于赋值的CZTimeSpan变量
* 输出参数：	
* 返 回 值：	
* 其它说明：	
*******************************************************************************/
CZTimeSpan& CZTimeSpan::operator = (const CZTimeSpan &timespan)
{
	this->m_nDays = timespan.GetDays();
	this->m_nSeconds = timespan.GetTimeSpanSeconds();
	return *this;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	设置时间间隔
* 输入参数：	lDay	:	时间间隔的日值
*				nHours	:	时间间隔的时值
*				nMins	:	时间间隔的分值
*				nSecs	:	时间间隔的秒值
* 输出参数：	无
* 返 回 值：	执行成功返回true，执行失败返回false。
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::_SetTimeSpan(long lDays, int nHours, int nMins, int nSecs)
{
	m_nDays = lDays;
	m_nSeconds = (long) nHours * SECOND_PERHOUR + (long) nMins * 60 + (long) nSecs;
	return true;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	设置时间间隔	
* 输入参数：	nDays		:	时间间隔的日计数
*				nSeconse	:	时间间隔的日计数
* 输出参数：	
* 返 回 值：	执行成功返回true，执行失败返回false。
* 其它说明：	
*******************************************************************************/
bool CZTimeSpan::SetTimeSpan(long nDays, long nSeconds)
{
	m_nDays = nDays;
	m_nSeconds = nSeconds;
	return true;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间间隔的秒计数	
* 输入参数：	
* 输出参数：	
* 返 回 值：	秒计数
* 其它说明：	
*******************************************************************************/
long CZTimeSpan::GetTimeSpanSeconds() const
{
	return m_nSeconds;
}
