// CreateLicenseTool.cpp : 定义控制台应用程序的入口点。
//


#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "../BHGX_CardLib/BHGX_CardLib.h"

#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)
using namespace std;

#define LICENSEFILE	"北航冠新.license"
#define LICENSEHISFILE	"北航冠新HIS.license"
void printHelp();

int main(int argc, char* argv[])
{
	printHelp();
	char szSelect[10];
	cout<<"请输入功能测试选项:";
	cin>>szSelect;

	int status = 0;
	char timeFMT[50];
	while (1)
	{
		if (strcmp(szSelect, "q") == 0 ||
			strcmp(szSelect, "quit") == 0)
			break;

		int nSelect = atoi(szSelect);
		switch (nSelect)
		{
		case 1:
			printf("请输入截止时间(格式如 2012-08-20_9:20:14):\n");
			scanf("%s", timeFMT);
			status = iCreateLicense(LICENSEFILE, timeFMT);
			if (status != 0)
				printf("%s\n",err(status));
			else 
				printf("Create license 成功\n");
			break;
		case 2:
			status = iCreateLicense(LICENSEHISFILE);
			if (status != 0)
				printf("%s\n",err(status));
			else 
				printf("Create license 成功\n");
			break;
		default:
			printHelp();
			break;
		}
		cout<<"请输入功能测试选项:";
		cin>>szSelect;
	}

	system("pause");

	return 0;
}

void printHelp()
{
	cout<<"************生成license程序******************"<<endl;
	cout<<"\t1:按时间产生License"<<endl;
	cout<<"\t2:产生授权License"<<endl;
	cout<<"\tq|quit:退出程序"<<endl;
}



