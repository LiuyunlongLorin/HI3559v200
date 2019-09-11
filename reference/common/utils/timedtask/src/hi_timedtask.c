/**
 * @file    hi_timedtask.c
 * @brief   timed-task interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 * @version   1.0

 */
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "hi_timedtask.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** time task max number */
#define TIMEDTASK_MAX_NUM (16)
/** check interval if none task in checking */
#define IDLE_CHECK_INTERVAL (1000000)
/** check interval if any task in checking */
#define TASK_CHECK_INTERVAL (300000)

/** timed-task check manage struct */
typedef struct tagTIMEDTASK_MNG_S
{
    HI_BOOL bUsed;
    pthread_mutex_t mutex;
    struct timespec stCheckTime; /**<task check base time */
    HI_TIMEDTASK_CFG_S stCfg;
} TIMEDTASK_MNG_S;
static TIMEDTASK_MNG_S s_stTIMEDTSKMng[TIMEDTASK_MAX_NUM];

/** timed task check thread id */
static pthread_t s_TIMEDTSKCheckId = 0;
/** timed task check thread runnging flag */
static HI_BOOL s_bTIMEDTSKCheckRun = HI_FALSE;


/**
 * @brief timed task module initialization
 * @param[in]pData : thread data
 * @return NULL
 */
static HI_VOID* TIMEDTASK_Check(HI_VOID* pData)
{
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
    struct timespec stCurTime;
    HI_U32 u32TimeDiff; /**<unit millisecond */
    HI_S32 s32Idx;
    HI_U32 u32CheckingTskCnt;

    while (s_bTIMEDTSKCheckRun)
    {
        u32CheckingTskCnt = 0;

        for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx)
        {
            HI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);

            if (s_stTIMEDTSKMng[s32Idx].bUsed && s_stTIMEDTSKMng[s32Idx].stCfg.stAttr.bEnable)
            {
                u32CheckingTskCnt++;
                clock_gettime(CLOCK_MONOTONIC, &stCurTime);
                u32TimeDiff = (stCurTime.tv_sec - s_stTIMEDTSKMng[s32Idx].stCheckTime.tv_sec) * 1000 +
                              (stCurTime.tv_nsec - s_stTIMEDTSKMng[s32Idx].stCheckTime.tv_nsec) / 1000000;

                if (u32TimeDiff >= (s_stTIMEDTSKMng[s32Idx].stCfg.stAttr.u32Time_sec * 1000))
                {
                    /* Disable This Timed-Task, in case repeated */
                    s_stTIMEDTSKMng[s32Idx].stCfg.stAttr.bEnable = HI_FALSE;
                    MLOGD("TimedTask[%d] Checked\n", s32Idx);

                    /* Process Task */
                    if (s_stTIMEDTSKMng[s32Idx].stCfg.pfnProc)
                    {
                        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
                        s_stTIMEDTSKMng[s32Idx].stCfg.pfnProc(s_stTIMEDTSKMng[s32Idx].stCfg.pvPrivData);
                        HI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
                    }
                }
            }

            HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        }

        HI_usleep((u32CheckingTskCnt > 0) ? TASK_CHECK_INTERVAL : IDLE_CHECK_INTERVAL);
    }

    HI_LOG_FuncExit();
    return NULL;
}

