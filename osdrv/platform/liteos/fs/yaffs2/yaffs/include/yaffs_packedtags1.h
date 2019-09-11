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

/* This is used to pack YAFFS1 tags, not YAFFS2 tags. */

#ifndef __YAFFS_PACKEDTAGS1_H__
#define __YAFFS_PACKEDTAGS1_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


struct yaffs_packed_tags1 {
    u32 chunk_id:20;
    u32 serial_number:2;
    u32 n_bytes:10;
    u32 obj_id:18;
    u32 ecc:12;
    u32 deleted:1;
    u32 unused_stuff:1;
    unsigned should_be_ff;

};

void yaffs_pack_tags1(struct yaffs_packed_tags1 *pt,
              const struct yaffs_ext_tags *t);
void yaffs_unpack_tags1(struct yaffs_ext_tags *t,
            const struct yaffs_packed_tags1 *pt);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_PACKEDTAGS1_H__ */
