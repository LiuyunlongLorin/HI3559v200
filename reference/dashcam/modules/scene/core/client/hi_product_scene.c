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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_INIT, HI_NULL, 0, HI_NULL,HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_INIT Failed, %x\n\n, "NONE, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_SetSceneMode(const HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETSCENEMODE, pstSceneMode, sizeof(HI_PDT_SCENE_MODE_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETSCENEMODE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Pause(HI_BOOL bEnable)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_PAUSE, &bEnable, sizeof(HI_BOOL), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_PAUSE Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_GetSceneMode(HI_PDT_SCENE_MODE_S* pstSceneMode)
{
    HI_APPCOMM_CHECK_POINTER(pstSceneMode, HI_PDT_SCENE_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_GETSCENEMODE, HI_NULL, 0, HI_NULL, pstSceneMode, sizeof(HI_PDT_SCENE_MODE_S));
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_GETSCENEMODE Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETDIS, &stPipeEnabe, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETDIS Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETISO, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETISO Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETEXPTIME, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETEXPTIME Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETEV, &stPipeEV, sizeof(HI_PDT_SCENE_PIPE_EV_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETEV Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETWB, &stPipeNumber, sizeof(HI_PDT_SCENE_PIPE_NUMBER_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETWB Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_SETMETRY, &stPipeMetry, sizeof(HI_PDT_SCENE_PIPE_METRY_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_SETMETRY Failed\n\n"NONE);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_SCENE_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_DEINIT, HI_NULL, 0, HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_DEINIT Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_LOCKAE, &stPipeEnable, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_LOCKAE Failed\n\n"NONE);
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

    s32Ret = HI_MSG_SendSync(MSG_PDT_SCENE_LOCKAWB, &stPipeEnable, sizeof(HI_PDT_SCENE_PIPE_ENABLE_S), HI_NULL, HI_NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(YELLOW"HI_MSG_CLIENT_SendSync MSG_PDT_SCENE_LOCKAWB Failed\n\n"NONE);
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
