/**
 * @file      product_media_osd_msg.c
 * @brief     product media osd msg source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 * @version   1.0

 */

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "hi_product_media.h"
#include "product_media_inner.h"
#include "product_media_msg.h"
#include "hi_appcomm_msg_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))

HI_S32 PDT_MEDIA_MSG_InitOsdCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                     HI_MSG_PRIV_DATA_S* pstPrivData,
                                     HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    if((NULL == pvRequest) || sizeof(PDT_MEDIA_MSG_OSDINIT_PARAM_S) != u32ReqLen)
    {
        MLOGE("ReqLen[%u] ParamSize[%lu] not equal\n\n", u32ReqLen,
            (HI_UL)sizeof(PDT_MEDIA_MSG_OSDINIT_PARAM_S));
        return HI_FAILURE;
    }
    PDT_MEDIA_MSG_OSDINIT_PARAM_S* pstParam = (PDT_MEDIA_MSG_OSDINIT_PARAM_S*)pvRequest;

    return PDT_MEDIA_OSD_Init(&pstParam->stVideoAttr, &pstParam->stOsdCfg);
}

HI_S32 PDT_MEDIA_MSG_TimeOsdStartCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                          HI_MSG_PRIV_DATA_S* pstPrivData,
                                          HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    if((NULL == pvRequest) || sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S) != u32ReqLen)
    {
        MLOGE("ReqLen[%u] ParamSize[%lu] not equal\n\n", u32ReqLen,
            (HI_UL)sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S));
        return HI_FAILURE;
    }
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstParam = (HI_PDT_MEDIA_OSD_VIDEO_INFO_S*)pvRequest;

    return PDT_MEDIA_OSD_StartTimeOsd(pstParam);
}

HI_S32 PDT_MEDIA_MSG_TimeOsdStopCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                         HI_MSG_PRIV_DATA_S* pstPrivData,
                                         HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    if((NULL == pvRequest) || sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S) != u32ReqLen)
    {
        MLOGE("ReqLen[%u] ParamSize[%lu] not equal\n\n", u32ReqLen,
            (HI_UL)sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S));
        return HI_FAILURE;
    }
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstParam = (HI_PDT_MEDIA_OSD_VIDEO_INFO_S*)pvRequest;

    return PDT_MEDIA_OSD_StopTimeOsd(pstParam);
}

HI_S32 PDT_MEDIA_MSG_DeinitOsdCallback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen,
                                       HI_MSG_PRIV_DATA_S* pstPrivData,
                                       HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    return PDT_MEDIA_OSD_Deinit();
}

HI_S32 PDT_MEDIA_MSG_RegisterCallback(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_INITOSD, PDT_MEDIA_MSG_InitOsdCallback, NULL);
    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_STARTTIMEOSD, PDT_MEDIA_MSG_TimeOsdStartCallback, NULL);
    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_STOPTIMEOSD, PDT_MEDIA_MSG_TimeOsdStopCallback, NULL);
    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_DEINITOSD, PDT_MEDIA_MSG_DeinitOsdCallback, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

