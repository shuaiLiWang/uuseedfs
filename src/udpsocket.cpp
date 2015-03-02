
#include "udpsocket.h"

CUdpSocket::CUdpSocket()
{
	m_packetNum = 0;
	m_localIp = NULL;
	m_localPort = 0;
	m_listenFd = -1;
	m_bUsed = false;
}

CUdpSocket::~CUdpSocket()
{
	m_packetNum = 0;
	m_localIp = NULL;
	m_localPort = 0;
	m_listenFd = -1;
}

int CUdpSocket::Init(const char* localIp, unsigned int localPort)
{
	m_localIp = localIp;
	m_localPort = localPort;

	pthread_t thread;
	pthread_create(&thread, NULL, RecvData, this);
	pthread_create(&thread, NULL, SendAgain, this);
	return 1;	
}

void* CUdpSocket::RecvData(void* param)
{
	pthread_detach(pthread_self());

	CUdpSocket *pThis = static_cast<CUdpSocket*>(param);

	char *recvBuf = new char[MAX_BUF_LEN];
	if(recvBuf == NULL)
	{
		log_err(g_pLogHelper, "(US::RecvData) new buf failed(%s)", strerror(errno));
		pthread_exit((void*)0);
	}

	//socket
	pThis->m_listenFd = socket(AF_INET, SOCK_DGRAM, 0);		
	if(pThis->m_listenFd == -1)
	{
		log_err(g_pLogHelper, "(US::RecvData) recv socket create failed(%s)\n", strerror(errno));
		delete [] recvBuf;
		recvBuf = NULL;
		pthread_exit((void*)0);
	}

	//set recvbuf
	int bufsize = MAX_RECV_BUF_SIZE;
	setsockopt(pThis->m_listenFd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(bufsize));

	//bind
	struct sockaddr_in localAdd;
	memset(&localAdd, 0, sizeof(localAdd));
	localAdd.sin_family 	 = AF_INET;
	if(pThis->m_localIp == NULL)
	{
		localAdd.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		localAdd.sin_addr.s_addr = inet_addr(pThis->m_localIp);
	}

	localAdd.sin_port = htons(pThis->m_localPort);

	int ret = bind(pThis->m_listenFd, (struct sockaddr*)&localAdd, sizeof(localAdd));
	if(ret == -1)
	{
		log_err(g_pLogHelper, "(US::RecvData) bind failed(%s)\n", strerror(errno));
		pThis->m_listenFd = -1;
		delete [] recvBuf;
		recvBuf = NULL;
		pthread_exit((void*)0);
	}

	while(1)
	{
		//		T_PACKNODE packNode;
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(pThis->m_listenFd, &fd);
		struct timeval timeOut;
		timeOut.tv_sec =10;
		timeOut.tv_usec = 0;

		int sleret = select(pThis->m_listenFd + 1, &fd, NULL, NULL, &timeOut);
		if(sleret == 0)
		{
			//log_info(g_pLogHelper,"(CUdpSocket::RecvData) select timeout(%s)\n", strerror(errno));
			continue;
		}
		else if(sleret < 0)
		{
			log_err(g_pLogHelper, "(US::RecvData) select error(%s)", strerror(errno));
			delete [] recvBuf;
			recvBuf = NULL;
			pthread_exit((void*)0);
		}

		struct sockaddr_in fromAdd;
		memset(&fromAdd, 0, sizeof(sockaddr_in));
		int fromLen = sizeof(fromAdd);

		memset(recvBuf, 0, MAX_BUF_LEN);
		int recvNum = recvfrom(pThis->m_listenFd, recvBuf, MAX_BUF_LEN, 0, (struct sockaddr*)&fromAdd, (socklen_t*)&fromLen);
		if(recvNum == -1)
		{
			//	log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv data error(%s)", strerror(errno));
			continue;	
		}
		else if(recvNum == 0)
		{
			//	log_info(g_pLogHelper, "(CUdpSocket::RecvData) recvNum = 0(%s)", strerror(errno));
			continue;
		}

		//log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv flag %d", ((T_PUUSEEHEAD)recvBuf)->uiUuseeFlag);
		if(((T_PUUSEEHEAD)recvBuf)->uiUuseeFlag != 123456)
		{
			//log_info(g_pLogHelper, "(US::RecvData) recv flag wrong, ignore");
			continue;
		}

		if(((T_PUUSEEHEAD)recvBuf)->Version > CURRENT_VERSION)
		{
			//	log_info(g_pLogHelper, "(US::RecvData) uusee version is too high, ignore");
			continue;
		}

		if(((T_PUUSEEHEAD)recvBuf)->PackType == '1')
		{
			//	log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv SYN from HEAD:%d ,IP:%s, Port:%d", ((T_PUUSEEHEAD)recvBuf)->usPackNum, inet_ntoa(fromAdd.sin_addr), ntohs(fromAdd.sin_port));
			pThis->m_testData.usRecvRspNum ++;
			pThis->m_recvListLock.Lock();
			unsigned short usRecvNum = pThis->m_recvList.size();
			pThis->m_testData.usRecvListNum = usRecvNum;
			if(usRecvNum > MAX_RECV_LIST_COUNT)
			{
				//		log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv list count more than MAX_RECV_LIST_COUNT, ignore");
				pThis->m_recvListLock.UnLock();
				//test
				//usleep(5 * 1000);
				/////////
				continue;
			}
			pThis->m_recvListLock.UnLock();

			unsigned int uiCrc = crc32buf((recvBuf + sizeof(T_UUSEEHEAD)), ((T_PUUSEEHEAD)recvBuf)->uiBufLen);
			if(uiCrc == ((T_PUUSEEHEAD)recvBuf)->uiSendBufCrc)
			{
				//		log_info(g_pLogHelper, "(CUdpSocket::RecvData) crc right");
				//send ack pack
				unsigned int acklen = sizeof(T_UUSEEHEAD); 
				char ackBuf[acklen];
				memset(ackBuf, 0, acklen);
				((T_PUUSEEHEAD)recvBuf)->PackType 		= '2';
				((T_PUUSEEHEAD)recvBuf)->uiSendBufCrc 	= 0;
				memcpy(ackBuf, (T_PUUSEEHEAD)recvBuf, acklen);

				int sendNum = sendto(pThis->m_listenFd, ackBuf, acklen, 0, (struct sockaddr*)&fromAdd, fromLen);
				if(sendNum == -1)
				{
					//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) send ack pack failed(%s)\n", strerror(errno));
				}
				else if(sendNum > 0)
				{
					//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) SEND ACK (ackHead:%d)", ((T_PUUSEEHEAD)recvBuf)->usPackNum);
					pThis->m_testData.usSendRspAckNum ++;
				}

				T_SENDNODE tSendNode;
				tSendNode.sendBuf		= new char[((T_PUUSEEHEAD)recvBuf)->uiBufLen];

				tSendNode.uiIp 			= fromAdd.sin_addr.s_addr;
				tSendNode.usPort 		= ntohs(fromAdd.sin_port);
				tSendNode.uiSendLen 	= ((T_PUUSEEHEAD)recvBuf)->uiBufLen;
				memcpy(tSendNode.sendBuf, (char*)((T_UUSEEHEAD*)recvBuf + 1), tSendNode.uiSendLen);

				//insert recvList
				pThis->m_recvListLock.Lock();
				pThis->m_recvList.push_back(tSendNode);
				pThis->m_recvListLock.UnLock();

				continue;
			}
			else
			{
				//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) crc wrong");
			}
		}
		else if(((T_PUUSEEHEAD)recvBuf)->PackType == '2')
		{
			//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv ACK(%d)from sendList", ((T_PUUSEEHEAD)recvBuf)->usPackNum);
			pThis->m_sendMapLock.Lock();
			pThis->m_testData.usRecvReqAckNum ++;

			ITR_MAP_T_PSENDNODE iter = pThis->m_sendMap.find(((T_PUUSEEHEAD)recvBuf)->usPackNum);
			if(iter != pThis->m_sendMap.end())
			{
				//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv ACK(%d), change ACK_FLAG, before flag(%d)", ((T_PUUSEEHEAD)recvBuf)->usPackNum,(iter->second)->usFlag);
				(iter->second)->usFlag = ACK_FALG;
				//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) after flag(%d)", (iter->second)->usFlag);

				struct timeval tempTime;
				struct timeval lastTime = (iter->second)->tmLastSendTime;
				gettimeofday(&tempTime, NULL);
				unsigned int useTime = ((tempTime.tv_sec - lastTime.tv_sec)*1000) + ((tempTime.tv_usec - lastTime.tv_usec)/1000); 
				if(useTime < pThis->m_testData.usFastTime)
				{
					pThis->m_testData.usFastTime = useTime;
				}
				if(useTime > pThis->m_testData.usSlowTime)
				{
					pThis->m_testData.usSlowTime = useTime;
				}
			}


			//			log_info(g_pLogHelper, "(CUdpSocket::RecvData) recv ACK, delete %d from sendList", ((T_PUUSEEHEAD)buf)->usPackNum);
			//	struct timeval startTime;
			//	gettimeofday(&startTime, NULL);
			/*
			   ITR_LIST_T_SENDNODE iter = pThis->m_sendList.begin();
			   for(iter; iter != pThis->m_sendList.end();)
			   {
			   if(((T_PUUSEEHEAD)(iter->sendBuf))->usPackNum == ((T_PUUSEEHEAD)recvBuf)->usPackNum)
			   {
			   struct timeval tempTime;
			   struct timeval lastTime = iter->tmLastSendTime;
			   gettimeofday(&tempTime, NULL);
			   unsigned int useTime = ((tempTime.tv_sec - lastTime.tv_sec)*1000) + ((tempTime.tv_usec - lastTime.tv_usec)/1000); 
			   if(useTime < pThis->m_testData.usFastTime)
			   {
			   pThis->m_testData.usFastTime = useTime;
			   }
			   if(useTime > pThis->m_testData.usSlowTime)
			   {
			   pThis->m_testData.usSlowTime = useTime;
			   }

			   if(iter->sendBuf != NULL)
			   {
			   delete [] iter->sendBuf;
			   iter->sendBuf = NULL;
			   }
			   iter = pThis->m_sendList.erase(iter);
			   break;
			   }
			   iter ++;
			   }
			   */
			pThis->m_sendMapLock.UnLock();

		}
	}

	delete []recvBuf;
	recvBuf = NULL;	
}

