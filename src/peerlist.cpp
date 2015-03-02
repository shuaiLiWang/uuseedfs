
#include "peerlist.h"
#include <algorithm>

#include "Log.h"

#define DEFINE_PEER_ALIVE_PERIOD		600
//#define DEFINE_PEER_ALIVE_PERIOD		120
//#define DEFINE_PEER_ALIVE_PERIOD		3600

CPeerList::CPeerList() : m_pStatistic(NULL)
{
}

CPeerList::~CPeerList()
{
}

string CPeerList::GetStatus()
{
	char pszPeerSize[64]="";
	sprintf(pszPeerSize, "CPeerList::GetStatus peer size is %d.", m_mapPeerInfo.size()+m_mapSpecPeerInfo.size());
	return pszPeerSize;
}

int CPeerList::GetNormalPeerCount()
{
	return m_mapPeerInfo.size();	
}

int CPeerList::AddPeer(const T_PPEERINFO acpPeerInfo)
{
	if( 0 == acpPeerInfo->guid.pID[0] &&	0 == acpPeerInfo->guid.pID[1] &&
		0 == acpPeerInfo->guid.pID[2] &&	0 == acpPeerInfo->guid.pID[3]	  )
		return AddSpecPeer(acpPeerInfo);

	T_PPEERINFO pExistPeerInfo = FindPeer(acpPeerInfo->guid);
	if(NULL != pExistPeerInfo)
	{
		time(&pExistPeerInfo->tmKeepaliveTime);
		return PEER_EXIST;
	}

	struct in_addr addPeerIP;
	addPeerIP.s_addr = acpPeerInfo->uiIP;
//	log_info(g_pLogHelper, "PL::AddPeer ip:%s, port:%d, mguid:%s",
//			inet_ntoa(addPeerIP), acpPeerInfo->usPort, CCommonStruct::GUIDToString(acpPeerInfo->guid).c_str());

	time(&(acpPeerInfo->tmKeepaliveTime));
	m_mapPeerInfo[acpPeerInfo->guid] = acpPeerInfo;
	//m_mapPeerInfo[CCommonStruct::GUIDToString(acpPeerInfo->guid)] = acpPeerInfo;
	return PEER_OPERATOR_SUCCESSED;
}

int CPeerList::AddSpecPeer(const T_PPEERINFO acpPeerInfo)
{
	T_PPEERINFO pExistPeer = FindSpecPeer(acpPeerInfo->guid);
	if(NULL != pExistPeer)
	{
		time(&pExistPeer->tmKeepaliveTime);
		return PEER_EXIST;
	}

	struct in_addr addPeerIP;
	addPeerIP.s_addr = acpPeerInfo->uiIP;
//	log_info(g_pLogHelper, "PL::AddSpecPeer ip:%s port:%d mid:%s",
//			inet_ntoa(addPeerIP), acpPeerInfo->usPort, CCommonStruct::GUIDToString(acpPeerInfo->guid).c_str());

	time(&acpPeerInfo->tmKeepaliveTime);
	m_mapSpecPeerInfo[acpPeerInfo->guid] = acpPeerInfo;
	return	PEER_OPERATOR_SUCCESSED;
}

int CPeerList::RefreshPeer(const T_GUID& acrMGuid)
{
	if( 0 == acrMGuid.pID[0] &&	0 == acrMGuid.pID[1] &&	0 == acrMGuid.pID[2] &&	0 == acrMGuid.pID[3]  )
		return RefreshSpecPeer(acrMGuid);

	T_PPEERINFO pExistPeerInfo = FindPeer(acrMGuid);
	if(NULL == pExistPeerInfo)
		return PEER_ISNOT_EXIST;

	time(&(pExistPeerInfo->tmKeepaliveTime));
	return PEER_OPERATOR_SUCCESSED;
}

int CPeerList::RefreshSpecPeer(const T_GUID& acrMGuid)
{
	T_PPEERINFO pExistPeerInfo = FindSpecPeer(acrMGuid);
	if(NULL == pExistPeerInfo)
		return PEER_ISNOT_EXIST;

	time(&(pExistPeerInfo->tmKeepaliveTime));
	return PEER_OPERATOR_SUCCESSED;
}

T_PPEERINFO CPeerList::RemovePeer(const T_GUID& acrMGuid)
{
	T_PPEERINFO pExistPeerInfo = FindPeer(acrMGuid);
	if(NULL == pExistPeerInfo)
		return NULL;

	m_mapPeerInfo.erase(pExistPeerInfo->guid);
	//m_mapPeerInfo.erase(CCommonStruct::GUIDToString(pExistPeerInfo->guid));
	return pExistPeerInfo;
}

T_PPEERINFO CPeerList::RemoveSpecPeer(const T_GUID& acrMGuid)
{
	T_PPEERINFO pExistPeerInfo = FindSpecPeer(acrMGuid);
	if(NULL == pExistPeerInfo)
		return NULL;

	m_mapSpecPeerInfo.erase(pExistPeerInfo->guid);
	//m_mapPeerInfo.erase(CCommonStruct::GUIDToString(pExistPeerInfo->guid));
	return pExistPeerInfo;
}

