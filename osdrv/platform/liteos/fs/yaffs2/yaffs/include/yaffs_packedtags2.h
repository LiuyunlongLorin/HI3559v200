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

/* This is used to pack YAFFS2 tags, not YAFFS1tags. */

#ifndef __YAFFS_PACKEDTAGS2_H__
#define __YAFFS_PACKEDTAGS2_H__

#include "yaffs_guts.h"
#include "yaffs_ecc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


struct yaffs_packed_tags2_tags_only {
    unsigned seq_number;
    unsigned obj_id;
    unsigned chunk_id;
    unsigned n_bytes;
};

struct yaffs_packed_tags2 {
    struct yaffs_packed_tags2_tags_only t;
    struct yaffs_ecc_other ecc;
};

/* Full packed tags with ECC, used for oob tags */
void yaffs_pack_tags2(struct yaffs_packed_tags2 *pt,
              const struct yaffs_ext_tags *t, int tags_ecc);
void yaffs_unpack_tags2(struct yaffs_ext_tags *t, struct yaffs_packed_tags2 *pt,
            int tags_ecc);

/* Only the tags part (no ECC for use with inband tags */
void yaffs_pack_tags2_tags_only(struct yaffs_packed_tags2_tags_only *pt,
                const struct yaffs_ext_tags *t);
void yaffs_unpack_tags2_tags_only(struct yaffs_ext_tags *t,
                  struct yaffs_packed_tags2_tags_only *pt);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_PACKEDTAGS2_H__ */
