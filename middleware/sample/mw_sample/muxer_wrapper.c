#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include "muxer_wrapper.h"
#include "hi_mp4_format.h"
#include "venc_wrapper.h"

static const HI_U32 WRITE_FRAME_TIMEOUT  = 200;
static const HI_U32 MAX_MUXER_NUM = 5;
extern HI_U32 g_u32StreamCnt;
static HI_U32 s_u32MuxerNum = 0;
static HI_MW_PTR s_ahMuxerThm[] = {0, 0, 0, 0, 0};
static HI_BOOL s_bThmReceived = HI_FALSE;

static pthread_mutex_t s_ThmLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_ThmCond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t s_FirstWriteLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_SecondWriteLock = PTHREAD_MUTEX_INITIALIZER;


static HI_S32 WriteThm(HI_MW_PTR hMuxerHandle, const HI_U8* pu8Data, HI_U32 u32Len)
{
    HI_MP4_ATOM_INFO_S stAtomInfo;
    printf("write thm len: %d\n", u32Len);
    stAtomInfo.u32DataLen = u32Len;
    stAtomInfo.pu8DataBuf = (HI_U8*)pu8Data;
    stAtomInfo.aszType[0] = 't';
    stAtomInfo.aszType[1] = 'h';
    stAtomInfo.aszType[2] = 'm';
    stAtomInfo.aszType[3] = ' ';
    stAtomInfo.aszType[4] = '\0';
    return HI_MP4_AddAtom(hMuxerHandle, "/", &stAtomInfo);
}

static void InsertMuxer(HI_MW_PTR hMuxerHandle)
{
    HI_U32 i = 0;
    for (; i < MAX_MUXER_NUM; i++)
    {
        if (0 == s_ahMuxerThm[i])
        {
            break;
        }
    }
    if (i > 1)
    {
        printf("not enough muxer\n");
    }
    else
    {
        s_ahMuxerThm[i] = hMuxerHandle;
    }
}

static HI_BOOL RemoveMuxer(HI_MW_PTR hMuxerHandle)
{
    HI_BOOL bRet = HI_FALSE;
    HI_U32 i = 0;
    for (; i < MAX_MUXER_NUM; i++)
    {
        if (hMuxerHandle == s_ahMuxerThm[i])
        {
            s_ahMuxerThm[i] = 0;
            bRet = HI_TRUE;
            break;
        }
    }
    return bRet;
}


HI_S32 OnThmReceived(HI_U8* u8Data, HI_U32 u32DataLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    int i = 0;
    for (; i < 2; i++)
    {
        if (0 != s_ahMuxerThm[i])
        {
            s32Ret |= WriteThm(s_ahMuxerThm[i], u8Data, u32DataLen);
        }
    }
    (void)pthread_mutex_lock(&s_ThmLock);
    s_bThmReceived = HI_TRUE;
    (void)pthread_cond_signal(&s_ThmCond);
    (void)pthread_mutex_unlock(&s_ThmLock);
    return s32Ret;
}

typedef struct hiREPAIR_CONTEXT_S
{
    HI_CHAR  aszBackOne[HI_MP4_MAX_FILE_NAME];/*back file name*/
    HI_CHAR  aszBackTwo[HI_MP4_MAX_FILE_NAME];/*back file name*/
    FILE* pFileFdOne;
    FILE* pFileFdTwo;
    HI_BOOL bWriteOne;
} HI_REPAIR_CONTEXT_S;

HI_S32 Repair(HI_CHAR* pszBackFilePath)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = HI_MP4_RepairFile(pszBackFilePath, HI_TRUE);

    return s32Ret;
}

