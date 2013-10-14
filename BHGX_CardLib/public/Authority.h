#pragma once
#define MainKey "Software\\北航冠新\\CardProcess"
#define CONFIG  "C:\\WINDOWS\\system32\\"
#define LICENSESTR	"北航冠新.license"

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall InitCompanyList(const char *namelist);
int __stdcall CheckCompanyInList(const char *name);

int __stdcall InitCountLicense(const char *filename, int maxCounts);
int __stdcall CheckCounts(char* filename,  int maxtrys);

int __stdcall InitTimeLicense(const char *filename,const char *time);
int __stdcall CheckTimeLicense(const char *filename);

int __stdcall InitFullLicense(const char *filename);
int __stdcall CheckFullLicense(const char *filename);

#ifdef __cplusplus
}
#endif
