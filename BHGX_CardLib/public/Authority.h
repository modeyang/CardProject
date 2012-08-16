#pragma once
#define MainKey "Software\\±±º½¹ÚÐÂ\\CardProcess"
#define CONFIG  "C:\\WINDOWS\\system32\\"
#define MAXTRY   80

int __stdcall InitCompanyList(const char *namelist);
int __stdcall CheckCompanyInList(const char *name);

int __stdcall CheckCounts(int maxtrys);

int __stdcall InitTimeLicense(const char *filename,const char *time);
int __stdcall CheckTimeLicense();
