
#include "ThreadObj.h"
#include "CallbackFuncObj.cpp"

 int 	ERR_JTHREAD_CANINITMUTEX 		= -1;
 int 	ERR_JTHREAD_CANTSTARTTHREAD		= -2;
 int  	ERR_JTHREAD_THREADFUNCNOTSET 	= -3;
 int 	ERR_JTHREAD_NOTRUNNING 			= -4;
 int  	ERR_JTHREAD_ALREADYRUNNING		= -5;
 int    ERR_JTHREAD_NOT	 				= -6;


CThreadObj::CThreadObj() : m_ThreadID(0)
						, m_bIsRunning(false)
						, m_pCallbackFuncObj(NULL)
{
}

CThreadObj::CThreadObj(TCallbackFuncObj<pThreadCallbackFunc>* pCObj)	: m_ThreadID(0)
																	, m_bIsRunning(false)
																	, m_pCallbackFuncObj(pCObj)
{
}

CThreadObj::~CThreadObj()
{
	Kill();
}

int CThreadObj::Start()
{
	if(NULL == m_pCallbackFuncObj)
		return ERR_JTHREAD_THREADFUNCNOTSET;

	m_muxRunning.Lock();
	if(m_bIsRunning)
	{
		m_muxRunning.UnLock();
		return ERR_JTHREAD_ALREADYRUNNING;
	}
	m_muxRunning.UnLock();
	
	m_muxContinue.Lock();

#ifdef WIN32
		DWORD dwThreadID=0;
		m_ThreadID = CreateThread(NULL,0, ThreadEntry, this,0, &dwThreadID);
		int iRtn = (NULL == m_ThreadID) ? 1 : 0;
#else
		int iRtn = pthread_create(&m_ThreadID,NULL, ThreadEntry,this);
#endif //WIN32

	if(0 != iRtn)
	{
		m_muxContinue.UnLock();
		return  ERR_JTHREAD_CANTSTARTTHREAD;
	}

	//wait until running is set;
	m_muxRunning.Lock();
	while(!m_bIsRunning)
	{
		m_muxRunning.UnLock();
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif //WIN32
	
		m_muxRunning.Lock();
	}
	m_muxRunning.UnLock();

	m_muxContinue.UnLock();
	
	m_muxContinue2.Lock();
	m_muxContinue2.UnLock();

	return 0;
}

int CThreadObj::Start(TCallbackFuncObj<pThreadCallbackFunc>* pCObj)
{
	if(NULL == pCObj && NULL == m_pCallbackFuncObj)
		return ERR_JTHREAD_THREADFUNCNOTSET;
	
	if(NULL != pCObj)
		m_pCallbackFuncObj = pCObj;

	return Start();
}

void CThreadObj::Join()
{
	if(!IsRunning())	
		return;
#ifdef WIN32
	WaitForSingleObject(m_ThreadID, INFINITE);
#else
	pthread_join(m_ThreadID, NULL);	
#endif //WIN32
	
}

int CThreadObj::Kill()
{
	m_muxRunning.Lock();
	if(!m_bIsRunning)
	{
		m_muxRunning.UnLock();
		return ERR_JTHREAD_NOT;
	}
#ifdef WIN32
	TerminateThread(m_ThreadID, 0);
	CloseHandle(m_ThreadID);
#else
	pthread_cancel(m_ThreadID);
#endif //WIN32
	
	m_ThreadID = 0;
	m_bIsRunning = false;
	m_muxRunning.UnLock();

	return 0;
}

bool CThreadObj::IsRunning()
{
	bool bRtn = false;
	m_muxRunning.Lock();
	bRtn = m_bIsRunning;
	m_muxRunning.UnLock();

	return bRtn;
}

void CThreadObj::ThreadStarted()
{
	m_muxContinue2.UnLock();
}

void* CThreadObj::ThreadImp()
{
	return m_pCallbackFuncObj->GetCallbackFunc()(m_pCallbackFuncObj->GetCallbackParam());
}
	
#ifdef WIN32
DWORD CThreadObj::ThreadEntry( LPVOID param )
#else
void* CThreadObj::ThreadEntry(void* param)
#endif
{
	CThreadObj* pThis = NULL;

	void* pRet = NULL;
	pThis = (CThreadObj*)(param);

	pThis->m_muxContinue2.Lock();
	pThis->m_muxRunning.Lock();
	pThis->m_bIsRunning = true;
	pThis->m_muxRunning.UnLock();

	pThis->m_muxContinue.Lock();
	pThis->m_muxContinue.UnLock();

	pRet = pThis->ThreadImp();
		
	pThis->m_muxRunning.Lock();
	pThis->m_bIsRunning = false;
	pThis->m_ThreadID = 0;
	pThis->m_muxRunning.UnLock();

	return 0;
}






	

