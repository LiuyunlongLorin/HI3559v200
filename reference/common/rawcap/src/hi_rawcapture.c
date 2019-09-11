/**
* @file    hi_rawcapture.c
* @brief   implementation of record manager interface
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
* @version   1.0

*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "hi_eventhub.h"
#include "hi_timer_ext.h"
#include "hi_comm_vi.h"
#include "hi_comm_video.h"
#include "hi_comm_isp.h"
#include "hi_awb_comm.h"
#include "hi_ae_comm.h"

#include "mpi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_sys.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_rawcapture.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/** redefine module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "RawCap"

/** minimum task interval, ms*/
#define RAWCAP_TASK_MIN_INTERVAL (10000)
#define RAWCAP_WAIT_SAVE_TIMEOUT  (15)        /**< unit:second */


/** maximum  task count */
/** Actually, each sensor need a indivial task*/
#define RAWCAP_TASK_MAX_CNT (2)

/** maximum frame count */
#define RAWCAP_FRAME_MAX_CNT (4)

/**  frame depth of isp debug info */
#define RAWCAP_DEBUGINFO_FRAME_DEPTH (5)

/**  count of isp debug info when once dump */
#define RAWCAP_DEBUGINFO_CNT (2)

/**  count of isp debug info when once dump */
#define RAWCAP_DEBUGINFO_WAITTIME (300000)

/**Attention!!*/
/**Attention!!*/
/**Attention!!*/
/**Max Width, Currently use 4096. If the frame is bigger, The number need to be change*/
/**We specified the number, because we don't when to use malloc and free freqently*/
/**Avoiding to generate memory fragmentation*/
#define RAWCAP_FRAME_WIDTH_MAX_SIZE  (4096)

/**< time accuracy, unit:us */
#define RAWCAP_TIMER_TICKVALUE_US  (20*1000)

/** RawCap task status */
typedef enum tagRAWCAP_TASK_STATE_E
{
    RAWCAP_TASK_STATE_READY = 0, /**<task ready, can be start */
    RAWCAP_TASK_STATE_BUSY,      /**<task started, can not be start again*/
    RAWCAP_TASK_STATE_ONCESTART, /**<task started, timer task once start*/
    RAWCAP_TASK_STATE_ONCEEND,   /**<task started, timer task once end*/
    RAWCAP_TASK_STATE_BUTT
} RAWCAP_TASK_STATE_E;

typedef struct tagRAWCAP_DUMP_EVENT_MUTEX_COND_S
{
    pthread_mutex_t rawcapEventMutex;
    pthread_cond_t rawcapEventCond;
} RAWCAP_DUMP_EVENT_MUTEX_COND_S;

/** rawcap task context */
typedef struct RAWCAP_TASK_CONTEXT_S
{
    HI_BOOL bUsed;
    HI_HANDLE TskHdl;
    RAWCAP_TASK_STATE_E enRawcapTaskState;
    HI_RAWCAP_GET_FILENAME_S stGetFileNameCb;
    HI_RAWCAP_FILENAME_S stCurrentFileName;
    HI_RAWCAP_FILENAME_S stFormerFileName;
    HI_U32 u32FrameCnt;
    HI_U32 u32YuvSavedCnt;
    HI_U32 u32RawSavedCnt;
    HI_U32 u32SamePrefixCnt;
    HI_BOOL bRawFileSeparated;
    HI_RAWCAP_DATA_SOURCE_S stDataSource;
    HI_RAWCAP_TRIGGER_MODE_S stTriggerMode;
    COMPRESS_MODE_E enCompressMode;
    HI_TIMER_HANDLE TimedTaskHdl;
    pthread_mutex_t stateMutex; /**<task state mutex */
    RAWCAP_DUMP_EVENT_MUTEX_COND_S stRawcapEventMutexCond;
} RAWCAP_TASK_CONTEXT_S;
static RAWCAP_TASK_CONTEXT_S s_astRawCapTaskCtx[RAWCAP_TASK_MAX_CNT];

/** rawcap init flag */
static HI_BOOL s_bRawCapInitFlg = HI_FALSE;

static HI_S32 s_s32TimerGrp = 0;

#define RAWCAP_CHECK_INIT() if(!s_bRawCapInitFlg) {MLOGE("not init yet\n"); return HI_RAWCAP_ENOINIT;}

/** check task index(handle) created or not */
#define RAWCAP_CHECK_USED(idx) if(!s_astRawCapTaskCtx[idx].bUsed) \
    {MLOGE("Task handle[%d] has already destroyed or not created yet\n", idx); return HI_RAWCAP_EINVAL;}

/** check task index(handle) invalid or not */
#define RAWCAP_CHECK_TSKINDEX(idx) if (idx < 0 || idx >= RAWCAP_TASK_MAX_CNT) \
    {MLOGE("Invalid Task handle[%d]\n", idx); return HI_RAWCAP_EINVAL;}

static HI_S32 RAWCAP_ConvertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, PIXEL_FORMAT_E enPixelFormat, HI_U16 *pu16OutData)
{
    HI_S32 i, u32Tmp, s32OutCnt = 0;
    HI_U32 u32Val = 0;
    HI_U64 u64Val = 0;
    HI_U8 *pu8Tmp = pu8Data;

    s32OutCnt = 0;
    switch(enPixelFormat)
    {
        case PIXEL_FORMAT_RGB_BAYER_10BPP:
        {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte4 byte3 byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 5 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                     ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32);

                pu16OutData[s32OutCnt++] = u64Val & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 10) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 20) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 30) & 0x3ff;
            }
        }
        break;
        case PIXEL_FORMAT_RGB_BAYER_12BPP:
        {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;
                u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
            }
        }
        break;
        case PIXEL_FORMAT_RGB_BAYER_14BPP:
        {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                pu8Tmp = pu8Data + 7 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                      ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32) +
                      ((HI_U64)pu8Tmp[5] << 40) + ((HI_U64)pu8Tmp[6] << 48);

                pu16OutData[s32OutCnt++] = u64Val & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 14) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 28) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 42) & 0x3fff;
            }
        }
        break;
        default:
            MLOGE("unsuport enPixelFormat: %d\n", enPixelFormat);
            return HI_FAILURE;
            break;
    }

    return s32OutCnt;
}

static HI_S32 RAWCAP_RAWDATA_Save(HI_MAPI_FRAME_DATA_S* pVBuf, FILE* pfd, HI_S32 s32FrameCnt)
{
    if (!HI_APPCOMM_CHECK_RANGE(s32FrameCnt, 1, RAWCAP_FRAME_MAX_CNT))
    {
        MLOGE("Invalid Raw Cap s32FrameCnt[%u]\n", s32FrameCnt);
        return HI_RAWCAP_EINVAL;
    }

    HI_U32 u32H = 0;
    HI_S32 i = 0;
    HI_U16 *apu16Data[RAWCAP_FRAME_MAX_CNT] = {NULL};
    HI_U16 au16Data[RAWCAP_FRAME_MAX_CNT][RAWCAP_FRAME_WIDTH_MAX_SIZE * 2] = {0};
    HI_U8* apUserPageAddr[RAWCAP_FRAME_MAX_CNT] = {HI_NULL};
    HI_U8  *pu8Data = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    for (i = 0; i < s32FrameCnt; i++)
    {
        apUserPageAddr[i] = (HI_U8*)(HI_UL)(pVBuf + i)->u64VirAddr[0];
        if (NULL == apUserPageAddr[i])
        {
            MLOGE("Error VirAddr.\n");
            s32Ret = HI_FAILURE;
            goto exit;
        }

        pu8Data = apUserPageAddr[i];
        if ((PIXEL_FORMAT_RGB_BAYER_8BPP != (pVBuf + i)->enPixelFormat) && (PIXEL_FORMAT_RGB_BAYER_16BPP != (pVBuf + i)->enPixelFormat))
        {
            apu16Data[i] = au16Data[i];
            if (NULL == apu16Data[i])
            {
                MLOGE("Malloc Error.\n");
                s32Ret = HI_FAILURE;
                goto exit;
            }
        }

        MLOGD("saving......raw data......u32Stride[0]: %d, width: %d\n", (pVBuf + i)->u32Stride[0], (pVBuf + i)->u32Width);

        for (u32H = 0; u32H < (pVBuf + i)->u32Height; u32H++)
        {
            if (PIXEL_FORMAT_RGB_BAYER_8BPP == (pVBuf + i)->enPixelFormat)
            {
                fwrite(pu8Data, (pVBuf + i)->u32Width, 1, pfd);
                fflush(pfd);
            }
            else if (PIXEL_FORMAT_RGB_BAYER_16BPP == (pVBuf + i)->enPixelFormat)
            {
                fwrite(pu8Data, (pVBuf + i)->u32Width, 2, pfd);
                fflush(pfd);
            }
            else
            {
                RAWCAP_ConvertBitPixel(pu8Data, (pVBuf + i)->u32Width, (pVBuf + i)->enPixelFormat, apu16Data[i]);
                fwrite(apu16Data[i], (pVBuf + i)->u32Width, 2, pfd);
            }
            pu8Data += (pVBuf + i)->u32Stride[0];
        }

        fflush(pfd);
        MLOGI("\n************saving raw...No%d...done!****************************\n", i);
    }
exit:
    for(i = 0; i <s32FrameCnt ; i++)
    {
        if (HI_NULL != apu16Data[i])
        {
            apu16Data[i] = HI_NULL;

        }

        if (HI_NULL != apUserPageAddr[i])
        {
            apUserPageAddr[i] = HI_NULL;
        }
    }
    return s32Ret;
}

