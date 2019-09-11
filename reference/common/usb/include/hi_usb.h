/**
 * @file      hi_usb.h
 * @brief     usb module interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version

 */

#ifndef __HI_USB_H__
#define __HI_USB_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     USB */
/** @{ */  /** <!-- [USB] */


/** error code define */
#define HI_USB_EINVAL               HI_APPCOMM_ERR_ID(HI_APP_MOD_USBMNG, HI_EINVAL)       /**<Invalid argument */
#define HI_USB_ENOTINIT             HI_APPCOMM_ERR_ID(HI_APP_MOD_USBMNG, HI_ENOINIT)      /**<Not inited */
#define HI_USB_EINITIALIZED         HI_APPCOMM_ERR_ID(HI_APP_MOD_USBMNG, HI_EINITIALIZED) /**<reinitialized */


/** usb event define */
#define HI_EVENT_USB_OUT            HI_APPCOMM_EVENT_ID(HI_APP_MOD_USBMNG, 0) /**<usb out */
#define HI_EVENT_USB_INSERT         HI_APPCOMM_EVENT_ID(HI_APP_MOD_USBMNG, 1) /**<usb insert */
#define HI_EVENT_USB_UVC_READY      HI_APPCOMM_EVENT_ID(HI_APP_MOD_USBMNG, 2) /**<uvc mode: pc ready */
#define HI_EVENT_USB_STORAGE_READY  HI_APPCOMM_EVENT_ID(HI_APP_MOD_USBMNG, 3) /**<storage mode: pc ready */

/** ---------------- UVC Define Begin ---------------- */

/** uvc stream format enum */
typedef enum hiUVC_STREAM_FORMAT_E
{
    HI_UVC_STREAM_FORMAT_YUV420 = 0,
    HI_UVC_STREAM_FORMAT_MJPEG,
    HI_UVC_STREAM_FORMAT_H264,
    HI_UVC_STREAM_FORMAT_BUTT
} HI_UVC_STREAM_FORMAT_E;

/** uvc video mode enum, including resolution and framerate
   * should be a subset of driver capabilities */
typedef enum hiUVC_VIDEOMODE_E
{
    HI_UVC_VIDEOMODE_VGA30 = 0, /**<640x360   30fps */
    HI_UVC_VIDEOMODE_720P30,    /**<1280x720  30fps */
    HI_UVC_VIDEOMODE_1080P30,   /**<1920x1080 30fps */
    HI_UVC_VIDEOMODE_4K30,      /**<3840x2160 30fps */
    HI_UVC_VIDEOMODE_BUTT
} HI_UVC_VIDEOMODE_E;

typedef struct hiUVC_VIDEOATTR_S
{
    HI_U32 u32BitRate;
    HI_UVC_VIDEOMODE_E enMode;
} HI_UVC_VIDEOATTR_S;
/** uvc format capabilities, including videomode cnt and list */
typedef struct hiUVC_FORMAT_CAP_S
{
    HI_U32 u32Cnt;
    HI_UVC_VIDEOMODE_E enDefMode;
    HI_UVC_VIDEOATTR_S astModes[HI_UVC_VIDEOMODE_BUTT];
} HI_UVC_FORMAT_CAP_S;

/** uvc device capabilities, including all format */
typedef struct hiUVC_DEVICE_CAP_S
{
    HI_UVC_FORMAT_CAP_S astFmtCaps[HI_UVC_STREAM_FORMAT_BUTT];
} HI_UVC_DEVICE_CAP_S;

/** uvc data source handle */
typedef struct hiUVC_DATA_SOURCE_S
{
    HI_HANDLE VprocHdl; /**<video processor handle */
    HI_HANDLE VportHdl; /**<video processor port handle */
    HI_HANDLE VencHdl;  /**<video encoder handle */
    HI_HANDLE AcapHdl;  /**<audio capture handle */
} HI_UVC_DATA_SOURCE_S;

/** uvc data source handle */
typedef struct hiUVC_BUFFER_CFG_S
{
    HI_U32 u32BufCnt;
    HI_U32 u32BufSize;
} HI_UVC_BUFFER_CFG_S;

