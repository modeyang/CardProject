#pragma once

#ifndef _BHGX_HOSPPROC_H_
#define _BHGX_HOSPPROC_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT int __stdcall iFormatHospInfo();

	DLL_EXPORT int __stdcall iWriteHospInfo(char *xml);

	DLL_EXPORT int __stdcall iReadIdentify(char *xml);

	DLL_EXPORT int __stdcall iReadHumanInfo(char *xml);

	DLL_EXPORT int __stdcall iReadConnInfo(char *xml);

	//基本信息和发卡机构信息
	DLL_EXPORT int __stdcall iReadCardInfo(char *xml);

	DLL_EXPORT int __stdcall iReadHealthInfo(char *xml);

	//门诊摘要
	DLL_EXPORT int __stdcall iReadClinicInfo(char *, char *xml);

	//病案首页
	DLL_EXPORT int __stdcall iReadMedicalInfo(char *, char *xml);

	//费用结算
	DLL_EXPORT int __stdcall iReadFeeInfo(char *, char *xml);

#ifdef __cplusplus
}
#endif

#endif