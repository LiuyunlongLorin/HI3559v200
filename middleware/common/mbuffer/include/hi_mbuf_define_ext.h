/**
    * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
    * @Description  common module header file- structs ext definition of mbuffer
    * @Author   HiMobileCam middleware develop team
    * @Create:      2016.06.29
 */

#ifndef __HI_MBUF_DEFINE_EXT_H__
#define __HI_MBUF_DEFINE_EXT_H__

#define HI_MBUF_FRAME_MAX_BLOCK (8)

#define HI_MBUF_MAX_PAYLOAD_TYPE_CNT (8)

#include "hi_mw_type.h"
/** \addtogroup     COMMON */
/** @{ */ /** <!-- [COMMON] */

/** error number  */
#define HI_ERR_MBUF_BASE         HI_FAILURE
#define HI_ERR_MBUF_ILLEGALPARAM (HI_ERR_MBUF_BASE - 1)  /* illegal params */
#define HI_ERR_MBUF_REINTIALIZE  (HI_ERR_MBUF_BASE - 2)  /* re init of mbuffer */
#define HI_ERR_MBUF_UNINTIALIZE  (HI_ERR_MBUF_BASE - 3)  /* used with out init */
#define HI_ERR_MBUF_FULL         (HI_ERR_MBUF_BASE - 4)  /* mbuffer is full */
#define HI_ERR_MBUF_NODATA       (HI_ERR_MBUF_BASE - 5)  /*  cannot find the dada of the payload type */
#define HI_ERR_MBUF_REACHLIMIT   (HI_ERR_MBUF_BASE - 6)  /*  PayloadType reach the max num definition */
#define HI_ERR_MBUF_DISABLED     (HI_ERR_MBUF_BASE - 7)  /* read or write is not able to access */
#define HI_ERR_MBUF_DISLOCATED   (HI_ERR_MBUF_BASE - 8)  /*  err with the pos ,only can read in slice*/
#define HI_ERR_MBUF_UNCOMPLETED  (HI_ERR_MBUF_BASE - 9)  /* do not write the frame compelety */
#define HI_ERR_MBUF_NOTKEYFRAME  (HI_ERR_MBUF_BASE - 10) /* frame which is first or whose prev frame is not key framef */

#define CHECK_START_SLICE(x)  ((x) & (0x01)) /* check start slice of a frame */
#define CHECK_NORMAL_SLICE(x) ((x) & (0x02)) /* check middle slice of a frame */
#define CHECK_END_SLICE(x)    ((x) & (0x04)) /* check end slice of a frame */

#define SET_START_SLICE(x)  ((x) |= (0x01)) /* set the start slice of a frame */
#define SET_NORMAL_SLICE(x) ((x) |= (0x02)) /* set the middle slice of a frame */
#define SET_END_SLICE(x)    ((x) |= (0x04)) /* set the end slice of a frame */
#define CLEAR_SLICE(x)      ((x) = 0)       /* clear the slice of a frame */

/* call back funs of mbuffer */
typedef HI_S32 (*Mbuf_Alloc_Func)(HI_VOID *argv, HI_U32 u32BufSize, HI_VOID **ppVmAddr);

typedef HI_S32 (*Mbuf_Free_Func)(HI_VOID *argv, HI_VOID *pVmAddr);

typedef HI_S32 (*Mbuf_Mcopy_Func)(HI_VOID *argv, HI_VOID *pDest, HI_VOID *pSrc, HI_U32 u32Size);

typedef HI_S32 (*Mbuf_CreateLock_Func)(HI_VOID *argv);

typedef HI_S32 (*Mbuf_DestroyLock_Func)(HI_VOID *argv);

typedef HI_S32 (*Mbuf_Lock_Func)(HI_VOID *argv);

typedef HI_S32 (*Mbuf_UnLock_Func)(HI_VOID *argv);

/* funcs of mbuffer abstract */
typedef struct hi_MBUF_ABSTRACTFUNC_S {
    Mbuf_Alloc_Func pfnAlloc;             /* get the memory addr of the mbuffer */
    Mbuf_Free_Func pfnFree;               /* release memory space of the mbuffer func */
    Mbuf_Mcopy_Func pfnMemcpy;            /* mbuffer memory copy func */
    Mbuf_CreateLock_Func pfnCreateLock;   /* mbuffer func of create lock */
    Mbuf_DestroyLock_Func pfnDestroyLock; /* mbuffer func of destroy lock */
    Mbuf_Lock_Func pfnLock;               /* lock of mbuffer */
    Mbuf_UnLock_Func pfnUnLock;           /* unlock of mbuffer */
} HI_MBUF_ABSTRACTFUNC_S;

/* mbuffer config */
typedef struct hi_MBUF_CFG_S {
    HI_U32 u32BufSize;                 /* memory size of mbuffer */
    HI_S32 s32MaxPayload;              /* max patload type num in mbuffer */
    HI_MBUF_ABSTRACTFUNC_S stMBufFunc; /* register mbuf func structs */
} HI_MBUF_CFG_S;

/* mbuffer slice info */
typedef struct hi_MBUF_SLICEINFO_S {
    HI_U64 u64Pts;                                /* timestamp if the frame */
    HI_U32 u32Seq;                                /* sequence num of the frame */
    HI_U32 u32FrameLen;                           /* length of the frame data */
    HI_U32 u32SliceLen;                           /* length of the slice data */
    HI_U32 u32HeadLen;                            /* length of slice data head space  */
    HI_U32 u32TailLen;                            /* length of slilce data tail space */
    HI_U32 u32SliceNum;                           /* slice sequence number */
    HI_U32 u32SliceCnt;                           /* slice count of the frame data */
    HI_BOOL bIsKeyFrame;                          /* whether keyframe or not */
    HI_U8 u8PayloadType;                          /* mbuffer data payload type 0~254 */
    HI_U8 u8SliceType;                            /* slice type 0x01-start 0x02-middle 0x04-end 0x05-single frame */
    HI_U8 *pAddr[HI_MBUF_FRAME_MAX_BLOCK];        /* start address of the data block */
    HI_U32 au32BlockLen[HI_MBUF_FRAME_MAX_BLOCK]; /* len of the block data agree with pAdder */
    HI_U32 u32BlockCount;
} HI_MBUF_SLICEINFO_S;

/* mbuffer frame data info */
typedef struct hi_MBUF_FRAMEINFO_S {
    HI_U32 u32SliceCnt;                /* mbuffer slice data count */
    HI_MBUF_SLICEINFO_S *pstSliceInfo; /* mbuffer slice data  info */
} HI_MBUF_FRAMEINFO_S;

/** @} */ /** <!-- ==== COMMON End ==== */

#endif /* __HI_MBUF_DEFINE_EXT_H__ */
