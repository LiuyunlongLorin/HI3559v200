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

#ifndef __YAFFS_NAND_H__
#define __YAFFS_NAND_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


int yaffs_rd_chunk_tags_nand(struct yaffs_dev *dev, int nand_chunk,
                 u8 *buffer, struct yaffs_ext_tags *tags);

int yaffs_wr_chunk_tags_nand(struct yaffs_dev *dev,
                 int nand_chunk,
                 const u8 *buffer, struct yaffs_ext_tags *tags);

int yaffs_mark_bad(struct yaffs_dev *dev, int block_no);

int yaffs_query_init_block_state(struct yaffs_dev *dev,
                 int block_no,
                 enum yaffs_block_state *state,
                 unsigned *seq_number);

int yaffs_erase_block(struct yaffs_dev *dev, int flash_block);

int yaffs_init_nand(struct yaffs_dev *dev);
int yaffs_deinit_nand(struct yaffs_dev *dev);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_NAND_H__ */
