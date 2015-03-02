
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <string>
#include "commonstruct.h"
#include "md5.h"
#include "Log.h"
#include "dfsmanager.h"

#include "log_agent_const.h"
#include "log_agent.h"


#define VERSION_STRING  ""

LOG_AGENT g_tLogStatus;

void SIGHandler(int signo);
void Help();

int main(int argc, char *argv[])
{
	int ret = 0;
	//test arguments
	switch (argc)
	{
	case 1:
		Help();
		return 0;
		break;
	case 2:
		{
			if( !strcmp(argv[1], "-h") || !strcmp(argv[1],"-H") )
			{
				Help();
				return 0;
			}
		}
		break;
	default:
		Help();
		return 0;
		break;
	}

	//set some signals's handle
//	signal(SIGINT, SIGHandler);
	signal(SIGTERM, SIGHandler);
	signal(SIGPIPE, SIG_IGN);

	//read configure information from configu file
	if(!CCommonStruct::ReadConfig(argv[1]))
		return 0;

	//create log 
	string strLogFile = g_confFile.strLogDir;
	strLogFile += "uusee_dfs.log";
	g_pLogHelper = open_logfile(strLogFile.c_str());

	log_info(g_pLogHelper, "main: uusee_dfs start......");


//+++++++++++++++++++++++++++++++++

	//printf("/data0/wengy/test/\n");

	char szLogStatusFullName[1024]="";
	snprintf(szLogStatusFullName, 1024, "%sstatus/", g_confFile.strLogDir.c_str());

	log_agent_setup(&g_tLogStatus);
	log_agent_init(&g_tLogStatus);
	log_agent_set_log_level(&g_tLogStatus, LA_LOG_LEVEL_DEBUG | LA_LOG_LEVEL_ERROR | LA_LOG_LEVEL_INFO);
	log_agent_set_log_type(&g_tLogStatus, LA_LOG_TYPE_FILE);
	log_agent_set_log_file_dir(&g_tLogStatus,  szLogStatusFullName, strlen(szLogStatusFullName));

//	log_agent_trace(&g_tLogStatus, LA_LOG_LEVEL_INFO, __FILE__, __LINE__, "This is a log agent test!\n");
//	log_agent_debug(&g_tLogStatus, __FILE__, __LINE__, "This is a log agent test 2!\n");
	log_agent_info(&g_tLogStatus, __FILE__, __LINE__, "start log status............!\n");



//+++++++++++++++++++++++++++++++++


	CDfsManager dfsManager;
	dfsManager.Init();
	dfsManager.Start();

	dfsManager.Join();
	
	while(1)
	{
		sleep(10);
	}
	
	log_agent_cleanup(&g_tLogStatus);
	return ret;
}

void SIGHandler(int signo)
{
	/*//if((SIGINT == signo) || (SIGTERM == signo))
	//{
		if(theServer != NULL)
		{
			theServer->End();
		}
	//}
	if (SIGPIPE == signo)
	{
		stx_info("receive sigpipe signal");
	}
	*/
}

void Help()
{
	printf("uusee_dfs. Version %s\r\n", VERSION_STRING);
	printf("-h can be used to show this information\r\n");
	printf("example: ./uusee_dfs xxx.conf\r\n");
	printf("report bugs to <wanglj@uusee.com>\r\n");
	printf("Last Modify time is 2013-12-18 12:45.\r\n");
}

