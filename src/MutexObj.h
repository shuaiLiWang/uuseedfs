// $_FILEHEADER_BEGIN ****************************
// 
// Copyright (C) Badu Corporation.  All Rights Reserved
// �ļ����ƣ�CriticalSection.h
// �������ڣ�20050404
// �����ˣ� 
// �ļ�˵�����������

// 0.01 �ź��� 20050403
// �����ļ�
// $_FILEHEADER_END ******************************
#ifndef __MUTEX_OBJ_H__
#define __MUTEX_OBJ_H__

#ifdef WIN32
//#include <WinBase.h>
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif// WIN32


class CMutexObj
{
public:
	CMutexObj();
	~CMutexObj();

	void  Lock();
	void UnLock();
	int TryLock();

private:
	//windows OS
#ifdef WIN32
	CRITICAL_SECTION	m_oSection;
#else
	pthread_mutex_t m_hMutex;
#endif

};


class CAutoMutexLock
{
public:
	CAutoMutexLock(CMutexObj& aCriticalSection);
	~CAutoMutexLock();
private:
	CMutexObj& m_oCriticalSection;
};

#endif // __MUTEX_OBJ_H__
