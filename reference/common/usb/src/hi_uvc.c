/**
 * @file      hi_uvc.c
 * @brief     uvc mode interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#ifdef __HuaweiLite__
#include "implementation/usb_init.h"
#endif
#include "hi_mapi_aenc_define.h"
#include "hi_mapi_aenc.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_venc_define.h"
#include "hi_mapi_comm_define.h"

#include "hi_uvc.h"
#include "hi_uvc_gadget.h"
#include "uvc_framebuffer.h"
#include "hi_appcomm_util.h"
#include "hi_timestamp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** uvc buffer macro */
//#define UVC_BUF_SIZE    (2073600)
//#define UVC_BUF_SIZE    (16588800)
//#define UVC_BUF_COUNT   (6)
#define UVC_TIMELAPSE_CHECK (0)

#if UVC_TIMELAPSE_CHECK
#define UVC_TIMELAPSE_CHECK_COUNT (60)
static HI_S32 g_s32TimeLapseCount = 0;
#endif
/** UVC Stream Context */
typedef struct tagUVC_STREAM_CONTEXT_S
{
    HI_UVC_DEVICE_CAP_S  stDeviceCap;
    HI_UVC_DATA_SOURCE_S stDataSource;
    UVC_STREAM_ATTR_S stStreamAttr;   /**<stream attribute, update by uvc driver */
    HI_BOOL bVportStart;
    HI_BOOL bVencStart;
    HI_BOOL bFirstFrame;
    HI_BOOL bInited;
} UVC_STREAM_CONTEXT_S;
static UVC_STREAM_CONTEXT_S s_stUVCStreamCtx;

/** UVC Context */
static UVC_CONTEXT_S s_stUVCCtx =
{
    .bRun = HI_FALSE,
    .bPCConnect = HI_FALSE,
    .TskId = -1,
    .Tsk2Id = -1
};

HI_BOOL g_bPushVencData = HI_FALSE;
/**------------------- UVC STREAM Start ------------------------- */

static HI_S32 UVC_STREAM_VencDataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S* pVStreamData, HI_VOID *pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pVStreamData, HI_FAILURE);

    if(HI_FALSE == g_bPushVencData)
    {
        //MLOGD("no need push VencData.\n");
        return HI_SUCCESS;
    }

    UVC_FRAMEBUF_QUEUES_S* pstQueues = UVC_FRAMEBUF_GetQueues();
    UVC_FRAME_NODE_S* pstNode = NULL;

    UVC_FRAMEBUF_GetNodeFromQueue(&pstQueues->stIdle, &pstNode);
    if (!pstNode)
    {
        MLOGE("get node from Queue faiked.\n");
        return HI_FAILURE;
    }
    pstNode->u32used = 0;
    pstNode->offset = 0;

    HI_U32 i=0;

    for ( i = 0 ; i < pVStreamData->u32PackCount; i++ )
    {
            HI_U64 u64PackPhyAddr = pVStreamData->astPack[i].au64PhyAddr[0];
            HI_U32 u32PackLen = pVStreamData->astPack[i].au32Len[0];
            HI_U32 u32PackOffset = pVStreamData->astPack[i].u32Offset;

            HI_U8 * s_pDataVirtAddr = pVStreamData->astPack[i].apu8Addr[1];
            HI_U64 s_u64DataPhyAddr = pVStreamData->astPack[i].au64PhyAddr[1];
            HI_U32 s_u32DataLen = pVStreamData->astPack[i].au32Len[1];
            HI_U8* pPackVirtAddr = pVStreamData->astPack[i].apu8Addr[0];

            if (u64PackPhyAddr + u32PackLen >= s_u64DataPhyAddr + s_u32DataLen)
            {
                /* physical address retrace in offset segment */
                if (u64PackPhyAddr + u32PackOffset >= s_u64DataPhyAddr + s_u32DataLen)
                {
                    HI_U8* pSrcVirtAddr = (HI_U8*)s_pDataVirtAddr +
                                            ( (u64PackPhyAddr + u32PackOffset) - (s_u64DataPhyAddr + s_u32DataLen) );
                    memcpy((pstNode->pu8Mem+ pstNode->u32used),pSrcVirtAddr,(u32PackLen - u32PackOffset));
                    pstNode->u32used+=(u32PackLen - u32PackOffset);

                }
                else
                {
                    /* physical address retrace in data segment */
                    HI_U32 u32Left = (s_u64DataPhyAddr + s_u32DataLen) - u64PackPhyAddr;
                    memcpy((pstNode->pu8Mem+ pstNode->u32used),(pPackVirtAddr + u32PackOffset),(u32Left - u32PackOffset));
                    pstNode->u32used+=(u32Left - u32PackOffset);

                    memcpy((pstNode->pu8Mem+ pstNode->u32used),s_pDataVirtAddr,(u32PackLen - u32Left));
                    pstNode->u32used+=(u32PackLen - u32Left);
                }
            }
            else
            {
                /* physical address retrace does not happen */
                memcpy((pstNode->pu8Mem+ pstNode->u32used),(pPackVirtAddr + u32PackOffset),(u32PackLen - u32PackOffset));
                pstNode->u32used+=(u32PackLen - u32PackOffset);
            }
        }

    UVC_FRAMEBUF_PutNodeToQueue(&pstQueues->stReady, pstNode);
    if (s_stUVCStreamCtx.bFirstFrame)
    {
        UVC_FRAME_NODE_S *pstNode2 = NULL;
        UVC_FRAMEBUF_GetNodeFromQueue(&pstQueues->stIdle, &pstNode2);

        if (pstNode2)
        {
            memcpy(pstNode2->pu8Mem, pstNode->pu8Mem, pstNode->u32used);
            pstNode2->u32used = pstNode->u32used;
            UVC_FRAMEBUF_PutNodeToQueue(&pstQueues->stReady, pstNode2);
        }

        s_stUVCStreamCtx.bFirstFrame = HI_FALSE;
    }

