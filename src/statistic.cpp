#include "statistic.h"
#include "CallbackFuncObj.cpp"
#include "Log.h"

#define MAX_STATISTICS_MAPFILE_SIZE			(1*1024*1024*1024)

CStatistics::CStatistics() : m_strPath("")
							 , m_uiFrequence(0)
{
	m_funcWorker.Set(WorkerThreadEntry, this);
	m_funcPrintFile.Set(PrintFileThreadEntry, this);
}

CStatistics::~CStatistics()
{
}

int CStatistics::AddPeerToTotalList(const T_PGUID mid, const char* ip, unsigned short port )
{
	int iRtn = 0;
	//add some at total 
	T_PSPEERINFO pPeerInfo = new T_SPEERINFO;

	strcpy(pPeerInfo->ip, ip);
	pPeerInfo->mid = mid;
	pPeerInfo->port = port;

	ITR_MAP_PEER_FILELIST itr = m_mapPeerAndFileList.find(pPeerInfo);
	if(itr != m_mapPeerAndFileList.end() )
	{
		delete pPeerInfo;
		pPeerInfo = NULL;
		iRtn = -1;
	}
	else
	{
		MAP_PFILEINFO* pmapFileInfo = new MAP_PFILEINFO;
		m_mapPeerAndFileList.insert(pair<T_PSPEERINFO, MAP_PFILEINFO*>(pPeerInfo, pmapFileInfo));
	}
	return iRtn;
}

int CStatistics::addPeer(const T_PGUID  mid, const char* ip, unsigned short port )
{
	int iRtn = m_muxTotalMap.TryLock();

	T_PGUID pmid= new T_GUID;
	*pmid = *mid;

	//log_info(g_pLogHelper, "CS::addPeer mid:%s ip:%s port:%d", CCommonStruct::GUIDToString(*mid).c_str(), ip, port);

	if(EBUSY == iRtn)
	{
		CAutoMutexLock autoLock(m_muxStatTask);

		T_PSTATTASK		pStatTask = new T_STATTASK;
		pStatTask->type = TASK_TYPE_ADD_PEER;
		strcpy(pStatTask->para.addPeer.ip, ip);
		pStatTask->para.addPeer.port = port;
		pStatTask->para.addPeer.mid = pmid;

		m_lstStatTask.push_back(pStatTask);
	}
	else
	{
		if( 0 != AddPeerToTotalList(pmid, ip, port) )
		{
			delete pmid;
			pmid = 0;
		}

		m_muxTotalMap.UnLock();
	}
	return 0;
}

void CStatistics::delPeer(const T_PGUID mid)
{
	int iRtn = m_muxTotalMap.TryLock();

	T_PGUID pmid= new T_GUID;
	*pmid = *mid;

	if(EBUSY == iRtn)
	{
		CAutoMutexLock autoLock(m_muxStatTask);

		T_PSTATTASK		pStatTask = new T_STATTASK;
		pStatTask->type = TASK_TYPE_DEL_PEER;
		pStatTask->para.delPeer.mid = pmid;

		m_lstStatTask.push_back(pStatTask);
	}
	else
	{
		DelPeerFromTotalList(pmid);
		delete pmid;
		pmid = NULL;

		m_muxTotalMap.UnLock();
	}
}

int CStatistics::DelPeerFromTotalList(const T_PGUID mid)
{
	T_SPEERINFO PeerInfo;
	PeerInfo.mid = mid;

	ITR_MAP_PEER_FILELIST itr = m_mapPeerAndFileList.find(&PeerInfo);
	if(itr != m_mapPeerAndFileList.end())
	{
		T_PSPEERINFO pPeerInfoTemp = itr->first;
		MAP_PFILEINFO* pmapFileInfo = itr->second;

		m_mapPeerAndFileList.erase(itr);

		//delete file list
//		ITR_MAP_PFILEINFO itrFile = pmapFileInfo->begin();
//		T_PFILEINFO pFileInfo = NULL;
//		while(itrFile != pmapFileInfo->end())
//		{
//			pFileInfo = itrFile->first;
//			if(NULL != pFileInfo)
//			{
//				delete pFileInfo->cid;
//				pFileInfo->cid = NULL;

//				pmapFileInfo->erase(itrFile++);
//				continue;
//			}
//			itrFile++;
//		}
		delete pmapFileInfo;
		pmapFileInfo = NULL;

		delete pPeerInfoTemp->mid;
		pPeerInfoTemp->mid = NULL;
		delete pPeerInfoTemp;
		pPeerInfoTemp = NULL;
	}

	return 0;
}


