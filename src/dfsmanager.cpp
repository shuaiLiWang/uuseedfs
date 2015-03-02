
#include "dfsmanager.h"
#include "CallbackFuncObj.cpp"
#include "commonstruct.h"
#include "dfsprotocol.h"
#include "Log.h"

#include "log_agent_const.h"
#include "log_agent.h"

#define MAX_SEND_USERLIST_RSP_PERIOD	(60*60)
//#define MAX_SEND_USERLIST_RSP_PERIOD	(60*60*2)
#define MAX_RESOURCE_STORE_COUNT		5
#define MAX_SPEC_RESOURCE_STORE_COUNT		3
#define MAX_RECV_UDP_BUF_LEN			(30*1024)
#define MAX_PING_EXPIRE					60

typedef struct _tdfsmanagerstatus
{
	unsigned int uiGetReqMsgCount;
	unsigned int uiGetRspMsgSuccessedCount;
	unsigned int uiGetRspMsgFailedCount;
	unsigned int uiSendStoreMsgCount;
	unsigned int uiRecvStoreMsgCount;
	unsigned int uiSendFindMsgCount;
	unsigned int uiRecvFindMsgCount;

	void clearStatus()
	{
		uiGetReqMsgCount=0;
		uiGetRspMsgSuccessedCount=0;
		uiGetRspMsgFailedCount=0;
		uiSendStoreMsgCount=0;
		uiRecvStoreMsgCount=0;
		uiSendFindMsgCount=0;
		uiRecvFindMsgCount=0;
	}
	_tdfsmanagerstatus()
	{
		clearStatus();
	}
}T_DFSMANAGERSTATUS, *T_PDFSMANAGERSTATUS;


bool g_bPeerListIsSet = false;
T_DFSMANAGERSTATUS g_tDfsManagerStatus;
extern LOG_AGENT g_tLogStatus;

CDfsManager::CDfsManager() : m_pStatistic(NULL)
{
}

CDfsManager::~CDfsManager()
{
}

bool CDfsManager::Init()
{
	/*
	//+++++++++++test+++++++++++++
	T_PPEERINFO pPeerInfo = NULL;
	char szGuid[128];
	for(int i=0; i<3; i++)
	{
		pPeerInfo = new T_PEERINFO;
		memset(szGuid, 0, 128);

		sprintf(pPeerInfo->pszIP, "10.1.15.%d", i);
		pPeerInfo->usPort = i;
		
		sprintf(szGuid, "%s:%d", pPeerInfo->pszIP, pPeerInfo->usPort);

		CCommonStruct::GetGUID(szGuid, strlen(szGuid), &(pPeerInfo->guid));
	
		m_PeerList.AddPeer(pPeerInfo);
	}

	int iPeerListSize=0;
	pPeerInfo = m_PeerList.CreatePeers(iPeerListSize);
	//++++++++++++test+++++++++++++
	*/

	m_funcFileWatcher.Set(FileWatcherEntry, this);
	m_funcUdpProcess.Set(UdpProcessEntry, this);
	m_funcPeerList.Set(PeerListEntry, this);
	m_funcWriteStatus.Set(WriteStatusEntry, this);
	m_funcPeerListCheck.Set(PeerListCheckEntry, this);

	if(!g_confFile.bReadConfFlag)
		return false;

	if(0 != g_confFile.uiFrequence)
	{
		m_pStatistic = new CStatistics();
		m_pStatistic->Init(g_confFile.strLogDir.c_str(), g_confFile.uiFrequence);
		m_pStatistic->Start();

		m_PeerList.SetStatistic(m_pStatistic);
	}

	char* pszLocalIP = NULL;
	if(0 != strcmp(g_confFile.pszLocalIP,"0.0.0.0"))
	{
		pszLocalIP = g_confFile.pszLocalIP;
		//add local to peer list
		T_PPEERINFO pPeerInfo = new T_PEERINFO;
		pPeerInfo->uiIP = inet_addr(g_confFile.pszLocalIP); 
		pPeerInfo->usPort = g_confFile.usLocalPort;
		pPeerInfo->guid	  = g_confFile.tMGuid;
		m_PeerList.AddPeer(pPeerInfo);
	}
	
	if(1 != m_UdpSocket.Init(pszLocalIP, g_confFile.usLocalPort))
		return false;
	m_UdpSocket.SetUsed(true);

	if(0 != strlen(g_confFile.pszLocalIPDouble) && 
			0 != g_confFile.usLocalPortDouble	&& 
			0 != g_confFile.strMIDDouble.length())
	{
		if(1 != m_UdpSocketDouble.Init(g_confFile.pszLocalIPDouble, g_confFile.usLocalPortDouble))
			return false;
		m_UdpSocketDouble.SetUsed(true);
	}

	//log_info(g_pLogHelper, "DM::Init init udp ok port:%d",
	//		g_confFile.usLocalPort);	

	return true;
}

bool CDfsManager::Start()
{
	UpdateLocalPeerList(true);

	//start udp process thread
	if(0 != m_threadUdpProcess.Start(&m_funcUdpProcess))
		return false;

	//start file watcher thread
	if(0 != m_threadFileWatcher.Start(&m_funcFileWatcher))
	   return false;

	if(0 != m_threadPeerList.Start(&m_funcPeerList))
		return false;

	if(0 != m_threadWriteStatus.Start(&m_funcWriteStatus))
		return false;

	if(0 != m_threadPeerListCheck.Start(&m_funcPeerListCheck))
		return false;

	return true;	
}

void CDfsManager::Stop()
{
	//stop file watcher thread
	m_threadWriteStatus.Kill();

	m_threadFileWatcher.Kill();

	m_threadPeerListCheck.Kill();

	m_threadPeerList.Kill();

	m_threadUdpProcess.Kill();

}

void CDfsManager::Uninit()
{
}
	
string CDfsManager::GetStatus()
{
	return "";

	char pszManagerStatus[1024]="";
	snprintf(pszManagerStatus, 1024, "CDfsManager::GetStatusv GetReqMsgCount:%d GetRspMsgSuccessedCount:%d \
GetRspMsgFailedCount:%d SendStoreMsgCount:%d RecvStoreMsgCount:%d \nSendFindMsgCount:%d RecvFindMsgCount:%d",
			g_tDfsManagerStatus.uiGetReqMsgCount, 
			g_tDfsManagerStatus.uiGetRspMsgSuccessedCount,
			g_tDfsManagerStatus.uiGetRspMsgFailedCount,
			g_tDfsManagerStatus.uiSendStoreMsgCount,
			g_tDfsManagerStatus.uiRecvStoreMsgCount,
			g_tDfsManagerStatus.uiSendFindMsgCount,
			g_tDfsManagerStatus.uiRecvFindMsgCount);

	g_tDfsManagerStatus.clearStatus();
	return pszManagerStatus;
}


/*
string CDfsManager::GetStatus()
{
	char pszManagerStatus[1024]="";
	snprintf(pszManagerStatus, 1024, "CDfsManager::GetStatusv GetReqMsgCount:%d GetRspMsgSuccessedCount:%d \
GetRspMsgFailedCount:%d SendStoreMsgCount:%d RecvStoreMsgCount:%d \nSendFindMsgCount:%d RecvFindMsgCount:%d",
			g_tDfsManagerStatus.uiGetReqMsgCount/10, 
			g_tDfsManagerStatus.uiGetRspMsgSuccessedCount/10,
			g_tDfsManagerStatus.uiGetRspMsgFailedCount/10,
			g_tDfsManagerStatus.uiSendStoreMsgCount/10,
			g_tDfsManagerStatus.uiRecvStoreMsgCount/10,
			g_tDfsManagerStatus.uiSendFindMsgCount/10,
			g_tDfsManagerStatus.uiRecvFindMsgCount/10);

	g_tDfsManagerStatus.clearStatus();
	return pszManagerStatus;
}
*/

void CDfsManager::Join()
{
	//wait file watch thread
	m_threadWriteStatus.Join();
	
	m_threadFileWatcher.Join();

	m_threadPeerListCheck.Join();

	m_threadPeerList.Join();

	m_threadUdpProcess.Join();

}
	
void CDfsManager::UpdateLocalPeerList(bool bSendJoinMsg/*=false*/)
{
	//send peer join request message and user list request message to all peers
	T_DFSPROTOCOLPEERJOINREQ tPeerJoinReqMsg;
	tPeerJoinReqMsg.tType.ucType	= DFS_PROTOCOL_PEERJOIN_REQ;	
	gettimeofday(&tPeerJoinReqMsg.tType.tvTimeStamp, 0);
	tPeerJoinReqMsg.tMGuid			= g_confFile.tMGuid;

	T_DFSPROTOCOLUSERLISTREQ tUserListReqMsg;
	tUserListReqMsg.tType.ucType	= DFS_PROTOCOL_USERLIST_REQ;
	gettimeofday(&tUserListReqMsg.tType.tvTimeStamp, 0);
	tUserListReqMsg.tMGuid			= g_confFile.tMGuid;

	ITR_VEC_T_PPEERCONF itr = g_confFile.vecPeerConf.begin();
	T_PPEERCONF ptPeerConf = NULL;
	while(itr != g_confFile.vecPeerConf.end())
	{
		ptPeerConf = *itr;

		struct in_addr addPeerIP;
		addPeerIP.s_addr = ptPeerConf->tPeerAddr.uiIP;
		if(bSendJoinMsg)
		{
		//	log_info(g_pLogHelper, "DM::UpdateLocalPeerList send peer join req to ip:%s port:%d", 
		//			inet_ntoa(addPeerIP), ptPeerConf->tPeerAddr.uiPort);
			m_UdpSocket.Send(ptPeerConf->tPeerAddr.uiIP, ptPeerConf->tPeerAddr.uiPort, (const char*)&tPeerJoinReqMsg, sizeof(tPeerJoinReqMsg));
		}

//		log_info(g_pLogHelper, "DM::UpdateLocalPeerList send user list req to ip:%s port:%d", 
//					inet_ntoa(addPeerIP), ptPeerConf->tPeerAddr.uiPort);

		m_UdpSocket.Send(ptPeerConf->tPeerAddr.uiIP, ptPeerConf->tPeerAddr.uiPort, (const char*)&tUserListReqMsg, sizeof(tUserListReqMsg));
		itr++;
	}
/*
	//wait get peer list response message
	char* pRecvBuf = new char[MAX_RECV_UDP_BUF_LEN];
	unsigned int uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
	char szPeerIP[MAX_IP_LEN]="";
	unsigned int uiPeerPort=0;
	T_PDFSPROTOCOLUSERLISTRSP pDfsUserListRsp = NULL;
	int iRetry = 10;
	while(0<iRetry--)
	{
		sleep(1);

		memset(pRecvBuf, 0, MAX_RECV_UDP_BUF_LEN);
		memset(szPeerIP, 0, MAX_IP_LEN);
		uiPeerPort = 0;
		uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
			
		int iRecvFlag = m_UdpSocket.Recv((char*)szPeerIP, uiPeerPort, pRecvBuf, uiRecvBufLen);
		if(1 == iRecvFlag)
			break;

		pDfsUserListRsp = T_PDFSPROTOCOLUSERLISTRSP(pRecvBuf);
		if( DFS_PROTOCOL_SUCCESS !=	pDfsUserListRsp->tResult.tType.ucType )
		{
			pDfsUserListRsp = NULL;
			continue;
		}

	}
	
	//add all peer information in peer list 
	if(NULL != pDfsUserListRsp)
	{
		T_PDFSPROTOCOLPEER pDfsProtocolPeer = NULL;
		for(int i=0; i<pDfsUserListRsp->uiCount; i++)
		{
			pDfsProtocolPeer = pDfsUserListRsp->ptDfsProtocolPeers+i;
			T_PPEERINFO pPeerInfo = new T_PEERINFO;
			pPeerInfo->guid		= pDfsProtocolPeer->tMGuid;
			strcpy(pPeerInfo->pszIP, pDfsProtocolPeer->pszIP);
			pPeerInfo->usPort	= pDfsProtocolPeer->usPort;
		
			int iResult = m_PeerList.AddPeer(pPeerInfo);
		
			if(CPeerList::PEER_OPERATOR_SUCCESSED != iResult)
			{
				delete pPeerInfo;
				pPeerInfo = NULL;
			}
		}

	}
	
	delete[] pRecvBuf;
	pRecvBuf = NULL;

*/
}

