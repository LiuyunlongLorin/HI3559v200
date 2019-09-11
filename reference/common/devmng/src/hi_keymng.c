/**
* @file    hi_keymng.c
* @brief   product keymng function
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/14
* @version

*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hi_keymng.h"
#include "hi_hal_key.h"
#include "hi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */


#define KEYMNG_CHECK_INTERVAL  (50) /**< key check interval, unit ms */

#define KEYMNG_LONG_CLICK_MIN_TIME  (1000) /**< long click key min time, unit ms */

static HI_BOOL s_bKEYMNGInitState = HI_FALSE;
static pthread_mutex_t s_KEYMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_KEYMNGCheckId;
static HI_BOOL s_bKEYMNGCheckRun;



/** keymng information */
typedef struct tagKEYMNG_INFO_S
{
    HI_HAL_KEY_IDX_E enKeyIdx;
    HI_HAL_KEY_STATE_E  enKeyState;
    HI_U32 u32KeyDownCnt;
    HI_BOOL bMultiKeyFound;
    HI_KEYMNG_KEY_ATTR_S stKeyAttr;
} KEYMNG_INFO_S;

/** keymng information */
typedef struct tagKEYMNG_INFO_SET_S
{
    HI_U32 u32KeyCnt;
    KEYMNG_INFO_S astKeyInfo[HI_KEYMNG_KEY_IDX_BUTT];
    HI_KEYMNG_GRP_KEY_CFG_S stGrpKeyCfg;
} KEYMNG_INFO_SET_S;


static KEYMNG_INFO_SET_S s_stKEYMNGInfoSet;/**< keymng info set*/


static HI_S32 KEYMNG_InParmValidChck(const HI_KEYMNG_CFG_S* pstCfg)
{
    HI_S32 i = 0, j = 0;
    /**parm check*/
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.u32KeyCnt <= HI_KEYMNG_KEY_IDX_BUTT, HI_FAILURE);

    if (pstCfg->stGrpKeyCfg.bEnable == HI_TRUE)
    {
        HI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.u32KeyCnt >= HI_KEYMNG_KEY_NUM_EACH_GRP, HI_FAILURE);

        for (i = 0; i < HI_KEYMNG_KEY_NUM_EACH_GRP; i++)
        {
            HI_APPCOMM_CHECK_EXPR(pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i] < pstCfg->stKeyCfg.u32KeyCnt, HI_FAILURE);
            HI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.astKeyAttr[pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i]].enType == HI_KEYMNG_KEY_TYPE_CLICK, HI_FAILURE);

            for (j = i + 1; j < HI_KEYMNG_KEY_NUM_EACH_GRP; j++)
            {
                HI_APPCOMM_CHECK_EXPR(pstCfg->stGrpKeyCfg.au32GrpKeyIdx[i] != pstCfg->stGrpKeyCfg.au32GrpKeyIdx[j], HI_FAILURE);
            }
        }
    }

    for (i = 0; i < pstCfg->stKeyCfg.u32KeyCnt; i++)
    {
        if ((pstCfg->stKeyCfg.astKeyAttr[i].enType == HI_KEYMNG_KEY_TYPE_CLICK) && (pstCfg->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.bLongClickEnable == HI_TRUE))
        {
            HI_APPCOMM_CHECK_EXPR(pstCfg->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.u32LongClickTime_msec >= KEYMNG_LONG_CLICK_MIN_TIME, HI_FAILURE);
        }
    }

    return HI_SUCCESS;
}

