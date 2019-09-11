/**
 * @file      hi_uvc.h
 * @brief     uvc mode interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#ifndef __HI_UVC_H__
#define __HI_UVC_H__

#include "hi_usb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     USB */
/** @{ */  /** <!-- [USB] */

/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "USB"

/** uvc stream attribute */
typedef struct tagUVC_STREAM_ATTR_S
{
    HI_UVC_STREAM_FORMAT_E enFormat;
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32Fps;
    HI_U32 u32BitRate;
} UVC_STREAM_ATTR_S;

/** uvc stream operation set */
typedef struct tagUVC_STREAM_OPS_S
{
    HI_S32 (*pfnOpen)(HI_VOID);
    HI_S32 (*pfnClose)(HI_VOID);
    HI_S32 (*pfnReqIDR)(HI_VOID);
    HI_S32 (*pfnSetAttr)(UVC_STREAM_ATTR_S *pstAttr);
} UVC_STREAM_OPS_S;

/* UVC Context */
typedef struct tagUVC_CONTEXT_S
{
    HI_CHAR szDevPath[HI_APPCOMM_MAX_PATH_LEN];
    HI_BOOL bRun;
    HI_BOOL bPCConnect;
    pthread_t TskId;
    pthread_t Tsk2Id;
} UVC_CONTEXT_S;


/**
 * @brief     uvc mode init, load module
 * @param[in] pstCap : uvc device capabilities
 * @param[in] pstDataSrc : uvc stream data source
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_Init(const HI_UVC_DEVICE_CAP_S *pstCap, const HI_UVC_DATA_SOURCE_S *pstDataSrc, HI_UVC_BUFFER_CFG_S *pstBufferCfg);

/**
 * @brief     uvc mode deinit, unload module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_Deinit(HI_VOID);

/**
 * @brief     start uvc
 * @param[in] pszDevPath : uvc device path
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_Start(const HI_CHAR *pszDevPath);

/**
 * @brief     stop uvc
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
HI_S32 UVC_Stop(HI_VOID);

UVC_CONTEXT_S* UVC_GetCtx(HI_VOID);

/**
 * @brief     get uvc stream operation set
 * @return    operation set pointer
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 */
UVC_STREAM_OPS_S* UVC_GetStreamOps(HI_VOID);

/** @}*/  /** <!-- ==== USB End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_UVC_H__ */