void* CDfsManager::FileWatcherEntry(void* pParam)
{
	CDfsManager* pThis = static_cast<CDfsManager*>(pParam);
	if(NULL != pThis)
		pThis->FileWatcherImp();

	return NULL;
}

void CDfsManager::FindAnDelFile()
{
	T_FILENODE tCurFileNode;
	//read file info
	if(CFileNotify::FILE_LIST_EMPTY == m_FileNotify.GetDeleteFile(tCurFileNode))
		return;

	//just ignore the delete event
	return;
	
	T_PEERINFO pPeerInfo[MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT];
	memset(pPeerInfo, 0, sizeof(T_PEERINFO)*MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT);
	
	m_muxPeerList.Lock();
	int iPeersCount = m_PeerList.GetPeersByCID(tCurFileNode.tGuid, pPeerInfo, MAX_RESOURCE_STORE_COUNT, MAX_SPEC_RESOURCE_STORE_COUNT);	
	m_muxPeerList.UnLock();

//	T_DFSPROTOCOLSTOREREQ tDfsProtocolStoreReq;
	T_DFSPROTOCOLREMOVEREQ tDfsProtocolRemoveReq;
	tDfsProtocolRemoveReq.tType.ucType	= DFS_PROTOCOL_REMOVE_REQ;
	tDfsProtocolRemoveReq.tMGuid		= g_confFile.tMGuid;
	tDfsProtocolRemoveReq.tCGuid		= tCurFileNode.tGuid;
	tDfsProtocolRemoveReq.uiCSize		= tCurFileNode.fileLen;

	char szAllPeers[256]=("");
	char szTempPeers[256]=("");
	for(int i=0; i<iPeersCount; i++)
	{
		gettimeofday(&tDfsProtocolRemoveReq.tType.tvTimeStamp, 0);
	
		struct in_addr addPeerIP;
		addPeerIP.s_addr = pPeerInfo[i].uiIP;

		m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolRemoveReq, sizeof(tDfsProtocolRemoveReq));
		if(m_UdpSocketDouble.IsUsed())
			m_UdpSocketDouble.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolRemoveReq, sizeof(tDfsProtocolRemoveReq));
		
		strcpy(szTempPeers, szAllPeers);
		snprintf(szAllPeers, 256, "%s SA%d:%s", szTempPeers, i+1, inet_ntoa(addPeerIP));
	}
		
	log_info(g_pLogHelper, "R:E N:%s I:%s %s", 
			tCurFileNode.fileName.c_str(), 
			CCommonStruct::GUIDToString(tCurFileNode.tGuid).c_str(),
			szAllPeers);

	ProcessRemoveReqMsg(g_confFile.pszLocalIP, g_confFile.usLocalPort, (char*)&tDfsProtocolRemoveReq, sizeof(tDfsProtocolRemoveReq), NULL);	
}

void CDfsManager::FindAnAddFile(char* pDfsProtocolStoreInfo, char* pDfsProtocolStoreInfoFileName)
{
	T_FILENODE tCurFileNode;
	//memset(&tCurFileNode, 0, sizeof(tCurFileNode));
	//read file info
	if(CFileNotify::FILE_LIST_EMPTY == m_FileNotify.GetFile(tCurFileNode))
	{
		usleep(10*1000);
		//sleep(1);
		return;
	}

	T_PDFSPROTOCOLSTOREINFO pDPSI = (T_PDFSPROTOCOLSTOREINFO)(pDfsProtocolStoreInfo);

	pDPSI->tType.ucType			= DFS_PROTOCOL_STORE_INFO;
	gettimeofday(&pDPSI->tType.tvTimeStamp, 0);
	pDPSI->tCGuid				= tCurFileNode.tGuid;
	pDPSI->usFileNameLen		= tCurFileNode.fileName.length()+1;
	strncpy(pDfsProtocolStoreInfoFileName, tCurFileNode.fileName.c_str(), MAX_FILE_NAME_LEN-1);
	pDfsProtocolStoreInfoFileName[MAX_FILE_NAME_LEN-1]='\0';

//	log_info(g_pLogHelper, "DM::FWI get a file cid:%s name:%s len:%d", 
//			CCommonStruct::GUIDToString(tCurFileNode.tGuid).c_str(), tCurFileNode.fileName.c_str(), tCurFileNode.fileLen);
	//get remote peer list the resource will store
	T_PEERINFO pPeerInfo[MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT];
	memset(pPeerInfo, 0, sizeof(T_PEERINFO)*(MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT));
	m_muxPeerList.Lock();
	int iPeersCount = m_PeerList.GetPeersByCID(tCurFileNode.tGuid, pPeerInfo, MAX_RESOURCE_STORE_COUNT, MAX_SPEC_RESOURCE_STORE_COUNT);	
	m_muxPeerList.UnLock();

	T_DFSPROTOCOLSTOREREQ tDfsProtocolStoreReq;
	char szAllPeers[256]=("");
	char szTempPeers[256]=("");
	
	tDfsProtocolStoreReq.tType.ucType	= DFS_PROTOCOL_STORE_REQ;
	gettimeofday(&tDfsProtocolStoreReq.tType.tvTimeStamp, 0);
	tDfsProtocolStoreReq.tMGuid			= g_confFile.tMGuid;
	tDfsProtocolStoreReq.tCGuid			= tCurFileNode.tGuid;
	tDfsProtocolStoreReq.uiCSize		= tCurFileNode.fileLen;

	for(int i=0; i<iPeersCount; i++)
	{
		//send the the file info
		struct in_addr addPeerIP;
		addPeerIP.s_addr = pPeerInfo[i].uiIP;

		while(2000<m_UdpSocket.GetSendListNum())
			usleep(10*1000);

		log_info(g_pLogHelper, "DM::FWI send store request to  ip:%s port:%d iIndex:%d peercounts:%d type:%d", 
				inet_ntoa(addPeerIP), pPeerInfo[i].usPort, i, iPeersCount, tDfsProtocolStoreReq.tType.ucType);

		m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
		if(m_UdpSocketDouble.IsUsed())
		{
			tDfsProtocolStoreReq.tMGuid		= g_confFile.tMGuidDouble;
			m_UdpSocketDouble.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
		}
		else
			tDfsProtocolStoreReq.tMGuid		= g_confFile.tMGuid;


		//send store info message to all special peer
		m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)pDfsProtocolStoreInfo, 
				sizeof(T_DFSPROTOCOLSTOREINFO)+pDPSI->usFileNameLen);
		
		strcpy(szTempPeers, szAllPeers);
		snprintf(szAllPeers, 256, "%s SA%d:%s", szTempPeers, i+1, inet_ntoa(addPeerIP));
		g_tDfsManagerStatus.uiSendStoreMsgCount++;
	}
		
	log_info(g_pLogHelper, "R:F N:%s I:%s %s", 
			tCurFileNode.fileName.c_str(), 
			CCommonStruct::GUIDToString(tCurFileNode.tGuid).c_str(),
			szAllPeers);

	ProcessStoreReqMsg(g_confFile.pszLocalIP, g_confFile.usLocalPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq), NULL);	
	//	m_UdpSocket.Send(inet_addr("127.0.0.1"), pPeerInfo[0].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
}

void CDfsManager::ReSendDhtData()
{
	VEC_CONTENTINFO vecContentInfo;
	m_DhtDataSet.GetAllDhtData(vecContentInfo);

	ITR_VEC_CONTENTINFO itrCI = vecContentInfo.begin();
	T_CONTENTINFO tci;
	while(itrCI != vecContentInfo.end())
	{
		tci = *itrCI;
		
		//get remote peer list the resource will store
		T_PEERINFO pPeerInfo[MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT];
		memset(pPeerInfo, 0, sizeof(T_PEERINFO)*MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT);
		m_muxPeerList.Lock();
		int iPeersCount = m_PeerList.GetPeersByCID(tci.tCGuid, pPeerInfo, MAX_RESOURCE_STORE_COUNT, MAX_SPEC_RESOURCE_STORE_COUNT);	
		m_muxPeerList.UnLock();

		T_DFSPROTOCOLSTOREREQ tDfsProtocolStoreReq;
		tDfsProtocolStoreReq.tType.ucType	= DFS_PROTOCOL_STORE_REQ;
		tDfsProtocolStoreReq.tMGuid			= g_confFile.tMGuid;
		tDfsProtocolStoreReq.tCGuid			= tci.tCGuid;
		tDfsProtocolStoreReq.uiCSize		= tci.uiContentSize;
	
		for(int i=0; i<iPeersCount; i++)
		{
			//send the the file info
			gettimeofday(&tDfsProtocolStoreReq.tType.tvTimeStamp, 0);

			while(2000<m_UdpSocket.GetSendListNum())
				usleep(10*1000);

			m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
			if(m_UdpSocketDouble.IsUsed())
				m_UdpSocketDouble.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
		}	
		itrCI++;
	}
}

