#include "videodetect_md.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "mpi_sys.h"
#include "mpi_ive.h"
#include "hi_queue.h"

#include "hi_eventhub.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define VIDEODETECT_MD_NODE_MAX_CNT        (30)
#define VIDEODETECT_MD_IMAGE_NUM           (2)
#define VIDEODETECT_MD_IVE_ALIGN           (16)

#define VIDEODETECT_MD_MAX_THR             (1600)
#define VIDEODETECT_MD_MIN_THR             (1000)



#define VIDEODETECT_MD_DEBUG                1

//free mmz
#define VIDEODETECT_MD_MMZ_FREE(phy,vir)\
    do{\
        if ((0 != (phy)) && (0 != (vir)))\
        {\
            HI_MPI_SYS_MmzFree((phy),(HI_VOID *)(HI_UL)(vir));\
            (phy) = 0;\
            (vir) = 0;\
        }\
    }while(0)

typedef struct tagVIDEODETECT_MD_TASKCTX_S
{
    pthread_t taskThreadCheckId;
    HI_BOOL procStatus;
    HI_BOOL motionState;
    HI_HANDLE queueHdl;
    HI_U32 stableCnt;
    IVE_SRC_IMAGE_S srcImage[VIDEODETECT_MD_IMAGE_NUM];
    IVE_DST_MEM_INFO_S blobInfo;
    HI_VIDEODETECT_ALGPROC_MD_S algProcMdAttr;
} VIDEODETECT_MD_TASKCTX_S;

typedef struct tagVIDEODETECT_MD_CTX_S
{
    VIDEODETECT_MD_TASKCTX_S mdTaskCtx[HI_VIDEODETECT_TASK_NUM_MAX];
} VIDEODETECT_MD_CTX_S;
static VIDEODETECT_MD_CTX_S g_mdCtx;

HI_S32 VIDEODETECT_MD_AlgProc(HI_VIDEODETECT_DATA_S* svpData, HI_VOID* pPrivateData)
{
    HI_S32 ret = HI_SUCCESS;
    HI_HANDLE* taskHdl = (HI_HANDLE*)pPrivateData;
    HI_HANDLE queueHdl = g_mdCtx.mdTaskCtx[*taskHdl].queueHdl;
    HI_VIDEODETECT_MD_INFO_S popMdinfo = {0};
    if (svpData->algType!= HI_VIDEODETECT_ALG_TYPE_MD)
    {
        MLOGE("The AlgType is not correct. %d!\n",svpData->algType);
        return HI_VIDEODETECT_EOTHER;
    }

    ret = HI_QUEUE_Push(queueHdl, &(svpData->unData.mdInfo));
    if (ret == HI_EINVAL)
    {
        MLOGE("push queue error.\n");
        return HI_VIDEODETECT_EOTHER;
    }
    else if (ret == HI_EFULL)
    {
        HI_QUEUE_Pop(queueHdl, &popMdinfo);
        HI_QUEUE_Push(queueHdl, &(svpData->unData.mdInfo));
    }

    return HI_SUCCESS;
}

