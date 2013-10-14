1.0.0.7 2013-08-07 2:00:00
1、添加两个参数给iReadHISInfo和iReadOnlyHIS
	* 函数名称：	iReadHISInfo	
	* 功能描述：			xml			读卡信息
	*				pszCardCheckWSDL	WSDL文件地址
	*				pszCardServerURL	WSDL服务器地址
	*				bMsg			是否发送短信
	*				msgSUrl			短信发送服务器地址

	* 函数名称：	iReadOnlyHIS	
	* 功能描述：			xml		读卡信息
	*				bMsg		是否发送短信
	*				msgSUrl		短信发送服务器地址

2. webserver发送5个参数依次为卡号、身份证号、农合号、姓名、健康档案号