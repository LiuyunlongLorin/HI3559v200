/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_demuxer_err.h
* @brief     hiDEMUXER module header file
* @author    HiMobileCam middleware develop team
* @date      2016.11.07
*/

#ifndef __HI_DEMUXER_ERR_H__
#define __HI_DEMUXER_ERR_H__

#include "hi_error_def.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*´Ó0x40~0xFF*/
typedef enum hiAPP_DEMUXER_ERR_CODE_E
{
    APP_DEMUXER_ERR_OPEN_FILE = 0x40,               /**<open media file failed*/
    APP_DEMUXER_ERR_READ_PACKET  = 0x41,                   /**<read one packet from media file failed*/
    APP_DEMUXER_ERR_SEEK = 0x42,                      /**<seek in media file failed*/
    APP_DEMUXER_ERR_MEM_MALLOC = 0x43,                     /**<memery alloc failed*/
    APP_DEMUXER_ERR_PROBE = 0x44,            /**<probe the media info failed*/
    APP_DEMUXER_ERR_ACTION = 0x45,               /**<should not exec this action here*/
    APP_DEMUXER_ERR_INVALID_HANDLE = 0x46,                   /**< handle is invalid*/
    APP_DEMUXER_ERR_FREE_PACKET = 0x47,                   /**< free the packet failed*/
    APP_DEMUXER_ERR_SET_ATTR = 0x48,                   /**< set demuxer attr failed*/
    APP_DEMUXER_ERR_NOT_SUPPORT = 0x49,                   /**< current action not support */
    APP_DEMUXER_ERR_CLOSE_FILE = 0x50,              /**<close media file failed*/

    APP_DEMUXER_ERR_SKIP = 0x51,              /**<close media file failed*/
    APP_DEMUXER_BUTT = 0xFF
} APP_DEMUXER_ERR_CODE_E;


/*general error code*/
#define HI_ERR_DEMUXER_NULL_PTR                            HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_ERR_NULL_PTR)
#define HI_ERR_DEMUXER_ILLEGAL_PARAM                       HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_ERR_ILLEGAL_PARAM)
#define HI_ERR_DEMUXER_OPEN_FILE                         HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_OPEN_FILE)
#define HI_ERR_DEMUXER_READ_PACKET                  HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_READ_PACKET)
#define HI_ERR_DEMUXER_SEEK                                HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_SEEK)
#define HI_ERR_DEMUXER_MEM_MALLOC                          HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_MEM_MALLOC)
#define HI_ERR_DEMUXER_PROBE                     HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_PROBE)
#define HI_ERR_DEMUXER_ACTION                    HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_ACTION)
#define HI_ERR_DEMUXER_INVALID_HANDLE        HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_INVALID_HANDLE)
#define HI_ERR_DEMUXER_FREE_PACKET        HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_FREE_PACKET)
#define HI_ERR_DEMUXER_SET_ATTR        HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_SET_ATTR)
#define HI_ERR_DEMUXER_NOT_SUPPORT        HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_NOT_SUPPORT)
#define HI_ERR_DEMUXER_CLOSE_FILE                         HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_CLOSE_FILE)
#define HI_ERR_DEMUXER_SKIP_FILE                         HI_APP_DEF_ERR(HI_APPID_DEMUXER,APP_ERR_LEVEL_ERROR,APP_DEMUXER_ERR_SKIP)






#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_DEMUXER_ERR_H__ */
