#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <signal.h>
#include "hi_mw_type.h"
#include "hi_rtsp_server.h"
#include "hi_track_source.h"
#include "hi_server_state_listener.h"
#include "avc.h"
#include "aac.h"
#include "hevc.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define SRC_FILE_NAME_LEN   (128)

#ifndef __HIX86__
#define MOUNT_POINT  "/tmp"
#define SAMPLE_DIR MOUNT_POINT
#else
#define MOUNT_POINT  "."
#define SAMPLE_DIR MOUNT_POINT
#endif

#ifdef __HuaweiLite__
#define SAMPLE_SRC_H264    SAMPLE_DIR"/1.h264"
#define SAMPLE_SRC_AAC     SAMPLE_DIR"/1.aac"
#define SAMPLE_SRC_H265    SAMPLE_DIR"/2.h265"
#define SAMPLE_SRC_AAC_2   SAMPLE_DIR"/2.aac"
#else
#define SAMPLE_SRC_H264    "1.h264"
#define SAMPLE_SRC_AAC     "1.aac"
#define SAMPLE_SRC_H265    "2.h265"
#define SAMPLE_SRC_AAC_2   "2.aac"
#endif

#ifdef __HuaweiLite__
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p0";
#else
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p1";
#endif


static HI_S32 SAMPLE_mount_tmp_dir()
{
#ifndef __HIX86__
    HI_S32 s32Ret = HI_SUCCESS;
    (HI_VOID)mkdir(MOUNT_POINT, 0770);
#ifdef __HuaweiLite__
    s32Ret = mount("/dev/mmcblk0p0", MOUNT_POINT, "vfat", 0, 0);
#else
    s32Ret = mount("/dev/mmcblk0p1", MOUNT_POINT, "vfat", MS_NOEXEC, 0);
#endif
    if (HI_SUCCESS != s32Ret)
    {
        printf("mount tmp failed, errno:%d \n", errno);
    }
#endif
    return s32Ret;
}

static HI_VOID SAMPLE_umount_tmp_dir()
{
 #ifndef __HIX86__
    HI_S32 s32Ret = umount(MOUNT_POINT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("umount %s failed, errno:%d \n",MOUNT_POINT, errno);
    }
 #endif
    return;
}

typedef struct hiSampleAVCSource_S
{
    pthread_t    readFrameThd;
    AVC_Handle_S avcHandle;
    HI_S32       s32privateHdl;
    HI_BOOL      bRead;
    HI_MW_PTR    pRtspHandle;
    HI_MW_PTR    pSessionHandle;
    HI_Track_Source_Handle  hTrackSrcHandle;
    HI_CHAR      aszSrcFileName[SRC_FILE_NAME_LEN];
} SAMPLE_AVC_SRC_S;


typedef struct hiSampleAACSource_S
{
    pthread_t    readFrameThd;
    AAC_Handle_S aacHandle;
    HI_S32       s32privateHdl;
    HI_BOOL      bRead;
    HI_MW_PTR    pRtspHandle;

    HI_MW_PTR    pSessionHandle;
    HI_Track_Source_Handle  hTrackSrcHandle;
    HI_CHAR      aszSrcFileName[SRC_FILE_NAME_LEN];
} SAMPLE_AAC_SRC_S;

typedef struct hiSampleHEVCSource_S
{
    pthread_t     readFrameThd;
    HEVC_Handle_S hevcHandle;
    HI_S32        s32privateHdl;
    HI_BOOL       bRead;
    HI_MW_PTR     pRtspHandle;

    HI_MW_PTR    pSessionHandle;
    HI_Track_Source_Handle  hTrackSrcHandle;
    HI_CHAR      aszSrcFileName[SRC_FILE_NAME_LEN];
} SAMPLE_HEVC_SRC_S;

HI_RTSP_CONFIG_S g_sRTSPConf = {0};
static HI_Track_Source_S stHevcVideo = {0};
static HI_Track_Source_S stHevcAudio = {0};
static HI_Track_Source_S stAvcVideo = {0};
static HI_Track_Source_S stAvcAudio = {0};

static HI_MW_PTR g_pServer = HI_NULL;

