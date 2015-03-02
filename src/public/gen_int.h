/**
 * @file gen_int.h  General Integer Type
 * @author Robert Weng <robert_3000@sina.com>
 * @version 0.1
 * @date 2009.03.03
 *
 *
 */

#ifndef __GEN_INT_H__
#define __GEN_INT_H__

typedef char                 int8;
typedef unsigned char        uint8;
typedef short                int16;
typedef unsigned short       uint16;
typedef int                  int32;
typedef unsigned int         uint32;

#if defined (WIN32)

typedef __int64              int64;
typedef unsigned __int64     uint64;

#else

typedef long long            int64;
typedef unsigned long long   uint64;

#endif

#ifndef INT8_MIN
#define INT8_MIN                                           ((int8)0x80)
#endif

#ifndef INT8_MAX
#define INT8_MAX                                           ((int8)0x7F)
#endif

#ifndef UINT8_MIN
#define UINT8_MIN                                          (0x00)
#endif

#ifndef UINT8_MAX
#define UINT8_MAX                                          (0xFF)
#endif

#ifndef INT16_MIN
#define INT16_MIN                                          ((int16)0x8000)
#endif

#ifndef INT16_MAX
#define INT16_MAX                                          ((int16)0x7FFF)
#endif

#ifndef UINT16_MIN
#define UINT16_MIN                                         (0x0000)
#endif

#ifndef UINT16_MAX
#define UINT16_MAX                                         (0xFFFF)
#endif

#ifndef INT32_MIN
#define INT32_MIN                                          ((int32)0x80000000)
#endif

#ifndef INT32_MAX
#define INT32_MAX                                          ((int32)0x7FFFFFFF)
#endif

#ifndef UINT32_MIN
#define UINT32_MIN                                         (0x00000000)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX                                         (0xFFFFFFFF)
#endif

#ifndef UINT8_OVERFLOW
#define UINT8_OVERFLOW                                     (0x100)
#endif

#ifndef UINT16_OVERFLOW
#define UINT16_OVERFLOW                                    (0x10000)
#endif

#ifndef UINT32_OVERFLOW
#define UINT32_OVERFLOW                                    (0x100000000)
#endif

#ifndef NULL

#ifdef  __cplusplus

#define NULL                                               0

#else

#define NULL                                               ((void *)0)

#endif

#endif//NULL


/** gen fourcc */
#ifndef GEN_FOURCC
#define GEN_FOURCC(ch0, ch1, ch2, ch3)                     ( (uint32)(uint8)(ch0) \
															| ((uint32)(uint8)(ch1) << 8) \
															| ((uint32)(uint8)(ch2) << 16) \
															| ((uint32)(uint8)(ch3) << 24 ) )
#endif

#endif //__GEN_INT_H__