string CUdpSocket::GetStatus()
{
	string retData = "";
	char buf[1024] = {0};
	sprintf(buf, "(CUdpSocket::GetStatues) insertSendListNum:%d RetryTimes:%d ActualSendReqNum:%d RecvReqAckNum:%d RecvRspNum:%d SendRspAckNum:%d SendFaied:%d \nsendListTaskNum:%u FastTime:%d SlowTime:%d RecvListNum:%d", m_GetData.usInsertSendListNum, m_GetData.usRetryTimes, m_GetData.usActualSendReqNum, m_GetData.usRecvReqAckNum, m_GetData.usRecvRspNum, m_GetData.usSendRspAckNum, m_GetData.usSendFailed, m_GetData.usSendListCount, m_GetData.usFastTime, m_GetData.usSlowTime, m_GetData.usRecvListNum);
	retData = buf;
	return retData;
}
void CUdpSocket::OupPutTestData()
{
	log_info(g_pLogHelper, "(CUdpSocket::OutPutTestData) insertSendListNum:%d RetryTimes:%d ActualSendReqNum:%d RecvReqAckNum:%d RecvRspNum:%d SendRspAckNum:%d SendFaied:%d  sendListTaskNum:%u FastTime:%d SlowTime:%d ", m_GetData.usInsertSendListNum, m_GetData.usRetryTimes, m_GetData.usActualSendReqNum, m_GetData.usRecvReqAckNum, m_GetData.usRecvRspNum, m_GetData.usSendRspAckNum, m_GetData.usSendFailed, m_GetData.usSendListCount, m_GetData.usFastTime, m_GetData.usSlowTime);
}