static SAMPLE_AVC_SRC_S  stAVCSrc;
static SAMPLE_AAC_SRC_S  stAACSrc1;

static SAMPLE_HEVC_SRC_S stHEVCSrc;
static SAMPLE_AAC_SRC_S  stAACSrc2;
static HI_U64 s_u64BasePts;


static HI_VOID print_help_info(HI_VOID)
{
    printf("List all testtool command\n");
    printf("h             list all command we provide\n");
    printf("q             quit sample test\n");
    printf("1             start one 264 stream and the rtspserver \n");
    printf("2             start one 265 stream and the rtspserver \n");

}

HI_VOID onServerError(HI_SERVER_STATE_LISTENER_S* handle, HI_S32 errId, HI_CHAR* msg)
{
    printf("server error handle:%p, id:%d, msd:%s\n", handle, errId, msg);
    return;
}

HI_VOID onClientConnect(HI_SERVER_STATE_LISTENER_S* handle, HI_CHAR* ip)
{
    printf("onClientConnect handle:%p, ip:%s\n", handle, ip);
    return;

}

HI_VOID onClientDisconnect(HI_SERVER_STATE_LISTENER_S* handle, HI_CHAR* ip)
{
    printf("onClientDisconnect handle:%p, ip:%s \n", handle, ip);
    return;

}

/*add the statelistener of rtspserver*/

static HI_SERVER_STATE_LISTENER_S  statelistener =
{
    .onClientConnect = onClientConnect,
    .onClientDisconnect = onClientDisconnect,
    .onServerError = onServerError,
};

static HI_VOID* Sample_ReadAVCThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAVCThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    HI_U64 u64frameCnt = 0;

    HI_RTSP_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_RTSP_DATA_S));
    struct timeval tv;
    SAMPLE_AVC_SRC_S* pAvcCtx = (SAMPLE_AVC_SRC_S*)pArg;

    HI_U32 u32FrameInterVal = 1000 / pAvcCtx->hTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;
    while (HI_FALSE != pAvcCtx->bRead)
    {
        s32Ret = AVC_ReadFrame(&pAvcCtx->avcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AVC_Reset begin\n");
            AVC_Reset(&pAvcCtx->avcHandle);
        }
        gettimeofday(&tv, NULL);

        stFrameData.u32Seq = 0;
        stFrameData.apu8DataPtr[0] = u8Frame;
        stFrameData.au32DataLen[0] = u32FrameLen;
        stFrameData.u32BlockCnt = 1;
        stFrameData.bIsKeyFrame = (HI_BOOL)u8KeyFrame;
        stFrameData.u64Pts  = u64frameCnt * u32FrameInterVal * 1000 + s_u64BasePts;
        HI_RTSPSVR_WriteFrame(g_pServer, pAvcCtx->hTrackSrcHandle, &stFrameData);
        u64frameCnt++;
        usleep(u32FrameInterVal * 1000);
    }
    return NULL;
}


static HI_VOID* Sample_ReadHEVCThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadHEVCThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    HI_U64 u64frameCnt = 0;


    HI_RTSP_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_RTSP_DATA_S));
    struct timeval tv;
    SAMPLE_HEVC_SRC_S* pHevcCtx = (SAMPLE_HEVC_SRC_S*)pArg;
    HI_U32 u32FrameInterVal = 1000 / pHevcCtx->hTrackSrcHandle->unTrackSourceAttr.stVideoInfo.u32FrameRate;


    while (HI_FALSE != pHevcCtx->bRead)
    {
        gettimeofday(&tv, NULL);
        s32Ret = HEVC_ReadFrame(&pHevcCtx->hevcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HEVC_Reset begin\n");
            HEVC_Reset(&pHevcCtx->hevcHandle);
        }
        stFrameData.u32Seq = 0;
        stFrameData.apu8DataPtr[0] = u8Frame;
        stFrameData.au32DataLen[0] = u32FrameLen;
        stFrameData.u32BlockCnt = 1;
        stFrameData.bIsKeyFrame = (HI_BOOL)u8KeyFrame;
        stFrameData.u64Pts  = u64frameCnt * u32FrameInterVal * 1000 + s_u64BasePts;
        HI_RTSPSVR_WriteFrame(g_pServer,  pHevcCtx->hTrackSrcHandle, &stFrameData);
        u64frameCnt++;
        usleep(u32FrameInterVal * 1000);
    }
    return NULL;
}