static HI_VOID* VIDEODETECT_MDProcThread(HI_VOID* pArgs)
{
    VIDEODETECT_MD_TASKCTX_S* taskCtx = (VIDEODETECT_MD_TASKCTX_S*)pArgs;
    HI_VIDEODETECT_MD_INFO_S stMdinfo = {0};
    HI_S32 ret = HI_SUCCESS;
    HI_EVENT_S stEvent;
    while (taskCtx->procStatus == HI_TRUE)
    {
        if (!HI_QUEUE_GetLen(taskCtx->queueHdl))
        {
            HI_usleep(100 * 1000);
            continue;
        }

        HI_QUEUE_Pop(taskCtx->queueHdl, &stMdinfo);
        if (ret == HI_EINVAL)
        {
            MLOGE("pop queue error.\n");
            HI_usleep(1000 * 1000);
            return NULL;
        }
        else if (ret == HI_EEMPTY)
        {
            HI_usleep(100 * 1000);
            continue;
        }

#if VIDEODETECT_MD_DEBUG
       // MLOGI("The Mdinfo Num is %d. Thr is %d, Status is %d.\n",stMdinfo.iveCCBlob.u8RegionNum, stMdinfo.iveCCBlob.u16CurAreaThr,
       //     stMdinfo.iveCCBlob.s8LabelStatus);
#endif
        if (taskCtx->motionState == HI_FALSE && stMdinfo.iveCCBlob.u8RegionNum > 0)
        {
            taskCtx->motionState = HI_TRUE;
            memset(&stEvent, '\0', sizeof(stEvent));
            stEvent.EventID = HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE;
            stEvent.arg1 = taskCtx->motionState;
            HI_EVTHUB_Publish(&stEvent);
#if VIDEODETECT_MD_DEBUG
            MLOGI("Publish Move.\n");
#endif
        }

        if (taskCtx->motionState == HI_TRUE)
        {
            if (stMdinfo.iveCCBlob.u8RegionNum == 0)
            {
                taskCtx->stableCnt++;
            }
            else
            {
                taskCtx->stableCnt = 0;
            }

            if (taskCtx->stableCnt == taskCtx->algProcMdAttr.mdStableCnt)
            {
                taskCtx->motionState = HI_FALSE;
                taskCtx->stableCnt = 0;
                memset(&stEvent, '\0', sizeof(stEvent));
                stEvent.EventID = HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE;
                stEvent.arg1 = taskCtx->motionState;
                HI_EVTHUB_Publish(&stEvent);
#if VIDEODETECT_MD_DEBUG
                MLOGI("Publish Stable.\n");
#endif
            }
        }
    }

    return NULL;
}