void* CUdpSocket::SendAgain(void *param)
{
	pthread_detach(pthread_self());

	CUdpSocket* pThis = static_cast<CUdpSocket*>(param);

	struct timeval  tmOutPutControlTime;
	gettimeofday(&tmOutPutControlTime, NULL);

	unsigned int sendControlNum = 0;
	time_t       tmLastTime = 0;

	struct timeval timeBefore,timeEnd;
	while(1)
	{
		pThis->m_sendMapLock.Lock();
		if(pThis->m_sendList.empty())
		{
			//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) the send list is empty");
			pThis->m_sendMapLock.UnLock();
			usleep(5 * 1000);
			continue;
		}

		pThis->m_sendMapLock.UnLock();

		//	sleep(1);
		usleep(10*1000);

		gettimeofday(&timeBefore, NULL);
		pThis->m_sendMapLock.Lock();
		ITR_LIST_T_PSENDNODE iter;
		struct timeval tmNow, tmTemp;
		int tempNum = 0;
		for(iter = pThis->m_sendList.begin(); iter != pThis->m_sendList.end(); )
		{
			if(tempNum >= 150)
			{
				break;
			}

			if((*iter)->uiRetryTimes >= 5 || (*iter)->usFlag == ACK_FALG)
			{
				struct in_addr testIp;
				testIp.s_addr = (*iter)->uiIp;
				unsigned int uiTempPack = ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum;

				bool bFlag = false;	
				if((*iter)->usFlag == ACK_FALG)
				{			
					//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) send to IP:%s(%d) (%d times), recv ack, delete", inet_ntoa(testIp), uiTempPack, (*iter)->uiRetryTimes);
					bFlag = true;
				}

				if(!bFlag)
				{
					if((*iter)->uiRetryTimes >= 5)
					{
						pThis->m_testData.usSendFailed ++;
				//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) send to IP:%s(%d) had send morn than 5 times, ignore,falg(%d)", inet_ntoa(testIp), uiTempPack, (*iter)->usFlag);
					}
				}
				
				//erase sendMap
				ITR_MAP_T_PSENDNODE iter_map;
				iter_map = pThis->m_sendMap.find(uiTempPack);
				if(iter_map != pThis->m_sendMap.end())
				{
					if(iter_map->second != NULL)
					{
						iter_map->second = NULL;
					}

					pThis->m_sendMap.erase(iter_map);
				}
				else
				{
					log_info(g_pLogHelper, "ERROR:not find %d", uiTempPack);
				}
				
				//erase sendList;
				if((*iter)->sendBuf != NULL)
				{
					delete [](*iter)->sendBuf;
					(*iter)->sendBuf = NULL;
				}	
				if((*iter) != NULL)
				{
					delete (*iter);
					(*iter) == NULL;
				}
				iter = pThis->m_sendList.erase(iter);


				continue; 
			}

			gettimeofday(&tmNow, NULL);
			unsigned int intervalTime = ((tmNow.tv_sec  - (*iter)->tmLastSendTime.tv_sec)*1000 + (tmNow.tv_usec - (*iter)->tmLastSendTime.tv_usec)/1000);
			if(((*iter)->uiRetryTimes == 0) || (intervalTime > 200))
			{	
				pThis->m_testData.usActualSendReqNum ++;
				//				pThis->m_testData.usSendListCount = pThis->m_sendList.size();
				pThis->m_testData.usSendListCount = pThis->m_sendMap.size();
				if((*iter)->uiRetryTimes == 0)
				{
					//					log_info(g_pLogHelper, "(CUdpSocket::SendAgain) packNum:%d first time send...., totalSendNum:%d, intervalTime:%d", ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum, pThis->m_sendList.size(), intervalTime);

					//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) packNum:%d first time send...., totalSendNum:%d, intervalTime:%d", ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum, pThis->m_sendMap.size(), intervalTime);
				}
				else if((*iter)->uiRetryTimes > 0)
				{
					pThis->m_testData.usRetryTimes ++;
					//log_info(g_pLogHelper, "(CUdpSocket::SendAgain) packNum:%d timeout(lastSendTime>100ms) send....(times:%d) sendlistNum:%d, intervalTime:%d", ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum, (*iter)->uiRetryTimes, pThis->m_sendList.size(), intervalTime);  
					//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) packNum:%d timeout(lastSendTime>100ms) send....(times:%d) sendlistNum:%d, intervalTime:%d", ((T_PUUSEEHEAD)((*iter)->sendBuf))->usPackNum, (*iter)->uiRetryTimes, pThis->m_sendMap.size(), intervalTime);  
				}

				//				log_info(g_pLogHelper, "(CUdpSocket::SendAgain) packNum:%d timeout(lastSendTime>100ms) send....(times:%d) sendlistNum:%d, intervalTime:%d", ((T_PUUSEEHEAD)(iter->sendBuf))->usPackNum, iter->uiRetryTimes, pThis->m_sendList.size(), intervalTime);  
				struct sockaddr_in serverAdd;
				memset(&serverAdd, 0, sizeof(serverAdd));
				serverAdd.sin_family = AF_INET;
				serverAdd.sin_port = (*iter)->usPort;
				serverAdd.sin_addr.s_addr = (*iter)->uiIp;

				if(pThis->m_listenFd == -1)
				{
					//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) socket fd disabled(%s)", strerror(errno));
					iter ++;
					continue;
				}

				int sedNum = sendto(pThis->m_listenFd, (*iter)->sendBuf, (*iter)->uiSendLen, 0, (struct sockaddr*)&serverAdd, sizeof(serverAdd));
				if(sedNum < 0)
				{
					//		log_info(g_pLogHelper, "(CUdpSocket::SendAgain) sendto failed(%s)", strerror(errno));
					iter ++;
					continue;
				}

				gettimeofday(&tmTemp, NULL);
				(*iter)->tmLastSendTime = tmTemp;
				(*iter)->uiRetryTimes ++;

				pThis->m_sendList.push_back(*iter);		
				iter = pThis->m_sendList.erase(iter);

				tempNum ++;
				sendControlNum ++;
			}
			else
			{
				break;
			}
		}
		gettimeofday(&timeEnd, NULL);
		int useTime = (timeEnd.tv_sec - timeBefore.tv_sec)*1000 + (timeEnd.tv_usec - timeBefore.tv_usec)/1000;
		// log_info(g_pLogHelper, "(CUdpSocket::SendAgain) for use time %d", useTime);

		struct timeval outPutTempTime;
		gettimeofday(&outPutTempTime, NULL);
		bool bOutPutFlag = false;
		if(((outPutTempTime.tv_sec - tmOutPutControlTime.tv_sec)*1000 + (outPutTempTime.tv_usec - tmOutPutControlTime.tv_usec)/1000) >= 1000)
		{
			pThis->m_GetData = pThis->m_testData;

			pThis->m_testData.usInsertSendListNum = 0;
			pThis->m_testData.usRetryTimes = 0;
			pThis->m_testData.usActualSendReqNum = 0;
			pThis->m_testData.usRecvReqAckNum = 0; 
			pThis->m_testData.usRecvRspNum = 0; 
			pThis->m_testData.usSendRspAckNum = 0;
			pThis->m_testData.usSendFailed = 0; 
			pThis->m_testData.usSendListCount = 0;
			pThis->m_testData.usFastTime = 100;
			pThis->m_testData.usSlowTime = 0;
			//		pThis->m_testData.usRecvRspNum = 0;

			gettimeofday(&tmOutPutControlTime, NULL);
			bOutPutFlag = true;
		}
		pThis->m_sendMapLock.UnLock();


		//	if(bOutPutFlag)
		//	{
		//		pThis->OupPutTestData();
		//	}

	}
}

