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
 * Chunk bitmap manipulations
 */

#ifndef __YAFFS_BITMAP_H__
#define __YAFFS_BITMAP_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void yaffs_verify_chunk_bit_id(struct yaffs_dev *dev, int blk, int chunk);
void yaffs_clear_chunk_bits(struct yaffs_dev *dev, int blk);
void yaffs_clear_chunk_bit(struct yaffs_dev *dev, int blk, int chunk);
void yaffs_set_chunk_bit(struct yaffs_dev *dev, int blk, int chunk);
int yaffs_check_chunk_bit(struct yaffs_dev *dev, int blk, int chunk);
int yaffs_still_some_chunks(struct yaffs_dev *dev, int blk);
int yaffs_count_chunk_bits(struct yaffs_dev *dev, int blk);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_CHECKPTRW_H__ */
