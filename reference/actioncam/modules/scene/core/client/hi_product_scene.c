/**
 * @file    hi_product_scene.c
 * @brief   photo picture.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */
#include <string.h>
#include "hi_product_scene.h"
#include "hi_appcomm_msg_client.h"
#include "hi_product_scene_msg_client.h"
#include "hi_ipcmsg.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */
/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "SCENE_CLIENT"


HI_S32 HI_PDT_SCENE_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_INIT, HI_NULL, 0, NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_INIT Failed, %x\n\n, "NONE, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetSceneMode(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETSCENEMODE, pstSceneMode, sizeof(HI_PDT_SCENE_MODE_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETSCENEMODE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Pause(HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_PAUSE, &bEnable, sizeof(HI_BOOL), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_PAUSE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetSceneMode(HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETSCENEMODE, HI_NULL, 0, NULL,
        pstSceneMode, sizeof(HI_PDT_SCENE_MODE_S));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_GETSCENEMODE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef CFG_POST_PROCESS
HI_S32 HI_PDT_SCENE_GetDetectPostProcessInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_POSTPROCESS_INFO_S* pstDetectInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstDetectInfo, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETPOSTPROCESSINFO, &VcapPipeHdl, sizeof(HI_HANDLE), NULL,
        pstDetectInfo, sizeof(HI_PDT_SCENE_POSTPROCESS_INFO_S));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_GETPOSTPROCESSINFO Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}
#endif

HI_S32 HI_PDT_SCENE_GetDetectLongExpInfo(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enSnapLongExpType, HI_PDT_SCENE_LONGEXP_PARAM_S* pstLongExpParam)
{
    HI_APPCOMM_CHECK_POINTER(pstLongExpParam, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = enSnapLongExpType;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETLONGEXPINFO, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL,
        pstLongExpParam, sizeof(HI_PDT_SCENE_LONGEXP_PARAM_S));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_GETLONGEXPINFO Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_RefreshPhotoPipe(HI_HANDLE VcapPipeHdl, HI_U8 u8PipeParamIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PHOTOPIPE_INFO_S stPhotoPipeInfo;
    stPhotoPipeInfo.VcapPipeHdl = VcapPipeHdl;
    stPhotoPipeInfo.u8PipeParamIndex = u8PipeParamIndex;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_REFRESHPHOTOPIPE, &stPhotoPipeInfo, sizeof(HI_PDT_SCENE_PHOTOPIPE_INFO_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_REFRESHPHOTOPIPE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetLongExpGain(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us, HI_U32* pu32SysGain)
{
    HI_APPCOMM_CHECK_POINTER(pu32SysGain, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = u32ExpTime_us;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETLONGEXPGAIN, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL, pu32SysGain, sizeof(HI_U32));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_GETLONGEXPGAIN Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetLongExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO, HI_U32* pu32ExpTime_us)
{
    HI_APPCOMM_CHECK_POINTER(pu32ExpTime_us, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = u32ISO;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETLONGEXPTIME, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL, pu32ExpTime_us, sizeof(HI_U32));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_GETLONGEXPTIME Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_S32 HI_PDT_SCENE_SetDIS(HI_HANDLE VcapPipeHdl, HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_ENABLE_S stPipeEnabe;
    stPipeEnabe.VcapPipeHdl = VcapPipeHdl;
    stPipeEnabe.bEnable = bEnable;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETDIS, &stPipeEnabe, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETDIS Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetISO(HI_HANDLE VcapPipeHdl, HI_U32 u32ISO)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = u32ISO;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETISO, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETISO Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetExpTime(HI_HANDLE VcapPipeHdl, HI_U32 u32ExpTime_us)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = u32ExpTime_us;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETEXPTIME, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETEXPTIME Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetEV(HI_HANDLE VcapPipeHdl, HI_PDT_SCENE_EV_E enEV)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_EV_S stPipeEV;
    stPipeEV.VcapPipeHdl = VcapPipeHdl;
    stPipeEV.enEV = enEV;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETEV, &stPipeEV, sizeof(HI_PDT_SCENE_PIPE_EV_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETEV Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetWB(HI_HANDLE VcapPipeHdl, HI_U32 u32WB)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_NUMBER_S stPipeNumber;
    stPipeNumber.VcapPipeHdl = VcapPipeHdl;
    stPipeNumber.u32Number = u32WB;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETWB, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETWB Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetMetry(HI_HANDLE VcapPipeHdl, const HI_PDT_SCENE_METRY_S* pstMetry)
{
    HI_APPCOMM_CHECK_POINTER(pstMetry, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_SCENE_PIPE_METRY_S stPipeMetry;
    stPipeMetry.VcapPipeHdl = VcapPipeHdl;
    memcpy(&stPipeMetry.stMetry, pstMetry, sizeof(HI_PDT_SCENE_METRY_S));

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETMETRY, &stPipeMetry, sizeof(HI_PDT_SCENE_PIPE_METRY_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_SETMETRY Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_DEINIT, HI_NULL, 0, NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_DEINIT Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef CFG_POST_PROCESS
HI_S32 HI_PDT_SCENE_LockAE(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_ENABLE_S stPipeEnable;
    stPipeEnable.VcapPipeHdl = VcapPipeHdl;
    stPipeEnable.bEnable = bLock;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_LOCKAE, &stPipeEnable, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_LOCKAE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_LockAWB(HI_HANDLE VcapPipeHdl, const HI_BOOL bLock)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_PIPE_ENABLE_S stPipeEnable;
    stPipeEnable.VcapPipeHdl = VcapPipeHdl;
    stPipeEnable.bEnable = bLock;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_LOCKAWB, &stPipeEnable, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_SendSync MSG_PDT_SCENE_LOCKAWB Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

#endif

HI_S32 HI_PDT_SCENE_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = HI_EVTHUB_Register(HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_EVENT_PDT_SCENE_FRAMERATE_CHANGE register fail \n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== SCENE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
