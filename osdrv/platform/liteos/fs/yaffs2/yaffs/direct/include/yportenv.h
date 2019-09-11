/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2015 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This software is the subject of a Licence Agreement between Aleph One Limited and Hi_Silicon_Huawei_DigiMedia.
 *
 */


#ifndef __YPORTENV_H__
#define __YPORTENV_H__

#include "errno.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/stat.h"
#include "dirent.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define CONFIG_YAFFS_DIRECT
#define CONFIG_YAFFS_PROVIDE_DEFS
#define __CC_ARM

/* Definition of types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

#ifdef CONFIG_YAFFS_PROVIDE_DEFS
/*
 * Attribute flags.
 * These are or-ed together to select what has been changed.
 */
#define ATTR_MODE   1
#define ATTR_UID    2
#define ATTR_GID    4
#define ATTR_SIZE   8
#define ATTR_ATIME  16
#define ATTR_MTIME  32
#define ATTR_CTIME  64

struct iattr {
    unsigned int ia_valid;
    unsigned ia_mode;
    unsigned ia_uid;
    unsigned ia_gid;
    unsigned ia_size;
    unsigned ia_atime;
    unsigned ia_mtime;
    unsigned ia_ctime;
    unsigned int ia_attr_flags;
};
#endif

#if defined CONFIG_YAFFS_WINCE
#include "ywinceenv.h"
#elif defined CONFIG_YAFFS_DIRECT
/* Direct interface */
#include "ydirectenv.h"
#elif defined CONFIG_YAFFS_UTIL
#include "yutilsenv.h"
#else
/* Should have specified a configuration type */
#error Unknown configuration
#endif

#ifndef Y_DUMP_STACK
#define Y_DUMP_STACK() do { } while (0)
#endif
#ifndef XATTR_CREATE
#define XATTR_CREATE 1
#endif

#ifndef XATTR_REPLACE
#define XATTR_REPLACE 2
#endif
#ifndef BUG
#define BUG() do {\
    yaffs_trace(YAFFS_TRACE_BUG,\
        "==>> yaffs bug: " __FILE__ " %d",\
        __LINE__);\
    Y_DUMP_STACK();\
} while (0)
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YPORTENV_H__ */
