
#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include "commonstruct.h"
#include "MutexObj.h"
#include "Log.h"
#include "crc32.h"

#define MAX_RECV_BUF_SIZE 102400
#define MAX_BUF_LEN (1024 * 64)
#define MAX_RECV_LIST_COUNT 20000
#define MAX_INTER_FACES 16
#define MAX_CRC_LEN 16
#define CURRENT_VERSION '1'
#define MAX_LIST_NUM 4294967295

enum _esendtype
{
	SYN_TYPE = 1,
	ACK_TYPE = 2
};
enum _erecvflag
{
	DEFAULT = 0,
	ACK_FALG
};

#pragma pack(1)
typedef struct _ttestdata
{
    unsigned short    usInsertSendListNum;
    unsigned short usRetryTimes;
    unsigned short usActualSendReqNum;
	volatile unsigned short usRecvReqAckNum;
    unsigned short usRecvRspNum;
    unsigned short usSendRspAckNum;
	unsigned short usSendFailed;
	unsigned int   usSendListCount;
	unsigned short usFastTime;
	unsigned short usSlowTime;
	unsigned short usRecvListNum;	

	_ttestdata()
	{
		usInsertSendListNum = 0;
		usRetryTimes 		= 0;
		usActualSendReqNum 	= 0;
		usRecvReqAckNum 	= 0;
		usRecvRspNum 		= 0;
		usSendRspAckNum 	= 0;
		usSendFailed 		= 0;
		usSendListCount     = 0;
		usFastTime			= 100;
		usSlowTime			= 0;
		usRecvListNum		= 0;
	}

	_ttestdata& operator = (const _ttestdata& param)
	{
		if(this != &param)
		{
			this->usInsertSendListNum = param.usInsertSendListNum;
			this->usRetryTimes        = param.usRetryTimes;
			this->usActualSendReqNum  = param.usActualSendReqNum;
			this->usRecvReqAckNum     = param.usRecvReqAckNum;
			this->usRecvRspNum		  = param.usRecvRspNum;
			this->usSendRspAckNum     = param.usSendRspAckNum;
			this->usSendFailed		  = param.usSendFailed;
			this->usSendListCount     = param.usSendListCount;
			this->usFastTime		  = param.usFastTime;
			this->usSlowTime		  = param.usSlowTime;
			this->usRecvListNum 	  = param.usRecvListNum;
		}
		return *this;
	}
}T_TESTDATA, *T_PTESTDATA;


//use by sendAgain
typedef struct _tsendnode
{
	 unsigned short				usPort;
	 unsigned int 				uiIp;
	 unsigned int 				uiSendLen;
	 unsigned int 				uiRetryTimes;
	 char 						*sendBuf;
	 struct timeval				tmLastSendTime;
	 unsigned short				usFlag;	 
}T_SENDNODE, *T_PSENDNODE;

typedef struct _tuuseehead
{
	 unsigned int	usPackNum;
	 unsigned int   uiUuseeFlag;//flag is 123456
	 unsigned int   uiSendBufCrc;
     unsigned int  	uiBufLen;
	 char		  	PackType;//SYN_TYPE = 1; ACK_TYPE = 2
	 char			Version; //current version = 1
}T_UUSEEHEAD, *T_PUUSEEHEAD;

#pragma pack()
typedef list<T_SENDNODE>			 LIST_T_SENDNODE;
typedef LIST_T_SENDNODE::iterator    ITR_LIST_T_SENDNODE;

typedef list<T_PSENDNODE>            LIST_T_PSENDNODE;
typedef LIST_T_PSENDNODE::iterator   ITR_LIST_T_PSENDNODE;

//typedef map<int, ITR_LIST_T_SENDNODE> MAP_T_ITR_LIST_T_SENDNODE;
//typedef MAP_T_ITR_LIST_T_SENDNODE::iterator ITR_MAP_T_ITR_LIST_T_SENDNODE;
typedef map<unsigned int, T_PSENDNODE>		MAP_T_PSENDNODE;
typedef MAP_T_PSENDNODE::iterator    ITR_MAP_T_PSENDNODE;

class CUdpSocket
{
public:
	
	CUdpSocket();
	virtual ~CUdpSocket();

	enum _esendresult
	{
		LOCAL_IP = -2,
		SEND_FAILED = -1,
		SEND_SUCCESS
	};

	enum _erecvresult
	{
		RECV_LIST_EMPTY = -1,
		RECV_SUCCESS = 1,
		RECV_BUF_NOT_ENOUGH
	};

public:
	void SetUsed(bool bFlag) { m_bUsed = bFlag; }
	bool IsUsed(){ return m_bUsed; }
	int Init(const char* localIp, unsigned int localPort);
	int Send(unsigned int ip, unsigned short port, const char* buf, unsigned int len);
	int Recv(unsigned int &ip, unsigned short &port, char* buf, unsigned int &len);
	unsigned int GetSendListNum();
	string GetStatus();

private:
	static void*	RecvData(void* param);
//	static void*	CheckSendListAndRetry(void* param);
	static void*	SendAgain(void*);
	int				GetNetWorkCardInfo();
	string			GetLocalIp(int fd, const string& eth);
	void 			OupPutTestData();
private:
	bool					m_bUsed;
    unsigned int			m_packetNum;
	unsigned int			m_localPort;
	const char				*m_localIp;
	int						m_listenFd;
	LIST_T_SENDNODE			m_recvList;
	LIST_T_PSENDNODE		m_sendList;	//sendAgain used to save time
	MAP_T_PSENDNODE			m_sendMap;  //find had sended data and erase it, used to save time
	CMutexObj				m_recvListLock;
	CMutexObj				m_sendMapLock;
	struct ifreq			m_buf[MAX_INTER_FACES];
	int						m_networkCardNum;
	T_TESTDATA				m_testData;
	T_TESTDATA				m_GetData;
};


#endif //__UDP_SOCKET_H__