HI_S32 RAWCAP_YUVDATA_Save(HI_MAPI_FRAME_DATA_S* pVPortYUV, FILE* pfd, HI_VOID* pPrivateData)
{
    HI_U32 u32W, u32H = 0;
    HI_CHAR* pCVBufVirt_Y = HI_NULL;
    HI_CHAR* pCVBufVirt_C = HI_NULL;
    HI_CHAR* pCMemContent = HI_NULL;
    HI_CHAR cTmpBuf[RAWCAP_FRAME_WIDTH_MAX_SIZE] = {0}; /**If this value is too small and the image is big, this memory may not be enough */
    HI_U8* pUserPageAddr = HI_NULL;
    HI_U32 u32UvHeight = 0;/*When the storage format is a planar format, this variable is used to keep the height of the UV component */

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == pVPortYUV->enPixelFormat)
    {
        u32UvHeight = pVPortYUV->u32Height / 2;
    }
    else if(PIXEL_FORMAT_YVU_SEMIPLANAR_422 == pVPortYUV->enPixelFormat)
    {
        u32UvHeight = pVPortYUV->u32Height;
    }
    else
    {
        MLOGE("enPixelFormat error.\n");
        return HI_FAILURE;
    }

    pUserPageAddr = (HI_U8*)(HI_UL)pVPortYUV->u64VirAddr[0];
    if (HI_NULL == pUserPageAddr)
    {
        MLOGE("Error VirAddr.\n");
        return HI_FAILURE;
    }

    pCVBufVirt_Y = (HI_CHAR*)pUserPageAddr;
    pCVBufVirt_C = pCVBufVirt_Y + (pVPortYUV->u32Stride[0]) * (pVPortYUV->u32Height);

    /* save Y ----------------------------------------------------------------*/
    MLOGD("saving.....Y......\n");
    for (u32H = 0; u32H < pVPortYUV->u32Height; u32H++)
    {
        pCMemContent = pCVBufVirt_Y + u32H * pVPortYUV->u32Stride[0];
        fwrite(pCMemContent, pVPortYUV->u32Width, 1, pfd);
    }
    fflush(pfd);

    /* save U ----------------------------------------------------------------*/
    MLOGD("U......\n");
    for (u32H = 0; u32H< u32UvHeight; u32H++)
    {
        pCMemContent = pCVBufVirt_C + u32H * pVPortYUV->u32Stride[1];

        pCMemContent += 1;

        for (u32W = 0; u32W < pVPortYUV->u32Width / 2; u32W++)
        {
            cTmpBuf[u32W] = *pCMemContent;
            pCMemContent += 2;
        }
        fwrite(cTmpBuf, pVPortYUV->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    /* save V ----------------------------------------------------------------*/
    MLOGD("V......\n");
    for (u32H = 0; u32H < u32UvHeight; u32H++)
    {
        pCMemContent = pCVBufVirt_C + u32H * pVPortYUV->u32Stride[1];

        for (u32W = 0; u32W < pVPortYUV->u32Width / 2; u32W++)
        {
            cTmpBuf[u32W] = *pCMemContent;
            pCMemContent += 2;
        }
        fwrite(cTmpBuf, pVPortYUV->u32Width / 2, 1, pfd);
    }
    fflush(pfd);

    pUserPageAddr = HI_NULL;
    return HI_SUCCESS;
}

HI_S32 RAWCAP_YUVDATA_DumpCallback(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_MAPI_FRAME_DATA_S* pVPortYUV, HI_VOID* pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* pfd = NULL;
    HI_CHAR szYUVFileName[HI_RAWCAP_MAX_FILENAME_LEN];

    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pPrivateData;

    /**Here is just for debug*/
    MLOGD("The YUVFrame's  u32Width is %d \n", pVPortYUV->u32Width);
    snprintf(szYUVFileName, sizeof(szYUVFileName), "%s_sameprefix%d_pipe%d_sameaction%d.yuv", pstTskCtx->stCurrentFileName.szYUV_FileName, pstTskCtx->u32SamePrefixCnt, pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->u32YuvSavedCnt);

    pfd = fopen(szYUVFileName, "wb");
    MLOGD("YUVFileName is %s.\n",szYUVFileName);
    if(pfd > 0)
    {
        s32Ret = RAWCAP_YUVDATA_Save(pVPortYUV, pfd, pstTskCtx);
        fclose(pfd);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("RawData_Save fail\n");
            return HI_FAILURE;
        }
    }
    else
    {
        MLOGE("open file fail\n");
        perror("A");
        return HI_FAILURE;
    }

    pstTskCtx->u32YuvSavedCnt++;
    MLOGD("RAWCAP_YUVDATA_DumpCallback is End.\n");
    return HI_SUCCESS;
}

