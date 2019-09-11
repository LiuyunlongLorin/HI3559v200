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

#ifndef __YAFFS_ATTRIBS_H__
#define __YAFFS_ATTRIBS_H__

#include "yaffs_guts.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void yaffs_load_attribs(struct yaffs_obj *obj, struct yaffs_obj_hdr *oh);
void yaffs_load_attribs_oh(struct yaffs_obj_hdr *oh, struct yaffs_obj *obj);
void yaffs_attribs_init(struct yaffs_obj *obj, u32 gid, u32 uid, u32 rdev);
void yaffs_load_current_time(struct yaffs_obj *obj, int do_a, int do_c);
int yaffs_set_attribs(struct yaffs_obj *obj, struct iattr *attr);
int yaffs_get_attribs(struct yaffs_obj *obj, struct iattr *attr);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YAFFS_ATTRIBS_H__ */
