/**
 * @file    filemng_spacemonitor.c
 * @brief   file manager space monitor function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/prctl.h>

#include "hi_eventhub.h"
#ifdef CONFIG_FILEMNG_DTCF
#include "hi_filemng_dtcf.h"
#endif
#include "hi_storagemng.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define SPACEMONITOR_DEFAULT_INVL    (5)

static pthread_t s_SMThread = -1;
static pthread_mutex_t g_SMCheckMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_SMExitMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_SMCheckCond;
static pthread_cond_t  g_SMExitCond;
static struct timespec s_SMLasttime = {.tv_sec = 0};
static SPACEMONITOR_CFG_S s_stSMCfg;
static HI_S32 s_s32SMFullFlag = 0; /**<0x000:space enough;0x001:total space full;0x010:movie space full;0x100:emr movie space full */

#ifdef CONFIG_FILEMNG_DTCF
static HI_U32 s_u32SMMovieSpace = 0;
static HI_U32 s_u32SMEmrSpace = 0;

static HI_VOID FILEMNG_SPACEMONITOR_CheckMovie(HI_U32 u32TotalSize_MB)
{
    if ((s_u32SMMovieSpace + s_stSMCfg.u32WarningStage) >= (s_stSMCfg.u8SharePercent * u32TotalSize_MB / 100)) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_MOVIEFULL;
        MLOGW("movie space full(used:%u warning:%u ratio:%u)\n", s_u32SMMovieSpace, s_stSMCfg.u32WarningStage,
              (s_stSMCfg.u8SharePercent * u32TotalSize_MB / 100));
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_MOVIEFULL;
    }
}

static HI_VOID FILEMNG_SPACEMONITOR_CheckEmr(HI_U32 u32TotalSize_MB)
{
    if ((s_u32SMEmrSpace + s_stSMCfg.u32WarningStage) >= ((100 - s_stSMCfg.u8SharePercent)*u32TotalSize_MB / 100)) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_EMRFULL;
        MLOGW("emr movie space full(used:%u warning:%u ratio:%u)\n", s_u32SMEmrSpace, s_stSMCfg.u32WarningStage,
              ((100 - s_stSMCfg.u8SharePercent)*u32TotalSize_MB / 100));
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_EMRFULL;
    }
}
#endif

static HI_VOID FILEMNG_SPACEMONITOR_CheckTotal(HI_U32 u32AvailableSize_MB)
{
    if (u32AvailableSize_MB < s_stSMCfg.u32WarningStage) {
        s_s32SMFullFlag |= SPACEMONITOR_MASK_TOTALFULL;
        MLOGW("total space full(warning:%u free:%u)\n", s_stSMCfg.u32WarningStage, u32AvailableSize_MB);
    } else {
        s_s32SMFullFlag = s_s32SMFullFlag & ~SPACEMONITOR_MASK_TOTALFULL;
    }
}