static HI_S32 KEYMNG_InternalParmInit(const HI_KEYMNG_CFG_S* pstCfg)
{
    HI_S32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FAILURE);
    s_stKEYMNGInfoSet.u32KeyCnt = pstCfg->stKeyCfg.u32KeyCnt;
    memcpy(&s_stKEYMNGInfoSet.stGrpKeyCfg, &pstCfg->stGrpKeyCfg, sizeof(HI_KEYMNG_GRP_KEY_CFG_S));

    for (i = 0; i < pstCfg->stKeyCfg.u32KeyCnt; i++)
    {
        memcpy(&s_stKEYMNGInfoSet.astKeyInfo[i].stKeyAttr, &pstCfg->stKeyCfg.astKeyAttr[i], sizeof(HI_KEYMNG_KEY_ATTR_S));
        s_stKEYMNGInfoSet.astKeyInfo[i].enKeyIdx = (HI_HAL_KEY_IDX_E)i;
        s_stKEYMNGInfoSet.astKeyInfo[i].enKeyState = HI_HAL_KEY_STATE_UP;
        s_stKEYMNGInfoSet.astKeyInfo[i].u32KeyDownCnt = 0;
        s_stKEYMNGInfoSet.astKeyInfo[i].bMultiKeyFound = HI_FALSE;
    }

    return HI_SUCCESS;
}

