/**
 * @file    hi_videodetect.c
 * @brief   videodetect manager.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/2
 * @version   1.0

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "mpi_sys.h"

#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"

#include "hi_appcomm.h"
#include "hi_eventhub.h"


#include "hi_videodetect.h"
#include "videodetect_queue.h"

#ifdef CONFIG_MOTIONDETECT_ON
#include "videodetect_md.h"
#endif

#include "hi_queue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     VIDEODETECT */
/** @{ */  /** <!-- [VIDEODETECT] */
/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "VIDEODETECT"

#define VIDEODETECT_MDRPOC_FRAME_NUM                   (2)
#define VIDEODETECT_DUMP_FRAMENUM_MAX                  (2)

static HI_BOOL g_videoDetectInitFlg = HI_FALSE;
static pthread_mutex_t g_videoDetectInitMutex = PTHREAD_MUTEX_INITIALIZER;

#define VIDEODETECT_CHECK_INIT()   \
    do{ \
        HI_MUTEX_LOCK(g_videoDetectInitMutex); \
        if(HI_TRUE != g_videoDetectInitFlg)  \
        {   \
            HI_MUTEX_UNLOCK(g_videoDetectInitMutex);   \
            MLOGW("VideoDetect has not been inited,please init first \n"); \
            return HI_VIDEODETECT_ENOTINIT;    \
        }   \
        HI_MUTEX_UNLOCK(g_videoDetectInitMutex);   \
    }while(0)

/**free MMZ*/
#define VIDEODETECT_MMZ_FREE(phy,vir)\
do{\
    if ((0 != (phy)) && (0 != (vir)))\
    {\
        HI_MPI_SYS_MmzFree((phy),(HI_VOID*)(HI_UL)(vir));\
    }\
}while(0)

#define VIDEODETECT_CONVERT_64BIT_ADDR(Type,Addr) (Type*)(HI_UL)(Addr)

typedef enum tagVIDEODETECT_TASK_STATE_E
{
    VIDEODETECT_TASK_STATE_INVALID = 0,
    VIDEODETECT_TASK_STATE_STOPED,           /**< inited but not start */
    VIDEODETECT_TASK_STATE_STARTED,
    VIDEODETECT_TASK_STATE_PREPARE,
    VIDEODETECT_TASK_STATE_BUTT
} VIDEODETECT_TASK_STATE_E;

typedef struct tagVIDEODETECT_TASK_CALLBACK_S
{
    HI_VIDEODETECT_ALG_TYPE_E algType;
    HI_VIDEODETECT_TASK_CALLBACK_S taskCallback[HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX];
} VIDEODETECT_TASK_CALLBACK_S;

typedef struct tagVIDEODETECT_TASK_CTX_S
{
    pthread_t taskThreadCheckId;
    pthread_mutex_t taskMutex;
    pthread_mutex_t regMutex;
    HI_HANDLE taskHdl;
    HI_HANDLE queueHdl;
    HI_BOOL taskThreadProcFlag;
    VIDEODETECT_TASK_STATE_E taskState;
    HI_VIDEODETECT_TASK_CFG_S taskCfg;
    VIDEODETECT_TASK_CALLBACK_S taskCallbackArray[HI_VIDEODETECT_ALG_TYPE_BUTT];
} VIDEODETECT_TASK_CTX_S;

typedef struct tagVIDEODETECT_CTX_S
{
    pthread_mutex_t ctxMutex;
    VIDEODETECT_TASK_CTX_S taskCtx[HI_VIDEODETECT_TASK_NUM_MAX];
} VIDEODETECT_CTX_S;
static VIDEODETECT_CTX_S g_videoDetectCtx;


static HI_BOOL VIDEODETECT_PARAMCHECK_TaskCfg(const HI_VIDEODETECT_TASK_CFG_S* taskConfig)
{
    HI_S32 i = 0;
    if(taskConfig->algCnt < 0 || taskConfig->algCnt > HI_VIDEODETECT_ALG_TYPE_BUTT)
    {
        MLOGE("alg cnt is [%d], but must in [0, %d]",taskConfig->algCnt,HI_VIDEODETECT_ALG_TYPE_BUTT);
        return HI_FALSE;
    }

    for (i = 0; i < taskConfig->algCnt; i++)
    {
        if(taskConfig->algAttr[i].algType == HI_VIDEODETECT_ALG_TYPE_BUTT)
        {
            MLOGE("currently only support MD, here you use [%d]",taskConfig->algAttr[i].algType);
            return HI_FALSE;
        }
    }
    return HI_TRUE;
}
//