void CDfsManager::FileWatcherImp()
{
	m_threadFileWatcher.ThreadStarted();

	//log_info(g_pLogHelper, "DM::FileWatcherImp file watcher thread entry.");

	while(!g_bPeerListIsSet)
	{
	//	log_info(g_pLogHelper, "DM::FileWatcherImp wait g_bPeerListIsSet flag %d.", g_bPeerListIsSet);
		//sleep(1);
		usleep(100*1000);
	}

	//log_info(g_pLogHelper, "DM::FileWatcherImp waiting g_bPeerListIsSet flag %d.", g_bPeerListIsSet);
	if(0 != m_FileNotify.Init(g_confFile.strWatchDir.c_str(), g_confFile.uiScanFilePeriod))
	{
		log_err(g_pLogHelper, "DM::FileWatcherImp init file notify object failed. dir:%s, period:%d"
			, g_confFile.strWatchDir.c_str(), g_confFile.uiScanFilePeriod);
		return;
	}
//	log_info(g_pLogHelper, "DM::FileWatcherImp init file notify object ok. dir:%s, period:%d"
//			, g_confFile.strWatchDir.c_str(), g_confFile.uiScanFilePeriod);


	T_FILENODE tCurFileNode;
	char* pDfsProtocolStoreInfo = (char*)malloc(sizeof(T_DFSPROTOCOLSTOREINFO)+MAX_FILE_NAME_LEN);
	char* pDfsProtocolStoreInfoFileName = pDfsProtocolStoreInfo + sizeof(T_DFSPROTOCOLSTOREINFO);
	while(1)
	{
		FindAnDelFile();
		FindAnAddFile(pDfsProtocolStoreInfo, pDfsProtocolStoreInfoFileName);
/*		//memset(&tCurFileNode, 0, sizeof(tCurFileNode));
		//read file info
		if(CFileNotify::FILE_LIST_EMPTY == m_FileNotify.GetFile(tCurFileNode))
		{
			usleep(10*1000);
			//sleep(1);
			continue;
		}

		T_PDFSPROTOCOLSTOREINFO pDPSI = (T_PDFSPROTOCOLSTOREINFO)(pDfsProtocolStoreInfo);

		pDPSI->tType.ucType			= DFS_PROTOCOL_STORE_INFO;
		gettimeofday(&pDPSI->tType.tvTimeStamp, 0);
		pDPSI->tCGuid				= tCurFileNode.tGuid;
		pDPSI->usFileNameLen		= tCurFileNode.fileName.length()+1;
		strncpy(pDfsProtocolStoreInfoFileName, tCurFileNode.fileName.c_str(), MAX_FILE_NAME_LEN-1);
		pDfsProtocolStoreInfoFileName[MAX_FILE_NAME_LEN-1]='\0';

		//log_info(g_pLogHelper, "DM::FWI get a file cid:%s name:%s len:%d", 
		//		CCommonStruct::GUIDToString(tCurFileNode.tGuid).c_str(), tCurFileNode.fileName.c_str(), tCurFileNode.fileLen);
		//get remote peer list the resource will store
		T_PEERINFO pPeerInfo[MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT];
		memset(pPeerInfo, 0, sizeof(T_PEERINFO)*MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT);
		m_muxPeerList.Lock();
		int iPeersCount = m_PeerList.GetPeersByCID(tCurFileNode.tGuid, pPeerInfo, MAX_RESOURCE_STORE_COUNT);	
		m_muxPeerList.UnLock();

		T_DFSPROTOCOLSTOREREQ tDfsProtocolStoreReq;
		char szAllPeers[128]=("");
		char szTempPeers[128]=("");
		for(int i=0; i<iPeersCount; i++)
		{
			//send the the file info
			tDfsProtocolStoreReq.tType.ucType	= DFS_PROTOCOL_STORE_REQ;
			gettimeofday(&tDfsProtocolStoreReq.tType.tvTimeStamp, 0);
			tDfsProtocolStoreReq.tMGuid			= g_confFile.tMGuid;
			tDfsProtocolStoreReq.tCGuid			= tCurFileNode.tGuid;
			tDfsProtocolStoreReq.uiCSize		= tCurFileNode.fileLen;
		
			struct in_addr addPeerIP;
			addPeerIP.s_addr = pPeerInfo[i].uiIP;
		//	log_info(g_pLogHelper, "DM::FWI send store request to ip:%s port:%d iIndex:%d peercounts:%d", 
		//			inet_ntoa(addPeerIP), pPeerInfo[i].usPort, i, iPeersCount);

			while(2000<m_UdpSocket.GetSendListNum())
				usleep(10*1000);

			m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
			//send store info message to all special peer
			m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)pDfsProtocolStoreInfo, 
					sizeof(T_DFSPROTOCOLSTOREINFO)+pDPSI->usFileNameLen);
			
			strcpy(szTempPeers, szAllPeers);
			snprintf(szAllPeers, 128, "%s SA%d:%s", szTempPeers, i+1, inet_ntoa(addPeerIP));
			g_tDfsManagerStatus.uiSendStoreMsgCount++;
		}
		
		log_info(g_pLogHelper, "R:F N:%s I:%s %s", 
				tCurFileNode.fileName.c_str(), 
				CCommonStruct::GUIDToString(tCurFileNode.tGuid).c_str(),
				szAllPeers);

		ProcessStoreReqMsg(g_confFile.pszLocalIP, g_confFile.usLocalPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));	
	//	m_UdpSocket.Send(inet_addr("127.0.0.1"), pPeerInfo[0].usPort, (char*)&tDfsProtocolStoreReq, sizeof(tDfsProtocolStoreReq));
*/
	}

}

void* CDfsManager::UdpProcessEntry(void* pParam)
{
	CDfsManager* pThis = static_cast<CDfsManager*>(pParam);
	if(NULL != pThis)
		pThis->UdpProcessImp();
	return NULL;
}

void CDfsManager::UdpProcessImp()
{
	m_threadUdpProcess.ThreadStarted();

	//log_info(g_pLogHelper, "DM::UdpProcess  udp process is running." );

	char* pRecvBuf = new char[MAX_RECV_UDP_BUF_LEN];
	unsigned int uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
	char pszPeerIP[MAX_IP_LEN]="";
	unsigned int     uiPeerIP = 0;
	unsigned short usPeerPort=0;
	time_t tmSendUserlistRsp;
	time(&tmSendUserlistRsp);
	int iRecvPackIndex=0;
	int iResult = 0;

	CUdpSocket* pCurUdpSocket = &m_UdpSocket;
	int iTempTimes=0;
	while(1)
	{

		memset(pRecvBuf, 0, MAX_RECV_UDP_BUF_LEN);			
		uiRecvBufLen = MAX_RECV_UDP_BUF_LEN;
		memset(pszPeerIP, 0, MAX_IP_LEN);
		usPeerPort = 0;
		uiPeerIP = 0;

		//send user list rsp to all peers
		time_t tmNow;
		time(&tmNow);
		if(MAX_SEND_USERLIST_RSP_PERIOD<(tmNow-tmSendUserlistRsp))
		{
			tmSendUserlistRsp = tmNow;
			SendUserListRspMsgToPeers();
		}
		if(iRecvPackIndex++ > 10000)
			iRecvPackIndex=0;

/*		if(0 == (iRecvPackIndex % 2))
		{
			pCurUdpSocket = &m_UdpSocket;
		}
		else
		{
			if(m_UdpSocketDouble.IsUsed())
			{
				pCurUdpSocket = &m_UdpSocketDouble;
			}
			else
			{
				pCurUdpSocket = &m_UdpSocket;
			}
		}
*/			
		iResult = pCurUdpSocket->Recv(uiPeerIP,usPeerPort, pRecvBuf, uiRecvBufLen);

		if(1!=iResult)
		{
			if((int)pCurUdpSocket == (int)&m_UdpSocket)
				pCurUdpSocket = &m_UdpSocketDouble;
			else
			{
				pCurUdpSocket = &m_UdpSocket;
				usleep(10*1000);
			}
//			if(iTempTimes++ == 5)
//			{
//				iTempTimes=0;
//				usleep(10);
//			}
			continue;
		}


		struct in_addr addPeerIP;
		addPeerIP.s_addr = uiPeerIP;
		strcpy(pszPeerIP, inet_ntoa(addPeerIP));
	
		T_PDFSPROTOCOLTYPE ptType = (T_PDFSPROTOCOLTYPE)(pRecvBuf);

		//log_info(g_pLogHelper, "CDfsManager::UdpProcessImp recv a pack ip:%s port:%d. type:%d",  pszPeerIP, usPeerPort, ptType->ucType);
		switch(ptType->ucType)
		{
		case DFS_PROTOCOL_PING_REQ:
			ProcessPingReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket );
			break;
		case DFS_PROTOCOL_PING_RSP:
			//ProcessPingRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_STORE_REQ:
			ProcessStoreReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_STORE_RSP:
			//ProcessStoreRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_REMOVE_REQ:
			ProcessRemoveReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_REMOVE_RSP:
			//ProcessRemoveRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_FIND_REQ:
			ProcessFindReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_FIND_RSP:
			ProcessFindRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_USERLIST_REQ:
			ProcessUserListReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_USERLIST_RSP:
			ProcessUserListRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_PEERJOIN_REQ:
			ProcessPeerJoinReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_PEERJOIN_RSP:
			ProcessPeerJoinRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_GET_REQ:
			ProcessGetReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_GET_RSP:
			//ProcessGetRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_PEERLEAVE_REQ:
			ProcessPeerLeaveReqMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_PEERLEAVE_RSP:
			ProcessPeerLeaveRspMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		case DFS_PROTOCOL_STORE_INFO:
			ProcessStoreInfoMsg(pszPeerIP, usPeerPort, pRecvBuf, uiRecvBufLen, pCurUdpSocket);
			break;
		default :
			break;

		}
	
	}
	delete[] pRecvBuf;
	pRecvBuf = NULL;
}

void* CDfsManager::PeerListEntry(void* pParam)
{
	CDfsManager* pThis = static_cast<CDfsManager*>(pParam);
	if(NULL != pThis)
		pThis->PeerListImp();
	return NULL;	
}

