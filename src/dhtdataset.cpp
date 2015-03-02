
#include "dhtdataset.h"

#include <algorithm>

#include "Log.h"

#define DHT_PEER_MAX_KEEPALIVE		(60*60*24*4.5)

CDhtDataSet::CDhtDataSet()
{
}

CDhtDataSet::~CDhtDataSet()
{
}

string CDhtDataSet::GetStatus()
{
	char pszDhtSize[64]="";
	sprintf(pszDhtSize, "CDhtDataSet::GetStatus dht size is %d", m_mapDhtData.size());
	
	return pszDhtSize;
}

int CDhtDataSet::AddDhtData(const T_GUID& arCID, unsigned int aruiContentSize, T_PDHTPEER apDhtPeer)
{
	//log_info(g_pLogHelper, "DDS::AddDhtData. start function cid:%s add a new", CCommonStruct::GUIDToString(arCID).c_str());
	struct in_addr addPeerIP;
	addPeerIP.s_addr = apDhtPeer->taddPeer.uiIP;

	T_PDHTDATA pDhtData = NULL;
	//find the file first
	pDhtData = FindByCGuid(arCID);
	if(NULL != pDhtData)
	{
		//find the file in peers vector
	//	log_info(g_pLogHelper, "DDS::AddDhtData. find the cid:%s pip:%s pport:%d", 
	//			CCommonStruct::GUIDToString(arCID).c_str(), inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort);
		ITR_VEC_T_PDHTPEER itr = find_if(pDhtData->vecDhtPeer.begin(), pDhtData->vecDhtPeer.end(), CEqualDhtPeer(*apDhtPeer));
		if(itr != pDhtData->vecDhtPeer.end())
		{
//			T_PDHTPEER pDhtPeerTemp = *itr;
			return DHTDATA_EXIST;
		}

		pDhtData->vecDhtPeer.push_back(apDhtPeer);
	//	log_info(g_pLogHelper, "DDS::AddDhtData add dht in an exist cid. cid:%s pip:%s pport:%d, dhtsize:%d",
	//			CCommonStruct::GUIDToString(arCID).c_str(), inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort, m_mapDhtData.size());
		return DHTDATA_OPERATOR_SUCCESSED;
	}
	
//	log_info(g_pLogHelper, "DDS::AddDhtData. not find the cid:%s add a new pip:%s pport:%d", 
//			CCommonStruct::GUIDToString(arCID).c_str(), inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort);
	//find the peer in vector
	pDhtData		= new T_DHTDATA;
	pDhtData->tCGuid		= arCID;
	pDhtData->uiContentSize = aruiContentSize;
	pDhtData->vecDhtPeer.reserve(10);
	pDhtData->vecDhtPeer.push_back(apDhtPeer);
	m_mapDhtData[arCID] = pDhtData;
//	log_info(g_pLogHelper, "DDS::AddDhtData add dht peer a new cid. cid:%s pip:%s pport:%d, dhtsize:%d",
//			CCommonStruct::GUIDToString(arCID).c_str(), inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort, m_mapDhtData.size());
	return DHTDATA_OPERATOR_SUCCESSED;

}

int CDhtDataSet::RefreshDhtData(const T_GUID& arCID, T_PDHTPEER apDhtPeer)
//int CDhtDataSet::RefreshDhtData(T_PDHTDATA apDhtData)
{
	//log_info(g_pLogHelper, "DDS::RefreshDhtData start function cid:%s", CCommonStruct::GUIDToString(arCID).c_str());
	T_PDHTDATA pDhtData = FindByCGuid(arCID);
	if(NULL == pDhtData)
	{
		//log_info(g_pLogHelper, "DDS::RefreshDhtData not found the cid:%s", CCommonStruct::GUIDToString(arCID).c_str());
		return DHTDATA_ISNOT_EXIST;
	}

	struct in_addr addPeerIP;
	addPeerIP.s_addr = apDhtPeer->taddPeer.uiIP;

	T_PDHTPEER pDhtPeerTemp = NULL;
	ITR_VEC_T_PDHTPEER itr = find_if(pDhtData->vecDhtPeer.begin(), pDhtData->vecDhtPeer.end(), CEqualDhtPeer(*apDhtPeer));
	if(itr == pDhtData->vecDhtPeer.end())
	{
//		log_info(g_pLogHelper, "DDS::RefreshDhtData not found.  cid:%s pIP:%s, pPort:%d",
//			   	CCommonStruct::GUIDToString(arCID).c_str(),
//				inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort);
		return DHTDATA_ISNOT_EXIST;
	}

	pDhtPeerTemp = *itr;
	if(NULL != pDhtPeerTemp)
	{
		time(&pDhtPeerTemp->tmKeepalive);

	//	log_info(g_pLogHelper, "DDS::RefreshDhtData refresh peer's time. cid:%s pip:%s pport:%d, dhtsize:%d keepalive:%s",
	//			CCommonStruct::GUIDToString(arCID).c_str(), inet_ntoa(addPeerIP), apDhtPeer->taddPeer.uiPort, m_mapDhtData.size(),
	//			CCommonStruct::Time2String(pDhtPeerTemp->tmKeepalive));
	}
	return DHTDATA_OPERATOR_SUCCESSED;
}

