#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <time.h>
#include <vector>
#include "Markup.h"

#include "../BHGX_CardLib/BHGX_CardLib.h"

#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#endif

using namespace std;

static int ReadFile(char *filename, char **xml)
{
	FILE *handle;
	int size;

	fopen_s(&handle, filename, "r");

	if (NULL == handle)
		return -1;
	fseek(handle, 0, SEEK_END);
	size = ftell(handle) + 1;
	rewind(handle);
	*xml = (char *) malloc(size);
	memset(*xml, 0, size);
	fread(*xml,size,1,handle);
	fclose(handle);

	return size;
}

int get_printer_list(char *pinters, std::vector<std::string> &vecPrinter) {
	CMarkup xml;
	xml.SetDoc(pinters);
	if (!xml.FindElem("PrinterList")){
		return -1;
	}
	xml.IntoElem();
	while ( xml.FindElem("PrinterName")){
		vecPrinter.push_back(xml.GetAttrib("NAME"));
	}
	xml.OutOfElem();
	return vecPrinter.size();
}

void printByCardLib()
{
	char printers[1024];
	std::vector<std::string> vecPrinter;
	int size = iGetPrinterList(printers);
	if (size < 0) {
		cout << "无法获取打印机列表" << endl;
		goto done;
	}

	get_printer_list(printers, vecPrinter);
	for (int i=0 ; i < vecPrinter.size(); i++) {
		cout << i+1 << ":\t" << vecPrinter[i] << endl;
	}
	cout << "请输入打印机编号[1|2..]" <<endl;
	int printIndex;
	cin >> printIndex;

	char *buf, *readBuf;
	cout<<"卡面打印数据默认文件cardprintData.xml"<<endl;
	int ret = ReadFile("cardprintData.xml", &buf);
	if (ret == -1) {
		printf("cardprintData.xml文件不存在\n");
		goto done;
	}
	//cout << buf << endl;

	cout<<"卡面风格数据默认文件cardprint_652822.xml"<<endl;
	ret = ReadFile("cardprint_652822.xml", &readBuf);
	if (ret == -1) {
		printf("cardprint_652822.xml文件不存在\n");
		goto done;
	}
	//cout << readBuf << endl;
	printf("*************开始打印卡*************\n");

	printf("***********************************\n");
	int nStart = GetTickCount();
	ret = iPrintCard((char*)vecPrinter[printIndex - 1].c_str(), buf, readBuf);
	int nEnd = GetTickCount();
	printf("iPrintCard: %d    所需时间: %d \n", ret, nEnd - nStart);
	free(buf);
	free(readBuf);

done:
	getchar();
}

void printByRawLib()
{
	cout << "dayin" << endl;
}

void printHelp()
{
	cout<<"*************一卡通打印测试*********************"<<endl;
	cout<<"功能测试："<<endl;
	cout<<"\t1:根据cardlib动态库打印"<<endl;
	cout<<"\t2:根据P310e动态库打印"<<endl;
}

int main(int argc, char* argv[])
{
	std::locale::global(std::locale(""));

	printHelp();
	char szSelect[10];
	cout<<"请输入功能测试选项:";
	cin>>szSelect;
	std::string strSelect = szSelect;
	while (1)
	{
		if (strSelect == "q" || strSelect == "quit")
			break;

		int nSelect = atoi(szSelect);
		nSelect -- ;
		if (nSelect < 0 || nSelect > 2 ) {
		} else {
			if (nSelect == 0) {
				printByCardLib();
			} else if (nSelect == 1) {
				printByRawLib();
			}
		}
		memset(szSelect, 0 , sizeof(szSelect));
		printHelp();
		cout<<"\n请输入功能测试选项:";
		cin>>szSelect;
		strSelect = szSelect;
	}
	return 0;
}