/** uvc configure */
typedef struct hiUVC_CFG_S
{
    HI_CHAR szDevPath[HI_APPCOMM_MAX_PATH_LEN]; /**<uvc device path */
    HI_UVC_DEVICE_CAP_S  stDevCap;
    HI_UVC_DATA_SOURCE_S stDataSource;
    HI_UVC_BUFFER_CFG_S stBufferCfg;
} HI_UVC_CFG_S;

/** ---------------- UVC Define End ---------------- */

/** ---------------- USB Storage Define Begin ---------------- */

/** usb storage configure */
typedef struct hiUSB_STORAGE_CFG_S
{
    HI_CHAR szProcFile[HI_APPCOMM_MAX_PATH_LEN]; /**<usb state proc file */
    HI_CHAR szSysFile[HI_APPCOMM_MAX_PATH_LEN]; /**<usb storage device system file */
    HI_CHAR szDevPath[HI_APPCOMM_MAX_PATH_LEN]; /**<usb storage device path(sd device path) */
} HI_USB_STORAGE_CFG_S;

/** ---------------- USB Storage Define End ---------------- */

/** usb event information */
typedef struct hiUSB_EVENT_INFO_S
{
    HI_S32  s32EventId;
    HI_CHAR szPayload[HI_APPCOMM_MAX_PATH_LEN];
} HI_USB_EVENT_INFO_S;

/** usb state callback */
typedef HI_S32 (*HI_USB_EVENTPROC_CALLBACK_FN_PTR)(const HI_USB_EVENT_INFO_S *pstEventInfo);


/** usb state enum, including uvc/storage state */
typedef enum hiUSB_STATE_E
{
    HI_USB_STATE_OUT = 0,          /**<usb out */
    HI_USB_STATE_INSERT,           /**<usb insert */
    HI_USB_STATE_UVC_READY,        /**<uvc driver load ready */
    HI_USB_STATE_UVC_PC_READY,     /**<uvc pc interaction ready */
    HI_USB_STATE_UVC_MEDIA_READY,  /**<uvc media ready */
    HI_USB_STATE_STORAGE_READY,    /**<storage driver load ready */
    HI_USB_STATE_STORAGE_PC_READY, /**<storage pc interaction ready */
    HI_USB_STATE_STORAGE_SD_READY, /**<storage sd device ready */
    HI_USB_STATE_BUTT
} HI_USB_STATE_E;

/** usb mode enum */
typedef enum hiUSB_MODE_E
{
    HI_USB_MODE_CHARGE = 0,
    HI_USB_MODE_UVC,
    HI_USB_MODE_STORAGE,
    HI_USB_MODE_BUTT
} HI_USB_MODE_E;

typedef HI_S32 (*HI_USB_GET_STORAGE_STATE_FN_PTR)(HI_VOID* pvPrivData);

/** usb configure */
typedef struct hiUSB_CFG_S
{
    HI_UVC_CFG_S stUvcCfg;
    HI_USB_STORAGE_CFG_S stStorageCfg;
    HI_USB_EVENTPROC_CALLBACK_FN_PTR pfnEventProc;
    HI_USB_GET_STORAGE_STATE_FN_PTR pfnGetStorageState;
} HI_USB_CFG_S;


/**
 * @brief     Usb Module Init
 * @param[in] pstCfg : usb configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_USB_Init(const HI_USB_CFG_S* pstCfg);

/**
 * @brief     Usb Module Deinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_USB_Deinit(HI_VOID);

/**
 * @brief     set uvc configure
 * @param[in] pstCfg : uvc configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/26
 */
HI_S32 HI_USB_SetUvcCfg(const HI_UVC_CFG_S* pstCfg);

/**
 * @brief     set usb storage configure
 * @param[in] pstCfg : usb storage configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/26
 */
HI_S32 HI_USB_SetStorageCfg(const HI_USB_STORAGE_CFG_S* pstCfg);

/**
 * @brief     set usb mode
 * @param[in] enMode : usb mode
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_USB_SetMode(HI_USB_MODE_E enMode);

/**
 * @brief     get usb mode
 * @param[out]penMode : usb mode
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_USB_GetMode(HI_USB_MODE_E *penMode);

/**
 * @brief     get usb state
 * @param[out]penState : usb state
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 */
HI_S32 HI_USB_GetState(HI_USB_STATE_E *penState);


/** @}*/  /** <!-- ==== USB End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HI_USB_H__ */

