// InfoProcess.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "ConvertUtil.h"

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
static int iCreateListFile(ofstream &out, const vector<string>&  v)
{
	int colum; 
	// 从PIN这一列开始，打印出说有的列
	// Pin值列为29l列
	colum = 0;

	// 中间需要打印的列
	for(; colum < (int)(v.size()-1); colum++ )
	{
		out << v[colum].c_str() << "|";
	}
	out << v[colum].c_str() << endl;

	return 0;
}

static int ConvertInfo(std::vector<std::string> &vecInfo)
{
	std::string &strName = vecInfo[8];
	strName = CConvertUtil::uf_gbk_int_covers(strName.c_str(), "toint");
	std::string &strConName = vecInfo[23];
	strConName = CConvertUtil::uf_gbk_int_covers(strConName.c_str(), "toint");
	return 0;
}

int main(int argc, char* argv[])
{
	std::locale::global(std::locale(""));
	string delimit = "|";
	int nCount = 0;
	char lines[1024];
	char filename[256], outfile[256];
	memset(filename, 0, sizeof(filename));
	memset(outfile, 0, sizeof(outfile));

	cout<<"请输入需要处理的文件名:";
	cin>>filename;
	cout<<"请输入输出文件名:";
	cin>>outfile;

	ifstream in(filename);
	ofstream out(outfile);
	if(in && out) 
	{
		while(in.getline(lines, 1024))
		{
			// 定义变量，组织vector信息
			string strlines = lines;
			vector<string> v = split(strlines, delimit);
			if(v.size() == 0 && v.size() < 28)
				return -2;

			ConvertInfo(v);

			// 创建信息列表文件
			iCreateListFile(out, v);

			++nCount;
		}
	}
	else 
	{
		cout<<"打开文件失败"<<endl;
	}
	out.close();
	in.close();

	cout<<"处理完成, 共:"<<nCount<<"条记录"<<endl;
	getchar();
	return 0;
}

