/**
 * @file log_agent_public.h  Log Agent Public
 * @author Robert Weng <robert_3000@sina.com>
 * @version 0.1
 * @date 2013.04.03
 *
 *
 */

#ifndef __LOG_AGENT_PUBLIC_H__
#define __LOG_AGENT_PUBLIC_H__

#if RUN_OS_WINDOWS

//#include <winsock2.h>
//#include <Ws2tcpip.h>
//#pragma comment(lib, "ws2_32.lib")

#else

#include <unistd.h>
#include <pthread.h>

#endif

#include <public/gen_int.h>
#include <public/gen_platform.h>

/** max log file dir size */
#define LA_MAX_LOG_FILE_DIR_SIZE                           (1 * 1024)
/** max log string size */
#define LA_MAX_LOG_STRING_SIZE                             (2 * 1024)


/** log agent */
typedef struct tag_log_agent
{
	/** init */
	int32   b_init;

	/** type */
	uint32  type;
	/** level */
	uint32  level;

	/** split file rule */
	int32   split_file_rule;
	/** split time */
	int32   split_time;
	/** split size */
	uint32  split_size;

	/** init mutex */
	int32   b_init_mutex;
#if RUN_OS_WINDOWS

	/** critical section */
	CRITICAL_SECTION  cs;

	/** file handle */
	HANDLE  h;

#else

	/** mutex attr */
	pthread_mutexattr_t mutex_attr;
	/** mutex */
	pthread_mutex_t     mutex;

	/** file handle */
	int32   fd;

#endif

	/** year of now time */
	int32  now_time_year;
	/** month of now time */
	int32  now_time_month;
	/** day of now time */
	int32  now_time_day;
	/** hour of now time */
	int32  now_time_hour;

	/** last split time, unit: second */
	int64  last_split_time;
	/** next split time, unit: second */
	int64  next_split_time;
	/** split file seq */
	int32  split_file_seq;

	/** file dir */
	int8   file_dir[LA_MAX_LOG_FILE_DIR_SIZE + 4];
	/** file dir size */
	int32  file_dir_size;

	/** str buf */
	int8   str_buf[LA_MAX_LOG_STRING_SIZE + 4];
	/** str buf size */
	int32  str_buf_size;

} LOG_AGENT, *PLOG_AGENT;


#endif ///__LOG_AGENT_PUBLIC_H__
