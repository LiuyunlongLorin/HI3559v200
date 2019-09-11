#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/vfs.h>
#include <sys/mount.h>
#include <mntent.h>
#include "hi_mw_type.h"
#include "avc.h"
#include "aac.h"
#include "hevc.h"
#include "hi_track_source.h"
#include "hi_recorder_pro.h"
#include "hi_mp4_format.h"

#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#define MAX_READ_PTS_US (200 * 1000)

#define STREAM_CNT (2)

#define TRACK_CNT (2)

#define SRC_FILE_NAME_LEN (128)

#define MANUAL_SPLIT_TIME_S (3)
static const HI_CHAR *MOUNT_POINT = "/tmp";
#ifdef __HuaweiLite__
static const HI_CHAR *SDCARD_DEV = "/dev/mmcblk0p0";
#else
static const HI_CHAR *SDCARD_DEV = "/dev/mmcblk0p1";
#endif

#ifdef __HuaweiLite__
#define MAIN_H264_ES_NAME  "/home/mainstream.h264"
#define SLAVE_H264_ES_NAME "/home/slavestream.h264"
#define MAIN_AAC_ES_NAME   "/home/mainstream.aac"
#define SLAVE_AAC_ES_NAME  "/home/slavestream.aac"
#else
#define MAIN_H264_ES_NAME  "mainstream.h264"
#define SLAVE_H264_ES_NAME "slavestream.h264"
#define MAIN_AAC_ES_NAME   "mainstream.aac"
#define SLAVE_AAC_ES_NAME  "slavestream.aac"
#endif

static const HI_CHAR *REC_DIR = "RECTEST";

typedef struct hiSampleAVCSource_S {
    pthread_t readFrameThd;
    AVC_Handle_S avcHandle;
    HI_S32 s32privateHdl;
    HI_BOOL bRead;
    HI_MW_PTR hRecHandle;
    HI_Track_Source_Handle hTrackSrcHandle;
    HI_CHAR aszSrcFileName[SRC_FILE_NAME_LEN];
    HI_U32 u32FrameInterval;  // every frame cnt write to recorder
    HI_U64 u64CoPts;
} SAMPLE_AVC_SRC_S;

typedef struct hiSampleAACSource_S {
    pthread_t readFrameThd;
    AAC_Handle_S aacHandle;
    HI_S32 s32privateHdl;
    HI_BOOL bRead;
    HI_MW_PTR hRecHandle;
    HI_Track_Source_Handle hTrackSrcHandle;
    HI_CHAR aszSrcFileName[SRC_FILE_NAME_LEN];
    HI_U32 u32FrameInterval;  // every frame cnt write to recorder
    HI_U64 u64CoPts;
} SAMPLE_AAC_SRC_S;

typedef struct hiSampleHEVCSource_S {
    pthread_t readFrameThd;
    HEVC_Handle_S hevcHandle;
    HI_S32 s32privateHdl;
    HI_BOOL bRead;
    HI_MW_PTR hRecHandle;
    HI_Track_Source_Handle hTrackSrcHandle;
    HI_CHAR aszSrcFileName[SRC_FILE_NAME_LEN];
    HI_U32 u32FrameInterval;  // every frame cnt write to recorder
    HI_U64 u64CoPts;
} SAMPLE_HEVC_SRC_S;

static HI_Track_Source_S astTrackSource[STREAM_CNT][TRACK_CNT];

static SAMPLE_AVC_SRC_S astAVCSrc[STREAM_CNT];
static SAMPLE_AAC_SRC_S astAACSrc[STREAM_CNT];
static SAMPLE_HEVC_SRC_S astHEVCSrc[STREAM_CNT];
static HI_U64 s_u64BasePts;

