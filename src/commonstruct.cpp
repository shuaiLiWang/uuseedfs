
#include "commonstruct.h"
#include "md5.h"
#include "Ini.h"

T_CONFFILE	g_confFile;

CCommonStruct::CCommonStruct()
{
}

CCommonStruct::~CCommonStruct()
{
}

void CCommonStruct::GetGUID(char* acpBuf, unsigned int auiBufLen, T_PGUID pID)
{
	//get und path
	MD5_CTX  md5;
	string sReqPath = acpBuf;
	string sMd5Path = "";
	int iPos = sReqPath.rfind("/");
	if(iPos != string::npos)
	{
		string sHashBasePath = sReqPath.substr(0, iPos + 1);
		string sHashFileName = sReqPath.substr(iPos + 1, sReqPath.length() - iPos);
		iPos = sHashFileName.find(".");
		if(iPos != string::npos)
		{
			string sHashFileNameBase = sHashFileName.substr(0, iPos);
			string sSuffixName       = sHashFileName.substr(iPos, sHashFileName.length() - iPos);
			
			if(sSuffixName.find(".unp") != string::npos || (sSuffixName.find(".und") != string::npos && sHashFileNameBase.find("0000-0000") != string::npos))
			{
				string tempPath = sHashFileNameBase.substr(0, sHashFileNameBase.length() - 2);
	
			//	sMd5Path = sHashBasePath;
				sMd5Path += tempPath;
				sMd5Path += "00";
			}
			else if( sSuffixName.find(".mp4") != string::npos || 
					 sSuffixName.find(".ucf") != string::npos ||
					 sSuffixName.find(".wmv") != string::npos ||
					 sSuffixName.find(".zip") != string::npos ||
					 sSuffixName.find(".exe") != string::npos ||
					 sSuffixName.find(".rm")  != string::npos ||
					 sSuffixName.find(".rmvb") != string::npos ||
					 sSuffixName.find(".uni") != string::npos ||
					 sSuffixName.find(".m3u8") != string::npos
					)
			{
			//	sMd5Path = sHashBasePath;
				sMd5Path += sHashFileNameBase;
			}
			else if(sSuffixName.find(".unt") != string::npos)
			{
				int iUntPos = sHashFileNameBase.find("_");
				
				string tempPath = sHashFileNameBase.substr(0, iUntPos);
			//	sMd5Path = sHashBasePath;
				sMd5Path += tempPath;
			}
			else
			{
				sMd5Path = acpBuf;
			}
		}
		else
		{
			sMd5Path = acpBuf;
		}
	}
	else
	{
		sMd5Path = acpBuf;
	}

	//printf("...file:%s \nMd5File:%s \n", acpBuf, sMd5Path.c_str());

	md5.MD5Update((unsigned char*)sMd5Path.c_str(), sMd5Path.length());
/*
//	string sUnpPath = acpBuf;
//	int iPos = sUnpPath.rfind(".unp");
	if(iPos != string::npos)
	{	
		string sUndPath = sUnpPath.substr(0, iPos - 2);
		sUndPath += "00.und";
	
		printf("unp:%s und:%s\n", sUnpPath.c_str(), sUndPath.c_str());
		md5.MD5Update((unsigned char*)sUndPath.c_str(), auiBufLen);
	}
	else
	{
		md5.MD5Update((unsigned char*)acpBuf, auiBufLen);
	}
*/
	md5.MD5Final(pID->pID);

	iPos = sReqPath.rfind(":0000");
	if(iPos != string::npos)
	{
		pID->pID[0]	= 0;
		pID->pID[1] = 0;
		pID->pID[2] = 0;
		pID->pID[3] = 0;
	}
}

int CCommonStruct::CompareGUID(const T_GUID& arguidFirst, const T_GUID& arguidSecond)
{
	const unsigned char* pFirst = NULL;
	const unsigned char* pSecond = NULL;
	for(int i=0; i<DEF_GUID_LEN; i++)
	{
		pFirst = arguidFirst.pID+i;
		pSecond = arguidSecond.pID+i;
		if(*pFirst>*pSecond)
			return 1;
		if(*pFirst<*pSecond)
			return -1;
	}
	return 0;
}

void CCommonStruct::GUIDToStr(char* getStr, T_PGUID pguid)
{
	char ucBuf[10] = {0};

	unsigned int uiNum = 0;
	for(uiNum; uiNum < 16; uiNum ++)
	{
		sprintf(ucBuf, "%02x", pguid->pID[uiNum]&0x0ff);
		strcat(getStr, ucBuf);	
	}
}

T_GUID DistanceGuid(const T_GUID& arLeft, const T_GUID& arRight)
{
	T_GUID guidDistance;
	T_GUID left			= arLeft;
	T_GUID right		= arRight;
	if(arLeft<arRight)
	{
		left = arRight;
		right =  arLeft;
	}

	for(int i=0; i<DEF_GUID_LEN; i++)
	{
		guidDistance.pID[i] = left.pID[i] - right.pID[i];
	}

	return guidDistance;
}

void CCommonStruct::ReplaceAll(string& str,const string& old_value,const string& new_value)
{
     if (old_value.empty())
           return;

     int Pos = 0;
     while ((Pos = str.find(old_value, Pos)) != string::npos) {
	     str.erase(Pos, old_value.size());
     str.insert(Pos, new_value);
	 Pos += new_value.size();
	}
}

void CCommonStruct::ReparePath(string& astrPath)
{
#ifdef WIN32
     ReplaceAll(astrPath, "/", "\\");
#else
     ReplaceAll(astrPath, "\\", "/");
#endif
}
	
