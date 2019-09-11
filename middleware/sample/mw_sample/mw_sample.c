
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef __HuaweiLite__
#include <getopt.h>
#endif
#include "dtcf_wrapper.h"
#include "muxer_wrapper.h"
#include "venc_wrapper.h"
#include "rtsp_wrapper.h"
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

static const char* short_options = "hnlb:r:pts:d";
#ifndef __HuaweiLite__
static const struct option long_options[] =
{
    {"help", 0, NULL, 'h'},
    {"norm", 0, NULL, 'n'},
    {"loop", 0, NULL, 'l'},
    {"backup", 1, NULL, 'b'},
    {"repair", 1, NULL, 'r'},
    {"prealloc", 0, NULL, 'p'},
    {"thumbnail", 0, NULL, 't'},
    {"rtsp", 1, NULL, 's'},
    {"output", 1, NULL, 0},
    {"input", 1, NULL, 0},
    {"lapse", 1, NULL, 0},
    {"dual", 1, NULL, 'd'},
    {NULL, 0, NULL, 0},
};
#endif

HI_MW_PTR g_hRecHandle = 0;
HI_U32 g_u32StreamCnt = 2;

HI_S32 RecEventCallback(__attribute__((unused))HI_MW_PTR hRecHandle, const HI_REC_EVENT_INFO_S *pstEventInfo)
{
    switch (pstEventInfo->enEventCode)
    {
        case HI_REC_EVENT_START:
            printf("====recorder start====\n");
            break;
        case HI_REC_EVENT_STOP:
            printf("~~~~recorder end~~~~\n");
            break;
        case HI_REC_EVENT_NEW_FILE_BEGIN:
            printf("file %s begin\n", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;
        case HI_REC_EVENT_NEW_FILE_END:
            printf("file %s end\n", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            AddNorm(pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;
        case HI_REC_EVENT_NEW_MANUAL_SPLIT_FILE_END:
            printf("emr file %s end\n", pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            MoveEmr(pstEventInfo->unEventInfo.stFileInfo.aszFileName);
            break;
        default:
            printf("pass event %d\n", pstEventInfo->enEventCode);
            break;
    }
    return HI_SUCCESS;
}

void Usage(const HI_CHAR* pszCmd)
{
    printf("Norm: without split\n");
    printf("\t%s -n[--normal] -d[--dual] -s[--rtsp] Port -b[--backup] BackInSec --lapse LapseInMs\n", pszCmd);
    printf("Loop: split by time\n");
    printf("\t%s -l[--loop] -s[--rtsp] Port -b[--backup] BackInSec -p[--prealloc] --lapse LapseInMs\n", pszCmd);
    printf("\t\tquit: exit sample\n");
    printf("\t\temr: trigger post emergency recorder\n");
    printf("\t\tpmr: trigger pre emergency recorder\n");
    printf("Repair from backup file\n");
    printf("\t%s -r[--repair] BackupFile\n", pszCmd);
    printf("Extract thumbnail from mp4\n");
    printf("\t%s -t[--thumbnail] --input MP4FilePath --output JPGFilePath\n", pszCmd);
}

static HI_Track_Source_S s_stMainVideoTrackSource;
static HI_Track_Source_S s_stSlaveVideoTrackSource;
static HI_Track_Source_S s_stMainAudioTrackSource;
static HI_Track_Source_S s_stSlaveAudioTrackSource;
static MuxerConfig_S s_stMainMuxerConfig = {-1, NULL};
static MuxerConfig_S s_stSlaveMuxerConfig = {-1, NULL};
static const HI_U32 SPLIT_TIME_IN_SEC = 60;

static HI_S32 Rec_AllocBuf(HI_VOID* pBufArg, HI_U32 u32BufSize, HI_VOID** ppVmAddr)
{
     printf("%s  %d buffersize %d\n", __FUNCTION__, __LINE__,u32BufSize);
    if ((NULL == ppVmAddr) || (0 == u32BufSize))
    {
        printf("Sample_Alloc_Buf invalid input param!\n");
        return HI_FAILURE;
    }

    *ppVmAddr = malloc(u32BufSize);
    if (NULL == *ppVmAddr)
    {
        printf("Sample_Free_Buf malloc mbuf memory failed\n");
        return HI_FAILURE;
    }

    memset(*ppVmAddr, 0, u32BufSize);

    return HI_SUCCESS;
}

static HI_S32 Rec_FreeBuf(HI_VOID* pBufArg,HI_U32 u32BufSize,HI_VOID* pVmAddr)
{
    printf("%s  %d buffersize %d\n", __FUNCTION__, __LINE__,u32BufSize);
    if (NULL == pVmAddr)
    {
        printf("Sample_Free_Buf invalid input param!\n");
        return HI_FAILURE;
    }

    free(pVmAddr);
    pVmAddr = NULL;

    return HI_SUCCESS;
}

HI_BOOL IsMainVideoTrackSource(HI_Track_Source_S* pstTrackSource)
{
    return &s_stMainVideoTrackSource == pstTrackSource?HI_TRUE:HI_FALSE;
}

static const HI_S32 LAPSE_TARGET_FPS = 30;

HI_S32 REC_Start(HI_BOOL bSplit, HI_BOOL bPreAlloc, HI_S32 s32LapseInMs)
{
    printf("REC_Start\n");
    g_u32StreamCnt = 2;
    HI_MW_VEncAttr stMainVEncAttr;
    HI_MW_VEncAttr stSlaveVEncAttr;
    HI_HANDLE hAEncHdl;
    HI_FLOAT fSpeed = 1;
    if (s32LapseInMs > 0)
    {
        fSpeed = s32LapseInMs * 1.0f * LAPSE_TARGET_FPS / 1000;
    }
    GetMainVEncAttr(&stMainVEncAttr);
    GetSlaveVEncAttr(&stSlaveVEncAttr);
    GetAEncAttr(&hAEncHdl);
    s_stMainVideoTrackSource.s32PrivateHandle = stMainVEncAttr.HANDLE;
    if (s32LapseInMs > 0)
    {
        s_stMainVideoTrackSource.pfnSourceStart = LapseVencStart;
    }
    else
    {
        s_stMainVideoTrackSource.pfnSourceStart = VencStart;
    }
    s_stMainVideoTrackSource.pfnSourceStop = VencStop;
    s_stMainVideoTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    if (HI_MW_PAYLOAD_TYPE_H264 == stMainVEncAttr.PAYLOAD_TYPE)
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == stMainVEncAttr.PAYLOAD_TYPE)
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    }
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Width = stMainVEncAttr.WIDTH;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Height = stMainVEncAttr.HEIGHT;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate = stMainVEncAttr.BITRATE * 1024;
    if (s32LapseInMs > 0)
    {

        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = LAPSE_TARGET_FPS;
    }
    else
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = stMainVEncAttr.FRAMERATE;
    }
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Gop = stMainVEncAttr.GOP;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed = fSpeed;

    s_stSlaveVideoTrackSource.s32PrivateHandle = stSlaveVEncAttr.HANDLE;
    if (s32LapseInMs > 0)
    {
        s_stSlaveVideoTrackSource.pfnSourceStart = LapseVencStart;
    }
    else
    {
        s_stSlaveVideoTrackSource.pfnSourceStart = VencStart;
    }
    s_stSlaveVideoTrackSource.pfnSourceStop = VencStop;
    s_stSlaveVideoTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    if (HI_MW_PAYLOAD_TYPE_H264 == stSlaveVEncAttr.PAYLOAD_TYPE)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == stSlaveVEncAttr.PAYLOAD_TYPE)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    }
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Width = stSlaveVEncAttr.WIDTH;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Height = stSlaveVEncAttr.HEIGHT;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate = stSlaveVEncAttr.BITRATE * 1024;
    if (s32LapseInMs > 0)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = LAPSE_TARGET_FPS;
    }
    else
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = stSlaveVEncAttr.FRAMERATE;
    }
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Gop = stSlaveVEncAttr.GOP;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed = fSpeed;

    s_stMainAudioTrackSource.s32PrivateHandle = hAEncHdl;
    s_stMainAudioTrackSource.pfnSourceStart = AencStart;
    s_stMainAudioTrackSource.pfnSourceStop = AencStop;
    s_stMainAudioTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    memcpy(&s_stSlaveAudioTrackSource, &s_stMainAudioTrackSource, sizeof(HI_Track_Source_S));

    HI_REC_ATTR_S stRecAttr;
    if (s32LapseInMs < 0)
    {
        stRecAttr.enRecType = HI_REC_TYPE_NORMAL;
        stRecAttr.unRecAttr.stNormalRecAttr.u32Rsv = 0;
    }
    else
    {
        stRecAttr.enRecType = HI_REC_TYPE_LAPSE;
        stRecAttr.unRecAttr.stLapseRecAttr.u32IntervalMs = s32LapseInMs;
    }
    if (bSplit)
    {
        stRecAttr.stSplitAttr.enSplitType = HI_REC_SPLIT_TYPE_TIME;
    }
    else
    {
        stRecAttr.stSplitAttr.enSplitType = HI_REC_SPLIT_TYPE_NONE;
    }
    stRecAttr.stSplitAttr.u32SplitTimeLenSec = SPLIT_TIME_IN_SEC;

    stRecAttr.u32StreamCnt = g_u32StreamCnt;

    if (s32LapseInMs > 0)
    {
        // laspse no audio
        stRecAttr.astStreamAttr[0].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[1].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[2].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[3].u32TrackCnt = 1;
    }
    else
    {
#ifdef ENABLE_AUDIO
        stRecAttr.astStreamAttr[0].u32TrackCnt = 2;
        stRecAttr.astStreamAttr[1].u32TrackCnt = 2;
        stRecAttr.astStreamAttr[2].u32TrackCnt = 2;
        stRecAttr.astStreamAttr[3].u32TrackCnt = 2;
#else
        stRecAttr.astStreamAttr[0].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[1].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[2].u32TrackCnt = 1;
        stRecAttr.astStreamAttr[3].u32TrackCnt = 1;
#endif
    }
    stRecAttr.astStreamAttr[0].aHTrackSrcHandle[0] = &s_stMainVideoTrackSource;
    stRecAttr.astStreamAttr[0].aHTrackSrcHandle[1] = &s_stMainAudioTrackSource;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnWriteFrame = WriteFrame;
    stRecAttr.astStreamAttr[0].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[0].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[0].stBufOps.pBufArg = HI_NULL;
    if (bPreAlloc)
    {
        s_stMainMuxerConfig.s32FileAllocSize = 20 * 1024 * 1024;
        s_stSlaveMuxerConfig.s32FileAllocSize = 20 * 1024 * 1024;
    }
    else
    {
        s_stMainMuxerConfig.s32FileAllocSize = -1;
        s_stSlaveMuxerConfig.s32FileAllocSize = -1;
    }
    stRecAttr.astStreamAttr[0].stMuxerOps.pMuxerCfg = &s_stMainMuxerConfig;

    stRecAttr.astStreamAttr[1].aHTrackSrcHandle[0] = &s_stSlaveVideoTrackSource;
    stRecAttr.astStreamAttr[1].aHTrackSrcHandle[1] = &s_stSlaveAudioTrackSource;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnWriteFrame = WriteFrame;
    stRecAttr.astStreamAttr[1].stMuxerOps.pMuxerCfg = &s_stSlaveMuxerConfig;
    stRecAttr.astStreamAttr[1].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[1].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[1].stBufOps.pBufArg = HI_NULL;

    stRecAttr.astStreamAttr[2].aHTrackSrcHandle[0] = &s_stMainVideoTrackSource;
    stRecAttr.astStreamAttr[2].aHTrackSrcHandle[1] = &s_stMainAudioTrackSource;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnWriteFrame = WriteFrame;
    stRecAttr.astStreamAttr[2].stMuxerOps.pMuxerCfg = &s_stMainMuxerConfig;
    stRecAttr.astStreamAttr[2].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[2].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[2].stBufOps.pBufArg = HI_NULL;

    stRecAttr.astStreamAttr[3].aHTrackSrcHandle[0] = &s_stSlaveVideoTrackSource;
    stRecAttr.astStreamAttr[3].aHTrackSrcHandle[1] = &s_stSlaveAudioTrackSource;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnWriteFrame = WriteFrame;
    stRecAttr.astStreamAttr[3].stMuxerOps.pMuxerCfg= &s_stSlaveMuxerConfig;
    stRecAttr.astStreamAttr[3].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[3].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[3].stBufOps.pBufArg = HI_NULL;



    stRecAttr.stRecCallbacks.pfnRequestFileNames = RequestFileNames;
    stRecAttr.u32BufferTimeMSec = 5*1000;
    stRecAttr.u32PreRecTimeSec = 2;
    (HI_VOID)HI_REC_Create(&stRecAttr, &g_hRecHandle);
    (HI_VOID)HI_REC_RegisterEventCallback(g_hRecHandle, RecEventCallback);
    return HI_REC_Start(g_hRecHandle);
}


