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
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadOnlyHIS	
	* 功能描述：	xml			读卡信息
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHIS(char *xml);

	/*******************************************************************************
	* 函数名称：	iReadInfoForXJ	
	* 功能描述：	xml			读卡信息
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadHISInfoLog	
	* 功能描述：	xml			读卡信息
	*				pszCardCheckWSDL	WSDL文件地址
	*				pszCardServerURL	WSDL服务器地址
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfoLog(char *pszCardCheckWSDL, char *pszCardServerURL, 
										  char *pszLogXml, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadOnlyHISLog	
	* 功能描述：	xml			读卡信息
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHISLog(char *pszLogXml, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadInfoForXJLog	
	* 功能描述：	xml			读卡信息
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJLog(char *pszCardCheckWSDL, char *pszCardServerURL, 
											char *pszLogXml, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadHISInfoLocal	
	* 功能描述：	xml			读卡信息
	*				pszCardCheckWSDL	WSDL文件地址
	*				pszCardServerURL	WSDL服务器地址
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfoLocal(char *pszLogXml, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadInfoForXJLocal	
	* 功能描述：	xml			读卡信息
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJLocal(char *pszLogXml, char *xml);

	/*******************************************************************************
	* 函数名称：	iReadOnlyHISLocal
	* 功能描述：	xml			读卡信息
	*				pszLogXml			日志描述xml
	* 输入参数：	
	* 输出参数：	
	* 返 回 值：	0:成功 其他：失败
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHISLocal(char *pszLogXml, char *xml);

#ifdef __cplusplus
}
#endif

#endif