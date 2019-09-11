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

/*
 * Header file for using yaffs in an application via
 * a direct interface.
 */

#ifndef __YAFFS_OSGLUE_H__
#define __YAFFS_OSGLUE_H__

#include "yportenv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void yaffsfs_Lock(void);
void yaffsfs_Unlock(void);
void yaffsfs_LockInit(void);
void yaffsfs_LockDestroy(void);
u32 yaffsfs_CurrentTime(void);

void yaffsfs_SetError(int err);

void *yaffsfs_malloc(size_t size);
void yaffsfs_free(void *ptr);

int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request);

void yaffsfs_OSInitialisation(void);
void yaffsfs_OsDestroy(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_OSGLUE_H__ */
