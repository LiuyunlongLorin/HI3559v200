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

#ifndef __YTRACE_H__
#define __YTRACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


extern unsigned int yaffs_trace_mask;
extern unsigned int yaffs_wr_attempts;

/*
 * Tracing flags.
 * The flags masked in YAFFS_TRACE_ALWAYS are always traced.
 */

#define YAFFS_TRACE_OS          0x00000002
#define YAFFS_TRACE_ALLOCATE        0x00000004
#define YAFFS_TRACE_SCAN        0x00000008
#define YAFFS_TRACE_BAD_BLOCKS      0x00000010
#define YAFFS_TRACE_ERASE       0x00000020
#define YAFFS_TRACE_GC          0x00000040
#define YAFFS_TRACE_WRITE       0x00000080
#define YAFFS_TRACE_TRACING     0x00000100
#define YAFFS_TRACE_DELETION        0x00000200
#define YAFFS_TRACE_BUFFERS     0x00000400
#define YAFFS_TRACE_NANDACCESS      0x00000800
#define YAFFS_TRACE_GC_DETAIL       0x00001000
#define YAFFS_TRACE_SCAN_DEBUG      0x00002000
#define YAFFS_TRACE_MTD         0x00004000
#define YAFFS_TRACE_CHECKPOINT      0x00008000

#define YAFFS_TRACE_VERIFY      0x00010000
#define YAFFS_TRACE_VERIFY_NAND     0x00020000
#define YAFFS_TRACE_VERIFY_FULL     0x00040000
#define YAFFS_TRACE_VERIFY_ALL      0x000f0000

#define YAFFS_TRACE_SYNC        0x00100000
#define YAFFS_TRACE_BACKGROUND      0x00200000
#define YAFFS_TRACE_LOCK        0x00400000
#define YAFFS_TRACE_MOUNT       0x00800000

#define YAFFS_TRACE_ERROR       0x40000000
#define YAFFS_TRACE_BUG         0x80000000
#define YAFFS_TRACE_ALWAYS      0xf0000000


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __YTRACE_H__ */
