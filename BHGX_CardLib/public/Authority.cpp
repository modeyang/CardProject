#include "Authority.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "../Encry/DESEncry.h"
#include "liberr.h"
#include <string>
#include <vector>
#include "ZTime.h"
#include "debug.h"

using namespace std;

#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)

#define PADDING		"Card"
#define TIMEFMT		"2012-8-19_16:20:23"

//制卡单位
std::vector<std::string> g_vecCompany;
static int isTimeExpired(const char *cTime);
static int isTimeFormat(const char *time);

int __stdcall InitCompanyList(const char *namelist)
{
	return 0;
}
int __stdcall CheckCompanyInList(const char *name)
{
	std::string strComp(name);
	for (size_t i=0; i<g_vecCompany.size(); ++i){
		if (strComp == g_vecCompany[i]) {
			return 1;
		}
	}
	return 0;
}

int  __stdcall CheckCounts(int maxtrys)
{
	char path[256];
	char szCount[10];
	char szDst[10];
	int  nCounts;
	CDESEncry des;
	FILE *fp;
	memset(szCount, 0, sizeof(szCount));
	memset(szDst, 0, sizeof(szDst));
	memset(path, 0, 256);
	strcpy(path, CONFIG);
	strcat(path,"cralbiss.cpl");
	fp = fopen(path, "rb");
	if (fp == NULL) {
		fp = fopen(path, "wb");
		sprintf_s(szCount, sizeof(szCount), "%d" , 1);
		des.EncryString(szCount, szDst);
		fwrite(szDst, sizeof(szDst), 1, fp);
		fclose(fp);
		return 1;
	}

	fread(szCount, sizeof(szCount), 1, fp);
	des.DescryString(szCount, szDst);
	nCounts = atoi(szDst);
	nCounts++;
	if (nCounts > maxtrys) {
		LOG_DEBUG("%d超过尝试最大数量%d,请联系供应商\n", nCounts, maxtrys);
		fclose(fp);
		return 0;
	}
	fclose(fp);

	
	sprintf_s(szCount, sizeof(szCount), "%d", nCounts);
	des.EncryString(szCount, szDst);

	fp = fopen(path, "wb");
	if (fp == NULL) {
		return 0;
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(szDst, sizeof(szDst), 1, fp);
	fclose(fp);
	return 1;
}

static void EncryTest()
{
	char license[256], enlicense[256];
	sprintf_s(license, sizeof(license), "%s|%s",LICENSESTR, TIMEFMT);
	CDESEncry entry;
	memset(enlicense, 0, sizeof(enlicense));
	entry.EncryString(license, enlicense);
	FILE *fp;
	fp = fopen("test.txt", "wb");
	fwrite(enlicense, strlen(enlicense), 1, fp);
	fclose(fp);

	fp = fopen("test.txt", "rb");
	memset(enlicense, 0, sizeof(enlicense));
	fread(enlicense, sizeof(enlicense), 1, fp);
	fclose(fp);
	memset(license, 0, sizeof(license));
	entry.DescryString(enlicense, license);
	printf("%s\n", license);
}

static int InitLicense(const char *filename,const char *enlicense) 
{
	FILE *fp;
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		return CardCreateLicenseFailed;
	}
	fwrite(enlicense, strlen(enlicense),1, fp);
	fclose(fp);
	return 0;
}

int __stdcall InitTimeLicense(const char *filename,const char *ctime)
{
	char license[256], enlicense[256];
	if (!isTimeFormat(ctime)) {
		return CardCreateLicenseFailed;
	}
	sprintf_s(license, sizeof(license), "%s|%s",LICENSESTR, ctime);
	CDESEncry entry;
	memset(enlicense, 0, sizeof(enlicense));
	entry.EncryString(license, enlicense);
	return InitLicense(filename, enlicense);
}
int  __stdcall  CheckTimeLicense(const char *filename)
{
	char cTime[256], license[256];
	FILE *fp;
	int counts = 0;
	if ((fp = fopen(filename, "rb")) == NULL){
		printf("无法打开license文件，请联系供应商");
		return -1;
	}
	memset(cTime, 0, sizeof(cTime));
	counts = fread(cTime, sizeof(cTime), 1, fp);
	if (counts < 0) {
		printf("无法打开license文件，请联系供应商");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	memset(license, 0, sizeof(license));
	CDESEncry entry;
	entry.DescryString(cTime, license);
	counts = isTimeExpired(license);
	return (counts==1 ? -1 : 0);
}   

int __stdcall InitFullLicense(const char *filename)
{
	char license[256], enlicense[256];
	sprintf_s(license, sizeof(license), "%s|%s",LICENSESTR, PADDING);
	CDESEncry entry;
	memset(enlicense, 0, sizeof(enlicense));
	entry.EncryString(license, enlicense);
	return InitLicense(filename, enlicense);
}

int __stdcall CheckFullLicense(const char *filename)
{
	//EncryTest();
	char cTime[256], license[256];
	FILE *fp;
	int counts = 0;
	if ((fp = fopen(filename, "rb")) == NULL){
		printf("无法打开license文件，请联系供应商");
		return -1;
	}
	memset(cTime, 0, sizeof(cTime));
	counts = fread(cTime, sizeof(cTime), 1, fp);
	if (counts < 0) {
		printf("无法打开license文件，请联系供应商");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	memset(license, 0, sizeof(license));
	CDESEncry entry;
	entry.DescryString(cTime, license);
	char *ptr = strchr(license, '|');
	if (ptr != NULL)
		*ptr = 0;

	counts = strcmp(license, LICENSESTR);
	return (counts == 0 ? 0 : -1); 
}
static int  isTimeExpired(const char *cTime)
{
	int y, m, d, h, mn, se;
	int status = sscanf(cTime, "%*[^|]|%d-%d-%d_%d:%d:%d",
		                &y, &m, &d, &h, &mn, &se);
	if (status == 0) 
		return 0;

	CZTime mytime(y, m, d, h, mn, se);;
	CZTime curTime = CZTime::GetZCurrentTime();
	return (mytime < curTime);
}

#define ASSERT_RANGE(a, b, c) !((a<=b) && (a>=c))
static int isTimeFormat(const char *time)
{
	int y, m, d, h, mn, se;
	int status = sscanf(time, "%d-%d-%d_%d:%d:%d", 
						&y, &m, &d, &h, &mn, &se);
	int ret = ASSERT_RANGE(m, 12, 0);
	ret |= ASSERT_RANGE(d, 30, 0);
	ret |= ASSERT_RANGE(h, 24, 0);
	ret |= ASSERT_RANGE(mn, 60, 0);
	ret |= ASSERT_RANGE(se, 60, 0);
	return (status == 6 && ret==0);
}