static HI_S32  KEYMNG_GroupKeyCheck(KEYMNG_INFO_SET_S* pstKeySetInfo)
{
    HI_S32 i = 0;
    HI_U32 u32KeyIndex;
    HI_EVENT_S stEvent;
    HI_APPCOMM_CHECK_POINTER(pstKeySetInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(pstKeySetInfo->stGrpKeyCfg.bEnable == HI_TRUE, HI_FAILURE);

    for (i = 0; i < HI_KEYMNG_KEY_NUM_EACH_GRP; i++)
    {
        u32KeyIndex = pstKeySetInfo->stGrpKeyCfg.au32GrpKeyIdx[i];

        if (pstKeySetInfo->astKeyInfo[u32KeyIndex].bMultiKeyFound == HI_TRUE)
        {
            return HI_SUCCESS;
        }

        if (pstKeySetInfo->astKeyInfo[u32KeyIndex].enKeyState == HI_HAL_KEY_STATE_UP)
        {
            return HI_SUCCESS;
        }
    }

    memset(&stEvent, '\0', sizeof(stEvent));
    stEvent.EventID = HI_EVENT_KEYMNG_GROUP;

    for (i = 0; i < HI_KEYMNG_KEY_NUM_EACH_GRP; i++)
    {
        u32KeyIndex = pstKeySetInfo->stGrpKeyCfg.au32GrpKeyIdx[i];
        pstKeySetInfo->astKeyInfo[u32KeyIndex].bMultiKeyFound = HI_TRUE;
        stEvent.aszPayload[i] = u32KeyIndex;
    }

    HI_EVTHUB_Publish(&stEvent);
    MLOGD("group key event\n");
    return HI_SUCCESS;
}


static HI_S32  KEYMNG_HoldCheck (KEYMNG_INFO_S* pstHoldKeyInfo)
{

    HI_HAL_KEY_STATE_E enKeyState = HI_HAL_KEY_STATE_UP;
    HI_EVENT_S stEvent;

    HI_APPCOMM_CHECK_POINTER(pstHoldKeyInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(pstHoldKeyInfo->stKeyAttr.enType == HI_KEYMNG_KEY_TYPE_HOLD, HI_FAILURE);

    HI_HAL_KEY_GetState(pstHoldKeyInfo->enKeyIdx, &enKeyState);

    if (HI_HAL_KEY_STATE_DOWN == enKeyState)
    {
        if (HI_HAL_KEY_STATE_UP == pstHoldKeyInfo->enKeyState)
        {
            pstHoldKeyInfo->enKeyState = HI_HAL_KEY_STATE_DOWN;
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_KEYMNG_HOLD_DOWN;
            stEvent.arg1 = pstHoldKeyInfo->stKeyAttr.s32Id;      /**<KEYID 0/1/2*/
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("key down event[%u]\n", pstHoldKeyInfo->stKeyAttr.s32Id);
        }
    }
    else
    {
        if (HI_HAL_KEY_STATE_DOWN == pstHoldKeyInfo->enKeyState)
        {
            pstHoldKeyInfo->enKeyState = HI_HAL_KEY_STATE_UP;

            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_KEYMNG_HOLD_UP;
            stEvent.arg1 = pstHoldKeyInfo->stKeyAttr.s32Id;      /**<KEYID 0/1/2*/
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("key up event[%u]\n", pstHoldKeyInfo->stKeyAttr.s32Id);
        }
    }

    return HI_SUCCESS;
}


static HI_S32  KEYMNG_ClickCheck (KEYMNG_INFO_S* pstClickKeyInfo)
{
    HI_HAL_KEY_STATE_E enKeyState = HI_HAL_KEY_STATE_UP;
    HI_EVENT_S stEvent;

    HI_APPCOMM_CHECK_POINTER(pstClickKeyInfo, HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(pstClickKeyInfo->stKeyAttr.enType == HI_KEYMNG_KEY_TYPE_CLICK, HI_FAILURE);
    HI_HAL_KEY_GetState(pstClickKeyInfo->enKeyIdx, &enKeyState);

    /** Check Key Event */
    if (HI_HAL_KEY_STATE_DOWN == enKeyState)
    {
        if (HI_HAL_KEY_STATE_UP == pstClickKeyInfo->enKeyState)
        {
            pstClickKeyInfo->enKeyState = HI_HAL_KEY_STATE_DOWN;
        }

        if (pstClickKeyInfo->u32KeyDownCnt < (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL))
        {
            pstClickKeyInfo->u32KeyDownCnt++;

            if (pstClickKeyInfo->u32KeyDownCnt >= (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL))
            {
                if (pstClickKeyInfo->bMultiKeyFound == HI_FALSE)
                {
                    memset(&stEvent, '\0', sizeof(stEvent));
                    stEvent.EventID = HI_EVENT_KEYMNG_LONG_CLICK;
                    stEvent.arg1 = pstClickKeyInfo->enKeyIdx;      /**<KEYID 0/1/2*/
                    HI_EVTHUB_Publish(&stEvent);
                    MLOGD("long click event[%u]\n", pstClickKeyInfo->enKeyIdx);
                }

            }
        }
    }
    else
    {
        if (HI_HAL_KEY_STATE_DOWN == pstClickKeyInfo->enKeyState)
        {
            pstClickKeyInfo->enKeyState = HI_HAL_KEY_STATE_UP;

            if (pstClickKeyInfo->u32KeyDownCnt < (pstClickKeyInfo->stKeyAttr.unAttr.stClickKeyAttr.u32LongClickTime_msec / KEYMNG_CHECK_INTERVAL))
            {
                if (pstClickKeyInfo->bMultiKeyFound == HI_FALSE)
                {
                    memset(&stEvent, '\0', sizeof(stEvent));
                    stEvent.EventID = HI_EVENT_KEYMNG_SHORT_CLICK;
                    stEvent.arg1 = pstClickKeyInfo->stKeyAttr.s32Id;      /**<KEYID 0/1/2*/
                    HI_EVTHUB_Publish(&stEvent);
                    MLOGD("short click event[%u]\n", pstClickKeyInfo->stKeyAttr.s32Id);
                }
            }

            pstClickKeyInfo->u32KeyDownCnt = 0;
            pstClickKeyInfo->bMultiKeyFound = HI_FALSE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32  KEYMNG_KeyCheck(KEYMNG_INFO_SET_S* pstKeySetInfo)
{
    HI_S32 i = 0;

    HI_APPCOMM_CHECK_POINTER(pstKeySetInfo, HI_FAILURE);

    for (i = 0; i < pstKeySetInfo->u32KeyCnt; i++)
    {
        if (pstKeySetInfo->astKeyInfo[i].stKeyAttr.enType == HI_KEYMNG_KEY_TYPE_CLICK)
        {
            KEYMNG_ClickCheck(&pstKeySetInfo->astKeyInfo[i]);
        }
        else
        {
            KEYMNG_HoldCheck(&pstKeySetInfo->astKeyInfo[i]);
        }

    }

    if (pstKeySetInfo->stGrpKeyCfg.bEnable == HI_TRUE)
    { KEYMNG_GroupKeyCheck(pstKeySetInfo); }

    return HI_SUCCESS;
}


static HI_VOID*  KEYMNG_CheckThread(HI_VOID* pData)
{

    MLOGD("thread KEY_CHECK enter\n");
    prctl(PR_SET_NAME, "KEY_CHECK", 0, 0, 0); /**< Set Task Name */

    while (s_bKEYMNGCheckRun)
    {

        KEYMNG_KeyCheck(&s_stKEYMNGInfoSet);
        HI_usleep(KEYMNG_CHECK_INTERVAL * 1000);
    }

    MLOGD("thread KEY_CHECK exit\n");
    return NULL;
}

HI_S32 HI_KEYMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret=HI_EVTHUB_Register(HI_EVENT_KEYMNG_SHORT_CLICK);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register short click key event fail\n");
        return HI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_KEYMNG_LONG_CLICK);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register long click key event fail\n");
        return HI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_KEYMNG_HOLD_DOWN);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register hold down key event fail\n");
        return HI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_KEYMNG_HOLD_UP);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register hold up key event fail\n");
        return HI_KEYMNG_EREGISTEREVENT;
    }
    s32Ret=HI_EVTHUB_Register(HI_EVENT_KEYMNG_GROUP);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Register group key event fail\n");
        return HI_KEYMNG_EREGISTEREVENT;
    }
    return HI_SUCCESS;
}