#ifdef __HuaweiLite__
    extern EVENT_CB_S g_frame_event;
    LOS_EventWrite(&g_frame_event, 0x01);
#endif

#if UVC_TIMELAPSE_CHECK
    if(g_s32TimeLapseCount < UVC_TIMELAPSE_CHECK_COUNT)
    {
        HI_TIME_STAMP;
        g_s32TimeLapseCount++;
    }
#endif
    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_VPortDataProc(HI_HANDLE VprocHdl, HI_HANDLE VportHdl, HI_MAPI_FRAME_DATA_S* pstVportYUV, HI_VOID* pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pstVportYUV, HI_FAILURE);

    HI_U8* pUserPageAddr = HI_NULL;
    HI_CHAR* pCVBufVirt_Y = HI_NULL;
    HI_CHAR* pCVBufVirt_C = HI_NULL;
    HI_CHAR* pCMemContent = HI_NULL;
    HI_U32 u32W, u32H = 0;
    HI_U32 u32CopySize=0;
    HI_U32 u32UvHeight = 0;
    HI_CHAR cTmpBuf[4096] = {0};

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pstVportYUV->enPixelFormat)
    {
        u32UvHeight = pstVportYUV->u32Height / 2;
    }
    else if(PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pstVportYUV->enPixelFormat)
    {
        u32UvHeight = pstVportYUV->u32Height;
    }
    else
    {
        MLOGE("enPixelFormat error.\n");
        return HI_FAILURE;
    }
    //MLOGD("Frame[%ux%u]\n", pVPortYUV->u32Width, pVPortYUV->u32Height);
    //MLOGD("stride[%u, %u]\n", pVPortYUV->u32Stride[0], pVPortYUV->u32Stride[1]);
    //MLOGD("FrameSize[%u], UV Height[%u]\n", u32FrameSize, u32UVHeight);

#if UVC_TIMELAPSE_CHECK
    MLOGW(YELLOW"vport pts(%lld)\n"NONE, pstVportYUV->u64pts);