void CDfsManager::PeerListImp()
{
	m_threadPeerList.ThreadStarted();
	
	int iIndex = 0;
	while(1)
	{
		sleep(MAX_PING_EXPIRE);
	
		T_DFSPROTOCOLPINGREQ tpingReqMsgDouble;
		tpingReqMsgDouble.tType.ucType	= DFS_PROTOCOL_PING_REQ;
		tpingReqMsgDouble.tMGuid			= g_confFile.tMGuidDouble;
		gettimeofday(&tpingReqMsgDouble.tType.tvTimeStamp, 0);

		T_DFSPROTOCOLPINGREQ tpingReqMsg;
		tpingReqMsg.tType.ucType	= DFS_PROTOCOL_PING_REQ;
		tpingReqMsg.tMGuid			= g_confFile.tMGuid;
		gettimeofday(&tpingReqMsg.tType.tvTimeStamp, 0);

		//send ping req to all peers that in configure
		if(0 == iIndex % 10)
		{
			T_PPEERCONF pPeerConf = NULL;
			ITR_VEC_T_PPEERCONF itrConf = g_confFile.vecPeerConf.begin();
			while(itrConf != g_confFile.vecPeerConf.end())
			{
				pPeerConf = *itrConf;
				if(NULL != pPeerConf)
				{
					m_UdpSocket.Send(pPeerConf->tPeerAddr.uiIP, pPeerConf->tPeerAddr.uiPort, (const char*)&tpingReqMsg, sizeof(tpingReqMsg));
					if(m_UdpSocketDouble.IsUsed())
						m_UdpSocketDouble.Send(pPeerConf->tPeerAddr.uiIP, pPeerConf->tPeerAddr.uiPort, (const char*)&tpingReqMsg, sizeof(tpingReqMsgDouble));
				}
				itrConf++;
			}
		}

		m_muxPeerList.Lock();
		//log_info(g_pLogHelper, "send ping message to all peers.");
		m_PeerList.SendDataToAllPeers(m_UdpSocket, (char*)&tpingReqMsg, sizeof(tpingReqMsg), 1);
		if(m_UdpSocketDouble.IsUsed())
			m_PeerList.SendDataToAllPeers(m_UdpSocketDouble, (char*)&tpingReqMsgDouble, sizeof(tpingReqMsgDouble), 1);
		
		m_PeerList.RemoveOldPeer();

		m_muxPeerList.UnLock();

		if(90 < iIndex++)
		{
			iIndex = 0;
			m_muxDataSet.Lock();
			m_DhtDataSet.RemoveOldDhtData();
			m_muxDataSet.UnLock();
		}
	}
}

void* CDfsManager::WriteStatusEntry(void* pParam)
{
	CDfsManager* pThis = static_cast<CDfsManager*>(pParam);
	if(NULL != pThis)
		pThis->WriteStatusImp();
	return NULL;
}

void CDfsManager::WriteStatusImp()
{
	m_threadWriteStatus.ThreadStarted();

	while(!g_bPeerListIsSet)
	{
		//log_info(g_pLogHelper, "DM::WriteStatusImp waiting g_bPeerListIsSet flag %d.", g_bPeerListIsSet);
		//sleep(1);
		usleep(100*1000);
	}

	//open status file
//	string strStatuFile = g_confFile.strLogDir;
//	strStatuFile += "uusee_status.rec";

  //  FILE* fp = fopen(strStatuFile.c_str(),"a");
//	if(NULL == fp)
//		return;

	string strTxt("");
	while(1)
	{
		sleep(1);
		//sleep(10);

		strTxt = m_FileNotify.GetStatus();
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
//		fprintf(fp, "%s\n", strTxt.c_str());
//		fflush(fp);

		strTxt = m_UdpSocket.GetStatus();
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
		//fprintf(fp, "%s\n", strTxt.c_str());
		//fflush(fp);

		if(m_UdpSocketDouble.IsUsed())
		{
			strTxt = m_UdpSocketDouble.GetStatus();
			log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
			//fprintf(fp, "%s\n", strTxt.c_str());
			//fflush(fp);
		}

		strTxt = m_PeerList.GetStatus();
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
		//fprintf(fp, "%s\n", strTxt.c_str());
		//fflush(fp);

		strTxt = m_DhtDataSet.GetStatus();
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
		//fprintf(fp, "%s\n", strTxt.c_str());
		//fflush(fp);
		
		strTxt = GetStatus();
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "%s\n", strTxt.c_str());
		//fprintf(fp, "%s\n", strTxt.c_str());
		//fflush(fp);
		
		log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "++++++++++++++++++++++++\n");
		//fprintf(fp, "%s\n", "++++++++++++++++++++++++++");
		//fflush(fp);
	}
//	fclose(fp);
}

void* CDfsManager::PeerListCheckEntry(void* pParam)
{
	CDfsManager* pThis = static_cast<CDfsManager*>(pParam);
	if(NULL != pThis)
		pThis->PeerListCheckImp();
	return NULL;	
}

void CDfsManager::PeerListCheckImp()
{
	m_threadPeerListCheck.ThreadStarted();
	
	int iWaitStartSpace = 60*60;
	sleep(iWaitStartSpace);

	bool bExitflag = 0;
	int iPeerCountsBefore = m_PeerList.GetNormalPeerCount();
	int iPeerCountsCur  = 0;
	int iSpace = 0;
	while(!bExitflag)
	{
		sleep(iWaitStartSpace);
		iPeerCountsCur = m_PeerList.GetNormalPeerCount();

		if(iPeerCountsCur > iPeerCountsBefore)
			iSpace = iPeerCountsCur - iPeerCountsBefore;
		else
			iSpace = iPeerCountsBefore - iPeerCountsCur;

		iPeerCountsBefore = iPeerCountsCur;
		//if(iSpace < MAX_RESOURCE_STORE_COUNT)
		if(iSpace < 3)
			continue;

		//send the local dht data to special peer
		ReSendDhtData();		
	}
}

void CDfsManager::ProcessPingReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLPINGREQ pDfsProtocolPingReq = (T_PDFSPROTOCOLPINGREQ)(pBuf);
	log_info(g_pLogHelper, "DM::ProPingReqMsg recv ping request from ip:%s port:%d mid", 
			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolPingReq->tMGuid).c_str());
		
	//find the peer in peer list
	CAutoMutexLock muxAuto(m_muxPeerList);

	T_PPEERINFO pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPingReq->tMGuid);
	if(NULL == pPeerInfo)
		pPeerInfo = m_PeerList.FindSpecPeer(pDfsProtocolPingReq->tMGuid);
	if(NULL == pPeerInfo)
	{
		//if not find add the peer into the peer list
		pPeerInfo = new T_PEERINFO;
		pPeerInfo->guid = pDfsProtocolPingReq->tMGuid;
//		strcpy(pPeerInfo->pszIP, pszIP);
		pPeerInfo->uiIP = inet_addr(pszIP);
		pPeerInfo->usPort = usPort;

		if(CPeerList::PEER_OPERATOR_SUCCESSED != m_PeerList.AddPeer(pPeerInfo))
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
		else
		{
			//send user list
			SendUserListToPeer(pPeerInfo->uiIP, pPeerInfo->usPort);
			AddPeerToStatistics(pPeerInfo);
		}
			
	//	log_info(g_pLogHelper, "DM::ProPingReqMsg add to peerlist mid:%s", CCommonStruct::GUIDToString(pDfsProtocolPingReq->tMGuid).c_str());

		g_bPeerListIsSet = true;
	}
	else
	{
	//	log_info(g_pLogHelper, "DM::ProPingReqMsg refresh the keep alive of the peer mid:%s", CCommonStruct::GUIDToString(pDfsProtocolPingReq->tMGuid).c_str());

		//update the keep alive time.
		time(&pPeerInfo->tmKeepaliveTime);
	}
	
	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessPingReqMsg process ping request message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessPingReqMsg process ping request message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);
}

void CDfsManager::ProcessPingRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
}
void CDfsManager::ProcessStoreReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	CAutoMutexLock muxAuto(m_muxDataSet);
	g_tDfsManagerStatus.uiRecvStoreMsgCount++;
	
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLSTOREREQ pDfsProtocolStoreReq = (T_PDFSPROTOCOLSTOREREQ)(pBuf);

	T_PDHTPEER pDhtPeer = new T_DHTPEER;
	pDhtPeer->tMGuid	= pDfsProtocolStoreReq->tMGuid;
//	strcpy(pDhtPeer->taddPeer.pszIP, pszIP);
	pDhtPeer->taddPeer.uiIP = inet_addr(pszIP);
	pDhtPeer->taddPeer.uiPort	= usPort;
	time(&pDhtPeer->tmKeepalive);
	if(0 == pDhtPeer->taddPeer.uiIP)
	{
		log_err(g_pLogHelper, "DM::ProStoreReqMsg recv store req is exist. ip:%s port:%d mid:%s cid:%s",
			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tMGuid).c_str(), CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tCGuid).c_str());

		delete pDhtPeer;
		pDhtPeer = NULL;
		return;
	}
	if(CDhtDataSet::DHTDATA_OPERATOR_SUCCESSED !=
		   	m_DhtDataSet.AddDhtData(pDfsProtocolStoreReq->tCGuid, pDfsProtocolStoreReq->uiCSize, pDhtPeer))
	{
		log_info(g_pLogHelper, "DM::ProStoreReqMsg recv store req is exist. ip:%s port:%d mid:%s cid:%s",
			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tMGuid).c_str(), CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tCGuid).c_str());
		m_DhtDataSet.RefreshDhtData(pDfsProtocolStoreReq->tCGuid, pDhtPeer);
		delete pDhtPeer;
		pDhtPeer = NULL;
	}
	else
	{
		log_info(g_pLogHelper, "R:D I:%s RA:%s", CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tCGuid).c_str(), pszIP);
	//	log_info(g_pLogHelper, "DM::ProStoreReqMsg recv store req ip:%s port:%d mid:%s cid:%s",
	//		pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tMGuid).c_str(), CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tCGuid).c_str());

	}
//	else
	{
		if(NULL != m_pStatistic)
			m_pStatistic->addFile(&pDfsProtocolStoreReq->tCGuid, &pDfsProtocolStoreReq->tMGuid, 1, g_confFile.uiScanFilePeriod);
	}

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessStoreReqMsg process store request message used %d milliseconds", 
//			tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessStoreReqMsg process store request message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

}


/*
void CDfsManager::ProcessStoreReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PDFSPROTOCOLSTOREREQ pDfsProtocolStoreReq = (T_PDFSPROTOCOLSTOREREQ)pBuf;
	log_info(g_pLogHelper, "CDfsManager::ProcessStoreReqMsg recv store request message ip:%s port:%d mid:%s cid:%s",
			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tMGuid).c_str(), CCommonStruct::GUIDToString(pDfsProtocolStoreReq->tCGuid).c_str());

	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(pDfsProtocolStoreReq->tCGuid);
	if(NULL == pDhtData)
	{
		pDhtData = new T_DHTDATA;
		pDhtData->tCGuid	= pDfsProtocolStoreReq->tCGuid;
		pDhtData->tMGuid	= pDfsProtocolStoreReq->tMGuid;
		strcpy(pDhtData->taddPeer.pszIP, pszIP);
		pDhtData->taddPeer.uiPort = usPort;
		pDhtData->uiContentSize = pDfsProtocolStoreReq->uiCSize;

		m_DhtDataSet.AddDhtData(pDhtData);

	}
	else
	{
		time(&pDhtData->tmKeepalive);
	}
}
*/