int CUdpSocket::GetNetWorkCardInfo()
{
	int fd;
	struct ifconf ifc;

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		log_err(g_pLogHelper, "US::GetNetWorkCardInfo socket failed!(%s)", strerror(errno));
		return -1;
	}

	ifc.ifc_len = sizeof(m_buf);
	ifc.ifc_buf = (caddr_t)m_buf;
	if(!ioctl(fd, SIOCGIFCONF, (char*)&ifc))
		m_networkCardNum = ifc.ifc_len/sizeof(struct ifreq);
	//	log_info(g_pLogHelper, "CUdpSocket::GetNetWorkCardInfo network card num is %d \n", m_networkCardNum);
	//	while(m_networkCardNum -- > 0)
	return fd;
}

string CUdpSocket::GetLocalIp(int fd, const string& eth)
{
	if(fd == -1)
	{
		log_err(g_pLogHelper, "US::GetLocalIp bad socket_fd");
		return "";
	}
	char ipbuf[16];    
	struct ifreq req;    
	struct sockaddr_in *host;    
	bzero(&req, sizeof(struct ifreq));   
	strcpy(req.ifr_name, eth.c_str());    
	ioctl(fd, SIOCGIFADDR, &req);    
	host = (struct sockaddr_in*)&req.ifr_addr;    
	strcpy(ipbuf, inet_ntoa(host->sin_addr));    
	close(fd);    
	string r = string(ipbuf);    
	return r; 
}

