/************************************************************************
 * Operating System Configuration                                       *
 * (C) Copyright embWiSe Technologies                                   *
 *                                                                      *
 * Change log                                                           *
 *   07.AUG.2014 : Alagu Sankar : Skeleton File Created                 *
 ************************************************************************/

#ifndef _OSS_CONFIG_H_
#define _OSS_CONFIG_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * String and memory function Selection
 *
 * 0 = Internal functions are used. During development stages, it is good
 *     to have this disabled so as to enable debugging and statistics.
 *
 * 1 = Standard C library functions are used. When available it is good
 *     enable this in production releases, to reduce code size
 */
#define USE_LIBC_STRING                 (1)

#define DEBUG_OSSUPPORT                 (1)

#define OSS_BIG_ENDIAN                  (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _OSS_CONFIG_H_ */