void CDfsManager::ProcessStoreRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
}


void CDfsManager::ProcessRemoveReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	//remove the file from dht
	CAutoMutexLock muxAuto(m_muxDataSet);
	
	T_PDFSPROTOCOLREMOVEREQ pDfsProtocolRemoveReq = (T_PDFSPROTOCOLREMOVEREQ)(pBuf);

	T_PDHTPEER pDhtPeer = new T_DHTPEER;
	pDhtPeer->tMGuid	= pDfsProtocolRemoveReq->tMGuid;
	pDhtPeer->taddPeer.uiIP = inet_addr(pszIP);
	pDhtPeer->taddPeer.uiPort	= usPort;
	time(&pDhtPeer->tmKeepalive);
	
	m_DhtDataSet.RemoveDhtData(pDfsProtocolRemoveReq->tCGuid, pDhtPeer);

	delete pDhtPeer;
	pDhtPeer = NULL;

	log_info(g_pLogHelper, "R:M I:%s RA:%s", CCommonStruct::GUIDToString(pDfsProtocolRemoveReq->tCGuid).c_str(), pszIP);
}

void CDfsManager::ProcessRemoveRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
}

void CDfsManager::ProcessStoreInfoMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	if(NULL == m_pStatistic)
		return;

	T_PDFSPROTOCOLSTOREINFO pDPSI = (T_PDFSPROTOCOLSTOREINFO)(pBuf);
	char* pszFileName = (char*)(pDPSI+1);
	string strFileName(pszFileName, pDPSI->usFileNameLen);

	m_pStatistic->PrintFileInfo(&pDPSI->tCGuid, strFileName.c_str());	
}

void CDfsManager::ProcessFindReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	g_tDfsManagerStatus.uiRecvFindMsgCount++;

	T_PDFSPROTOCOLFINDREQ pDfsProtocolFindReq = (T_PDFSPROTOCOLFINDREQ)(pBuf);
	
//	log_info(g_pLogHelper, "DM::ProFindReqMsg recv find req. Ip:%s Port:%d cid:%s",
//			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolFindReq->tCGuid).c_str());

	CAutoMutexLock muxAuto(m_muxDataSet);

//	struct timeval tvFindStart, tvFindEnd;
//	gettimeofday(&tvFindStart, 0);
	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(pDfsProtocolFindReq->tCGuid);
//	gettimeofday(&tvFindEnd, 0);

//	tvSpac.tv_sec	= tvFindEnd.tv_sec - tvFindStart.tv_sec;
//	tvSpac.tv_usec = tvFindEnd.tv_usec- tvFindStart.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg process find cid in map used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

	T_PDFSPROTOCOLFINDRSP pDfsProtocolFindRsp	= NULL;
	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers		= NULL;
	int iBufRspSize = 0;
	if(NULL == pDhtData)
	{
		iBufRspSize = sizeof(T_DFSPROTOCOLFINDRSP);
		pDfsProtocolFindRsp							= (T_PDFSPROTOCOLFINDRSP)malloc(iBufRspSize);
		pDfsProtocolFindRsp->tResult.tType.ucType	= DFS_PROTOCOL_FIND_RSP;
		pDfsProtocolFindRsp->tResult.tType.tvTimeStamp = pDfsProtocolFindReq->tType.tvTimeStamp;
		pDfsProtocolFindRsp->tResult.iResult		= DFS_PROTOCOL_FAILED;
		pDfsProtocolFindRsp->tCGuid					= pDfsProtocolFindReq->tCGuid;
		pDfsProtocolFindRsp->uiCSize				= 0;
		pDfsProtocolFindRsp->uiCount				= 0;
		ptDfsProtocolPeers							= NULL;

//		log_info(g_pLogHelper, "DM::ProFindReqMsg not find the content. cid:%s",
//				CCommonStruct::GUIDToString(pDfsProtocolFindReq->tCGuid).c_str());
	}
	else
	{
		int iDhtPeerCount = pDhtData->vecDhtPeer.size();
		iBufRspSize = sizeof(T_DFSPROTOCOLFINDRSP)+iDhtPeerCount*sizeof(T_DFSPROTOCOLPEER);
		pDfsProtocolFindRsp = (T_PDFSPROTOCOLFINDRSP)malloc(iBufRspSize);
		pDfsProtocolFindRsp->tResult.tType.ucType	= DFS_PROTOCOL_FIND_RSP;
		pDfsProtocolFindRsp->tResult.tType.tvTimeStamp = pDfsProtocolFindReq->tType.tvTimeStamp;
		pDfsProtocolFindRsp->tResult.iResult	= DFS_PROTOCOL_SUCCESS;
		pDfsProtocolFindRsp->tCGuid				= pDfsProtocolFindReq->tCGuid;
		pDfsProtocolFindRsp->uiCSize			= pDhtData->uiContentSize;
		pDfsProtocolFindRsp->uiCount			= iDhtPeerCount;

		ITR_VEC_T_PDHTPEER itr = pDhtData->vecDhtPeer.begin();
		T_PDHTPEER pDhtPeer = NULL;
		ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolFindRsp+1);
		while(itr != pDhtData->vecDhtPeer.end())
		{
			pDhtPeer = *itr;
			if(NULL != pDhtPeer)
			{
				ptDfsProtocolPeers->tMGuid		= pDhtPeer->tMGuid;;
//				strcpy(ptDfsProtocolPeers->pszIP, pDhtPeer->taddPeer.pszIP);
				ptDfsProtocolPeers->uiIP		= pDhtPeer->taddPeer.uiIP;
				//strcpy(ptDfsProtocolPeers->pszIP, pDhtPeer->taddPeer.pszIP);
				ptDfsProtocolPeers->usPort		= pDhtPeer->taddPeer.uiPort;

				in_addr t1;
				t1.s_addr=ptDfsProtocolPeers->uiIP;
	//			log_info(g_pLogHelper, "DM::ProFindReqMsg recv find req peer. mid:%s cid:%s pip:%s pPort:%d, iBufRspSize:%d=%d+%d*%d",
	//					CCommonStruct::GUIDToString(pDhtPeer->tMGuid).c_str(), CCommonStruct::GUIDToString(pDhtData->tCGuid).c_str(),
	//					inet_ntoa(t1), ptDfsProtocolPeers->usPort, iBufRspSize, sizeof(T_DFSPROTOCOLFINDRSP), iDhtPeerCount, sizeof(T_DFSPROTOCOLPEER));

				ptDfsProtocolPeers++;
			}
			itr++;
		}

	}

	pUdpSSocket->Send(inet_addr(pszIP), (unsigned int)usPort, (const char*)pDfsProtocolFindRsp, (unsigned int)iBufRspSize);
	//m_UdpSocket.Send(inet_addr(pszIP), (unsigned int)usPort, (const char*)pDfsProtocolFindRsp, (unsigned int)iBufRspSize);

	free(pDfsProtocolFindRsp);
	pDfsProtocolFindRsp = NULL;

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg process find request message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
	//log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg process find request message used %d microseconds", 
	//		tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

}


/*
void CDfsManager::ProcessFindReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PDFSPROTOCOLFINDREQ pDfsProtocolFindReq = (T_PDFSPROTOCOLFINDREQ)pBuf;
	
//	log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg recv find request message. peerIp:%s peerPort:%d cid:%s",
//			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolFindReq->tCGuid).c_str());

	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(pDfsProtocolFindReq->tCGuid);

	T_PDFSPROTOCOLFINDRSP pDfsProtocolFindRsp	= NULL;
	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers		= NULL;
	int iBufRspSize = 0;
	if(NULL == pDhtData)
	{
		iBufRspSize = sizeof(T_DFSPROTOCOLFINDRSP);
		pDfsProtocolFindRsp							= (T_PDFSPROTOCOLFINDRSP)malloc(iBufRspSize);
		pDfsProtocolFindRsp->tResult.tType.ucType	= DFS_PROTOCOL_FIND_RSP;
		pDfsProtocolFindRsp->tResult.iResult		= DFS_PROTOCOL_FAILED;
		pDfsProtocolFindRsp->tCGuid					= pDfsProtocolFindReq->tCGuid;
		pDfsProtocolFindRsp->uiCSize				= 0;
		pDfsProtocolFindRsp->uiCount				= 0;
		ptDfsProtocolPeers							= NULL;

		log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg not find peer that store the cid. cid:%s",
				CCommonStruct::GUIDToString(pDfsProtocolFindReq->tCGuid).c_str());
	}
	else
	{
		iBufRspSize = sizeof(T_DFSPROTOCOLFINDRSP)+1*sizeof(T_DFSPROTOCOLPEER);
		pDfsProtocolFindRsp = (T_PDFSPROTOCOLFINDRSP)malloc(iBufRspSize);
		pDfsProtocolFindRsp->tResult.tType.ucType	= DFS_PROTOCOL_FIND_RSP;
		pDfsProtocolFindRsp->tResult.iResult	= DFS_PROTOCOL_FAILED;
		pDfsProtocolFindRsp->tCGuid				= pDfsProtocolFindReq->tCGuid;
		pDfsProtocolFindRsp->uiCSize			= pDhtData->uiContentSize;
		pDfsProtocolFindRsp->uiCount			= 1;
		ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolFindRsp+1);

		ptDfsProtocolPeers->tMGuid		= pDhtData->tMGuid;
		strcpy(ptDfsProtocolPeers->pszIP, pDhtData->taddPeer.pszIP);
		ptDfsProtocolPeers->usPort		= pDhtData->taddPeer.uiPort;

		log_info(g_pLogHelper, "CDfsManager::ProcessFindReqMsg find peer that store the cid. mid:%s cid:%s",
				CCommonStruct::GUIDToString(pDhtData->tMGuid).c_str(), CCommonStruct::GUIDToString(pDhtData->tCGuid).c_str());
	}

	m_UdpSocket.Send((const char*)pszIP, (unsigned int)usPort, (const char*)pDfsProtocolFindRsp, (unsigned int)iBufRspSize);
}
*/

void CDfsManager::ProcessFindRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLFINDRSP pFindRspMsg = (T_PDFSPROTOCOLFINDRSP)(pBuf);