static HI_VOID  *FILEMNG_SPACEMONITOR_SpaceCheckThread(HI_VOID *pData)
{
    HI_S32 s32Ret = 0;
    prctl(PR_SET_NAME, "SpaceMonitor", 0, 0, 0);
    HI_EVENT_S stEvent;
    stEvent.EventID = HI_EVENT_FILEMNG_BUTT;
    HI_STORAGE_FS_INFO_S stFSInfo;
    HI_U32 u32CheckDelay = s_stSMCfg.u32MaxCheckDelay;

    while (s_stSMCfg.u32Interval) {
        HI_MUTEX_LOCK(g_SMExitMutex);
        s32Ret = HI_STORAGEMNG_GetFSInfo(s_stSMCfg.szMntPath, &stFSInfo);
        HI_U32 u32AvailableSize_MB = stFSInfo.u64AvailableSize >> 20;
        if (HI_SUCCESS == s32Ret) {
#ifdef CONFIG_FILEMNG_DTCF
            if (0 != s_stSMCfg.u8SharePercent && NULL != s_stSMCfg.pfnGetRatioSpace) {
                /*ratio mode */
                s32Ret = s_stSMCfg.pfnGetRatioSpace(&s_u32SMMovieSpace, &s_u32SMEmrSpace);
                if (HI_SUCCESS == s32Ret) {
                    HI_U32 u32TotalSize_MB = stFSInfo.u64TotalSize >> 20;
                    FILEMNG_SPACEMONITOR_CheckMovie(u32TotalSize_MB);
                    FILEMNG_SPACEMONITOR_CheckEmr(u32TotalSize_MB);
                } else {
                    HI_LOG_PrintFuncErr(FILEMNG_SPACEMONITOR_GetRatioSpace, s32Ret);
                }
            }
#endif
            FILEMNG_SPACEMONITOR_CheckTotal(u32AvailableSize_MB);
            if (SPACEMONITOR_MASK_ENOUGH != s_s32SMFullFlag) {
                if (HI_EVENT_FILEMNG_SPACE_FULL != stEvent.EventID) {
                    stEvent.EventID = HI_EVENT_FILEMNG_SPACE_FULL;
                    HI_EVTHUB_Publish(&stEvent);
                }

#ifdef CONFIG_FILEMNG_DTCF
                if (s_stSMCfg.u32GuaranteedStage >= s_stSMCfg.u32WarningStage && NULL != s_stSMCfg.pfnCoverCB) {
                    s32Ret = s_stSMCfg.pfnCoverCB(s_s32SMFullFlag);
                    if (HI_SUCCESS == s32Ret) {
                        u32CheckDelay = s_stSMCfg.u32MaxCheckDelay;
                        HI_MUTEX_UNLOCK(g_SMExitMutex);
                        continue;
                    } else if (-1 == s32Ret) {
                        u32CheckDelay = 0;
                    } else {
                        HI_LOG_PrintFuncErr(FILEMNG_SPACEMONITOR_Cover, s32Ret);
                    }
                }
#endif
            } else {
                if (HI_EVENT_FILEMNG_SPACE_ENOUGH != stEvent.EventID) {
                    stEvent.EventID = HI_EVENT_FILEMNG_SPACE_ENOUGH;
                    HI_EVTHUB_Publish(&stEvent);
                }
            }
        } else {
            HI_LOG_PrintFuncErr(HI_STORAGEMNG_GetFSInfo, s32Ret);
        }

        HI_MUTEX_LOCK(g_SMCheckMutex);
        HI_COND_WAIT(g_SMCheckCond, g_SMCheckMutex);
        HI_MUTEX_UNLOCK(g_SMCheckMutex);

        HI_COND_TIMEDWAIT(g_SMExitCond, g_SMExitMutex, u32CheckDelay * 1000 * 1000);
        HI_MUTEX_UNLOCK(g_SMExitMutex);
    }

    MLOGD("SpaceMonitor thread exit\n");
    return NULL;
}

HI_S32 FILEMNG_SPACEMONITOR_Create(const SPACEMONITOR_CFG_S *pstConfig)
{
    memset(&s_stSMCfg, 0, sizeof(SPACEMONITOR_CFG_S));

    if (0 == pstConfig->u32WarningStage) {
        return HI_SUCCESS;
    }
#ifdef CONFIG_FILEMNG_DTCF
    if (pstConfig->u8SharePercent >= 100) {
        return HI_FILEMNG_EINVAL;
    }
#endif
    if (-1 == s_SMThread) {
        if (clock_gettime(CLOCK_MONOTONIC, &s_SMLasttime) < 0) {
            MLOGE("clock_gettime error:%s \n", strerror(errno));
            return HI_FILEMNG_EINTER;
        }

        /*init cond*/
        HI_COND_INIT(g_SMCheckCond);
        HI_COND_INIT(g_SMExitCond);
        memcpy(&s_stSMCfg, pstConfig, sizeof(SPACEMONITOR_CFG_S));

        if (0 == s_stSMCfg.u32Interval) {
            s_stSMCfg.u32Interval = SPACEMONITOR_DEFAULT_INVL;
        }

        if (0 != pthread_create(&s_SMThread, NULL, FILEMNG_SPACEMONITOR_SpaceCheckThread, &s_stSMCfg)) {
            MLOGE("pthread_create error:%s \n", strerror(errno));
            HI_COND_DESTROY(g_SMCheckCond);
            HI_COND_DESTROY(g_SMExitCond);
            return HI_FILEMNG_EINTER;
        }
    }
    return HI_SUCCESS;
}