#endif

    UVC_FRAME_NODE_S* pstNode = NULL;
    UVC_FRAMEBUF_GetNodeFromQueue(&UVC_FRAMEBUF_GetQueues()->stIdle, &pstNode);
    pUserPageAddr = (HI_U8*)(HI_UL)pstVportYUV->u64VirAddr[0];
    if (HI_NULL == pUserPageAddr)
    {
        MLOGE("Error VirAddr.\n");
        return HI_FAILURE;
    }

    pCVBufVirt_Y = (HI_CHAR*)pUserPageAddr;
    pCVBufVirt_C = pCVBufVirt_Y + (pstVportYUV->u32Stride[0]) * (pstVportYUV->u32Height);

    /* save Y ----------------------------------------------------------------*/
    for (u32H = 0; u32H < pstVportYUV->u32Height; u32H++)
    {
        pCMemContent = pCVBufVirt_Y + u32H * pstVportYUV->u32Stride[0];
        u32CopySize=pstVportYUV->u32Width;
        if((pstNode->u32used+u32CopySize)>pstNode->u32BufLen)
        {
            MLOGE("No buf for save yuv !\n");
            return HI_FAILURE;
        }
        memcpy((pstNode->pu8Mem+ pstNode->u32used),pCMemContent,u32CopySize);
        pstNode->u32used+=u32CopySize;
    }

    /* save U ----------------------------------------------------------------*/
    for (u32H = 0; u32H< u32UvHeight; u32H++)
    {
        pCMemContent = pCVBufVirt_C + u32H * pstVportYUV->u32Stride[1];
        pCMemContent += 1;
        for (u32W = 0; u32W < pstVportYUV->u32Width / 2; u32W++)
        {
            cTmpBuf[u32W] = *pCMemContent;
            pCMemContent += 2;
        }
        u32CopySize=pstVportYUV->u32Width / 2;
        if((pstNode->u32used+u32CopySize)>pstNode->u32BufLen)
        {
            MLOGE("No buf for save yuv !\n");
            return HI_FAILURE;
        }
        memcpy((pstNode->pu8Mem+ pstNode->u32used),cTmpBuf,u32CopySize);
        pstNode->u32used+=u32CopySize;
    }

    /* save V ----------------------------------------------------------------*/

    for (u32H = 0; u32H < u32UvHeight; u32H++)
    {
        pCMemContent = pCVBufVirt_C + u32H * pstVportYUV->u32Stride[1];
        for (u32W = 0; u32W < pstVportYUV->u32Width / 2; u32W++)
        {
            cTmpBuf[u32W] = *pCMemContent;
            pCMemContent += 2;
        }
        u32CopySize=pstVportYUV->u32Width / 2;
        if((pstNode->u32used+u32CopySize)>pstNode->u32BufLen)
        {
            MLOGE("No buf for save yuv !\n");
            return HI_FAILURE;
        }
        memcpy((pstNode->pu8Mem+ pstNode->u32used),cTmpBuf,u32CopySize);
        pstNode->u32used+=u32CopySize;
    }
    pUserPageAddr = HI_NULL;

    UVC_FRAMEBUF_PutNodeToQueue(&UVC_FRAMEBUF_GetQueues()->stReady, pstNode);
    MLOGD("FrameSize[%u], UV Height[%u] u32Width[%u] C\n", pstNode->u32used,pstVportYUV->u32Height,pstVportYUV->u32Width);
    return HI_SUCCESS;
}

static HI_VOID* YUV_DumpTask(HI_VOID *pvArg)
{
    HI_S32 s32Ret;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    /* Enable Dump */
    HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
    stDumpYUVAttr.bEnable = HI_TRUE;
    stDumpYUVAttr.u32Depth = 3;

    HI_DUMP_YUV_CALLBACK_FUNC_S stDumpYuvCb;
    stDumpYuvCb.pfunVProcYUVProc = UVC_STREAM_VPortDataProc;
    stDumpYuvCb.pPrivateData =(HI_VOID*)__FUNCTION__;
    while(1)
    {
        stDumpYUVAttr.bEnable = HI_TRUE;
        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstSrc->VprocHdl,pstSrc->VportHdl, &stDumpYUVAttr);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "SetVpssDumpYUVAttr");
        s32Ret = HI_MAPI_VPROC_StartVpssDumpYUV(pstSrc->VprocHdl,pstSrc->VportHdl,5, &stDumpYuvCb);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "StartVpssDumpYUV");
        sleep(1);
        s32Ret = HI_MAPI_VPROC_StopPort(pstSrc->VprocHdl,pstSrc->VportHdl);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "StopPort");
        stDumpYUVAttr.bEnable = HI_FALSE;
        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstSrc->VprocHdl,pstSrc->VportHdl, &stDumpYUVAttr);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "SetVpssDumpYUVAttr");
        sleep(1);
    }
    return NULL;
}

#if UVC_TIMELAPSE_CHECK
static HI_S32 UVC_STREAM_VPortDataProc_TimeLapseCheck(HI_HANDLE VprocHdl, HI_HANDLE VportHdl, HI_MAPI_FRAME_DATA_S* pstVportYUV, HI_VOID* pvPrivData)
{
    HI_APPCOMM_CHECK_POINTER(pstVportYUV, HI_FAILURE);
    //MLOGD("Frame[%ux%u]\n", pVPortYUV->u32Width, pVPortYUV->u32Height);
    //MLOGD("stride[%u, %u]\n", pVPortYUV->u32Stride[0], pVPortYUV->u32Stride[1]);
    //MLOGD("FrameSize[%u], UV Height[%u]\n", u32FrameSize, u32UVHeight);

    HI_TIME_STAMP;
    //MLOGW(YELLOW"tcount(%d) vport pts(%lld)\n"NONE, g_s32TimeLapseCount, pstVportYUV->u64pts);

    return HI_SUCCESS;
}

