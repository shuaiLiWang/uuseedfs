
#ifndef __DHT_DATASET_H__
#define __DHT_DATASET_H__

#include "commonstruct.h"

typedef struct _tdhtpeer
{
	T_PEERADDRESS	taddPeer;
	T_GUID			tMGuid;
	time_t			tmKeepalive;
	bool operator == (const _tdhtpeer& acr)
	{
		if(tMGuid == acr.tMGuid)
			return true;
		return false;
	}
}T_DHTPEER, *T_PDHTPEER;

class CEqualDhtPeer
{
public:
	CEqualDhtPeer(const T_DHTPEER& arDhtPeer)
	{
		m_dhtPeer.taddPeer.uiIP		= arDhtPeer.taddPeer.uiIP;
		m_dhtPeer.taddPeer.uiPort	= arDhtPeer.taddPeer.uiPort;
		m_dhtPeer.tMGuid			= arDhtPeer.tMGuid;
		m_dhtPeer.tmKeepalive		= arDhtPeer.tmKeepalive;
	}
	~CEqualDhtPeer(){};

	bool operator()(const T_PDHTPEER& acpDhtPeer)
	{
		if(m_dhtPeer.tMGuid == acpDhtPeer->tMGuid)
			return true;
//		if(m_dhtPeer.taddrPeer.uiIP == acpDhtPeer->taddPeer.uiIP)
//			return true;
			
		return false;
	}

private:
	T_DHTPEER	m_dhtPeer;
};

typedef std::vector<T_PDHTPEER>		VEC_T_PDHTPEER;
typedef VEC_T_PDHTPEER::iterator	ITR_VEC_T_PDHTPEER;

typedef struct _tdhtdata
{
	VEC_T_PDHTPEER  vecDhtPeer;
	T_GUID			tCGuid;
	unsigned int	uiContentSize;
}T_DHTDATA, *T_PDHTDATA;

typedef struct _tContentInfo
{
	T_GUID			tCGuid;
	unsigned int	uiContentSize;

	_tContentInfo operator = (const _tContentInfo& arRes)
	{
		if(this != &arRes)
		{
			tCGuid = arRes.tCGuid;
			uiContentSize = arRes.uiContentSize;
		}
		return *this;
	}
}T_CONTENTINFO, *T_PCONTENTINFO;

typedef std::vector<T_CONTENTINFO>		VEC_CONTENTINFO;
typedef VEC_CONTENTINFO::iterator		ITR_VEC_CONTENTINFO;

typedef std::map<T_GUID, T_PDHTDATA>	MAP_T_PDHTDATA;
typedef MAP_T_PDHTDATA::iterator		ITR_MAP_T_PDHTDATA;

class CDhtDataSet
{
public:
	
	CDhtDataSet();
	virtual ~CDhtDataSet();

	enum _epeerfindresult
	{
		DHTDATA_EXIST = -2,
		DHTDATA_ISNOT_EXIST = -1,
		DHTDATA_OPERATOR_SUCCESSED = 0
	};

public:
	string GetStatus();

	int AddDhtData(const T_GUID& arCID, unsigned int aruiContentSize, T_PDHTPEER apDhtPeer);
	//int AddDhtData(T_PDHTDATA apDhtData);
//	int RefreshDhtData(T_PDHTDATA apDhtData);
	int RefreshDhtData(const T_GUID& arCID, T_PDHTPEER apDhtPeer);
	void RemoveDhtData(const T_GUID& arCID, T_PDHTPEER apDhtPeer);

	void RemoveOldDhtData();
	//void RemoveDhtData(T_PDHTDATA apDhtData);
	unsigned int GetSize();
	T_PDHTDATA FindByCGuid(const T_GUID& acrCGuid);

	void GetAllDhtData(VEC_CONTENTINFO& vecCI);
		
private:
	MAP_T_PDHTDATA	m_mapDhtData;

};


#endif //__DHT_DATASET__
