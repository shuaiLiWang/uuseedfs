/**
 * @file log_debug.h  Log Debugger
 * @author ���� <robert_3000@sina.com>
 * @version 0.1
 * @date 2009.03.05
 *
 *
 */

#ifndef __LOG_DEBUG_H__
#define __LOG_DEBUG_H__

#include <public/gen_int.h>
#include <public/gen_platform.h>
#include <public/gen_char_public.h>

/** �������Ļ */
#define LOG_OUTPUT_PRINT                                   (0x1)
/** �����sock */
#define LOG_OUTPUT_SOCK                                    (0x2)
/** �������־�ļ� */
#define LOG_OUTPUT_FILE                                    (0x4)
/** ȫ����� */
#define LOG_OUTPUT_ALL                                     (LOG_OUTPUT_PRINT | LOG_OUTPUT_FILE | LOG_OUTPUT_SOCK)

/** �������� */
#define LOG_FATAL_LEVEL                                    (0)
/** һ����� */
#define LOG_ERROR_LEVEL                                    (1)
/** ���� */
#define LOG_WARN_LEVEL                                     (2)
/** ���� */
#define LOG_NORMAL_LEVEL                                   (3)
/** ���� */
#define LOG_DEBUG_LEVEL                                    (4)



#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ��ʼ��
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1���ýӿڷǶ��̰߳�ȫ!
 */
int32 log_init();

/** 
 * @brief ����ʼ��
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1���ýӿڷǶ��̰߳�ȫ!
 */
int32 log_deinit();

/** 
 * @brief ������־�������
 * @param [in] level �������
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_set_level(int32 level);

/** 
 * @brief �����Ƿ���ʾ
 * @param [in] b_output �Ƿ���ʾ
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_set_output(int32 b_output);

/** 
 * @brief ����Զ�̲���
 * @param [in] ip Զ��IP�������ֽ���
 * @param [in] port Զ�̶˿ڣ������ֽ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1���ýӿڷǶ��̰߳�ȫ!
 */
int32 log_set_remote(uint32 ip, uint16 port);

/** 
 * @brief ������־�ļ�
 * @param [in] p_path ��־�ļ�·��
 * @param [in] path_len ·������
 * @param [in] file_len ��־�ļ�����
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1������־�ļ����ȳ���ָ������ʱ�����Զ������־
 */
int32 log_set_fileA(int8 *p_path, int32 path_len, uint32 file_len);

#if RUN_OS_WINDOWS
/** 
 * @brief ������־�ļ�
 * @param [in] p_path ��־�ļ�·��
 * @param [in] path_len ·������
 * @param [in] file_len ��־�ļ�����
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1������־�ļ����ȳ���ָ������ʱ�����Զ������־
 */
int32 log_set_fileW(wchar_t *p_path, int32 path_len, uint32 file_len);
#endif

/**  
 * @brief ���ָ������������
 * @param [in] p_buf ָ����ַ
 * @param [in] buf_size ����������
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1�����������ÿ��������16��
 */
int32 log_output_buf_data(int8* p_buf, int32 buf_size);

/**  
 * @brief д��־��ָ��������
 * @param [in] level ��־����
 * @param [in] p_buf ָ����ַ
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * >=0���ɹ�������ֵ��ʾʵ��д�볤��
 * -1��ʧ��
 * @remark
 * 1��ֻ�е���־����С�ڵ��������õ���־����ʱ�Żᱻд��
 * 2��ʵ��д�볤�Ȳ�����ĩβ��'\0'
 */
int32 log_to_bufA(int32 level, int8* p_buf, int8 *p_format, ...);

/**  
 * @brief д��־
 * @param [in] level ��־����
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1��ֻ�е���־����С�ڵ��������õ���־����ʱ�Żᱻд��
 */
int32 log_traceA(int32 level, int8 *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_debugA(int8 *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_warnA(int8 *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_errorA(int8 *p_format, ...);

#if RUN_OS_WINDOWS

/**  
 * @brief д��־��ָ��������
 * @param [in] level ��־����
 * @param [in] p_buf ָ����ַ
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * >=0���ɹ�������ֵ��ʾʵ��д�볤��
 * -1��ʧ��
 * @remark
 * 1��ֻ�е���־����С�ڵ��������õ���־����ʱ�Żᱻд��
 * 2��ʵ��д�볤�Ȳ�����ĩβ��'\0'
 */
int32 log_to_bufW(int32 level, wchar_t *p_buf, const wchar_t *p_format, ...);

/**  
 * @brief д��־
 * @param [in] level ��־����
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 * @remark
 * 1��ֻ�е���־����С�ڵ��������õ���־����ʱ�Żᱻд��
 * 2�������ر�˵������־���Ȳ��ܳ���2 * 1024!
 */
int32 log_traceW(int32 level, const wchar_t *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_debugW(const wchar_t *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_warnW(const wchar_t *p_format, ...);

/**  
 * @brief д������־
 * @param [in] p_format ��ʽ���ַ���
 * @return
 * 0���ɹ�
 * -1��ʧ��
 */
int32 log_errorW(const wchar_t *p_format, ...);

#endif

#if RUN_OS_WINDOWS

/** windows */
#ifdef UNICODE

#define log_set_file  log_set_fileW
#define log_to_buf    log_to_bufW
#define log_trace     log_traceW
#define log_debug     log_debugW
#define log_warn      log_warnW
#define log_error     log_errorW

#else

#define log_set_file  log_set_fileA
#define log_to_buf    log_to_bufA
#define log_trace     log_traceA
#define log_debug     log_debugA
#define log_warn      log_warnA
#define log_error     log_errorA

#endif//unicode

#else

/** linux */
#define log_set_file  log_set_fileA
#define log_to_buf    log_to_bufA
#define log_trace     log_traceA
#define log_debug     log_debugA
#define log_warn      log_warnA
#define log_error     log_errorA

#endif//if RUN_OS_WINDOWS



#ifdef __cplusplus
}
#endif

#endif ///__LOG_DEBUG_H__
