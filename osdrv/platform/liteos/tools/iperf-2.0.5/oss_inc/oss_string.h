/************************************************************************
 *   os_string.h                                                        *
 *   Operating System Wrapper Prototypes and Defines                    *
 *   (C) Copyright embWiSe Technologies                                 *
 *                                                                      *
 *    Change log                                                        *
 *     07.AUG.2014 : Alagu Sankar : Base Version Created                *
 ************************************************************************/

#ifndef _OS_STRING_H_
#define _OS_STRING_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#include <oss_config.h>
#include <os_support.h>

#if USE_LIBC_STRING

#include <string.h>

#define oss_memcpy(dst, src, size)      memcpy((dst), (src), (size))
#define oss_memmove(p1, p2, size)       memmove((p1), (p2), (size))
#define oss_memset(dst, type, size)     memset((dst), (type), (size))
#define oss_memcmp(p1, p2, size)        memcmp((p1), (p2), (size))

#define oss_strlen(str)                 strlen(str)
#define oss_strdup(str)                 strdup(str)
#define oss_strcpy(dst, src)            strcpy((dst), (src))
#define oss_strcmp(s1, s2)              strcmp((s1), (s2))
#define oss_strcasecmp(s1, s2)          strcasecmp((s1), (s2))

#define oss_strlcpy(dst, src, n)        strncpy((dst), (src), (n))
#define oss_strncpy(dst, src, n)        strncpy((dst), (src), (n))
#define oss_strncmp(s1, s2, n)          strncmp((s1), (s2), (n))
#define oss_strncasecmp(s1, s2, n)      strncasecmp((s1), (s2), (n))

#define oss_strstr(phs, pn)             strstr((phs), (pn))
#define oss_strchr(str, c)              strchr((str), (c))
#define oss_strrchr(str, c)             strrchr((str), (c))

#else /* USE_LIBC_STRING */

#define oss_strlcpy(dst, src, n)        oss_strncpy((dst), (src), (n))
int oss_memcpy(void *pdest, const void *psrc, int size);
int oss_memset(void *pbuf, char value, int size);
int oss_memcmp(const void *p1, const void *p2, int size);
int oss_memmove(void *pdest, void *psrc, int size);

int oss_strlen(const char *str);
char *oss_strcpy(char *dest, const char *src);
int oss_strcmp(const char *str1, const char *str2);
char *oss_strdup(const char *str);
int oss_strcasecmp(const char *str1, const char *str2);

int oss_strncasecmp(const char *str1, const char *str2, unsigned long length);
unsigned int oss_strncpy(char *pdest, const char *psrc, int size);
int oss_strncmp(const char *str1,const char *str2, unsigned long  length);

char *oss_strstr(const char *phaystack, char *pneedle);
char *oss_strchr(const char *str, int c);
char *oss_strrchr(const char *str, int c);

#endif /* USE_LIBC_STRING */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _OS_STRING_H_ */