static HI_VOID *Sample_ReadAVCThread(void *pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAVCThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 *u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    HI_U64 u64frameCnt = 0;

    HI_REC_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_REC_FRAME_DATA_S));
    SAMPLE_AVC_SRC_S *pAvcCtx = (SAMPLE_AVC_SRC_S *)pArg;
    HI_U32 u32FrameInterVal = pAvcCtx->u32FrameInterval;
    struct timeval start, end;
    HI_S64 diff = 0;
    while (HI_FALSE != pAvcCtx->bRead) {
        s32Ret = AVC_ReadFrame(&pAvcCtx->avcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret) {
            printf("AVC_Reset begin\n");
            AVC_Reset(&pAvcCtx->avcHandle);
            continue;
        }
        stFrameData.u32SliceCnt = 1;
        stFrameData.apu8SliceAddr[0] = u8Frame;
        stFrameData.au32SliceLen[0] = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL)u8KeyFrame;
        stFrameData.u64TimeStamp = u64frameCnt * u32FrameInterVal * 1000 + s_u64BasePts;
        astAACSrc[pAvcCtx->s32privateHdl].u64CoPts = stFrameData.u64TimeStamp;
        HI_REC_WriteData(pAvcCtx->hRecHandle, pAvcCtx->hTrackSrcHandle, &stFrameData);
        diff = stFrameData.u64TimeStamp - pAvcCtx->u64CoPts;
        if (diff > MAX_READ_PTS_US) {
            usleep(MAX_READ_PTS_US);
        } else {
            usleep(u32FrameInterVal * 1000);
        }
        u64frameCnt++;
    }
    return NULL;
}

static HI_VOID *Sample_ReadHEVCThread(void *pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadHEVCThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U8 *u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    HI_U64 u64frameCnt = 0;

    HI_REC_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_REC_FRAME_DATA_S));
    struct timeval start, end;
    HI_S64 diff = 0;
    SAMPLE_HEVC_SRC_S *pHevcCtx = (SAMPLE_HEVC_SRC_S *)pArg;
    HI_U32 u32FrameInterVal = pHevcCtx->u32FrameInterval;
    while (HI_FALSE != pHevcCtx->bRead) {
        gettimeofday(&start, NULL);
        s32Ret = HEVC_ReadFrame(&pHevcCtx->hevcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret) {
            printf("HEVC_Reset begin\n");
            HEVC_Reset(&pHevcCtx->hevcHandle);
            continue;
        }
        stFrameData.u32SliceCnt = 1;
        stFrameData.apu8SliceAddr[0] = u8Frame;
        stFrameData.au32SliceLen[0] = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL)u8KeyFrame;
        stFrameData.u64TimeStamp = u64frameCnt * u32FrameInterVal * 1000 + s_u64BasePts;
        HI_REC_WriteData(pHevcCtx->hRecHandle, pHevcCtx->hTrackSrcHandle, &stFrameData);
        usleep(u32FrameInterVal * 1000);
        if (diff > MAX_READ_PTS_US) {
            usleep(MAX_READ_PTS_US);
        } else {
            usleep(u32FrameInterVal * 1000);
        }
        u64frameCnt++;
    }
    return NULL;
}

static HI_VOID *Sample_ReadAACThread(void *pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAACThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U8 *u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U64 u64frameCnt = 0;

    HI_REC_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_REC_FRAME_DATA_S));
    struct timeval start, end;
    HI_S64 diff = 0;
    SAMPLE_AAC_SRC_S *pAACCtx = (SAMPLE_AAC_SRC_S *)pArg;
    HI_U32 u32FrameInterVal = pAACCtx->u32FrameInterval;
    while (HI_FALSE != pAACCtx->bRead) {
        s32Ret = AAC_ReadFrame(&pAACCtx->aacHandle, &u8Frame, &u32FrameLen);
        if (HI_SUCCESS != s32Ret) {
            printf("AAC_Reset begin\n");
            AAC_Reset(&pAACCtx->aacHandle);
            continue;
        }
        stFrameData.u32SliceCnt = 1;
        stFrameData.apu8SliceAddr[0] = u8Frame;
        stFrameData.au32SliceLen[0] = u32FrameLen;
        stFrameData.bKeyFrameFlag = HI_FALSE;
        stFrameData.u64TimeStamp = u64frameCnt * u32FrameInterVal * 1000 + s_u64BasePts;
        astAVCSrc[pAACCtx->s32privateHdl].u64CoPts = stFrameData.u64TimeStamp;
        astHEVCSrc[pAACCtx->s32privateHdl].u64CoPts = stFrameData.u64TimeStamp;
        diff = stFrameData.u64TimeStamp - pAACCtx->u64CoPts;
        HI_REC_WriteData(pAACCtx->hRecHandle, pAACCtx->hTrackSrcHandle, &stFrameData);

        if (diff > MAX_READ_PTS_US) {
            usleep(MAX_READ_PTS_US);
        } else {
            usleep(u32FrameInterVal * 1000);
        }
        u64frameCnt++;
    }
    return NULL;
}