HI_S32 CreateRepair(HI_MW_PTR* pHandle, const HI_CHAR* pszFileName)
{
    printf("create repair for %s\n", pszFileName);
    HI_REPAIR_CONTEXT_S* pstRepairContext = (HI_REPAIR_CONTEXT_S*)malloc(sizeof(HI_REPAIR_CONTEXT_S));
    memset(pstRepairContext, 0, sizeof(HI_REPAIR_CONTEXT_S));
    snprintf(pstRepairContext->aszBackOne, HI_MP4_MAX_FILE_NAME, "%s.bak1", pszFileName);
    snprintf(pstRepairContext->aszBackTwo, HI_MP4_MAX_FILE_NAME, "%s.bak2", pszFileName);
    pstRepairContext->bWriteOne = HI_TRUE;
    pstRepairContext->pFileFdOne = fopen(pstRepairContext->aszBackOne, "w+");
    if (NULL == pstRepairContext->pFileFdOne)
    {
        free(pstRepairContext);
        return HI_FAILURE;
    }
    pstRepairContext->pFileFdTwo = fopen(pstRepairContext->aszBackTwo, "w+");
    if (NULL == pstRepairContext->pFileFdTwo)
    {
        fclose(pstRepairContext->pFileFdOne);
        free(pstRepairContext);
        return HI_FAILURE;
    }
    *pHandle = (HI_MW_PTR)pstRepairContext;
    return HI_SUCCESS;
}

HI_S32 RepairUpdate(HI_MW_PTR hHandle, HI_U8* pu8Buf, HI_U32 u32BufSize)
{
    printf("repair update\n");
    HI_REPAIR_CONTEXT_S* pstRepairContext = (HI_REPAIR_CONTEXT_S*)hHandle;
    FILE* pWriteFile = NULL;
    const HI_CHAR* pszFileName = NULL;
    HI_S32 s32FileNo = -1;
    if (NULL == pstRepairContext)
    {
        return HI_FAILURE;
    }
    if (pstRepairContext->bWriteOne)
    {
        pWriteFile = pstRepairContext->pFileFdOne;
        pstRepairContext->bWriteOne = HI_FALSE;
        pszFileName = pstRepairContext->aszBackOne;
    }
    else
    {
        pWriteFile = pstRepairContext->pFileFdTwo;
        pstRepairContext->bWriteOne = HI_TRUE;
        pszFileName = pstRepairContext->aszBackTwo;
    }
    if (-1 == fseeko(pWriteFile, 0, SEEK_SET))
    {
        printf("fseeko fail %s\n", strerror(errno));
        return HI_FAILURE;
    }
    if (1 != fwrite(pu8Buf, u32BufSize, 1, pWriteFile))
    {
        printf("fwrite moov fail\n");
        return HI_FAILURE;
    }
    if (EOF == fflush(pWriteFile))
    {
        printf("fflush error\n");
    }
    s32FileNo = fileno(pWriteFile);
    if (-1 != s32FileNo)
    {
        printf("repair before fsync name: %s size: %d\n", pszFileName, u32BufSize);
        if (-1 == fsync(s32FileNo))
        {
            printf("call fsync fail %s\n", strerror(errno));
            return HI_FAILURE;
        }
        printf("repair after fsync name: %s size: %d\n", pszFileName, u32BufSize);
    }
    return HI_SUCCESS;
}

HI_S32 DestroyRepair(HI_MW_PTR hHandle, HI_BOOL bCleanBackup)
{
    printf("destroy repair\n");
    HI_REPAIR_CONTEXT_S* pstRepairContext = (HI_REPAIR_CONTEXT_S*)hHandle;
    if (NULL != pstRepairContext)
    {
        fclose(pstRepairContext->pFileFdOne);
        fclose(pstRepairContext->pFileFdTwo);
        if (bCleanBackup)
        {
            if (-1 == remove(pstRepairContext->aszBackOne))
            {
                printf("rm %s fail\n", pstRepairContext->aszBackOne);
            }
            if (-1 == remove(pstRepairContext->aszBackTwo))
            {
                printf("rm %s fail\n", pstRepairContext->aszBackTwo);
            }
        }
        free(pstRepairContext);
        pstRepairContext = NULL;
    }
    return HI_SUCCESS;
}