HI_S32 REC_Start_Dual(HI_BOOL bSplit, HI_BOOL bPreAlloc, HI_S32 s32LapseInMs)
{
    printf("REC_Start_Dual\n");
    g_u32StreamCnt = 4;
    HI_MW_VEncAttr stMainVEncAttr;
    HI_MW_VEncAttr stSlaveVEncAttr;
    HI_HANDLE hAEncHdl;
    HI_FLOAT fSpeed = 1;
    if (s32LapseInMs > 0)
    {
        fSpeed = s32LapseInMs * 1.0f * LAPSE_TARGET_FPS / 1000;
    }
    GetMainVEncAttr(&stMainVEncAttr);
    GetSlaveVEncAttr(&stSlaveVEncAttr);
    GetAEncAttr(&hAEncHdl);
    s_stMainVideoTrackSource.s32PrivateHandle = stMainVEncAttr.HANDLE;
    if (s32LapseInMs > 0)
    {
        s_stMainVideoTrackSource.pfnSourceStart = LapseVencStart;
    }
    else
    {
        s_stMainVideoTrackSource.pfnSourceStart = VencStart;
    }
    s_stMainVideoTrackSource.pfnSourceStop = VencStop;
    s_stMainVideoTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    if (HI_MW_PAYLOAD_TYPE_H264 == stMainVEncAttr.PAYLOAD_TYPE)
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == stMainVEncAttr.PAYLOAD_TYPE)
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    }
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Width = stMainVEncAttr.WIDTH;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Height = stMainVEncAttr.HEIGHT;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate = stMainVEncAttr.BITRATE * 1024;
    if (s32LapseInMs > 0)
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = LAPSE_TARGET_FPS;
    }
    else
    {
        s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = stMainVEncAttr.FRAMERATE;
    }
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Gop = stMainVEncAttr.GOP;
    s_stMainVideoTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed = fSpeed;

    s_stSlaveVideoTrackSource.s32PrivateHandle = stSlaveVEncAttr.HANDLE;
    if (s32LapseInMs > 0)
    {
        s_stSlaveVideoTrackSource.pfnSourceStart = LapseVencStart;
    }
    else
    {
        s_stSlaveVideoTrackSource.pfnSourceStart = VencStart;
    }
    s_stSlaveVideoTrackSource.pfnSourceStop = VencStop;
    s_stSlaveVideoTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    if (HI_MW_PAYLOAD_TYPE_H264 == stSlaveVEncAttr.PAYLOAD_TYPE)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == stSlaveVEncAttr.PAYLOAD_TYPE)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    }
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Width = stSlaveVEncAttr.WIDTH;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Height = stSlaveVEncAttr.HEIGHT;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32BitRate = stSlaveVEncAttr.BITRATE * 1024;
    if (s32LapseInMs > 0)
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = LAPSE_TARGET_FPS;
    }
    else
    {
        s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32FrameRate = stSlaveVEncAttr.FRAMERATE;
    }
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.u32Gop = stSlaveVEncAttr.GOP;
    s_stSlaveVideoTrackSource.unTrackSourceAttr.stVideoInfo.fSpeed = fSpeed;

    s_stMainAudioTrackSource.s32PrivateHandle = hAEncHdl;
    s_stMainAudioTrackSource.pfnSourceStart = AencStart;
    s_stMainAudioTrackSource.pfnSourceStop = AencStop;
    s_stMainAudioTrackSource.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;
    s_stMainAudioTrackSource.unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    memcpy(&s_stSlaveAudioTrackSource, &s_stMainAudioTrackSource, sizeof(HI_Track_Source_S));

    HI_REC_ATTR_S stRecAttr;
    if (s32LapseInMs < 0)
    {
        stRecAttr.enRecType = HI_REC_TYPE_NORMAL;
        stRecAttr.unRecAttr.stNormalRecAttr.u32Rsv = 0;
    }
    else
    {
        stRecAttr.enRecType = HI_REC_TYPE_LAPSE;
        stRecAttr.unRecAttr.stLapseRecAttr.u32IntervalMs = s32LapseInMs;
    }
    if (bSplit)
    {
        stRecAttr.stSplitAttr.enSplitType = HI_REC_SPLIT_TYPE_TIME;
    }
    else
    {
        stRecAttr.stSplitAttr.enSplitType = HI_REC_SPLIT_TYPE_NONE;
    }
    stRecAttr.stSplitAttr.u32SplitTimeLenSec = SPLIT_TIME_IN_SEC;

    stRecAttr.u32StreamCnt = g_u32StreamCnt;

    if (s32LapseInMs > 0)
    {
        // laspse no audio
        stRecAttr.astStreamAttr[0].u32TrackCnt = 1;
    }
    else
    {
        stRecAttr.astStreamAttr[0].u32TrackCnt = 2;
    }
    stRecAttr.astStreamAttr[0].aHTrackSrcHandle[0] = &s_stMainVideoTrackSource;
    stRecAttr.astStreamAttr[0].aHTrackSrcHandle[1] = &s_stMainAudioTrackSource;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[0].stMuxerOps.pfnWriteFrame = WriteFrameToFirst;
    stRecAttr.astStreamAttr[0].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[0].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[0].stBufOps.pBufArg = HI_NULL;
    if (bPreAlloc)
    {
        s_stMainMuxerConfig.s32FileAllocSize = 20 * 1024 * 1024;
        s_stSlaveMuxerConfig.s32FileAllocSize = 20 * 1024 * 1024;
    }
    else
    {
        s_stMainMuxerConfig.s32FileAllocSize = -1;
        s_stSlaveMuxerConfig.s32FileAllocSize = -1;
    }
    s_stMainMuxerConfig.hMuxerHandle = HI_NULL;
    s_stSlaveMuxerConfig.hMuxerHandle = HI_NULL;
    stRecAttr.astStreamAttr[0].stMuxerOps.pMuxerCfg = &s_stMainMuxerConfig;

    if (s32LapseInMs > 0)
    {
        // laspse no audio
        stRecAttr.astStreamAttr[1].u32TrackCnt = 1;
    }
    else
    {
        stRecAttr.astStreamAttr[1].u32TrackCnt = 2;
    }
    stRecAttr.astStreamAttr[1].aHTrackSrcHandle[0] = &s_stMainVideoTrackSource;
    stRecAttr.astStreamAttr[1].aHTrackSrcHandle[1] = &s_stSlaveAudioTrackSource;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnCreateMuxer = CreateMuxer;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnDestroyMuxer = DestroyMuxer;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnCreateTrack = CreateTrack;
    stRecAttr.astStreamAttr[1].stMuxerOps.pfnWriteFrame = WriteFrameToSecond;
    stRecAttr.astStreamAttr[1].stMuxerOps.pMuxerCfg = &s_stSlaveMuxerConfig;
    stRecAttr.astStreamAttr[1].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[1].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[1].stBufOps.pBufArg = HI_NULL;


    stRecAttr.astStreamAttr[2].u32TrackCnt = 1;
    stRecAttr.astStreamAttr[2].aHTrackSrcHandle[0] = &s_stSlaveVideoTrackSource;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnCreateMuxer = CreateSlaveMuxer;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnDestroyMuxer = DestroySlaveMuxer;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnCreateTrack = CreateSlaveTrack;
    stRecAttr.astStreamAttr[2].stMuxerOps.pfnWriteFrame = WriteFrameToFirst;
    stRecAttr.astStreamAttr[2].stMuxerOps.pMuxerCfg = &s_stMainMuxerConfig;
    stRecAttr.astStreamAttr[2].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[2].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[2].stBufOps.pBufArg = HI_NULL;


    stRecAttr.astStreamAttr[3].u32TrackCnt = 1;
    stRecAttr.astStreamAttr[3].aHTrackSrcHandle[0] = &s_stSlaveVideoTrackSource;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnCreateMuxer = CreateSlaveMuxer;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnDestroyMuxer = DestroySlaveMuxer;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnCreateTrack = CreateSlaveTrack;
    stRecAttr.astStreamAttr[3].stMuxerOps.pfnWriteFrame = WriteFrameToSecond;
    stRecAttr.astStreamAttr[3].stMuxerOps.pMuxerCfg = &s_stSlaveMuxerConfig;
    stRecAttr.astStreamAttr[3].stBufOps.pfnBufAlloc = Rec_AllocBuf;
    stRecAttr.astStreamAttr[3].stBufOps.pfnBufFree = Rec_FreeBuf;
    stRecAttr.astStreamAttr[3].stBufOps.pBufArg = HI_NULL;

    stRecAttr.stRecCallbacks.pfnRequestFileNames = RequestDualFileNames;
    stRecAttr.u32BufferTimeMSec = 5*1000;
    stRecAttr.u32PreRecTimeSec = 2;
    (HI_VOID)HI_REC_Create(&stRecAttr, &g_hRecHandle);
    (HI_VOID)HI_REC_RegisterEventCallback(g_hRecHandle, RecEventCallback);
    return HI_REC_Start(g_hRecHandle);
}