void CPeerList::RemoveOldPeer()
{
	ITR_MAP_T_PPEERINFO itr = m_mapPeerInfo.begin();
	T_PPEERINFO pPeerTemp = NULL;
	time_t tmNow;
	time(&tmNow);
	struct in_addr addPeer;
	while(itr != m_mapPeerInfo.end())
	{
		pPeerTemp = itr->second;
		if(NULL != pPeerTemp)
		{
			if( DEFINE_PEER_ALIVE_PERIOD < tmNow - pPeerTemp->tmKeepaliveTime)
			{
				m_mapPeerInfo.erase(itr++);

				if(NULL != m_pStatistic)
				{
					m_pStatistic->delPeer(&pPeerTemp->guid);
				}

				addPeer.s_addr = pPeerTemp->uiIP;
			//	log_info(g_pLogHelper, "PL::RemoveOldPeer remove an old peer. mid:%s prip:%s, pport:%d keepalive:%s space:%d",
			//			CCommonStruct::GUIDToString(pPeerTemp->guid).c_str(), 
			//			inet_ntoa(addPeer), pPeerTemp->usPort,
			//			CCommonStruct::Time2String(pPeerTemp->tmKeepaliveTime),
			//				DEFINE_PEER_ALIVE_PERIOD);
				delete pPeerTemp;
				pPeerTemp = NULL;
				continue;
			}
		}

		itr++;
	}

	//remove spec peer
	itr = m_mapSpecPeerInfo.begin();
	while(itr != m_mapSpecPeerInfo.end())
	{
		pPeerTemp = itr->second;
		if(NULL != pPeerTemp)
		{
			if(DEFINE_PEER_ALIVE_PERIOD < tmNow-pPeerTemp->tmKeepaliveTime)
			{
				m_mapSpecPeerInfo.erase(itr++);;
				delete pPeerTemp;
				pPeerTemp = NULL;
			}
		}
		itr++;
	}
}

/*
void CountDistance( T_PPERINFO pPeerInfo, const T_GUID& acrCGuid)
{
	pPeerInfo->guid = DistanceGuid(pPeerInfo->guid, acrCGuid);	
}
*/

int CPeerList::GetPeersByCID(const T_GUID& acCGuid, T_PPEERINFO apPeerInfos, unsigned int auiPeerCount, unsigned int auiSpecPeerCount)
{
	struct timeval tvStart, tvEnd, tvSpac;
	gettimeofday(&tvStart, 0);

	VEC_T_PPEERINFO vecPeerInfo;
	ITR_MAP_T_PPEERINFO itr = m_mapPeerInfo.begin();
	T_PPEERINFO pPeerInfoTemp = NULL;

	int iIndex = 0;
	while(itr != m_mapPeerInfo.end())
	{
		pPeerInfoTemp = itr->second;
		vecPeerInfo.push_back(pPeerInfoTemp);
		itr++;
	}

	//for_each(vecPeerInfo.begin(), vecPeerInfo.end((), bind2nd(ptr_fun(CountDistance), acCGuid));
	
	CSortByGuidDistance sbgd(acCGuid);
	sort(vecPeerInfo.begin(), vecPeerInfo.end(), sbgd);

	ITR_VEC_T_PPEERINFO itrVec = vecPeerInfo.begin();
	T_PPEERINFO pPeerInfo = NULL;
	int iSize = vecPeerInfo.size();
	struct in_addr addPeerIP;
	while(itrVec != vecPeerInfo.end())
	{
		if(iIndex<auiPeerCount)
		{
			pPeerInfo = *itrVec;
			addPeerIP.s_addr = pPeerInfo->uiIP;
//			log_info(g_pLogHelper, "PL::GetPeersByCID iIndex:%d the cid:%s is need stored in mid:%s pip:%s pPort:%d.iIndex:%d MaxCount:%d size:%d", 
//					iIndex, CCommonStruct::GUIDToString(acCGuid).c_str(), CCommonStruct::GUIDToString(pPeerInfo->guid).c_str(),
//				   	inet_ntoa(addPeerIP), pPeerInfo->usPort, iIndex, auiPeerCount, iSize);
			*(apPeerInfos+iIndex) = *(*itrVec);
			iIndex++;
		}
		else
			break;

		itrVec++;
	}

	// add special peer
	itr = m_mapSpecPeerInfo.begin();
	while(itr != m_mapSpecPeerInfo.end())
	{
		pPeerInfoTemp = itr->second;
		if(NULL != pPeerInfoTemp)
		{
			if(iIndex<auiPeerCount+auiSpecPeerCount)
			{
				addPeerIP.s_addr = pPeerInfoTemp->uiIP;
//				log_info(g_pLogHelper, "PL::GetPeersByCID iIndex:%d the cid:%s is need stored in mid:%s pip:%s pPort:%d.iIndex:%d MaxCount:%d size:%d", 
//						iIndex, CCommonStruct::GUIDToString(acCGuid).c_str(), CCommonStruct::GUIDToString(pPeerInfoTemp->guid).c_str(),
//					   	inet_ntoa(addPeerIP), pPeerInfoTemp->usPort, iIndex, auiPeerCount, iSize);
				*(apPeerInfos+iIndex) = *(pPeerInfoTemp);
				iIndex++;
			}
		}
		itr++;
	}

	gettimeofday(&tvEnd, 0);
	tvSpac.tv_sec	= tvEnd.tv_sec - tvStart.tv_sec;
	tvSpac.tv_usec = tvEnd.tv_usec- tvStart.tv_usec;
//	log_info(g_pLogHelper, "CPeerList::GetPeersByCID find %d peers in %d total peers by cid:%s, used %d milliseconds", 
//			iIndex, iSize, CCommonStruct::GUIDToString(acCGuid).c_str(), tvSpac.tv_sec*1000+tvSpac.tv_usec/1000);
//	log_info(g_pLogHelper, "PL::GetPeersByCID find %d peers in %d total peers by cid:%s, used %d mis", 
//			iIndex, iSize, CCommonStruct::GUIDToString(acCGuid).c_str(), tvSpac.tv_sec*1000*1000+tvSpac.tv_usec);

	return iIndex;
}


