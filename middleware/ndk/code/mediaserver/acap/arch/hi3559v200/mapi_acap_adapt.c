/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap_adapt.c
 * @brief   NDK acap server arch functions(hi3559v200)
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */

#include "mapi_comm_inner.h"
#include "mapi_acap_inner.h"
#include "mpi_audio.h"
#include "hi_mapi_acap_define.h"
#include "mapi_acap_adapt.h"

#ifndef __HuaweiLite__
#include <dlfcn.h>
#else
#include <los_ld_elflib.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HIVE_RES_LIB_NAME "libhive_RES.so"
typedef HI_VOID *(*pfn_HI_Resampler_Create_Callback)(HI_S32 s32Inrate, HI_S32 s32Outrate, HI_S32 s32Chans);
typedef HI_S32 (*pfn_HI_Resampler_Process_Callback)(HI_VOID *inst, HI_S16 *s16Inbuf, HI_S32 s32Insamps,
                                                    HI_S16 *s16Outbuf);
typedef HI_VOID (*pfn_HI_Resampler_Destroy_Callback)(HI_VOID *inst);

typedef struct{
    HI_VOID *pLibHandle;
    pfn_HI_Resampler_Create_Callback pHI_Resampler_Create;
    pfn_HI_Resampler_Process_Callback pHI_Resampler_Process;
    pfn_HI_Resampler_Destroy_Callback pHI_Resampler_Destroy;
} MAPI_RESAMPLE_FUN_S;

static MAPI_RESAMPLE_FUN_S g_stResampleFun = {0};

static HI_S32 MAPI_Dlpath(HI_CHAR *pChLibPath)
{
#ifndef __HuaweiLite__
#else
    if (LOS_PathAdd(pChLibPath) != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "add path %s failed!\n", pChLibPath);
        return HI_FAILURE;
    }
#endif

    return HI_SUCCESS;
}

static HI_S32 MAPI_Dlopen(HI_VOID **pLibhandle, HI_CHAR *pChLibName)
{
    if (pLibhandle == HI_NULL || pChLibName == HI_NULL) {
        return HI_FAILURE;
    }

    *pLibhandle = HI_NULL;
#ifndef __HuaweiLite__
    *pLibhandle = dlopen(pChLibName, RTLD_LAZY | RTLD_LOCAL);
#else
    *pLibhandle = LOS_SoLoad(pChLibName);
#endif

    if (*pLibhandle == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "dlopen %s failed!\n", pChLibName);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_Dlsym(HI_VOID **pFunchandle, HI_VOID *Libhandle, HI_CHAR *pChFuncName)
{
    if (Libhandle == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "LibHandle is empty!");
        return HI_FAILURE;
    }

    *pFunchandle = HI_NULL;
#ifndef __HuaweiLite__
    *pFunchandle = dlsym(Libhandle, pChFuncName);
#else
    *pFunchandle = LOS_FindSymByName(Libhandle, pChFuncName);
#endif

    if (*pFunchandle == HI_NULL) {
#ifndef __HuaweiLite__
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "dlsym %s fail,error msg is %s!\n", pChFuncName, dlerror());
#else
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "dlsym %s fail!\n", pChFuncName);
#endif
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_Dlclose(HI_VOID *LibHandle)
{
    if (LibHandle == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "LibHandle is NULL!\n");
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    dlclose(LibHandle);
#else
    LOS_ModuleUnload(LibHandle);
#endif

    return HI_SUCCESS;
}

static HI_VOID MAPI_ACAP_DeInitResampleFun(HI_VOID)
{
    if (g_stResampleFun.pLibHandle != HI_NULL) {
        MAPI_Dlclose(g_stResampleFun.pLibHandle);
        memset(&g_stResampleFun, 0, sizeof(MAPI_RESAMPLE_FUN_S));
    }

    return;
}

static HI_S32 MAPI_ACAP_InitResampleFun(HI_VOID)
{
    HI_S32 s32Ret;
    MAPI_RESAMPLE_FUN_S stResampleFun;
    memset(&stResampleFun, 0, sizeof(MAPI_RESAMPLE_FUN_S));

    MAPI_ACAP_DeInitResampleFun();
    s32Ret = MAPI_Dlopen(&(stResampleFun.pLibHandle), HIVE_RES_LIB_NAME);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "dlopen lib %s error!\n", HIVE_RES_LIB_NAME);
        return HI_FAILURE;
    }

    s32Ret = MAPI_Dlsym((HI_VOID **)&(stResampleFun.pHI_Resampler_Create), stResampleFun.pLibHandle,
                        "HI_Resampler_Create");
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "find HI_Resampler_Create symbol error!\n");
        return HI_FAILURE;
    }

    s32Ret = MAPI_Dlsym((HI_VOID **)&(stResampleFun.pHI_Resampler_Process), stResampleFun.pLibHandle,
                        "HI_Resampler_Process");
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "find HI_Resampler_Process symbol error!\n");
        return HI_FAILURE;
    }

    s32Ret = MAPI_Dlsym((HI_VOID **)&(stResampleFun.pHI_Resampler_Destroy), stResampleFun.pLibHandle,
                        "HI_Resampler_Destroy");
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "find HI_Resampler_Destroy symbol error!\n");
        return HI_FAILURE;
    }

    memcpy(&g_stResampleFun, &stResampleFun, sizeof(MAPI_RESAMPLE_FUN_S));

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_ACAP_RegisterVQEModule(HI_VOID)
{
    return HI_SUCCESS;
}

HI_VOID *HAL_MAPI_Resampler_Create(HI_S32 s32Inrate, HI_S32 s32Outrate, HI_S32 s32Chans)
{
    HI_S32 s32Ret;

    if (g_stResampleFun.pLibHandle == NULL) {
        s32Ret = MAPI_ACAP_InitResampleFun();
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MAPI_ACAP_InitResampleFun Failed!\n");
            return NULL;
        }
    }

    return g_stResampleFun.pHI_Resampler_Create(s32Inrate, s32Outrate, s32Chans);
}

HI_VOID HAL_MAPI_Resampler_Destroy(HI_VOID *inst)
{
    if (g_stResampleFun.pLibHandle != NULL) {
        g_stResampleFun.pHI_Resampler_Destroy(inst);
        MAPI_ACAP_DeInitResampleFun();
    }

    return;
}

HI_S32 HAL_MAPI_Resampler_Process(HI_VOID *inst, HI_S16 *s16Inbuf, HI_S32 s32Insamps, HI_S16 *s16Outbuf)
{
    if (g_stResampleFun.pLibHandle == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "it does not init resample fun!\n");
        return HI_FAILURE;
    }

    return g_stResampleFun.pHI_Resampler_Process(inst, s16Inbuf, s32Insamps, s16Outbuf);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