HI_S32 REC_Stop()
{
    (HI_VOID)HI_REC_Stop(g_hRecHandle,HI_TRUE);
    return HI_REC_Destroy(g_hRecHandle);
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __HuaweiLite__
    /*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
    /*init cpp running context on liteos end*/
#endif
    int opt = 0;
    HI_BOOL bSplit = HI_TRUE;
    HI_BOOL bPreAlloc = HI_FALSE;
    HI_S32 s32RepairFreq = -1;
    HI_S32 s32OptIndex = 0;
    HI_BOOL bExtractThm = HI_FALSE;
    HI_S32 s32LapseInMs = -1;
    HI_S32 s32Port = -1;
    HI_BOOL bDualRecord = HI_FALSE;
    const HI_CHAR* pszInputFile = NULL;
    const HI_CHAR* pszOutputFile = NULL;
#ifndef __HuaweiLite__
    while (-1 != (opt = getopt_long(argc, argv, short_options, long_options, &s32OptIndex)))
    {
        switch (opt)
        {
            case 'h':
                Usage(argv[0]);
                return -1;
            case 'n':
                bSplit = HI_FALSE;
                break;
            case 'l':
                bSplit = HI_TRUE;
                break;
            case 'b':
                s32RepairFreq = atoi(optarg);
                break;
            case 'r':
                return Repair(optarg);
            case 'p':
                bPreAlloc = HI_TRUE;
                break;
            case 't':
                bExtractThm = HI_TRUE;
                break;
            case 's':
                s32Port = atoi(optarg);
                break;
            case 'd':
                bDualRecord = HI_TRUE;
                break;
            case 0:
                if (0 == strncmp(long_options[s32OptIndex].name, "output", strlen("output")))
                {
                    pszOutputFile = optarg;
                }
                else if (0 == strncmp(long_options[s32OptIndex].name, "input", strlen("input")))
                {
                    pszInputFile = optarg;
                }
                else if (0 == strncmp(long_options[s32OptIndex].name, "lapse", strlen("lapse")))
                {
                    s32LapseInMs = atoi(optarg);
                }
                break;
        }
    }
    if (bExtractThm)
    {
        if (NULL != pszInputFile && NULL != pszOutputFile)
        {
            return ExtractThm(pszInputFile, pszOutputFile);
        }
        else
        {
            printf("must give mp4 and jpg file path\n");
        }
        return HI_FAILURE;
    }
#else
    bSplit = HI_TRUE;
    bPreAlloc = HI_TRUE;
    bDualRecord = HI_FALSE;
    s32LapseInMs = -1;
    s32Port = 554;
#endif
    if (HI_FALSE == bSplit && HI_TRUE == bPreAlloc)
    {
        printf("Can't PreAlloc in normal mode\n");
        return -1;
    }

    if (HI_SUCCESS != FileOptInit())
    {
        printf("FileOptInit fail\n");
        return -1;
    }
    if (HI_SUCCESS != MediaInit())
    {
        printf("MediaInit fail\n");
        FileOptDeInit();
        return -1;
    }

    if (s32RepairFreq > 0)
    {
        RepairInit(s32RepairFreq);
    }
    if (bDualRecord)
    {
        REC_Start_Dual(bSplit, bPreAlloc, s32LapseInMs);
    }
    else
    {
        REC_Start(bSplit, bPreAlloc, s32LapseInMs);
    }
    if (s32Port > 0)
    {
        RTSPSERVER_Start(s32Port);
    }
#ifdef __HuaweiLite__
    while(1)
    {
        sleep(1000);
        continue;
    }
#endif
    char cmd[516] = {0};
    printf("Input CMD: ");
    while (NULL != fgets(cmd, 516, stdin))
    {
        if (0 == strncmp(cmd, "quit", 4))
        {
            printf("will quit\n");
            break;
        }
        else if (0 == strncmp(cmd, "emr", 3))
        {
            printf("triggle emr\n");
            HI_REC_MANUAL_SPLIT_ATTR_S stManualSplitAttr;
            stManualSplitAttr.enManualType = HI_REC_POST_MANUAL_SPLIT;
            stManualSplitAttr.stPostSplitAttr.u32AfterSec  = 10;
            (HI_VOID)HI_REC_ManualSplit(g_hRecHandle, &stManualSplitAttr);
        }
        else if (0 == strncmp(cmd, "pmr", 3))
        {
            printf("triggle emr pre recorder\n");
            HI_REC_MANUAL_SPLIT_ATTR_S stManualSplitAttr;
            stManualSplitAttr.enManualType = HI_REC_PRE_MANUAL_SPLIT;
            stManualSplitAttr.stPreSplitAttr.u32DurationSec = 20;
            (HI_VOID)HI_REC_ManualSplit(g_hRecHandle, &stManualSplitAttr);
        }
        printf("Input CMD: ");
    }


    if (s32Port > 0)
    {
        RTSPSERVER_Stop();
    }
    REC_Stop();

    if (s32RepairFreq > 0)
    {
        RepairDeInit();
    }
    MediaDeInit();
    FileOptDeInit();
    return 0;
}