HI_S32 HI_KEYMNG_Init(const HI_KEYMNG_CFG_S* pstCfg)
{
    HI_S32 s32Ret;

    if (KEYMNG_InParmValidChck(pstCfg) == HI_FAILURE)
    {
        MLOGE("parm check error\n");
        return HI_KEYMNG_EINVAL;
    }

    HI_MUTEX_LOCK(s_KEYMNGMutex);

    if (s_bKEYMNGInitState  == HI_TRUE)
    {
        MLOGE("keymng has already been started\n");
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_EINITIALIZED;
    }

    s32Ret = HI_HAL_KEY_Init();

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_HAL_KEY_Init Failed\n");
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_EINTER;
    }

    KEYMNG_InternalParmInit(pstCfg);

    /** Create Key Check Task */
    s_bKEYMNGCheckRun = HI_TRUE;
    s32Ret = pthread_create(&s_KEYMNGCheckId, NULL, KEYMNG_CheckThread, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Create KeyCheck Thread Fail!\n");
        HI_HAL_KEY_Deinit();
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_ETHREAD;
    }

    s_bKEYMNGInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_KEYMNGMutex);
    return HI_SUCCESS;

}

HI_S32 HI_KEYMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret;

    HI_MUTEX_LOCK(s_KEYMNGMutex);

    if (s_bKEYMNGInitState  == HI_FALSE)
    {
        MLOGE("keymng no init\n");
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_ENOINIT;
    }

    /** Destory Key Check Task */
    s_bKEYMNGCheckRun = HI_FALSE;

    s32Ret = pthread_join(s_KEYMNGCheckId, NULL);

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Join KeyCheck Thread Fail!\n");
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_ETHREAD;
    }

    /** Close Key */
    s32Ret = HI_HAL_KEY_Deinit();

    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("HI_HAL_KEY_Deinit Fail!\n");
        HI_MUTEX_UNLOCK(s_KEYMNGMutex);
        return HI_KEYMNG_EINTER;
    }

    s_bKEYMNGInitState = HI_FALSE;
    HI_MUTEX_UNLOCK(s_KEYMNGMutex);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
