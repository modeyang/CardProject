#pragma once

#ifndef _BHGX_CREATE_CARD_H_
#define _BHGX_CREATE_CARD_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* 函数名称：	iCreateCardData	
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

	DLL_EXPORT int __stdcall iCreatePubSysCardData( 
							const char *filename, 
							const char *license,
							const char *datafile = "card.data", 
							const char *infofile = "cardpage.data" 
							);
#ifdef __cplusplus
}
#endif

#endif	//_CREATE_CARD_H