void CDhtDataSet::RemoveDhtData(const T_GUID& arCID, T_PDHTPEER apDhtPeer)
//void CDhtDataSet::RemoveDhtData(T_PDHTDATA apDhtData)
{
	/*
	T_PDHTDATA pDhtData = FindByCGuid(apDhtData->tCGuid);
	if(NULL == pDhtData)
		return;

	m_mapDhtData.erase(apDhtData->tCGuid);
	*/

	T_PDHTDATA pDhtData = FindByCGuid(arCID);
	if(NULL == pDhtData)
		return;

	ITR_VEC_T_PDHTPEER itr = pDhtData->vecDhtPeer.begin();
	T_PDHTPEER pDhtPeerTemp = NULL;
	while(itr != pDhtData->vecDhtPeer.end())
	{
		pDhtPeerTemp = *itr;
		if(NULL != pDhtPeerTemp)
		{
			if(apDhtPeer->tMGuid == pDhtPeerTemp->tMGuid)
			{
				pDhtData->vecDhtPeer.erase(itr);
				delete pDhtPeerTemp;
				pDhtPeerTemp = NULL;
				break;
			}
		}
		itr++;
	}
	
	int iPeersCount = pDhtData->vecDhtPeer.size();
	if(0 == iPeersCount)
	{
		m_mapDhtData.erase(pDhtData->tCGuid);
		delete pDhtData;
		pDhtData = NULL;
	}
}

void CDhtDataSet::RemoveOldDhtData()
{
	//log_info(g_pLogHelper, "DDS::RemoveOldDhtData start............");
	ITR_MAP_T_PDHTDATA itrC = m_mapDhtData.begin();
	T_PDHTDATA pDhtData = NULL;
	time_t tmNow;
	time(&tmNow);
	struct in_addr addPeer;
	while(itrC != m_mapDhtData.end())
	{
		pDhtData = itrC->second;
		if(NULL != pDhtData)
		{
			ITR_VEC_T_PDHTPEER itr = pDhtData->vecDhtPeer.begin();
			T_PDHTPEER pDhtPeerTemp = NULL;
			while(itr != pDhtData->vecDhtPeer.end())
			{
				pDhtPeerTemp = *itr;
				if(NULL != pDhtPeerTemp)
				{
					time_t tmSpace = tmNow - pDhtPeerTemp->tmKeepalive;
					int iDhtPeerMaxKeepalive = g_confFile.uiScanFilePeriod*2.5;
					if(iDhtPeerMaxKeepalive < tmSpace)
					//if(apDhtPeer->tMGuid == pDhtPeerTemp->tMGuid)
					{
						itr = pDhtData->vecDhtPeer.erase(itr);
				
						addPeer.s_addr = pDhtPeerTemp->taddPeer.uiIP;
			//			log_info(g_pLogHelper, "DDS::RemoveOldDhtData erase peer. cid:%s pip:%s pport:%d keepalive:%s space:%d",
			//					CCommonStruct::GUIDToString(pDhtData->tCGuid).c_str(),
			//				   	inet_ntoa(addPeer), pDhtPeerTemp->taddPeer.uiPort,
			//					CCommonStruct::Time2String(pDhtPeerTemp->tmKeepalive),
			//					DHT_PEER_MAX_KEEPALIVE);

						delete pDhtPeerTemp;
						pDhtPeerTemp = NULL;
						continue;
					}
				}
				itr++;
			}
		}

		if(0 == pDhtData->vecDhtPeer.size())
		{
			m_mapDhtData.erase(itrC++);
			//log_info(g_pLogHelper, "DDS::RemoveOldDhtData erase dht data. cid:%s",
			//	   CCommonStruct::GUIDToString(pDhtData->tCGuid).c_str()	);

			delete pDhtData;
			pDhtData = NULL;
			continue;
		}

		itrC++;
	}
//	log_info(g_pLogHelper, "DDS::RemoveOldDhtData end............");
}

unsigned int CDhtDataSet::GetSize()
{
	return m_mapDhtData.size();
}

T_PDHTDATA CDhtDataSet::FindByCGuid(const T_GUID& acrCGuid)
{
	ITR_MAP_T_PDHTDATA itr = m_mapDhtData.find(acrCGuid);
	
	if(itr == m_mapDhtData.end())
		return NULL;

	return itr->second;

}

void CDhtDataSet::GetAllDhtData(VEC_CONTENTINFO& vecCI)
{
	ITR_MAP_T_PDHTDATA itrC = m_mapDhtData.begin();
	T_PDHTDATA pDhtData = NULL;
	T_CONTENTINFO ci;
	bool bLocalDht = false;

	while(itrC != m_mapDhtData.end())
	{
		bLocalDht = false;
		pDhtData = itrC->second;
		if(NULL != pDhtData)
		{
			ITR_VEC_T_PDHTPEER itr = pDhtData->vecDhtPeer.begin();
			T_PDHTPEER pDhtPeerTemp = NULL;
			while(itr != pDhtData->vecDhtPeer.end())
			{
				pDhtPeerTemp = *itr;
				if(NULL != pDhtPeerTemp)
				{
					if(	pDhtPeerTemp->tMGuid == g_confFile.tMGuid)
					{
						bLocalDht = true;
						break;
					}
				}
				itr++;
			}

			if(bLocalDht)
			{
				ci.tCGuid = pDhtData->tCGuid;
				ci.uiContentSize = pDhtData->uiContentSize;
				vecCI.push_back(ci);
			}
		}
		itrC++;
	}

}