static HI_BOOL VIDEODETECT_GetAlgRegisterStatus(HI_VIDEODETECT_ALG_TYPE_E algType,
    const VIDEODETECT_TASK_CTX_S* taskCtx, HI_S32* arrayIndex)
{
    HI_S32 i, j = 0;
    for (i = 0; i < HI_VIDEODETECT_ALG_TYPE_BUTT; i++)
    {
        for (j = 0; j < HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX; j++)
        {
            if (HI_NULL != taskCtx->taskCallbackArray[i].taskCallback[j].fnGetParamCallBack)
            {
                if (algType == taskCtx->taskCallbackArray[i].algType)
                {
                    *arrayIndex = i;
                    return HI_TRUE;
                }
            }
        }
    }
    return HI_FALSE;
}

static HI_BOOL VIDEODETECT_GetAlgUsedStatus(HI_VIDEODETECT_ALG_TYPE_E algType,
    const HI_VIDEODETECT_TASK_CFG_S* taskCtx, HI_S32* arrayIndex)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_VIDEODETECT_ALG_TYPE_BUTT; i++)
    {
        if (algType == taskCtx->algAttr[i].algType)
        {
            *arrayIndex = i;
            return HI_TRUE;
        }
    }
    return HI_FALSE;
}

/**if TaskState is not started or no AlgProc Add, then mainThread will do nothing*/
static HI_BOOL VIDEODETECT_AlgProc_ThreadCheck(VIDEODETECT_TASK_CTX_S* taskCtx)
{
    HI_BOOL mdReg = HI_FALSE;
    HI_S32 mdIndex = -1;
    HI_MUTEX_LOCK(taskCtx->taskMutex);
    if (taskCtx->taskState != VIDEODETECT_TASK_STATE_STARTED)
    {
        HI_MUTEX_UNLOCK(taskCtx->taskMutex);
        //MLOGI("Task[%d] not started Failed.\n",taskCtx->taskHdl);
        return HI_FALSE;
    }

    HI_MUTEX_LOCK(taskCtx->regMutex);
    mdReg = VIDEODETECT_GetAlgRegisterStatus(HI_VIDEODETECT_ALG_TYPE_MD, taskCtx, &mdIndex);
    if (!mdReg)
    {
        HI_MUTEX_UNLOCK(taskCtx->regMutex);
        HI_MUTEX_UNLOCK(taskCtx->taskMutex);
        //MLOGI("Task[%d] not reg Failed.\n",taskCtx->taskHdl);
        return HI_FALSE;
    }
    HI_MUTEX_UNLOCK(taskCtx->regMutex);
    HI_MUTEX_UNLOCK(taskCtx->taskMutex);
    return HI_TRUE;
}