static HI_VOID* YUV_TimeLapseCheck(HI_VOID *pvArg)
{
    HI_S32 s32Ret;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    /* Enable Dump */
    HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
    stDumpYUVAttr.bEnable = HI_TRUE;
    stDumpYUVAttr.u32Depth = 3;

    HI_DUMP_YUV_CALLBACK_FUNC_S stDumpYuvCb;
    stDumpYuvCb.pfunVProcYUVProc = UVC_STREAM_VPortDataProc_TimeLapseCheck;
    stDumpYuvCb.pPrivateData =(HI_VOID*)__FUNCTION__;
    while(1)
    {
        stDumpYUVAttr.bEnable = HI_TRUE;
        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstSrc->VprocHdl,pstSrc->VportHdl, &stDumpYUVAttr);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "SetVpssDumpYUVAttr");
        s32Ret = HI_MAPI_VPROC_StartVpssDumpYUV(pstSrc->VprocHdl, pstSrc->VportHdl, UVC_TIMELAPSE_CHECK_COUNT, &stDumpYuvCb);
        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN((HI_SUCCESS == s32Ret), "StartVpssDumpYUV");
        break;
    }
    return NULL;
}
#endif

static HI_S32 UVC_STREAM_VPortStart(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    UVC_STREAM_ATTR_S *pstAttr = &s_stUVCStreamCtx.stStreamAttr;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;

    /* Set Vport Attribute */
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    MLOGD("prochdl[%d] porthdl[%d]\n", pstSrc->VprocHdl, pstSrc->VportHdl);
    s32Ret = HI_MAPI_VPROC_GetPortAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stVPortAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VPROC_GetPortAttr");

    stVPortAttr.stFrameRate.s32SrcFrameRate = pstAttr->u32Fps;
    stVPortAttr.stFrameRate.s32DstFrameRate = pstAttr->u32Fps;
    stVPortAttr.u32Width  = pstAttr->u32Width;
    stVPortAttr.u32Height = pstAttr->u32Height;
    MLOGD("VProc Port Attr: %ux%u,src:%ufps,dst:%ufps\n", stVPortAttr.u32Width,
        stVPortAttr.u32Height, stVPortAttr.stFrameRate.s32SrcFrameRate, stVPortAttr.stFrameRate.s32DstFrameRate);
    s32Ret = HI_MAPI_VPROC_SetPortAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stVPortAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VProc_Port_SetAttr");

    /* Start Vport */
    s32Ret = HI_MAPI_VPROC_StartPort(pstSrc->VprocHdl, pstSrc->VportHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VProc_Port_Start");

    if(pthread_create(&s_stUVCCtx.Tsk2Id, NULL, YUV_DumpTask, NULL))
    {
        MLOGE("YUV_DumpTask create failed\n");
        s_stUVCCtx.bRun = HI_FALSE;
        return HI_FAILURE;
    }

    /* Dump */
    s_stUVCStreamCtx.bVportStart = HI_TRUE;

    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_VPortStop(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    MLOGD("\n");

    if (s_stUVCStreamCtx.bVportStart)
    {
        pthread_join(s_stUVCCtx.Tsk2Id, NULL);
        s32Ret = HI_MAPI_VPROC_StopVpssDumpYUV(pstSrc->VprocHdl, pstSrc->VportHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VPROC_StopVpssDumpYUV(%d,%d) failed[%08x]\n", pstSrc->VprocHdl, pstSrc->VportHdl, s32Ret);
        }

        HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
        stDumpYUVAttr.bEnable = HI_FALSE;
        stDumpYUVAttr.u32Depth = 1;
        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stDumpYUVAttr);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VPROC_SetVpssDumpYUVAttr(%d,%d) failed[%08x]\n", pstSrc->VprocHdl, pstSrc->VportHdl, s32Ret);
        }

        s32Ret = HI_MAPI_VPROC_StopPort(pstSrc->VprocHdl, pstSrc->VportHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VProc_Port_Stop failed[%08x]\n", s32Ret);
        }

        s_stUVCStreamCtx.bVportStart = HI_FALSE;
    }

    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_VEncStart(HI_VOID)
{
#if UVC_TIMELAPSE_CHECK
    HI_TIME_CLEAN;
#endif

    HI_S32 s32Ret = HI_SUCCESS;
    UVC_STREAM_ATTR_S *pstAttr = &s_stUVCStreamCtx.stStreamAttr;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;

    /* ====== VProc Port ====== */
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    MLOGD("prochdl[%d] porthdl[%d]\n", pstSrc->VprocHdl, pstSrc->VportHdl);
    s32Ret = HI_MAPI_VPROC_GetPortAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stVPortAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VProc_Port_GetAttr");

    stVPortAttr.stFrameRate.s32SrcFrameRate = pstAttr->u32Fps;
    stVPortAttr.stFrameRate.s32DstFrameRate = pstAttr->u32Fps;
    stVPortAttr.u32Width = pstAttr->u32Width;
    stVPortAttr.u32Height = pstAttr->u32Height;
    MLOGD("VProc Port Attr: %ux%u, src: %ufps, dst: %ufps\n", stVPortAttr.u32Width,
        stVPortAttr.u32Height, stVPortAttr.stFrameRate.s32SrcFrameRate,stVPortAttr.stFrameRate.s32DstFrameRate);
    s32Ret = HI_MAPI_VPROC_SetPortAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stVPortAttr);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VProc_Port_SetAttr");

    /* ====== VEnc ====== */
    MLOGD("venchdl[%d]\n", pstSrc->VencHdl);
    HI_MAPI_VENC_ATTR_S stVencAttr;
    stVencAttr.stVencPloadTypeAttr.u32Width = pstAttr->u32Width;
    stVencAttr.stVencPloadTypeAttr.u32Height = pstAttr->u32Height;
    stVencAttr.stVencPloadTypeAttr.u32BufSize = 2 * pstAttr->u32Width * pstAttr->u32Height;
    stVencAttr.stVencPloadTypeAttr.enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV;
    switch (pstAttr->enFormat)
    {
        case HI_UVC_STREAM_FORMAT_H264:
        {
            MLOGD("Stream H264\n");
            stVencAttr.stVencPloadTypeAttr.enType = HI_MAPI_PAYLOAD_TYPE_H264;
            stVencAttr.stVencPloadTypeAttr.u32Profile = 1;
            break;
        }
        case HI_UVC_STREAM_FORMAT_MJPEG:
        {
            MLOGD("Stream MJPEG\n");
            stVencAttr.stVencPloadTypeAttr.u32Profile = 0;
            stVencAttr.stVencPloadTypeAttr.enType = HI_MAPI_PAYLOAD_TYPE_MJPEG;
            break;
        }
        default:
        {
            MLOGE("Unsupport PayloadType\n");
            return HI_FAILURE;
        }
    }

    stVencAttr.stRcAttr.enRcMode = HI_MAPI_VENC_RC_MODE_CBR;
    stVencAttr.stRcAttr.stAttrCbr.u32SrcFrameRate = pstAttr->u32Fps;
    stVencAttr.stRcAttr.stAttrCbr.fr32DstFrameRate = pstAttr->u32Fps;
    stVencAttr.stRcAttr.stAttrCbr.u32Gop = pstAttr->u32Fps;
    stVencAttr.stRcAttr.stAttrCbr.u32BitRate =pstAttr->u32BitRate;
    stVencAttr.stRcAttr.stAttrCbr.u32StatTime = 1;
    MLOGD("VEnc Attr: Res[%ux%u], [%u]fps BitRate[%d]\n", stVencAttr.stVencPloadTypeAttr.u32Width,
        stVencAttr.stVencPloadTypeAttr.u32Height, stVencAttr.stRcAttr.stAttrCbr.fr32DstFrameRate,
        stVencAttr.stRcAttr.stAttrCbr.u32BitRate);
    s32Ret = HI_MAPI_VENC_Init(pstSrc->VencHdl, &stVencAttr);
    //HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VEnc_Init");

    if(HI_UVC_STREAM_FORMAT_MJPEG == pstAttr->enFormat)
    {
        MLOGD("Stream MJPEG set Qfactor\n");
        VENC_RC_PARAM_S stRcParam;
        s32Ret = HI_MAPI_VENC_GetAttrEx(pstSrc->VencHdl, HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        stRcParam.stParamMjpegCbr.u32MaxQfactor = 80;
        stRcParam.stParamMjpegCbr.u32MinQfactor = 20;

        s32Ret = HI_MAPI_VENC_SetAttrEx(pstSrc->VencHdl, HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX,
            &stRcParam, sizeof(VENC_RC_PARAM_S));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    if(HI_UVC_STREAM_FORMAT_MJPEG == pstAttr->enFormat ||
       HI_UVC_STREAM_FORMAT_H264 == pstAttr->enFormat)
    {
        HI_BOOL bFrmLostOpen = HI_TRUE;
        s32Ret = HI_MAPI_VENC_SetAttrEx(pstSrc->VencHdl, HI_MAPI_VENC_CMD_FRAMELOST_STRATEGY,
            &bFrmLostOpen, sizeof(HI_BOOL));
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        MLOGI("Stream bFrmLostOpen set HI_TRUE\n");
    }

    HI_MAPI_VENC_CALLBACK_S stVencCB;
    stVencCB.pfnDataCB = UVC_STREAM_VencDataProc;
    stVencCB.pPrivateData = (HI_VOID*)__FUNCTION__;
    s32Ret = HI_MAPI_VENC_RegisterCallback(pstSrc->VencHdl, &stVencCB);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VEnc_RegisterCallback");

    s32Ret = HI_MAPI_VENC_BindVProc(pstSrc->VprocHdl, pstSrc->VportHdl, pstSrc->VencHdl, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VEnc_Bind_VProc");

    s32Ret = HI_MAPI_VPROC_StartPort(pstSrc->VprocHdl, pstSrc->VportHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VProc_Port_Start");

#if UVC_TIMELAPSE_CHECK
    if(pthread_create(&s_stUVCCtx.Tsk2Id, NULL, YUV_TimeLapseCheck, NULL))
    {
        MLOGE("YUV_TimeLapseCheck create failed\n");
        s_stUVCCtx.bRun = HI_FALSE;
        return HI_FAILURE;
    }
    else
    {
        MLOGI("YUV_TimeLapseCheck create success\n");
    }
    /* Dump */
    s_stUVCStreamCtx.bVportStart = HI_TRUE;
#endif

    s32Ret = HI_MAPI_VENC_Start(pstSrc->VencHdl, -1);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_MAPI_VEnc_Start");

    s_stUVCStreamCtx.bVencStart = HI_TRUE;
    s_stUVCStreamCtx.bFirstFrame = HI_TRUE;
    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_VEncStop(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    MLOGD("\n");

    if (s_stUVCStreamCtx.bVencStart)
    {
        s32Ret = HI_MAPI_VENC_Stop(pstSrc->VencHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VEnc_Stop(%d) failed[%08x]\n", pstSrc->VencHdl, s32Ret);
        }

#if UVC_TIMELAPSE_CHECK
        if (s_stUVCStreamCtx.bVportStart)
        {
            pthread_join(s_stUVCCtx.Tsk2Id, NULL);
            s32Ret = HI_MAPI_VPROC_StopVpssDumpYUV(pstSrc->VprocHdl, pstSrc->VportHdl);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("HI_MAPI_VPROC_StopVpssDumpYUV(%d,%d) failed[%08x]\n", pstSrc->VprocHdl, pstSrc->VportHdl, s32Ret);
            }

            HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
            stDumpYUVAttr.bEnable = HI_FALSE;
            stDumpYUVAttr.u32Depth = 1;
            s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstSrc->VprocHdl, pstSrc->VportHdl, &stDumpYUVAttr);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("HI_MAPI_VPROC_SetVpssDumpYUVAttr(%d,%d) failed[%08x]\n", pstSrc->VprocHdl, pstSrc->VportHdl, s32Ret);
            }

            s_stUVCStreamCtx.bVportStart = HI_FALSE;
            g_s32TimeLapseCount = 0;
        }
#endif

        s32Ret = HI_MAPI_VPROC_StopPort(pstSrc->VprocHdl, pstSrc->VportHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VProc_Port_Stop failed[%08x]\n", s32Ret);
        }

        s32Ret = HI_MAPI_VENC_UnBindVProc(pstSrc->VprocHdl, pstSrc->VportHdl, pstSrc->VencHdl, HI_FALSE);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VEnc_UnBind_VProc failed[%08x]\n", s32Ret);
        }

        s32Ret = HI_MAPI_VENC_Deinit(pstSrc->VencHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VEnc_DeInit failed[%08x]\n", s32Ret);
        }

        s_stUVCStreamCtx.bVencStart = HI_FALSE;
    }

#if UVC_TIMELAPSE_CHECK
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
#endif

    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_Start(HI_VOID)
{
    HI_USB_STATE_E enState;
    HI_USB_GetState(&enState);
    if (HI_UVC_STREAM_FORMAT_YUV420 == s_stUVCStreamCtx.stStreamAttr.enFormat)
    {
        return UVC_STREAM_VPortStart();
    }
    else
    {
        return UVC_STREAM_VEncStart();
    }
}

static HI_S32 UVC_STREAM_Stop(HI_VOID)
{
    if (HI_UVC_STREAM_FORMAT_YUV420 == s_stUVCStreamCtx.stStreamAttr.enFormat)
    {
        return UVC_STREAM_VPortStop();
    }
    else
    {
        return UVC_STREAM_VEncStop();
    }
}

static HI_UVC_VIDEOMODE_E UVC_STREAM_GetVideoMode(HI_U32 u32Width,HI_U32 u32Height)
{
    switch(u32Width*u32Height)
    {
        case(640*480): /**<640x480 */
            return HI_UVC_VIDEOMODE_VGA30;
        case(1280*720):/**<1280x720  */
            return HI_UVC_VIDEOMODE_720P30;
        case(1920*1080):/**<1920x1080 */
            return HI_UVC_VIDEOMODE_1080P30;
        case(3840*2160): /**<3840x2160 */
            return HI_UVC_VIDEOMODE_4K30;
        default:
            return HI_UVC_VIDEOMODE_720P30;
    }
}
static HI_S32 UVC_STREAM_SetAttr(UVC_STREAM_ATTR_S *pstAttr)
{
    HI_APPCOMM_CHECK_POINTER(pstAttr, HI_FAILURE);
    MLOGD("Format[%d] Resolution[%ux%u] FrameRate[%ufps]\n",
        pstAttr->enFormat, pstAttr->u32Width, pstAttr->u32Height, pstAttr->u32Fps);
    s_stUVCStreamCtx.stStreamAttr = *pstAttr;
    HI_UVC_VIDEOMODE_E eVideoMode=HI_UVC_VIDEOMODE_BUTT;
    eVideoMode=UVC_STREAM_GetVideoMode(pstAttr->u32Width,pstAttr->u32Height);
    s_stUVCStreamCtx.stStreamAttr.u32BitRate=s_stUVCStreamCtx.stDeviceCap.astFmtCaps[pstAttr->enFormat].astModes[eVideoMode].u32BitRate;
    MLOGD("u32BitRate[%d]\n", s_stUVCStreamCtx.stStreamAttr.u32BitRate);
    return HI_SUCCESS;
}

static HI_S32 UVC_STREAM_ReqIDR(HI_VOID)
{
    if (HI_UVC_STREAM_FORMAT_H264 == s_stUVCStreamCtx.stStreamAttr.enFormat)
    {
        return HI_MAPI_VENC_RequestIDR(s_stUVCStreamCtx.stDataSource.VencHdl);
    }
    else
    {
        return HI_SUCCESS;
    }
}

/** UVC Stream Operation Set */
static UVC_STREAM_OPS_S s_stUvcStreamOps = {
    .pfnOpen    = UVC_STREAM_Start,
    .pfnClose   = UVC_STREAM_Stop,
    .pfnReqIDR  = UVC_STREAM_ReqIDR,
    .pfnSetAttr = UVC_STREAM_SetAttr,
};

UVC_STREAM_OPS_S *UVC_GetStreamOps(HI_VOID)
{
    return &s_stUvcStreamOps;
}

/**------------------- UVC STREAM End ------------------------- */

/**------------------- UVC Start ------------------------- */

static HI_S32 UVC_LoadMod(HI_VOID)
{
#ifndef __HuaweiLite__
    HI_insmod(HI_APPFS_KOMOD_PATH"/videobuf2-core.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/videobuf2-memops.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/videobuf2-v4l2.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/videobuf2-vmalloc.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/libcomposite.ko", NULL);
    HI_insmod(HI_APPFS_KOMOD_PATH"/usb_f_uvc.ko", "bulk_streaming_ep=1 bulk_max_size=1843200");
    HI_insmod(HI_APPFS_KOMOD_PATH"/g_webcam.ko", "req_num=2 req_buf_size=20 streaming_interval=1");
#else
    usb_init(DEVICE, DEV_UVC);
#endif
    return HI_SUCCESS;
}

static HI_VOID UVC_UnloadMod(HI_VOID)
{
#ifndef __HuaweiLite__
    HI_rmmod(HI_APPFS_KOMOD_PATH"/g_webcam.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/usb_f_uvc.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/libcomposite.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/videobuf2-vmalloc.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/videobuf2-v4l2.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/videobuf2-memops.ko");
    HI_rmmod(HI_APPFS_KOMOD_PATH"/videobuf2-core.ko");
#else
    usb_deinit();
#endif
}

static HI_VOID* UVC_CheckTask(HI_VOID *pvArg)
{
    prctl(PR_SET_NAME, "HiUVC_Check", 0, 0, 0);

    HI_S32 sRet = HI_SUCCESS;
    while(s_stUVCCtx.bRun)
    {
        sRet = UVC_GADGET_DeviceCheck();
        if (sRet < 0)
        {
            if (UVC_GetCtx()->bPCConnect)
            {
                MLOGD(YELLOW"PC Disconnect\n"NONE);
                UVC_GetCtx()->bPCConnect = HI_FALSE;
            }
            break;
        }
        // be careful. if return code is timeout,
        // maybe the host is disconnected,so here to start device again
        // it maybe to find a another nice method to checking host connects or disconnects
        else if (sRet == 0)
        {
#if 0
            MLOGW("timeout\n");
            if (UVC_GetCtx()->bPCConnect)
            {
                MLOGW(YELLOW"PC Disconnect\n"NONE);
                UVC_GetCtx()->bPCConnect = HI_FALSE;
            }
            UVC_GADGET_DeviceClose();
            if (UVC_GADGET_DeviceOpen(s_stUVCCtx.szDevPath) != 0)
            {
                MLOGE("Open Device failed\n");
                break;
            }
            MLOGW(YELLOW"Reconnect PC success\n"NONE);
#else
            MLOGD("timeout do nothing\n");
            if(HI_FALSE != g_bPushVencData)
            {
                g_bPushVencData = HI_FALSE;
            }
#endif
        }
    }
    return NULL;
}

HI_S32 UVC_Init(const HI_UVC_DEVICE_CAP_S *pstCap, const HI_UVC_DATA_SOURCE_S *pstDataSrc,
    HI_UVC_BUFFER_CFG_S *pstBufferCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCap, HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pstDataSrc, HI_FAILURE);

    if(HI_TRUE == s_stUVCStreamCtx.bInited)
    {
        MLOGD("UVC already inited\n");
        return HI_SUCCESS;
    }
#ifdef __HuaweiLite__
    HI_VOID UVC_SetDeviceCap(const HI_UVC_DEVICE_CAP_S *pstCap);
    UVC_SetDeviceCap(pstCap);
#endif
    HI_S32 s32Ret = UVC_LoadMod();
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "LoadKo");
    MLOGD("VProc[%d], VPort[%d], VEnc[%d], Acap[%d]\n", pstDataSrc->VprocHdl,
        pstDataSrc->VportHdl, pstDataSrc->VencHdl, pstDataSrc->AcapHdl);
    s_stUVCStreamCtx.stDataSource = *pstDataSrc;
    s_stUVCStreamCtx.stDeviceCap=*pstCap;
    s_stUVCStreamCtx.bVportStart = HI_FALSE;
    s_stUVCStreamCtx.bVencStart = HI_FALSE;
    //s_stUVCStreamCtx.u32BitRate=pstCap->u32BitRate;
    UVC_GADGET_Init(pstCap, pstBufferCfg->u32BufSize);
    s_stUVCStreamCtx.bInited = HI_TRUE;
    return UVC_FRAMEBUF_Create(pstBufferCfg->u32BufSize, pstBufferCfg->u32BufCnt);
}