HI_S32 RepairInit(HI_S32 s32RepairFreq)
{


    return HI_SUCCESS;
}

HI_S32 RepairDeInit()
{
    return HI_SUCCESS;
}

HI_S32 ExtractThm(const HI_CHAR* pszMP4File, const HI_CHAR* pszThmFile)
{
    HI_MP4_CONFIG_S stMuxerConfig = {0};
    HI_MW_PTR pDemuxerHandle;
    HI_MP4_ATOM_INFO_S stAtomInfo = {0};

    stMuxerConfig.enConfigType = HI_MP4_CONFIG_DEMUXER;
    strncpy(stMuxerConfig.aszFileName, pszMP4File, HI_MP4_MAX_FILE_NAME);
    stMuxerConfig.stDemuxerConfig.u32VBufSize = 1024 * 1024;

    if (HI_SUCCESS != HI_MP4_Create(&pDemuxerHandle, &stMuxerConfig))
    {
        printf("call HI_MP4_Create fail\n");
        return HI_FAILURE;
    }
    stAtomInfo.u32DataLen = 100 * 1024;
    stAtomInfo.pu8DataBuf = (HI_U8*)malloc(stAtomInfo.u32DataLen);
    if (NULL == stAtomInfo.pu8DataBuf)
    {
        printf("malloc atom buf fail\n");
        HI_MP4_Destroy(pDemuxerHandle, NULL);
        return HI_FAILURE;
    }
    if (HI_SUCCESS != HI_MP4_GetAtom(pDemuxerHandle, "/thm ", &stAtomInfo))
    {
        printf("call HI_MP4_GetAtom fail\n");
        HI_MP4_Destroy(pDemuxerHandle, NULL);
        free(stAtomInfo.pu8DataBuf);
        return HI_FAILURE;
    }
    FILE* pThmFile = fopen(pszThmFile, "w");
    if (NULL == pThmFile)
    {
        printf("open for write thm file wrong\n");
        HI_MP4_Destroy(pDemuxerHandle, NULL);
        free(stAtomInfo.pu8DataBuf);
        return HI_FAILURE;
    }
    printf("thm size is %d\n", stAtomInfo.u32DataLen);
    if (1 != fwrite(stAtomInfo.pu8DataBuf, stAtomInfo.u32DataLen, 1, pThmFile))
    {
        printf("fwrite fail\n");
        HI_MP4_Destroy(pDemuxerHandle, NULL);
        free(stAtomInfo.pu8DataBuf);
        fclose(pThmFile);
        return HI_FAILURE;
    }
    HI_MP4_Destroy(pDemuxerHandle, NULL);
    free(stAtomInfo.pu8DataBuf);
    fclose(pThmFile);
    return HI_SUCCESS;
}

HI_S32 CreateMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerConfig, HI_MW_PTR* pMuxerHandle)
{
    HI_MP4_CONFIG_S stMuxerConfig = {0};
    HI_S32 s32Ret = HI_FAILURE;
    MuxerConfig_S* pstMuxerConfig = (MuxerConfig_S*)pMuxerConfig;
    stMuxerConfig.enConfigType = HI_MP4_CONFIG_MUXER;
    strncpy(stMuxerConfig.aszFileName, pszFileName, HI_MP4_MAX_FILE_NAME);

    if (pstMuxerConfig->s32FileAllocSize > 0)
    {
        stMuxerConfig.stMuxerConfig.u32PreAllocUnit = 20 * 1024 * 1024;
    }
    else
    {
        stMuxerConfig.stMuxerConfig.u32PreAllocUnit = 0;
    }
    stMuxerConfig.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerConfig.stMuxerConfig.bConstantFps = HI_FALSE;
    stMuxerConfig.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerConfig.stMuxerConfig.u32BackupUnit = 50 * 1024 * 1024;
    //stMuxerConfig.u32BackupUnit = 0;
    s32Ret = HI_MP4_Create(pMuxerHandle, &stMuxerConfig);
    if (HI_SUCCESS == s32Ret)
    {
        InsertMuxer(*pMuxerHandle);
        s_u32MuxerNum++;
        if (g_u32StreamCnt == s_u32MuxerNum)
        {
            (void)pthread_mutex_lock(&s_ThmLock);
            s_bThmReceived = HI_FALSE;
            (void)pthread_mutex_unlock(&s_ThmLock);
            RequestThumbnail(OnThmReceived);
        }
    }
    pstMuxerConfig->hMuxerHandle = *pMuxerHandle;
    return s32Ret;
}