int CStatistics::addFile(const T_PGUID fid, const T_PGUID mid, unsigned short cmid, unsigned int timeout)
{
	int iRtn = m_muxTotalMap.TryLock();
	timeout = timeout*2.5;

	//log_info(g_pLogHelper, "CS::addFile fid:%s mid:%s timeout:%d", CCommonStruct::GUIDToString(*fid).c_str(),
	//		CCommonStruct::GUIDToString(*mid).c_str(), timeout);

	if(EBUSY == iRtn)
	{
		CAutoMutexLock autoLock(m_muxStatTask);

		T_PGUID pcid = NULL;
		T_PGUID pmid = NULL;
		T_PGUID pmidTemp = mid;

		for(int i=0; i<cmid; i++)
		{
			pcid = new T_GUID;
			*pcid = *fid;
			pmid= new T_GUID;
			*pmid = *pmidTemp++;
			T_PSTATTASK		pStatTask = new T_STATTASK;
			pStatTask->type = TASK_TYPE_ADD_FILE;
			pStatTask->para.addFile.cid = pcid;
			pStatTask->para.addFile.mid = pmid;
			pStatTask->para.addFile.timeout = timeout;

			m_lstStatTask.push_back(pStatTask);
		}
	}
	else
	{
		AddFileToTotalList(fid, mid, cmid, timeout);	

		m_muxTotalMap.UnLock();
	}
	return 0;
}

void CStatistics::PrintFileInfo(const T_PGUID cid, const char* acpszName)
{
	//log_info(g_pLogHelper, "CS::PrintFileInfo cid:%s name:%s ", CCommonStruct::GUIDToString(*cid).c_str(), acpszName);

	T_PPRINTFILE pPF = new T_PRINTFILE;
	pPF->cid = new T_GUID;
	pPF->pszName = new char[strlen(acpszName)+1];

	*pPF->cid = *cid;
	strcpy(pPF->pszName, acpszName);

	m_lstPrintFile.push_back(pPF);
}

int CStatistics::AddFileToTotalList(const T_PGUID fid, const T_PGUID mid, unsigned short cmid, unsigned int timeout)
{
	T_PFILEINFO pFileInfo = new T_FILEINFO;
	pFileInfo->cid = new T_GUID;
	*pFileInfo->cid = *fid;
	pFileInfo->timeout = timeout;
	time(&pFileInfo->tmAdd);

	ITR_MAP_FILE_PEERLIST itrFilePeer = m_mapFileAndPeerList.find(pFileInfo);
	if(itrFilePeer != m_mapFileAndPeerList.end())
	{
		delete pFileInfo->cid;
		pFileInfo->cid = NULL;
		delete pFileInfo;
		pFileInfo = itrFilePeer->first;
		time(&pFileInfo->tmAdd);
	}
	else
	{
		MAP_PSPEERINFO* pmapPeer = new MAP_PSPEERINFO;
		m_mapFileAndPeerList.insert(pair<T_PFILEINFO, MAP_PSPEERINFO*>(pFileInfo, pmapPeer));
	}

	T_PGUID pmid = mid;
	for(int i=0; i<cmid; i++, pmid++)
	{
		T_SPEERINFO peerInfo;
		peerInfo.mid = pmid;
		
		//find the peer
		ITR_MAP_PEER_FILELIST itrPeer = m_mapPeerAndFileList.find(&peerInfo);
		if(itrPeer == m_mapPeerAndFileList.end())
			continue;
		
		MAP_PFILEINFO* pmapFileInfo = itrPeer->second;
		if(NULL != pmapFileInfo)
		{
			pmapFileInfo->insert(pair<T_PFILEINFO, void*>(pFileInfo, NULL));
		}
	}
	
	return 0;
}

