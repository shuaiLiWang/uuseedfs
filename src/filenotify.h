
#ifndef __FILE_NOTIFY_H__
#define __FILE_NOTIFY_H__

#include "commonstruct.h"
#include "Log.h"
#include "FileSystemWatcher.h"

typedef	list<T_FILENODE>			LIST_T_FILENODE;
typedef	LIST_T_FILENODE::iterator	ITR_LIST_T_FILENODE;	

class CFileNotify
{
	public: 
		CFileNotify();
		virtual ~CFileNotify();

		enum _efilenotifyresult{
			INOTIFY_FAILED = -1,
			INOTIFY_SUCCESS = 0
		};

		enum _egetfileresult{
			FILE_LIST_EMPTY = -1,
			GET_FILE_SUCCESS = 0
		};

	public: 
		int Init(const char* path, unsigned int time);
		int GetDeleteFile(T_FILENODE &fileNode);
		int GetFile(T_FILENODE &fileNode);
		string GetStatus();		
		
	private:
		static void *Routin(void* param);//遍历读取文件线程
		int TraverseFile(char* dir);
		static int FileWatch(void *param);
	    static void FileChangedCallBack(uint32_t act, const char* fileName, void* param);
	
	public:
		unsigned int				m_totalFileNum;
		unsigned int				m_intervalTime;
		string						m_notifyPath;

		pthread_cond_t				m_getFileCond;
		pthread_mutex_t				m_getFileLock;
		LIST_T_FILENODE				m_fileList;	
		
		pthread_mutex_t				m_deleteFileLock;
		LIST_T_FILENODE				m_deleteFileList;
		//	FileSystemWatcher	m_fsw;
};

//int FileWatch(void* param);
//void FileChangedCallBack(uint32_t act, const char* fileName, void* param);
#endif //__FILE_NOTIFY_H__