HI_S32 RAWCAP_RAWDATA_DumpCallback(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S* pVCapRawData, HI_S32 s32DataNum, HI_VOID* pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* pfd = NULL;
    HI_S32 i = 0;
    HI_CHAR szRawFileName[HI_RAWCAP_MAX_FILENAME_LEN];
    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pPrivateData;
    ISP_FRAME_INFO_S* pstIspFrameInfo = HI_NULL;
    HI_U32 u32ExposureTime = 0;
    HI_U32 u32IspDgain = 0;
    HI_U32 u32Again = 0;
    HI_U32 u32Dgain = 0;
    /**Here is just for debug*/
    MLOGD("The RawFrame's  u32Width is %d \n", pVCapRawData->u32Width);


    if (pstTskCtx->bRawFileSeparated)
    {
        HI_S32 s32FramePerGrp = 0;
        if (0 == pstTskCtx->u32FrameCnt)
        {
            MLOGE("The FrameCnt Can't be zero.\n");
            return HI_FAILURE;
        }
        /** s32DataNum means one dump action will get this numbers frame*/
        /** In LEANER mode, the number equals u32FrameCnt*/
        /** In WDR mode, the number equals u32FrameCnt * WDR base number*/
        s32FramePerGrp = s32DataNum / pstTskCtx->u32FrameCnt;

        for (i = 0; i < pstTskCtx->u32FrameCnt; i++)
        {
            HI_MAPI_FRAME_DATA_S* pstCurrentRaw = pVCapRawData + i * s32FramePerGrp;

            pstIspFrameInfo = HI_MPI_SYS_Mmap(pstCurrentRaw->stVideoSupplement.u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));
            if (HI_NULL == pstIspFrameInfo)
            {
                MLOGD( "HI_MPI_SYS_Mmap failed.\n");
                return HI_FAILURE;
            }
            u32ExposureTime = pstIspFrameInfo->u32ExposureTime;
            u32IspDgain = pstIspFrameInfo->u32IspDgain;
            u32Again = pstIspFrameInfo->u32Again;
            u32Dgain = pstIspFrameInfo->u32Dgain;
            s32Ret = HI_MPI_SYS_Munmap(pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
            if (HI_SUCCESS != s32Ret)
            {
                MLOGD( "HI_MPI_SYS_Mmap failed.\n");
                return HI_FAILURE;
            }

            snprintf(szRawFileName, sizeof(szRawFileName), "%s_sameprefix%d_pipe%d_separate%d_ExpTime_%d_ag_%d_dg_%d_ispdg_%d.raw", pstTskCtx->stCurrentFileName.szRaw_FileName, pstTskCtx->u32SamePrefixCnt, pstTskCtx->stDataSource.VcapPipeHdl,
                pstTskCtx->u32RawSavedCnt, u32ExposureTime, u32Again, u32Dgain, u32IspDgain);
            pstTskCtx->u32RawSavedCnt++;

            pfd = fopen(szRawFileName, "wb");
            MLOGD("RawFileName is %s.\n",szRawFileName);
            if(pfd > 0)
            {

                s32Ret = RAWCAP_RAWDATA_Save(pstCurrentRaw, pfd, s32FramePerGrp);
                fclose(pfd);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("RawData_Save fail\n");
                    return HI_FAILURE;
                }
            }
            else
            {
                MLOGE("open file fail\n");
                perror("A");
                return HI_FAILURE;
            }
        }
    }
    else
    {
        HI_MAPI_FRAME_DATA_S* pstCurrentRaw = pVCapRawData;

        pstIspFrameInfo = HI_MPI_SYS_Mmap(pstCurrentRaw->stVideoSupplement.u64IspInfoPhyAddr, sizeof(ISP_FRAME_INFO_S));
        if (HI_NULL == pstIspFrameInfo)
        {
            MLOGD( "HI_MPI_SYS_Mmap failed.\n");
            return HI_FAILURE;
        }
        u32ExposureTime = pstIspFrameInfo->u32ExposureTime;
        u32IspDgain = pstIspFrameInfo->u32IspDgain;
        u32Again = pstIspFrameInfo->u32Again;
        u32Dgain = pstIspFrameInfo->u32Dgain;
        s32Ret = HI_MPI_SYS_Munmap(pstIspFrameInfo, sizeof(ISP_FRAME_INFO_S));
        if (HI_SUCCESS != s32Ret)
        {
            MLOGD( "HI_MPI_SYS_Mmap failed.\n");
            return HI_FAILURE;
        }

        snprintf(szRawFileName, sizeof(szRawFileName), "%s_sameprefix%d_pipe%d_aggregate%d_ExpTime_%d_ag_%d_dg_%d_ispdg_%d.raw", pstTskCtx->stCurrentFileName.szRaw_FileName, pstTskCtx->u32SamePrefixCnt, pstTskCtx->stDataSource.VcapPipeHdl,
             pstTskCtx->u32FrameCnt, u32ExposureTime, u32Again, u32Dgain, u32IspDgain);

        pfd = fopen(szRawFileName, "wb");
        MLOGD("RawFileName is %s.\n",szRawFileName);
        if(pfd > 0)
        {
            s32Ret = RAWCAP_RAWDATA_Save(pstCurrentRaw, pfd, s32DataNum);
            fclose(pfd);
            if(HI_SUCCESS != s32Ret)
            {
                MLOGE("RawData_Save fail\n");
                return HI_FAILURE;
            }
        }
        else
        {
            MLOGE("open file fail\n");
            perror("A");
            return HI_FAILURE;
        }
    }

    MLOGD("RAWCAP_RAWDATA_DumpCallback is End.\n");
    return HI_SUCCESS;
}

static inline HI_VOID RAWCAP_SetTaskState(HI_HANDLE Hdl, RAWCAP_TASK_STATE_E enTaskState)
{
    HI_MUTEX_LOCK(s_astRawCapTaskCtx[Hdl].stateMutex);
    s_astRawCapTaskCtx[Hdl].enRawcapTaskState = enTaskState;
    HI_MUTEX_UNLOCK(s_astRawCapTaskCtx[Hdl].stateMutex);
}

static HI_S32 RAWCAP_Analysis_AEDebug(FILE* pfd, HI_VOID* pVData, HI_U32 u32Depth)
{
    HI_S32 i = 0;
    AE_DBG_ATTR_S*  pstAeDbgAttr = HI_NULL;
    AE_DBG_STATUS_S * pstAeDbgStatus = HI_NULL;

    pstAeDbgAttr = (AE_DBG_ATTR_S*)pVData;
    pstAeDbgStatus = (AE_DBG_STATUS_S*)((HI_UL)pVData + sizeof(AE_DBG_ATTR_S));

    fprintf(pfd,"MaxInttime                     %u\r\n",pstAeDbgAttr->u32MaxIntTime);
    fprintf(pfd,"MinInttime                     %u\r\n",pstAeDbgAttr->u32MinIntTime);
    fprintf(pfd,"MaxAgain                       %u\r\n",pstAeDbgAttr->u32MaxAgain);
    fprintf(pfd,"MinAgain                       %u\r\n",pstAeDbgAttr->u32MinAgain);
    fprintf(pfd,"MaxDgain                       %u\r\n",pstAeDbgAttr->u32MaxDgain);
    fprintf(pfd,"MinDgain                       %u\r\n",pstAeDbgAttr->u32MinDgain);
    fprintf(pfd,"MaxISPDgain                    %u\r\n",pstAeDbgAttr->u32MaxIspDgain);
    fprintf(pfd,"MinISPDgain                    %u\r\n",pstAeDbgAttr->u32MinIspDgain);
    fprintf(pfd,"MaxSysGain                     %u\r\n",pstAeDbgAttr->u32MaxSysGain);
    fprintf(pfd,"MinSysGain                     %u\r\n",pstAeDbgAttr->u32MinSysGain);
    fprintf(pfd,"Compensation                   %u\r\n",pstAeDbgAttr->u32Compensation);
    fprintf(pfd,"EVBias                         %u\r\n",pstAeDbgAttr->u32EVBias);

    fprintf(pfd,"ManualExposureEnable           %d\r\n",pstAeDbgAttr->bManualExposureEn);
    fprintf(pfd,"ManualTimeEnable               %d\r\n",pstAeDbgAttr->bManualTimeEn);
    fprintf(pfd,"ManualAgainEnable              %d\r\n",pstAeDbgAttr->bManualAgainEn);
    fprintf(pfd,"ManualDgainEnable              %d\r\n",pstAeDbgAttr->bManualDgainEn);
    fprintf(pfd,"ManualISPDgainEnable           %d\r\n",pstAeDbgAttr->bManualIspDgainEn);
    fprintf(pfd,"ManualExposureLines            %u\r\n",pstAeDbgAttr->u32ManualExposureLines);
    fprintf(pfd,"ManualAgain                    %u\r\n",pstAeDbgAttr->u32ManualAgain);
    fprintf(pfd,"ManualDgain                    %u\r\n",pstAeDbgAttr->u32ManualDgain);
    fprintf(pfd,"ManualISPDgain                 %u\r\n",pstAeDbgAttr->u32ManualIspDgain);
    fprintf(pfd,"AEWeights:\r\n");

    for (i = 0; i < (AE_ZONE_ROW * AE_ZONE_COLUMN); i++)
    {
        fprintf(pfd, "%u  ", pstAeDbgAttr->au32AeWeights[i]);
        if((i + 1) % AE_ZONE_COLUMN == 0)
        {
            fprintf(pfd, "\r\n");
        }
    }

    fprintf(pfd, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %24s %12s %12s %12s %12s %12s %12s %12s %12s\r\n",
            "FrameCnt", "FullLines", "IntTime", "ShortIntTime", "MedIntTime", "LongIntTime",
            "Again", "Dgain", "ISPDgain", "IrisFNOLin", "Exposure", "Increment", "HistError", "HistOriAvg", "LumaOffset", "ISO"
            , "u32ExpRatio", "u32OverExpRatio", "u32OverExpRatioFilter");

    for(i = 0; i < u32Depth; i++)
    {
        fprintf(pfd,"%12u %12u %12u %12u %12u %12u %12u ",pstAeDbgStatus[i].u32FrmNumBgn,pstAeDbgStatus[i].u32FullLines,pstAeDbgStatus[i].u32IntTime,
                pstAeDbgStatus[i].u32ShortIntTime, pstAeDbgStatus[i].u32MedIntTime, pstAeDbgStatus[i].u32LongIntTime, pstAeDbgStatus[i].u32Again);
        fprintf(pfd,"%12u %12u %12u %24llu %12u ",pstAeDbgStatus[i].u32Dgain, pstAeDbgStatus[i].u32IspDgain, pstAeDbgStatus[i].u32IrisFNOLin, pstAeDbgStatus[i].u64Exposure, pstAeDbgStatus[i].u32Increment);
        fprintf(pfd,"%12d %12d %12d %12u %12u %12u %12u\r\n",pstAeDbgStatus[i].s32HistError, pstAeDbgStatus[i].s32HistOriAverage, pstAeDbgStatus[i].s32LumaOffset, pstAeDbgStatus[i].u32Iso, pstAeDbgStatus[i].u32ExpRatio
                , pstAeDbgStatus[i].u32OverExpRatio, pstAeDbgStatus[i].u32OverExpRatioFilter);
    }

    return HI_SUCCESS;
}

