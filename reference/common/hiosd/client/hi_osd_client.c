/**
 * @file    hi_osd_client.c
 * @brief   osd module client interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */
#include "hi_osd.h"
#include "hi_appcomm_msg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "OSD"

/**
 * @brief    osd module initialization, eg. create time osd update task.
 * @param[in] pstFonts:osd fonts lib
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Init(const HI_OSD_FONTS_S* pstFonts)
{
    MLOGD("OSD init on the server side,client do noting\n");
    return HI_SUCCESS;
}

/**
 * @brief    osd module de-initialization, eg. destroy time osd update task, release resource.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Deinit(HI_VOID)
{
    HI_S32 s32Idx = 0;
    for (s32Idx = 0; s32Idx < HI_OSD_MAX_CNT; ++s32Idx)
    {
        /* Stop Osd */
        HI_OSD_Stop(s32Idx);
    }
    MLOGD("OSD deinit on the server side,just stop on client\n");
    return HI_SUCCESS;
}

/**
 * @brief    set osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[in] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_SetAttr(HI_S32 s32OsdIdx, const HI_OSD_ATTR_S* pstAttr)
{
    /* Check Input Param */
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_SET_ATTR);
    HI_MSG_PRIV_DATA_S stPrivData;
    stPrivData.as32PrivData[0]=OSD_CMD_SET_ATTR;
    stPrivData.as32PrivData[1]=s32OsdIdx;
    return HI_MSG_SendSync(s32MsgID, pstAttr, sizeof(HI_OSD_ATTR_S), &stPrivData, NULL, 0);
}

/**
 * @brief    get osd attribute.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @param[out] pstAttr:osd configure attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_GetAttr(HI_S32 s32OsdIdx, HI_OSD_ATTR_S* pstAttr)
{
    /* Check Input Param */
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_GET_ATTR);
    HI_MSG_PRIV_DATA_S stPrivData;
    stPrivData.as32PrivData[0]=OSD_CMD_GET_ATTR;
    stPrivData.as32PrivData[1]=s32OsdIdx;
    return HI_MSG_SendSync(s32MsgID, NULL, 0, &stPrivData, pstAttr, sizeof(HI_OSD_ATTR_S));
}

/**
 * @brief    start osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Start(HI_S32 s32OsdIdx)
{
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_START);
    HI_MSG_PRIV_DATA_S stPrivData;
    stPrivData.as32PrivData[0]=OSD_CMD_START;
    stPrivData.as32PrivData[1]=s32OsdIdx;
    return HI_MSG_SendSync(s32MsgID, NULL, 0, &stPrivData, NULL, 0);
}

/**
 * @brief    stop osd by index.
 * @param[in] s32OsdIdx:osd index, range[0,HI_OSD_MAX_CNT)
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/18
 */
HI_S32 HI_OSD_Stop(HI_S32 s32OsdIdx)
{
    /* Check Input Param */
    HI_APPCOMM_CHECK_EXPR(0 <= s32OsdIdx, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_OSD_MAX_CNT > s32OsdIdx, HI_EINVAL);
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_STOP);
    HI_MSG_PRIV_DATA_S stPrivData;
    stPrivData.as32PrivData[0]=OSD_CMD_STOP;
    stPrivData.as32PrivData[1]=s32OsdIdx;
    return HI_MSG_SendSync(s32MsgID, NULL, 0, &stPrivData, NULL, 0);
}

HI_S32 HI_OSD_Batch(HI_S32 s32Batch, HI_BOOL bShow)
{
    HI_MSG_PRIV_DATA_S stPrivData;
    stPrivData.as32PrivData[0]=OSD_CMD_BATCH;
    stPrivData.as32PrivData[1]=s32Batch;
    stPrivData.as32PrivData[2]=bShow;
    HI_S32 s32MsgID = HI_APPCOMM_MSG_ID(HI_APP_MOD_OSD, OSD_CMD_BATCH);
    return HI_MSG_SendASync(s32MsgID, NULL, 0, &stPrivData);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