/**dump YUV & put YUVNode to queue*/
static HI_S32 VIDEODETECT_AlgProc_DumpYUV(VIDEODETECT_TASK_CTX_S* taskCtx)
{
    HI_S32 ret = HI_SUCCESS;
    HI_HANDLE grpHdl = taskCtx->taskCfg.yuvSrc.modHdl;
    HI_HANDLE portHdl = taskCtx->taskCfg.yuvSrc.chnHdl;
    HI_HANDLE queueHdl = taskCtx->queueHdl;
    HI_MAPI_FRAME_DATA_S dumpFrameData = {0};

    HI_MUTEX_LOCK(taskCtx->taskMutex);
    ret = HI_MAPI_VPROC_GetPortFrame(grpHdl, portHdl, HI_FALSE, &dumpFrameData);
    if(HI_SUCCESS != ret)
    {
        HI_MUTEX_UNLOCK(taskCtx->taskMutex);
        MLOGE("Task[%d] HI_MAPI_VPROC_GetChnFrame Failed. Continue to get next frame.\n",taskCtx->taskHdl);
        return HI_FAILURE;
    }

    ret = VIDEODETECT_QUEUE_Push(queueHdl, &dumpFrameData);
    if(HI_SUCCESS != ret)
    {
        HI_MUTEX_UNLOCK(taskCtx->taskMutex);
        MLOGE("Task[%d] VIDEODETECT_QUEUE_Push Failed. Continue to get next frame.\n",taskCtx->taskHdl);
        return HI_FAILURE;
    }
    HI_MUTEX_UNLOCK(taskCtx->taskMutex);
    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 VIDEODETECT_AlgProc_ProcessMD(VIDEODETECT_TASK_CTX_S* taskCtx)
{
    HI_S32 ret = HI_SUCCESS;
    HI_BOOL mdReg = HI_FALSE;
    HI_S32 mdIndex = -1;
    HI_BOOL readNodeSuccess = HI_FALSE;
    HI_S32 i = 0;
    HI_MAPI_FRAME_DATA_S readFrameData[VIDEODETECT_MDRPOC_FRAME_NUM] = {0};

    HI_MUTEX_LOCK(taskCtx->regMutex);

    mdReg = VIDEODETECT_GetAlgRegisterStatus(HI_VIDEODETECT_ALG_TYPE_MD, taskCtx, &mdIndex);

    VIDEODETECT_QUEUE_Read(taskCtx->queueHdl, VIDEODETECT_MDRPOC_FRAME_NUM,
        readFrameData, &readNodeSuccess);
    /**algProc has Registered and can get enought yuvnode from queue*/
    if (readNodeSuccess && mdReg)
    {
        HI_VIDEODETECT_DATA_S mdData = {0};
        ret = VIDEODETECT_MD_Process(taskCtx->taskHdl, &(readFrameData[0]), &(readFrameData[1]), &(mdData.unData.mdInfo.iveCCBlob));
        if(HI_SUCCESS != ret)
        {
            MLOGE("Task[%d] VIDEODETECT_MD_Process Failed. Continue to do next proc.\n",taskCtx->taskHdl);
            HI_MUTEX_UNLOCK(taskCtx->regMutex);
            return HI_FAILURE;
        }

        mdData.algType = HI_VIDEODETECT_ALG_TYPE_MD;
        for (i = 0; i < HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX; i++)
        {
            if (HI_NULL != taskCtx->taskCallbackArray[mdIndex].taskCallback[i].fnGetParamCallBack)
            {
                taskCtx->taskCallbackArray[mdIndex].taskCallback[i].fnGetParamCallBack(&mdData,
                    taskCtx->taskCallbackArray[mdIndex].taskCallback[i].privateData);
            }
        }
    }
    HI_MUTEX_UNLOCK(taskCtx->regMutex);

    return HI_SUCCESS;
}
#endif

/**pop YUVNode to queue & removeYUV*/
static HI_S32 VIDEODETECT_AlgProc_ReleaseYUV(VIDEODETECT_TASK_CTX_S* taskCtx)
{
    HI_S32 ret = HI_SUCCESS;
    HI_HANDLE grpHdl = taskCtx->taskCfg.yuvSrc.modHdl;
    HI_HANDLE portHdl = taskCtx->taskCfg.yuvSrc.chnHdl;
    HI_HANDLE queueHdl = taskCtx->queueHdl;
    HI_BOOL popNodeSuccess = HI_FALSE;
    HI_MAPI_FRAME_DATA_S releaseFrameData = {0};
    HI_MUTEX_LOCK(taskCtx->taskMutex);
    ret =  VIDEODETECT_QUEUE_Pop(queueHdl, &releaseFrameData, &popNodeSuccess);
    if (popNodeSuccess && HI_SUCCESS == ret)
    {
        ret = HI_MAPI_VPROC_ReleasePortFrame(grpHdl, portHdl, HI_FALSE, &releaseFrameData);
        if(HI_SUCCESS != ret)
        {
            HI_MUTEX_UNLOCK(taskCtx->taskMutex);
            MLOGE("Task[%d] HI_MAPI_VPROC_ReleaseChnFrame Failed. Continue to do next proc.\n",taskCtx->taskHdl);
            return HI_FAILURE;
        }
    }
    HI_MUTEX_UNLOCK(taskCtx->taskMutex);
    return HI_SUCCESS;
}

static HI_VOID* VIDEODETECT_AlgProcThread(HI_VOID* pArgs)
{
    VIDEODETECT_TASK_CTX_S* taskCtx = (VIDEODETECT_TASK_CTX_S*)pArgs;
    HI_S32 ret = HI_SUCCESS;

    while (taskCtx->taskThreadProcFlag)
    {

        if (!VIDEODETECT_AlgProc_ThreadCheck(taskCtx))
        {
            HI_usleep(100*1000);
            continue;
        }

        ret = VIDEODETECT_AlgProc_DumpYUV(taskCtx);
        if (ret != HI_SUCCESS)
        {
            HI_usleep(100*1000);
            continue;
        }

#ifdef CONFIG_MOTIONDETECT_ON
        ret = VIDEODETECT_AlgProc_ProcessMD(taskCtx);
        if (ret != HI_SUCCESS)
        {
            HI_usleep(100*1000);
            continue;
        }
#endif

        ret = VIDEODETECT_AlgProc_ReleaseYUV(taskCtx);
        if (ret != HI_SUCCESS)
        {
            HI_usleep(100*1000);
            continue;
        }
    }

    return NULL;
}

HI_S32 VIDEODETECT_RegisterCallBack(HI_HANDLE taskHdl, HI_S32 algIdx,
    HI_VIDEODETECT_ALG_TYPE_E algType, VIDEODETECT_CTX_S* videoDetectCtx,
    const HI_VIDEODETECT_TASK_CALLBACK_S* algProcCB)
{
    HI_S32 i = 0;
    HI_S32 index = -1;
    for (i = 0; i < HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX; i++)
    {
        if (HI_NULL == videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[i].fnGetParamCallBack)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[index].fnGetParamCallBack =
            algProcCB->fnGetParamCallBack;
        videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[index].privateData =
            algProcCB->privateData;
        MLOGI("task is %d, mdIndex is %d, cb is %d.\n",taskHdl, algIdx, index);
        videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].algType = algType;
    }
    else
    {
        MLOGE("TskHdl[%d] Can't register.\n",taskHdl);
        return HI_VIDEODETECT_EINVAL;
    }
    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_UnRegisterCallBack(HI_HANDLE taskHdl, HI_S32 algIdx,
    VIDEODETECT_CTX_S* videoDetectCtx, const HI_VIDEODETECT_TASK_CALLBACK_S* algProcCB)
{
    HI_S32 i = 0;

    for (i = 0; i < HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX; i++)
    {
        if (algProcCB->fnGetParamCallBack ==
            videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[i].fnGetParamCallBack)
        {
            videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[i].fnGetParamCallBack = HI_NULL;
            videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[algIdx].taskCallback[i].privateData = HI_NULL;
            MLOGI("task is %d, mdIndex is %d, cb is %d.\n",taskHdl, algIdx, i);
            return HI_SUCCESS;
        }
    }
    MLOGE("Not register before.\n");
    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_GetTaskHdl(VIDEODETECT_CTX_S* videoDetectCtx, HI_HANDLE* taskHdl)
{
    HI_S32 i = 0;
    for (i = 0; i < HI_VIDEODETECT_TASK_NUM_MAX; i++)
    {
        if (videoDetectCtx->taskCtx[i].taskState == VIDEODETECT_TASK_STATE_INVALID)
        {
            break;
        }
    }

    if (i == HI_VIDEODETECT_TASK_NUM_MAX)
    {
        MLOGE(" task count can be not larger than %d \n", HI_VIDEODETECT_TASK_NUM_MAX);
        return HI_VIDEODETECT_EFULL;
    }

    *taskHdl = i;
    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_SetDumpDepth(HI_VIDEODETECT_TASK_CFG_S* taskConfig, HI_U32 dumpDepth)
{
    HI_S32 ret = HI_SUCCESS;
    /**Set DumpRawDepth*/
    if(HI_VIDEODETECT_YUV_MOD_VPSS == taskConfig->yuvSrc.yuvMode)
    {
        HI_MAPI_DUMP_YUV_ATTR_S dumpYUVAttr = {0};
        if (dumpDepth == 0)
        {
            dumpYUVAttr.bEnable = HI_FALSE;
        }
        else
        {
            dumpYUVAttr.bEnable = HI_TRUE;
        }
        dumpYUVAttr.u32Depth = dumpDepth;

        ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(taskConfig->yuvSrc.modHdl,
            taskConfig->yuvSrc.chnHdl, &dumpYUVAttr);
        if (HI_SUCCESS != ret)
        {
            MLOGE("HI_MAPI_VPROC_SetVpssDumpYUVAttr fail:%d\n", ret);
            return HI_FAILURE;
        }
    }
    else
    {
        MLOGE("Other BindMod Currently not support.\n");
        return HI_VIDEODETECT_EINVAL;
    }
    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 VIDEODETECT_CreateMdChn(HI_HANDLE taskHdl, HI_VIDEODETECT_TASK_CFG_S* taskConfig)
{
    HI_S32 ret = HI_SUCCESS;
    HI_BOOL mdAlgUsed = HI_FALSE;
    HI_S32 mdAlgIndex = -1;

    mdAlgUsed = VIDEODETECT_GetAlgUsedStatus(HI_VIDEODETECT_ALG_TYPE_MD, taskConfig, &mdAlgIndex);
    if (mdAlgUsed == HI_TRUE)
    {
        ret = VIDEODETECT_MD_CreateChn(taskHdl, &(taskConfig->algAttr[mdAlgIndex].unAttr.mdAttr));
        if (HI_SUCCESS != ret)
        {
            MLOGE("VIDEODETECT_MD_CreateChn fail:%d\n", ret);
            return HI_FAILURE;
        }
    }
    MLOGI(" Alg Used Status is %d, mdAlgIndex is %d. \n", mdAlgUsed, mdAlgIndex);
    return HI_SUCCESS;
}
#endif

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 VIDEODETECT_DestroyMdChn(HI_HANDLE taskHdl, HI_VIDEODETECT_TASK_CFG_S* taskConfig)
{
    HI_S32 ret = HI_SUCCESS;
    HI_BOOL mdAlgUsed = HI_FALSE;
    HI_S32 mdAlgIndex = -1;

    mdAlgUsed = VIDEODETECT_GetAlgUsedStatus(HI_VIDEODETECT_ALG_TYPE_MD, taskConfig, &mdAlgIndex);
    if (mdAlgUsed == HI_TRUE)
    {
        ret = VIDEODETECT_MD_DestroyChn(taskHdl);
        if (HI_SUCCESS != ret)
        {
            MLOGE("Task[%d] VIDEODETECT_MD_DestroyChn fail:%d\n", taskHdl, ret);
            return HI_FAILURE;
        }
    }
    MLOGI(" Alg Used Status is %d, mdAlgIndex is %d. \n", mdAlgUsed, mdAlgIndex);
    return HI_SUCCESS;
}
#endif

static HI_S32 VIDEODETECT_ReleaseQueueFrame(VIDEODETECT_TASK_CTX_S* taskCtx, HI_BOOL destroyQueue)
{
    HI_MAPI_FRAME_DATA_S queueFrameData[VIDEODETECT_DUMP_FRAMENUM_MAX] = {0};
    HI_U32 frameCnt = 0;
    HI_S32 i = 0;
    HI_S32 ret = HI_SUCCESS;
    if (destroyQueue)
    {
        VIDEODETECT_QUEUE_Destroy(taskCtx->queueHdl, queueFrameData, &frameCnt);
    }
    else
    {
        VIDEODETECT_QUEUE_Clear(taskCtx->queueHdl, queueFrameData, &frameCnt);
    }

    if (frameCnt > 0 && taskCtx->taskCfg.yuvSrc.yuvMode == HI_VIDEODETECT_YUV_MOD_VPSS)
    {
        for (i = 0; i < frameCnt; i++)
        {
            ret = HI_MAPI_VPROC_ReleasePortFrame(taskCtx->taskCfg.yuvSrc.modHdl,
                taskCtx->taskCfg.yuvSrc.chnHdl, HI_FALSE, &queueFrameData[i]);
            if(HI_SUCCESS != ret)
            {
                MLOGE("HI_MAPI_VPROC_ReleaseChnFrame Failed. \n");
                return HI_VIDEODETECT_EOTHER;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_Init(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i, j = 0;

    HI_MUTEX_LOCK(g_videoDetectInitMutex);
    if (g_videoDetectInitFlg)
    {
        HI_MUTEX_UNLOCK(g_videoDetectInitMutex);
        MLOGE("has already inited!\n");
        return HI_VIDEODETECT_EINITIALIZED;
    }
    HI_MUTEX_INIT_LOCK(g_videoDetectCtx.ctxMutex);

    for (i = 0; i < HI_VIDEODETECT_TASK_NUM_MAX; i++)
    {
        memset(&g_videoDetectCtx.taskCtx[i], 0x0, sizeof(VIDEODETECT_TASK_CTX_S));
        HI_MUTEX_INIT_LOCK(g_videoDetectCtx.taskCtx[i].taskMutex);
        HI_MUTEX_INIT_LOCK(g_videoDetectCtx.taskCtx[i].regMutex);
        g_videoDetectCtx.taskCtx[i].taskThreadCheckId = -1;
        for (j = 0; j < HI_VIDEODETECT_ALG_TYPE_BUTT; j++)
        {
            g_videoDetectCtx.taskCtx[i].taskCfg.algAttr[j].algType = HI_VIDEODETECT_ALG_TYPE_BUTT;
        }
        g_videoDetectCtx.taskCtx[i].taskState = VIDEODETECT_TASK_STATE_INVALID;
    }

#ifdef CONFIG_MOTIONDETECT_ON
    ret = VIDEODETECT_MD_Init();
    if (ret != HI_SUCCESS)
    {
        HI_MUTEX_UNLOCK(g_videoDetectInitMutex);
        MLOGE("VIDEODETECT_MD_Init failed!\n");
        return HI_VIDEODETECT_EOTHER;
    }
#endif

    g_videoDetectInitFlg = HI_TRUE;
    HI_MUTEX_UNLOCK(g_videoDetectInitMutex);
    return ret;
}

HI_S32 HI_VIDEODETECT_TaskCreate(const HI_VIDEODETECT_TASK_CFG_S* taskConfig, HI_HANDLE* taskHdl)
{
    VIDEODETECT_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(taskHdl, HI_VIDEODETECT_EINVAL);
    HI_APPCOMM_CHECK_POINTER(taskConfig, HI_VIDEODETECT_EINVAL);

    if(!VIDEODETECT_PARAMCHECK_TaskCfg(taskConfig))
    {
        return HI_VIDEODETECT_EINVAL;
    }

    HI_S32 ret = HI_SUCCESS;

    HI_MUTEX_LOCK(g_videoDetectCtx.ctxMutex);

    ret = VIDEODETECT_GetTaskHdl(&g_videoDetectCtx, taskHdl);
    if (ret != HI_SUCCESS)
    {
        HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
        return ret;
    }

    /**Taskhdl and SvpAlgChn is same*/
    g_videoDetectCtx.taskCtx[*taskHdl].taskHdl = *taskHdl;
    memcpy(&(g_videoDetectCtx.taskCtx[*taskHdl].taskCfg), taskConfig, sizeof(HI_VIDEODETECT_TASK_CFG_S));
    MLOGI(" task param check ok, taskHdl is %d. \n",*taskHdl);

    ret = VIDEODETECT_SetDumpDepth(&(g_videoDetectCtx.taskCtx[*taskHdl].taskCfg),
        taskConfig->yuvSrc.dumpDepth);
    if (ret != HI_SUCCESS)
    {
        HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
        return ret;
    }

#ifdef CONFIG_MOTIONDETECT_ON
    ret = VIDEODETECT_CreateMdChn(*taskHdl, &(g_videoDetectCtx.taskCtx[*taskHdl].taskCfg));
    if (ret != HI_SUCCESS)
    {
        HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
        return ret;
    }
#endif

    //HI_U32 nodeSize = sizeof(HI_MAPI_FRAME_DATA_S);
    g_videoDetectCtx.taskCtx[*taskHdl].queueHdl = VIDEODETECT_QUEUE_Create(sizeof(HI_MAPI_FRAME_DATA_S),
        taskConfig->yuvSrc.dumpDepth);
    if (g_videoDetectCtx.taskCtx[*taskHdl].queueHdl == 0)
    {
        HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
        MLOGE("Create Queue fail\n");
        return HI_FAILURE;
    }
    g_videoDetectCtx.taskCtx[*taskHdl].taskState = VIDEODETECT_TASK_STATE_PREPARE;
    g_videoDetectCtx.taskCtx[*taskHdl].taskThreadProcFlag = HI_TRUE;

    /**TO BE MODIFIED*/
    ret = pthread_create(&(g_videoDetectCtx.taskCtx[*taskHdl].taskThreadCheckId),
        NULL, VIDEODETECT_AlgProcThread, &(g_videoDetectCtx.taskCtx[*taskHdl]));
    if (HI_SUCCESS != ret)
    {
        g_videoDetectCtx.taskCtx[*taskHdl].taskThreadProcFlag = HI_FALSE;
        HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
        MLOGE("Create Alg Proc Thread Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    HI_MUTEX_UNLOCK(g_videoDetectCtx.ctxMutex);
    return HI_SUCCESS;
}



HI_S32 HI_VIDEODETECT_TaskDestroy(HI_HANDLE taskHdl)
{
    VIDEODETECT_CHECK_INIT();
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i, j = 0;
    VIDEODETECT_CTX_S* videoDetectCtx = &g_videoDetectCtx;

    if (VIDEODETECT_TASK_STATE_INVALID == videoDetectCtx->taskCtx[taskHdl].taskState)
    {
        MLOGE("Task has not created.\n");;
        return HI_VIDEODETECT_EOTHER;
    }

    HI_MUTEX_LOCK(videoDetectCtx->ctxMutex);
    videoDetectCtx->taskCtx[taskHdl].taskThreadProcFlag = HI_FALSE;
    ret = pthread_join(videoDetectCtx->taskCtx[taskHdl].taskThreadCheckId, NULL);
    if (ret != HI_SUCCESS)
    {
        MLOGE("Join Task Thread Fail!\n");
        HI_MUTEX_UNLOCK(videoDetectCtx->ctxMutex);
        return HI_VIDEODETECT_EOTHER;
    }

#ifdef CONFIG_MOTIONDETECT_ON
    ret = VIDEODETECT_DestroyMdChn(taskHdl, &(videoDetectCtx->taskCtx[taskHdl].taskCfg));
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_DestroyMdChn Fail!\n");
        HI_MUTEX_UNLOCK(videoDetectCtx->ctxMutex);
        return HI_VIDEODETECT_EOTHER;
    }
#endif

    ret = VIDEODETECT_ReleaseQueueFrame(&(videoDetectCtx->taskCtx[taskHdl]), HI_TRUE);
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_ReleaseQueueFrame Fail!\n");
        HI_MUTEX_UNLOCK(videoDetectCtx->ctxMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    ret = VIDEODETECT_SetDumpDepth(&(videoDetectCtx->taskCtx[taskHdl].taskCfg), 0);
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_SetDumpDepth Fail!\n");
        HI_MUTEX_UNLOCK(videoDetectCtx->ctxMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    for (i = 0; i < HI_VIDEODETECT_ALG_TYPE_BUTT; i++)
    {
        videoDetectCtx->taskCtx[taskHdl].taskCfg.algAttr[j].algType = HI_VIDEODETECT_ALG_TYPE_BUTT;
        videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[i].algType = HI_VIDEODETECT_ALG_TYPE_BUTT;
        for (j = 0; j < HI_VIDEODETECT_TASK_CALLBACK_NUM_MAX; j++)
        {
            videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[i].taskCallback[j].fnGetParamCallBack = NULL;
            videoDetectCtx->taskCtx[taskHdl].taskCallbackArray[i].taskCallback[j].privateData = HI_NULL;
        }
    }
    videoDetectCtx->taskCtx[taskHdl].taskCfg.algCnt = 0;
    videoDetectCtx->taskCtx[taskHdl].taskState = VIDEODETECT_TASK_STATE_INVALID;
    HI_MUTEX_UNLOCK(videoDetectCtx->ctxMutex);
    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_TaskStart(HI_HANDLE taskHdl)
{
    VIDEODETECT_CHECK_INIT();
    HI_MUTEX_LOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
    MLOGI("g_videoDetectCtx.taskCtx[taskHdl].taskState is %d.\n",g_videoDetectCtx.taskCtx[taskHdl].taskState);
    if (VIDEODETECT_TASK_STATE_INVALID == g_videoDetectCtx.taskCtx[taskHdl].taskState)
    {
        MLOGE("Task has not created.\n");
        HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    if (VIDEODETECT_TASK_STATE_STARTED == g_videoDetectCtx.taskCtx[taskHdl].taskState)
    {
        MLOGI("Task has already started.\n");
        HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    g_videoDetectCtx.taskCtx[taskHdl].taskState = VIDEODETECT_TASK_STATE_STARTED;
    HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);

    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_TaskStop(HI_HANDLE taskHdl)
{
    VIDEODETECT_CHECK_INIT();
    HI_MUTEX_LOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
    HI_S32 ret = HI_SUCCESS;
    if (VIDEODETECT_TASK_STATE_INVALID == g_videoDetectCtx.taskCtx[taskHdl].taskState)
    {
        MLOGE("Task has not created.\n");
        HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    if (VIDEODETECT_TASK_STATE_STOPED == g_videoDetectCtx.taskCtx[taskHdl].taskState
        || VIDEODETECT_TASK_STATE_STARTED != g_videoDetectCtx.taskCtx[taskHdl].taskState)
    {
        MLOGI("Task has already stoped or not start\n");
        HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
        return HI_VIDEODETECT_EOTHER;
    }


    ret = VIDEODETECT_ReleaseQueueFrame(&(g_videoDetectCtx.taskCtx[taskHdl]), HI_FALSE);
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_ReleaseQueueFrame Fail!\n");
        HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);
        return HI_VIDEODETECT_EOTHER;
    }

    g_videoDetectCtx.taskCtx[taskHdl].taskState = VIDEODETECT_TASK_STATE_STOPED;
    HI_MUTEX_UNLOCK(g_videoDetectCtx.taskCtx[taskHdl].taskMutex);

    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_AddAlgProc(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_CFG_S* algProcCfg)
{
    VIDEODETECT_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(algProcCfg->svpAlgType < HI_VIDEODETECT_ALG_TYPE_BUTT, HI_VIDEODETECT_EINVAL);
    HI_APPCOMM_CHECK_POINTER(algProcCfg, HI_VIDEODETECT_EINVAL);

    VIDEODETECT_CTX_S* videoDetectCtx = &g_videoDetectCtx;
    HI_S32 arrayIndex = -1;
    HI_S32 ret = HI_SUCCESS;
    HI_MUTEX_LOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
    if (videoDetectCtx->taskCtx[taskHdl].taskState == VIDEODETECT_TASK_STATE_INVALID)
    {
        HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
        MLOGE("Task has not created.\n");
        return HI_VIDEODETECT_EINVAL;
    }

    if (!VIDEODETECT_GetAlgUsedStatus(algProcCfg->svpAlgType, &(videoDetectCtx->taskCtx[taskHdl].taskCfg), &arrayIndex))
    {
        HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
        MLOGE("Alg has not used before. [%d]\n", algProcCfg->svpAlgType);
        return HI_VIDEODETECT_EINVAL;
    }

    /**cb index equals to alg index*/
    if (algProcCfg->algProcType == HI_VIDEODETECT_ALGPROC_TYPE_CALLBACK)
    {
        ret = VIDEODETECT_RegisterCallBack(taskHdl, arrayIndex, algProcCfg->svpAlgType , videoDetectCtx, &(algProcCfg->taskCB));
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Register CallBack error.\n");
            return HI_VIDEODETECT_EINVAL;
        }
    }
#ifdef CONFIG_MOTIONDETECT_ON
    /**if Event, means the module inner will set a default thread and callback to publish a event*/
    else if (algProcCfg->algProcType == HI_VIDEODETECT_ALGPROC_TYPE_EVENT)
    {
        HI_VIDEODETECT_TASK_CALLBACK_S mdAlgProcCB = {0};
        if (algProcCfg->svpAlgType  != HI_VIDEODETECT_ALG_TYPE_MD)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Currently only support MD. [%d]\n",algProcCfg->svpAlgType);
            return HI_VIDEODETECT_EINVAL;
        }

        ret = VIDEODETECT_MD_EventProcCreate(taskHdl, &(algProcCfg->unAlgProcAttr.algProcMdAttr));
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("VIDEODETECT_MD_EventProcCreate error\n");
            return HI_VIDEODETECT_EINVAL;
        }

        mdAlgProcCB.fnGetParamCallBack = VIDEODETECT_MD_AlgProc;
        mdAlgProcCB.privateData = (HI_VOID*)&(videoDetectCtx->taskCtx[taskHdl].taskHdl);
        ret = VIDEODETECT_RegisterCallBack(taskHdl, arrayIndex, algProcCfg->svpAlgType , videoDetectCtx, &mdAlgProcCB);
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Register CallBack error.\n");
            return HI_VIDEODETECT_EINVAL;
        }
    }
#endif
    HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_RemoveAlgProc(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_CFG_S* algProcCfg)
{
    VIDEODETECT_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(algProcCfg->svpAlgType  < HI_VIDEODETECT_ALG_TYPE_BUTT, HI_VIDEODETECT_EINVAL);
    HI_APPCOMM_CHECK_POINTER(algProcCfg, HI_VIDEODETECT_EINVAL);

    VIDEODETECT_CTX_S* videoDetectCtx = &g_videoDetectCtx;
    HI_S32 arrayIndex = -1;
    HI_S32 ret = HI_SUCCESS;
    HI_MUTEX_LOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
    if (videoDetectCtx->taskCtx[taskHdl].taskState == VIDEODETECT_TASK_STATE_INVALID)
    {
        HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
        MLOGE("Task has not created.\n");
        return HI_VIDEODETECT_EINVAL;
    }

    if (!VIDEODETECT_GetAlgUsedStatus(algProcCfg->svpAlgType, &(videoDetectCtx->taskCtx[taskHdl].taskCfg), &arrayIndex))
    {
        HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
        MLOGE("Alg has not used before. [%d]\n", algProcCfg->svpAlgType);
        return HI_VIDEODETECT_EINVAL;
    }

    if (algProcCfg->algProcType == HI_VIDEODETECT_ALGPROC_TYPE_CALLBACK)
    {
        ret = VIDEODETECT_UnRegisterCallBack(taskHdl, arrayIndex, videoDetectCtx, &(algProcCfg->taskCB));
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Register CallBack error.\n");
            return HI_VIDEODETECT_EINVAL;
        }
    }
#ifdef CONFIG_MOTIONDETECT_ON
    else if (algProcCfg->algProcType == HI_VIDEODETECT_ALGPROC_TYPE_EVENT)
    {
        HI_VIDEODETECT_TASK_CALLBACK_S mdAlgProcCB = {0};
        if (algProcCfg->svpAlgType  != HI_VIDEODETECT_ALG_TYPE_MD)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Currently only support MD. [%d]\n",algProcCfg->svpAlgType );
            return HI_VIDEODETECT_EINVAL;
        }

        ret = VIDEODETECT_MD_EventProcDestry(taskHdl);
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("VIDEODETECT_MD_EventProcDestry error\n");
            return HI_VIDEODETECT_EINVAL;
        }

        mdAlgProcCB.fnGetParamCallBack = VIDEODETECT_MD_AlgProc;
        mdAlgProcCB.privateData = NULL;
        ret = VIDEODETECT_UnRegisterCallBack(taskHdl, arrayIndex, videoDetectCtx, &mdAlgProcCB);
        if (ret != HI_SUCCESS)
        {
            HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
            MLOGE("Register CallBack error.\n");
            return HI_VIDEODETECT_EINVAL;
        }
    }
#endif
    HI_MUTEX_UNLOCK(videoDetectCtx->taskCtx[taskHdl].regMutex);
    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_Deinit(HI_VOID)
{
    VIDEODETECT_CHECK_INIT();
    HI_S32 i = 0;
    VIDEODETECT_CTX_S* videoDetectCtx = &g_videoDetectCtx;
    HI_MUTEX_LOCK(g_videoDetectInitMutex);
    for (i = 0; i < HI_VIDEODETECT_TASK_NUM_MAX; i++)
    {
        if(VIDEODETECT_TASK_STATE_INVALID != videoDetectCtx->taskCtx[i].taskState)
        {
            /**Just for debuging, not return error*/
            MLOGE("Task %d is not destroy.\n", i);
        }
        HI_MUTEX_DESTROY(videoDetectCtx->taskCtx[i].taskMutex);
        HI_MUTEX_DESTROY(videoDetectCtx->taskCtx[i].regMutex);
        memset(&(videoDetectCtx->taskCtx[i]), 0, sizeof(VIDEODETECT_TASK_CTX_S));
    }

#ifdef CONFIG_MOTIONDETECT_ON
    VIDEODETECT_MD_Deinit();
#endif
    HI_MUTEX_DESTROY(videoDetectCtx->ctxMutex);
    g_videoDetectInitFlg = HI_FALSE;
    HI_MUTEX_UNLOCK(g_videoDetectInitMutex);
    HI_MUTEX_DESTROY(g_videoDetectInitMutex);
    return HI_SUCCESS;
}

HI_S32 HI_VIDEODETECT_RegisterEvent(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    ret  = HI_EVTHUB_Register(HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE);
    HI_APPCOMM_CHECK_RETURN(ret, HI_FAILURE);
    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== VIDEODETECT End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
