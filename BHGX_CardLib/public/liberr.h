#pragma once

#ifndef _CARD_LIB_ERR_H_
#define _CARD_LIB_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

enum ErrType
{
	CardProcSuccess = 0,
	CardInitErr = 1,
	CardXmlErr = 2,
	CardReadErr = 3,
	CardScanErr = 4,
	CardWriteErr = 5,
	CardCoverPrintErr = 6,
	CardDLLLoadErr = 7,
	CardCreateErr = 8,
	CardFormatErr = 9,
	CardCreateDateErr = 10,
	FeedCardError = 11,
	CardCheckError = 12,
	CardRegError = 13,
	EncryFileError = 14,
	DescryFileError = 15,
	CardIsNotEmpty = 16,
	CardNotOpen = 17,
	CardNoAuthority = 18,
	CardAuthExpired = 19,
	CardCreateLicenseFailed = 20,
	CardUAuthFailed = 21,
	SendMsgFailed   = 22,

};

static char *errString[] =
{
	"卡操作成功",				// 0
	"卡初始化失败",			// -1
	"XML文件格式有误",		// -2
	"卡读取失败",		// -3
	"寻卡失败",		// -4
	"卡写入失败",		//-5
	"打印卡面失败",	//-6
	"卡打印动态链接库未接入",	//-7
	"制卡失败",			//-8
	"卡格式化失败",				//-9
	"生成卡数据失败",			//-10
	"进卡失败",					// -11
	"卡校验失败",				// -12	
	"卡注册失败",				//-13
	"加密文件失败",				//-14
	"解密文件失败",				//-15
	"卡不是空卡",
	"设备没有被打开",
	"没有被授权操作，请联系供应商",
	"授权已经过期，请联系供应商",
	"生成license文件错误，请检查时间格式",
	"设备没有被授权，请插入U盾",
	"发送短信wsdl失败",
};

char *_err(int errcode);

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_ERR_H_