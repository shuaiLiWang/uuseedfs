
#ifndef  __TRHEAD_OBJECT__
#define  __TRHEAD_OBJECT__

#ifdef WIN32
#include <Windows.h>
//#include <WinBase.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif// WIN32


#include <stdio.h>

#include "MutexObj.h"
#include "CallbackFuncObj.h"

extern  int 	ERR_JTHREAD_CANINITMUTEX;
extern  int 	ERR_JTHREAD_CANTSTARTTHREAD;
extern  int  	ERR_JTHREAD_THREADFUNCNOTSET;
extern  int 	ERR_JTHREAD_NOTRUNNING;
extern  int  	ERR_JTHREAD_ALREADYRUNNING;

typedef void* (*pThreadCallbackFunc) (void* pParam);
 
class CThreadObj
{
public:
	CThreadObj();
	CThreadObj(TCallbackFuncObj<pThreadCallbackFunc>* pCObj);
	~CThreadObj();

	int Start();
	int Start(TCallbackFuncObj<pThreadCallbackFunc>* pCObj);
	int Kill();
	void Join();
	bool IsRunning();	

	void ThreadStarted();

private:
#ifdef WIN32
	static DWORD WINAPI ThreadEntry( LPVOID param );
#else
	static	void* ThreadEntry(void* param);
#endif //WIN32
	
	void* ThreadImp();
	
private:
#ifdef WIN32
	HANDLE		m_ThreadID;
#else
	pthread_t	m_ThreadID;
#endif
	
	bool 		m_bIsRunning;
	CMutexObj 	m_muxRunning;
	CMutexObj 	m_muxContinue;
	CMutexObj 	m_muxContinue2;

	TCallbackFuncObj<pThreadCallbackFunc>* 	m_pCallbackFuncObj;
};

#endif //__TRHEAD_OBJECT__

