#include "Authority.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "../Encry/DESEncry.h"
#include <string>
#include <vector>
#include "ZTime.h"

using namespace std;

#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)

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
	strcat(path,".cardLicense");
	fp = fopen(path, "r+b");
	if (fp == NULL) {
		fp = fopen(path, "w+b");
		sprintf(szCount, "%d" , 1);
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
		printf("超过尝试最大数量,请联系供应商\n");
		fclose(fp);
		return 0;
	}
	printf("%d  ", nCounts);
	fseek(fp, 0, SEEK_SET);
	memset(szCount, 0, sizeof(szCount));
	memset(szDst, 0, sizeof(szDst));
	sprintf(szCount, "%d", nCounts);
	des.EncryString(szCount, szDst);
	fwrite(szDst, sizeof(szDst), 1, fp);
	fclose(fp);
	return 1;
}

int __stdcall InitTimeLicense(const char *filename,const char *ctime)
{
	FILE *fp;
	char license[256], enlicense[256];
	if (!isTimeFormat(license)) {
		return -1;
	}
	fp = fopen(filename, "w");
	if (fp == NULL) {
		return -1;
	}
	sprintf_s(license, sizeof(license), "BHGX|%s", ctime);
	CDESEncry entry;
	entry.EncryString(license, enlicense);
	fwrite(enlicense, sizeof(enlicense),1, fp);
	fclose(fp);
	return 0;
}
int  __stdcall  CheckTimeLicense(const char *filename)
{
	char cTime[256], license[256];
	FILE *fp;
	int counts = 0;
	if ((fp = fopen(filename, "r")) == NULL){
		printf("无法打开license文件，请联系供应商");
		return -1;
	}
	memset(cTime, 0, sizeof(cTime));
	counts = fread(cTime, sizeof(cTime), 1, fp);
	if (counts <= 0) {
		printf("无法打开license文件，请联系供应商");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	memset(license, 0, sizeof(license));
	CDESEncry entry;
	entry.DescryString(cTime, license);
	counts = isTimeExpired(license);
	return (counts==1 ? 0:-1);
}   

static int  isTimeExpired(const char *cTime)
{
	int y, m, d, h, mn, se;
	int status = sscanf(cTime, "%*[^|]|%d-%d-%d %d:%d:%d",
		                &y, &m, &d, &h, &mn, &se);
	if (status == 0) 
		return 0;

	CZTime mytime(y, m, d, h, mn, se);;
	CZTime curTime = CZTime::GetZCurrentTime();
	return (mytime < curTime);
}

static int isTimeFormat(const char *time)
{
	int y, m, d, h, mn, se;
	int status = sscanf(time, "%*[^|]|%d-%d-%d %d:%d:%d", 
						&y, &m, &d, &h, &mn, &se);
	return (status > 0);
}