//	log_info(g_pLogHelper, "DM::ProFindRspMsg recv find resp from:%s. cid:%s result:%d, csize:%d, count:%d ", pszIP,
//			CCommonStruct::GUIDToString(pFindRspMsg->tCGuid).c_str(), pFindRspMsg->tResult.iResult, pFindRspMsg->uiCSize,  pFindRspMsg->uiCount);

	CAutoMutexLock muxAuto(m_muxDataSet);

//	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(pFindRspMsg->tCGuid);
//	if(NULL != pDhtData)
//		return;

	T_PDHTPEER pDhtPeer = NULL;
	T_PDFSPROTOCOLPEER pPeer = (T_PDFSPROTOCOLPEER)(pFindRspMsg+1);
	for(int i=0; i<pFindRspMsg->uiCount; i++, pPeer+=1)
	{
//		pPeer += i;
		pDhtPeer = new T_DHTPEER;
		pDhtPeer->tMGuid	= pPeer->tMGuid;
//		strcpy(pDhtPeer->taddPeer.pszIP, pPeer->pszIP);
		if(INADDR_ANY == pPeer->uiIP)
		{
//		log_info(g_pLogHelper, "DM::ProFindRspMsg err recv find resp.indes:%d peerip:%d peerPort:%d, cid:%s, mid:%s, aPeer:%p",
//				i, pPeer->uiIP, pDhtPeer->taddPeer.uiPort, CCommonStruct::GUIDToString(pFindRspMsg->tCGuid).c_str(),
//			   CCommonStruct::GUIDToString(pDhtPeer->tMGuid).c_str(), pPeer);
			delete pDhtPeer;
			pDhtPeer = NULL;
			continue;
			//pDhtPeer->taddPeer.uiIP = inet_addr(pszIP);
		}
		else
			pDhtPeer->taddPeer.uiIP = pPeer->uiIP;
		pDhtPeer->taddPeer.uiPort = pPeer->usPort;
		time(&pDhtPeer->tmKeepalive);

		struct in_addr addPeerIP;
		addPeerIP.s_addr = pDhtPeer->taddPeer.uiIP;
//		log_info(g_pLogHelper, "DM::ProFindRspMsg recv find resp.indes:%d peerip:%s peerPort:%d, cid:%s, mid:%s",
//				i, inet_ntoa(addPeerIP), pDhtPeer->taddPeer.uiPort, CCommonStruct::GUIDToString(pFindRspMsg->tCGuid).c_str(),
//			   CCommonStruct::GUIDToString(pDhtPeer->tMGuid).c_str());

		if(CDhtDataSet::DHTDATA_OPERATOR_SUCCESSED !=
				m_DhtDataSet.AddDhtData(pFindRspMsg->tCGuid, pFindRspMsg->uiCSize, pDhtPeer))
		{
			delete pDhtPeer;
			pDhtPeer = NULL;
		}
//		else
		{
			if(NULL != m_pStatistic)
				m_pStatistic->addFile(&pFindRspMsg->tCGuid, &pPeer->tMGuid,  1, g_confFile.uiScanFilePeriod);
		}
	}
	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessFindRspMsg process find responce message used %d milliseconds", 
//			tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessFindRspMsg process find responce message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

	tvSpac.tv_sec	= tvEnd.tv_sec - pFindRspMsg->tResult.tType.tvTimeStamp.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - pFindRspMsg->tResult.tType.tvTimeStamp.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessFindRspMsg process find responce message used %d microseconds from send find request message", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);
}

/*
void CDfsManager::ProcessFindRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen)
{
	T_PDFSPROTOCOLFINDRSP pFindRspMsg = (T_PDFSPROTOCOLFINDRSP)(pBuf);

	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(pFindRspMsg->tCGuid);
	if(NULL != pDhtData)
		return;
	
	T_PDFSPROTOCOLPEER pPeer = (T_PDFSPROTOCOLPEER)pFindRspMsg+1;
	for(int i=0; i<pFindRspMsg->uiCount; i++)
	{
		pPeer += i;
		pDhtData = new T_DHTDATA;
		pDhtData->tCGuid	= pFindRspMsg->tCGuid;
		pDhtData->tMGuid	= pPeer->tMGuid;
		strcpy(pDhtData->taddPeer.pszIP, pPeer->pszIP);
		pDhtData->taddPeer.uiPort	= pPeer->usPort;
		pDhtData->uiContentSize		= pFindRspMsg->uiCSize;
		if(CDhtDataSet::DHTDATA_OPERATOR_SUCCESSED != m_DhtDataSet.AddDhtData(pDhtData))
		{
			delete pDhtData;
			pDhtData = NULL;
		}
	}
	
}
*/


void PrintGetRspMsg(T_PDFSPROTOCOLGETRSP pRsp, const char* pcid)
{
	log_info(g_pLogHelper, "print get rsp msg cid:%s type:%d timestamp:%d-%d result:%d peerCount:%d",
			pcid,
	      pRsp->tResult.tType.ucType, 
	      pRsp->tResult.tType.tvTimeStamp.tv_sec,
	      pRsp->tResult.tType.tvTimeStamp.tv_usec,
	      pRsp->tResult.iResult,
		  pRsp->uiPeerCount);
	
	struct in_addr addrIP;
	T_PDFSPROTOCOLPEER pPeer = (T_PDFSPROTOCOLPEER)(pRsp+1);

	for(int i=0; i<pRsp->uiPeerCount; i++, pPeer++)
	{
//		pPeer += i;// (T_PDFSPROTOCOLPEER)(pRsp+i+1);
		memset(&addrIP, 0, sizeof(addrIP));
		addrIP.s_addr = pPeer->uiIP;
        log_info(g_pLogHelper, "print get rsp msg cid:%s type:%d timestamp:%d-%d result:%d peerCount:%d pip:%s, pport:%d",
				pcid,
	            pRsp->tResult.tType.ucType, 
		        pRsp->tResult.tType.tvTimeStamp.tv_sec,
			    pRsp->tResult.tType.tvTimeStamp.tv_usec,
				pRsp->tResult.iResult,
	            pRsp->uiPeerCount, 
		        inet_ntoa(addrIP),
			    pPeer->usPort);
    }
}


void CDfsManager::ProcessGetReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	g_tDfsManagerStatus.uiGetReqMsgCount++;

	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLGETREQ  pReqMsg = (T_PDFSPROTOCOLGETREQ)(pBuf);
	char* pFileName = (char*)(pReqMsg+1);
	char pszFileNameTemp[1024];
	memcpy(pszFileNameTemp, pFileName, pReqMsg->uiFileNameLen);
	pszFileNameTemp[pReqMsg->uiFileNameLen]=0;

//	char pFileNameTest[32]="";
//	memset(pFileNameTest, 0, 32);
//	memcpy(pFileNameTest, "11.conf", 7);
//	string strFileNameTest = "11.conf";
	//get guid
	T_GUID tcid;
	//CCommonStruct::GetGUID((char*)strFileNameTest.c_str(), pReqMsg->uiFileNameLen, &tcid);
	CCommonStruct::GetGUID(pFileName, pReqMsg->uiFileNameLen, &tcid);

	log_info(g_pLogHelper, "DM::ProGetReqMsg recv get req from IP:%s port:%d cid:%s filenamelen:%d filename:%s addrPack:%p filePos:%p",
			pszIP, usPort, CCommonStruct::GUIDToString(tcid).c_str(), pReqMsg->uiFileNameLen, pszFileNameTemp, pBuf, pFileName);
	//find in local dhtdataset
	m_muxDataSet.Lock();

	struct timeval tvFindStart, tvFindEnd;
	gettimeofday(&tvFindStart, 0);
	T_PDHTDATA pDhtData = m_DhtDataSet.FindByCGuid(tcid);
	gettimeofday(&tvFindEnd, 0);

	tvSpac.tv_sec	= tvFindEnd.tv_sec - tvFindStart.tv_sec;
	tvSpac.tv_usec = tvFindEnd.tv_usec- tvFindStart.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessGetReqMsg process get req cid in map used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

	//pRspMsq.tResult.tType.ucType	= DFS_PROTOCOL_GET_RSP;
//	if(NULL == pDhtData)
	{
		//send find request
		g_tDfsManagerStatus.uiSendFindMsgCount++;

		T_DFSPROTOCOLFINDREQ tFindReqMsg;
		tFindReqMsg.tType.ucType	= DFS_PROTOCOL_FIND_REQ;
		gettimeofday(&tFindReqMsg.tType.tvTimeStamp, 0);
		tFindReqMsg.tCGuid			= tcid;

		T_PEERINFO pPeerInfo[MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT];
		memset(pPeerInfo, 0, sizeof(T_PEERINFO)*MAX_RESOURCE_STORE_COUNT+MAX_SPEC_RESOURCE_STORE_COUNT);
		m_muxPeerList.Lock();
		int iPeersCount = m_PeerList.GetPeersByCID(tcid, pPeerInfo, MAX_RESOURCE_STORE_COUNT, MAX_SPEC_RESOURCE_STORE_COUNT);	
		m_muxPeerList.UnLock();

		struct in_addr addPeerIP;
		for(int i=0; i<iPeersCount; i++)
		{			
			addPeerIP.s_addr = pPeerInfo[i].uiIP;
//			log_info(g_pLogHelper, "DM::ProGetReqMsg send find req to ip:%s port:%d iIndex:%d pcounts:%d cid:%s", 
//					inet_ntoa(addPeerIP), pPeerInfo[i].usPort, i, iPeersCount, CCommonStruct::GUIDToString(tcid).c_str());
		//	m_UdpSocket.Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tFindReqMsg, sizeof(tFindReqMsg));
			pUdpSSocket->Send(pPeerInfo[i].uiIP, pPeerInfo[i].usPort, (char*)&tFindReqMsg, sizeof(tFindReqMsg));
		}

	}
	
	int iPeersCount = 0;
	if(NULL == pDhtData)	
	{
		iPeersCount = 0;
		g_tDfsManagerStatus.uiGetRspMsgFailedCount++;
	}
	else
	{
		iPeersCount = pDhtData->vecDhtPeer.size();
		g_tDfsManagerStatus.uiGetRspMsgSuccessedCount++;
	}

	int iGetRspMsgLen = sizeof(T_DFSPROTOCOLGETRSP)+iPeersCount*sizeof(T_DFSPROTOCOLPEER);
	T_PDFSPROTOCOLGETRSP pGetRspMsg = (T_PDFSPROTOCOLGETRSP)malloc(iGetRspMsgLen);
	pGetRspMsg->tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	pGetRspMsg->tResult.tType.ucType	= DFS_PROTOCOL_GET_RSP;
	pGetRspMsg->tResult.tType.tvTimeStamp.tv_sec = pReqMsg->tType.tvTimeStamp.tv_sec;
	pGetRspMsg->tResult.tType.tvTimeStamp.tv_usec = pReqMsg->tType.tvTimeStamp.tv_usec;
	pGetRspMsg->uiPeerCount			= iPeersCount;
	
	if(NULL != pDhtData)
	{
		ITR_VEC_T_PDHTPEER itr = pDhtData->vecDhtPeer.begin();
		T_PDHTPEER pDhtPeer = NULL;
		T_PDFSPROTOCOLPEER pPeer = (T_PDFSPROTOCOLPEER)(pGetRspMsg+1);
		struct in_addr addPeerIP;
		while(itr != pDhtData->vecDhtPeer.end())
		{
			pDhtPeer = *itr;
			if(NULL !=pDhtPeer)
			{
				pPeer->tMGuid 	= pDhtPeer->tMGuid;
				//strcpy(pPeer->pszIP, pDhtPeer->taddPeer.pszIP);
				pPeer->uiIP = pDhtPeer->taddPeer.uiIP;
				pPeer->usPort = pDhtPeer->taddPeer.uiPort;
				addPeerIP.s_addr = pPeer->uiIP;
				//addPeerIP.s_addr = pDhtPeer->taddPeer.uiIP;
				log_info(g_pLogHelper, "DM:: send get resp to ip:%s port:%d peercount:%d cid:%s pip:%s pport:%d timestamp:%d-%d", 
						pszIP, usPort, pGetRspMsg->uiPeerCount, CCommonStruct::GUIDToString(tcid).c_str(),
						inet_ntoa(addPeerIP), pPeer->usPort,
						pGetRspMsg->tResult.tType.tvTimeStamp.tv_sec, pGetRspMsg->tResult.tType.tvTimeStamp.tv_usec);
			}
			pPeer++;
			itr++;
		}
	}

	//PrintGetRspMsg(pGetRspMsg, CCommonStruct::GUIDToString(tcid).c_str());
	m_muxDataSet.UnLock();
	pUdpSSocket->Send(inet_addr(pszIP), usPort, (char*)pGetRspMsg, iGetRspMsgLen);
	//m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pGetRspMsg, iGetRspMsgLen);
	//log_info(g_pLogHelper, "DM:: send get resp pack:%.48x ", pGetRspMsg);

	free(pGetRspMsg);
	pGetRspMsg = NULL;

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessGetReqMsg process get request message used %d milliseconds", 
//			tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
	//log_info(g_pLogHelper, "CDfsManager::ProcessGetReqMsg process get request message used %d microseconds", 
	//		tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);


}

