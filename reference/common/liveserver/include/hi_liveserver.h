/**
 * @file    hi_liveserver.h
 * @brief   live server module struct and interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/14
 * @version   1.0

 */

#ifndef __HI_LIVESVR_H__
#define __HI_LIVESVR_H__

#include "hi_appcomm.h"
#include "hi_media_comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**\addtogroup     LIVESVR */
/**@{ */  /**<!-- [LIVESVR] */

/** live server name pattern, used in webserver distrib link */
#define LIVESTREAM_NAME_PATTERN "livestream"

/** error code define */
#define HI_LIVESVR_EINVAL               HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_EINVAL)
#define HI_LIVESVR_ENOINIT              HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_ENOINIT)
#define HI_LIVESVR_EINITIALIZED         HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_EINITIALIZED)
#define HI_LIVESVR_EEXIST               HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_EEXIST)
#define HI_LIVESVR_ELOST                HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_ELOST)
#define HI_LIVESVR_EMAXSOURCE           HI_APPCOMM_ERR_ID(HI_APP_MOD_LIVESVR, HI_EFULL)

/** Event ID Define */
typedef enum hiLIVESVR_EVENT_E
{
    HI_EVENT_LIVESVR_CLIENT_CONNECT = HI_APPCOMM_EVENT_ID(HI_APP_MOD_LIVESVR, 0), /**<client connection event */
    HI_EVENT_LIVESVR_CLIENT_DISCONNECT, /**<client disconnection event */
    HI_EVENT_LIVESVR_SERVER_ERROR, /**<server error event */
    HI_EVENT_LIVESVR_BUTT,
} HI_LIVESVR_EVENT_E;

/** liveserver callback: get video information */
typedef HI_S32 (*HI_LIVESVR_GET_VIDEO_INFO_CALLBACK_FN_PTR)(HI_HANDLE VencHdl, HI_MEDIA_VIDEOINFO_S *pstVideoInfo);

/** liveserver callback: get audio information */
typedef HI_S32 (*HI_LIVESVR_GET_AUDIO_INFO_CALLBACK_FN_PTR)(HI_HANDLE AencHdl, HI_MEDIA_AUDIOINFO_S *pstAudioInfo);

/** liveserver callback: start venc */
typedef HI_S32 (*HI_LIVESVR_VENC_START_CALLBACK_FN_PTR)(HI_HANDLE VencHdl, HI_S32 s32FrameCnt);

/** liveserver callback: stop venc */
typedef HI_S32 (*HI_LIVESVR_VENC_STOP_CALLBACK_FN_PTR)(HI_HANDLE VencHdl);

/** liveserver callback: start aenc */
typedef HI_S32 (*HI_LIVESVR_AENC_START_CALLBACK_FN_PTR)(HI_HANDLE AencHdl);

/** liveserver callback: stop aenc */
typedef HI_S32 (*HI_LIVESVR_AENC_STOP_CALLBACK_FN_PTR)(HI_HANDLE AencHdl);

typedef struct hiLIVESVR_MEDIA_OPERATE_S
{
    HI_LIVESVR_GET_VIDEO_INFO_CALLBACK_FN_PTR pfnGetVideoInfo; /**<get video information */
    HI_LIVESVR_GET_AUDIO_INFO_CALLBACK_FN_PTR pfnGetAudioInfo; /**<get audio information */
    HI_LIVESVR_VENC_START_CALLBACK_FN_PTR pfnVencStart; /**<start venc */
    HI_LIVESVR_VENC_STOP_CALLBACK_FN_PTR pfnVencStop; /**<stop venc */
    HI_LIVESVR_AENC_START_CALLBACK_FN_PTR pfnAencStart; /**<start aenc */
    HI_LIVESVR_AENC_STOP_CALLBACK_FN_PTR pfnAencStop; /**<stop aenc */
} HI_LIVESVR_MEDIA_OPERATE_S;

/**
* @brief    liveStreamServer init
* @param[in] s32MaxConnNum: max number of client connections, default 2, value[1,32]
* @param[in] pstMediaOps: media operate
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_Init(HI_S32 s32MaxConnNum, const HI_LIVESVR_MEDIA_OPERATE_S *pstMediaOps);

/**
* @brief    liveStreamServer deinit
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_Deinit(HI_VOID);

/**
* @brief    add stream
* @param[in] VencHdl: video data source handle, -1: not support
* @param[in] AencHdl: audio data source handle, -1: not support
* @param[in] pszStreamName: stream name, used in URL
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_AddStream(HI_HANDLE VencHdl,HI_HANDLE AencHdl, const HI_CHAR *pszStreamName);

/**
* @brief    remove stream
* @param[in] pszStreamName: stream name, used in URL
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_RemoveStream(const HI_CHAR *pszStreamName);

/**
* @brief    remove all stream
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_RemoveAllStream(HI_VOID);

/**
* @brief    register event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/02/07
*/
HI_S32 HI_LIVESVR_RegisterEvent(HI_VOID);

/**@}*/  /**<!-- ==== LIVESVR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_LIVESVR_H__ */