HI_S32 FILEMNG_SPACEMONITOR_Destroy(HI_VOID)
{
    if (-1 != s_SMThread) {
        s_stSMCfg.u32Interval = 0;
        HI_MUTEX_LOCK(g_SMCheckMutex);
        HI_COND_SIGNAL(g_SMCheckCond);
        HI_MUTEX_UNLOCK(g_SMCheckMutex);
        HI_MUTEX_LOCK(g_SMExitMutex);
        HI_COND_SIGNAL(g_SMExitCond);
        HI_MUTEX_UNLOCK(g_SMExitMutex);
        pthread_join(s_SMThread, NULL);
        HI_COND_DESTROY(g_SMCheckCond);
        HI_COND_DESTROY(g_SMExitCond);
        s_SMThread = -1;
    }

    memset(&s_stSMCfg, 0, sizeof(SPACEMONITOR_CFG_S));
    return HI_SUCCESS;
}

HI_S32 FILEMNG_SPACEMONITOR_CheckSpace(HI_VOID)
{
    static struct timespec s_Curtime;

    if (clock_gettime(CLOCK_MONOTONIC, &s_Curtime) < 0) {
        MLOGE("clock_gettime error:%s \n", strerror(errno));
        return HI_FILEMNG_EINTER;
    }

    if ((0 < s_stSMCfg.u32Interval && (s_Curtime.tv_sec - s_SMLasttime.tv_sec) > s_stSMCfg.u32Interval)
            || (0 == s_SMLasttime.tv_sec)) {
        HI_MUTEX_LOCK(g_SMCheckMutex);
        HI_COND_SIGNAL(g_SMCheckCond);
        HI_MUTEX_UNLOCK(g_SMCheckMutex);
        memcpy(&s_SMLasttime, &s_Curtime, sizeof(struct timespec));
    }
    return HI_SUCCESS;
}

HI_S32 FILEMNG_SPACEMONITOR_JudgeStage(HI_U64 u64RealUsedSize_MB)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_EVENT_S stEvent;
    HI_STORAGE_FS_INFO_S stFSInfo;

    if (0 < s_stSMCfg.u32WarningStage) {
        s32Ret = HI_STORAGEMNG_GetFSInfo(s_stSMCfg.szMntPath, &stFSInfo);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_STORAGEMNG_GetFSInfo, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        if ((stFSInfo.u64AvailableSize >> 20) <= s_stSMCfg.u32WarningStage) {
            s32Ret = HI_FILEMNG_EFULL;
            stEvent.EventID = HI_EVENT_FILEMNG_SPACE_FULL;
            HI_EVTHUB_Publish(&stEvent);
            MLOGW("space is NOT enough!\n");
        }

        if ((stFSInfo.u64UsedSize >> 20) > (u64RealUsedSize_MB + s_stSMCfg.u32WarningStage)) {
            stEvent.EventID = HI_EVENT_FILEMNG_UNIDENTIFICATION;
            s32Ret = HI_FILEMNG_EUNIDENTIFICATION;
            HI_EVTHUB_Publish(&stEvent);
            MLOGW("Too many Unrecognized files!used:%llu,realused:%llu,warning:%u\n", (stFSInfo.u64UsedSize >> 20),
                  u64RealUsedSize_MB, s_stSMCfg.u32WarningStage);
        }
    }

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