void CDfsManager::ProcessGetRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{

}

void CDfsManager::ProcessUserListReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLUSERLISTREQ pDfsProtocolUserlistReq = (T_PDFSPROTOCOLUSERLISTREQ)(pBuf);

//	log_info(g_pLogHelper, "DM::ProUserListReqMsg recv get user list req. ip:%s port:%d mid:%s",
//			pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolUserlistReq->tMGuid).c_str());

	CAutoMutexLock muxAuto(m_muxPeerList);

	//creat userlist response
	T_PDFSPROTOCOLUSERLISTRSP pDfsProtocolUserlistRsp = NULL;
	int iPeersCount			= 0;
	T_PPEERINFO pPeerInfo	= m_PeerList.CreatePeers(iPeersCount);	

	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers = NULL;
	int iDfsProtocolUserlistRspLen = sizeof(T_DFSPROTOCOLUSERLISTRSP) + iPeersCount*sizeof(T_DFSPROTOCOLPEER);
	pDfsProtocolUserlistRsp = (T_PDFSPROTOCOLUSERLISTRSP)malloc(iDfsProtocolUserlistRspLen);
	memset(pDfsProtocolUserlistRsp, 0, iDfsProtocolUserlistRspLen);

	pDfsProtocolUserlistRsp->tResult.tType.ucType	= DFS_PROTOCOL_USERLIST_RSP;
	pDfsProtocolUserlistRsp->tResult.tType.tvTimeStamp = pDfsProtocolUserlistReq->tType.tvTimeStamp;
	pDfsProtocolUserlistRsp->tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	pDfsProtocolUserlistRsp->uiCount				= iPeersCount;
	if(0 == iPeersCount)
		ptDfsProtocolPeers = NULL;
	else
		ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolUserlistRsp+1);

	struct in_addr addPeerIP;
	for(int i=0; i<iPeersCount; i++)
	{
		addPeerIP.s_addr = pPeerInfo[i].uiIP;
		//log_info(g_pLogHelper, "DM::ProUserListReqMsg  pinfo ip:%s port:%d index:%d total:%d",
		//		inet_ntoa(addPeerIP), pPeerInfo[i].usPort, i, iPeersCount);
		ptDfsProtocolPeers[i].tMGuid = pPeerInfo[i].guid;
		//strcpy(ptDfsProtocolPeers[i].pszIP, pPeerInfo[i].pszIP);
		ptDfsProtocolPeers[i].uiIP = pPeerInfo[i].uiIP;

		ptDfsProtocolPeers[i].usPort = pPeerInfo[i].usPort;
	}

	if(NULL != pPeerInfo)
	{
		m_PeerList.DestoryPeers(pPeerInfo);
		pPeerInfo = NULL;
	}

	pUdpSSocket->Send(inet_addr(pszIP), usPort, (char*)pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen );
	//m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen );
	free(pDfsProtocolUserlistRsp);
	pDfsProtocolUserlistRsp = NULL;

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessUserListReqMsg process user list request message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessUserListReqMsg process user list request message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);


}

void CDfsManager::ProcessUserListRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLUSERLISTRSP pDfsProtocolUserlistRsp = (T_PDFSPROTOCOLUSERLISTRSP)(pBuf);
	if(DFS_PROTOCOL_SUCCESS != pDfsProtocolUserlistRsp->tResult.iResult)
		return;

	CAutoMutexLock muxAuto(m_muxPeerList);
	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers = NULL;
	T_PDFSPROTOCOLPEER pDfsProtocolPeerTemp = NULL;
	ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolUserlistRsp+1);

	//log_info(g_pLogHelper, "DM::ProUserListRspMsg recv user list resp. ip:%s port:%d count:%d",
		   //	pszIP, usPort, pDfsProtocolUserlistRsp->uiCount);

	T_PPEERINFO pPeerInfo = NULL;
	for(int i=0; i<pDfsProtocolUserlistRsp->uiCount; i++)
	{
		pDfsProtocolPeerTemp = ptDfsProtocolPeers+i;

		in_addr addrIP;
		addrIP.s_addr = pDfsProtocolPeerTemp->uiIP;
		//log_info(g_pLogHelper, "DM::ProUserListRspMsg Add peer. ip:%s port:%d mid:%s", 
			//	inet_ntoa(addrIP), pDfsProtocolPeerTemp->usPort, 
			//	CCommonStruct::GUIDToString(pDfsProtocolPeerTemp->tMGuid).c_str());

		pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPeerTemp->tMGuid);
		if(NULL != pPeerInfo)
			continue;

		pPeerInfo = new T_PEERINFO;
		pPeerInfo->guid		= pDfsProtocolPeerTemp->tMGuid;
		
//		strcpy(pPeerInfo->pszIP, pDfsProtocolPeerTemp->pszIP);
		pPeerInfo->uiIP		= pDfsProtocolPeerTemp->uiIP;
		pPeerInfo->usPort = pDfsProtocolPeerTemp->usPort;

		if(CPeerList::PEER_OPERATOR_SUCCESSED !=  m_PeerList.AddPeer(pPeerInfo))
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
		else
			AddPeerToStatistics(pPeerInfo);
	}

	g_bPeerListIsSet = true;


	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessUserListRspMsg process user list response message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessUserListRspMsg process user list response message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

	tvSpac.tv_sec	= tvEnd.tv_sec - pDfsProtocolUserlistRsp->tResult.tType.tvTimeStamp.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - pDfsProtocolUserlistRsp->tResult.tType.tvTimeStamp.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessUserListRspMsg process user list response message used %d microseconds from send user list request message", 
	//		tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

}

void CDfsManager::ProcessPeerJoinReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLPEERJOINREQ pDfsProtocolPeerJoinRsq = (T_PDFSPROTOCOLPEERJOINREQ)(pBuf);
	//log_info(g_pLogHelper, "DM::ProPeerJoinReqMsg recv peer join req. ip:%s port:%d mid:%s",
	//		pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolPeerJoinRsq->tMGuid).c_str());

	CAutoMutexLock muxAuto(m_muxPeerList);
	//add the peer to peer list	
	T_PPEERINFO pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPeerJoinRsq->tMGuid);
	if(NULL == pPeerInfo)
	{
		pPeerInfo = new T_PEERINFO;
		pPeerInfo->guid		= pDfsProtocolPeerJoinRsq->tMGuid;
//		strcpy(pPeerInfo->pszIP, pszIP);
		pPeerInfo->uiIP = inet_addr(pszIP);
		pPeerInfo->usPort = usPort;

		if(CPeerList::PEER_OPERATOR_SUCCESSED !=m_PeerList.AddPeer(pPeerInfo))
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
		else
			AddPeerToStatistics(pPeerInfo);
	}

	//send response
	T_DFSPROTOCOLPEERJOINRSP DfsProtocolPeerJoinRsp;
	DfsProtocolPeerJoinRsp.tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	DfsProtocolPeerJoinRsp.tResult.tType.ucType = DFS_PROTOCOL_PEERJOIN_RSP;
	DfsProtocolPeerJoinRsp.tResult.tType.tvTimeStamp = pDfsProtocolPeerJoinRsq->tType.tvTimeStamp;
	DfsProtocolPeerJoinRsp.tmGuid				= g_confFile.tMGuid;

	pUdpSSocket->Send(inet_addr(pszIP), usPort, (char*)&DfsProtocolPeerJoinRsp, sizeof(DfsProtocolPeerJoinRsp));
	//m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)&DfsProtocolPeerJoinRsp, sizeof(DfsProtocolPeerJoinRsp));

	g_bPeerListIsSet = true;

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessPeerJoinReqMsg process peer join request message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessPeerJoinReqMsg process peer join request message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);


}