HI_S32 UVC_Deinit(HI_VOID)
{
    if(HI_FALSE == s_stUVCStreamCtx.bInited)
    {
        MLOGD("UVC already deinited\n");
        return HI_SUCCESS;
    }

    MLOGD(YELLOW"\n"NONE);
    UVC_FRAMEBUF_Destroy();
    UVC_UnloadMod();
    s_stUVCStreamCtx.bInited = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 UVC_Start(const HI_CHAR *pszDevPath)
{
    HI_APPCOMM_CHECK_POINTER(pszDevPath, HI_FAILURE);

    if(HI_FALSE == s_stUVCCtx.bRun)
    {
        memcpy(s_stUVCCtx.szDevPath, pszDevPath, HI_APPCOMM_MAX_PATH_LEN);
        MLOGD("DevicePath[%s]\n", pszDevPath);

        /* Open Device */
        UVC_GADGET_DeviceOpen(pszDevPath);

        /* Create uvc check task thread */
        s_stUVCCtx.bPCConnect = HI_FALSE;
        s_stUVCCtx.bRun = HI_TRUE;
        if(pthread_create(&s_stUVCCtx.TskId, NULL, UVC_CheckTask, NULL))
        {
            MLOGE("UVC_CheckTask create failed\n");
            s_stUVCCtx.bRun = HI_FALSE;
            return HI_FAILURE;
        }
        MLOGD("UVC_CheckTask create successful\n");
    }
    else
    {
        MLOGD("UVC already started\n");
    }

    return HI_SUCCESS;
}

HI_S32 UVC_Stop(HI_VOID)
{
    if(HI_FALSE == s_stUVCCtx.bRun)
    {
        MLOGD("UVC not run\n");
        return HI_SUCCESS;
    }
    /* Destroy check task */
    s_stUVCCtx.bRun = HI_FALSE;
    pthread_join(s_stUVCCtx.TskId, NULL);

    return UVC_GADGET_DeviceClose();
}

UVC_CONTEXT_S* UVC_GetCtx(HI_VOID)
{
    return &s_stUVCCtx;
}

/**------------------- UVC End ------------------------- */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

