#pragma once

#ifndef _BHGX_HISReader_H_
#define _BHGX_HISReader_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* 函数名称：	iReadHISInfo	
	* 功能描述：	xml			读卡信息
	*				pszCardCheckWSDL	WSDL文件地址
	*				pszCardServerURL	WSDL服务器地址
	*				bMsg				是否发送短信
	*				msgSUrl				短信发送服务器地址
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml,  bool bMsg=false, char *msgSUrl="");

	/*******************************************************************************
	* 函数名称：	iReadOnlyHIS	
	* 输入参数：	xml			读卡信息
	*				bMsg		是否发送短信
	*				msgSUrl		短信发送服务器地址
	* ：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHIS(char *xml,  bool bMsg=false, char *msgSUrl="");

	/*******************************************************************************
	* 函数名称：	iReadInfoForXJ	
	* 功能描述：	
	* 输入参数：	xml			读卡信息
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);

	/*******************************************************************************
	* 函数名称：	iSendMessage	
	* 功能描述：	发送短信webservice接口
	* 输入参数：	url			短信发送服务器地址
	*				cardNO		卡号
	*				identity	身份证号
	*				farmID		农合号
	*				name		姓名
	*				healthID	健康档案号
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iSendMessage(
			char *url, 
			const char *cardNO, 
			const char *identity, 
			const char *farmID, 
			const char *name, 
			const char *healthID
		);
#ifdef __cplusplus
}
#endif

#endif