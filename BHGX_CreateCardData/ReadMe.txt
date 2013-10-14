BHGX_CreateCardData.dll 1.0.0.7
时间：2013-08-8 16:36
修改功能：
1、修改为自定义License

BHGX_CreateCardData.dll 1.0.0.6
时间：2012-09-25 16:36
修改功能：
1、增加制卡数据1000条

BHGX_CreateCardData.dll 1.0.0.5
时间：2011-05-07 16:00
修改功能：
1、制卡数据自适应农合和公卫

BHGX_CreateCardData.dll 1.0.0.4
时间：2011-04-20 11:30
修改功能：
1、修改特殊字符功能

BHGX_CreateCardData.dll 1.0.0.2
时间：2011-11-16 11:30
修改功能：
1、增加制卡数据解密功能

BHGX_CreateCardData.dll 1.0.0.1
时间：2011-10-15 16:30
修改功能：
1、根据制卡数据成功.data文件

	/*******************************************************************************
	* 函数名称：	iCreateCardData  农合制卡接口	
	* 功能描述：	filename	制卡信息文本文件
	*				license		必须为"北航冠新.license"
					datafile	生成卡数据 默认"card.data"
					infofile	多余卡数据
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	bool 成功 ， false 失败	
	*******************************************************************************/
	DLL_EXPORT int __stdcall iCreateCardData( 
						const char *filename, 
						const char *license,
						const char *datafile = "card.data", 
						const char *infofile = "cardpage.data" 
						);

