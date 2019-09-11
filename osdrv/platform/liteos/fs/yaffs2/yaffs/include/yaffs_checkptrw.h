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

#ifndef __YAFFS_CHECKPTRW_H__
#define __YAFFS_CHECKPTRW_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


int yaffs2_checkpt_open(struct yaffs_dev *dev, int writing);

int yaffs2_checkpt_wr(struct yaffs_dev *dev, const void *data, int n_bytes);

int yaffs2_checkpt_rd(struct yaffs_dev *dev, void *data, int n_bytes);

int yaffs2_get_checkpt_sum(struct yaffs_dev *dev, u32 * sum);

int yaffs_checkpt_close(struct yaffs_dev *dev);

int yaffs2_checkpt_invalidate_stream(struct yaffs_dev *dev);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_CHECKPTRW_H__ */
