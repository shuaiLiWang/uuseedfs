


#ifdef WIN32

#include <stdlib.h>
#include <windows.h>

#else

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#endif

#include <public/gen_error.h>
#include "gen_dir.h"

/** max dir size */
#define GEN_DIR_MAX_PATH_SIZE                              (1024)


/////////////////////////////// Outter Interface //////////////////////////////
int32 gen_dir_createA(int8* p_path, int32 path_size)
{
	int8   str_path[GEN_DIR_MAX_PATH_SIZE + 4];
	//int32  str_size;

	int32  i;
	int32  ret;

	if( p_path == NULL )
	{
		return GEN_E_INVALID_PARAM;
	}

	if( path_size < 1 || path_size > GEN_DIR_MAX_PATH_SIZE )
	{
		return GEN_E_INVALID_PARAM;
	}

#if RUN_OS_WINDOWS

	for( i = 0; i < path_size; i++ )
	{
		if( p_path[i] == '\\' )
		{
			str_path[i] = '\0';

			ret = CreateDirectoryA(str_path, NULL);
		}

		str_path[i] = p_path[i];
	}

	if( str_path[path_size - 1] != '\\' )
	{
		str_path[path_size] = '\0';

		ret = CreateDirectoryA(str_path, NULL);
	}

#else

	str_path[0] = p_path[0];

	for( i = 1; i < path_size; i++ )
	{
		if( p_path[i] == '/' )
		{
			str_path[i] = '\0';

			ret = mkdir(str_path, 0777);
			//printf("i = %d, path = %s, ret = %d\n", i, str_path, ret);
		}

		str_path[i] = p_path[i];
	}

	if( str_path[path_size - 1] != '/' )
	{
		str_path[path_size] = '\0';

		ret = mkdir(str_path, 0777);
	}

#endif

	return GEN_S_OK;
}

#if RUN_OS_WINDOWS
int32 gen_dir_createW(wchar_t* p_path, int32 path_size)
{
	uint16  str_path[GEN_DIR_MAX_PATH_SIZE + 4];
	int32  char_path_size;

	int32  i;
	int32  ret;

	if( p_path == NULL )
	{
		return GEN_E_INVALID_PARAM;
	}

	if( path_size < 1 || path_size > (GEN_DIR_MAX_PATH_SIZE * sizeof(wchar_t)) )
	{
		return GEN_E_INVALID_PARAM;
	}

	char_path_size = path_size / sizeof(wchar_t);
	for( i = 0; i < char_path_size; i++ )
	{
		if( p_path[i] == L'\\' )
		{
			str_path[i] = 0x0;

			ret = CreateDirectoryW(str_path, NULL);
		}

		str_path[i] = p_path[i];
	}

	if( str_path[char_path_size - 1] != L'\\' )
	{
		str_path[char_path_size] = 0x0;

		ret = CreateDirectoryW(str_path, NULL);
	}

	return GEN_S_OK;
}
#endif