HI_S32 DestroyMuxer(HI_MW_PTR hMuxerHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_FALSE == RemoveMuxer(hMuxerHandle))
    {
        printf("muxer not fount\n");
    }
    if (g_u32StreamCnt == s_u32MuxerNum)
    {
        struct timespec timeout = {0, 0};
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_nsec += 200000000l;
        if (timeout.tv_nsec > 1000000000l)
        {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000l;
        }
        (void)pthread_mutex_lock(&s_ThmLock);
        while (HI_FALSE == s_bThmReceived)
        {
            if (ETIMEDOUT == pthread_cond_timedwait(&s_ThmCond, &s_ThmLock, &timeout))
            {
                printf("wait thm timeout\n");
                break;
            }
        }
        (void)pthread_mutex_unlock(&s_ThmLock);
        RequestThumbnailEnd(OnThmReceived);
    }
    s_u32MuxerNum--;
    if (HI_SUCCESS != HI_MP4_DestroyAllTracks(hMuxerHandle, NULL))
    {
        printf("call HI_MP4_DestroyAllTracks fail\n");
    }
    printf("HI_MP4_DestroyAllTracks success\n");

    s32Ret = HI_MP4_Destroy(hMuxerHandle, NULL);

    return s32Ret;
}

HI_S32 CreateSlaveMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerConfig, HI_MW_PTR* pMuxerHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //set muxhandle to first muxer handle.
    MuxerConfig_S* pstMuxerConfig = (MuxerConfig_S*)pMuxerConfig;
    *pMuxerHandle = pstMuxerConfig->hMuxerHandle;
    printf("CreateSlaveMuxer 0x%x\n", *pMuxerHandle);
    return s32Ret;
}

HI_S32 DestroySlaveMuxer(HI_MW_PTR hMuxerHandle)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

