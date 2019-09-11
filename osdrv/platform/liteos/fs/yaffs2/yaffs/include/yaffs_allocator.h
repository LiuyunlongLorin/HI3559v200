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

#ifndef __YAFFS_ALLOCATOR_H__
#define __YAFFS_ALLOCATOR_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void yaffs_init_raw_tnodes_and_objs(struct yaffs_dev *dev);
void yaffs_deinit_raw_tnodes_and_objs(struct yaffs_dev *dev);

struct yaffs_tnode *yaffs_alloc_raw_tnode(struct yaffs_dev *dev);
void yaffs_free_raw_tnode(struct yaffs_dev *dev, struct yaffs_tnode *tn);

struct yaffs_obj *yaffs_alloc_raw_obj(struct yaffs_dev *dev);
void yaffs_free_raw_obj(struct yaffs_dev *dev, struct yaffs_obj *obj);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_ALLOCATOR_H__ */
