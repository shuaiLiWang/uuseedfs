
#ifndef __PEER_LIST_H__
#define __PEER_LIST_H__

#include "commonstruct.h"
#include "udpsocket.h"
#include "statistic.h"

typedef struct _tPeerInfo
{
	T_GUID			guid;					//the peer's identify
//	char			pszIP[MAX_IP_LEN];      //the peer's ip
	uint32_t		uiIP;
	unsigned short	usPort;                 //the peer's port
	time_t			tmKeepaliveTime;        //the peer's keepalive time

	_tPeerInfo operator = (const _tPeerInfo& arRes)
	{
		if(this != &arRes)
		{
			guid			= arRes.guid;
//			strcpy(pszIP, arRes.pszIP);
			uiIP			= arRes.uiIP;
			usPort			= arRes.usPort;
			tmKeepaliveTime = arRes.tmKeepaliveTime;
		}

		return *this;
	}

}T_PEERINFO, *T_PPEERINFO;

class CSortByGuidDistance
{
private:
	T_GUID m_guid;

public:
	CSortByGuidDistance(const T_GUID& arGuid)
	{
		m_guid = arGuid;
	}
	bool operator() (T_PPEERINFO apLeft, T_PPEERINFO apRight)
	{
		int iResult = CCommonStruct::CompareGUID( DistanceGuid(apLeft->guid, m_guid), DistanceGuid(apRight->guid, m_guid) );	
		if(0>iResult)
			return true;
		return false;
	}
};

//typedef std::map<string, T_PPEERINFO>		MAP_T_PPEERINFO;
typedef std::map<T_GUID, T_PPEERINFO>		MAP_T_PPEERINFO;
typedef MAP_T_PPEERINFO::iterator			ITR_MAP_T_PPEERINFO;

typedef std::vector<T_PPEERINFO>			VEC_T_PPEERINFO;
typedef VEC_T_PPEERINFO::iterator			ITR_VEC_T_PPEERINFO;

class CPeerList
{
public:
	CPeerList();
	virtual ~CPeerList();

	enum _epeerfindresult
	{
		PEER_EXIST = -2,
		PEER_ISNOT_EXIST = -1,
		PEER_OPERATOR_SUCCESSED = 0
	};

public:
	string GetStatus();
	void SetStatistic(CStatistics* pStatistic) { m_pStatistic = pStatistic; }
	int GetNormalPeerCount();
	int AddPeer(const T_PPEERINFO acpPeerInfo);
	int RefreshPeer(const T_GUID& acrMGuid);
	T_PPEERINFO RemovePeer(const T_GUID& acrMGuid);

	int AddSpecPeer(const T_PPEERINFO acpPeerInfo);
	int RefreshSpecPeer(const T_GUID& acrMGuid);
	T_PPEERINFO RemoveSpecPeer(const T_GUID& acrMGuid);

	void RemoveOldPeer();
	int GetPeersByCID(const T_GUID& acrCGuid, T_PPEERINFO apPeerInfos, unsigned int auiPeerCount, unsigned int auiSpecPeerCount);
	T_PPEERINFO FindPeer(const T_GUID& acrMGuid);
	T_PPEERINFO FindSpecPeer(const T_GUID& acrMGuid);

	T_PPEERINFO CreatePeers(int& aiSize);
	void DestoryPeers(T_PPEERINFO apPeerInfos);

	void SendDataToAllPeers(CUdpSocket& arUdpSocket, const char* pBuf, int iBufLen, int iType=0);

private:

	CStatistics*	m_pStatistic;

	MAP_T_PPEERINFO m_mapPeerInfo;
	MAP_T_PPEERINFO m_mapSpecPeerInfo;

};


#endif //__DFS_MANAGER_H__
