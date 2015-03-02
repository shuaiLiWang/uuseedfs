#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include "ThreadObj.h"
#include "commonstruct.h"
#include "MutexObj.h"

typedef struct _tspeerinfo
{
	T_PGUID mid;
	char ip[MAX_IP_LEN];
	unsigned short port;

	_tspeerinfo()
	{
		mid		= NULL;
		ip[0]	= '\0';
		port	= 0;
	}

	bool operator == (const _tspeerinfo& arRes) const
	{
		if(this == &arRes)
			return true;
		
		return *this->mid == *arRes.mid;
	}

	bool operator < (const _tspeerinfo& arRes) const
	{
		if(this == &arRes)
			return false;

		return *this->mid < *arRes.mid;
	}

}T_SPEERINFO, *T_PSPEERINFO;

struct _peerCompareFunc
{
	bool operator()(T_PSPEERINFO aLeft, T_PSPEERINFO aRight) const
	{
		if(aLeft == aRight)
			return false;
		
		bool bRtn =  *aLeft->mid < *aRight->mid;
		return bRtn;
	}
};

typedef struct _tfileinfo
{
	T_PGUID cid;
	unsigned int timeout;
	time_t  tmAdd;
	int		iUnabled;

	_tfileinfo()
	{
		cid = NULL;
		timeout = 0;
		tmAdd = 0;
		iUnabled = 0;
	}

	bool operator == (const _tfileinfo& arRes) const
	{
		if(this == &arRes)
			return true;
		
		return *this->cid == *arRes.cid;
	}

	bool operator < (const _tfileinfo& arRes) const
	{
		if(this == &arRes)
			return false;

		return *this->cid < *arRes.cid;
	}

}T_FILEINFO, *T_PFILEINFO;

struct _fileCompareFunc
{
	bool operator()(T_PFILEINFO aLeft, T_PFILEINFO aRight) const
	{
		if(aLeft == aRight)
			return false;

		return *aLeft->cid < *aRight->cid;
	}
};

typedef enum _etasktype
{
	TASK_TYPE_NULL=0,
	TASK_TYPE_ADD_PEER=1,
	TASK_TYPE_ADD_FILE=2,
	TASK_TYPE_DEL_PEER=3
}E_TASKTYPE;

typedef struct _taddpeerpara
{
	T_PGUID mid;
	char    ip[16];
	unsigned short port;
}T_ADDPEERPARA, *T_PADDPEERPARA;

typedef struct _tdelpeerpara
{
	T_PGUID mid;
}T_DELPEERPARA, *T_PDELPEERPARA;

typedef struct _taddfilepara
{
	T_PGUID mid;
	T_PGUID cid;
	unsigned int timeout;
}T_ADDFILEPARA, *T_PADDFILEPARA;

typedef struct _tstattask
{
	E_TASKTYPE type;
	union
	{
		T_ADDPEERPARA addPeer;
		T_DELPEERPARA delPeer;
		T_ADDFILEPARA addFile;
	}para;
}T_STATTASK, *T_PSTATTASK;

typedef struct _tprintfile
{
	T_PGUID cid;
	char* pszName;
}T_PRINTFILE, *T_PPRINTFILE;

typedef std::list<T_PPRINTFILE> LST_PPRINTFILE;
typedef LST_PPRINTFILE::iterator ITR_LST_PPRINTFILE;

typedef std::list<T_PSTATTASK> LST_PSTATTASK;

typedef std::map<T_PFILEINFO, void*, _fileCompareFunc> MAP_PFILEINFO;
typedef MAP_PFILEINFO::iterator		ITR_MAP_PFILEINFO;
typedef std::map<T_PSPEERINFO, void*, _peerCompareFunc > MAP_PSPEERINFO;
typedef MAP_PSPEERINFO::iterator		ITR_MAP_PSPEERINFO;


//typedef std::map<T_PFILEINFO, int>				MAP_FILE_INT;
//typedef MAP_FILE_INT::iterator					ITR_MAP_FILE_INT;

typedef std::map<T_PSPEERINFO, MAP_PFILEINFO*,  _peerCompareFunc>	MAP_PEER_FILELIST;
typedef MAP_PEER_FILELIST::iterator				ITR_MAP_PEER_FILELIST;
typedef std::map<T_PFILEINFO, MAP_PSPEERINFO*, _fileCompareFunc>	MAP_FILE_PEERLIST;
typedef MAP_FILE_PEERLIST::iterator				ITR_MAP_FILE_PEERLIST;

class CStatistics
{
public:
	CStatistics();
	virtual ~CStatistics();
	
public:
	int addPeer(const T_PGUID mid, const char* ip, unsigned short port );
	void delPeer(const T_PGUID mid);
	int addFile(const T_PGUID fid, const T_PGUID mid, unsigned short cmid, unsigned int timeout);
	void PrintFileInfo(const T_PGUID cid, const char* acpszName);
	int Init(const char* path, unsigned int frequence);
	int Start();
	void Stop();

	unsigned int GetFileSize(){ m_mapFileAndPeerList.size(); }
	unsigned int GetPeerSize(){ m_mapPeerAndFileList.size(); }

private:
	static void* WorkerThreadEntry(void* pParam); 
	void WorkerThreadImp();

	static void* PrintFileThreadEntry(void* pParam);
	void PrintFileThreadImp();

	int AddPeerToTotalList(const T_PGUID mid, const char* ip, unsigned short port );
	int DelPeerFromTotalList(const T_PGUID mid);
	int AddFileToTotalList(const T_PGUID fid, const T_PGUID mid, unsigned short cmid, unsigned int timeout);

	int AddStatTaskToTotalListAndReleaseTask( T_PSTATTASK pStatTask);
	
	int WritePeerInfoToFile(FILE* pFile, T_PSPEERINFO pPeerInfo);
	int WriteFileInfoToFile(FILE* pFile, T_PFILEINFO pFileInfo);
	int WriteFileMapToFile(FILE* pFile, T_PPRINTFILE pPrintFile);

private:
	string			m_strPath;
	unsigned int	m_uiFrequence;

	CThreadObj	m_threadWorker;
	TCallbackFuncObj<pThreadCallbackFunc>	m_funcWorker;

	CMutexObj			m_muxTotalMap;
	MAP_PEER_FILELIST	m_mapPeerAndFileList;
	MAP_FILE_PEERLIST	m_mapFileAndPeerList;

	CMutexObj		 m_muxStatTask;
	LST_PSTATTASK	 m_lstStatTask;

	CMutexObj			m_muxPrintFile;	
	LST_PPRINTFILE		m_lstPrintFile;
	CThreadObj			m_threadPrintFile;
	TCallbackFuncObj<pThreadCallbackFunc>	m_funcPrintFile;
};


#endif //__STATISTICS_H__