bool CCommonStruct::ReadConfig(const char* acpszConfigFilePath)
{
	CIniFile iniFile;
	bool bRtn = iniFile.open(acpszConfigFilePath);
	if(!bRtn)
		return false;

	string strTemp("");
	strTemp = iniFile.read("local", "ip");
	if(strTemp.empty())	
		strcpy(g_confFile.pszLocalIP, "0.0.0.0");
	else
		strcpy(g_confFile.pszLocalIP, strTemp.c_str());

	strTemp = iniFile.read("local", "ipdouble");
	if(strTemp.empty())	
		strcpy(g_confFile.pszLocalIPDouble, "");
	else
		strcpy(g_confFile.pszLocalIPDouble, strTemp.c_str());

	strTemp = iniFile.read("local", "port");
	if(strTemp.empty())
		g_confFile.usLocalPort = 4879;
	else
		g_confFile.usLocalPort = atoi(strTemp.c_str());
	
	strTemp = iniFile.read("local", "portdouble");
	if(strTemp.empty())
		g_confFile.usLocalPortDouble = 0;
	else
		g_confFile.usLocalPortDouble = atoi(strTemp.c_str());
	
	strTemp = iniFile.read("local", "frequence");
	if(strTemp.empty())
		g_confFile.uiFrequence = 0;
	else
		g_confFile.uiFrequence = atoi(strTemp.c_str());

	strTemp = iniFile.read("local", "watchdir");
	if(strTemp.empty())
		g_confFile.strWatchDir = "/";
	else
		g_confFile.strWatchDir = strTemp;

	strTemp = iniFile.read("local", "mid");
	if(strTemp.empty())
	{
		char pszMID[32]="";
		sprintf(pszMID, "%s:%d", g_confFile.pszLocalIP, g_confFile.usLocalPort);
		g_confFile.strMID = pszMID;
	}
	else
		g_confFile.strMID = strTemp;
	CCommonStruct::GetGUID((char*)g_confFile.strMID.c_str(), g_confFile.strMID.length(), &g_confFile.tMGuid);

	strTemp = iniFile.read("local", "middouble");
	if(strTemp.empty())
	{
		g_confFile.strMIDDouble = "";
		memset(g_confFile.tMGuidDouble.pID, 0, DEF_GUID_LEN);
	}
	else
	{
		g_confFile.strMIDDouble = strTemp;
		CCommonStruct::GetGUID((char*)g_confFile.strMIDDouble.c_str(), g_confFile.strMIDDouble.length(), &g_confFile.tMGuidDouble);
	}

	strTemp = iniFile.read("local", "logdir");
	if(strTemp.empty())
		g_confFile.strLogDir = "/var/log/";
	else
		g_confFile.strLogDir = strTemp;

	strTemp = iniFile.read("local", "scanfileperiod");
	if(strTemp.empty())
		g_confFile.uiScanFilePeriod = 3600;
	else
		g_confFile.uiScanFilePeriod = atoi(strTemp.c_str());
	

	//read peer information
	int iIndex=0;
	char szSection[16]="";
	char szKey[32]="";
	char szPeerIP[16]="";
	unsigned short usPeerPort=0;
	while(1)		
	{
		iIndex++;
		sprintf(szSection, "peer%d", iIndex);
		sprintf(szKey, "ip%d", iIndex);
		strTemp = iniFile.read(szSection, szKey);
		if(strTemp.empty())
			break;

		T_PPEERCONF pPeerInfo = new T_PEERCONF;
//		strcpy(pPeerInfo->tPeerAddr.pszIP, strTemp.c_str());	
		pPeerInfo->tPeerAddr.uiIP = inet_addr(strTemp.c_str());

		sprintf(szKey, "port%d", iIndex);
		strTemp = iniFile.read(szSection, szKey);
		if(strTemp.empty())
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
			continue;
		}
		pPeerInfo->tPeerAddr.uiPort = atoi(strTemp.c_str());
		g_confFile.vecPeerConf.push_back(pPeerInfo);
	}

	g_confFile.bReadConfFlag = true;
	return true;
}

char* CCommonStruct::Time2String(time_t time1)
{
	static char szTime[1024]="";
	memset(szTime, 0, 1024);
	struct tm tm1;  
    localtime_r(&time1, &tm1 );  
	sprintf( szTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",  
	               tm1.tm_year+1900, tm1.tm_mon+1, tm1.tm_mday,  
	               tm1.tm_hour, tm1.tm_min,tm1.tm_sec);  
	return szTime;
}

string CCommonStruct::GUIDToString(const T_GUID& arGuid)
{
	char buf[100];
     sprintf( buf, "{%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X}",
			arGuid.pID[0],  arGuid.pID[1],  arGuid.pID[2],  arGuid.pID[3], 
			arGuid.pID[4],  arGuid.pID[5],  arGuid.pID[6],  arGuid.pID[7], 
			arGuid.pID[8],  arGuid.pID[9],  arGuid.pID[10], arGuid.pID[11], 
			arGuid.pID[12], arGuid.pID[13], arGuid.pID[14], arGuid.pID[15]);
     return string(buf);
}

T_GUID StringToGUID(const char *str)
{
	int t1,t2,t3,t4;
	T_GUID guid;
	sscanf(str,
	      "{%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X}",
	         &guid.pID[0],  &guid.pID[1],  &guid.pID[2],  &guid.pID[3], 
	         &guid.pID[4],  &guid.pID[5],  &guid.pID[6],  &guid.pID[7], 
	         &guid.pID[8],  &guid.pID[9],  &guid.pID[10],  &guid.pID[11], 
	         &guid.pID[12],  &guid.pID[13],  &guid.pID[14],  &guid.pID[15] );

	return guid;
}

