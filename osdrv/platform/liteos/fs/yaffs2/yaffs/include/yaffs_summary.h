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

#ifndef __YAFFS_SUMMARY_H__
#define __YAFFS_SUMMARY_H__

#include "yaffs_packedtags2.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


int yaffs_summary_init(struct yaffs_dev *dev);
void yaffs_summary_deinit(struct yaffs_dev *dev);

int yaffs_summary_add(struct yaffs_dev *dev,
            struct yaffs_ext_tags *tags,
            int chunk_in_block);
int yaffs_summary_fetch(struct yaffs_dev *dev,
            struct yaffs_ext_tags *tags,
            int chunk_in_block);
int yaffs_summary_read(struct yaffs_dev *dev,
            struct yaffs_summary_tags *st,
            int blk);
void yaffs_summary_gc(struct yaffs_dev *dev, int blk);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_TAGSCOMPAT_H__ */
