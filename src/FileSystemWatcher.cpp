
#include <cassert>
#include <string>
#include "FileSystemWatcher.h"

//#define WriteLog() MangerLog::Instance()

#ifndef WIN32
int FileSystemWatcher::m_iFD = -1;
int FileSystemWatcher::m_iWD = -1;
//map<int, string>   FileSystemWatcher::dirset;
#endif

FileSystemWatcher::FileSystemWatcher() : 
#ifdef WIN32
	m_hDir(INVALID_HANDLE_VALUE)
,m_hThread(NULL)
#else
m_hThread(-1)
#endif
{
#ifdef WIN32
	assert( FILTER_FILE_NAME        == FILE_NOTIFY_CHANGE_FILE_NAME   );
	assert( FILTER_DIR_NAME         == FILE_NOTIFY_CHANGE_DIR_NAME    );
	assert( FILTER_ATTR_NAME        == FILE_NOTIFY_CHANGE_ATTRIBUTES  );
	assert( FILTER_SIZE_NAME        == FILE_NOTIFY_CHANGE_SIZE        );
	assert( FILTER_LAST_WRITE_NAME  == FILE_NOTIFY_CHANGE_LAST_WRITE  );
	assert( FILTER_LAST_ACCESS_NAME == FILE_NOTIFY_CHANGE_LAST_ACCESS );
	assert( FILTER_CREATION_NAME    == FILE_NOTIFY_CHANGE_CREATION    );
	assert( FILTER_SECURITY_NAME    == FILE_NOTIFY_CHANGE_SECURITY    );

	assert( ACTION_ADDED            == FILE_ACTION_ADDED              );
	assert( ACTION_REMOVED          == FILE_ACTION_REMOVED            );
	assert( ACTION_MODIFIED         == FILE_ACTION_MODIFIED           );
	assert( ACTION_RENAMED_OLD      == FILE_ACTION_RENAMED_OLD_NAME   );
	assert( ACTION_RENAMED_NEW      == FILE_ACTION_RENAMED_NEW_NAME   );
#endif
}

FileSystemWatcher::~FileSystemWatcher()
{
	Close();
}

#ifndef WIN32
bool FileSystemWatcher::WatchInit()
{
	m_iFD = inotify_init();
	if(0 > m_iFD)
	{
//		log_info(g_pLogHelper, "FileSystemWatcher: inotify_init failed.");
		log_err(g_pLogHelper,"(FSW::WatchInit) init fail.(%s)", strerror(errno));
		return false;
	}

//	log_info(g_pLogHelper,"inotify_init success....");
	//log_info(g_pLogHelper, "(FSW::WatchInit) init ok....");
	return true;
}
#endif

#ifndef WIN32
void FileSystemWatcher::AddWatch(int fd, char* dir, int mask, void *param, int flag)
{
	FileSystemWatcher& obj = *(FileSystemWatcher*)param;	

	DIR *odir;
	struct dirent *dent;

	if ((odir = opendir(dir)) == NULL)
	{
		//                 perror("fail to open root dir");
		log_err(g_pLogHelper, "(FSW::AddWatch) opendir %s fail(%s)", dir, strerror(errno));
//		log_info(g_pLogHelper,"opendir %s filed, the error is %s", dir, strerror(errno));
//		exit(1);
		return;
	}

	obj.m_iWD = inotify_add_watch(fd, dir, mask);
	//m_iWD = inotify_add_watch(m_iFD, m_szDir.c_str(), IN_ALL_EVENTS);
	if(0 > obj.m_iWD)
	{
//		log_info(g_pLogHelper, "FileSystemWatcher: inotify_add_watch failed. dirName:%s", dir);
		log_err(g_pLogHelper, "(FSW::AddWatch) inotify_add_watch failed. dirName:%s (%s)",dir, strerror(errno));
		close(obj.m_iFD);
		m_iFD = -1;
		return;
		//  SendErrorToMonitor(20003, 3);
	}
//	log_info(g_pLogHelper, "FileSystemWatcher: inotify_add_watch success. dirName:%s", dir);
	//log_info(g_pLogHelper, "(FSW::AddWatch) inotify_add_watch ok. dirName:%s",dir);
	obj.g_dirset.insert(make_pair(obj.m_iWD, string(dir)));

	errno = 0;
	while ((dent = readdir(odir)) != NULL)
	{
		if (strcmp(dent->d_name, ".") == 0
				|| strcmp(dent->d_name, "..") == 0)
			continue;
		if (dent->d_type == DT_DIR || dent->d_type == DT_LNK)
		{
			memset(obj.m_cSubDir, 0, 512);
			sprintf(obj.m_cSubDir, "%s/%s", dir, dent->d_name);
			//		 printf("the subdir is... %s\n", m_cSubDir);
			string tempSubDir = obj.m_cSubDir;
			CCommonStruct::ReparePath(tempSubDir);
			CCommonStruct::ReplaceAll(tempSubDir,"//", "/");
			//log_info(g_pLogHelper, "the watch dir is %s..................", tempSubDir.c_str());
		//	WriteLog()->message(__FILE__,__LINE__,5,errno, "the watch dir is %s..................",tempSubDir.c_str());
			// printf("the subdir is %s\n", tempSubDir.c_str());
			AddWatch(fd, (char*)tempSubDir.c_str(), mask, param, flag); //flag 1:old dir 2:new dir(monitor)
		}
		else if(2 == flag)
        {
             char file[1024] = {0};
             sprintf(file, "%s/%s", dir, dent->d_name);
             obj.m_DealFun(IN_CREATE, file, obj.m_DealFunParam);
        }
	}

	closedir (odir);
}
#endif

