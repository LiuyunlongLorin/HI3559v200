/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file       dng_muxer.h
  * @brief      middleware log function.
  * @author   HiMobileCam middleware develop team
  * @date      2018.03.08
  */
#ifndef __DNG_COMMON_H__
#define __DNG_COMMON_H__

#include "hi_mw_type.h"
#include "hi_defs.h"
#include "hi_dng.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define DNG_MUXER_SUBIFD_MAX (2)
#define DNG_MAX_COUNT        (32) /* instance count max */

typedef struct hiDNG_FORMAT_S {
    HI_DNG_CONFIG_TYPE_E enConfigType;
    TIFF *pstTiffHandle;
    HI_BOOL bIFD0Flag;     // add ifd0 or not
    HI_BOOL bGetInfoFlag;  // get image info or not
    HI_U32 u32SubIfdNum;   // subifd num
    HI_U32 u32CurIndex;
    List_Head_S pstDngListPtr; /* list node for mp4 */
    pthread_mutex_t mDngLock;
} DNG_FORMAT_S;

typedef struct hiDNG_CTX_S {
    HI_BOOL bProc;
    HI_S32 s32DngNum;
    pthread_mutex_t mDngListLock;
    List_Head_S pDngList;
} HI_DNG_CTX_S;

HI_S32 DNG_MUXER_SetScreenNailIFD(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo);
HI_S32 DNG_MUXER_SetRawIFD(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo);
HI_S32 DNG_MUXER_SetIFD0(TIFF *tif, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo);
HI_S32 DNG_MUXER_CheckIfdInfo(DNG_FORMAT_S *pstDng, HI_DNG_MUXER_IFD_INFO_S *pstIFDInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DNG_COMMON_H__ */
