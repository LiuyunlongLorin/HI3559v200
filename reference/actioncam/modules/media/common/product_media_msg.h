/**
 * @file      product_media_osd_msg.h
 * @brief     product media osd msg inferface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/4/09
 * @version   1.0

 */

#ifndef __PRODUCT_MEDIA_MSG_H__
#define __PRODUCT_MEDIA_MSG_H__

#include "hi_appcomm_msg.h"
#include "hi_product_media.h"
#include "hi_osd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#if defined(AMP_LINUX_HUAWEILITE)

typedef enum hiMSG_MEDIA_E
{
    HI_MSG_MEDIA_INITOSD = HI_APPCOMM_MSG_ID(HI_APP_MOD_MEDIA, 1),
    HI_MSG_MEDIA_STARTTIMEOSD,
    HI_MSG_MEDIA_STOPTIMEOSD,
    HI_MSG_MEDIA_DEINITOSD,
    HI_MSG_MEDIA_BUIT
} HI_MSG_MEDIA_E;

typedef struct tagPDT_MEDIA_MSG_OSDINIT_PARAM_S
{
    HI_PDT_MEDIA_OSD_VIDEO_ATTR_S stVideoAttr;
    HI_PDT_MEDIA_OSD_CFG_S stOsdCfg;
} PDT_MEDIA_MSG_OSDINIT_PARAM_S;

#endif

#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))

HI_S32 PDT_MEDIA_MSG_OsdInitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                     HI_MSG_PRIV_DATA_S* pstPrivData,
                                     HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData);

HI_S32 PDT_MEDIA_MSG_TimeOsdStartCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                     HI_MSG_PRIV_DATA_S* pstPrivData,
                                     HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData);

HI_S32 PDT_MEDIA_MSG_TimeOsdStopCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                     HI_MSG_PRIV_DATA_S* pstPrivData,
                                     HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData);

HI_S32 PDT_MEDIA_MSG_OsdDeinitCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                     HI_MSG_PRIV_DATA_S* pstPrivData,
                                     HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __PRODUCT_MEDIA_MSG_H__ */

