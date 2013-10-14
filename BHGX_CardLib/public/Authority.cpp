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

#define PREFIX      "#df%(IW$#f"
#define PADDING		"Card"
#define TIMEFMT		"2012-8-19_16:20:23"

//制卡单位
std::vector<std::string> g_vecCompany;
static int isTimeExpired(const char *cTime);
static int isTimeFormat(const char *time);

char EncryArray[11] = {'k', 'f', 'e', 't', 'y', 'u', 'i', 'l', 'p', 'n'};
int getEncry(int counts, char *encry) 
{
	char countStr[20];
	memset(countStr, 0, sizeof(countStr));
	sprintf(countStr, "%d", counts);

	char intArray[2];
	memset(intArray, 0, sizeof(intArray));
	memset(encry, 0, sizeof(encry));
	for (int i=0; i<strlen(countStr); i++) {
		sprintf(intArray, "%c", countStr[i]);
		int num = atoi(intArray);
		encry[i] = EncryArray[num];
	}
	return strlen(encry);
}

int getIndex(char c) 
{
	for (int i=0; i<sizeof(EncryArray); i++) {
		if (c == EncryArray[i]) {
			return i;
		}
	}
	return -1;
}

int getDecry(char *encry)
{
	char intArray[20];
	memset(intArray, 0, sizeof(intArray));
	for (int i=0; i<strlen(encry); i++) {
		int index = getIndex(encry[i]);
		sprintf(intArray, "%s%d", intArray, index);
	}
	int count = atoi(intArray);
	return count;
}

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

int  __stdcall CheckCounts(char *filename, int maxtrys)
{
	int status = 1;
	FILE *fp = fopen(filename, "rb+");
	if (fp == NULL) {
		LogPrinter("License文件错误，请购买license\n");
		return 0;
	}
	char desContent[30];
	memset(desContent, 0, sizeof(desContent));
	fread(desContent, sizeof(desContent),  1, fp);
	strcpy(desContent, desContent+10);

	char *dst = strchr(desContent, '|');
	if (dst == NULL) {
		LogPrinter("License文件错误，请购买license\n");
		fclose(fp);
		return 0;
	}

	char content[30];
	memset(content, 0, sizeof(content));
	memcpy(content, desContent, dst - &desContent[0]);

	char behind[30];
	memset(behind, 0, sizeof(behind));
	strcpy(behind, dst+1);

	int counts = getDecry(content);
	//printf("before************%d, \t %s\n", counts, desContent);
	if (counts <= 0) {
		LogPrinter("超过尝试最大数量,请联系供应商\n");
		status = 0;
		goto done;
	} else {
		counts -= 1;
	}
done:
	fseek(fp, 0, 0);
	memset(content, 0, sizeof(content));
	getEncry(counts, content);

	char encryContent[30];
	memset(encryContent, 0, sizeof(encryContent));
	sprintf(encryContent, "%s%s|%s", PREFIX,  content, behind);
	fwrite(encryContent, sizeof(encryContent), 1, fp);
	//printf("after********** %d, \t %s\n", counts, encryContent);
	fclose(fp);
	return status;
}

int  __stdcall CheckCounts(void *fd)
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
	strcat(path,"cralbise.cpl");
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
	if (nCounts > 0) {
		LogPrinter("%d超过尝试最大数量%d,请联系供应商\n", nCounts, 0);
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


int __stdcall InitCountLicense(const char *filename, int maxCounts) 
{
	char license[30];
	memset(license, 0, sizeof(license));
	getEncry(maxCounts, license);

	char encryLicense[50];
	memset(encryLicense, 0, sizeof(encryLicense));
	sprintf(encryLicense, "%s%s|%s", PREFIX, license, "@$#q");
	//printf("%s\n", encryLicense);
	return InitLicense(filename, encryLicense);
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
	sprintf_s(license, sizeof(license), "%s|%s", LICENSESTR, PADDING);
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

