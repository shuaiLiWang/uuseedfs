#ifndef __FILESYSTEMWATCHER_HPP__
#define __FILESYSTEMWATCHER_HPP__

//#include "FileHelper.h"
//
#ifdef WIN32
#include <windows.h>
#else
#include <sys/inotify.h>
#endif

#include "commonstruct.h"
#include "Log.h"
#define INOTIFY_EVENT_SIZE (sizeof(struct inotify_event))
#define INOTIFY_EVENT_BUF_LEN (1024 * (INOTIFY_EVENT_SIZE + 16))

class FileSystemWatcher
{
public:
	FileSystemWatcher();
	~FileSystemWatcher();

private: // no-impl
    FileSystemWatcher( const FileSystemWatcher& );
    FileSystemWatcher operator=( const FileSystemWatcher );

#ifdef WIN32
public:
	enum Filter
	{
		FILTER_FILE_NAME        = 0x00000001, // add/remove/rename
		FILTER_DIR_NAME         = 0x00000002, // add/remove/rename
		FILTER_ATTR_NAME        = 0x00000004,
		FILTER_SIZE_NAME        = 0x00000008,
		FILTER_LAST_WRITE_NAME  = 0x00000010, // timestamp
		FILTER_LAST_ACCESS_NAME = 0x00000020, // timestamp
		FILTER_CREATION_NAME    = 0x00000040, // timestamp
		FILTER_SECURITY_NAME    = 0x00000100
	};
	enum ACTION
	{
		ACTION_ERRSTOP          = -1,
		ACTION_ADDED            = 0x00000001,
		ACTION_REMOVED          = 0x00000002,
		ACTION_MODIFIED         = 0x00000003,
		ACTION_RENAMED_OLD      = 0x00000004,
		ACTION_RENAMED_NEW      = 0x00000005
	};
	typedef void (__stdcall *LPDEALFUNCTION)( ACTION act, LPCSTR filename, LPVOID lParam );

	// LPCTSTR dir: ended-with "\\"
	bool Run( LPCTSTR dir, bool bWatchSubtree, DWORD dwNotifyFilter, LPDEALFUNCTION dealfun, LPVOID lParam );
	void Close( DWORD dwMilliseconds=INFINITE );

//protected:
//	void CallFileHelper(LPCTSTR dir);

//private: // no-impl
	//FileSystemWatcher( const FileSystemWatcher& );
	//FileSystemWatcher operator=( const FileSystemWatcher );

private:

	 HANDLE  			m_hDir;
	 HANDLE  			m_hThread;
	 DWORD   			m_dwNotifyFilter;
	 bool    			m_bWatchSubtree;
	 volatile bool   	m_bRequestStop;
	 LPDEALFUNCTION  	m_DealFun;
	 LPVOID          	m_DealFunParam;

	 static DWORD WINAPI Routine( LPVOID lParam );
#else
public:
	enum{NEWDIR = IN_CREATE | IN_ISDIR};
	enum{MASK = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE};

	enum {EVENT_SIZE = sizeof(struct inotify_event)};
	enum {BUF_SIZE = (EVENT_SIZE + 16) << 10};

public:
	 typedef void (*LPDEALFUNCTION)( uint32_t act, const char* filename, void* lParam );

	 bool WatchInit();
	 static void AddWatch(int fd, char* dir, int mask, void* param, int flag);
	 bool Run(string dir, LPDEALFUNCTION dealfun, void* param);
	 void Close(); 
private:

     static int     	m_iFD;
     static int     	m_iWD;
	 pthread_t 			m_hThread;
	 LPDEALFUNCTION   	m_DealFun;
     void*   			m_DealFunParam;
	 volatile bool   	m_bRequestStop;
	 char 				m_cSubDir[512];
	 map<int, string> 	g_dirset;
	// UUFile      m_uufile;

	 static void *Routine( void *param); 
	 static void do_action(int fd, struct inotify_event *event, void* param);
     static int filter_action(uint32_t mask);
#endif //WIN32
};

#endif // __FILESYSTEMWATCHER_HPP__