int CStatistics::Init(const char* path, unsigned int frequence)
{
	m_strPath.append(path);
	m_uiFrequence = frequence;

	return 0;
}

int CStatistics::Start()
{
	if(0 != m_threadWorker.Start(&m_funcWorker))
		return -1;

	if(0 != m_threadPrintFile.Start(&m_funcPrintFile))
		return -1;

	return 0;
}

void* CStatistics::PrintFileThreadEntry(void* pParam)
{
	CStatistics* pThis = static_cast<CStatistics*>(pParam);
	if(NULL != pThis)
		pThis->PrintFileThreadImp();

	return NULL;
}

void CStatistics::PrintFileThreadImp()
{
	m_threadPrintFile.ThreadStarted();
	pthread_detach(pthread_self());	

	int iIndex = 0;
	char szFileName[1024]="";
	char szSize[32]="";
	FILE* pFileMap = NULL;
	char szLine[2048]="";
	char tmp[FILENAME_MAX];
	time_t tmNow;
	while(1)	
	{
		iIndex++;
		time(&tmNow);
		strftime(tmp, FILENAME_MAX, "%Y%m%d_%H%M%S", localtime(&tmNow)); 

		//wait a period
		if(10 > m_uiFrequence)
			sleep(10);
		else
			sleep(m_uiFrequence);

		//open statistics log file
		snprintf(szFileName, 1024, "%sstatistics_filemap_%s.log", m_strPath.c_str(), tmp);
		//snprintf(szFileName, 1024, "%sstatistics_filemap_%d.log", m_strPath.c_str(), iIndex);
		//log_info(g_pLogHelper, "CS::PrintFileThread is running. szFileName:%s", szFileName);
		
		if(NULL == pFileMap)
		{
			pFileMap = fopen(szFileName, "a");
		}
		else
		{
			int iFileSize = ftell(pFileMap);
			if(iFileSize >= MAX_STATISTICS_MAPFILE_SIZE )
			{
				fclose(pFileMap);
				pFileMap = fopen(szFileName, "a");
			}
		}

		if(NULL == pFileMap)
		{
			continue;
		}

//		m_muxPrintFile.Lock();
		T_PPRINTFILE pPF = NULL;	
		while(!m_lstPrintFile.empty())
		{
			pPF = m_lstPrintFile.front();
			m_lstPrintFile.pop_front();

			if(NULL == pPF)
				continue;

			WriteFileMapToFile(pFileMap, pPF);			
			
			delete pPF->cid;
			pPF->cid = NULL;
			delete[] pPF->pszName;
			pPF->pszName = NULL;
			delete pPF;
			pPF= NULL;
		}
//		m_muxPrintFile.UnLock();
	}

}

void* CStatistics::WorkerThreadEntry(void* pParam)
{
	CStatistics* pThis = static_cast<CStatistics*>(pParam);
	if(NULL != pThis)
		pThis->WorkerThreadImp();

	return NULL;
}

