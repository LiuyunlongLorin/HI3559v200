/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_sys_client.c
 * @brief   NDK sys client functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include "mpi_sys.h"

#include "hi_mapi_comm_define.h"
#include "msg_wrapper.h"
#include "msg_sys.h"
#include "msg_server.h"
#include "msg_define.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_log.h"
#include "mapi_vcap_inner.h"
#include "mapi_vproc_inner.h"
#include "mapi_venc_inner.h"
#include "mapi_aenc_inner.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STRNAME_LEN 128

static HI_BOOL g_s32SysInit = HI_FALSE;
static HI_BOOL g_s32MediaInit = HI_FALSE;

pthread_mutex_t g_mediaInitLock;

HI_S32 HI_MAPI_Sys_Init()
{
    HI_S32 s32Ret;

    s32Ret = MAPI_Media_MSG_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "Media_MSG_Init fail\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    g_s32SysInit = HI_TRUE;
    pthread_mutex_init(&g_mediaInitLock, NULL);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_Sys_EnableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr)
{
    HI_S32 s32Ret;

    if (pstDspAttr == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pstDspAttr Null pointer!\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_ENABLE_DSP,
                           (HI_VOID *)pstDspAttr, sizeof(HI_MAPI_Sys_DspAttr_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_ENABLE_DSP fail, ret:%x \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 HI_MAPI_Sys_DisableDsp(const HI_MAPI_Sys_DspAttr_S *pstDspAttr)
{
    HI_S32 s32Ret;

    if (pstDspAttr == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pstDspAttr Null pointer!\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_DISABLE_DSP,
                           (HI_VOID *)pstDspAttr, sizeof(HI_MAPI_Sys_DspAttr_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_DISABLE_DSP fail, ret:%x \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 HI_MAPI_Sys_Deinit()
{
    HI_S32 s32Ret;

    g_s32SysInit = HI_FALSE;

    s32Ret = MAPI_Media_MSG_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "Media_MSG_Deinit fail\n");
        return HI_MAPI_SYS_EBUSY;
    }

    pthread_mutex_destroy(&g_mediaInitLock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_Sys_GetVprocVencWrapBufferLine(VPSS_VENC_WRAP_PARAM_S *pWrapParam, HI_U32 *pu32BufLine)
{
    MAPI_UNUSED(pWrapParam);
    MAPI_UNUSED(pu32BufLine);

    MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "client don't support GetVprocVencWrapBufferLine!!\n");
    return HI_MAPI_SYS_ENOT_PERM;
}

HI_S32 HI_MAPI_Media_Init(const HI_MAPI_MEDIA_ATTR_S *pstMediaAttr)
{
    HI_S32 s32Ret;

    if (pstMediaAttr == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pstMediaAttr Null pointer!\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    MUTEX_LOCK(g_mediaInitLock);

    if (g_s32MediaInit == HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "media has already been inited\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_SUCCESS;
    }

    s32Ret = MAPI_VCAP_Client_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VCAP_Client_Init fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_VPROC_Client_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VPROC_Client_Init fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_Client_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VEnc_Client_Init fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_AENC_Client_Init();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_AEnc_Client_Init fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_INIT,
                           (HI_VOID *)pstMediaAttr, sizeof(HI_MAPI_MEDIA_ATTR_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_INIT fail\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    g_s32MediaInit = HI_TRUE;

    MUTEX_UNLOCK(g_mediaInitLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_Media_Deinit()
{
    HI_S32 s32Ret;

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    MUTEX_LOCK(g_mediaInitLock);

    if (g_s32MediaInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "media has already been deinited\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_SUCCESS;
    }

    s32Ret = MAPI_VCAP_Client_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VCAP_Client_Deinit fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_VPROC_Client_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VPROC_Client_Deinit fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_VENC_Client_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_VEnc_Client_DeInit fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_AENC_Client_Deinit();

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MAPI_AEnc_Client_DeInit fail s32Ret:%x\n", s32Ret);
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_DEINIT, NULL, 0, HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_DEINIT fail\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    g_s32MediaInit = HI_FALSE;

    MUTEX_UNLOCK(g_mediaInitLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_Sys_SetDatafifoNodeNum(HI_MAPI_DATAFIFO_CONFIG_S *pstDatafifoCfg)
{
    if (pstDatafifoCfg == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pstDatafifoCFG Null pointer!\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    MUTEX_LOCK(g_mediaInitLock);

    if (g_s32MediaInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "media not init yet\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_MAPI_SYS_ENOTREADY;
    }

    MAPI_VENC_Client_SetDatafifoNodeNum(pstDatafifoCfg->u32VencDatafifeNodeNum);

    MUTEX_UNLOCK(g_mediaInitLock);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_AllocBuffer(HI_U64 *pu64PhyAddr, HI_VOID **ppVirtAddr,
                           HI_U32 u32Len, const HI_CHAR *pstrName)
{
    HI_S32 s32Ret;
    MSG_ALLOCBUFFER_S stAllocBuffer;
    HI_VOID *pDataVirtAddr;

    if (u32Len <= 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "Buffer len can not be less than zero\n");
        return HI_MAPI_SYS_EILLEGAL_PARAM;
    }

    if (pu64PhyAddr == NULL || pstrName == NULL || ppVirtAddr == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pu32PhyAddr or pstrName or ppVirtAddr is NULL\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    MUTEX_LOCK(g_mediaInitLock);

    if (g_s32MediaInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "media not init yet\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_MAPI_SYS_ENOTREADY;
    }

    stAllocBuffer.u32Len = u32Len;
    memset(stAllocBuffer.aszstrName, 0, STRNAME_LEN);

    if (STRNAME_LEN >= strlen(pstrName)) {
        memcpy(stAllocBuffer.aszstrName, pstrName, strlen(pstrName));
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pstrName is too long!\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_MAPI_SYS_EILLEGAL_PARAM;
    }

    stAllocBuffer.aszstrName[strlen(pstrName) + 1] = '\0';

    MAPI_INFO_TRACE(HI_MAPI_MOD_SYS, "buffer len:%d,name:%s\n", stAllocBuffer.u32Len, stAllocBuffer.aszstrName);

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_ALLOC_BUFFER,
                           &stAllocBuffer, sizeof(MSG_ALLOCBUFFER_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_ALLOC_BUFFER fail\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    *pu64PhyAddr = stAllocBuffer.u64PhyAddr;

    pDataVirtAddr = HI_MPI_SYS_Mmap(stAllocBuffer.u64PhyAddr, stAllocBuffer.u32Len);

    if ((HI_VOID *)-1 == pDataVirtAddr) {
        perror("HI_MAPI_AllocBuffer mmap error\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_MAPI_SYS_EILLEGAL_PARAM;
    }

    *ppVirtAddr = pDataVirtAddr;

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_ALLOC_BUFFER fail\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_mediaInitLock);
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_FreeBuffer(HI_U64 u64PhyAddr, HI_VOID *pVirtAddr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    MSG_FREEBUFFER_S stFreeBuffer;

    if (pVirtAddr == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "pVirtAddr is NULL\n");
        return HI_MAPI_SYS_ENULL_PTR;
    }

    if (g_s32SysInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "sys not init yet\n");
        return HI_MAPI_SYS_ENOTREADY;
    }

    MUTEX_LOCK(g_mediaInitLock);

    if (g_s32MediaInit == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "media not init yet\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return HI_MAPI_SYS_ENOTREADY;
    }

    s32Ret = HI_MPI_SYS_Munmap(pVirtAddr, u32Len);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "munmap fail VirAddr:0x%p\n", pVirtAddr);
    }

    stFreeBuffer.pVirtAddr = &u64PhyAddr;
    stFreeBuffer.u64PhyAddr = u64PhyAddr;
    stFreeBuffer.u32Len = u32Len;

    s32Ret = MAPI_SendSync(MODFD(HI_MAPI_MOD_SYS, 0, 0), MSG_CMD_MEDIA_SYS_FREE_BUFFER,
                           &stFreeBuffer, sizeof(MSG_FREEBUFFER_S), HI_NULL);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_SYS, "MSG_CMD_MEDIA_SYS_FREE_BUFFER fail\n");
        MUTEX_UNLOCK(g_mediaInitLock);
        return s32Ret;
    }

    MUTEX_UNLOCK(g_mediaInitLock);
    return HI_SUCCESS;
}

HI_BOOL MAPI_GetMediaInitStatus(HI_VOID)
{
    MUTEX_LOCK(g_mediaInitLock);
    HI_BOOL bFlag = g_s32MediaInit;
    MUTEX_UNLOCK(g_mediaInitLock);
    return bFlag;
}

HI_BOOL MAPI_GetSysInitStatus(HI_VOID)
{
    return g_s32SysInit;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