static HI_VOID Sample_FileOptInit()
{
    mount(SDCARD_DEV, MOUNT_POINT, "vfat", MS_NOEXEC, 0);
    HI_CHAR azTmpDir[256] = { 0 };
    snprintf(azTmpDir, 256, "%s/%s/", MOUNT_POINT, REC_DIR);
    mkdir(azTmpDir, 0760);
    return;
}

static HI_VOID Sample_FileOptDeInit()
{
    umount(MOUNT_POINT);
    return;
}

HI_S32 startAVCTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_FALSE == astAVCSrc[s32TracksrcHdl].bRead) {
        s32Ret = AVC_Open(&astAVCSrc[s32TracksrcHdl].avcHandle, astAVCSrc[s32TracksrcHdl].aszSrcFileName);
        if (HI_SUCCESS != s32Ret) {
            printf("AVC_Open failed\n");
            return s32Ret;
        }
        astAVCSrc[s32TracksrcHdl].bRead = HI_TRUE;
        astAVCSrc[s32TracksrcHdl].hRecHandle = pCaller;
        s32Ret = pthread_create(&(astAVCSrc[s32TracksrcHdl].readFrameThd), HI_NULL, Sample_ReadAVCThread,
                                (void *)&(astAVCSrc[s32TracksrcHdl]));
    }
    return s32Ret;
}

HI_S32 stopAVCTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_TRUE == astAVCSrc[s32TracksrcHdl].bRead) {
        astAVCSrc[s32TracksrcHdl].bRead = HI_FALSE;
        (void)pthread_join(astAVCSrc[s32TracksrcHdl].readFrameThd, HI_NULL);
        AVC_Close(&(astAVCSrc[s32TracksrcHdl].avcHandle));
    }
    return s32Ret;
}

HI_S32 startHEVCTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_FALSE == astHEVCSrc[s32TracksrcHdl].bRead) {
        s32Ret = HEVC_Open(&astHEVCSrc[s32TracksrcHdl].hevcHandle, astHEVCSrc[s32TracksrcHdl].aszSrcFileName);
        if (HI_SUCCESS != s32Ret) {
            printf("AVC_Open failed\n");
            return s32Ret;
        }
        astHEVCSrc[s32TracksrcHdl].bRead = HI_TRUE;
        astHEVCSrc[s32TracksrcHdl].hRecHandle = pCaller;
        s32Ret = pthread_create(&(astHEVCSrc[s32TracksrcHdl].readFrameThd), HI_NULL, Sample_ReadHEVCThread,
                                (void *)&(astHEVCSrc[s32TracksrcHdl]));
    }
    return s32Ret;
}

HI_S32 stopHEVCTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_TRUE == astHEVCSrc[s32TracksrcHdl].bRead) {
        astHEVCSrc[s32TracksrcHdl].bRead = HI_FALSE;
        (void)pthread_join(astHEVCSrc[s32TracksrcHdl].readFrameThd, HI_NULL);
        HEVC_Close(&(astHEVCSrc[s32TracksrcHdl].hevcHandle));
    }
    return s32Ret;
}

HI_S32 startAACTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_FALSE == astAACSrc[s32TracksrcHdl].bRead) {
        s32Ret = AAC_Open(&astAACSrc[s32TracksrcHdl].aacHandle, astAACSrc[s32TracksrcHdl].aszSrcFileName);
        if (HI_SUCCESS != s32Ret) {
            printf("AAC_Open failed\n");
            return s32Ret;
        }
        astAACSrc[s32TracksrcHdl].bRead = HI_TRUE;
        astAACSrc[s32TracksrcHdl].hRecHandle = pCaller;
        s32Ret = pthread_create(&(astAACSrc[s32TracksrcHdl].readFrameThd), HI_NULL, Sample_ReadAACThread,
                                (void *)&(astAACSrc[s32TracksrcHdl]));
    }

    return s32Ret;
}

HI_S32 stopAACTrackSource(HI_S32 s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    if (HI_TRUE == astAACSrc[s32TracksrcHdl].bRead) {
        astAACSrc[s32TracksrcHdl].bRead = HI_FALSE;
        (void)pthread_join(astAACSrc[s32TracksrcHdl].readFrameThd, HI_NULL);
        AAC_Close(&astAACSrc[s32TracksrcHdl].aacHandle);
    }
    return s32Ret;
}