/**
 * @brief    timed task module initialization.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_EXPR(HI_TRUE != s_bTIMEDTSKCheckRun, HI_EINITIALIZED);

    HI_S32 s32Idx = 0;

    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx)
    {
        HI_MUTEX_INIT_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        HI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);
        s_stTIMEDTSKMng[s32Idx].bUsed = HI_FALSE;
        memset(&s_stTIMEDTSKMng[s32Idx].stCfg, 0, sizeof(HI_TIMEDTASK_CFG_S));
        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
    }

    s_bTIMEDTSKCheckRun = HI_TRUE;
    s32Ret = pthread_create(&s_TIMEDTSKCheckId, NULL, TIMEDTASK_Check, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        HI_TIMEDTASK_Deinit();
        MLOGE("Create TimedTask Check Thread Failed\n");
        return HI_FAILURE;
    }

    MLOGD("TimedTask Check Thread[%lu] Create Successful\n", s_TIMEDTSKCheckId);
    return HI_SUCCESS;
}

/**
 * @brief    timed task module deinitialization.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Deinit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);

    s_bTIMEDTSKCheckRun = HI_FALSE;

    if (0 != s_TIMEDTSKCheckId)
    {
        s32Ret = pthread_join(s_TIMEDTSKCheckId, NULL);

        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("Join TimedTask Check Thread Failed\n");
            return HI_FAILURE;
        }

        MLOGD("TimedTask Check Thread[%lu] Destroy Successful\n", s_TIMEDTSKCheckId);
        s_TIMEDTSKCheckId = 0;
    }

    HI_S32 s32Idx = 0;

    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx)
    {
        HI_MUTEX_DESTROY(s_stTIMEDTSKMng[s32Idx].mutex);
    }

    return HI_SUCCESS;
}

/**
 * @brief    create timed task.
 * @param[in] pstTimeTskCfg : timed task config.
 * @param[out] pTimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Create(const HI_TIMEDTASK_CFG_S* pstTimeTskCfg, HI_HANDLE* pTimeTskhdl)
{
    HI_S32 s32Idx = 0;
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);
    HI_APPCOMM_CHECK_POINTER(pstTimeTskCfg, HI_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pTimeTskhdl, HI_EINVAL);
    if(pstTimeTskCfg->stAttr.bEnable)
    {
        HI_APPCOMM_CHECK_EXPR(pstTimeTskCfg->stAttr.u32Time_sec != 0, HI_EINVAL);
    }
    HI_APPCOMM_CHECK_EXPR(pstTimeTskCfg->pfnProc != 0, HI_EINVAL);

    for (s32Idx = 0; s32Idx < TIMEDTASK_MAX_NUM; ++s32Idx)
    {
        HI_MUTEX_LOCK(s_stTIMEDTSKMng[s32Idx].mutex);

        if (HI_FALSE == s_stTIMEDTSKMng[s32Idx].bUsed)
        {
            memcpy(&s_stTIMEDTSKMng[s32Idx].stCfg, pstTimeTskCfg, sizeof(HI_TIMEDTASK_CFG_S));
            clock_gettime(CLOCK_MONOTONIC, &s_stTIMEDTSKMng[s32Idx].stCheckTime);
            s_stTIMEDTSKMng[s32Idx].bUsed = HI_TRUE;
            *pTimeTskhdl = s32Idx;
            HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
            return HI_SUCCESS;
        }

        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[s32Idx].mutex);
    }

    return HI_ENORES;
}

/**
 * @brief    destroy timed task.
 * @param[in] TimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_Destroy(HI_HANDLE TimeTskhdl)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);
    HI_APPCOMM_CHECK_EXPR(TimeTskhdl < TIMEDTASK_MAX_NUM, HI_EINVAL);
    HI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);

    if (HI_FALSE == s_stTIMEDTSKMng[TimeTskhdl].bUsed)
    {
        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
        return HI_EINVAL;
    }

    s_stTIMEDTSKMng[TimeTskhdl].bUsed = HI_FALSE;
    memset(&s_stTIMEDTSKMng[TimeTskhdl].stCfg, 0, sizeof(HI_TIMEDTASK_CFG_S));
    HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
    return HI_SUCCESS;
}

/**
 * @brief    get timed task attribute.
 * @param[in] TimeTskhdl : timed task handle.
 * @param[out] pstTimeTskCfg : timed task attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_GetAttr(HI_HANDLE TimeTskhdl, HI_TIMEDTASK_ATTR_S * pstTimeTskAttr)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);
    HI_APPCOMM_CHECK_POINTER(pstTimeTskAttr, HI_EINVAL);
    HI_APPCOMM_CHECK_EXPR(TimeTskhdl < TIMEDTASK_MAX_NUM, HI_EINVAL);
    HI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);

    if (HI_FALSE == s_stTIMEDTSKMng[TimeTskhdl].bUsed)
    {
        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
        return HI_EINVAL;
    }

    memcpy(pstTimeTskAttr, &s_stTIMEDTSKMng[TimeTskhdl].stCfg.stAttr, sizeof(HI_TIMEDTASK_ATTR_S));
    HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
    return HI_SUCCESS;
}

/**
 * @brief    set timed task attribute.
 * @param[in] TimeTskhdl : timed task handle.
 * @param[in] pstTimeTskCfg : timed task attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_SetAttr(HI_HANDLE TimeTskhdl, const HI_TIMEDTASK_ATTR_S * pstTimeTskAttr)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);
    HI_APPCOMM_CHECK_POINTER(pstTimeTskAttr, HI_EINVAL);
    if(pstTimeTskAttr->bEnable)
    {
        HI_APPCOMM_CHECK_EXPR(pstTimeTskAttr->u32Time_sec != 0, HI_EINVAL);
    }
    HI_APPCOMM_CHECK_EXPR(TimeTskhdl < TIMEDTASK_MAX_NUM, HI_EINVAL);
    HI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);

    if (HI_FALSE == s_stTIMEDTSKMng[TimeTskhdl].bUsed)
    {
        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
        return HI_EINVAL;
    }
    if(pstTimeTskAttr->bEnable)
    {
        clock_gettime(CLOCK_MONOTONIC, &s_stTIMEDTSKMng[TimeTskhdl].stCheckTime);
    }
    memcpy(&s_stTIMEDTSKMng[TimeTskhdl].stCfg.stAttr, pstTimeTskAttr, sizeof(HI_TIMEDTASK_ATTR_S));
    HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
    return HI_SUCCESS;
}

/**
 * @brief    reset specified task check time.
 * @param[in] TimeTskhdl : timed task handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/12
 */
HI_S32 HI_TIMEDTASK_ResetTime(HI_HANDLE TimeTskhdl)
{
    HI_APPCOMM_CHECK_EXPR(HI_TRUE == s_bTIMEDTSKCheckRun, HI_ENOINIT);
    HI_APPCOMM_CHECK_EXPR(TimeTskhdl < TIMEDTASK_MAX_NUM, HI_EINVAL);
    HI_MUTEX_LOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);

    if (HI_FALSE == s_stTIMEDTSKMng[TimeTskhdl].bUsed)
    {
        HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
        return HI_EINVAL;
    }
    clock_gettime(CLOCK_MONOTONIC, &s_stTIMEDTSKMng[TimeTskhdl].stCheckTime);
    HI_MUTEX_UNLOCK(s_stTIMEDTSKMng[TimeTskhdl].mutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