void CDfsManager::ProcessPeerJoinRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLPEERJOINRSP pDfsProtocolPeerJoinRsp = (T_PDFSPROTOCOLPEERJOINRSP)(pBuf);
	//log_info(g_pLogHelper, "DM::ProPeerJoinRspMsg, recv peer join resp ip:%s port:%d mid:%s",
	//		pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolPeerJoinRsp->tmGuid).c_str());

	CAutoMutexLock muxAuto(m_muxPeerList);
	T_PPEERINFO pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPeerJoinRsp->tmGuid);
	if(NULL == pPeerInfo)
	{
		pPeerInfo = new T_PEERINFO;
		pPeerInfo->guid				= pDfsProtocolPeerJoinRsp->tmGuid;
//		strcpy(pPeerInfo->pszIP, pszIP);
		pPeerInfo->uiIP = inet_addr(pszIP);
		pPeerInfo->usPort = usPort;

		if(CPeerList::PEER_OPERATOR_SUCCESSED != m_PeerList.AddPeer(pPeerInfo))
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
		else
			AddPeerToStatistics(pPeerInfo);
	}

	g_bPeerListIsSet = true;


	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessPeerJoinRspMsg process peer join response message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessPeerJoinRspMsg process peer join response message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);
	
	tvSpac.tv_sec	= tvEnd.tv_sec - pDfsProtocolPeerJoinRsp->tResult.tType.tvTimeStamp.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - pDfsProtocolPeerJoinRsp->tResult.tType.tvTimeStamp.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessPeerJoinRspMsg process peer join response message used %d microseconds from send join request message", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

}

void CDfsManager::ProcessPeerLeaveReqMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLPEERLEAVEREQ pDfsProtocolPeerLeaveRsq = (T_PDFSPROTOCOLPEERLEAVEREQ)(pBuf);
	//log_info(g_pLogHelper, "DM::ProPeerLeaveReqMsg recv peer Leave req. ip:%s port:%d mid:%s",
	//		pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolPeerLeaveRsq->tmGuid).c_str());

	CAutoMutexLock muxAuto(m_muxPeerList);
	//add the peer to peer list	
	T_PPEERINFO pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPeerLeaveRsq->tmGuid);
	if(NULL != pPeerInfo)
	{
		if(NULL != m_PeerList.RemovePeer(pPeerInfo->guid))
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
	}

	//send response
	T_DFSPROTOCOLPEERLEAVERSP DfsProtocolPeerLeaveRsp;
	DfsProtocolPeerLeaveRsp.tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	DfsProtocolPeerLeaveRsp.tResult.tType.ucType = DFS_PROTOCOL_PEERJOIN_RSP;
	DfsProtocolPeerLeaveRsp.tResult.tType.tvTimeStamp = pDfsProtocolPeerLeaveRsq->tType.tvTimeStamp;
	DfsProtocolPeerLeaveRsp.tmGuid				= g_confFile.tMGuid;

	pUdpSSocket->Send(inet_addr(pszIP), usPort, (char*)&DfsProtocolPeerLeaveRsp, sizeof(DfsProtocolPeerLeaveRsp));
	//m_UdpSocket.Send(inet_addr(pszIP), usPort, (char*)&DfsProtocolPeerLeaveRsp, sizeof(DfsProtocolPeerLeaveRsp));

	g_bPeerListIsSet = true;

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessPeerLeaveReqMsg process peer join request message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
	//log_info(g_pLogHelper, "CDfsManager::ProcessPeerLeaveReqMsg process peer join request message used %d microseconds", 
	//		tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);


}

void CDfsManager::ProcessPeerLeaveRspMsg(char* pszIP, unsigned short usPort, char* pBuf, unsigned int uiBufLen, CUdpSocket* pUdpSSocket)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	T_PDFSPROTOCOLPEERLEAVERSP pDfsProtocolPeerLeaveRsp = (T_PDFSPROTOCOLPEERLEAVERSP)(pBuf);
	//log_info(g_pLogHelper, "DM::ProPeerLeaveRspMsg, recv peer leave resp ip:%s port:%d mid:%s",
	//		pszIP, usPort, CCommonStruct::GUIDToString(pDfsProtocolPeerLeaveRsp->tmGuid).c_str());

	CAutoMutexLock muxAuto(m_muxPeerList);
	T_PPEERINFO pPeerInfo = m_PeerList.FindPeer(pDfsProtocolPeerLeaveRsp->tmGuid);
	if(NULL != pPeerInfo)
	{
		if( NULL !=	m_PeerList.RemovePeer(pPeerInfo->guid) )
		{
			delete pPeerInfo;
			pPeerInfo = NULL;
		}
	}

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
	//log_info(g_pLogHelper, "CDfsManager::ProcessPeerLeaveRspMsg process peer join response message used %d milliseconds", 
	//		tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "CDfsManager::ProcessPeerLeaveRspMsg process peer join response message used %d microseconds", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);
	
	tvSpac.tv_sec	= tvEnd.tv_sec - pDfsProtocolPeerLeaveRsp->tResult.tType.tvTimeStamp.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec - pDfsProtocolPeerLeaveRsp->tResult.tType.tvTimeStamp.tv_usec;
//	log_info(g_pLogHelper, "CDfsManager::ProcessPeerLeaveRspMsg process peer join response message used %d microseconds from send join request message", 
//			tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);
}

void CDfsManager::SendPingReqMsgToPeers()
{
	T_DFSPROTOCOLPINGREQ tPingReq;

//	CAutoMutexLock muxAuto(m_muxPeerList);
	tPingReq.tMGuid			= g_confFile.tMGuid;
	tPingReq.tType.ucType	= DFS_PROTOCOL_PING_REQ;
	gettimeofday(&tPingReq.tType.tvTimeStamp, 0);

	m_PeerList.SendDataToAllPeers(m_UdpSocket, (const char*)&tPingReq, sizeof(tPingReq), 1);
}

void CDfsManager::SendUserListToPeer(unsigned int uiIP, unsigned short usPort)
{
	//creat userlist response
	CAutoMutexLock muxAuto(m_muxPeerList);

	T_PDFSPROTOCOLUSERLISTRSP pDfsProtocolUserlistRsp = NULL;
	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers = NULL;
	int iPeersCount			= 0;
	T_PPEERINFO pPeerInfo	= m_PeerList.CreatePeers(iPeersCount);	

	//log_info(g_pLogHelper, "DM::SendUserListRspMsgToPeers send user list resp to all peers. pCount:%d",
	//		iPeersCount);
	if(NULL == pPeerInfo || 0 == iPeersCount)
	{
		return;	
	}

	int iDfsProtocolUserlistRspLen = sizeof(T_DFSPROTOCOLUSERLISTRSP) + iPeersCount*sizeof(T_DFSPROTOCOLPEER);
	pDfsProtocolUserlistRsp = (T_PDFSPROTOCOLUSERLISTRSP)malloc(iDfsProtocolUserlistRspLen);

	pDfsProtocolUserlistRsp->tResult.tType.ucType	= DFS_PROTOCOL_USERLIST_RSP;
	gettimeofday(&pDfsProtocolUserlistRsp->tResult.tType.tvTimeStamp, 0);
	pDfsProtocolUserlistRsp->tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	pDfsProtocolUserlistRsp->uiCount				= iPeersCount;
	if(0 == iPeersCount)
		ptDfsProtocolPeers = NULL;
	else
		ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolUserlistRsp+1);

	for(int i=0; i<iPeersCount; i++)
	{
		ptDfsProtocolPeers[i].tMGuid = pPeerInfo[i].guid;
		ptDfsProtocolPeers[i].uiIP = pPeerInfo[i].uiIP;
		ptDfsProtocolPeers[i].usPort = pPeerInfo[i].usPort;
	}

	if(NULL != pPeerInfo)
	{
		m_PeerList.DestoryPeers(pPeerInfo);
		pPeerInfo = NULL;
	}
	
	m_UdpSocket.Send(uiIP, usPort, (char*)pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen );
	free(pDfsProtocolUserlistRsp);
	pDfsProtocolUserlistRsp = NULL;
}

void CDfsManager::SendUserListRspMsgToPeers()
{
	//creat userlist response
	CAutoMutexLock muxAuto(m_muxPeerList);

	T_PDFSPROTOCOLUSERLISTRSP pDfsProtocolUserlistRsp = NULL;
	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers = NULL;
	int iPeersCount			= 0;
	T_PPEERINFO pPeerInfo	= m_PeerList.CreatePeers(iPeersCount);	

	//log_info(g_pLogHelper, "DM::SendUserListRspMsgToPeers send user list resp to all peers. pCount:%d",
	//		iPeersCount);
	if(NULL == pPeerInfo || 0 == iPeersCount)
	{
		return;	
	}

	int iDfsProtocolUserlistRspLen = sizeof(T_DFSPROTOCOLUSERLISTRSP) + iPeersCount*sizeof(T_DFSPROTOCOLPEER);
	pDfsProtocolUserlistRsp = (T_PDFSPROTOCOLUSERLISTRSP)malloc(iDfsProtocolUserlistRspLen);

	pDfsProtocolUserlistRsp->tResult.tType.ucType	= DFS_PROTOCOL_USERLIST_RSP;
	gettimeofday(&pDfsProtocolUserlistRsp->tResult.tType.tvTimeStamp, 0);
	pDfsProtocolUserlistRsp->tResult.iResult		= DFS_PROTOCOL_SUCCESS;
	pDfsProtocolUserlistRsp->uiCount				= iPeersCount;
	if(0 == iPeersCount)
		ptDfsProtocolPeers = NULL;
	else
		ptDfsProtocolPeers = (T_PDFSPROTOCOLPEER)(pDfsProtocolUserlistRsp+1);

	for(int i=0; i<iPeersCount; i++)
	{
		ptDfsProtocolPeers[i].tMGuid = pPeerInfo[i].guid;
//		strcpy(ptDfsProtocolPeers[i].pszIP, pPeerInfo[i].pszIP);
		ptDfsProtocolPeers[i].uiIP = pPeerInfo[i].uiIP;
		ptDfsProtocolPeers[i].usPort = pPeerInfo[i].usPort;
	}

	if(NULL != pPeerInfo)
	{
		m_PeerList.DestoryPeers(pPeerInfo);
		pPeerInfo = NULL;
	}

	//log_info(g_pLogHelper, "DM::SendUserListRspMsgToPeers ptrbuf:%p buflen:%d", pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen);
	m_PeerList.SendDataToAllPeers(m_UdpSocket, (const char*)pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen);
	free(pDfsProtocolUserlistRsp);
	pDfsProtocolUserlistRsp = NULL;
//	m_UdpSocket.Send(pszIP, usPort, (char*)pDfsProtocolUserlistRsp, iDfsProtocolUserlistRspLen );
}

void CDfsManager::AddPeerToStatistics(const T_PPEERINFO cpPeerInfo)
{
	if(NULL != m_pStatistic)
	{
		in_addr t1;
		t1.s_addr=cpPeerInfo->uiIP;
		m_pStatistic->addPeer(&cpPeerInfo->guid, inet_ntoa(t1), cpPeerInfo->usPort );
	}
}