HI_S32 Sample_Request_File_Names(HI_MW_PTR hRecHdl, HI_U32 u32FileCnt,
                                 HI_CHAR (*paszFilename)[HI_REC_FILE_NAME_LEN])
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
    if (NULL == t) {
        printf("loacal time failed\n");
        return HI_FAILURE;
    }
    snprintf(paszFilename[0], HI_REC_FILE_NAME_LEN, "%s/%s/%4d_%02d%02d_%02d%02d%02d.MP4", MOUNT_POINT, REC_DIR,
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    if (u32FileCnt == 2) {
        snprintf(paszFilename[1], HI_REC_FILE_NAME_LEN, "%s/%s/%4d_%02d%02d_%02d%02d%02d.LRV", MOUNT_POINT, REC_DIR,
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
    HI_U32 u32Cnt = 0;
    for (u32Cnt = 0; u32Cnt < u32FileCnt; u32Cnt++) {
        printf(" %d file  %s\n\n", u32Cnt, paszFilename[u32Cnt]);
    }
    return HI_SUCCESS;
}

HI_S32 Sample_Create_Muxer(HI_CHAR *pszFileName, HI_VOID *pMuxerCfg, HI_MW_PTR *phMuxerHandle)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR hMuxerHdl;
    HI_MP4_CONFIG_S stMp4MuxerCfg;
    memset(&stMp4MuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMp4MuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s", pszFileName);
    stMp4MuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMp4MuxerCfg.stMuxerConfig.u32PreAllocUnit = 0;
    stMp4MuxerCfg.stMuxerConfig.u32BackupUnit = 0;
    stMp4MuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMp4MuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMp4MuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    s32Ret = HI_MP4_Create(&hMuxerHdl, &stMp4MuxerCfg);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    } else {
        *phMuxerHandle = hMuxerHdl;
    }
    return s32Ret;
}

HI_S32 Sample_Destroy_Muxer(HI_MW_PTR hMuxerHandle)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64Duration = 0;
    s32Ret = HI_MP4_DestroyAllTracks(hMuxerHandle, NULL);
    if (HI_SUCCESS != s32Ret) {
        printf("HI_MP4_DestroyAllTracks ret 0x%x\n", s32Ret);
    }

    s32Ret = HI_MP4_Destroy(hMuxerHandle, &u64Duration);
    if (HI_SUCCESS != s32Ret) {
        printf("HI_MP4_Destroy ret 0x%x\n", s32Ret);
    }
    return s32Ret;
}

HI_S32 Sample_Create_Track(HI_MW_PTR hMuxerHandle, HI_Track_Source_Handle pstTrackSrcHandle,
                           HI_MW_PTR *phTrackHandle)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_MP4_TRACK_INFO_S stTrackInfo;
    HI_MW_PTR hTrackHdl;
    memset(&stTrackInfo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));

    if (HI_TRACK_SOURCE_TYPE_VIDEO == pstTrackSrcHandle->enTrackType) {
        stTrackInfo.enTrackType = HI_MP4_STREAM_VIDEO;
        stTrackInfo.fSpeed = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.fSpeed;
        if (HI_TRACK_VIDEO_CODEC_H264 == pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType) {
            stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
        } else if (HI_TRACK_VIDEO_CODEC_H265 == pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.enCodecType) {
            stTrackInfo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
        }
        stTrackInfo.stVideoInfo.u32BitRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32BitRate;
        stTrackInfo.stVideoInfo.u32FrameRate = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;
        stTrackInfo.stVideoInfo.u32Height = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Height;
        stTrackInfo.stVideoInfo.u32Width = pstTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32Width;
        snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");
        stTrackInfo.u32TimeScale = 120000;

    } else if (HI_TRACK_SOURCE_TYPE_AUDIO == pstTrackSrcHandle->enTrackType) {
        stTrackInfo.enTrackType = HI_MP4_STREAM_AUDIO;
        stTrackInfo.fSpeed = 1;
        stTrackInfo.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
        stTrackInfo.stAudioInfo.u32Channels = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32ChnCnt;
        stTrackInfo.stAudioInfo.u16SampleSize = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u16SampleBitWidth;
        stTrackInfo.stAudioInfo.u32SampleRate = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
        snprintf(stTrackInfo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");
        stTrackInfo.stAudioInfo.u32SamplePerFrame = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame;
        stTrackInfo.u32TimeScale = pstTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;
    }

    HI_S32 s32Ret = HI_MP4_CreateTrack(hMuxerHandle, &hTrackHdl, &stTrackInfo);
    if (HI_SUCCESS != s32Ret) {
        printf("HI_MP4_CreateTrack ret 0x%x\n", s32Ret);
    } else {
        *phTrackHandle = hTrackHdl;
    }
    return s32Ret;
}

