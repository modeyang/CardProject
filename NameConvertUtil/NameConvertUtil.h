#pragma once

#ifndef _NameConvertUtil_H_
#define _NameConvertUtil_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* 函数名称：	
	* 功能描述：	姓名转化成姓名码	
	* 输入参数：	strName		-- 姓名
	* 输出参数：	strCode		-- 姓名码
	*				nLen		-- 姓名码长度
	* 返 回 值：	bool 成功 ， false 失败	
	*******************************************************************************/
	DLL_EXPORT int __stdcall Name2Code(const char *strName, char *strCode, int *nLen);

	/*******************************************************************************
	* 函数名称：	
	* 功能描述：	姓名转化成姓名码	
	* 输入参数：	strCode		-- 姓名码
	* 输出参数：	strName		-- 姓名
	*				nLen		-- 姓名长度
	* 返 回 值：	bool 成功 ， false 失败	
	*******************************************************************************/
	DLL_EXPORT int __stdcall Code2Name(const char *strCode, char *strName, int *nLen);
#ifdef __cplusplus
}
#endif

#endif	//_CREATE_CARD_H