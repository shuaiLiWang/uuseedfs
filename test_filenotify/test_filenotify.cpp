#include <stdio.h>
#include "../src/filenotify.h"

void Help()
{
	printf("test_filenotify. Version %d\r\n", 1.0);
	printf("-h can be used to show this information\r\n");
	printf("example: ./test_filenotify /home/\r\n");
	printf("report bugs to <wangls@uusee.com>\r\n");
	printf("Last Modify time is 2012-09-11 12:45.\r\n");
}

int main(int argc, char *argv[])
{
	string srzWatchDir("");
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
				else
				{
					srzWatchDir = argv[1];
				}
			}   
			break;
		default:
			Help();
			return 0;
			break;
	}  

	CFileNotify m_FileNotify;
	if(0 != m_FileNotify.Init(srzWatchDir.c_str(), 100000000))
	{
		printf("DM::FileWatcherImp init file notify object failed. dir:%s" , srzWatchDir.c_str());
		return 0;
	}

	while(1)
	{
		sleep(10);
	}
	return 1;

}
