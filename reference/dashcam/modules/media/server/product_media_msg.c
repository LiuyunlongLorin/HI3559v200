/**
 * @file      product_media_msg.c
 * @brief     product media msg source
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
#include "hi_mapi_sys.h"
#include "product_media_inner.h"
#include "product_media_msg.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__))

static HI_S32 PDT_MEDIA_SetCrop_Callback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
        HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    return HI_PDT_MEDIA_DispWindowCrop((HI_PDT_MEDIA_DISP_WND_CROP_CFG_S*)pvRequest);
}

static HI_S32 PDT_MEDIA_StartInfoOsd_Callback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
        HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    return PDT_MEDIA_StartInfoOsd();
}

static HI_S32 PDT_MEDIA_StopInfoOsd_Callback(HI_S32 s32MsgID, const HI_VOID* pvRequest, HI_U32 u32ReqLen, HI_MSG_PRIV_DATA_S* pstPrivData,
        HI_VOID** ppvResponse, HI_U32* pu32RespLen, HI_VOID* pvUserData)
{
    return PDT_MEDIA_StopInfoOsd();
}

HI_S32 PDT_MEDIA_MSG_RegisterCallback(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_SETCROP, PDT_MEDIA_SetCrop_Callback, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_INFO_OSD_START, PDT_MEDIA_StartInfoOsd_Callback, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret |= HI_MSG_RegisterMsgProc(HI_MSG_MEDIA_INFO_OSD_STOP, PDT_MEDIA_StopInfoOsd_Callback, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

