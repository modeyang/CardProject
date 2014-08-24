#pragma once

#ifndef _CARD_LIB_H_
#define _CARD_LIB_H_

#define DLL_EXPORT _declspec(dllexport)

#define F1		0x01		//制卡信息
#define F2		0x02		//参合基本信息
#define F3		0x04		//农合补偿信息
#define F4		0x08		//补偿日志
#define F5		0x10		//健康档案信息

#ifdef __cplusplus
extern "C" {
#endif
	// 得到卡版本号
	DLL_EXPORT int __stdcall iGetCardVersion(char *pszVersion);

	DLL_EXPORT int __stdcall iCardCompany(char *szCompanyXml);

	// 动态链接库初始化函数
	DLL_EXPORT int __stdcall iCardInit(char *xml = "");

	DLL_EXPORT int __stdcall iCardDeinit();

	DLL_EXPORT int __stdcall iCardClose();

	DLL_EXPORT int __stdcall iCardOpen();

	DLL_EXPORT int __stdcall iCardIsEmpty();

	// 读取flag标志，输出xml
	DLL_EXPORT int __stdcall iReadInfo(int flag, char *xml);

	DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name="");

	// 读取name信息，单个标志情况，
	DLL_EXPORT int __stdcall iQueryInfo(char *name, char *xml);

	// 写入xml内容
	DLL_EXPORT int __stdcall iWriteInfo(char *xml);

	DLL_EXPORT int __stdcall iScanCard();

	//错误信息
	DLL_EXPORT char* __stdcall err(int errcode);

	//制卡并打印
	DLL_EXPORT int __stdcall iPatchCard(
							char *pszCardDataXml,
							char *pszCardCoverDataXml,
							char *pszPrinterType,
							char *pszCardCoverXml 
							);

	//获取打印机列表名字
	DLL_EXPORT int __stdcall iGetPrinterList(char *PrinterXML);

	//打印卡封面
	DLL_EXPORT int __stdcall iPrintCard(
							char *pszPrinterType,
							char *pszCardCoverDataXml,
							char *pszCardCoverXml 
							);


	//制卡
	DLL_EXPORT int __stdcall iCreateCard(char *pszCardDataXml);

	DLL_EXPORT int __stdcall iFormatCard();

	DLL_EXPORT int __stdcall iCardCtlCard(int cmd, void *data);

	//卡校验
	DLL_EXPORT int __stdcall iCheckMsgForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	//1|2|3|4 对应 姓名|身份证号|出生日期|性别
	DLL_EXPORT int __stdcall iReadConfigMsg(
		char *pszConfigXML,
		char *pszReadXML
		);

	//卡注册
	DLL_EXPORT int __stdcall iRegMsgForNH(
		char *pszCardServerURL, 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadOnlyCardMessageForNH( 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	//卡注册
	DLL_EXPORT int __stdcall iRegMsgForNHLog(
		char *pszCardServerURL, 
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNHLog(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNHLocal(
		char* pszLogXml,
		char* pszXml
		);

	//卡校验 黑名单校验
	DLL_EXPORT int __stdcall iCheckMsgForNHLocal(
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iEncryFile(char *filename);

	DLL_EXPORT int __stdcall iCreateLicense(
		char *filename, 
		char *timeStr
		);  

#define TIMELICENSE		0
#define OTHERLICENSE	1
#define COUNTS			2
	DLL_EXPORT int __stdcall iCheckLicense(
		char *filename,
		int type					//0按时间  1其他
		);

	// 黑名单校验
	DLL_EXPORT int __stdcall iCheckException(
		char *pszLogXml,
		char *pszXml
		);


#ifdef __cplusplus
}
#endif

#endif	// _CARD_LIB_H_