
#ifndef WLAN_OSS_CONFIG_H
#define WLAN_OSS_CONFIG_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef unsigned char            OS_BOOL;

#define OSS_INLINE          inline
#define OSS_STATIC          static

#ifndef NULL
#define NULL                0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define oss_signal_pending(cur_thread)  0

#define wpa_message_handler(cmd,reason)

#define A_GET_CACHE_LINE_BYTES()    OSS_CACHE_LINE_SIZE

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