static HI_VOID* Sample_ReadAACThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAACThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U64 u64frameCnt = 0;
    HI_RTSP_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_RTSP_DATA_S));
    struct timeval tv;
    SAMPLE_AAC_SRC_S* pAACCtx = (SAMPLE_AAC_SRC_S*)pArg;

    HI_U32 interval = pAACCtx->hTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame * 1000 / pAACCtx->hTrackSrcHandle->unTrackSourceAttr.stAudioInfo.u32SampleRate;

    while (HI_FALSE != pAACCtx->bRead)
    {
        gettimeofday(&tv, NULL);
        s32Ret = AAC_ReadFrame(&pAACCtx->aacHandle, &u8Frame, &u32FrameLen);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AAC_Reset begin\n");
            AAC_Reset(&pAACCtx->aacHandle);
        }

        stFrameData.u32Seq = 0;
        stFrameData.apu8DataPtr[0] = u8Frame;
        stFrameData.au32DataLen[0] = u32FrameLen;
        stFrameData.u32BlockCnt = 1;
        stFrameData.bIsKeyFrame = HI_TRUE;
        stFrameData.u64Pts  = u64frameCnt * interval * 1000 + s_u64BasePts;
        HI_RTSPSVR_WriteFrame(g_pServer,  pAACCtx->hTrackSrcHandle, &stFrameData);
        u64frameCnt++;
        usleep(interval * 1000);
    }
    return NULL;
}


HI_S32 startAVCTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_AVC_SRC_S* pstAvcSrc = &stAVCSrc;
    stAVCSrc.pSessionHandle = pCaller;
    if (HI_FALSE == pstAvcSrc->bRead)
    {
        s32Ret = AVC_Open(&pstAvcSrc->avcHandle, pstAvcSrc->aszSrcFileName);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AVC_Open failed\n");
            return s32Ret;
        }
        pstAvcSrc->bRead = HI_TRUE;
        s32Ret = pthread_create(&(pstAvcSrc->readFrameThd), HI_NULL, Sample_ReadAVCThread, (void*)pstAvcSrc);
    }

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    return s32Ret;
}

HI_S32 stopAVCTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_AVC_SRC_S* pstAvcSrc = &stAVCSrc;
    if (HI_TRUE == pstAvcSrc->bRead)
    {
        pstAvcSrc->bRead = HI_FALSE;
        (void)pthread_join(pstAvcSrc->readFrameThd, HI_NULL);
        AVC_Close(&(pstAvcSrc->avcHandle));
    }

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    return s32Ret;
}


HI_S32 startHEVCTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;

    SAMPLE_HEVC_SRC_S* pstHevcSrc = &stHEVCSrc;

    stHEVCSrc.pSessionHandle = pCaller;
    if (HI_FALSE == pstHevcSrc->bRead)
    {
        s32Ret = HEVC_Open(&pstHevcSrc->hevcHandle, pstHevcSrc->aszSrcFileName);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AVC_Open failed\n");
            return s32Ret;
        }
        pstHevcSrc->bRead = HI_TRUE;
        s32Ret = pthread_create(&(pstHevcSrc->readFrameThd), HI_NULL, Sample_ReadHEVCThread, (void*)pstHevcSrc);
    }

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    return s32Ret;
}

HI_S32 stopHEVCTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;

    SAMPLE_HEVC_SRC_S* pstHevcSrc = &stHEVCSrc;
    if (HI_TRUE == pstHevcSrc->bRead)
    {
        pstHevcSrc->bRead = HI_FALSE;
        (void)pthread_join(pstHevcSrc->readFrameThd, HI_NULL);
        HEVC_Close(&(pstHevcSrc->hevcHandle));
    }

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    return s32Ret;
}


