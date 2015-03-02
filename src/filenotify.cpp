
#include "filenotify.h"

CFileNotify::CFileNotify()
{
	m_totalFileNum = 0;
	m_notifyPath = "";
	m_intervalTime = 0;
	pthread_mutex_init(&m_getFileLock,NULL);
	pthread_mutex_init(&m_deleteFileLock, NULL);
	pthread_cond_init(&m_getFileCond,NULL);
}

CFileNotify::~CFileNotify()
{
	m_totalFileNum = 0;
	m_notifyPath = "";
	m_intervalTime = 0;
	pthread_mutex_destroy(&m_getFileLock);
	pthread_mutex_destroy(&m_deleteFileLock);
	pthread_cond_destroy(&m_getFileCond);
}

int CFileNotify::Init(const char* path, unsigned int time)
{
	m_intervalTime = time;

	m_notifyPath = path;
	int ret = CFileNotify::FileWatch((void*)this);
	if(ret == -1)
	{
		return INOTIFY_FAILED;
	}

	pthread_t pthread;
	pthread_create(&pthread, NULL, Routin, this);

	return INOTIFY_SUCCESS;
}

void* CFileNotify::Routin(void *param)
{
	pthread_detach(pthread_self());

	CFileNotify* cfileNotify = static_cast<CFileNotify*>(param);

	while(1)
	{
		time_t tmBefore;
		time(&tmBefore);
		cfileNotify->TraverseFile((char*)cfileNotify->m_notifyPath.c_str());
//		log_info(g_pLogHelper, "(CFileNotify::Routin) upload all file used time:%d, inotify total file num:%d", (time(NULL) - tmBefore), cfileNotify->m_totalFileNum);
	
		//TODO:not need Re-traversal,because add m_deleteFileList
		sleep(cfileNotify->m_intervalTime);
//		log_info(g_pLogHelper, "(CFileNotify::Routin) %d second pass away, Traverse File again", cfileNotify->m_intervalTime);
		cfileNotify->m_fileList.clear();
		cfileNotify->m_totalFileNum = 0;
	}

	pthread_exit((void*)0);
}

string CFileNotify::GetStatus()
{
	string retData = "";
	char tempBuf[1024] = {0};

	pthread_mutex_lock(&m_getFileLock);	
	sprintf(tempBuf, "(CFileNotify::GetStatues) inotifyListNum:%d", m_fileList.size());	
	pthread_mutex_unlock(&m_getFileLock);

	retData = tempBuf;
	return retData;
}

int CFileNotify::TraverseFile(char* dir)
{
	DIR      *dir_fd;
    struct   dirent *s_dir;
    struct   stat file_stat;
    char     currfile[1024]={0};
    int      len = strlen(dir);
    
	if(dir[len-1] != '/')
    {
        dir[len] = '/';
        dir[len+1] = 0;
    }
    
	if((dir_fd = opendir(dir)) == NULL)
    {
         log_err(g_pLogHelper, "(CFileNotify::TraverseFile) opendir(%s) failed (%s).", dir, strerror(errno));
         return -1;
    }
	
	while((s_dir=readdir(dir_fd))!=NULL)
    {
        if((strcmp(s_dir->d_name,".")==0)||(strcmp(s_dir->d_name,"..")==0))
            continue;
		
        sprintf(currfile, "%s%s", dir, s_dir->d_name);
        stat(currfile,&file_stat);
        if(S_ISDIR(file_stat.st_mode))
        {
            TraverseFile(currfile);
    //        log_info(g_pLogHelper, "(CFileNotify::TraverseFile) the dir is %s", currfile);
        }
        else
        {
			pthread_mutex_lock(&m_getFileLock);
            while(m_fileList.size() > 1000)
			{
		//		pthread_mutex_unlock(&m_getFileLock);
			//	log_info(g_pLogHelper, "(FN::TraverseFile) list number more than 1000, wait getfile...");
				pthread_cond_wait(&m_getFileCond, &m_getFileLock);	
		//		pthread_mutex_lock(&m_getFileLock);
			}
			
			string temp_dir = dir;
			string insert_file = temp_dir.substr(m_notifyPath.length(), temp_dir.length());
			insert_file += s_dir->d_name;
			CCommonStruct::ReparePath(insert_file);	

			T_FILENODE fileNode;
			fileNode.fileName = insert_file;	
			fileNode.fileLen = file_stat.st_size;

			CCommonStruct::GetGUID((char*)fileNode.fileName.c_str(), fileNode.fileName.length(), &fileNode.tGuid);
			//char ucGetStr[33] = {0};
			string ucGetStr = "";
			ucGetStr = CCommonStruct::GUIDToString(fileNode.tGuid);
	//		log_info(g_pLogHelper, "(CFileNotify::TraverseFile) TraverseFile, FileName: %s, FileLen:%d, FileGuid:%s\n", fileNode.fileName.c_str(), fileNode.fileLen, ucGetStr.c_str());
				
			m_fileList.push_back(fileNode);
			m_totalFileNum ++;//total file num 
			
			pthread_mutex_unlock(&m_getFileLock);
        }
    }
 
    closedir(dir_fd);
    return 0;	
}

