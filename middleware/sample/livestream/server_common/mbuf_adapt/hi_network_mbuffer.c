/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_mbuffer.c
* @brief     livestream common mbuffer adapt src file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "securec.h"
#include "hi_server_log.h"
#include "hi_server_state_listener.h"
#include "hi_network_mbuffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define UNUSED(x) (void)x

typedef struct hi_MBUF_S
{
    HI_U32             u32PhyAddr;
    HI_VOID*           pBufHdr;
    HI_S32 s32VSrcChn;
    HI_S32 s32VEncChn;
    HI_S32 s32ASrcChn;
    HI_S32 s32AEncChn;
    pthread_mutex_t     mutex;
} HI_MBUF_S;

static HI_S32 NETWORK_MBuf_Alloc(HI_VOID* argv, HI_U32 u32BufSize, HI_VOID** ppVmAddr)
{
    if ((NULL == argv) || (NULL == ppVmAddr) || (0 == u32BufSize))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_Mbuf_Alloc invalid input param!\n");
        return HI_FAILURE;
    }

    *ppVmAddr = malloc(u32BufSize);
    if (NULL == *ppVmAddr)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "malloc mbuf memory failed\n");
        return HI_FAILURE;
    }

    if (HI_SUCCESS != memset_s(*ppVmAddr, u32BufSize, 0, u32BufSize))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "set *ppVmAddr fail \n");
        free(*ppVmAddr);
        *ppVmAddr = NULL;
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_Free(HI_VOID* argv, HI_VOID* pVmAddr)
{
    if ((NULL == argv) || (NULL == pVmAddr))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_Mbuf_Free invalid input param!\n");
        return HI_FAILURE;
    }

    free(pVmAddr);
    pVmAddr = NULL;

    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_Mcopy(HI_VOID* argv, HI_VOID* pDest, HI_VOID* pSrc, HI_U32 n)
{
    UNUSED(argv);
    size_t len = (size_t)n;
    if (HI_SUCCESS != memcpy_s(pDest, len, pSrc, (size_t)n))
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "copy pDest fail \n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_CreateLock(HI_VOID* argv)
{
    HI_NETWORK_BUFFER_INFO_S* pBufInfo = (HI_NETWORK_BUFFER_INFO_S*)argv;
    pthread_mutex_init(&(pBufInfo->mutex), NULL);
    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_DestroyLock(HI_VOID* argv)
{
    HI_NETWORK_BUFFER_INFO_S* pBufInfo = (HI_NETWORK_BUFFER_INFO_S*)argv;
    pthread_mutex_destroy(&(pBufInfo->mutex));

    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_Lock(HI_VOID* argv)
{
    HI_NETWORK_BUFFER_INFO_S* pBufInfo = (HI_NETWORK_BUFFER_INFO_S*)argv;
    (void)pthread_mutex_lock(&(pBufInfo->mutex));
    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBuf_UnLock(HI_VOID* argv)
{
    HI_NETWORK_BUFFER_INFO_S* pBufInfo = (HI_NETWORK_BUFFER_INFO_S*)argv;

    (void)pthread_mutex_unlock(&(pBufInfo->mutex));

    return HI_SUCCESS;
}

static HI_S32 NETWORK_MBufferEnable(HI_MW_PTR handle, HI_U8 u8payloadType)
{
    HI_S32 s32Rtn = HI_SUCCESS;

    s32Rtn = HI_MBUF_RegisterPayload(handle, u8payloadType);
    if (HI_SUCCESS != s32Rtn)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_RegisterPayload fail!,handle = %p u8payloadType:%d \n", handle, u8payloadType);
        return s32Rtn;
    }

    s32Rtn = HI_MBUF_SetRWEnable(handle, u8payloadType, HI_TRUE, HI_TRUE);
    if (HI_SUCCESS != s32Rtn)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_SetRWEnable fail, s32Rtn=%d!\n", s32Rtn);
        if (HI_SUCCESS != HI_MBUF_UnregisterPayload(handle, u8payloadType))
        {
            HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_UnregisterPayload fail\n");
        }

        return s32Rtn;
    }

    return s32Rtn;
}

HI_S32 HI_NETWORK_MBuf_OnCBFrmWrite(HI_MW_PTR hHandle,  HI_MBUF_SLICEINFO_S* pstFrameInfo)
{
    HI_S32 s32Ret = 0;

    if (pstFrameInfo)
    {
        s32Ret = HI_MBUF_WriteSlice(hHandle, pstFrameInfo);
        return s32Ret;
    }

    return HI_FAILURE;
}


HI_S32 HI_NETWORK_MBufferCreate(HI_VOID* argv, HI_MW_PTR* pBufHandle, HI_U32 u32BufSize, HI_S32 s32MaxPayload)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MBUF_CFG_S stMbufCfg;
    stMbufCfg.s32MaxPayload = s32MaxPayload;
    stMbufCfg.u32BufSize = u32BufSize;
    stMbufCfg.stMBufFunc.pfnAlloc = NETWORK_MBuf_Alloc;
    stMbufCfg.stMBufFunc.pfnFree = NETWORK_MBuf_Free;
    stMbufCfg.stMBufFunc.pfnMemcpy = NETWORK_MBuf_Mcopy;
    stMbufCfg.stMBufFunc.pfnCreateLock = NETWORK_MBuf_CreateLock;
    stMbufCfg.stMBufFunc.pfnDestroyLock = NETWORK_MBuf_DestroyLock;
    stMbufCfg.stMBufFunc.pfnLock = NETWORK_MBuf_Lock;
    stMbufCfg.stMBufFunc.pfnUnLock = NETWORK_MBuf_UnLock;

    if (stMbufCfg.u32BufSize == 0)
    {stMbufCfg.u32BufSize = DEFAULT_MBUF_SIZE;}

    s32Ret = HI_MBUF_GetBuffer(argv, &stMbufCfg, pBufHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_GetBuffer failed Ret: %d\n", s32Ret);
        return HI_FAILURE;
    }

    HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_INFO, "buffersize :%d\n", stMbufCfg.u32BufSize);
    return HI_SUCCESS;
}