HI_S32 startAACTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_AAC_SRC_S* pstAacSrc = NULL;
    if (0 == s32TracksrcHdl)
    {
        stAACSrc1.pSessionHandle = pCaller;

        pstAacSrc = &stAACSrc1;
    }
    else if (1 == s32TracksrcHdl)
    {
        stAACSrc1.pSessionHandle = pCaller;

        pstAacSrc = &stAACSrc2;
    }
    if (HI_FALSE == pstAacSrc->bRead)
    {
        s32Ret = AAC_Open(&pstAacSrc->aacHandle, pstAacSrc->aszSrcFileName);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AAC_Open failed\n");
            return s32Ret;
        }
        pstAacSrc->bRead = HI_TRUE;
        s32Ret = pthread_create(&(pstAacSrc->readFrameThd), HI_NULL, Sample_ReadAACThread, (void*)pstAacSrc);
    }
    printf("%s  %d \n", __FUNCTION__, __LINE__);

    return s32Ret;
}


HI_S32 stopAACTrackSource(HI_S32  s32TracksrcHdl, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_AAC_SRC_S* pstAacSrc = NULL;
    if (0 == s32TracksrcHdl)
    {
        pstAacSrc = &stAACSrc1;
    }
    else if (1 == s32TracksrcHdl)
    {
        pstAacSrc = &stAACSrc2;
    }

    if (HI_TRUE == pstAacSrc->bRead)
    {

        printf("%s  %d \n", __FUNCTION__, __LINE__);
        pstAacSrc->bRead = HI_FALSE;
        (void)pthread_join(pstAacSrc->readFrameThd, HI_NULL);
        printf("%s  %d \n", __FUNCTION__, __LINE__);
        AAC_Close(&pstAacSrc->aacHandle);
    }

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    return s32Ret;
}
HI_S32 HI_VTrack_Source_RequestKeyFrame(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);

    return HI_SUCCESS;
}
HI_S32 HI_VTrack_Source_Start(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    HI_S32 s32Ret = HI_SUCCESS;

    printf("%s  %d \n", __FUNCTION__, __LINE__);
    if (HI_TRACK_VIDEO_CODEC_H264 == pTrackSource->unTrackSourceAttr.stVideoInfo.enCodecType )
    {
        s32Ret = startAVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    }
    else if (HI_TRACK_VIDEO_CODEC_H265 == pTrackSource->unTrackSourceAttr.stVideoInfo.enCodecType )
    {
        s32Ret = startHEVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    }
    return s32Ret;
}

HI_S32 HI_VTrack_Source_Stop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("%s  %d \n", __FUNCTION__, __LINE__);
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_TRACK_VIDEO_CODEC_H264 == pTrackSource->unTrackSourceAttr.stVideoInfo.enCodecType )
    {
        s32Ret = stopAVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    }
    else if (HI_TRACK_VIDEO_CODEC_H265 == pTrackSource->unTrackSourceAttr.stVideoInfo.enCodecType )
    {
        s32Ret = stopHEVCTrackSource(pTrackSource->s32PrivateHandle, pCaller);
    }


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