HI_S32 CreateSlaveTrack(HI_MW_PTR hMuxerHandle, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* phTrackHandle)
{
    printf("CreateSlaveTrack 0x%x\n", hMuxerHandle);
    HI_MP4_TRACK_INFO_S stTrakInfo;
    if (HI_TRACK_SOURCE_TYPE_VIDEO == pstTrackSrcHandle->enTrackType)
    {
        stTrakInfo.enTrackType = HI_MP4_STREAM_DATA;
        stTrakInfo.stDataInfo.u32Width = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Width;
        stTrakInfo.stDataInfo.u32Height = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Height;
        snprintf(stTrakInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon MetaData");
        stTrakInfo.stDataInfo.enCodecID = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType == HI_TRACK_VIDEO_CODEC_H264 ? HI_MP4_CODEC_ID_H264 : HI_MP4_CODEC_ID_H265;
        stTrakInfo.fSpeed = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.fSpeed;
        stTrakInfo.u32TimeScale = 120000;
    }
    else
    {
        printf("slave track not video type\n");
        return HI_FAILURE;
    }
    return HI_MP4_CreateTrack(hMuxerHandle, phTrackHandle, &stTrakInfo);
}

HI_S32 CreateTrack(HI_MW_PTR hMuxerHandle, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* phTrackHandle)
{
    HI_MP4_TRACK_INFO_S stTrakInfo;
    if (HI_TRACK_SOURCE_TYPE_VIDEO == pstTrackSrcHandle->enTrackType)
    {
        stTrakInfo.enTrackType = HI_MP4_STREAM_VIDEO;
        snprintf(stTrakInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");
        stTrakInfo.stVideoInfo.u32Width = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Width;
        stTrakInfo.stVideoInfo.u32Height = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Height;
        stTrakInfo.stVideoInfo.u32BitRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32BitRate;
        stTrakInfo.stVideoInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;
        stTrakInfo.stVideoInfo.enCodecID = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType == HI_TRACK_VIDEO_CODEC_H264 ? HI_MP4_CODEC_ID_H264 : HI_MP4_CODEC_ID_H265;
        stTrakInfo.fSpeed = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.fSpeed;
        stTrakInfo.u32TimeScale = 120000;
    }
    else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstTrackSrcHandle->enTrackType)
    {
        stTrakInfo.enTrackType = HI_MP4_STREAM_AUDIO;
        snprintf(stTrakInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");
        stTrakInfo.stAudioInfo.u32Channels = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32ChnCnt;
        stTrakInfo.stAudioInfo.u32SampleRate = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
        stTrakInfo.stAudioInfo.u16SampleSize = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u16SampleBitWidth;
        stTrakInfo.stAudioInfo.u32SamplePerFrame = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame;
        stTrakInfo.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
        stTrakInfo.fSpeed = 1;
        stTrakInfo.u32TimeScale = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
    }
    return HI_MP4_CreateTrack(hMuxerHandle, phTrackHandle, &stTrakInfo);
}

HI_S32 WriteFrame(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S* pstFrameData)
{
    struct timeval start;
    struct timeval end;
    unsigned  long diff;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MP4_FRAME_DATA_S stFrameData;
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;
    (void)gettimeofday(&start, NULL);
    s32Ret =  HI_MP4_WriteFrame(hMuxerHandle, hTrackHandle, &stFrameData);
    (void)gettimeofday(&end, NULL);
    diff = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    if (diff > WRITE_FRAME_TIMEOUT)
    {
        printf(" WriteFrame len %d cost %ld ms\n", stFrameData.u32DataLength, diff);
    }
    return s32Ret;
}


HI_S32 WriteFrameToFirst(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S* pstFrameData)
{
    (void)pthread_mutex_lock(&s_FirstWriteLock);
    struct timeval start;
    struct timeval end;
    unsigned  long diff;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MP4_FRAME_DATA_S stFrameData;
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;
    gettimeofday(&start, NULL);
    s32Ret =  HI_MP4_WriteFrame(hMuxerHandle, hTrackHandle, &stFrameData);
    gettimeofday(&end, NULL);
    diff = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    if (diff > WRITE_FRAME_TIMEOUT)
    {
        printf(" WriteFrame len %d cost %ld ms\n", stFrameData.u32DataLength, diff);
    }
    (void)pthread_mutex_unlock(&s_FirstWriteLock);
    return s32Ret;
}

HI_S32 WriteFrameToSecond(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S* pstFrameData)
{
    (void)pthread_mutex_lock(&s_SecondWriteLock);
    struct timeval start;
    struct timeval end;
    unsigned  long diff;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MP4_FRAME_DATA_S stFrameData;
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;
    gettimeofday(&start, NULL);
    s32Ret =  HI_MP4_WriteFrame(hMuxerHandle, hTrackHandle, &stFrameData);
    gettimeofday(&end, NULL);
    diff = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    if (diff > WRITE_FRAME_TIMEOUT)
    {
        printf(" WriteFrame len %d cost %ld ms\n", stFrameData.u32DataLength, diff);
    }
    (void)pthread_mutex_unlock(&s_SecondWriteLock);
    return s32Ret;
}

