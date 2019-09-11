/**
 * @file    photomng_dngmux.h
 * @brief   dng muxer adapt header file
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/2
 * @version   1.0

 */
#include "hi_appcomm.h"
#include "hi_videodetect.h"

#include "ivs_md.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

HI_S32 VIDEODETECT_MD_Init(HI_VOID);

HI_S32 VIDEODETECT_MD_Deinit(HI_VOID);

HI_S32 VIDEODETECT_MD_CreateChn(HI_HANDLE taskHdl, const MD_ATTR_S* mdAttr);

HI_S32 VIDEODETECT_MD_DestroyChn(HI_HANDLE taskHdl);

HI_S32 VIDEODETECT_MD_Process(HI_HANDLE taskHdl, HI_MAPI_FRAME_DATA_S* pstCurFrame, HI_MAPI_FRAME_DATA_S* pstRefFrame, IVE_CCBLOB_S* pstBlob);


/***EventProc**/
HI_S32 VIDEODETECT_MD_EventProcCreate(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_MD_S* algProcMdAttr);

HI_S32 VIDEODETECT_MD_EventProcDestry(HI_HANDLE taskHdl);

HI_S32 VIDEODETECT_MD_AlgProc(HI_VIDEODETECT_DATA_S* svpData, HI_VOID* pPrivateData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