int CUdpSocket::Send(unsigned int ip, unsigned short port, const char* buf, unsigned int len)
{
	//if the param ip is myself, not send data
	//wangls 2011/12/8

	/*	int fd = GetNetWorkCardInfo();
		while(m_networkCardNum -- > 0)
		{
		string stempIp = GetLocalIp(fd, m_buf[m_networkCardNum].ifr_name);
		if(0 == stempIp.compare(ip) && 0 != stempIp.compare(""))
		{
		log_info(g_pLogHelper, "CUdpSocket::Send IP is Local ip...,not send...");
		return LOCAL_IP;
		}
		}
		*/	
	//	if(buf == NULL)
	//	{
	//		log_info(g_pLogHelper, "CUdpSocket::Send send buf is NULL, do nothing....");
	//		return SEND_FAILED;
	//	}

	/*char *sendBuf = new char[MAX_BUF_LEN];	
	  memset(sendBuf, 0, MAX_BUF_LEN);

	  ((T_PUUSEEHEAD)sendBuf)->uiUuseeFlag 	= 123456;
	  ((T_PUUSEEHEAD)sendBuf)->Version 	   	= '1';//current version = 1
	  ((T_PUUSEEHEAD)sendBuf)->PackType    	= '1';
	  ((T_PUUSEEHEAD)sendBuf)->usPackNum   	= m_packetNum;
	  ((T_PUUSEEHEAD)sendBuf)->uiBufLen	   	= len;
	  ((T_PUUSEEHEAD)sendBuf)->uiSendBufCrc	= crc32buf((char*)buf, len);

	  memcpy(sendBuf + sizeof(T_UUSEEHEAD), buf, len); 	
	  */
	m_sendMapLock.Lock();

	T_UUSEEHEAD tUuseeHead;
	tUuseeHead.uiUuseeFlag    = 123456;
	tUuseeHead.Version        = '1';//current version = 1
	tUuseeHead.PackType       = '1';
	tUuseeHead.usPackNum      = m_packetNum;
	tUuseeHead.uiBufLen       = len;
	tUuseeHead.uiSendBufCrc   = crc32buf((char*)buf, len);

	T_PSENDNODE tpSendNode = new T_SENDNODE;
	tpSendNode->sendBuf       = new char[sizeof(T_UUSEEHEAD) + len];

	memcpy(tpSendNode->sendBuf, &tUuseeHead, sizeof(T_UUSEEHEAD));
	memcpy(tpSendNode->sendBuf + sizeof(T_UUSEEHEAD), buf, len);

	tpSendNode->usFlag 		= DEFAULT;
	tpSendNode->uiIp	 		= ip;
	tpSendNode->usPort 		= htons(port);
	tpSendNode->uiSendLen		= sizeof(T_UUSEEHEAD) + len;
	tpSendNode->uiRetryTimes  = 0;
	gettimeofday(&tpSendNode->tmLastSendTime, NULL);

	//m_sendMapLock.Lock();
	m_sendList.push_front(tpSendNode);
	ITR_MAP_T_PSENDNODE iter = m_sendMap.find(m_packetNum);
	if(iter != m_sendMap.end())
	{
		log_info(g_pLogHelper, "ERROR:%d had exist.....", m_packetNum);
	}
	m_sendMap[m_packetNum] = tpSendNode;
	//	m_sendMapLock.UnLock();

	m_testData.usInsertSendListNum ++;
	m_packetNum ++;
	m_sendMapLock.UnLock();

	return SEND_SUCCESS;
}