void CStatistics::WorkerThreadImp()
{
	m_threadWorker.ThreadStarted();
	pthread_detach(pthread_self());

	int iIndex = 0;
	char szFileName[1024]="";
	char szSize[32]="";
	char tmp[FILENAME_MAX];
	time_t tmNow;
	while(1)	
	{
		iIndex++;
		time(&tmNow);
		strftime(tmp, FILENAME_MAX, "%Y%m%d_%H%M%S", localtime(&tmNow)); 

		//wait a period
		if(10 > m_uiFrequence)
			sleep(10);
		else
			sleep(m_uiFrequence);

		//open statistics log file
		snprintf(szFileName, 1024, "%sstatistics_indexbypeer_%s.log", m_strPath.c_str(), tmp);
		//snprintf(szFileName, 1024, "%sstatistics_indexbypeer_%d.log", m_strPath.c_str(), iIndex);
		FILE* pFilePeer = fopen(szFileName, "a");
		if(NULL == pFilePeer)
		{
			continue;
		}

		snprintf(szFileName, 1024, "%sstatistics_indexbyfile_%s.log", m_strPath.c_str(), tmp);
		//snprintf(szFileName, 1024, "%sstatistics_indexbyfile_%d.log", m_strPath.c_str(), iIndex);
		FILE* pFileFile = fopen(szFileName, "a");
		if(NULL == pFileFile)
		{
			fclose(pFilePeer);
			continue;
		}

		//traversal the total map and write the statistics log
		m_muxTotalMap.Lock();

		ITR_MAP_PEER_FILELIST itrPeerFile = m_mapPeerAndFileList.begin();
		ITR_MAP_FILE_PEERLIST itrFilePeer = m_mapFileAndPeerList.end();
		MAP_PFILEINFO* pmapFileInfo = NULL;	
		MAP_PSPEERINFO* pmapPeerInfo = NULL;
		T_PSPEERINFO pPeerInfo = NULL;
		T_PFILEINFO pFileInfo = NULL;
		
		//write peer list to file
		while(itrPeerFile != m_mapPeerAndFileList.end())
		{
			pPeerInfo = itrPeerFile->first;
			if(NULL != pPeerInfo)
			{
				//write peer information to file
				WritePeerInfoToFile(pFilePeer, pPeerInfo);	
				
				fwrite("{", 1, 1, pFilePeer);	

				//write file information to file
				pmapFileInfo = itrPeerFile->second;
				if(NULL != pmapFileInfo)
				{
					ITR_MAP_PFILEINFO itrFile = pmapFileInfo->begin();
					while(itrFile != pmapFileInfo->end())
					{
						pFileInfo = itrFile->first;
						if(NULL != pFileInfo)
						{
							itrFilePeer = m_mapFileAndPeerList.find(pFileInfo);
							if(itrFilePeer != m_mapFileAndPeerList.end())
							{
								pFileInfo = itrFilePeer->first;
								pmapPeerInfo = itrFilePeer->second;
								if(NULL != pFileInfo && NULL != pmapPeerInfo)
								{
									if(pFileInfo->iUnabled < 1)
									{
										pmapPeerInfo->insert(pair<T_PSPEERINFO, void*>(pPeerInfo, 0));
										WriteFileInfoToFile(pFilePeer, pFileInfo);
									}
									else
									{
										pmapFileInfo->erase(itrFile++);	
										continue;
									}
								}
							}
							else
							{
								pmapFileInfo->erase(itrFile++);
								continue;	
							}
						}
						
						itrFile++;
					}
				}
				fwrite("}\r\n", 1, 3, pFilePeer);	
				fflush(pFilePeer);	
			}
			itrPeerFile++;
		}
		
		//write file information to file
		itrFilePeer = m_mapFileAndPeerList.begin();
		while(itrFilePeer != m_mapFileAndPeerList.end())
		{
			pFileInfo = itrFilePeer->first;
			pmapPeerInfo = itrFilePeer->second;

			if(NULL != pFileInfo && NULL != pmapPeerInfo)
			{
				time_t tmNow;
				time(&tmNow);
				int iSpace = (int)(tmNow - pFileInfo->tmAdd);
				//if(iSpace > pFileInfo->timeout && 0 >= pmapPeerInfo->size())
				if(iSpace > pFileInfo->timeout)
				{
					//log_info(g_pLogHelper, "CS::WorkerThreadImp remove file from file map. iSpace:%d now:%lld create:%lld",
				//		iSpace, tmNow, pFileInfo->tmAdd);
					pFileInfo->iUnabled++;

					if(5 <= pFileInfo->iUnabled)
					{
						m_mapFileAndPeerList.erase(itrFilePeer++);
						delete pFileInfo->cid;
						pFileInfo->cid = NULL;
						delete pFileInfo;
						pFileInfo = NULL;
						delete pmapPeerInfo;
						pmapPeerInfo = NULL;
						continue;
					}
				}
			
				WriteFileInfoToFile(pFileFile, pFileInfo);
				fwrite("{", 1, 1, pFileFile);	
				
				snprintf(szSize, 32, "size:%d:", pmapPeerInfo->size());
				fwrite(szSize, 1, strlen(szSize), pFileFile);	

				ITR_MAP_PSPEERINFO itrPeer = pmapPeerInfo->begin();
				while(itrPeer != pmapPeerInfo->end())				
				{
					pPeerInfo = itrPeer->first;
					if(NULL != pPeerInfo)
						WritePeerInfoToFile(pFileFile, pPeerInfo);

					itrPeer++;
				}
				fwrite("}\r\n", 1, 3, pFileFile);	
				fflush(pFileFile);

				pmapPeerInfo->clear();
			}

			itrFilePeer++;
		}

		//add tempory list task to total list
		m_muxStatTask.Lock();

		T_PSTATTASK pStatTask = NULL;
		while(!m_lstStatTask.empty())
		{
			pStatTask = m_lstStatTask.front();
			m_lstStatTask.pop_front();

			if(NULL == pStatTask)
				continue;

			AddStatTaskToTotalListAndReleaseTask(pStatTask);
		}

		m_muxStatTask.UnLock();


		m_muxTotalMap.UnLock();

finished:

		fclose(pFilePeer);
		pFilePeer = NULL;
		
		fclose(pFileFile);
		pFileFile = NULL;
	}
}

