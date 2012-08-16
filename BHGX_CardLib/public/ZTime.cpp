
 #include "ZTime.h"
#include <sstream>
#include <windows.h>
#include <time.h>

#if (defined _WIN32) || (defined _WINDOWS_)
#else
#include <sys/sysinfo.h>
#endif

#define SECOND_PERDAY 86400
#define SECOND_PERHOUR 3600

CZTime::CZTime()
{
	m_nDays = 0;
	m_nSeconds = 0;
}

CZTime::CZTime(const CZTime &time)
{
	this->m_nDays = time.GetTotalDays();
	this->m_nSeconds = time.GetTotalSeconds();
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	构造函数	
* 输入参数：	nYear	:	公元纪年的年
*				nMonth	:	公元纪年的月
*				nDay	:	公元纪年的日
*				nHour	:	时
*				nMin	:	分
*				nSec	:	秒
* 输出参数：	无
* 返 回 值：	
* 其它说明：	
*******************************************************************************/
CZTime::CZTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
{
	m_nDays = 0;
	m_nSeconds = 0;
	_SetDate(nYear, nMonth, nDay, nHour, nMin,nSec);
}

CZTime::~CZTime()
{
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取系统当前时间
* 输入参数：	无
* 输出参数：	无
* 返 回 值：	当前系统时间的CZTime类型值
* 其它说明：	
*******************************************************************************/
CZTime CZTime::GetZCurrentTime()
{
	struct tm *times;       //定义时间指针变量    
	time_t t;               //定义系统时间变量,供提供系统时间  
	t = time(0);            //当前系统时间给变量t
	times = localtime(&t);  //把系统时间变量强制转换成tm结构体形式时间
	int nYear, nMonth, nDay, nHour, nMin, nSec;
	nYear = times->tm_year + 1900;
	nMonth = times->tm_mon + 1;
	nDay = times->tm_mday;
	nHour = times->tm_hour;
	nMin = times->tm_min;
	nSec = times->tm_sec;
	
	CZTime tmCur(nYear, nMonth, nDay, nHour, nMin, nSec);
	return tmCur;
}





/*******************************************************************************
* 函数名称：	
* 功能描述：	<
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	小于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator < (const CZTime &time) const
{
	if(this->GetTotalDays() < time.GetTotalDays())
	{
		return true;
	}
	else if(this->GetTotalDays() == time.GetTotalDays()
		&&this->GetTotalSeconds() < time.GetTotalSeconds())
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
* 功能描述：	==
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	等于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator == (const CZTime &time) const
{
	return !( *this  < time || time < *this );		
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	!=
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	不等于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator != (const CZTime &time) const
{
	return ( *this  < time || time < *this );
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	<=
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	小于等于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator <= (const CZTime &time) const
{
	return ( *this < time || *this == time );
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	>=
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	大于等于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator >= (const CZTime &time) const
{
	return !( *this < time);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	>
* 输入参数：	time	:	与*this进行比较的CZTime型变量
* 输出参数：	
* 返 回 值：	大于返回true， 否则返回false
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
bool CZTime::operator > (const CZTime &time) const
{
	return ( time < *this);
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	=
* 输入参数：	
* 输出参数：	
* 返 回 值：	
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-06	司文丽	      创建
*******************************************************************************/
CZTime& CZTime::operator =(const CZTime &time)
{
	this->m_nDays = time.GetTotalDays();
	this->m_nSeconds = time.GetTotalSeconds();
	return *this;
}


/*******************************************************************************
* 函数名称：	
* 功能描述：	获取相对1900年1月1日的日增量
* 输入参数：	
* 输出参数：	
* 返 回 值：	日增量值
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
long CZTime::GetTotalDays() const
{
	return m_nDays;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取相对00：00：00 的秒增量
* 输入参数：	
* 输出参数：	
* 返 回 值：	秒增量
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
long CZTime::GetTotalSeconds() const
{
	return m_nSeconds;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	设置时间值	
* 输入参数：	nYear	:	公元纪年的年
*				nMonth	:	公元纪年的月
*				nDay	:	公元纪年的日
*				nHour	:	时
*				nMin	:	分
*				nSec	:	秒
* 输出参数：	无
* 返 回 值：	执行成功返回true。
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
bool CZTime::_SetDate(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
{
	if (nYear < 1900)
	{
		return false;
	}

	int nDate = 0;
	int i = 0;
	int anMonthDay[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	//起点到该年初的天数
	for (i=1900; i<nYear; i++)
	{
		nDate += 365;
		//如果是闰年加一天
		if((i % 4 == 0) && (i % 100 != 0) || (i % 400 == 0))
		{
			nDate ++;
		}
	}

	//从年初到第一月初的天数
	if((nYear % 4 == 0) && (nYear % 100 != 0) || (nYear % 400 == 0))
	{
		anMonthDay[1] = 29;
	}
	for (i=1; i<nMonth; i++)
	{
		nDate += anMonthDay[i - 1];
	}

	//该月的天数
	nDate += nDay;

	m_nDays = nDate;
	m_nSeconds = nHour * SECOND_PERHOUR + nMin * 60 + nSec;

	return true;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	设置时间值	
* 输入参数：	nDays		:	相对1900年1月1日的时间增量的日计数
*				nSeconds	:	相对00：00：00 的时间增量的秒计数
* 输出参数：	无
* 返 回 值：	执行成功返回true
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
bool CZTime::_SetDate(int nDays, int nSeconds)
{
	m_nDays = (long)nDays;
	m_nSeconds = (long)nSeconds;
	return true;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取日期	
* 输入参数：	无
* 输出参数：	nYear	:	公元纪年的年
*				nMonth	:	公元纪年的月
*				nDay	:	公元纪年的日
* 返 回 值：	执行成功返回true，执行失败返回false。
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
bool CZTime::_GetDate(int &nYear, int &nMonth, int &nDay) const
{
	nYear = 0;
	nMonth = 0;
	nDay = 0;

	int nOffset = (int)m_nDays;
    
	int nTmp = 0;
	int i=0;
	for(i=1900; i<2100 && nOffset>0; i++) 
	{ 
		nTmp = 365;
		if((i % 4 == 0) && (i % 100 != 0) || (i % 400 == 0))
		{
			nTmp ++;
		}
		nOffset -= nTmp; 
	}
	if(nOffset<0) 
	{ 
		nOffset += nTmp;
		i--; 
	}
	nYear = i;

	int anMonthDay[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if((i % 4 == 0) && (i % 100 != 0) || (i % 400 == 0))
	{
		anMonthDay[1] = 29;
	}
	for(i=1; i<12 && nOffset>0; i++) 
	{
		nTmp = anMonthDay[i -1];
		nOffset -= nTmp;
	}

	if(nOffset <= 0)
	{ 
		nOffset += nTmp; 
		--i; 
	}
	nMonth = i;
	nDay = nOffset;	

	return true;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	获取时间，参数
* 输入参数：	无
* 输出参数：	nHour	:	时
*				nMin	:	分
*				nSec	:	秒
* 返 回 值：	执行成功返回true，执行失败返回false。
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-04	司文丽	      创建
*******************************************************************************/
bool CZTime::_GetTime(int &nHour, int &nMin, int &nSec) const
{
	nHour = 0;
	nMin = 0;
	nSec = 0;

	int nOffset = (int)m_nSeconds;

	nHour = nOffset / SECOND_PERHOUR;
	nOffset -= nHour * SECOND_PERHOUR;

	nMin = nOffset / 60;
	nOffset -= nMin * 60;
	
	nSec = nOffset;

	return true;    
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	根据星期几获取全名
* 输入参数：	星期几（1——7， 1为星期日）
* 输出参数：	
* 返 回 值：	星期全名
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-05	司文丽	      创建
*******************************************************************************/
string CZTime::_GetDayofWeekFullName(int nDayofWeek) const
{
	string strValue;

	switch(nDayofWeek)
	{
	case 1:
		strValue = "Sunday";
		break;
	case 2:
		strValue = "Monday";
		break;
	case 3:
		strValue = "Tuesday";
		break;
	case 4:
		strValue = "Wednesday";
		break;
	case 5:
		strValue = "Thursday";
		break;
	case 6:
		strValue = "Friday";
		break;
	case 7:
		strValue = "Saturday";
		break;
	default:
		break;
	}
	return strValue;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	根据几月获取月缩写
* 输入参数：	nMonth	:	几月
* 输出参数：	
* 返 回 值：	返回月缩写
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-05	司文丽	      创建
*******************************************************************************/
string CZTime::_GetMonthAbbrevName(int nMonth) const
{
	string strValue;

	switch(nMonth)
	{
	case 1:
		strValue = "Jan";
		break;
	case 2:
		strValue = "Feb";
		break;
	case 3:
		strValue = "Mar";
		break;
	case 4:
		strValue = "Apr";
		break;
	case 5:
		strValue = "May";
		break;
	case 6:
		strValue = "Jun";
		break;
	case 7:
		strValue = "Jul";
		break;
	case 8:
		strValue = "Aug";
		break;
	case 9:
		strValue = "Sep";
		break;
	case 10:
		strValue = "Oct";
		break;
	case 11:
		strValue = "Nov";
		break;
	case 12:
		strValue = "Dec";
		break;
	default:
		break;
	}
	return strValue;
}

/*******************************************************************************
* 函数名称：	
* 功能描述：	根据几月获取月全名
* 输入参数：	nMonth	:	几月
* 输出参数：	
* 返 回 值：	月全名
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2007-12-05	司文丽	      创建
*******************************************************************************/
string CZTime::_GetMonthFullName(int nMonth) const
{
	string strValue;

	switch(nMonth)
	{
	case 1:
		strValue = "January";
		break;
	case 2:
		strValue = "February";
		break;
	case 3:
		strValue = "March";
		break;
	case 4:
		strValue = "April";
		break;
	case 5:
		strValue = "May";
		break;
	case 6:
		strValue = "June";
		break;
	case 7:
		strValue = "July";
		break;
	case 8:
		strValue = "August";
		break;
	case 9:
		strValue = "September";
		break;
	case 10:
		strValue = "October";
		break;
	case 11:
		strValue = "November";
		break;
	case 12:
		strValue = "December";
		break;
	default:
		break;
	}
	return strValue;
}