HI_S32 Sample_Write_Frame(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S *pstFrameData)
{
    // printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MP4_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_MP4_FRAME_DATA_S));
    stFrameData.bKeyFrameFlag = pstFrameData->bKeyFrameFlag;
    stFrameData.pu8DataBuffer = pstFrameData->apu8SliceAddr[0];
    stFrameData.u32DataLength = pstFrameData->au32SliceLen[0];
    stFrameData.u64TimeStamp = pstFrameData->u64TimeStamp;
    s32Ret = HI_MP4_WriteFrame(hMuxerHandle, hTrackHandle, &stFrameData);
    return s32Ret;
}

HI_S32 Sample_Alloc_Buf(HI_VOID *pBufArg, HI_U32 u32BufSize, HI_VOID **ppVmAddr)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    if ((NULL == ppVmAddr) || (0 == u32BufSize)) {
        printf("Sample_Alloc_Buf invalid input param!\n");
        return HI_FAILURE;
    }

    *ppVmAddr = malloc(u32BufSize);
    if (NULL == *ppVmAddr) {
        printf("Sample_Free_Buf malloc mbuf memory failed\n");
        return HI_FAILURE;
    }

    memset(*ppVmAddr, 0, u32BufSize);

    return HI_SUCCESS;
}

HI_S32 Sample_Free_Buf(HI_VOID *pBufArg, HI_U32 u32BufSize, HI_VOID *pVmAddr)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    if (NULL == pVmAddr) {
        printf("Sample_Free_Buf invalid input param!\n");
        return HI_FAILURE;
    }

    free(pVmAddr);
    pVmAddr = NULL;

    return HI_SUCCESS;
}

HI_S32 HI_VTrack_Source_Start(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = startAVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    return s32Ret;
}

HI_S32 HI_VTrack_Source_Stop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = stopAVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    return s32Ret;
}

HI_S32 HI_ATrack_Source_Start(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = startAACTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    return s32Ret;
}
HI_S32 HI_ATrack_Source_Stop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = stopAACTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    return s32Ret;
}

HI_S32 Sample_EventCallback(HI_MW_PTR hRecHdl, const HI_REC_EVENT_INFO_S *pstEventInfo)
{
    printf("%s  %d eventcode %d \n", __FUNCTION__, __LINE__, pstEventInfo->enEventCode);
    return HI_SUCCESS;
}