int CUdpSocket::Recv(unsigned int &ip, unsigned short &port, char* buf, unsigned int &len)
{
	int nRet = RECV_LIST_EMPTY;

	m_recvListLock.Lock();
	if(m_recvList.empty())
	{   
		m_recvListLock.UnLock();    
		return nRet;
	}   

	T_SENDNODE sendNode;
	sendNode = m_recvList.front();
	ip = sendNode.uiIp;
	port = sendNode.usPort;
	if(len < sendNode.uiSendLen)
	{   
		nRet =  RECV_BUF_NOT_ENOUGH;
		log_info(g_pLogHelper, "(CUdpSocket::Recv) the buf len not enough....");
	}   
	else
	{   
		memcpy(buf, sendNode.sendBuf, sendNode.uiSendLen);
		nRet = RECV_SUCCESS;
	}   

	len = sendNode.uiSendLen;

	if(sendNode.sendBuf != NULL)
	{   
		delete [] sendNode.sendBuf;
		sendNode.sendBuf = NULL;
	}   
	m_recvList.pop_front();
	m_recvListLock.UnLock();

	return nRet;
}

unsigned int CUdpSocket::GetSendListNum()
{
	m_sendMapLock.Lock();
	//	unsigned int tempRet = m_sendList.size();
	unsigned int mapRet = m_sendMap.size();
	m_sendMapLock.UnLock();
	//	printf("listNum = %d mapNum = %d \n", tempRet, mapRet);
	return mapRet;
}