#ifdef WIN32
bool FileSystemWatcher::Run( LPCTSTR dir, bool bWatchSubtree, DWORD dwNotifyFilter, LPDEALFUNCTION dealfun, LPVOID lParam )
#else
bool FileSystemWatcher::Run(string dir, LPDEALFUNCTION dealfun, void* param)
#endif
{
	Close();
#ifdef WIN32
	m_hDir = CreateFile(
			dir,
			GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL
			);
	if( INVALID_HANDLE_VALUE == m_hDir ) return false;

	m_bWatchSubtree = bWatchSubtree;
	m_dwNotifyFilter = dwNotifyFilter;
	m_DealFun = dealfun;
	m_DealFunParam = lParam;
	m_bRequestStop = false;

	DWORD ThreadId;
	m_hThread = CreateThread( NULL,0,Routine,this,0,&ThreadId );
	if( NULL == m_hThread )
	{
		CloseHandle( m_hDir );
		m_hDir = INVALID_HANDLE_VALUE;
	}

	return NULL!=m_hThread;
#else	
	m_DealFun = dealfun;	
	m_DealFunParam = param;	
	m_bRequestStop = false;
	WatchInit();
	CCommonStruct::ReparePath(dir);
	AddWatch(m_iFD, (char*)dir.c_str(), MASK, this, 1);

	//log_info(g_pLogHelper, "(FSW::Run) add watch over......................");
	pthread_create(&m_hThread, NULL, Routine, this);

	sleep(2);
	return true;
#endif
}

#ifdef WIN32
void FileSystemWatcher::Close( DWORD dwMilliseconds )
#else
void FileSystemWatcher::Close()
#endif
{
#ifdef WIN32
	if( NULL != m_hThread )
	{
		m_bRequestStop = true;
		if( WAIT_TIMEOUT == WaitForSingleObject(m_hThread,dwMilliseconds) )
			TerminateThread( m_hThread, 0 );
		CloseHandle( m_hThread );
		m_hThread = NULL;
	}
	if( INVALID_HANDLE_VALUE != m_hDir )
	{
		CloseHandle( m_hDir );
		m_hDir = INVALID_HANDLE_VALUE;
	}
#else
	m_bRequestStop = true;
	inotify_rm_watch(m_iFD, m_iWD);
	m_iWD = -1;
	close(m_iFD);
	m_iFD = -1;	
#endif
}
/*
   void FileSystemWatcher::CallFileHelper(LPCTSTR dir)
   {
//CFileHelper::Instance().FindFiles((TCHAR*)dir);
}*/

