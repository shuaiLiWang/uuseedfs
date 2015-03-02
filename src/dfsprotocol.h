
#ifndef __DFS_PROTOCOL_H__
#define __DFS_PROTOCOL_H__

#include "commonstruct.h" 

enum _edfsprotocoltype
{
	DFS_PROTOCOL_PING_REQ=0,
	DFS_PROTOCOL_PING_RSP=1,
	DFS_PROTOCOL_STORE_REQ=2,
	DFS_PROTOCOL_STORE_RSP=3,
	DFS_PROTOCOL_FIND_REQ=4,
	DFS_PROTOCOL_FIND_RSP=5,
	DFS_PROTOCOL_USERLIST_REQ=6,
	DFS_PROTOCOL_USERLIST_RSP=8,
	DFS_PROTOCOL_PEERJOIN_REQ=9,
	DFS_PROTOCOL_PEERJOIN_RSP=10,
	DFS_PROTOCOL_GET_REQ=11,
	DFS_PROTOCOL_GET_RSP=12,
	DFS_PROTOCOL_PEERLEAVE_REQ=13,
	DFS_PROTOCOL_PEERLEAVE_RSP=14,
	DFS_PROTOCOL_STORE_INFO=15,
	DFS_PROTOCOL_REMOVE_REQ=16,
	DFS_PROTOCOL_REMOVE_RSP=17
};

enum _edfsprotocolresult
{
	DFS_PROTOCOL_NULL=0,
	DFS_PROTOCOL_SUCCESS=1,
	DFS_PROTOCOL_FAILED=2,
	DFS_PROTOCOL_NOTGETPEER=3
};

#pragma pack(1)

//typedef struct _tdfsprotocoltimestamp
//{
//	uint32_t uiSeconds;
//	uint32_t uiMicroseconds;

//}T_DFSPROTOCOLTIMESTAMP, T_DFSPROTOCOLTIMESTAMP;

typedef struct _tdfsprotocoltype
{
	unsigned char ucType;
	struct timeval tvTimeStamp;
//	T_DFSPROTOCOLTIMESTAMP tvTimeStamp;
}T_DFSPROTOCOLTYPE, *T_PDFSPROTOCOLTYPE;

typedef struct _tdfsprotocolrsp
{
	T_DFSPROTOCOLTYPE	tType;
	int					iResult;
}T_DFSPROTOCOLRSP, *T_PDFSPROTOCOLRSP;

typedef struct _tdfsprotocolpeer
{
	T_GUID			tMGuid;
	uint32_t		uiIP;
	//char			pszIP[MAX_IP_LEN];
	unsigned short	usPort;
}T_DFSPROTOCOLPEER, *T_PDFSPROTOCOLPEER;

typedef struct _tdfsprotocolpingreq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tMGuid;	
}T_DFSPROTOCOLPINGREQ, *T_PDFSPROTOCOLPINGREQ;

typedef struct _tdfsprotocolpingrsp
{
	T_DFSPROTOCOLRSP	tResult;
}T_DFSPROTOCOLPINGRSP, *T_PDFSPROTOCOLPINGRSP;

typedef struct _tdfsprotocolstorereq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tMGuid;
	T_GUID				tCGuid;
	unsigned int		uiCSize;
}T_DFSPROTOCOLSTOREREQ, *T_PDFSPROTOCOLSTOREREQ;

typedef struct _tdfsprotocolstorersp
{
	T_DFSPROTOCOLRSP	tResult;
}T_DFSPROTOCOLSTORERSP, *T_PDFSPROTOCOLSTORERSP;

typedef struct _tdfsprotocolremovereq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tMGuid;
	T_GUID				tCGuid;
	unsigned int		uiCSize;
}T_DFSPROTOCOLREMOVEREQ, *T_PDFSPROTOCOLREMOVEREQ;

typedef struct _tdfsprotocolremoversp
{
	T_DFSPROTOCOLRSP	tResult;
}T_DFSPROTOCOLREMOVERSP, *T_PDFSPROTOCOLREMOVERSP;


typedef struct _tdfsprotocolstoreinfo
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tCGuid;
	unsigned short		usFileNameLen;
}T_DFSPROTOCOLSTOREINFO, *T_PDFSPROTOCOLSTOREINFO;

typedef struct _tdfsprotocolfindreq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tCGuid;
}T_DFSPROTOCOLFINDREQ, *T_PDFSPROTOCOLFINDREQ;

typedef struct _tdfsprotocolfindrsp
{
	T_DFSPROTOCOLRSP	tResult;
	T_GUID				tCGuid;
	unsigned int		uiCSize;
	unsigned int		uiCount;
//	T_PDFSPROTOCOLPEER  ptDfsProtocolPeers;	
}T_DFSPROTOCOLFINDRSP, *T_PDFSPROTOCOLFINDRSP;

typedef struct _tdfsprotocolgetreq
{
	T_DFSPROTOCOLTYPE	tType;
	unsigned int		uiFileNameLen;
	//char* pszFileName;
}T_DFSPROTOCOLGETREQ, *T_PDFSPROTOCOLGETREQ;

typedef struct _tdfsprotocolgetrsp
{
	T_DFSPROTOCOLRSP	tResult;
	unsigned int		uiPeerCount;
	//T_DFSPROTOCOLPEER pDfsProtocolPeer;
}T_DFSPROTOCOLGETRSP, *T_PDFSPROTOCOLGETRSP;

typedef struct _tdfsprotocoluserlistreq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tMGuid;
}T_DFSPROTOCOLUSERLISTREQ, *T_PDFSPROTOCOLUSERLISTREQ;
typedef struct _tdfsprotocoluserlistrsp
{
	T_DFSPROTOCOLRSP	tResult;
	unsigned int		uiCount;
//	T_PDFSPROTOCOLPEER	ptDfsProtocolPeers;
}T_DFSPROTOCOLUSERLISTRSP, *T_PDFSPROTOCOLUSERLISTRSP;

typedef struct _tdfsprotocolpeerjoinreq
{
	T_DFSPROTOCOLTYPE	tType;
	T_GUID				tMGuid;
}T_DFSPROTOCOLPEERJOINREQ, *T_PDFSPROTOCOLPEERJOINREQ;

typedef struct _tdfsprotocolpeerjoinrsp
{
	T_DFSPROTOCOLRSP	tResult;
	T_GUID				tmGuid;
}T_DFSPROTOCOLPEERJOINRSP, *T_PDFSPROTOCOLPEERJOINRSP;

typedef struct _tdfsprotocolpeerleavereq
{
	T_DFSPROTOCOLTYPE tType;
	T_GUID				tmGuid;
}T_DFSPROTOCOLPEERLEAVEREQ, *T_PDFSPROTOCOLPEERLEAVEREQ;

typedef struct _tdfsprotocolpeerleaversp
{
	T_DFSPROTOCOLRSP	tResult;
	T_GUID				tmGuid;
}T_DFSPROTOCOLPEERLEAVERSP, *T_PDFSPROTOCOLPEERLEAVERSP;

#pragma pack()

class CDfsProtocol
{
public:
	
	CDfsProtocol();
	virtual ~CDfsProtocol();

public:

private:


};


#endif //__DFS_PROTOCOL_H__