HI_S32 VIDEODETECT_MD_Init(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i = 0;
    ret = HI_IVS_MD_Init();
    if (HI_SUCCESS != ret)
    {
        MLOGE("HI_IVS_MD_Init Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    for (i = 0; i < HI_VIDEODETECT_TASK_NUM_MAX; i++)
    {
        memset(&(g_mdCtx.mdTaskCtx[i]), 0x0, sizeof(VIDEODETECT_MD_TASKCTX_S));
    }
    return ret;
}

HI_S32 VIDEODETECT_MD_Deinit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    ret = HI_IVS_MD_Exit();
    if (HI_SUCCESS != ret)
    {
        MLOGE("HI_IVS_MD_Init Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }
    return ret;
}

static HI_U16 VIDEODETECT_MD_CalcStride(HI_U32 width, HI_U8 align)
{
    return (width + (align - width % align) % align);
}

static HI_S32 VIDEODETECT_MD_MallocImgMem(HI_U32 size, HI_U32 height, HI_U8 imageChannel, IVE_IMAGE_S* iveImg)
{
    HI_S32 ret = HI_SUCCESS;

    ret = HI_MPI_SYS_MmzAlloc(&iveImg->au64PhyAddr[0], (HI_VOID**)&iveImg->au64VirAddr[0],
        "MD_ImgMem", HI_NULL, size);
    if (ret != HI_SUCCESS)
    {
        MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
        return ret;
    }

    if (imageChannel == 1)
    {
        return HI_SUCCESS;
    }

    if (imageChannel == 2)
    {
        iveImg->au32Stride[1] = iveImg->au32Stride[0];
        iveImg->au64PhyAddr[1] = iveImg->au64PhyAddr[0] + iveImg->au32Stride[0] * height;
        iveImg->au64VirAddr[1] = iveImg->au64VirAddr[0] + iveImg->au32Stride[0] * height;
    }
    else if (imageChannel == 3)
    {
        iveImg->au64VirAddr[1] = iveImg->au64VirAddr[0] + 1;
        iveImg->au64VirAddr[2] = iveImg->au64VirAddr[1] + 1;
        iveImg->au64PhyAddr[1] = iveImg->au64PhyAddr[0] + 1;
        iveImg->au64PhyAddr[2] = iveImg->au64PhyAddr[1] + 1;
        iveImg->au32Stride[1] = iveImg->au32Stride[0];
        iveImg->au32Stride[2] = iveImg->au32Stride[0];
    }
    return HI_SUCCESS;
}


static HI_S32 VIDEODETECT_MD_CreateImage(IVE_IMAGE_S* iveImg, IVE_IMAGE_TYPE_E imageType,
    HI_U32 width, HI_U32 height)
{
    HI_U32 u32Size = 0;
    HI_S32 ret = HI_SUCCESS;

    iveImg->enType = imageType;
    iveImg->u32Width = width;
    iveImg->u32Height = height;
    iveImg->au32Stride[0] = VIDEODETECT_MD_CalcStride(iveImg->u32Width, VIDEODETECT_MD_IVE_ALIGN);

    switch (imageType)
    {
        case IVE_IMAGE_TYPE_U8C1:
        case IVE_IMAGE_TYPE_S8C1:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height;
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 1, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }

        case IVE_IMAGE_TYPE_YUV420SP:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * 3 / 2;
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 2, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }
        case IVE_IMAGE_TYPE_YUV422SP:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * 2;
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 2, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;

        }
        case IVE_IMAGE_TYPE_S16C1:
        case IVE_IMAGE_TYPE_U16C1:
        {

            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * sizeof(HI_U16);
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 1, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }
        case IVE_IMAGE_TYPE_U8C3_PACKAGE:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * 3;
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 3, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }
        case IVE_IMAGE_TYPE_S32C1:
        case IVE_IMAGE_TYPE_U32C1:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * sizeof(HI_U32);
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 1, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }
        case IVE_IMAGE_TYPE_S64C1:
        case IVE_IMAGE_TYPE_U64C1:
        {
            u32Size = iveImg->au32Stride[0] * iveImg->u32Height * sizeof(HI_U64);
            ret = VIDEODETECT_MD_MallocImgMem(u32Size, height, 1, iveImg);
            if (ret != HI_SUCCESS)
            {
                MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
                return ret;
            }
            break;
        }
        default:
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_MD_CreateMemInfo(IVE_MEM_INFO_S* memInfo, HI_U32 size)
{
    HI_S32 ret = HI_SUCCESS;

    memInfo->u32Size = size;
    ret = HI_MPI_SYS_MmzAlloc(&memInfo->u64PhyAddr, (HI_VOID**)&memInfo->u64VirAddr, "MD_MemInfo", HI_NULL, size);
    if (ret != HI_SUCCESS)
    {
         MLOGE("Mmz Alloc fail,Error(%#x)\n", ret);
         return ret;
    }

    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_MD_DestroyImage(IVE_IMAGE_S* iveImg)
{
    VIDEODETECT_MD_MMZ_FREE(iveImg->au64PhyAddr[0],iveImg->au64VirAddr[0]);
    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_MD_DestroyMemInfo(IVE_MEM_INFO_S* memInfo)
{
    VIDEODETECT_MD_MMZ_FREE(memInfo->u64PhyAddr,memInfo->u64VirAddr);
    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_MD_CreateChn(HI_HANDLE taskHdl, const MD_ATTR_S* mdAttr)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_U32 blobSize = 0;
    for (i = 0; i < VIDEODETECT_MD_IMAGE_NUM; i++)
    {
        ret = VIDEODETECT_MD_CreateImage(&(g_mdCtx.mdTaskCtx[taskHdl].srcImage[i]),
            IVE_IMAGE_TYPE_U8C1, mdAttr->u32Width, mdAttr->u32Height);
        if (HI_SUCCESS != ret)
        {
            MLOGE("VIDEODETECT_MD_CreateImage Fail!\n");
            return HI_VIDEODETECT_EOTHER;
        }
    }

    blobSize = sizeof(IVE_CCBLOB_S);

    ret = VIDEODETECT_MD_CreateMemInfo(&(g_mdCtx.mdTaskCtx[taskHdl].blobInfo), blobSize);
    if (HI_SUCCESS != ret)
    {
        for (i = 0; i < VIDEODETECT_MD_IMAGE_NUM; i++)
        {
            VIDEODETECT_MD_DestroyImage(&(g_mdCtx.mdTaskCtx[taskHdl].srcImage[i]));
        }
        MLOGE("SVPMNG_MD_CreateImage Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    MD_ATTR_S currentMdAttr = {0};
    memcpy(&currentMdAttr, mdAttr, sizeof(MD_ATTR_S));
    /**taskHdl same as the MDCHN*/
    ret = HI_IVS_MD_CreateChn(taskHdl, &currentMdAttr);
    if (HI_SUCCESS != ret)
    {
        for (i = 0; i < VIDEODETECT_MD_IMAGE_NUM; i++)
        {
            VIDEODETECT_MD_DestroyImage(&(g_mdCtx.mdTaskCtx[taskHdl].srcImage[i]));
        }
        VIDEODETECT_MD_DestroyMemInfo(&(g_mdCtx.mdTaskCtx[taskHdl].blobInfo));
        MLOGE("HI_IVS_MD_CreateChn Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_MD_DestroyChn(HI_HANDLE taskHdl)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i = 0;
    for (i = 0; i < VIDEODETECT_MD_IMAGE_NUM; i++)
    {
        VIDEODETECT_MD_DestroyImage(&(g_mdCtx.mdTaskCtx[taskHdl].srcImage[i]));
    }

    VIDEODETECT_MD_DestroyMemInfo(&(g_mdCtx.mdTaskCtx[taskHdl].blobInfo));
    ret = HI_IVS_MD_DestroyChn(taskHdl);
    if (HI_SUCCESS != ret)
    {
        MLOGE("HI_IVS_MD_DestroyChn Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }
    return HI_SUCCESS;
}

static HI_S32 VIDEODETECT_MD_DmaImage(HI_MAPI_FRAME_DATA_S* frameData,
    IVE_DST_IMAGE_S* dstImage, HI_BOOL instant)
{
    HI_S32 ret = HI_SUCCESS;
    IVE_HANDLE hIveHandle = -1;
    IVE_SRC_DATA_S srcData = {0};
    IVE_DST_DATA_S dstData = {0};
    IVE_DMA_CTRL_S dmaCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
    HI_BOOL finish = HI_FALSE;
    HI_BOOL block = HI_TRUE;

    srcData.u64PhyAddr = frameData->u64PhyAddr[0];
    srcData.u32Width   = frameData->u32Width;
    srcData.u32Height  = frameData->u32Height;
    srcData.u32Stride  = frameData->u32Stride[0];

    dstData.u64PhyAddr = dstImage->au64PhyAddr[0];
    dstData.u32Width   = dstImage->u32Width;
    dstData.u32Height  = dstImage->u32Height;
    dstData.u32Stride  = dstImage->au32Stride[0];

    ret = HI_MPI_IVE_DMA(&hIveHandle, &srcData, &dstData, &dmaCtrl, instant);
    if (ret != HI_SUCCESS)
    {
        MLOGE("HI_MPI_IVE_DMA Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    if (HI_TRUE == instant)
    {
        ret = HI_MPI_IVE_Query(hIveHandle, &finish, block);
        while(HI_ERR_IVE_QUERY_TIMEOUT == ret)
        {
            HI_usleep(100);
            ret = HI_MPI_IVE_Query(hIveHandle, &finish, block);
        }
    }

    return HI_SUCCESS;
}


HI_S32 VIDEODETECT_MD_Process(HI_HANDLE taskHdl, HI_MAPI_FRAME_DATA_S* pstCurFrame,
    HI_MAPI_FRAME_DATA_S* pstRefFrame, IVE_CCBLOB_S* pstBlob)
{
    HI_S32 ret = HI_SUCCESS;
    ret = VIDEODETECT_MD_DmaImage(pstCurFrame, &(g_mdCtx.mdTaskCtx[taskHdl].srcImage[0]),HI_TRUE);
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_MD_DmaImage Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    ret = VIDEODETECT_MD_DmaImage(pstRefFrame, &(g_mdCtx.mdTaskCtx[taskHdl].srcImage[1]),HI_TRUE);
    if (ret != HI_SUCCESS)
    {
        MLOGE("VIDEODETECT_MD_DmaImage Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    ret = HI_IVS_MD_Process(taskHdl, &(g_mdCtx.mdTaskCtx[taskHdl].srcImage[0]),&(g_mdCtx.mdTaskCtx[taskHdl].srcImage[1]),
        NULL, &(g_mdCtx.mdTaskCtx[taskHdl].blobInfo));
    if (ret != HI_SUCCESS)
    {
        MLOGE("HI_IVS_MD_Process Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    memcpy(pstBlob, (IVE_CCBLOB_S*)(HI_UL)(g_mdCtx.mdTaskCtx[taskHdl].blobInfo.u64VirAddr), sizeof(IVE_CCBLOB_S));

    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_MD_EventProcCreate(HI_HANDLE taskHdl, const HI_VIDEODETECT_ALGPROC_MD_S* algProcMdAttr)
{
    HI_U32 nodeSize = (HI_U32)sizeof(HI_VIDEODETECT_MD_INFO_S);
    HI_S32 ret = HI_SUCCESS;
    if (algProcMdAttr->mdSensitivity < 0 || algProcMdAttr->mdSensitivity > 100)
    {
        MLOGE("mdSensitivity is out of range [0, 100]. %d\n", algProcMdAttr->mdSensitivity);
        return HI_FAILURE;
    }
    g_mdCtx.mdTaskCtx[taskHdl].queueHdl = HI_QUEUE_Create(nodeSize,VIDEODETECT_MD_NODE_MAX_CNT);
    if (g_mdCtx.mdTaskCtx[taskHdl].queueHdl == 0)
    {
        MLOGE("queue create error.\n");
        return HI_FAILURE;
    }
    g_mdCtx.mdTaskCtx[taskHdl].algProcMdAttr.mdSensitivity = algProcMdAttr->mdSensitivity;
    g_mdCtx.mdTaskCtx[taskHdl].algProcMdAttr.mdStableCnt = algProcMdAttr->mdStableCnt;
    g_mdCtx.mdTaskCtx[taskHdl].procStatus = HI_TRUE;
    g_mdCtx.mdTaskCtx[taskHdl].motionState = HI_FALSE;
    g_mdCtx.mdTaskCtx[taskHdl].stableCnt = 0;
    MD_ATTR_S mdAttr = {0};
    ret =  HI_IVS_MD_GetChnAttr(taskHdl, &mdAttr);
    if (HI_SUCCESS != ret)
    {
        MLOGE("HI_IVS_MD_GetChnAttr Thread Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }
    mdAttr.u16SadThr = (VIDEODETECT_MD_MAX_THR - VIDEODETECT_MD_MIN_THR) / 100 *
        (100 - algProcMdAttr->mdSensitivity) + VIDEODETECT_MD_MIN_THR;

    ret =  HI_IVS_MD_SetChnAttr(taskHdl, &mdAttr);
    if (HI_SUCCESS != ret)
    {
        MLOGE("HI_IVS_MD_SetChnAttr Thread Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }
    HI_EVENT_S stEvent;
    memset(&stEvent, '\0', sizeof(stEvent));
    stEvent.EventID = HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE;
    stEvent.arg1 = g_mdCtx.mdTaskCtx[taskHdl].motionState;
    HI_EVTHUB_Publish(&stEvent);

    ret = pthread_create(&(g_mdCtx.mdTaskCtx[taskHdl].taskThreadCheckId), NULL,
        VIDEODETECT_MDProcThread, &(g_mdCtx.mdTaskCtx[taskHdl]));
    if (HI_SUCCESS != ret)
    {
        MLOGE("Create Alg Proc Thread Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    return HI_SUCCESS;
}

HI_S32 VIDEODETECT_MD_EventProcDestry(HI_HANDLE taskHdl)
{
    HI_S32 ret = HI_SUCCESS;
    g_mdCtx.mdTaskCtx[taskHdl].procStatus = HI_FALSE;
    ret = pthread_join(g_mdCtx.mdTaskCtx[taskHdl].taskThreadCheckId, NULL);
    if (HI_SUCCESS != ret)
    {
        MLOGE("Join MD check Thread Fail!\n");
        return HI_VIDEODETECT_EOTHER;
    }

    HI_QUEUE_Destroy(g_mdCtx.mdTaskCtx[taskHdl].queueHdl);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

