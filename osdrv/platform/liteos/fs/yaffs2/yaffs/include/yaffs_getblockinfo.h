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

#ifndef __YAFFS_GETBLOCKINFO_H__
#define __YAFFS_GETBLOCKINFO_H__

#include "yaffs_guts.h"
#include "yaffs_trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/* Function to manipulate block info */
static inline struct yaffs_block_info *yaffs_get_block_info(struct yaffs_dev
                                  *dev, int blk)
{
    if (blk < dev->internal_start_block || blk > dev->internal_end_block) {
        yaffs_trace(YAFFS_TRACE_ERROR,
            "**>> yaffs: get_block_info block %d is not valid",
            blk);
        BUG();
    }
    return &dev->block_info[blk - dev->internal_start_block];
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_GETBLOCKINFO_H__ */
