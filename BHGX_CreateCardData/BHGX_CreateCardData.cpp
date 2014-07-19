//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include "BHGX_CreateCardData.h"

#include "Encry/DESEncry.h"
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "tinyxml/headers/tinyxml.h"
#include "algorithm.h"
#include "Resource.h"

#ifdef _DEBUG
#pragma comment(lib, "../Debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../Release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#ifndef NULL
#define NULL
#endif

using namespace std;

/**
*
*/
void trim(string & str)
{
	string::size_type pos = str.find_last_not_of(' ');
	if(pos != string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}

void trim(wstring & str)
{
	wstring::size_type pos = str.find_last_not_of(L' ');
	if(pos != string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(L' ');
		if(pos != string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}
/**
*
*/
typedef basic_string<char>::size_type S_T;
static const S_T npos = -1;

//delimit为一个字符，严格分割
vector<string> split(const string& src, string delimit, string null_subst="")  
{  
	vector<string> v; 
	if( src.empty() || delimit.empty() ) 
		return v;
	//throw "split:empty string\0";    

	S_T deli_len = delimit.size();  
	long index = npos, last_search_position = 0;  
	while( (index=(long)src.find(delimit,last_search_position))!=npos )  
	{  

		if(index==last_search_position)  
			v.push_back(null_subst);  
		else 
		{
			string tmp = src.substr(last_search_position, index-last_search_position);
			trim(tmp);
			v.push_back(tmp);
		}

		last_search_position = (long)(index + deli_len);  
	}  
	string last_one = src.substr(last_search_position);  
	v.push_back( last_one.empty()? null_subst:last_one );  

	return v;  
}   

/**
*
*/
typedef basic_string<wchar_t>::size_type S_T;
static const S_T wnpos = -1;

//delimit为一个字符，严格分割
vector<wstring> split(const wstring& src, wstring delimit, wstring null_subst=L"")  
{  
	vector<wstring> v; 
	if( src.empty() || delimit.empty() ) 
		return v;
	//throw "split:empty string\0";    

	S_T deli_len = delimit.size();  
	long index = wnpos, last_search_position = 0;  
	while( (index=(long)src.find(delimit,last_search_position))!=wnpos )  
	{  

		if(index==last_search_position)  
			v.push_back(null_subst);  
		else 
		{
			wstring tmp = src.substr(last_search_position, index-last_search_position);
			trim(tmp);
			v.push_back(tmp);
		}

		last_search_position = (long)(index + deli_len);  
	}  
	wstring last_one = src.substr(last_search_position);  
	v.push_back( last_one.empty()? null_subst:last_one );  

	return v;  
}  

void convertW2CVector(vector<string> &v, const vector<wstring> &wv)
{
	char tmp[100];
	for (size_t i=0; i<wv.size(); ++i)
	{
		memset(tmp, 0, sizeof(tmp));
		wcstombs(tmp, wv[i].c_str(), wv[i].size()*2);
		v.push_back(tmp);
	}
}

/**
*
*/
static int iCreateXmlByVector(const vector<string>&  v, char *xml, int *length)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;

	int result = 0;

	char buf[10];

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID","2");

	for(int i = 0; i < 28; i++)
	{
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", i+1);

		Cloumn = new TiXmlElement("COLUMN");
		Cloumn->SetAttribute("ID", buf);
		Cloumn->SetAttribute("VALUE", v[i].c_str());

		Segment->LinkEndChild(Cloumn);
	}

	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return result;
}

static int iCreatePubSysXmlByVector(const vector<string>&  v, char *xml, int *length)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;

	int result = 0;

	char buf[10];

	// 创建XML文档
	XmlDoc = new TiXmlDocument();

	// 增加XML的头部说明
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// 产生TiXMLDoc文档
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID","2");

	for(int i = 0; i < 28; i++)
	{
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", i+1);

		Cloumn = new TiXmlElement("COLUMN");
		Cloumn->SetAttribute("ID", buf);
		Cloumn->SetAttribute("VALUE", v[i].c_str());

		Segment->LinkEndChild(Cloumn);
	}

	RootElement->LinkEndChild(Segment);

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID", "5");
	Cloumn = new TiXmlElement("COLUMN");
	Cloumn->SetAttribute("ID", 1);
	Cloumn->SetAttribute("VALUE", v[28].c_str());
	Segment->LinkEndChild(Cloumn);
	RootElement->LinkEndChild(Segment);

	XmlDoc->LinkEndChild(RootElement);

	// 把XML文档的内容传给上层
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return result;
}

/**
*
*/
static void stripLeadingAndTrailingBlanks(string& StringToModify)
{
	if(StringToModify.empty()) return;

	size_t startIndex = StringToModify.find_first_not_of(" ");
	size_t endIndex = StringToModify.find_last_not_of(" ");

	string tempString = StringToModify;
	StringToModify.erase();

	StringToModify = tempString.substr(startIndex, (endIndex-startIndex+ 1) );
}

/**
* 得到KEY值
*/
static int iGetKeyValue(unsigned char *seed,  unsigned char *key)
{
	unsigned char keyA[6] = {0x43,0x97,0x04,0x47,0x20,0x47};
	unsigned char control[4] = {0x08,0x77,0x8f,0x69};
	unsigned char keyB[6];

	// 获得KEYB值
	memset(keyB, 0, 6);
	iGetKeyBySeed(seed, keyB);

	// 写入到返回值
	memcpy(key, keyA, 6);
	memcpy(key+6, control, 4);
	memcpy(key+10, keyB, 6);

	return 0;
}

struct CreateKeyInfoS
{
	int offset;
	int length;
	unsigned char ID[9];
	unsigned char token[6];   
	unsigned char key[16];
};

/**
*
*/
static struct CreateKeyInfoS* CreateCardKeyInfo(unsigned char *seed)
{

	char OldKey[20];
	struct CreateKeyInfoS *res = NULL;

	res = (struct CreateKeyInfoS *)malloc(sizeof(struct CreateKeyInfoS));
	memset(res, 0, sizeof(struct CreateKeyInfoS));
	memset(OldKey, 0, 20);

	int length = 18;
	Str2Bcd((const char *)seed, res->ID, &length);
	iGetKeyValue(seed, res->key);

	//
	return res;
}

/**
*
*/
static void DestroyCardKeyInfo(struct CreateKeyInfoS *KeyInfo)
{
	free(KeyInfo);
}

/**
*
*/
static int iWriteKey(unsigned char *seed)
{
	struct CreateKeyInfoS* KeyInfo = NULL; 

	printf("Seed:%s\n", seed);
	KeyInfo = CreateCardKeyInfo(seed);
	iCardCtlCard(2, KeyInfo);
	DestroyCardKeyInfo(KeyInfo);

	return 0;
}

/**
* 校验License
* @license: 校验的对象
* @return: 如果正确返回0，其他返回负值
*/
static int _CheckLicense(const char *license)
{
	return strcmp("北航冠新.license", (const char *)license);
}

/**
*
*/
static int iCreateListFile(ofstream &out, const vector<string>&  v)
{
	int colum; 
	// 从PIN这一列开始，打印出说有的列
	// Pin值列为29l列
	colum = 29;

	// 中间需要打印的列
	for(; colum < (int)(v.size()-1); colum++ )
	{
		out << v[colum].c_str() << "|";
	}
	out << v[colum].c_str() << endl;

	return 0;
}

static int iCreatePubSysListFile(ofstream &out, const vector<string>&  v)
{
	int colum; 
	// 从健康档案号这一列开始，打印出说有的列
	// 健康档案号值列为29列
	colum = 29;

	// 中间需要打印的列
	for(; colum < (int)(v.size()-1); colum++ )
	{
		out << v[colum].c_str() << "|";
	}
	out << v[colum].c_str() << endl;

	return 0;
}

// 
/**
*
*/
int __stdcall iCreateCardData( 
				const char *filename, 
				const char *license,
				const char *datafile, 
				const char *infofile
				)
{
	std::locale::global(std::locale(""));
	wstring delimit = L"|";
	char lines[1024];
	wchar_t wlines[1024];
	int result = 0;
	int length;
	char *xml = NULL;

	if(filename == NULL || license == NULL)
		return -1;

	result = iCardInit();
	if(result)
		return result;

	result = _CheckLicense(license);
	if(result)
		return result;

	// 改变卡数据文件
	iCardCtlCard(3, (void*)datafile);
	int nCount = 0;

	// 分配XML空间
	xml = (char *)malloc(4096);
	ifstream in(filename);
	ofstream out(infofile);
	if(in && out) 
	{
		while(in.getline(lines, 1024))
		{
			mbstowcs(wlines, lines, sizeof(lines));

			// 定义变量，组织vector信息
			wstring wstrlines = wlines;
			vector<wstring> wv = split(wstrlines, delimit);
			if(wv.size() == 0 && wv.size() < 28){
				result = -2;
				break;
			}

			vector<string> v;
			convertW2CVector(v, wv);
			// 初始化卡
			iCardCtlCard(1, NULL);

			if (v[0].c_str()[0] == '0')
			{
				// 写入KEY
				iWriteKey((unsigned char *)v[6].c_str());

				// 写入实际的内容
				memset(xml, 0, 4096);
				iCreateXmlByVector(v, xml, &length);

				// 创建信息列表文件
				iCreateListFile(out, v);
			} else {
				// 写入KEY
				iWriteKey((unsigned char *)v[0].c_str());

				// 写入实际的内容
				memset(xml, 0, 4096);
				iCreatePubSysXmlByVector(v, xml, &length);

				// 创建信息列表文件
				iCreatePubSysListFile(out, v);
			}


			if (iWriteInfo(xml) != 0){
				result = -5;
				break;
			}

			// 换卡
			iCardCtlCard(0, NULL);
			++nCount;
		}
	}
	else 
	{
		//打开文件失败
	}
	out.close();
	in.close();

	printf("共生成卡数据<%d>条\n",nCount);

	// 释放XML空间
	free(xml);

	return result;
}

DLL_EXPORT int __stdcall iCreateCardDataForEncry( 
	const char *filename, 
	const char *license,
	const char *datafile, 
	const char *infofile
	)
{
	std::locale::global(std::locale(""));
	wstring delimit = L"|";
	char lines[1024];
	wchar_t wlines[1024];
	int result = 0;
	int length;
	char *xml = NULL;

	if(filename == NULL || license == NULL)
		return -1;

	result = iCardInit();
	if (result != 0)
	{
		return result;
	}

	result = _CheckLicense(license);
	if(result)
		return result;

	// 改变卡数据文件
	iCardCtlCard(3, (void*)datafile);
	int nCount = 0;

	// 分配XML空间
	xml = (char *)malloc(4096);
	ifstream in(filename);
	ofstream out(infofile);
	if(in && out) 
	{
		CDESEncry encry;
		encry.DesryFile((char*)filename);
		for (int i=0; i<encry.GetMaxLines(); ++i)
		{
			encry.GetlineInfo(lines, i);
			
			mbstowcs(wlines, lines, sizeof(lines));

			// 定义变量，组织vector信息
			wstring wstrlines = wlines;
			vector<wstring> wv = split(wstrlines, delimit);
			if(wv.size() == 0 && wv.size() < 28)
			{
				result = -2;
				break;
			}
			
			vector<string> v;
			convertW2CVector(v, wv);

			// 初始化卡
			iCardCtlCard(1, NULL);

			if (v[0].c_str()[0] == '0')
			{
				// 写入KEY
				iWriteKey((unsigned char *)v[6].c_str());

				// 写入实际的内容
				memset(xml, 0, 4096);
				iCreateXmlByVector(v, xml, &length);

				// 创建信息列表文件
				iCreateListFile(out, v);
			}
			else
			{
				// 写入KEY
				iWriteKey((unsigned char *)v[0].c_str());

				// 写入实际的内容
				memset(xml, 0, 4096);
				iCreatePubSysXmlByVector(v, xml, &length);

				// 创建信息列表文件
				iCreatePubSysListFile(out, v);
			}


			if (iWriteInfo(xml) != 0)
			{
				result = -5;
				break;
			}

			// 创建信息列表文件
			iCreatePubSysListFile(out, v);

			// 换卡
			iCardCtlCard(0, NULL);
			++nCount;
		}
	}
	else 
	{
		//打开文件失败
	}
	out.close();
	in.close();

	printf("共生成卡数据<%d>条\n",nCount);

	// 释放XML空间
	free(xml);

	return result;
}



/**
*
*/
static int iGetWriteKey(unsigned char *key)
{
	int res = 0;
	unsigned char OrigID[9];

	memset(OrigID, 0, 9);
	iQueryInfo("MEDICARECERTIFICATENO",(char *)OrigID);
	iGetKeyBySeed(OrigID, key);

	return res;
}