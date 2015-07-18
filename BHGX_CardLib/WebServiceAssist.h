#ifndef _WEBASSIST_H
#define _WEBASSIST_H
#include <string>
#include <map>

using namespace std;

class WebServiceUtil
{
public:
	WebServiceUtil(void);
	~WebServiceUtil(void);
	WebServiceUtil(char * strCheckWSDL, char* strServerURL);

	bool isValidUrl(char * url) { return (url != NULL && strlen(url) > 0); }

	int NHCheckValid(std::string strCardNO, char *pszXml);
	int NHRegCard(std::string strCardNO, char *pszXml);
	bool IsMedicalID(const std::string &strID);
protected:
	int  CreateCheckWsdlParams(const char *CardID, const char *pszCardCheckWSDL, char *strParams);
	int  CreateRegWsdlParams(const char *CardID, char *strParams);

	int  GetCheckState(const std::string &strXML, std::string &strRetCode, std::string &strSec);
	int  FormatWriteInfo(const char *strWrite, char *strFMTWrite);
	int  GetCheckRetDesc(const std::string &strStatus, std::string &strDesc);
	int  GetCardStatus(int nStatus, std::string &strStatus);
	int  _FormatWrite(std::map<int, std::string> &mapAll, char *strFMTWrite);

private:
	std::string m_strCheckWSDL;
	std::string m_strServerURL;
	std::map<std::string, std::string> m_mapCodeDesc;
	std::map<int, std::string> m_mapCardStatus;
};

#endif
