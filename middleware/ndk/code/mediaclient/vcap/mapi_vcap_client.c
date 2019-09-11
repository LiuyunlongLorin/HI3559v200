/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_client.c
 * @brief   hdmi client module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mpi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "hi_ipcmsg.h"
#include "hi_comm_ipcmsg.h"
#include "hi_mapi_sys.h"
#include "msg_vcap.h"
#include "msg_define.h"
#include "msg_wrapper.h"
#include <errno.h>
#include "mapi_comm_inner.h"

#define HI_VCAP_DUMP_RAW_TIMEOUT 60000  // (1 min)
#define HI_VCAP_DUMP_SCENE_NUM   2

/* raw info id */
#define NORMAL_RAW_INFO_ID 0
#define BNR_RAW_INFO_ID    1

VCAP_DUMP_RAWDATA_INFO_S g_astVcapDumpRawInfo[HI_VCAP_DUMP_SCENE_NUM][HI_MAPI_VCAP_MAX_PIPE_NUM];

HI_VOID MAPI_VCAP_Client_ResetStatus(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 j = 0;

    for (i = 0; i < HI_VCAP_DUMP_SCENE_NUM; i++) {
        for (j = 0; j < HI_MAPI_VCAP_MAX_PIPE_NUM; j++) {
            g_astVcapDumpRawInfo[i][j].bBNRRaw = HI_FALSE;
            g_astVcapDumpRawInfo[i][j].g_pthRawDump = -1;
            g_astVcapDumpRawInfo[i][j].u32GroupCount = 0;
            g_astVcapDumpRawInfo[i][j].stVCapRawData.pfn_VCAP_RawDataProc = HI_NULL;
            g_astVcapDumpRawInfo[i][j].stVCapRawData.pPrivateData = HI_NULL;
            g_astVcapDumpRawInfo[i][j].VcapPipeHdl = HI_INVALID_HANDLE;
            memset(&g_astVcapDumpRawInfo[i][j].stVcapDumpAttr, 0, sizeof(HI_MAPI_DUMP_ATTR_S));
        }
    }
}

HI_S32 MAPI_VCAP_Client_Init(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 j = 0;

    MAPI_VCAP_Client_ResetStatus();

    for (i = 0; i < HI_VCAP_DUMP_SCENE_NUM; i++) {
        for (j = 0; j < HI_MAPI_VCAP_MAX_PIPE_NUM; j++) {
            pthread_mutex_init(&g_astVcapDumpRawInfo[i][j].s_VcapDumpSyncLock, NULL);
            pthread_condattr_t syncCondAttr;
            pthread_condattr_init(&syncCondAttr);
            pthread_condattr_setclock(&syncCondAttr, CLOCK_MONOTONIC);
            (HI_VOID)pthread_cond_init(&g_astVcapDumpRawInfo[i][j].s_VcapDumpSyncCond, &syncCondAttr);
            (HI_VOID)pthread_condattr_destroy(&syncCondAttr);
        }
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_Client_Deinit(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 j = 0;

    MAPI_VCAP_Client_ResetStatus();

    for (i = 0; i < HI_VCAP_DUMP_SCENE_NUM; i++) {
        for (j = 0; j < HI_MAPI_VCAP_MAX_PIPE_NUM; j++) {
            pthread_mutex_destroy(&g_astVcapDumpRawInfo[i][j].s_VcapDumpSyncLock);
            pthread_cond_destroy(&g_astVcapDumpRawInfo[i][j].s_VcapDumpSyncCond);
        }
    }

    return HI_SUCCESS;
}
/* 1.bBNRraw = HI_TRUE   get BNR raw
**2.bBNRraw = HI_FALSE   get normal raw */
HI_S32 MAPI_VCAP_GetRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
    HI_S32 *ps32FrameCnt, HI_BOOL bBNRraw)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_FRAME_DATA_S szOutFrameData[HI_VCAP_RAW_EACHGROUP_MAX_NUM * HI_VCAP_RAWGROUP_MAX_NUM];
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    MAPI_PRIV_DATA_S stPrivData;
    HI_U32 u32Len;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);
    CHECK_MAPI_VCAP_NULL_PTR(ps32FrameCnt);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    u32Len = sizeof(HI_MAPI_FRAME_DATA_S) * HI_VCAP_RAW_EACHGROUP_MAX_NUM;
    stPrivData.as32PrivData[1] = bBNRraw;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_RAWFRAME, (HI_VOID *)pstFrameData, u32Len, &stPrivData);

    *ps32FrameCnt = stPrivData.as32PrivData[0];

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_RAWFRAME fail\n");

    return HI_SUCCESS;
}