static HI_S32 RAWCAP_Analysis_AWBDebug(FILE* pfd, HI_VOID* pVData, HI_U32 u32Depth)
{
    HI_S32 i = 0;
    HI_S32 j = 0;
    AWB_DBG_ATTR_S*  pstAwbDbgAttr = HI_NULL;
    AWB_DBG_STATUS_S * pstAwbDbgStatus = HI_NULL;

    pstAwbDbgAttr = (AWB_DBG_ATTR_S*)pVData;
    pstAwbDbgStatus = (AWB_DBG_STATUS_S*)((HI_UL)pVData + sizeof(AWB_DBG_ATTR_S));

    fprintf(pfd,"WhiteLevel                          0x%x\r\n",pstAwbDbgAttr->u16WhiteLevel);
    fprintf(pfd,"BlackLevel                          0x%x\r\n",pstAwbDbgAttr->u16BlackLevel);
    fprintf(pfd,"CrMax                               0x%x\r\n",pstAwbDbgAttr->u16CrMax);
    fprintf(pfd,"CrMin                               0x%x\r\n",pstAwbDbgAttr->u16CrMin);
    fprintf(pfd,"CbMax                               0x%x\r\n",pstAwbDbgAttr->u16CbMax);
    fprintf(pfd,"CbMin                               0x%x\r\n",pstAwbDbgAttr->u16CbMin);
    fprintf(pfd,"CrHigh                              0x%x\r\n",pstAwbDbgAttr->u16CrHigh);
    fprintf(pfd,"CrLow                               0x%x\r\n",pstAwbDbgAttr->u16CrLow);
    fprintf(pfd,"CbHigh                              0x%x\r\n",pstAwbDbgAttr->u16CbHigh);
    fprintf(pfd,"CbLow                               0x%x\r\n",pstAwbDbgAttr->u16CbLow);

    fprintf(pfd,"BayerCfg.WhiteLevel                 0x%x\r\n",pstAwbDbgAttr->u16RawWhiteLevel);
    fprintf(pfd,"BayerCfg.BlackLevel                 0x%x\r\n",pstAwbDbgAttr->u16RawBlackLevel);
    fprintf(pfd,"BayerCfg.CrMax                      0x%x\r\n",pstAwbDbgAttr->u16RawCrMax);
    fprintf(pfd,"BayerCfg.CrMin                      0x%x\r\n",pstAwbDbgAttr->u16RawCrMin);
    fprintf(pfd,"BayerCfg.CbMax                      0x%x\r\n",pstAwbDbgAttr->u16RawCbMax);
    fprintf(pfd,"BayerCfg.CbMin                      0x%x\r\n",pstAwbDbgAttr->u16RawCbMin);
    fprintf(pfd,"BayerCfg.CrHigh                     0x%x\r\n",pstAwbDbgAttr->u16RawCrHigh);
    fprintf(pfd,"BayerCfg.CrLow                      0x%x\r\n",pstAwbDbgAttr->u16RawCrLow);
    fprintf(pfd,"BayerCfg.CbHigh                     0x%x\r\n",pstAwbDbgAttr->u16RawCbHigh);
    fprintf(pfd,"BayerCfg.CbLow                      0x%x\r\n",pstAwbDbgAttr->u16RawCbLow);

    fprintf(pfd,"WDRMode                             %u\r\n",pstAwbDbgAttr->u16WDRMode);
    fprintf(pfd,"AWBATTR.Enable                      %u\r\n",pstAwbDbgAttr->u16Enable);
    fprintf(pfd,"WBATTR.WBType                       %u\r\n",pstAwbDbgAttr->u16ManualEnable);
    fprintf(pfd,"AWBATTR.ZoneSel                     %u\r\n",pstAwbDbgAttr->u16Zone);
    fprintf(pfd,"AWBATTR.HighColorTemp               %u\r\n",pstAwbDbgAttr->u16HighTemp);
    fprintf(pfd,"AWBATTR.LowColorTemp                %u\r\n",pstAwbDbgAttr->u16LowTemp);
    fprintf(pfd,"AWBATTR.RefTemp                     %u\r\n",pstAwbDbgAttr->u16RefTemp);
    fprintf(pfd,"AWBATTR.StaticWB[0]                 0x%x\r\n",pstAwbDbgAttr->u16RgainBase);
    fprintf(pfd,"AWBATTR.StaticWB[1]                 0x%x\r\n",pstAwbDbgAttr->u16GgainBase);
    fprintf(pfd,"AWBATTR.StaticWB[2]                 0x%x\r\n",pstAwbDbgAttr->u16BgainBase);
    fprintf(pfd,"AWBATTR.CurvePara[0]                %d\r\n",pstAwbDbgAttr->s32p1);
    fprintf(pfd,"AWBATTR.CurvePara[1]                %d\r\n",pstAwbDbgAttr->s32p2);
    fprintf(pfd,"AWBATTR.CurvePara[2]                %d\r\n",pstAwbDbgAttr->s32q);
    fprintf(pfd,"AWBATTR.CurvePara[3]                %d\r\n",pstAwbDbgAttr->s32a);
    fprintf(pfd,"AWBATTR.CurvePara[4]                %d\r\n",pstAwbDbgAttr->s32c);



    fprintf(pfd,"SaturationAttr.OpType               %u\r\n",pstAwbDbgAttr->u16ManSatEnable);
    fprintf(pfd,"SaturationAttr.SatTarget            %u\r\n",pstAwbDbgAttr->u16SatTarget);


    for(i = 0; i < u32Depth; i++)
    {

        fprintf(pfd,"\r\n==================== Frame %u-%u ====================\r\n",
                pstAwbDbgStatus[i].u32FrmNumBgn, pstAwbDbgStatus[i].u32FrmNumEnd);

        fprintf(pfd,"RGBSta.GlobalSum                    0x%x\r\n",pstAwbDbgStatus[i].u32GlobalSum);
        fprintf(pfd,"RGBSta.GlobalGR                     %u\r\n",pstAwbDbgStatus[i].u16GlobalRgSta);
        fprintf(pfd,"RGBSta.GlobalGB                     %u\r\n",pstAwbDbgStatus[i].u16GlobalBgSta);
        fprintf(pfd,"BayerSta.CountAll                   0x%x\r\n",pstAwbDbgStatus[i].u16GlobalCountAll);
        fprintf(pfd,"BayerSta.CountMin                   0x%x\r\n",pstAwbDbgStatus[i].u16GlobalCountMin);
        fprintf(pfd,"BayerSta.CountMax                   0x%x\r\n",pstAwbDbgStatus[i].u16GlobalCountMax);
        fprintf(pfd,"BayerSta.GlobalR                    0x%x\r\n",pstAwbDbgStatus[i].u16GlobalRAvg);
        fprintf(pfd,"BayerSta.GlobalG                    0x%x\r\n",pstAwbDbgStatus[i].u16GlobalGAvg);
        fprintf(pfd,"BayerSta.GlobalB                    0x%x\r\n",pstAwbDbgStatus[i].u16GlobalBAvg);

        fprintf(pfd,"WBInfo.ColorTemp                    %u\r\n",pstAwbDbgStatus[i].u16TK);
        fprintf(pfd,"WBInfo.Rgain                        %u\r\n",pstAwbDbgStatus[i].u16Rgain);
        fprintf(pfd,"WBInfo.Ggain                        %u\r\n",pstAwbDbgStatus[i].u16Ggain);
        fprintf(pfd,"WBInfo.Bgain                        %u\r\n",pstAwbDbgStatus[i].u16Bgain);
        fprintf(pfd,"WBInfo.au16CCM[0]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[0]);
        fprintf(pfd,"WBInfo.au16CCM[1]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[1]);
        fprintf(pfd,"WBInfo.au16CCM[2]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[2]);
        fprintf(pfd,"WBInfo.au16CCM[3]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[3]);
        fprintf(pfd,"WBInfo.au16CCM[4]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[4]);
        fprintf(pfd,"WBInfo.au16CCM[5]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[5]);
        fprintf(pfd,"WBInfo.au16CCM[6]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[6]);
        fprintf(pfd,"WBInfo.au16CCM[7]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[7]);
        fprintf(pfd,"WBInfo.au16CCM[8]                   0x%x\r\n",pstAwbDbgStatus[i].au16CCM[8]);

        fprintf(pfd,"\r\n\r\nRow&Col   u16Sum  u16Rg   u16Bg   u16CountAll  u16AvgR  u16AvgG  u16AvgB  u16TK  u16Weight  s16Shift\r\n");

        for (j = 0; j < AWB_ZONE_NUM; j++)
        {
            fprintf(pfd, "[%2d, %2d]: 0x%4x  0x%4x  0x%4x  0x%6x     0x%4x   0x%4x   0x%4x   %4u   %4u      %4d\r\n",
                    j / AWB_ZONE_ORIG_ROW, j % AWB_ZONE_ORIG_ROW,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16Sum,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16Rg,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16Bg,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16CountAll,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16RawRAvg,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16RawGAvg,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16RawBAvg,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16TK,
                    pstAwbDbgStatus[i].astZoneDebug[j].u16Weight,
                    pstAwbDbgStatus[i].astZoneDebug[j].s16Shift);
        }
    }

    return HI_SUCCESS;
}


static HI_S32 RAWCAP_DebugInfo_Save(HI_VOID* pVData, ISP_DEBUG_INFO_S* pIBuf, HI_S32* pu32Size, HI_U32 u32Depth)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szDebugInfoFileName[RAWCAP_DEBUGINFO_CNT][HI_RAWCAP_MAX_FILENAME_LEN] = {0};
    FILE* pfd[RAWCAP_DEBUGINFO_CNT];
    HI_U64 au64DebugInfoPhyaddr[RAWCAP_DEBUGINFO_CNT] = {0};
    HI_S32 as32DebugInfoSize[RAWCAP_DEBUGINFO_CNT] = {0};
    HI_VOID* apUserPageAddr[RAWCAP_DEBUGINFO_CNT] = {HI_NULL};

    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pVData;

    snprintf(szDebugInfoFileName[0], sizeof(szDebugInfoFileName[0]), "%s_sameprefix%d_pipe%d_%s.txt", pstTskCtx->stCurrentFileName.szDebugInfo_FileName,
         pstTskCtx->u32SamePrefixCnt,pstTskCtx->stDataSource.VcapPipeHdl,"isp_debug_ae");
    snprintf(szDebugInfoFileName[1], sizeof(szDebugInfoFileName[1]), "%s_sameprefix%d_pipe%d_%s.txt", pstTskCtx->stCurrentFileName.szDebugInfo_FileName,
         pstTskCtx->u32SamePrefixCnt,pstTskCtx->stDataSource.VcapPipeHdl,"isp_debug_awb");
    as32DebugInfoSize[0] = *(pu32Size + 0);
    as32DebugInfoSize[1] = *(pu32Size + 1);
    au64DebugInfoPhyaddr[0] = (*(pIBuf + 0)).u64PhyAddr;
    au64DebugInfoPhyaddr[1] = (*(pIBuf + 1)).u64PhyAddr;

    for(i = 0; i < RAWCAP_DEBUGINFO_CNT; i++)
    {
        apUserPageAddr[i] = HI_MemMap(au64DebugInfoPhyaddr[i], as32DebugInfoSize[i]);
        if (HI_NULL == apUserPageAddr[i])
        {
            MLOGE("HI_MemMap failed!\n");
            return HI_FAILURE;
        }

        pfd[i] = fopen(szDebugInfoFileName[i],"ab+");
        if (HI_NULL == pfd[i])
        {
            MLOGE("openfile failed!\n");
            perror("Error: ");
            HI_MemUnmap(apUserPageAddr[i]);
            apUserPageAddr[i] = HI_NULL;
            return HI_FAILURE;
        }
    }
    RAWCAP_Analysis_AEDebug(pfd[0],apUserPageAddr[0], u32Depth);
    RAWCAP_Analysis_AWBDebug(pfd[1],apUserPageAddr[1], u32Depth);


    for(i = 0; i < RAWCAP_DEBUGINFO_CNT; i++)
    {
        HI_MemUnmap(apUserPageAddr[i]);
        apUserPageAddr[i] = HI_NULL;

        fclose(pfd[i]);
    }

    return s32Ret;
}

static HI_S32 RAWCAP_DebugInfoProc(HI_VOID* pVData, HI_U32 u32Depth)
{
    HI_APPCOMM_CHECK_POINTER(pVData, HI_RAWCAP_EINVAL);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 as32Size[RAWCAP_DEBUGINFO_CNT] = {0};
    ISP_DEBUG_INFO_S  astISPDebug[RAWCAP_DEBUGINFO_CNT] = {0};
    HI_U64 u64AEPhyAddr = 0;
    HI_U64 u64AWBPhyAddr = 0;
    HI_VOID* pAEVitAddr = HI_NULL;
    HI_VOID* pAWBVitAddr = HI_NULL;

    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pVData;

    /**getMemory*/
    as32Size[0] = sizeof(AE_DBG_ATTR_S) + sizeof(AE_DBG_STATUS_S) * u32Depth;
    as32Size[1] = sizeof(AWB_DBG_ATTR_S) + sizeof(AWB_DBG_STATUS_S) * u32Depth;
    s32Ret = HI_MAPI_AllocBuffer(&u64AEPhyAddr, &pAEVitAddr, as32Size[0], "AEDebug");
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("AEDebug Mmz alloc failed! \n");
        return s32Ret;
    }

    s32Ret = HI_MAPI_AllocBuffer(&u64AWBPhyAddr, &pAWBVitAddr, as32Size[1], "AWBDebug");
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("AWBDebug Mmz alloc failed! \n");
        HI_MAPI_FreeBuffer(u64AEPhyAddr, pAEVitAddr, as32Size[0]);
        return s32Ret;
    }

    /**get ISPdebug*/
    astISPDebug[0].bDebugEn = HI_TRUE;
    astISPDebug[0].u32Depth = u32Depth;
    astISPDebug[0].u64PhyAddr = u64AEPhyAddr;
    s32Ret =  HI_MAPI_VCAP_SetAttrEx(pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_SetAEDebug, &(astISPDebug[0]), sizeof(ISP_DEBUG_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Get AEDebug failed! \n");
        goto ERROR_0;
    }

    astISPDebug[1].bDebugEn = HI_TRUE;
    astISPDebug[1].u32Depth = u32Depth;
    astISPDebug[1].u64PhyAddr = u64AWBPhyAddr;
    s32Ret =  HI_MAPI_VCAP_SetAttrEx(pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_SetAWBDebug, &(astISPDebug[1]), sizeof(ISP_DEBUG_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("Get AWBDebug failed! \n");
        goto ERROR_0;
    }

    //wait getting debug info
    HI_usleep(RAWCAP_DEBUGINFO_WAITTIME * u32Depth);

    MLOGI("\nGet data OK!\n");

        //savedebug
    s32Ret = RAWCAP_DebugInfo_Save(pstTskCtx, astISPDebug, as32Size, u32Depth);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("PDT_DUMP_SaveTwoDebug failed.");
    }


ERROR_0:

    astISPDebug[0].bDebugEn = HI_FALSE;
    astISPDebug[0].u32Depth = 0;
    astISPDebug[0].u64PhyAddr = 0;
    s32Ret =  HI_MAPI_VCAP_SetAttrEx(pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_SetAEDebug, &(astISPDebug[0]), sizeof(ISP_DEBUG_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("AEDebug Exit failed.");
    }

    astISPDebug[1].bDebugEn = HI_FALSE;
    astISPDebug[1].u32Depth = 0;
    astISPDebug[1].u64PhyAddr = 0;
    s32Ret =  HI_MAPI_VCAP_SetAttrEx(pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_SetAWBDebug, &(astISPDebug[1]), sizeof(ISP_DEBUG_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("ABDebug Exit failed.");
    }

    s32Ret = HI_MAPI_FreeBuffer(u64AEPhyAddr, pAEVitAddr, as32Size[0]);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_FreeBuffer failed! s32Ret=%x\n", s32Ret);
    }
    u64AEPhyAddr = 0;
    pAEVitAddr = HI_NULL;

    s32Ret = HI_MAPI_FreeBuffer(u64AWBPhyAddr, pAWBVitAddr, as32Size[1]);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_FreeBuffer failed! s32Ret=%x\n", s32Ret);
    }
    u64AWBPhyAddr = 0;
    pAWBVitAddr = HI_NULL;

    MLOGI("\nSave data OK!\n");
    return HI_SUCCESS;
}

static HI_S32 RAWCAP_DumpProc(HI_VOID* pVData)
{
    HI_APPCOMM_CHECK_POINTER(pVData, HI_RAWCAP_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pVData;

    HI_U32 u32FrameCnt = pstTskCtx->u32FrameCnt;
    MLOGI("Dump FrameCnt is %d.\n", u32FrameCnt);

    /**Get FileName*/
    s32Ret = pstTskCtx->stGetFileNameCb.pfnGetFileNameCb(&(pstTskCtx->stCurrentFileName), pstTskCtx->stGetFileNameCb.pvPrivateData);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Get FileName fail\n");
        return HI_FAILURE;
    }

    /**Name not same*/
    if (memcmp(pstTskCtx->stCurrentFileName.szDebugInfo_FileName, pstTskCtx->stFormerFileName.szDebugInfo_FileName,
        sizeof(pstTskCtx->stCurrentFileName.szDebugInfo_FileName)))
    {
        pstTskCtx->u32SamePrefixCnt = 0;
    }
    /**Name Same*/
    else
    {
        pstTskCtx->u32SamePrefixCnt++;
    }

    pstTskCtx->u32RawSavedCnt = 0;

    if (pstTskCtx->stDataSource.bDumpDebugInfo)
    {
        s32Ret = RAWCAP_DebugInfoProc(pstTskCtx, RAWCAP_DEBUGINFO_FRAME_DEPTH);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("SaveDebug failed.");
            return HI_FAILURE;
        }
    }

    HI_MAPI_VCAP_RAW_DATA_S stVCapRawData;
    stVCapRawData.pfn_VCAP_RawDataProc = RAWCAP_RAWDATA_DumpCallback;
    stVCapRawData.pPrivateData = (HI_VOID*)pstTskCtx;

    s32Ret = HI_MAPI_VCAP_StartDumpRaw(pstTskCtx->stDataSource.VcapPipeHdl, u32FrameCnt, &stVCapRawData);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VCAP_StartDumpRaw fail\n");
        return HI_FAILURE;
    }

    pstTskCtx->u32YuvSavedCnt = 0;

    if (pstTskCtx->stDataSource.bDumpYUV && pstTskCtx->stTriggerMode.bDumpYUV)
    {
        HI_DUMP_YUV_CALLBACK_FUNC_S pfunVProcYUVProc;
        pfunVProcYUVProc.pfunVProcYUVProc = RAWCAP_YUVDATA_DumpCallback;
        pfunVProcYUVProc.pPrivateData = (HI_VOID*)pstTskCtx;;

        s32Ret = HI_MAPI_VPROC_StartVpssDumpYUV(pstTskCtx->stDataSource.VpssHdl, pstTskCtx->stDataSource.VPortHdl, u32FrameCnt, &pfunVProcYUVProc);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VPROC_StartVpssDumpYUV fail\n");
            return HI_FAILURE;
        }

        s32Ret = HI_MAPI_VPROC_StopVpssDumpYUV(pstTskCtx->stDataSource.VpssHdl, pstTskCtx->stDataSource.VPortHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VPROC_StopVpssDumpYUV fail\n");
            return HI_FAILURE;
        }
    }

    s32Ret =  HI_MAPI_VCAP_StopDumpRaw(pstTskCtx->stDataSource.VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VCAP_StopDumpRaw fail\n");
        return HI_FAILURE;
    }

    memcpy(&(pstTskCtx->stFormerFileName), &(pstTskCtx->stCurrentFileName), sizeof(HI_RAWCAP_FILENAME_S));

    return HI_SUCCESS;
}

static HI_VOID RAWCAP_TimerDumpProc(HI_VOID* pClientData, struct timeval* nowP)
{
    MLOGI("Start Dump. ONCE START\n");
    HI_S32 s32Ret = HI_SUCCESS;
    RAWCAP_TASK_CONTEXT_S* pstTskCtx = (RAWCAP_TASK_CONTEXT_S*)pClientData;

    RAWCAP_SetTaskState(pstTskCtx->TskHdl, RAWCAP_TASK_STATE_ONCESTART);

    s32Ret = RAWCAP_DumpProc(pstTskCtx);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("DumpRaw proc error.\n");
    }

    RAWCAP_SetTaskState(pstTskCtx->TskHdl, RAWCAP_TASK_STATE_ONCEEND);

    HI_MUTEX_LOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
    HI_COND_SIGNAL(pstTskCtx->stRawcapEventMutexCond.rawcapEventCond);
    HI_MUTEX_UNLOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
    MLOGI("Start Dump. ONCE end\n");
    return ;
}

HI_S32 HI_RAWCAP_Init(HI_VOID)
{
    if (s_bRawCapInitFlg)
    {
        MLOGE("has already inited!\n");
        return HI_RAWCAP_EINITIALIZED;
    }
    HI_S32 s32Idx = 0;
    memset(&s_astRawCapTaskCtx[0], 0x0, sizeof(RAWCAP_TASK_CONTEXT_S) * RAWCAP_TASK_MAX_CNT);

    for (s32Idx = 0; s32Idx < RAWCAP_TASK_MAX_CNT; ++s32Idx)
    {
        RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[s32Idx];
        HI_MUTEX_INIT_LOCK(pstTskCtx->stateMutex);
        HI_MUTEX_INIT_LOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
        HI_COND_INIT(pstTskCtx->stRawcapEventMutexCond.rawcapEventCond);
    }
    s_s32TimerGrp = HI_Timer_Init(HI_FALSE);

    if (s_s32TimerGrp < 0)
    {
        MLOGE("HI_Timer_Init fail, s32TimerGrp: %d \n", s_s32TimerGrp);
        return HI_FAILURE;
    }
    HI_Timer_SetTickValue(s_s32TimerGrp, RAWCAP_TIMER_TICKVALUE_US);

    s_bRawCapInitFlg = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_CreateTask(const HI_RAWCAP_CFG_S *pstRawCapCfg, HI_HANDLE *pHdl)
{
    RAWCAP_CHECK_INIT();

    /* check input param */
    /**Source Param Can Not be Checked*/
    HI_APPCOMM_CHECK_POINTER(pHdl, HI_RAWCAP_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstRawCapCfg, HI_RAWCAP_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstRawCapCfg->stGetFilenameCb.pfnGetFileNameCb, HI_RAWCAP_EINVAL);
    if (!HI_APPCOMM_CHECK_RANGE(pstRawCapCfg->u32RawDepth, 0, 8))
    {
        MLOGE("Invalid Raw Cap Depth[%u]\n", pstRawCapCfg->u32RawDepth);
        return HI_RAWCAP_EINVAL;
    }

    /* check task count */
    RAWCAP_TASK_CONTEXT_S *pstTskCtx = NULL;
    HI_U32 u32Idx = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    for(u32Idx = 0; u32Idx < RAWCAP_TASK_MAX_CNT; ++u32Idx)
    {
        if(!s_astRawCapTaskCtx[u32Idx].bUsed)
        {
            pstTskCtx = &s_astRawCapTaskCtx[u32Idx];
            *pHdl = u32Idx;
            pstTskCtx->TskHdl = u32Idx;
            break;
        }
    }

    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(pstTskCtx, HI_RAWCAP_EMAXTASK, "beyond maximum task");
    memcpy(&(pstTskCtx->stDataSource), &(pstRawCapCfg->stDataSource), sizeof(HI_RAWCAP_DATA_SOURCE_S));

    if (pstTskCtx->stDataSource.bDumpYUV)
    {
        /**Enable Dump YUV and Raw Task*/
        HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
        stDumpYUVAttr.bEnable = HI_TRUE;
        stDumpYUVAttr.u32Depth = pstRawCapCfg->u32RawDepth;

        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstTskCtx->stDataSource.VpssHdl,     \
                             pstTskCtx->stDataSource.VPortHdl, &stDumpYUVAttr);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_MAPI_VPROC_SetVpssDumpYUVAttr fail\n");
            return HI_FAILURE;
        }
    }

    HI_MAPI_DUMP_ATTR_S stDumpAttr;
    memset(&stDumpAttr,0,sizeof(HI_MAPI_DUMP_ATTR_S));
    /**The PixFmt is constant*/
    stDumpAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
    stDumpAttr.stDumpAttr.bEnable = HI_TRUE;
    stDumpAttr.stDumpAttr.u32Depth = pstRawCapCfg->u32RawDepth;
    s32Ret = HI_MAPI_VCAP_SetDumpRawAttr(pstTskCtx->stDataSource.VcapPipeHdl, &stDumpAttr);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MAPI_VPROC_SetVpssDumpYUVAttr fail\n");
        return HI_FAILURE;
    }

    pstTskCtx->stGetFileNameCb.pfnGetFileNameCb = pstRawCapCfg->stGetFilenameCb.pfnGetFileNameCb;
    pstTskCtx->stGetFileNameCb.pvPrivateData = pstRawCapCfg->stGetFilenameCb.pvPrivateData;
    pstTskCtx->u32SamePrefixCnt = 0;
    pstTskCtx->stTriggerMode.bDumpBNR = HI_TRUE;
    pstTskCtx->stTriggerMode.bDumpYUV = HI_TRUE;
    pstTskCtx->bRawFileSeparated = pstRawCapCfg->bRawFileSeparated;
    pstTskCtx->bUsed = HI_TRUE;
    RAWCAP_SetTaskState(*pHdl, RAWCAP_TASK_STATE_READY);
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_DestroyTask(HI_HANDLE Hdl)
{
    RAWCAP_CHECK_INIT();
    RAWCAP_CHECK_TSKINDEX(Hdl);
    RAWCAP_CHECK_USED(Hdl);

    HI_S32 s32Ret = HI_SUCCESS;
    RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[Hdl];

    HI_MUTEX_LOCK(pstTskCtx->stateMutex);
    switch (pstTskCtx->enRawcapTaskState)
    {
        case RAWCAP_TASK_STATE_READY:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            break;
        default:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            MLOGE("task[%d] is busy\n", Hdl);
            return HI_RAWCAP_EBUSY;
    }

    /**Disable Dump YUV and Raw Task*/
    HI_MAPI_DUMP_ATTR_S stDumpAttr;
    memset(&stDumpAttr,0,sizeof(HI_MAPI_DUMP_ATTR_S));
    stDumpAttr.enPixFmt = PIXEL_FORMAT_BUTT;
    stDumpAttr.stDumpAttr.bEnable = HI_FALSE;
    stDumpAttr.stDumpAttr.u32Depth = 0;
    s32Ret = HI_MAPI_VCAP_SetDumpRawAttr(pstTskCtx->stDataSource.VcapPipeHdl, &stDumpAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if (pstTskCtx->stDataSource.bDumpYUV)
    {

        HI_MAPI_DUMP_YUV_ATTR_S stDumpYUVAttr;
        stDumpYUVAttr.bEnable = HI_FALSE;
        stDumpYUVAttr.u32Depth = 0;
        s32Ret = HI_MAPI_VPROC_SetVpssDumpYUVAttr(pstTskCtx->stDataSource.VpssHdl,     \
                             pstTskCtx->stDataSource.VPortHdl, &stDumpYUVAttr);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_BUTT);
    memset(&pstTskCtx->stGetFileNameCb, 0x0, sizeof(HI_RAWCAP_GET_FILENAME_S));
    memset(&pstTskCtx->stDataSource, 0x0, sizeof(HI_RAWCAP_DATA_SOURCE_S));
    pstTskCtx->bUsed = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_StartTask(HI_HANDLE Hdl, HI_U32 u32Interval_ms, HI_U32 u32Count)
{
    RAWCAP_CHECK_INIT();
    RAWCAP_CHECK_TSKINDEX(Hdl);
    RAWCAP_CHECK_USED(Hdl);

    if (!HI_APPCOMM_CHECK_RANGE(u32Count, 1, RAWCAP_FRAME_MAX_CNT))
    {
        MLOGE("Invalid Raw Cap Frame[%u]\n", u32Count);
        return HI_RAWCAP_EINVAL;
    }
    HI_APPCOMM_CHECK_EXPR(u32Interval_ms >= RAWCAP_TASK_MIN_INTERVAL, HI_RAWCAP_EINVAL);

    RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[Hdl];
    HI_TIMER_S stTskTimer;

    HI_MUTEX_LOCK(pstTskCtx->stateMutex);
    switch (pstTskCtx->enRawcapTaskState)
    {
        case RAWCAP_TASK_STATE_BUSY:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            MLOGD("task[%d] is busy\n", Hdl);
            return HI_RAWCAP_EBUSY;
        case RAWCAP_TASK_STATE_READY:
        default:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            break;
    }

    RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_BUSY);
    pstTskCtx->u32FrameCnt = u32Count;
    pstTskCtx->TimedTaskHdl = HI_NULL;
    memset(&stTskTimer, 0x00, sizeof(HI_TIMER_S));
    stTskTimer.now = HI_NULL;
    stTskTimer.interval_ms = u32Interval_ms;
    stTskTimer.periodic = HI_TRUE;
    stTskTimer.timer_proc = RAWCAP_TimerDumpProc;
    stTskTimer.clientData = (HI_VOID*)&s_astRawCapTaskCtx[Hdl];
    pstTskCtx->TimedTaskHdl = HI_Timer_Create(s_s32TimerGrp, &stTskTimer);
    if (HI_NULL == pstTskCtx->TimedTaskHdl)
    {
        RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_READY);
        MLOGE(RED"start task failure\n"NONE);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_StopTask(HI_HANDLE Hdl)
{
    RAWCAP_CHECK_INIT();
    RAWCAP_CHECK_TSKINDEX(Hdl);
    RAWCAP_CHECK_USED(Hdl);

    HI_S32 s32Ret = HI_SUCCESS;
    RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[Hdl];

    HI_MUTEX_LOCK(pstTskCtx->stateMutex);
    switch (pstTskCtx->enRawcapTaskState)
    {
        case RAWCAP_TASK_STATE_READY:
        case RAWCAP_TASK_STATE_BUTT:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            MLOGD("task[%d] has already stoped\n", Hdl);
            return HI_SUCCESS;
        case RAWCAP_TASK_STATE_BUSY:
        case RAWCAP_TASK_STATE_ONCESTART:
        case RAWCAP_TASK_STATE_ONCEEND:
        default:
            break;
    }

    RAWCAP_TASK_STATE_E enTempTaskState = pstTskCtx->enRawcapTaskState;
    HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);

    if (HI_NULL != pstTskCtx->TimedTaskHdl)
    {
        s32Ret = HI_Timer_Destroy(s_s32TimerGrp, pstTskCtx->TimedTaskHdl);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE(RED"Time task(%p) destroy error\n"NONE,pstTskCtx->TimedTaskHdl);
            RAWCAP_SetTaskState(Hdl, enTempTaskState);
            pstTskCtx->TimedTaskHdl = HI_NULL;
            return HI_FAILURE;
        }
        pstTskCtx->TimedTaskHdl = HI_NULL;
    }

    if (RAWCAP_TASK_STATE_BUSY == pstTskCtx->enRawcapTaskState || RAWCAP_TASK_STATE_ONCEEND == pstTskCtx->enRawcapTaskState)
    {
        RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_READY);
        return HI_SUCCESS;
    }
    else
    {
        HI_MUTEX_LOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
        MLOGI("wait TaskEndCond \n");
        HI_COND_TIMEDWAIT_WITH_RETURN(pstTskCtx->stRawcapEventMutexCond.rawcapEventCond,
            pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex, RAWCAP_WAIT_SAVE_TIMEOUT*1000*1000, s32Ret);
        if (HI_SUCCESS != s32Ret)
        {

            MLOGE("pthread_cond_timedwait failed, s32Ret:%d \n", s32Ret);
            HI_MUTEX_UNLOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
            RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_READY);
            return HI_FAILURE;
        }
        HI_MUTEX_UNLOCK(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
    }
    RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_READY);
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_Trigger(HI_HANDLE Hdl, HI_U32 u32Count, HI_RAWCAP_TRIGGER_MODE_S* pstTriggerMode)
{
    RAWCAP_CHECK_INIT();
    RAWCAP_CHECK_TSKINDEX(Hdl);
    RAWCAP_CHECK_USED(Hdl);

    if (!HI_APPCOMM_CHECK_RANGE(u32Count, 1, RAWCAP_FRAME_MAX_CNT))
    {
        MLOGE("Invalid Raw Cap Frame[%u]\n", u32Count);
        return HI_RAWCAP_EINVAL;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[Hdl];
    HI_MUTEX_LOCK(pstTskCtx->stateMutex);
    switch (pstTskCtx->enRawcapTaskState)
    {
        case RAWCAP_TASK_STATE_BUSY:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            MLOGD("task[%d] is busy\n", Hdl);
            return HI_RAWCAP_EBUSY;
        case RAWCAP_TASK_STATE_READY:
        default:
            HI_MUTEX_UNLOCK(pstTskCtx->stateMutex);
            break;
    }

    RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_BUSY);
    pstTskCtx->u32FrameCnt = u32Count;
    pstTskCtx->stTriggerMode.bDumpBNR = pstTriggerMode->bDumpBNR;
    pstTskCtx->stTriggerMode.bDumpYUV = pstTriggerMode->bDumpYUV;

    s32Ret = RAWCAP_DumpProc((RAWCAP_TASK_CONTEXT_S*)pstTskCtx);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    pstTskCtx->stTriggerMode.bDumpBNR = HI_TRUE;
    pstTskCtx->stTriggerMode.bDumpYUV = HI_TRUE;
    RAWCAP_SetTaskState(Hdl, RAWCAP_TASK_STATE_READY);
    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_SaveYUV(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S* pVPortYUV, HI_VOID* pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE* pHdl = (HI_HANDLE*)pPrivateData;
    HI_RAWCAP_FILENAME_S stCurrentFileName;
    HI_CHAR szYUVFileName[HI_RAWCAP_MAX_FILENAME_LEN];

    FILE* pfd = NULL;
    RAWCAP_TASK_CONTEXT_S* pstTskCtx = &(s_astRawCapTaskCtx[*pHdl]);

    s32Ret = pstTskCtx->stGetFileNameCb.pfnGetFileNameCb(&stCurrentFileName, pstTskCtx->stGetFileNameCb.pvPrivateData);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Get FileName fail\n");
        return HI_FAILURE;
    }

    MLOGD("The YUVFrame's  u32Width is %d \n", pVPortYUV->u32Width);
    snprintf(szYUVFileName, sizeof(szYUVFileName), "%s_sameprefix%d_pipe%d_sameaction%d.yuv", stCurrentFileName.szYUV_FileName, 0, pstTskCtx->stDataSource.VcapPipeHdl, pstTskCtx->u32YuvSavedCnt);

    pfd = fopen(szYUVFileName, "wb");
    MLOGD("YUVFileName is %s.\n",szYUVFileName);
    if(pfd > 0)
    {
        s32Ret = RAWCAP_YUVDATA_Save(pVPortYUV, pfd, pstTskCtx);
        fclose(pfd);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("RawData_Save fail\n");
            return HI_FAILURE;
        }
    }
    else
    {
        MLOGE("open file fail\n");
        perror("A");
        return HI_FAILURE;
    }

    pstTskCtx->u32YuvSavedCnt++;
    MLOGD("RAWCAP_YUVDATA_DumpCallback is End.\n");

    return HI_SUCCESS;
}