HI_VOID Sample_InitTrackSource(HI_U32 u32InterValMs)
{
    // main vstream
    astTrackSource[0][0].s32PrivateHandle = 0;
    astTrackSource[0][0].enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    astTrackSource[0][0].pfnSourceStart = HI_VTrack_Source_Start;
    astTrackSource[0][0].pfnSourceStop = HI_VTrack_Source_Stop;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32BitRate = 2 * 1024 * 1024;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32FrameRate = 30;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32Gop = 30;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32Width = 1024;
    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32Height = 576;

    astAVCSrc[0].hTrackSrcHandle = &(astTrackSource[0][0]);
    astAVCSrc[0].s32privateHdl = 0;
    astAVCSrc[0].hRecHandle = 0;
    astAVCSrc[0].bRead = HI_FALSE;
    astAVCSrc[0].u64CoPts = 0;
    snprintf(astAVCSrc[0].aszSrcFileName, SRC_FILE_NAME_LEN, "%s", MAIN_H264_ES_NAME);
    if (0 != u32InterValMs) {
        astAVCSrc[0].u32FrameInterval = u32InterValMs;
        astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.fSpeed = u32InterValMs *
                                                                    astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32FrameRate / 1000;
    } else {
        astAVCSrc[0].u32FrameInterval = (HI_U32)(1000 / astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.u32FrameRate);
        astTrackSource[0][0].unTrackSourceAttr.stVideoInfo.fSpeed = 1.0;
    }

    // main audio
    astTrackSource[0][1].s32PrivateHandle = 0;
    astTrackSource[0][1].enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    astTrackSource[0][1].pfnSourceStart = HI_ATrack_Source_Start;
    astTrackSource[0][1].pfnSourceStop = HI_ATrack_Source_Stop;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;

    astAACSrc[0].hTrackSrcHandle = &(astTrackSource[0][1]);
    astAACSrc[0].s32privateHdl = 0;
    astAACSrc[0].hRecHandle = 0;
    astAACSrc[0].bRead = HI_FALSE;
    astAACSrc[0].u64CoPts = 0;
    astAACSrc[0].u32FrameInterval = astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame * 1000 /
                                    astTrackSource[0][1].unTrackSourceAttr.stAudioInfo.u32SampleRate;
    snprintf(astAACSrc[0].aszSrcFileName, SRC_FILE_NAME_LEN, "%s", MAIN_AAC_ES_NAME);
    // slave video
    astTrackSource[1][0].s32PrivateHandle = 1;
    astTrackSource[1][0].enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    astTrackSource[1][0].pfnSourceStart = HI_VTrack_Source_Start;
    astTrackSource[1][0].pfnSourceStop = HI_VTrack_Source_Stop;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32BitRate = 2 * 1024 * 1024;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32FrameRate = 30;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32Gop = 30;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32Width = 1024;
    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32Height = 576;

    astAVCSrc[1].hTrackSrcHandle = &(astTrackSource[1][0]);
    astAVCSrc[1].s32privateHdl = 1;
    astAVCSrc[1].hRecHandle = 0;
    astAVCSrc[1].bRead = HI_FALSE;
    astAVCSrc[1].u64CoPts = 0;
    snprintf(astAVCSrc[1].aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SLAVE_H264_ES_NAME);
    if (0 != u32InterValMs) {
        astAVCSrc[1].u32FrameInterval = u32InterValMs;
        astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.fSpeed = u32InterValMs *
                                                                    astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32FrameRate / 1000;
    } else {
        astAVCSrc[1].u32FrameInterval = (HI_U32)(1000 / astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.u32FrameRate);
        astTrackSource[1][0].unTrackSourceAttr.stVideoInfo.fSpeed = 1.0;
    }

    // slave audio
    astTrackSource[1][1].s32PrivateHandle = 1;
    astTrackSource[1][1].enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    astTrackSource[1][1].pfnSourceStart = HI_ATrack_Source_Start;
    astTrackSource[1][1].pfnSourceStop = HI_ATrack_Source_Stop;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;

    astAACSrc[1].hTrackSrcHandle = &(astTrackSource[1][1]);
    astAACSrc[1].s32privateHdl = 1;
    astAACSrc[1].hRecHandle = 0;
    astAACSrc[1].bRead = HI_FALSE;
    astAACSrc[1].u64CoPts = 0;
    astAACSrc[1].u32FrameInterval = astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame * 1000 /
                                    astTrackSource[1][1].unTrackSourceAttr.stAudioInfo.u32SampleRate;
    snprintf(astAACSrc[1].aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SLAVE_AAC_ES_NAME);
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    ;
    s_u64BasePts = (HI_U64)tv.tv_sec * 1000000 + tv.tv_nsec / 1000;
}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef __HuaweiLite__
    extern char __init_array_start__, __init_array_end__;
    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