T_PPEERINFO CPeerList::FindPeer(const T_GUID& acrMGuid)
{
	ITR_MAP_T_PPEERINFO itr = m_mapPeerInfo.find(acrMGuid);
	if(itr == m_mapPeerInfo.end())
		return NULL;

	return itr->second;
}

T_PPEERINFO CPeerList::FindSpecPeer(const T_GUID& acrMGuid)
{
	ITR_MAP_T_PPEERINFO itr = m_mapSpecPeerInfo.find(acrMGuid);
	if(itr == m_mapSpecPeerInfo.end())
		return NULL;

	return itr->second;

}

T_PPEERINFO CPeerList::CreatePeers(int& aiSize)
{
	T_PPEERINFO pPeerInfos = NULL;
	aiSize = m_mapPeerInfo.size()+m_mapSpecPeerInfo.size();;
	if(0>=aiSize)
		return pPeerInfos;

	pPeerInfos = new T_PEERINFO[aiSize];
	int iIndex = 0;
	ITR_MAP_T_PPEERINFO itr = m_mapPeerInfo.begin();
	T_PPEERINFO pPeerTemp = NULL;
	while(itr != m_mapPeerInfo.end())	
	{
		pPeerTemp = itr->second;
		*(pPeerInfos+iIndex++) = *pPeerTemp;
		itr++;
	}

	itr = m_mapSpecPeerInfo.begin();
	while(itr != m_mapSpecPeerInfo.end())
	{
		pPeerTemp = itr->second;
		if(NULL != pPeerTemp)
		{
			*(pPeerInfos+iIndex++) = *pPeerTemp;
			itr++;
		}
	}

	return pPeerInfos;
}

void CPeerList::DestoryPeers(T_PPEERINFO apPeerInfos)
{
	if(NULL != apPeerInfos)
		delete[] apPeerInfos;
}

void CPeerList::SendDataToAllPeers(CUdpSocket& arUdpSocket, const char* pBuf, int iBufLen, int iType)
{
	ITR_MAP_T_PPEERINFO itr = m_mapPeerInfo.begin();
	T_PPEERINFO pPeerInfo = NULL;

	struct in_addr addPeerIP;
	int iSize = m_mapPeerInfo.size()+m_mapSpecPeerInfo.size();
	int iIndex=0;
	while(itr != m_mapPeerInfo.end())
	{
		pPeerInfo = itr->second;
		if(NULL != pPeerInfo)
		{
			addPeerIP.s_addr = pPeerInfo->uiIP;
			if(0 != iType)
			{
				log_info(g_pLogHelper, "send ping to ip:%s port:%d mid:%s Size:%d, index:%d",
					inet_ntoa(addPeerIP), pPeerInfo->usPort, CCommonStruct::GUIDToString(pPeerInfo->guid).c_str(), iSize, iIndex++);
			}

			arUdpSocket.Send(pPeerInfo->uiIP, pPeerInfo->usPort, pBuf, iBufLen);
		}

		itr++;
	}

	itr = m_mapSpecPeerInfo.begin();
	while(itr != m_mapSpecPeerInfo.end())
	{
		pPeerInfo = itr->second;
		if(NULL != pPeerInfo)
		{
			addPeerIP.s_addr = pPeerInfo->uiIP;
			log_info(g_pLogHelper, "send ping req to spec ip:%s port:%d guid:%s pSize:%d, index:%d",
					inet_ntoa(addPeerIP), pPeerInfo->usPort, CCommonStruct::GUIDToString(pPeerInfo->guid).c_str(), iSize, iIndex++);
			arUdpSocket.Send(pPeerInfo->uiIP, pPeerInfo->usPort, pBuf, iBufLen);
		}

		itr++;
	}	
}