HI_S32 HI_RAWCAP_SaveBNR(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S* pVPortRaw, HI_VOID* pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE* pHdl = (HI_HANDLE*)pPrivateData;
    HI_RAWCAP_FILENAME_S stCurrentFileName;
    HI_CHAR szRawFileName[HI_RAWCAP_MAX_FILENAME_LEN];

    FILE* pfd = NULL;
    RAWCAP_TASK_CONTEXT_S* pstTskCtx = &(s_astRawCapTaskCtx[*pHdl]);

    s32Ret = pstTskCtx->stGetFileNameCb.pfnGetFileNameCb(&stCurrentFileName, pstTskCtx->stGetFileNameCb.pvPrivateData);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Get FileName fail\n");
        return HI_FAILURE;
    }

    MLOGD("The RawFrame's  u32Width is %d \n", pVPortRaw->u32Width);


    snprintf(szRawFileName, sizeof(szRawFileName), "%s_sameprefix%d_pipe%d_aggregate%d_BNR.raw", stCurrentFileName.szRaw_FileName, 0, pstTskCtx->stDataSource.VcapPipeHdl,1);
    pfd = fopen(szRawFileName, "wb");
    MLOGD("RawFileName is %s.\n",szRawFileName);
    if(pfd > 0)
    {
        s32Ret = RAWCAP_RAWDATA_Save(pVPortRaw, pfd, 1);
        fclose(pfd);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGE("RawData_Save fail\n");
            return HI_FAILURE;
        }
    }
    else
    {
        MLOGE("open file fail\n");
        perror("A");
        return HI_FAILURE;
    }

    MLOGD("RAWCAP_BNRDATA_DumpCallback is End.\n");

    return HI_SUCCESS;
}


HI_S32 HI_RAWCAP_Deinit(HI_VOID)
{
    RAWCAP_CHECK_INIT();

    HI_S32 s32Idx = 0;
    HI_S32 s32Ret;
    for (s32Idx = 0; s32Idx < RAWCAP_TASK_MAX_CNT; ++s32Idx)
    {
        RAWCAP_TASK_CONTEXT_S *pstTskCtx = &s_astRawCapTaskCtx[s32Idx];
        if(pstTskCtx->bUsed)
        {
            s32Ret = HI_RAWCAP_DestroyTask(s32Idx);
            HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
        }
        HI_MUTEX_DESTROY(pstTskCtx->stateMutex);
        HI_MUTEX_DESTROY(pstTskCtx->stRawcapEventMutexCond.rawcapEventMutex);
        HI_COND_DESTROY(pstTskCtx->stRawcapEventMutexCond.rawcapEventCond);
    }

    s32Ret =  HI_Timer_DeInit(s_s32TimerGrp);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_Timer_DeInit fail, s32Ret:0x%x \n", s32Ret);
    }

    s_bRawCapInitFlg = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
