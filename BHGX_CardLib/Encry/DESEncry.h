#pragma once
#include <string>
#include <vector>

using namespace std;

class CDESEncry
{
public:
	CDESEncry(void);

	~CDESEncry(void);

	bool EncryFile(char *szFilename);

	bool DesryFile(char *szFilename);

	char *GetDescryContent()
	{
		return m_pfileContent;
	}

	int GetMaxLines()
	{
		return (int)m_vecInfoPos.size()-1;
	}

	int GetlineInfo(char *szRead, int nPos);

	bool EncryString(char *szSource, char *szDest);

	bool DescryString(char *szSource, char *szDest);

protected:

	void Transform_file(std::string &strOutfile, bool bEncry = true);

	int GetvecInfoPos();

protected:
	char *m_pfileContent;

	std::vector<int> m_vecInfoPos;
};