void FileSystemWatcher::do_action(int fd, struct inotify_event *event, void* param)
{
	FileSystemWatcher& obj = *(FileSystemWatcher*)param;	

	char file[512] = {0};
	int ia, i;
	if ((ia = filter_action(event->mask)) < 0)
		return;
	if ((event->mask & NEWDIR) == NEWDIR)
	{
		char ndir[512] = {0};
		int wd;
		sprintf(ndir, "%s/%s", obj.g_dirset.find(event->wd)->second.c_str(),event->name);
		string addMonitorDir = ndir;
		CCommonStruct::ReplaceAll(addMonitorDir, "//", "/");
//		log_info(g_pLogHelper,"(FSW::do_action) CREATE MONITOR DIR IS %s", addMonitorDir.c_str());
		//wd = inotify_add_watch(fd, (char*)addMonitorDir.c_str(), MASK);
		//obj.g_dirset.insert(make_pair(wd, addMonitorDir));
		
		AddWatch(fd, (char*)addMonitorDir.c_str(), MASK, param, 2);
	}


	sprintf(file, "%s/%s", obj.g_dirset.find(event->wd)->second.c_str(), event->name);
//	string sFile = file;
//	m_uufile.ReplaceAll(sFile, "//", "/");
//	log_err(g_pLogHelper, "THE MASK IS %d, THE NAME IS %s,the FILE IS %s", event->mask, event->name,file);
	obj.m_DealFun(event->mask, file, obj.m_DealFunParam);
//	log_err(g_pLogHelper, "THE MASK IS %d, THE NAME IS %s", event->mask, event->name);
	//send_mess(event->name, action[ia], event->wd);
}
/*
void FileSystemWatcher::append_dir(int fd, struct inotify_event *event, int mask)
{

	char ndir[512];
	int wd;
	sprintf(ndir, "%s/%s", dirset.find(event->wd)->second.c_str(),event->name);
	wd = inotify_add_watch(fd, ndir, mask);
	dirset.insert(make_pair(wd, string(ndir)));
}
*/
int FileSystemWatcher::filter_action(uint32_t mask)
{
	if (mask & IN_MODIFY)
		return 0;
	if (mask & IN_ACCESS)
		return 1;
	if (mask & IN_CREATE)
		return 2;
	if (mask & IN_DELETE)
		return 3;
	if (mask & IN_MOVE)
		return 4;
	return -1;
}

#ifdef WIN32
DWORD WINAPI FileSystemWatcher::Routine( LPVOID lParam )
#else
void *FileSystemWatcher::Routine(void *param)
#endif
{
	//	::SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)UnHandleException );
#ifdef WIN32
	FileSystemWatcher& obj = *(FileSystemWatcher*)lParam;

	BYTE buf[ 2*(sizeof(FILE_NOTIFY_INFORMATION)+2*MAX_PATH)+2 ];
	int n= sizeof(buf);
	FILE_NOTIFY_INFORMATION* pNotify=(FILE_NOTIFY_INFORMATION *)buf;
	DWORD BytesReturned;
	while( !obj.m_bRequestStop )
	{
		if( ReadDirectoryChangesW( obj.m_hDir,
					pNotify,
					sizeof(buf)-2,
					obj.m_bWatchSubtree,
					obj.m_dwNotifyFilter,
					&BytesReturned,
					NULL,
					NULL ) ) // 无限等待，应当使用异步方式
		{
			for( FILE_NOTIFY_INFORMATION* p=pNotify; p; )
			{
				TCHAR c = p->FileName[p->FileNameLength/2];
				p->FileName[p->FileNameLength/2] = L'\0';

				char szFileName[MAX_PATH] = {0};
				if (0 !=WideCharToMultiByte(CP_ACP, 0, p->FileName, p->FileNameLength/2, szFileName, MAX_PATH, NULL, NULL))
					obj.m_DealFun( (ACTION)p->Action, szFileName, obj.m_DealFunParam );

				p->FileName[p->FileNameLength/2] = c;

				if( p->NextEntryOffset )
					p  = (PFILE_NOTIFY_INFORMATION)( (BYTE*)p + p->NextEntryOffset );
				else
					p = 0;
			}
		}
		else
		{
			obj.m_DealFun( (ACTION)ACTION_ERRSTOP, 0, obj.m_DealFunParam );
			break;
		}
	}

	return 0;
#else	
	pthread_detach(pthread_self());

	//FileSystemWatcher& obj = *(FileSystemWatcher*)param;

	//log_info(g_pLogHelper, "(FSW::Routine) inotify thread start");
	int i, length;
	char buf[INOTIFY_EVENT_BUF_LEN]={""};
	struct inotify_event *event;
	while ((length = read(m_iFD, buf, INOTIFY_EVENT_BUF_LEN)) >= 0)
	{
		i = 0;
		while (i < length)
		{
			event = (struct inotify_event*)(buf + i);
			if (event->len)
				do_action(m_iFD, event, param);
			i += EVENT_SIZE + event->len;
		}
	}

	//log_info(g_pLogHelper, "(FSW::Routine) inotify thread end (%s)", strerror(errno));
	
	close(m_iFD);
	pthread_exit((void*)0);
#endif
}