#endif
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32InterValMs = 0;

    HI_REC_ATTR_S stRecAttr;
    printf("%d %s \n", argc, argv[1]);
    Sample_FileOptInit();
    if (argc > 1 && (NULL != strchr(argv[1], 'l'))) {
        printf("laspe recorder\n");
        stRecAttr.enRecType = HI_REC_TYPE_LAPSE;
        stRecAttr.unRecAttr.stLapseRecAttr.u32IntervalMs = 300;
        u32InterValMs = stRecAttr.unRecAttr.stLapseRecAttr.u32IntervalMs;
    } else {
        printf("normal recorder\n");
        stRecAttr.enRecType = HI_REC_TYPE_NORMAL;
        stRecAttr.unRecAttr.stNormalRecAttr.u32Rsv = 0;
        u32InterValMs = 0;
    }

    Sample_InitTrackSource(u32InterValMs);

    stRecAttr.stRecCallbacks.pfnRequestFileNames = Sample_Request_File_Names;
    stRecAttr.stSplitAttr.enSplitType = HI_REC_SPLIT_TYPE_TIME;
    stRecAttr.stSplitAttr.u32SplitTimeLenSec = 30;

    stRecAttr.u32BufferTimeMSec = 3 * 1000;
    stRecAttr.u32PreRecTimeSec = 1;
    stRecAttr.u32StreamCnt = 1;

    HI_U32 u32StreamIdx = 0;
    HI_U32 u32TrackCnt = 0;
    for (u32StreamIdx = 0; u32StreamIdx < stRecAttr.u32StreamCnt; u32StreamIdx++) {
        stRecAttr.astStreamAttr[u32StreamIdx].u32TrackCnt = 2;
        stRecAttr.astStreamAttr[u32StreamIdx].stMuxerOps.pfnCreateMuxer = Sample_Create_Muxer;
        stRecAttr.astStreamAttr[u32StreamIdx].stMuxerOps.pfnCreateTrack = Sample_Create_Track;
        stRecAttr.astStreamAttr[u32StreamIdx].stMuxerOps.pfnDestroyMuxer = Sample_Destroy_Muxer;
        stRecAttr.astStreamAttr[u32StreamIdx].stMuxerOps.pfnWriteFrame = Sample_Write_Frame;
        stRecAttr.astStreamAttr[u32StreamIdx].stMuxerOps.pMuxerCfg = HI_NULL;

        stRecAttr.astStreamAttr[u32StreamIdx].stBufOps.pfnBufAlloc = Sample_Alloc_Buf;
        stRecAttr.astStreamAttr[u32StreamIdx].stBufOps.pfnBufFree = Sample_Free_Buf;
        stRecAttr.astStreamAttr[u32StreamIdx].stBufOps.pBufArg = HI_NULL;
        for (u32TrackCnt = 0; u32TrackCnt < stRecAttr.astStreamAttr[u32StreamIdx].u32TrackCnt; u32TrackCnt++) {
            stRecAttr.astStreamAttr[u32StreamIdx].aHTrackSrcHandle[u32TrackCnt] = &(astTrackSource[u32StreamIdx][u32TrackCnt]);
        }
    }
    HI_MW_PTR hRecHdl = 0;
    s32Ret = HI_REC_Create(&stRecAttr, &hRecHdl);
    if (HI_SUCCESS != s32Ret) {
        printf("HI_REC_Create failed 0x%x\n", s32Ret);
    } else {
        printf("HI_REC_Create success \n");
    }
    s32Ret = HI_REC_RegisterEventCallback(hRecHdl, Sample_EventCallback);
    HI_REC_MANUAL_SPLIT_ATTR_S stManualSplitAttr;
    while (1) {
        int cmd;
        cmd = getchar();

        if ('q' == cmd) {
            break;
        }

        switch (cmd) {
            case 's':
                s32Ret = HI_REC_Start(hRecHdl);
                if (HI_SUCCESS != s32Ret) {
                    printf("HI_REC_Start failed 0x%x\n", s32Ret);
                } else {
                    printf("HI_REC_Start success \n");
                }
                break;

            case 'p':
                s32Ret = HI_REC_Stop(hRecHdl, HI_TRUE);
                if (HI_SUCCESS != s32Ret) {
                    printf("HI_REC_Stop failed 0x%x\n", s32Ret);
                } else {
                    printf("HI_REC_Start success \n");
                }
                break;

            case 'm':
                stManualSplitAttr.enManualType = HI_REC_POST_MANUAL_SPLIT;
                stManualSplitAttr.stPostSplitAttr.u32AfterSec = MANUAL_SPLIT_TIME_S;
                s32Ret = HI_REC_ManualSplit(hRecHdl, &stManualSplitAttr);
                if (HI_SUCCESS != s32Ret) {
                    printf("HI_REC_Split failed 0x%x\n", s32Ret);
                }
                break;
            case 'e':
                stManualSplitAttr.enManualType = HI_REC_PRE_MANUAL_SPLIT;
                stManualSplitAttr.stPreSplitAttr.u32DurationSec = MANUAL_SPLIT_TIME_S;
                s32Ret = HI_REC_ManualSplit(hRecHdl, &stManualSplitAttr);
                if (HI_SUCCESS != s32Ret) {
                    printf("HI_REC_Split pre failed 0x%x\n", s32Ret);
                }
                break;
            default:
                printf("s-----start record\n");
                printf("p-----stop record\n");
                printf("m-----manual post split record\n");
                printf("e-----manual pre split record\n");
                printf("q-----leave record\n");
                break;
        }
    }
    (HI_VOID)HI_REC_Stop(hRecHdl, HI_TRUE);
    (HI_VOID)HI_REC_Destroy(hRecHdl);
    Sample_FileOptDeInit();
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