HI_S32 HI_NETWORK_MBufferDestroy(HI_VOID* argv, HI_MW_PTR hHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MBUF_ReleaseBuffer(argv, hHandle);
    if (s32Ret != HI_SUCCESS)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_NETWORK_MBufferDestroy failed Ret: %d\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_NETWORK_MBufferRegister(HI_MW_PTR hHandle, HI_U8 u8MediaType)
{
    HI_S32 s32Rtn = HI_SUCCESS;

    s32Rtn = NETWORK_MBufferEnable(hHandle, u8MediaType);
    if (HI_SUCCESS != s32Rtn)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "MTBuf_Enable fail, s32Rtn=%d!\n", s32Rtn);
        return s32Rtn;
    }

    return HI_SUCCESS;
}
HI_S32 HI_NETWORK_MBufferUnRegister(HI_MW_PTR hHandle, HI_U8 u8MediaType)
{
    HI_S32 s32Rtn = HI_SUCCESS;

    s32Rtn = HI_MBUF_UnregisterPayload(hHandle, u8MediaType);
    if (HI_SUCCESS != s32Rtn)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_UnregisterPayload fail,media type:%d s32Rtn=%d!\n", u8MediaType, s32Rtn);
        return s32Rtn;
    }

    return HI_SUCCESS;
}