/* 1.bBNRraw = HI_TRUE   Release BNR raw
**2.bBNRraw = HI_FALSE   Release normal raw */
HI_S32 MAPI_VCAP_ReleaseRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
    HI_S32 s32DataNum, HI_BOOL bBNRraw)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    MAPI_PRIV_DATA_S stPrivData;
    HI_U32 u32Len;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);

    stPrivData.as32PrivData[0] = s32DataNum;
    stPrivData.as32PrivData[1] = bBNRraw;
    u32Len = sizeof(HI_MAPI_FRAME_DATA_S) * HI_VCAP_RAW_EACHGROUP_MAX_NUM;

    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_RELEASE_RAWFRAME, (HI_VOID *)pstFrameData, u32Len, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_RELEASE_RAWFRAME fail\n");

    return HI_SUCCESS;
}

static HI_VOID *MAPI_DumpRawProc(HI_VOID *pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;
    HI_S32 j = 0;
    HI_U32 u32Size = 0;
    HI_U32 u32Idx = 0;
    HI_S32 s32RetDataNum = 0;
    HI_HANDLE VcapPipeHdl;
    HI_BOOL bMmap = HI_FALSE;
    HI_MAPI_FRAME_DATA_S astRawTempBuff[HI_VCAP_RAW_EACHGROUP_MAX_NUM];
    HI_MAPI_FRAME_DATA_S astRawWriteBuff[HI_VCAP_RAWGROUP_MAX_NUM * HI_VCAP_RAW_EACHGROUP_MAX_NUM];
    VCAP_DUMP_RAWDATA_INFO_S *pstRawDataInfo;

    prctl(PR_SET_NAME, (unsigned long)"Hi_pTVcapDumpRaw", 0, 0, 0);

    pstRawDataInfo = (VCAP_DUMP_RAWDATA_INFO_S *)pVoid;
    VcapPipeHdl = pstRawDataInfo->VcapPipeHdl;

    /* 1. check call back function */
    if (pstRawDataInfo->stVCapRawData.pfn_VCAP_RawDataProc == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "pfn_VCAP_RawDataProc is Null,Pipe[%d]\n", VcapPipeHdl);
        goto VCAP_DUMP_FAIL;
    }

    /* 2. get raw */
    for (i = 0; i < pstRawDataInfo->u32GroupCount; i++) {
        s32Ret = MAPI_VCAP_GetRawFrame(VcapPipeHdl, astRawTempBuff, &s32RetDataNum, pstRawDataInfo->bBNRRaw);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call MAPI_VCAP_GetRawFrame fail,ret:%#x\n", s32Ret);
            pstRawDataInfo->stVCapRawData.pfn_VCAP_RawDataProc = HI_NULL;
            goto VCAP_DUMP_FAIL;
        }

        /* map each frame and supplement info */
        for (j = 0; j < s32RetDataNum; j++) {
            u32Size = (astRawTempBuff[j].u32Stride[0]) * (astRawTempBuff[j].u32Height);

            astRawTempBuff[j].u64VirAddr[0] = (HI_U64)(HI_UL)HI_MPI_SYS_Mmap(astRawTempBuff[j].u64PhyAddr[0], u32Size);
            bMmap = (astRawTempBuff[j].u64VirAddr[0] != HI_NULL) ? HI_TRUE : HI_FALSE;
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Mmap fail!", bMmap, VCAP_DUMP_FAIL);

            astRawTempBuff[j].stVideoSupplement.pFrameDNGVirAddr = HI_MPI_SYS_Mmap(
                astRawTempBuff[j].stVideoSupplement.u64FrameDNGPhyAddr, sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
            bMmap = (astRawTempBuff[j].stVideoSupplement.pFrameDNGVirAddr != HI_NULL) ? HI_TRUE : HI_FALSE;
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Mmap fail!", bMmap, VCAP_DUMP_FAIL);

            astRawTempBuff[j].stVideoSupplement.pJpegDCFVirAddr = HI_MPI_SYS_Mmap(
                astRawTempBuff[j].stVideoSupplement.u64JpegDCFPhyAddr, sizeof(JPEG_DCF_S));
            bMmap = (astRawTempBuff[j].stVideoSupplement.pJpegDCFVirAddr != HI_NULL) ? HI_TRUE : HI_FALSE;
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Mmap fail!", bMmap, VCAP_DUMP_FAIL);
        }

        memcpy((&astRawWriteBuff[0] + s32RetDataNum * i), &astRawTempBuff[0],
               sizeof(HI_MAPI_FRAME_DATA_S) * s32RetDataNum);
    }

    /* 3. write raw */
    pstRawDataInfo->stVCapRawData.pfn_VCAP_RawDataProc(VcapPipeHdl, (HI_MAPI_FRAME_DATA_S *)&astRawWriteBuff[0],
        s32RetDataNum * pstRawDataInfo->u32GroupCount, pstRawDataInfo->stVCapRawData.pPrivateData);

    /* 4. release raw */
    for (i = 0; i < pstRawDataInfo->u32GroupCount; i++) {
        s32Ret = MAPI_VCAP_ReleaseRawFrame(VcapPipeHdl,
                                           (HI_MAPI_FRAME_DATA_S *)(&astRawWriteBuff[0] + s32RetDataNum * i),
                                           s32RetDataNum, pstRawDataInfo->bBNRRaw);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call MAPI_VCAP_ReleaseRawFrame fail,ret:%#x\n", s32Ret);
            pstRawDataInfo->stVCapRawData.pfn_VCAP_RawDataProc = HI_NULL;
            goto VCAP_DUMP_FAIL;
        }

        /* munmap each frame and supplement info */
        for (j = 0; j < s32RetDataNum; j++) {
            u32Idx = (i * s32RetDataNum) + j;
            u32Size = (astRawWriteBuff[u32Idx].u32Stride[0] * astRawWriteBuff[u32Idx].u32Height);

            s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)(HI_UL)astRawWriteBuff[u32Idx].u64VirAddr[0], u32Size);
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Munmap fail", (s32Ret == 0), VCAP_DUMP_FAIL);

            s32Ret = HI_MPI_SYS_Munmap(astRawWriteBuff[u32Idx].stVideoSupplement.pFrameDNGVirAddr,
                                       sizeof(DNG_IMAGE_DYNAMIC_INFO_S));
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Munmap fail", (s32Ret == 0), VCAP_DUMP_FAIL);

            s32Ret = HI_MPI_SYS_Munmap(astRawWriteBuff[u32Idx].stVideoSupplement.pJpegDCFVirAddr, sizeof(JPEG_DCF_S));
            CHECK_MAPI_VCAP_GOTO("HI_MPI_SYS_Munmap fail", (s32Ret == 0), VCAP_DUMP_FAIL);
        }
    }

