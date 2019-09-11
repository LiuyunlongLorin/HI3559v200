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

#ifndef __YAFFS_VERIFY_H__
#define __YAFFS_VERIFY_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void yaffs_verify_blk(struct yaffs_dev *dev, struct yaffs_block_info *bi,
              int n);
void yaffs_verify_collected_blk(struct yaffs_dev *dev,
                struct yaffs_block_info *bi, int n);
void yaffs_verify_blocks(struct yaffs_dev *dev);

void yaffs_verify_oh(struct yaffs_obj *obj, struct yaffs_obj_hdr *oh,
             struct yaffs_ext_tags *tags, int parent_check);
void yaffs_verify_file(struct yaffs_obj *obj);
void yaffs_verify_link(struct yaffs_obj *obj);
void yaffs_verify_symlink(struct yaffs_obj *obj);
void yaffs_verify_special(struct yaffs_obj *obj);
void yaffs_verify_obj(struct yaffs_obj *obj);
void yaffs_verify_objects(struct yaffs_dev *dev);
void yaffs_verify_obj_in_dir(struct yaffs_obj *obj);
void yaffs_verify_dir(struct yaffs_obj *directory);
void yaffs_verify_free_chunks(struct yaffs_dev *dev);
int yaffs_verify_file_sane(struct yaffs_obj *obj);
int yaffs_skip_verification(struct yaffs_dev *dev);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_VERIFY_H__ */