int CFileNotify::GetFile(T_FILENODE &fileNode)
{
	pthread_mutex_lock(&m_getFileLock);
	if(m_fileList.empty())
	{
		//log_info(g_pLogHelper, "(CFileNotify::GetFile) inotify list is null");
		pthread_mutex_unlock(&m_getFileLock);
		return FILE_LIST_EMPTY;
	}

	fileNode = m_fileList.front();
	m_fileList.pop_front();
	pthread_cond_signal(&m_getFileCond);
	pthread_mutex_unlock(&m_getFileLock);
	//pthread_cond_signal(&m_getFileCond);
//	char cGetStr[64] = {0};
	string cGetStr = "";
    cGetStr = CCommonStruct::GUIDToString(fileNode.tGuid);	
//	log_info(g_pLogHelper, "(CFileNotify::GetFile) GetFile: %s, FileSize:%d, FileGuid:%s\n", fileNode.fileName.c_str(), fileNode.fileLen, cGetStr.c_str());
	return GET_FILE_SUCCESS;
}

int CFileNotify::GetDeleteFile(T_FILENODE &fileNode)
{
	pthread_mutex_lock(&m_deleteFileLock);
	if(m_deleteFileList.empty())
	{
		pthread_mutex_unlock(&m_deleteFileLock);
		return FILE_LIST_EMPTY;
	}

	fileNode = m_deleteFileList.front();
	m_deleteFileList.pop_front();
	pthread_mutex_unlock(&m_deleteFileLock);
	string cGetStr = "";
	cGetStr = CCommonStruct::GUIDToString(fileNode.tGuid);
	return GET_FILE_SUCCESS;
}

int CFileNotify::FileWatch(void *param)
{
	CFileNotify *pThis = static_cast<CFileNotify*>(param);

	static FileSystemWatcher fsw;
    log_info(g_pLogHelper, "(FN::FileWatch) the monitor path is %s",pThis->m_notifyPath.c_str());
    bool r = fsw.Run((pThis->m_notifyPath.c_str()), CFileNotify::FileChangedCallBack, (void*)pThis);
    if(!r)
    {
       // int sync_error = errno;
        log_err(g_pLogHelper, "(FN::FileWatch) the system watch run filled....(%s)",strerror(errno));
        return -1;
    }
 //   log_info(g_pLogHelper, "(FN::FileWatch) the system watch run ok");
    return 0;

}
/*
void CFileNotify::FileChangedCallBackEntry(uint32_t act, const char* fileName, void* param)
{
	CFileNotify* pThis = static_cast<CFileNotify*>(param);
	if(NULL != pThis)
		pThis->FileChangedCallBack(act,fileName, param);
}
*/
void CFileNotify::FileChangedCallBack(uint32_t act, const char* fileName, void* param)
{
	CFileNotify* pThis = static_cast<CFileNotify*>(param); 
	T_FILENODE fileNode; 

	//获取文件名称
	int len = strlen(fileName);
	string temp_file = fileName;
	CCommonStruct::ReplaceAll(temp_file, "//", "/");
	fileNode.fileName = temp_file.substr(pThis->m_notifyPath.length(), len - pThis->m_notifyPath.length());
 
	//获取文件长度
	struct stat fileStat;
	stat(fileName, &fileStat);
	fileNode.fileLen = fileStat.st_size;

	CCommonStruct::GetGUID((char*)fileNode.fileName.c_str(), fileNode.fileName.length(), &fileNode.tGuid);
//	char cGetStr[33] = {0};
	string cGetStr = "";
    cGetStr = CCommonStruct::GUIDToString(fileNode.tGuid);

	switch(act)
    {
         case IN_CREATE:
		 case IN_MOVED_TO:
             //log_info(g_pLogHelper, "(CFileNotify::FileChangedCallBack) FILE_ADD ,add fileName:%s, fileSize:%d, fileGuid:%s, ",fileNode.fileName.c_str(), fileNode.fileLen, cGetStr.c_str());
			 pthread_mutex_lock(&pThis->m_getFileLock);
			 pThis->m_fileList.push_back(fileNode); 
			 pThis->m_totalFileNum ++;
			 pthread_mutex_unlock(&pThis->m_getFileLock);			 
			 break;

		 case IN_DELETE:
		 case IN_MOVED_FROM:
             log_info(g_pLogHelper, "(CFileNotify::FileChangedCallBack) FILE_DELETE ,delete fileName:%s, fileSize:%d, fileGuid:%s, ",fileNode.fileName.c_str(), fileNode.fileLen, cGetStr.c_str());
			 pthread_mutex_lock(&pThis->m_deleteFileLock);
			 pThis->m_deleteFileList.push_back(fileNode);
			 pthread_mutex_unlock(&pThis->m_deleteFileLock);
			 break; 
         default:
    //         log_info(g_pLogHelper, "(CFileNotify::FileChangedCallBack) %s have other change ,ignore %x", fileNode.fileName.c_str(),act);
             break;
     }
}
