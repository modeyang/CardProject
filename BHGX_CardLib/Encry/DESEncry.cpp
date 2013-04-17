
#include <stdio.h>
#include "DESEncry.h"
#include "DES.h"
#include "../public/Markup.h"

using namespace std;
#pragma warning (disable : 4996)

#define SAFE_DELETE(a)  if (a != NULL) { free(a);a = NULL;}

const char C_Key[] = 
{
	'1','2','3','4','5','6','7','8'
};

CDESEncry::CDESEncry(void)
:m_pfileContent(NULL)
{
}

CDESEncry::~CDESEncry(void)
{
	SAFE_DELETE(m_pfileContent);
}

bool CDESEncry::EncryFile(char *szFilename)
{
	std::locale::global(std::locale(""));
	std::string szInFile = szFilename;

	FILE *ff, *fp;
	ff = fopen(szFilename,"rb");
	if (ff == NULL)
	{
		//szInFile =  CMarkup::UTF8ToGB2312(szFilename);
		ff = fopen(szInFile.c_str(),"rb");
		if (ff == NULL)
		{
			return false;
		}
		
	}

	std::string strOutFile = szInFile;
	Transform_file(strOutFile, true);	
	fp = fopen(strOutFile.c_str(),"wb");
	if (fp == NULL)
	{
		fclose(ff);
		return false;
	}

	fseek(ff,0,SEEK_END);
	int fsize=ftell(ff);						//获得文件大小(相对于文件首的位移)
	fseek(ff,0,SEEK_SET);						
	fwrite(&fsize,sizeof(int),1,fp);			//写入文件大小
	//fwrite(C_Key,sizeof(char),sizeof(C_Key),fp);	//写入文件类型

	//开始加密
	int nRead;
	DES jm;
	char inbuff[8],outbuff[8];
	while(!feof(ff))
	{
		nRead = fread(inbuff,sizeof(char),8,ff);
		jm.Des_one(outbuff,inbuff,C_Key,ENCRYPT);	
		fwrite(outbuff,sizeof(char),8,fp);		//输出分密文分组
	}
	fclose(fp);
	fclose(ff);
	return true;
}

bool CDESEncry::DesryFile(char *szFilename)
{
	std::locale::global(std::locale(""));
	FILE *ff;
	char inbuff[8],outbuff[8];

	ff = fopen(szFilename, "rb");

	if(ff==NULL)
	{
		//std::string szInFile =  CMarkup::UTF8ToGB2312(szFilename);
		ff = fopen(szFilename, "rb");
		if (NULL == ff)
		{
			return false;
		}
	}
		
	int fsize;
	fread(&fsize,sizeof(int),1,ff);			//获得明文文件大小

	int nTotalsize = fsize;
	fsize = fsize%8 ? (fsize/8+1)*8 : fsize;

	//fread(initbuff,sizeof(char),sizeof(initbuff),ff);	//获得明文件的类型、加密重数、加密模式

	m_pfileContent = (char*)malloc(sizeof(char)*fsize+1);

	//开始解密
	DES jm;
	int nStart = 0;
	while(fsize > 0)		//请参考加密部分
	{
		int nRead = (int)fread(inbuff,sizeof(char),8,ff);

		jm.Des_one(outbuff,inbuff,C_Key,DECRYPT);

		if (fsize > 8)
		{
			memcpy(m_pfileContent + nStart, outbuff, 8);
			nStart += 8;
		}
		else
		{
			memcpy(m_pfileContent + nStart, outbuff, fsize);	//输出明文，不包括最后一个分组
			nStart += nRead;
		}

		fsize -= nRead;		//保存剩余尚未输出的明文大小		
	}
	m_pfileContent[nTotalsize] = 0;
	fclose(ff);

	GetvecInfoPos();
	return true;
}

int CDESEncry::GetlineInfo(char *szRead, int nPos)
{
	if (nPos + 2 > (int)m_vecInfoPos.size())
	{
		return -1;
	}
	int nStart = m_vecInfoPos[nPos];
	int nEnd = m_vecInfoPos[nPos+1];
	int nLen = nEnd - 1 - nStart;
	memcpy(szRead, m_pfileContent + nStart, nLen);
	szRead[nLen] = 0;
	return 0;
}

bool CDESEncry::EncryString(char *szSource, char *szDest)
{
	char inbuff[8],outbuff[8];
	int nSourceLen = (int)strlen(szSource);
	int nStart = 0;
	DES jm;
	while (nStart < nSourceLen)
	{
		memcpy(inbuff, szSource + nStart, sizeof(inbuff));
		jm.Des_one(outbuff,inbuff,C_Key,ENCRYPT);
		memcpy(szDest + nStart, outbuff, sizeof(outbuff));
		nStart += sizeof(inbuff);
	}

	if (nSourceLen % 8 != 0)
	{
		nStart -= sizeof(inbuff);
		memcpy(inbuff, szSource + nStart, nSourceLen - nStart);
		jm.Des_one(outbuff,inbuff,C_Key,ENCRYPT);
		memcpy(szDest + nStart, outbuff, nSourceLen - nStart);
	}
	return true;
}

bool CDESEncry::DescryString(char *szSource, char *szDest)
{
	char inbuff[8],outbuff[8];
	int nSourceLen = (int)strlen(szSource);
	int nStart = 0;
	DES jm;
	while (nStart < nSourceLen)
	{
		memcpy(inbuff, szSource + nStart, sizeof(inbuff));
		jm.Des_one(outbuff,inbuff,C_Key,DECRYPT);
		memcpy(szDest + nStart, outbuff, sizeof(outbuff));
		nStart += sizeof(inbuff);
	}

	if (nSourceLen % 8 != 0)
	{
		nStart -= sizeof(inbuff);
		memcpy(inbuff, szSource + nStart, nSourceLen - nStart);
		jm.Des_one(outbuff,inbuff,C_Key,DECRYPT);
		memcpy(szDest + nStart, outbuff, nSourceLen - nStart);
	}

	return true;
}

void CDESEncry::Transform_file(std::string &strOutfile, bool bEncry)
{
	size_t pos = strOutfile.find_last_of("\\");
	std::string strfilename = strOutfile;
	if (pos != -1)
	{
		strfilename = strOutfile.substr(pos+1, strOutfile.size());
		strOutfile = strOutfile.substr(0, pos);
	}
	pos = strfilename.find("_");
	if (bEncry && pos == -1)
	{
		strfilename = "JM_" + strfilename;
	}

	if (!bEncry && pos != -1)
	{
		strfilename = strfilename.substr(pos+1, strfilename.size());
	}

	strOutfile = strOutfile + "\\" + strfilename;
}

int CDESEncry::GetvecInfoPos()
{	
	int nlen = (int)strlen(m_pfileContent);
	m_vecInfoPos.push_back(0);
	for (int i=0; i<nlen; ++i)
	{
		if (m_pfileContent[i] == '\n')
		{
			m_vecInfoPos.push_back(i+1);
		}
	}
	m_vecInfoPos.push_back(nlen + 1);
	return (int)m_vecInfoPos.size();
}



