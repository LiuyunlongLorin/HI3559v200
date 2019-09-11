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
#include "hi_comm_isp.h"
#include "hi_dng.h"
#include "hi_appcomm.h"
#include "hi_photomng.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

typedef struct tagPHOTOMNG_DNG_THM_BUF_S
{
    HI_BOOL bDataArrived;
    HI_DNG_MUXER_IFD_INFO_S stThmIFD;
} PHOTOMNG_DNG_THM_BUF_S;

typedef struct tagPHOTOMNG_DNG_SCREEMNAIL_BUF_S
{
    HI_BOOL bDataArrived;
    HI_DNG_MUXER_IFD_INFO_S stScreenIFD;
} PHOTOMNG_DNG_SCREEMNAIL_BUF_S;

typedef struct tagPHOTOMNG_DNG_RAW_BUF_S
{
    HI_DNG_MUXER_IFD_INFO_S stRawIFD;
} PHOTOMNG_DNG_RAW_BUF_S;

typedef struct tagPHOTOMNG_DNG_DATA_BUFFER_S
{
    PHOTOMNG_DNG_THM_BUF_S stThmInfo;
    PHOTOMNG_DNG_SCREEMNAIL_BUF_S stScreenNailInfo;
    PHOTOMNG_DNG_RAW_BUF_S stRawInfo;
} PHOTOMNG_DNG_DATA_BUFFER_S;


HI_S32 PHOTOMNG_DNG_SetThmIFD(HI_DNG_MUXER_IFD_INFO_S*  pstThmIFD,
                              HI_U32 u32Width,
                              HI_U32 u32Height,
                              DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                              DNG_IMAGE_STATIC_INFO_S* pstStaticInfo,
                              JPEG_DCF_S* pstJpegDcf,
                              HI_PHOTOMNG_DNG_USERINFO_S* pstDNGUserInfo);

HI_S32 PHOTOMNG_DNG_SetRawIFD(HI_DNG_MUXER_IFD_INFO_S* pstRawIFD,
                              DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                              DNG_IMAGE_STATIC_INFO_S* pstStaticInfo);

HI_S32 PHOTOMNG_DNG_SetScreenIFD(HI_DNG_MUXER_IFD_INFO_S* pstScreenIFD,
                                 HI_U32 u32Width,
                                 HI_U32 u32Height);

HI_S32 PHOTOMNG_DNG_SetIFD0(HI_DNG_MUXER_IFD_INFO_S* pstIFD,
                            DNG_IMAGE_DYNAMIC_INFO_S* pstDynInfo,
                            DNG_IMAGE_STATIC_INFO_S* pstStaticInfo,
                            JPEG_DCF_S* pstJpegDcf,
                            HI_PHOTOMNG_DNG_USERINFO_S* pstDNGUserInfo);

HI_S32 PHOTOMNG_DNG_Muxer(PHOTOMNG_DNG_DATA_BUFFER_S* pstDataBuf, HI_CHAR* pszFileName, HI_BOOL bOneRaw);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