HI_VOID Sample_InitTrackSource()
{
    //H264 vstream
    stAvcVideo.s32PrivateHandle = 0;
    stAvcVideo.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    stAvcVideo.pfnSourceStart = HI_VTrack_Source_Start;
    stAvcVideo.pfnSourceStop = HI_VTrack_Source_Stop;
    stAvcVideo.pfnRequestKeyFrame = HI_VTrack_Source_RequestKeyFrame;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.u32BitRate = 2 * 1024 * 1024;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.u32FrameRate = 30;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.u32Gop = 30;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.u32Width = 1024;
    stAvcVideo.unTrackSourceAttr.stVideoInfo.u32Height = 576;

    stAVCSrc.hTrackSrcHandle = &stAvcVideo;
    stAVCSrc.s32privateHdl = 0;
    stAVCSrc.pRtspHandle = g_pServer;
    stAVCSrc.bRead = HI_FALSE;
    snprintf(stAVCSrc.aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SAMPLE_SRC_H264);

    stAvcAudio.s32PrivateHandle = 0;
    stAvcAudio.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    stAvcAudio.pfnSourceStart = HI_ATrack_Source_Start;
    stAvcAudio.pfnSourceStop = HI_ATrack_Source_Stop;
    stAvcAudio.pfnRequestKeyFrame = NULL;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    stAvcAudio.unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;

    stAACSrc1.hTrackSrcHandle = &stAvcAudio;
    stAACSrc1.s32privateHdl = 0;
    stAACSrc1.pRtspHandle = g_pServer;
    stAACSrc1.bRead = HI_FALSE;
    snprintf(stAACSrc1.aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SAMPLE_SRC_AAC);

    //H265 stream
    stHevcVideo.s32PrivateHandle = 1;
    stHevcVideo.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    stHevcVideo.pfnSourceStart = HI_VTrack_Source_Start;
    stHevcVideo.pfnSourceStop = HI_VTrack_Source_Stop;
    stHevcVideo.pfnRequestKeyFrame = HI_VTrack_Source_RequestKeyFrame;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.u32BitRate = 2 * 1024 * 1024;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.u32FrameRate = 30;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.u32Gop = 30;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.u32Width = 1024;
    stHevcVideo.unTrackSourceAttr.stVideoInfo.u32Height = 576;

    stHEVCSrc.hTrackSrcHandle = &stHevcVideo;
    stHEVCSrc.s32privateHdl = 1;
    stHEVCSrc.pRtspHandle = g_pServer;
    stHEVCSrc.bRead = HI_FALSE;
    snprintf(stHEVCSrc.aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SAMPLE_SRC_H265);


    stHevcAudio.s32PrivateHandle = 1;
    stHevcAudio.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    stHevcAudio.pfnSourceStart = HI_ATrack_Source_Start;
    stHevcAudio.pfnSourceStop = HI_ATrack_Source_Stop;
    stHevcAudio.pfnRequestKeyFrame = NULL;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    stHevcAudio.unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;

    stAACSrc2.hTrackSrcHandle = &stHevcAudio;
    stAACSrc2.s32privateHdl = 1;
    stAACSrc2.pRtspHandle = g_pServer;
    stAACSrc2.bRead = HI_FALSE;
    snprintf(stAACSrc2.aszSrcFileName, SRC_FILE_NAME_LEN, "%s", SAMPLE_SRC_AAC_2);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    s_u64BasePts = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
}

HI_S32 SAMPLE_RTSPSVR_265()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RTSP_SOURCE_S stStreamSrc11;//for 264
    stStreamSrc11.pstVideoSrc = &stHevcVideo;
    stStreamSrc11.pstAudioSrc = &stHevcAudio;

    /*init the g_sRTSPConf and create rtspserver context*/
    g_sRTSPConf.s32ListenPort = 554;
    g_sRTSPConf.s32MaxConnNum = 5;
    g_sRTSPConf.s32MaxPayload = 3;
    g_sRTSPConf.s32PacketLen = 1500;
    g_sRTSPConf.s32Timeout = -1;

    s32Ret = HI_RTSPSVR_Create(&g_pServer, &g_sRTSPConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Create s32Ret:%d\n", s32Ret);
        return HI_SUCCESS;
    }

    statelistener.handle = HI_NULL;
    s32Ret = HI_RTSPSVR_SetListener(g_pServer, &statelistener);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Create s32Ret:%d\n", s32Ret);
        goto SERVER_DESTROY;
    }

    /*server start protocal parse */
    s32Ret = HI_RTSPSVR_Start(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Start failed s32Ret:%d\n", s32Ret);
        goto SERVER_DESTROY;
    }

    /*add mediastream 1 for session */
    s32Ret = HI_RTSPSVR_AddMediaStream(g_pServer, &stStreamSrc11, "11", 1024 * 1024 * 5);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_VSTREAM_Create fail s32Ret:%d\n", s32Ret);
        goto SERVER_STOP;
    }


    printf("HI_RTSPSVR_Start successs!\n");

    printf("please input any key to stop the rtsp server!\n");

    if (getchar())
    {
        printf("start to stop the server !\n");
    }


    s32Ret = HI_RTSPSVR_RemoveMediaStream(g_pServer, "11");
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_RemoveMediaStream2 failed s32Ret:%d\n", s32Ret);
    }
SERVER_STOP:
    /*server stop process */
    s32Ret = HI_RTSPSVR_Stop(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Stop failed s32Ret:%d\n", s32Ret);
    }

SERVER_DESTROY:
    s32Ret = HI_RTSPSVR_Destroy(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Destroy failed s32Ret:%d\n", s32Ret);
    }

    return s32Ret;
}