void CStatistics::Stop()
{
	m_threadWorker.Kill();
	m_threadPrintFile.Kill();
}

int CStatistics::AddStatTaskToTotalListAndReleaseTask( T_PSTATTASK pStatTask)
{
	switch(pStatTask->type)
	{
	case TASK_TYPE_ADD_PEER:
		{
			if( 0 != AddPeerToTotalList(pStatTask->para.addPeer.mid, pStatTask->para.addPeer.ip, pStatTask->para.addPeer.port) )
			{
				delete pStatTask->para.addPeer.mid;
				pStatTask->para.addPeer.mid = NULL;
			}
			delete pStatTask;
			pStatTask = NULL;
		}
		break;
	case TASK_TYPE_DEL_PEER:
		{
			DelPeerFromTotalList(pStatTask->para.delPeer.mid);
			delete pStatTask->para.delPeer.mid;
			pStatTask->para.delPeer.mid = NULL;
			delete pStatTask;
			pStatTask = NULL;
		}
		break;
	case TASK_TYPE_ADD_FILE:
		{
			AddFileToTotalList(pStatTask->para.addFile.cid, pStatTask->para.addFile.mid, 1, pStatTask->para.addFile.timeout);
			delete pStatTask->para.addFile.cid;
			pStatTask->para.addFile.cid = NULL;
			delete pStatTask->para.addFile.mid;
			pStatTask->para.addFile.mid = NULL;
			delete pStatTask;
			pStatTask = NULL;
		}
		break;
	default:
		break;
	}
	return 0;
}

int CStatistics::WritePeerInfoToFile(FILE* pFile, T_PSPEERINFO pPeerInfo)
{
	if(NULL == pFile)
		return -1;

	string strGuid = CCommonStruct::GUIDToString(*(pPeerInfo->mid));
	char pszPeerInfo[1024] = {""};
	snprintf(pszPeerInfo, 1024, "%s_%s_%d;", strGuid.c_str(), pPeerInfo->ip, pPeerInfo->port);

	fwrite(pszPeerInfo, 1, strlen(pszPeerInfo), pFile);

	return 1;
}

int CStatistics::WriteFileInfoToFile(FILE* pFile, T_PFILEINFO pFileInfo)
{
	if(NULL == pFile)
		return -1;

	string strGuid = CCommonStruct::GUIDToString(*(pFileInfo->cid));
	char pszFileInfo[1024] = {""};
	snprintf(pszFileInfo, 1024, "%s;", strGuid.c_str());

	fwrite(pszFileInfo, 1, strlen(pszFileInfo), pFile);

	return 1;
}

int CStatistics::WriteFileMapToFile(FILE* pFile, T_PPRINTFILE pPrintFile)
{
	if(NULL == pFile)
		return -1;

	char pszInfo[2048]="";
	snprintf(pszInfo, 2048, "id:%s name:%s\r\n", CCommonStruct::GUIDToString(*pPrintFile->cid).c_str(), pPrintFile->pszName);

	fwrite(pszInfo, 1, strlen(pszInfo), pFile);
	fflush(pFile);

	return 1;
}