VCAP_DUMP_FAIL:
    /* 5. dump raw success then broadcast this siganl */
    pstRawDataInfo->stVCapRawData.pfn_VCAP_RawDataProc = HI_NULL;

    MUTEX_LOCK(pstRawDataInfo->s_VcapDumpSyncLock);
    (HI_VOID)pthread_cond_broadcast(&pstRawDataInfo->s_VcapDumpSyncCond);
    MUTEX_UNLOCK(pstRawDataInfo->s_VcapDumpSyncLock);

    return (HI_VOID *)HI_NULL;
}

static HI_S32 MAPI_VCAP_StartDump(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
    HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData, HI_BOOL bBNRRaw)
{
    HI_S32 s32Ret;
    HI_U32 u32Idx;

    u32Idx = (bBNRRaw) ? BNR_RAW_INFO_ID : NORMAL_RAW_INFO_ID;

    if (g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pfn_VCAP_RawDataProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Dump thread not over,please wait\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    if (!bBNRRaw && !g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVcapDumpAttr.stDumpAttr.bEnable) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "please set dump normal raw enable first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (bBNRRaw && !g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVcapDumpAttr.stDumpBNRAttr.bEnable) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "please set dump BNR raw enable first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pPrivateData = pstVCapRawData->pPrivateData;
    g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pfn_VCAP_RawDataProc =
        pstVCapRawData->pfn_VCAP_RawDataProc;
    g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].u32GroupCount = u32Count;
    g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].VcapPipeHdl = VcapPipeHdl;
    g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].bBNRRaw = bBNRRaw;

    pthread_attr_t stPthreadAttr;
    pthread_attr_init(&stPthreadAttr);

    // Set the thread detached type ,the resource will be release by itself when the thread run out
    pthread_attr_setdetachstate(&stPthreadAttr,PTHREAD_CREATE_DETACHED);
    s32Ret = pthread_create(&g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].g_pthRawDump, &stPthreadAttr, MAPI_DumpRawProc,
                            (void *)&g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dump raw thread creat failed\n");
        pthread_attr_destroy(&stPthreadAttr);
        return s32Ret;
    }

    pthread_attr_destroy(&stPthreadAttr);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_StopDump(HI_HANDLE VcapPipeHdl, HI_BOOL bBNRRaw)
{
    HI_U32 u32TimeoutMs = HI_VCAP_DUMP_RAW_TIMEOUT;
    HI_U32 u32Idx;

    u32Idx = (bBNRRaw) ? BNR_RAW_INFO_ID : NORMAL_RAW_INFO_ID;

    MUTEX_LOCK(g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].s_VcapDumpSyncLock);
    struct timespec timeout = { 0, 0 };

    clock_gettime(CLOCK_MONOTONIC, &timeout);

    timeout.tv_sec += u32TimeoutMs / 1000;
    timeout.tv_nsec += (u32TimeoutMs % 1000) * 1000000;
    timeout.tv_sec += timeout.tv_nsec / 1000000000;
    timeout.tv_nsec = timeout.tv_nsec % 1000000000;

    while (g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pfn_VCAP_RawDataProc != HI_NULL) {
        HI_S32 S32WaitVal = pthread_cond_timedwait(&g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].s_VcapDumpSyncCond,
            &g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].s_VcapDumpSyncLock, &timeout);

        if (S32WaitVal == ETIMEDOUT) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "error:dump time out!\n");
            MUTEX_UNLOCK(g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].s_VcapDumpSyncLock);
            return HI_MAPI_VCAP_ETIME_OUT;
        }
    }

    MUTEX_UNLOCK(g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].s_VcapDumpSyncLock);

    if (g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pfn_VCAP_RawDataProc != HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Dump raw thread not over,please wait\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    if (g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].stVCapRawData.pfn_VCAP_RawDataProc == HI_NULL
        && g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].g_pthRawDump != (pthread_t)-1) {
        g_astVcapDumpRawInfo[u32Idx][VcapPipeHdl].g_pthRawDump = -1;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_InitSensor(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_INIT, pstSensorAttr, sizeof(HI_MAPI_SENSOR_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_INIT fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_DeinitSensor(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_DEINIT, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_DEINIT fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorAttr(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_GETATTR, pstSensorAttr, sizeof(HI_MAPI_SENSOR_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_GETATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pf32Framerate);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_SET_FRAMERATE, pf32Framerate, sizeof(HI_FLOAT), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_SET_FRAMERATE fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pf32Framerate);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_FRAMERATE, pf32Framerate, sizeof(HI_FLOAT), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_GET_FRAMERATE fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorModesCnt(HI_HANDLE VcapDevHdl, HI_S32 *ps32ModesCnt)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(ps32ModesCnt);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_MODESCNT, ps32ModesCnt, sizeof(HI_S32), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_GET_ModesCnt fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorAllModes(HI_HANDLE VcapDevHdl, HI_S32 s32ModesCnt,
                                      HI_MAPI_SENSOR_MODE_S *pstSensorModes)
{
    HI_S32 s32Ret;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    MAPI_PRIV_DATA_S stPrivData;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorModes);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    if (s32ModesCnt < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Get Sensor All Modes input count is error.\n");
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    stPrivData.as32PrivData[0] = s32ModesCnt;
    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_SENSOR_GET_ALLMODES, pstSensorModes,
                           sizeof(HI_MAPI_SENSOR_MODE_S) * s32ModesCnt, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_GET_ALLMODES fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_ATTR, pstVCapAttr, sizeof(HI_MAPI_VCAP_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_ATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_ATTR, pstVCapAttr, sizeof(HI_MAPI_VCAP_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_SENSOR_GETATTR fail fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartDev(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_START_DEV, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_START_DEV fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopDev(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapDevHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STOP_DEV, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STOP_DEV fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_START_CHN, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_START_CHN fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STOP_CHN, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STOP_CHN fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartISP(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_START_ISP, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_START_ISP fail\n");

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_VCAP_StopISP(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STOP_ISP, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STOP_ISP fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSnapAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_SNAPATTR, pstSnapAttr, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S),
                           HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_SNAPATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSnapAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_SNAPATTR, pstSnapAttr, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S),
                           HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_SNAPATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartTrigger(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_START_TRIGGER, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_START_TRIGGER fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopTrigger(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STOP_TRIGGER, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STOP_TRIGGER fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstStitchAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapStitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_STITCHATTR, pstStitchAttr, sizeof(VI_STITCH_GRP_ATTR_S),
                           HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_STITCHATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstStitchAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapStitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_STITCHATTR, pstStitchAttr, sizeof(VI_STITCH_GRP_ATTR_S),
                           HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_STITCHATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StitchTrigger(HI_HANDLE VcapStitchHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapStitchHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STITCH_TRIGGER, HI_NULL, 0, HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STITCH_TRIGGER fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstExifInfo);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SETEXIF, pstExifInfo, sizeof(HI_MAPI_SNAP_EXIF_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SETEXIF fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstExifInfo);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GETEXIF, pstExifInfo, sizeof(HI_MAPI_SNAP_EXIF_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SETEXIF fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = enCMD;
    stPrivData.as32PrivData[1] = u32Len;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_ATTR_EX, pAttr, u32Len, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_ATTR_EX fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = enCMD;
    stPrivData.as32PrivData[1] = u32Len;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_ATTR_EX, pAttr, u32Len, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_ATTR_EX fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
    VI_CROP_INFO_S *pstCropInfo)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstCropInfo);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_CROPATTR, pstCropInfo, sizeof(VI_CROP_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_CROPATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
    VI_CROP_INFO_S *pstCropInfo)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstCropInfo);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_CROPATTR, pstCropInfo, sizeof(VI_CROP_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_CROPATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_DUMP_RAW, pstDumpAttr, sizeof(HI_MAPI_DUMP_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_DUMP_RAW fail\n");

    if (pstDumpAttr->stDumpAttr.bEnable) {
        memcpy(&g_astVcapDumpRawInfo[NORMAL_RAW_INFO_ID][VcapPipeHdl].stVcapDumpAttr, pstDumpAttr,
               sizeof(HI_MAPI_DUMP_ATTR_S));
    }

    if (pstDumpAttr->stDumpBNRAttr.bEnable) {
        memcpy(&g_astVcapDumpRawInfo[BNR_RAW_INFO_ID][VcapPipeHdl].stVcapDumpAttr, pstDumpAttr,
               sizeof(HI_MAPI_DUMP_ATTR_S));
    }

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_VCAP_GetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_DUMP_RAW, pstDumpAttr, sizeof(HI_MAPI_DUMP_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_DUMP_RAW fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartDumpRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
    HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData)
{
    HI_S32 s32Ret;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_MAX_VAL("u32Count", u32Count, HI_VCAP_RAWGROUP_MAX_NUM);
    CHECK_MAPI_VCAP_MIN_VAL("u32Count", u32Count, 1);
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapRawData);
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapRawData->pfn_VCAP_RawDataProc);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_StartDump(VcapPipeHdl, u32Count, pstVCapRawData, HI_FALSE);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StartDump fail,normal raw\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopDumpRaw(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_StopDump(VcapPipeHdl, HI_FALSE);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StopDump fail,normal raw\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartDumpBNRRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
    HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData)
{
    HI_S32 s32Ret;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_MAX_VAL("u32Count", u32Count, HI_VCAP_RAWGROUP_MAX_NUM);
    CHECK_MAPI_VCAP_MIN_VAL("u32Count", u32Count, 1);
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapRawData);
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapRawData->pfn_VCAP_RawDataProc);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_StartDump(VcapPipeHdl, u32Count, pstVCapRawData, HI_TRUE);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StartDump fail,BNR raw\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopDumpBNRRaw(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_StopDump(VcapPipeHdl, HI_TRUE);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StopDump fail,BNR raw\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = OSDHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SETOSDATTR, pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S), &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SETOSDATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
    HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = OSDHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GETOSDATTR, pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S), &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GETOSDATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = OSDHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_START_OSD, HI_NULL, 0, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_START_OSD fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    stPrivData.as32PrivData[0] = OSDHdl;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_STOP_OSD, HI_NULL, 0, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_STOP_OSD fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
                                  HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    stPrivData.as32PrivData[0] = enCMD;
    stPrivData.as32PrivData[1] = u32Len;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_SNAPATTR_EX, pAttr, u32Len, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_SNAPATTR_EX fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
    HI_U32 u32Len)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    MAPI_PRIV_DATA_S stPrivData;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, 0);
    stPrivData.as32PrivData[0] = enCMD;
    stPrivData.as32PrivData[1] = u32Len;
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_SNAPATTR_EX, pAttr, u32Len, &stPrivData);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_SNAPATTR_EX fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
    HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDisAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_SET_DISATTR, pstDisAttr, sizeof(HI_MAPI_VCAP_DIS_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_SET_DISATTR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
    HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDisAttr);
    CHECK_MAPI_VCAP_TRUE(MAPI_GetMediaInitStatus(), "media not inited yet", HI_MAPI_VCAP_ENOT_INITED);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, VcapPipeHdl, PipeChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_GET_DISATTR, pstDisAttr, sizeof(HI_MAPI_VCAP_DIS_ATTR_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_GET_DISATTR fail\n");

    return HI_SUCCESS;
}