HI_S32 SAMPLE_RTSPSVR_264(  )
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RTSP_SOURCE_S stStreamSrc12;//for 264
    stStreamSrc12.pstVideoSrc = &stAvcVideo;
    stStreamSrc12.pstAudioSrc = &stAvcAudio;

    /*init the g_sRTSPConf and create rtspserver context*/
    g_sRTSPConf.s32ListenPort = 554;
    g_sRTSPConf.s32MaxConnNum = 5;
    g_sRTSPConf.s32MaxPayload = 3;
    g_sRTSPConf.s32PacketLen = 1500;
    g_sRTSPConf.s32Timeout = 6;

    s32Ret = HI_RTSPSVR_Create(&g_pServer, &g_sRTSPConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Create s32Ret:%d\n", s32Ret);
        return HI_SUCCESS;
    }

    statelistener.handle = HI_NULL;
    s32Ret = HI_RTSPSVR_SetListener(g_pServer, &statelistener);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Create s32Ret:%d\n", s32Ret);
        goto SERVER_DESTROY;
    }

    /*server start protocal parse */
    s32Ret = HI_RTSPSVR_Start(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Start failed s32Ret:%d\n", s32Ret);
        goto SERVER_DESTROY;
    }

    /*add mediastream 1 for session */
    s32Ret = HI_RTSPSVR_AddMediaStream(g_pServer, &stStreamSrc12, "12", 1024 * 1024 * 3);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_VSTREAM_Create fail s32Ret:%d\n", s32Ret);
        goto SERVER_STOP;
    }


    printf("HI_RTSPSVR_Start successs!\n");

    printf("please input any key to stop the rtsp server!\n");

    if (getchar())
    {
        printf("start to stop the server !\n");
    }

    s32Ret = HI_RTSPSVR_RemoveMediaStream(g_pServer, "12");
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_RemoveMediaStream2 failed s32Ret:%d\n", s32Ret);
    }

SERVER_STOP:
    /*server stop process */
    s32Ret = HI_RTSPSVR_Stop(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Stop failed s32Ret:%d\n", s32Ret);
    }

SERVER_DESTROY:
    s32Ret = HI_RTSPSVR_Destroy(g_pServer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_RTSPSVR_Destroy failed s32Ret:%d\n", s32Ret);
    }

    return s32Ret;
}

#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_CHAR   InputCmd[32];
    HI_S32 s32InputNumber = 0, i = 0;
    SAMPLE_mount_tmp_dir();
    Sample_InitTrackSource();

    printf("please input 'h' to get help or 'q' to quit!\n");

    while (1)
    {
        memset(InputCmd, 0, 30);

        printf("<input cmd:>");
        if (NULL == fgets((char*)InputCmd, 30, stdin))
        {
            print_help_info();
            continue;
        }


        /*filter backspace key*/
        for (i = 0; InputCmd[i] != '\0' && InputCmd[i] != 0xa; i++);

        if (InputCmd[i] == 0xa) { InputCmd[i] = '\0'; }

        if ( (0 == strcmp("q", (char*)InputCmd))
             || (0 == strcmp("Q", (char*)InputCmd))
             || (0 == strcmp("quit", (char*)InputCmd))
             || (0 == strcmp("QUIT", (char*)InputCmd)) )
        {
            printf("quit the program!\n");
            break;
        }

        if ( (0 == strcmp("help", (char*)InputCmd)) ||
             (0 == strcmp("h", (char*)InputCmd) ))
        {
            print_help_info();
            continue;
        }

        s32InputNumber = atoi(InputCmd);

        printf("[%s.%d]\n", __FUNCTION__, __LINE__);

        switch (s32InputNumber)
        {

            case 1:/* start one venc stream and the rtspserver  12 for the 1024*576 resolution */
                SAMPLE_RTSPSVR_264(12);
                break;

            case 2:/* start one venc stream and the rtspserver   11 for the 1920*1080 resolution*/
                SAMPLE_RTSPSVR_265(11);
                break;


            default:
            {
                printf("input cmd: %s is error \n", InputCmd);
                print_help_info();
                break;
            }
        }
    }

    SAMPLE_umount_tmp_dir();
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

