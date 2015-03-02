
#ifndef __COMMON_STRUCT_H__
#define __COMMON_STRUCT_H__

#include <map>
#include <list>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/time.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/errno.h>

using namespace std;
#define DEF_GUID_LEN	16
#define DEF_ONE_DAY     (60 * 60 * 24)
#define MAX_IP_LEN		16
#define MAX_RECV_BUF_LEN 1024
#define MAX_SEND_BUF_LEN 1024
#define MAX_FILE_NAME_LEN 1024


typedef struct _tpeeraddress
{
	//char pszIP[MAX_IP_LEN];
	uint32_t	uiIP;
	unsigned int uiPort;
}T_PEERADDRESS, *T_PPEERADDRESS;

typedef struct _tguid
{
	unsigned char pID[DEF_GUID_LEN];

	_tguid operator = (const _tguid& arRes)
	{
		if(this != &arRes)
		{
			for(int i=0; i<DEF_GUID_LEN; i++)
				pID[i] = arRes.pID[i];
		}
		return *this;
	}

	bool operator == (const _tguid& arRes) const
	{
		if(this == &arRes)
			return true;

		for(int i=0; i<DEF_GUID_LEN; i++)
		{
			if(pID[i] != arRes.pID[i])
				return false;
		}
		return true;
	}

	bool operator < (const _tguid& arRes) const
	{
		if(this == &arRes)
			return false;

		for(int i=0; i<DEF_GUID_LEN; i++)
		{
			if(pID[i] < arRes.pID[i])
			{
				return true;
			}
			else if(pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

	bool operator > (const _tguid& arRes)
	{
		if(this == &arRes)
			return false;

		for(int i=0; i<DEF_GUID_LEN; i++)
		{
			if(pID[i] > arRes.pID[i])
			{
				return true;
			}
			else if(pID[i] == arRes.pID[i])
			{
				continue;
			}
			else
				return false;

		}
		return false;
	}

}T_GUID, *T_PGUID;

T_GUID DistanceGuid(const T_GUID& arLeft, const T_GUID& arRight);


typedef struct _tfilenode
{
        string        fileName;
		T_GUID		  tGuid;
        unsigned int  fileLen;
		
		_tfilenode()
		{
			fileName = "";
			memset(tGuid.pID, 0, DEF_GUID_LEN);	
			fileLen = 0;
		}
		_tfilenode& operator = (const _tfilenode &arRes)
		{
			if(this != &arRes)
			{
				this->fileName = arRes.fileName;
				this->tGuid		= arRes.tGuid;
//				memcpy(this->tGuid.pID, arRes.tGuid.pID, DEF_GUID_LEN);
				this->fileLen = arRes.fileLen;
			}
			return *this;
		}

}T_FILENODE, *T_PFILENODE;

typedef struct _tpeerconf
{
	T_PEERADDRESS tPeerAddr;
}T_PEERCONF, *T_PPEERCONF;

typedef std::vector<T_PPEERCONF>	VEC_T_PPEERCONF;
typedef VEC_T_PPEERCONF::iterator   ITR_VEC_T_PPEERCONF;


typedef struct _tconffile
{
	bool			bReadConfFlag;
	char			pszLocalIP[MAX_IP_LEN];
	unsigned int    uiLocalIP;
	unsigned short	usLocalPort;
	unsigned int    uiFrequence;
	string			strWatchDir;
	string			strLogDir;
	string			strMID;
	unsigned int	uiScanFilePeriod;    //seconds
	T_GUID			tMGuid;

	string			strMIDDouble;	
	T_GUID			tMGuidDouble;
	char			pszLocalIPDouble[MAX_IP_LEN];
	unsigned int    uiLocalIPDouble;
	unsigned short  usLocalPortDouble;

	VEC_T_PPEERCONF vecPeerConf;
}T_CONFFILE, *T_PCONFFILE;

class CCommonStruct
{
private:
	
	CCommonStruct();
	virtual ~CCommonStruct();

public:
	static void GetGUID(char* acpBuf, unsigned int auiBufLen, T_PGUID pguid);
	static int CompareGUID(const T_GUID& arguidFirst, const T_GUID& arguidSecond);
	static void GUIDToStr(char* getStr, T_PGUID pguid);
	static string GUIDToString(const T_GUID& arGuid);

	static void ReplaceAll(string& str,const string& old_value,const string& new_value);
	static void ReparePath(string& astrPath);

	static bool ReadConfig(const char* acpszConfigFilePath);

	static char* Time2String(time_t time1);
private:
};

extern T_CONFFILE	g_confFile;

#endif //__COMMON_STRUCT_H__