HI_S32 HI_NETWORK_MBufferRead(HI_MW_PTR hHandle, HI_VOID** paddr, HI_U32* plen, HI_U64* ppts, BUF_DATATYPE_E* type, HI_BOOL* pbKeyFlag)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_U32 i = 0;
    HI_MBUF_SLICEINFO_S stSliceInfo;
    stSliceInfo.bIsKeyFrame = HI_FALSE;
    stSliceInfo.u32FrameLen = 0;
    stSliceInfo.u32HeadLen = 0;
    stSliceInfo.u32Seq = 0;
    stSliceInfo.u32SliceCnt = 0;
    stSliceInfo.u32SliceNum = 0;
    stSliceInfo.u32TailLen = 0;
    stSliceInfo.u32SliceLen = 0;
    stSliceInfo.u64Pts = 0;
    stSliceInfo.u8PayloadType = 0;
    stSliceInfo.u8SliceType = 0;
    stSliceInfo.u32BlockCount = 0;
    for (i = 0; i < HI_MBUF_FRAME_MAX_BLOCK; i++)
    {
        stSliceInfo.pAddr[i] = NULL;
        stSliceInfo.au32BlockLen[i] = 0;
    }

    *plen = 0;

    s32Ret = HI_MBUF_ReadSlice(hHandle, 255, &stSliceInfo);
    if (HI_SUCCESS == s32Ret)
    {
        *paddr = stSliceInfo.pAddr[0];
        *plen = stSliceInfo.u32SliceLen;
        *ppts = stSliceInfo.u64Pts;
        *pbKeyFlag = stSliceInfo.bIsKeyFrame;

        switch (stSliceInfo.u8PayloadType)
        {
            case HI_BUF_PAYLOAD_H264:

            case HI_BUF_PAYLOAD_H265:
                *type = BUF_DATATYPE_VIDEO;
                break;

            case HI_BUF_PAYLOAD_G711Mu:

            case HI_BUF_PAYLOAD_G711A:

            case HI_BUF_PAYLOAD_G726:

            case HI_BUF_PAYLOAD_AAC:
                *type = BUF_DATATYPE_AUDIO;
                break;

            default:
                *type = BUF_DATATYPE_DATA;
                break;
        }
    }

    return s32Ret;
}

HI_S32 HI_NETWORK_MBufferSet(HI_MW_PTR hHandle)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = HI_MBUF_Forward(hHandle, 255, 1);
    if (HI_SUCCESS != s32Ret)
    {
        HI_SERVER_LOG_Printf(MODULE_NAME_LIVESTREAM, HI_SERVER_LOG_LEVEL_ERR, "HI_MBUF_Forward fail, s32Ret=%d!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_S32 HI_NETWORK_MBufferGetPts(HI_MW_PTR hHandle, HI_U8 u8PayLoadType, HI_U64* ppts)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32Count = 0;
    HI_U32 i = 0;
    HI_MBUF_SLICEINFO_S stSliceInfo;
    stSliceInfo.bIsKeyFrame = HI_FALSE;
    stSliceInfo.u32FrameLen = 0;
    stSliceInfo.u32HeadLen = 0;
    stSliceInfo.u32Seq = 0;
    stSliceInfo.u32SliceCnt = 0;
    stSliceInfo.u32SliceNum = 0;
    stSliceInfo.u32TailLen = 0;
    stSliceInfo.u32SliceLen = 0;
    stSliceInfo.u64Pts = 0;
    stSliceInfo.u8PayloadType = 0;
    stSliceInfo.u8SliceType = 0;
    stSliceInfo.u32BlockCount = 0;
    for (i = 0; i < HI_MBUF_FRAME_MAX_BLOCK; i++)
    {
        stSliceInfo.pAddr[i] = NULL;
        stSliceInfo.au32BlockLen[i] = 0;
    }

    while (HI_SUCCESS != s32Ret && s32Count < MAX_WAIT_COUNT)
    {
        s32Ret = HI_MBUF_ReadSlice(hHandle, u8PayLoadType, &stSliceInfo);
        if (HI_SUCCESS == s32Ret)
        {
            *ppts = stSliceInfo.u64Pts;
            break;
        }
        hi_usleep(10000);
        s32Count++;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
