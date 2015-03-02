
#ifndef __DFS_MANAGER_H__
#define __DFS_MANAGER_H__


#include "ThreadObj.h"
#include "filenotify.h"
#include "peerlist.h"
#include "udpsocket.h"
#include "dhtdataset.h"
#include "statistic.h"

class CDfsManager
{
public:
	
	CDfsManager();
	virtual ~CDfsManager();

public:
	bool Init();

	bool Start();
	void Stop();
	void Join();
	void UpdateLocalPeerList(bool bSendJoinMsg=false);

	void Uninit();

	string GetStatus();

private:

	void FindAnDelFile();
	void FindAnAddFile(char* pDfsProtocolStoreInfo, char* pDfsProtocolStoreInfoFileName);
	void ReSendDhtData();

	static	void* FileWatcherEntry(void* pParam);
	void FileWatcherImp();

	static void* UdpProcessEntry(void* pParam);
	void UdpProcessImp();

	static void* PeerListEntry(void* pParam);
	void PeerListImp();
	
	static void* WriteStatusEntry(void* pParam);
	void WriteStatusImp();
	
	static void* PeerListCheckEntry(void* pParam);
	void PeerListCheckImp();
private:

	void ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessStoreReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessStoreRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessRemoveReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessRemoveRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessStoreInfoMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSocket);
	void ProcessFindReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessFindRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessGetReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessGetRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessUserListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessUserListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessPeerJoinReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessPeerJoinRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessPeerLeaveReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);
	void ProcessPeerLeaveRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket);

	void SendPingReqMsgToPeers();
	void SendUserListRspMsgToPeers();
	void SendUserListToPeer(unsigned int uiIP, unsigned short usPort);

	void AddPeerToStatistics(const T_PPEERINFO cpPeerInfo);

	CThreadObj								m_threadFileWatcher;
	TCallbackFuncObj<pThreadCallbackFunc>	m_funcFileWatcher;

	CThreadObj								m_threadUdpProcess;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcUdpProcess;

	CThreadObj								m_threadPeerList;
	TCallbackFuncObj<pThreadCallbackFunc>	m_funcPeerList;

	CThreadObj								m_threadWriteStatus;
	TCallbackFuncObj<pThreadCallbackFunc>	m_funcWriteStatus;

	CThreadObj								m_threadPeerListCheck;
	TCallbackFuncObj<pThreadCallbackFunc>   m_funcPeerListCheck;

	CFileNotify		m_FileNotify;
	CUdpSocket      m_UdpSocket;
	CUdpSocket      m_UdpSocketDouble;

	CPeerList		m_PeerList;
	CMutexObj		m_muxPeerList;

	CDhtDataSet     m_DhtDataSet;
	CMutexObj		m_muxDataSet;

	CStatistics*	m_pStatistic;
};


#endif //__DFS_MANAGER_H__
