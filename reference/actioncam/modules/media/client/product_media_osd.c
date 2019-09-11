/**
 * @file      product_media_osd.c
 * @brief     product media osd source
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
#include "hi_appcomm_msg_client.h"
#include "hi_appcomm_log.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 PDT_MEDIA_OSD_Init(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_SUCCESS;
    PDT_MEDIA_MSG_OSDINIT_PARAM_S stParam;
    memcpy(&stParam.stVideoAttr, pstVideoAttr, sizeof(HI_PDT_MEDIA_OSD_VIDEO_ATTR_S));
    memcpy(&stParam.stOsdCfg, pstOsdCfg, sizeof(HI_PDT_MEDIA_OSD_CFG_S));
    s32Ret = HI_MSG_SendSync(HI_MSG_MEDIA_INITOSD, (HI_VOID*)&stParam,
        sizeof(PDT_MEDIA_MSG_OSDINIT_PARAM_S), NULL, &s32Result, sizeof(HI_S32));
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && (HI_SUCCESS == s32Result), HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_StartTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_SUCCESS;
    s32Ret = HI_MSG_SendSync(HI_MSG_MEDIA_STARTTIMEOSD,
        (HI_VOID*)pstOsdInfo, sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S), NULL, &s32Result, sizeof(HI_S32));
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && (HI_SUCCESS == s32Result), HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_StopTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_SUCCESS;
    s32Ret = HI_MSG_SendSync(HI_MSG_MEDIA_STOPTIMEOSD,
        (HI_VOID*)pstOsdInfo, sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S), NULL, &s32Result, sizeof(HI_S32));
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && (HI_SUCCESS == s32Result), HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_Deinit(HI_VOID)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Result = HI_SUCCESS;
    s32Ret = HI_MSG_SendSync(HI_MSG_MEDIA_DEINITOSD, NULL, 0, NULL, &s32Result, sizeof(HI_S32));
    HI_APPCOMM_CHECK_EXPR((HI_SUCCESS == s32Ret) && (HI_SUCCESS == s32Result), HI_FAILURE);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

