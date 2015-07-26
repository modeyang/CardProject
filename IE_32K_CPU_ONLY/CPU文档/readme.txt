调用BHGX_HospitalProcess.dll 

函数接口:
int __stdcall iFormatHospInfo();
int __stdcall iWriteHospInfo(char *xml);
	
//门诊摘要
int __stdcall iReadClinicInfo(char *xml);

//病案首页
int __stdcall iReadMedicalInfo(char *xml);

//费用结算
int __stdcall iReadFeeInfo(char *xml);