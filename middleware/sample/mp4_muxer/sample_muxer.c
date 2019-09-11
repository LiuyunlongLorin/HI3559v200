#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "avc.h"
#include "aac.h"
#include "hevc.h"
#include "prores.h"
#include "hi_type.h"
#include "hi_mp4_format.h"
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#ifndef __HIX86__
static const HI_CHAR* MOUNT_POINT = "/tmp";
#else
static const HI_CHAR* MOUNT_POINT = ".";
#endif

#ifdef __HuaweiLite__
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p0";
#else
static const HI_CHAR* SDCARD_DEV = "/dev/mmcblk0p1";
#endif

#define MUXER_SAMPLE_SRC_H264    "/tmp/1.h264"
#define MUXER_SAMPLE_SRC_H264_2  "/tmp/2.h264"
#define MUXER_SAMPLE_SRC_H265    "/tmp/1.h265"
#define MUXER_SAMPLE_SRC_AAC     "/tmp/1.aac"
#define MUXER_SAMPLE_SRC_PRORES  "/tmp/apco.prores"

static HI_S32 SAMPLE_mount_tmp_dir()
{
#ifndef __HIX86__
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef __HuaweiLite__
    s32Ret = mount("/dev/mmcblk0p0", "/tmp", "vfat", 0, 0);
#else
    s32Ret = mount("/dev/mmcblk0p1", "/tmp", "vfat", MS_NOEXEC, 0);
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
    HI_S32 s32Ret = umount("/tmp");
    if (HI_SUCCESS != s32Ret)
    {
        printf("umount tmp failed, errno:%d \n", errno);
    }
 #endif
    return;
}



typedef struct TrackInfo
{
    HI_MW_PTR hTrack;
    HI_VOID *pHandle;
}HI_TRACK_INFO_S;

static HI_MW_PTR pMP4Muxer = NULL;
//static HI_U32 s_u32InsertFrameGap = 1;
static pthread_mutex_t s_AudioChunkMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_AudioChunkWriteCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_VideoChunkMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_VideoChunkWriteCond = PTHREAD_COND_INITIALIZER;
static HI_BOOL s_bAudioFinished = HI_FALSE;
static HI_BOOL s_bVideoFinished = HI_FALSE;
static HI_BOOL s_bChunkedVideo = HI_FALSE;

static HI_VOID print_help_info(HI_VOID)
{
    printf("List all testtool command\n");
    printf("h list all command we provide\n");
    printf("q quit sample test\n");
    printf("1 muxer h264 stream from reading file\n");
    printf("2 muxer h265 stream from reading file\n");
    printf("3 muxer h264 and h265 dual videotrack file\n");
    printf("4 muxer h264 and metadata track file\n");
    printf("5 muxer prores file\n");
    printf("6 repair broken mp4 file\n");
}

static HI_VOID* Sample_ReadAACThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAACThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TRACK_INFO_S *pTrackInfo = (HI_TRACK_INFO_S*)pArg;
    AAC_Handle_S* aacHandle = (AAC_Handle_S*)(pTrackInfo->pHandle);
    HI_MW_PTR hAudioTrack = pTrackInfo->hTrack;
    HI_MP4_FRAME_DATA_S stFrameData;
    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    struct timeval stNowTime;
    HI_U32 u32FrameInChunk = 0;
    static HI_U32 u32AACChunkNum = 0;

    while ( HI_TRUE )
    {
        if(s_bChunkedVideo && !s_bVideoFinished)
        {
            pthread_mutex_lock(&s_AudioChunkMutex);
            printf("s_AudioChunkWriteCond wait\n");
            pthread_cond_wait(&s_AudioChunkWriteCond, &s_AudioChunkMutex);
            printf("s_AudioChunkWriteCond recived\n");
            pthread_mutex_unlock(&s_AudioChunkMutex);
        }

        printf("AAC_ReadFrame begin\n");

        u32FrameInChunk = 0;
        u32AACChunkNum++;

READ_NEXT_AAC:

        s32Ret = AAC_ReadFrame(aacHandle, &u8Frame, &u32FrameLen);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AAC_ReadFrame failed\n");

            if(s_bChunkedVideo)
            {
                s_bAudioFinished = HI_TRUE;
                pthread_mutex_lock(&s_VideoChunkMutex);
                printf("s_VideoChunkWriteCond signal\n");
                pthread_cond_signal(&s_VideoChunkWriteCond);
                printf("s_VideoChunkWriteCond signal end\n");
                pthread_mutex_unlock(&s_VideoChunkMutex);
            }
            break;
        }

        gettimeofday(&stNowTime, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = HI_FALSE;
        stFrameData.u64TimeStamp = (HI_U64)stNowTime.tv_sec * 1000000 + stNowTime.tv_usec;
        s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hAudioTrack, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Sample_ReadAACThread HI_MP4_WriteFrame fail %d \n", s32Ret);
        }

        u32FrameInChunk++;

        printf("AAC_ReadFrame OK, u32FrameInChunk[%u], ChunkNum[%d]\n",u32FrameInChunk,u32AACChunkNum);
        usleep(2*1000);

        // make sure one aac chunk is about 0.5 second: sampleRate*0.5/samplePerFrame = 23.4
        if( s_bChunkedVideo && u32FrameInChunk <= 24 )
        {
            goto READ_NEXT_AAC;
        }

        printf("AAC_ReadFrame end\n");

        if(s_bChunkedVideo)
        {
            pthread_mutex_lock(&s_VideoChunkMutex);
            printf("s_VideoChunkWriteCond signal\n");
            pthread_cond_signal(&s_VideoChunkWriteCond);
            printf("s_VideoChunkWriteCond signal end\n");
            pthread_mutex_unlock(&s_VideoChunkMutex);
        }
    }
    return NULL;
}

static HI_VOID* Sample_ReadAVCThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadAVCThread", 0, 0, 0);
    HI_U32 u32FrameNum = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    struct timeval tv;
    HI_TRACK_INFO_S *pTrackInfo = (HI_TRACK_INFO_S*)pArg;
    AVC_Handle_S* avcHandle = (AVC_Handle_S*)(pTrackInfo->pHandle);
    HI_MW_PTR hAvcTrack = pTrackInfo->hTrack;

    HI_MP4_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_MP4_FRAME_DATA_S));
    printf("Sample_ReadAVCThread start read frame \n");
    while ( HI_TRUE )
    {
        s32Ret = AVC_ReadFrame(avcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AVC_ReadFrame failed\n");
            break;
        }

        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL)u8KeyFrame;
        stFrameData.u64TimeStamp = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
        s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hAvcTrack, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Sample_ReadAVCThread HI_MP4_WriteFrame fail %d \n", s32Ret);
        }
        u32FrameNum++;
#if 0
        if(u32FrameNum%s_u32InsertFrameGap==0 )
        {
            gettimeofday(&tv, NULL);
            stFrameData.pu8DataBuffer = NULL;
            stFrameData.u32DataLength = 0;
            stFrameData.bKeyFrameFlag = HI_FALSE;
            stFrameData.u64TimeStamp = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
            s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hAvcTrack, &stFrameData);
            if (HI_SUCCESS != s32Ret)
            {
                printf("Sample_ReadAVCThread HI_MP4_WriteFrame fail %d \n", s32Ret);
            }
        }
#endif
        usleep(5*1000);
    }

    return NULL;
}

static HI_VOID* Sample_ReadHEVCThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadHEVCThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TRACK_INFO_S *pTrackInfo = (HI_TRACK_INFO_S*)pArg;
    HI_MW_PTR hHEVCTrack = pTrackInfo->hTrack;
    HEVC_Handle_S* hevcHandle = (HEVC_Handle_S*)(pTrackInfo->pHandle);

    HI_U32 u32FrameNum = 0;
    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    HI_MP4_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_MP4_FRAME_DATA_S));
    struct timeval tv;

    while ( HI_TRUE )
    {
        s32Ret = HEVC_ReadFrame(hevcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HEVC_ReadFrame failed\n");
            break;
        }

        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL)u8KeyFrame;
        stFrameData.u64TimeStamp = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
        s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hHEVCTrack, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Sample_ReadHEVCThread HI_MP4_WriteFrame fail %d \n", s32Ret);
        }
        u32FrameNum++;

        usleep(5*1000);
    }
    return NULL;
}

static HI_VOID* Sample_ReadProresThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadProresThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    HI_TRACK_INFO_S *pTrackInfo = (HI_TRACK_INFO_S*)pArg;
    HI_MW_PTR hProresTrack = pTrackInfo->hTrack;
    PRORES_Handle_S* proresHandle = (PRORES_Handle_S*)(pTrackInfo->pHandle);

    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_MP4_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_MP4_FRAME_DATA_S));
    struct timeval tv;
    HI_U32 u32SizeOfChunk = 0;
    static HI_U32 u32ProResChunkNum = 0;

    while ( HI_TRUE )
    {
        if(!s_bAudioFinished)
        {
            pthread_mutex_lock(&s_VideoChunkMutex);
            printf("s_VideoChunkWriteCond wait\n");
            pthread_cond_wait(&s_VideoChunkWriteCond, &s_VideoChunkMutex);
            printf("s_VideoChunkWriteCond recived\n");
            pthread_mutex_unlock(&s_VideoChunkMutex);
        }

        printf("PRORES_ReadFrame begin\n");

        u32SizeOfChunk = 0;
        u32ProResChunkNum++;

READ_NEXT_PRORES:
        s32Ret = PRORES_ReadFrame(proresHandle, &u8Frame, &u32FrameLen);
        if (HI_SUCCESS != s32Ret)
        {
            printf("PRORES_ReadFrame failed\n");
            s_bVideoFinished = HI_TRUE;
            pthread_mutex_lock(&s_AudioChunkMutex);
            printf("s_AudioChunkWriteCond signal\n");
            pthread_cond_signal(&s_AudioChunkWriteCond);
            printf("s_AudioChunkWriteCond signal end\n");
            pthread_mutex_unlock(&s_AudioChunkMutex);
            break;
        }

        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = HI_FALSE;
        stFrameData.u64TimeStamp = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
        s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hProresTrack, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MP4_WriteFrame fail %d \n", s32Ret);
        }

        u32SizeOfChunk+=u32FrameLen;
        printf("PRORES_ReadFrame ok, u32SizeOfChunk[%u], ChunkNum[%d]\n", u32SizeOfChunk, u32ProResChunkNum);
        usleep(1000);

        // make sure one ProRes chunk size is ~4MB.
        if( u32SizeOfChunk<=(4*1024*1024-200*1024) )
        {
            goto READ_NEXT_PRORES;
        }

        printf("PRORES_ReadFrame end\n");

        pthread_mutex_lock(&s_AudioChunkMutex);
        printf("s_AudioChunkWriteCond signal\n");
        pthread_cond_signal(&s_AudioChunkWriteCond);
        printf("s_AudioChunkWriteCond signal end\n");
        pthread_mutex_unlock(&s_AudioChunkMutex);
    }
    return NULL;
}

static HI_VOID* Sample_ReadDataThread(void* pArg)
{
    prctl(PR_SET_NAME, "Sample_ReadDataThread", 0, 0, 0);
    HI_U32 u32FrameNum = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* u8Frame = HI_NULL;
    HI_U32 u32FrameLen = 0;
    HI_U8 u8KeyFrame = HI_FALSE;
    struct timeval tv;
    HI_TRACK_INFO_S *pTrackInfo = (HI_TRACK_INFO_S*)pArg;
    AVC_Handle_S* avcHandle = (AVC_Handle_S*)(pTrackInfo->pHandle);
    HI_MW_PTR hDataTrack = pTrackInfo->hTrack;

    HI_MP4_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_MP4_FRAME_DATA_S));
    printf("Sample_ReadDataThread start read frame \n");
    while ( HI_TRUE )
    {
        s32Ret = AVC_ReadFrame(avcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("AVC_ReadFrame failed\n");
            break;
        }

        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL)u8KeyFrame;
        stFrameData.u64TimeStamp = (HI_U64)tv.tv_sec * 1000000 + tv.tv_usec;
        s32Ret = HI_MP4_WriteFrame(pMP4Muxer, hDataTrack, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Sample_ReadDataThread HI_MP4_WriteFrame fail %d\n", s32Ret);
        }
        u32FrameNum++;
        usleep(5*1000);
    }

    return NULL;
}

static HI_S32 SAMPLE_SetDuration(HI_MW_PTR hHandle, HI_U32 u32Timescale, HI_U64* pu64Duration)
{
    *pu64Duration = (HI_U64)u32Timescale * 60;
    return HI_SUCCESS;
}

HI_S32 SAMPLE_MP4_WriteNoFrame()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64Duration = 0;
    HI_MW_PTR pMP4Muxer = NULL;
    HI_MW_PTR s_VideoTrack0 = NULL;
    HI_MP4_TRACK_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
    stVideo.fSpeed = 1.0f;
    stVideo.u32TimeScale = 120000;
    stVideo.stVideoInfo.u32BitRate = 3000;
    stVideo.stVideoInfo.u32FrameRate =  30;
    stVideo.stVideoInfo.u32Height = 1920;
    stVideo.stVideoInfo.u32Width = 1080;
    snprintf(stVideo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MW_PTR hAudioTrack = NULL;
    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.u32TimeScale = 48000;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");

    HI_MW_PTR hDataTrack = NULL;
    HI_MP4_TRACK_INFO_S stData;
    memset(&stData, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stData.fSpeed = 1.0f;
    stData.u32TimeScale = 48000;
    stData.enTrackType = HI_MP4_STREAM_DATA;
    stData.stDataInfo.u32FrameRate = 30;
    snprintf(stData.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon META");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT, "test_avc.mp4");
    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 0;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 100 * 1024 * 1024;

    printf("create muxer\n");
    s32Ret = HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_muxer_Create fail %d \n", s32Ret);
        return HI_FAILURE;
    }


    printf("create video\n");
    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &s_VideoTrack0, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Create fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }
    printf("create audio\n");
    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &hAudioTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    printf("create data stream\n");
    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &hDataTrack, &stData);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    printf("destroy all stream\n");
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, SAMPLE_SetDuration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }


    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
TRACK_DESTROY:
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, SAMPLE_SetDuration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 SAMPLE_MP4_MuxerAVC()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64Duration = 0;

    HI_TRACK_INFO_S stAVCTrackInfo;
    HI_TRACK_INFO_S stAACTrackInfo;
    AVC_Handle_S stAvcHandle;
    AAC_Handle_S stAACHandle;
    stAVCTrackInfo.pHandle = &stAvcHandle;
    stAACTrackInfo.pHandle = &stAACHandle;

    HI_MP4_TRACK_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
    stVideo.fSpeed = 1.0f;
    stVideo.stVideoInfo.u32BitRate = 3000;
    stVideo.stVideoInfo.u32FrameRate = 30;
    stVideo.stVideoInfo.u32Height = 1080;
    stVideo.stVideoInfo.u32Width = 1920;
    stVideo.u32TimeScale = 120000;
    snprintf(stVideo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    stAudio.u32TimeScale = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT, "test_avc.mp4");

    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 20*1024*1024;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024*1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 1*1024*1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret =  HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("create muxer fail %d \n", s32Ret);
        return HI_FAILURE;
    }
    /*create stream */
    printf("create video\n");
    s32Ret =  HI_MP4_CreateTrack(pMP4Muxer, &stAVCTrackInfo.hTrack, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    printf("create audio\n");
    s32Ret =  HI_MP4_CreateTrack(pMP4Muxer, &stAACTrackInfo.hTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    /*add user define box */
    HI_MP4_ATOM_INFO_S stBoxInfo;
    memset(&stBoxInfo, 0x00, sizeof(HI_MP4_ATOM_INFO_S));
    stBoxInfo.aszType[0] = 'd';
    stBoxInfo.aszType[1] = 'd';
    stBoxInfo.aszType[2] = 'd';
    stBoxInfo.aszType[3] = 'd';
    stBoxInfo.aszType[4] = '\0';
    HI_CHAR aszData[16] = "12345678abcdef";
    stBoxInfo.pu8DataBuf = (HI_U8*)aszData;
    stBoxInfo.u32DataLen = 16;
    printf("add box\n");
    s32Ret = HI_MP4_AddAtom(pMP4Muxer, "/moov/trak[1]/mdia", &stBoxInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_AddAtom fail /moov/trak[1]/mdia %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    s32Ret = HI_MP4_AddAtom(pMP4Muxer, "/moov", &stBoxInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_AddAtom  fail /moov %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    s32Ret = HI_MP4_AddAtom(pMP4Muxer, "/", &stBoxInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_AddAtom fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    /*start write frame data*/
    if (-1 == AVC_Open(stAVCTrackInfo.pHandle, MUXER_SAMPLE_SRC_H264))
    {
        printf("AVC_Open %s fail %d \n", MUXER_SAMPLE_SRC_H264, s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == AAC_Open(stAACTrackInfo.pHandle, MUXER_SAMPLE_SRC_AAC))
    {

        printf("AAC_Open %s fail %d \n", MUXER_SAMPLE_SRC_AAC, s32Ret);
        goto TRACK_DESTROY;
    }

    /* start audio/video read and mux thread */
    pthread_t AVCThread;
    pthread_t AACThread;
    pthread_create(&AVCThread, HI_NULL, Sample_ReadAVCThread, (void*)&stAVCTrackInfo);
//    sleep(1000);
    pthread_create(&AACThread, HI_NULL, Sample_ReadAACThread, (void*)&stAACTrackInfo);
    pthread_join(AVCThread, HI_NULL);
    pthread_join(AACThread, HI_NULL);

    printf("muxer end, wait before write tail\n");

    AVC_Close(stAVCTrackInfo.pHandle);
    AAC_Close(stAACTrackInfo.pHandle);
    /*write tail and destroy stream*/
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }

    /*destroy muxer*/
    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail  %d \n", s32Ret);
    }
    printf("destroy muxer duration %lld \n", u64Duration);

    return s32Ret;
TRACK_DESTROY:
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 SAMPLE_MP4_MuxerAVCPlusHEVC()
{
    HI_U64 u64Duration = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_TRACK_INFO_S stAVCTrackInfo;
    HI_TRACK_INFO_S stHEVCTrackInfo;
    HI_TRACK_INFO_S stAACTrackInfo;

    AVC_Handle_S stAvcHandle;
    HEVC_Handle_S stHevcHandle;
    AAC_Handle_S stAACHandle;
    stAVCTrackInfo.pHandle = &stAvcHandle;
    stHEVCTrackInfo.pHandle = &stHevcHandle;
    stAACTrackInfo.pHandle = &stAACHandle;

    HI_MP4_TRACK_INFO_S stVideo0;
    memset(&stVideo0, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo0.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo0.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
    stVideo0.fSpeed = 1.0f;
    stVideo0.stVideoInfo.u32BitRate = 3000;
    stVideo0.stVideoInfo.u32FrameRate = 30;
    stVideo0.stVideoInfo.u32Height = 1080;
    stVideo0.stVideoInfo.u32Width = 1920;
    stVideo0.u32TimeScale = 120000;
    snprintf(stVideo0.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stVideo1;
    memset(&stVideo1, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo1.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo1.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
    stVideo1.fSpeed = 1.0f;
    stVideo1.stVideoInfo.u32BitRate = 3000;
    stVideo1.stVideoInfo.u32FrameRate = 30;
    stVideo1.stVideoInfo.u32Height = 240;
    stVideo1.stVideoInfo.u32Width = 480;
    stVideo1.u32TimeScale = 120000;
    snprintf(stVideo1.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    stAudio.u32TimeScale = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT, "test_avc_hevc.mp4");
    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 0;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 4 * 1024 * 1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Create fail %d \n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stAVCTrackInfo.hTrack, &stVideo0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stHEVCTrackInfo.hTrack, &stVideo1);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stAACTrackInfo.hTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    if (-1 == AVC_Open(stAVCTrackInfo.pHandle, MUXER_SAMPLE_SRC_H264))
    {
        printf("AVC_Open %s fail %d \n", MUXER_SAMPLE_SRC_H264, s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == HEVC_Open(stHEVCTrackInfo.pHandle, MUXER_SAMPLE_SRC_H265))
    {
        printf("HEVC_Open %s fail %d \n", MUXER_SAMPLE_SRC_H265, s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == AAC_Open(stAACTrackInfo.pHandle, MUXER_SAMPLE_SRC_AAC))
    {

        printf("AAC_Open %s fail %d \n", MUXER_SAMPLE_SRC_AAC, s32Ret);
        goto TRACK_DESTROY;
    }

    /* start audio/video read and mux thread */
    pthread_t AVCThread;
    pthread_t HEVCThread;
    pthread_t AACThread;
    pthread_create(&AVCThread, HI_NULL, Sample_ReadAVCThread, (void*)&stAVCTrackInfo);
    usleep(300*1000);
    pthread_create(&HEVCThread, HI_NULL, Sample_ReadHEVCThread, (void*)&stHEVCTrackInfo);
    pthread_create(&AACThread, HI_NULL, Sample_ReadAACThread, (void*)&stAACTrackInfo);
    pthread_join(AVCThread, HI_NULL);
    pthread_join(HEVCThread, HI_NULL);
    pthread_join(AACThread, HI_NULL);

    AVC_Close(stAVCTrackInfo.pHandle);
    HEVC_Close(stHEVCTrackInfo.pHandle);
    AAC_Close(stAACTrackInfo.pHandle);
    printf("destroy streams\n");

TRACK_DESTROY:

    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 SAMPLE_MP4_MuxerAVCPlusData()
{
    HI_U64 u64Duration = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_TRACK_INFO_S stAVCTrackInfo;
    HI_TRACK_INFO_S stAACTrackInfo;
    HI_TRACK_INFO_S stDataTrackInfo;

    AVC_Handle_S stAvcHandle1;
    AVC_Handle_S stAvcHandle2;
    AAC_Handle_S stAACHandle;
    stAVCTrackInfo.pHandle = &stAvcHandle1;
    stDataTrackInfo.pHandle = &stAvcHandle2;
    stAACTrackInfo.pHandle = &stAACHandle;

    HI_MP4_TRACK_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H264;
    stVideo.fSpeed = 1.0f;
    stVideo.stVideoInfo.u32BitRate = 3000;
    stVideo.stVideoInfo.u32FrameRate = 30;
    stVideo.stVideoInfo.u32Height = 1080;
    stVideo.stVideoInfo.u32Width = 1920;
    stVideo.u32TimeScale = 120000;
    snprintf(stVideo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stData;
    memset(&stData, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stData.enTrackType = HI_MP4_STREAM_DATA;
    stData.stDataInfo.enCodecID = HI_MP4_CODEC_ID_H264;
    stData.fSpeed = 1.0f;
    stData.stDataInfo.u32FrameRate = 30;
    stData.stDataInfo.u32Height = 240;
    stData.stDataInfo.u32Width = 480;
    stData.u32TimeScale = 120000;
    snprintf(stData.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon DATA");

    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    stAudio.u32TimeScale = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT,"test_avc_metadata.mp4");
    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 0;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 4 * 1024 * 1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Create fail %d \n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stAVCTrackInfo.hTrack, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stDataTrackInfo.hTrack, &stData);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stAACTrackInfo.hTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    if (-1 == AVC_Open(stAVCTrackInfo.pHandle, MUXER_SAMPLE_SRC_H264))
    {
        printf("AVC_Open 1.h264 fail %d\n", s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == AVC_Open(stDataTrackInfo.pHandle, MUXER_SAMPLE_SRC_H264_2))
    {
        printf("AVC_Open %s fail %d\n", MUXER_SAMPLE_SRC_H264_2, s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == AAC_Open(stAACTrackInfo.pHandle, MUXER_SAMPLE_SRC_AAC))
    {

        printf("AAC_Open %s fail  %d\n", MUXER_SAMPLE_SRC_AAC, s32Ret);
        goto TRACK_DESTROY;
    }

    /* start audio/video read and mux thread */
    pthread_t AVCThread;
    pthread_t DataThread;
    pthread_t AACThread;
    pthread_create(&AVCThread, HI_NULL, Sample_ReadAVCThread, (void*)&stAVCTrackInfo);
    usleep(200*1000);
    pthread_create(&DataThread, HI_NULL, Sample_ReadDataThread, (void*)&stDataTrackInfo);
    pthread_create(&AACThread, HI_NULL, Sample_ReadAACThread, (void*)&stAACTrackInfo);
    pthread_join(AVCThread, HI_NULL);
    pthread_join(DataThread, HI_NULL);
    pthread_join(AACThread, HI_NULL);

    AVC_Close(stAVCTrackInfo.pHandle);
    AVC_Close(stDataTrackInfo.pHandle);
    AAC_Close(stAACTrackInfo.pHandle);
    printf("destroy streams\n");

TRACK_DESTROY:

    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

// DO NOT modify HdlrName unless you know what you are doing, see Apple Prores Container Guidline
HI_S32 SAMPLE_MP4_MuxerPRORES()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64Duration = 0;
    HI_MP4_TRACK_INFO_S stVideo;

    HI_TRACK_INFO_S stProresTrackInfo;
    HI_TRACK_INFO_S stAACTrackInfo;
    PRORES_Handle_S stProresHandle;
    AAC_Handle_S stAACHandle;
    stProresTrackInfo.pHandle = &stProresHandle;
    stAACTrackInfo.pHandle = &stAACHandle;

    memset(&stVideo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_PRORES_422_PROXY;
    stVideo.fSpeed = 1.0f;
    stVideo.stVideoInfo.u32BitRate = 3000;
    stVideo.stVideoInfo.u32FrameRate = 30;
    stVideo.stVideoInfo.u32Height = 1080;
    stVideo.stVideoInfo.u32Width = 1920;
    stVideo.u32TimeScale = 3000;
    snprintf(stVideo.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Apple ProRes 422 Proxy");

    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    stAudio.u32TimeScale = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Apple AUDIO");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT, "test_prores_apco.mp4");
    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 0;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 0;
    stMuxerCfg.stMuxerConfig.enFormatProfile = HI_MP4_FORMAT_QT;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret =  HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("create muxer fail %d \n", s32Ret);
        return HI_FAILURE;
    }
    /*create stream */
    printf("create video\n");
    s32Ret =  HI_MP4_CreateTrack(pMP4Muxer, &stProresTrackInfo.hTrack, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    printf("create audio\n");
    s32Ret =  HI_MP4_CreateTrack(pMP4Muxer, &stAACTrackInfo.hTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }

    /*start write frame data*/
    if (-1 == PRORES_Open(stProresTrackInfo.pHandle, MUXER_SAMPLE_SRC_PRORES))
    {
        printf("PRORES_Open %s fail %d \n", MUXER_SAMPLE_SRC_PRORES, s32Ret);
        goto TRACK_DESTROY;
    }
    if (-1 == AAC_Open(stAACTrackInfo.pHandle, MUXER_SAMPLE_SRC_AAC))
    {
        printf("AAC_Open %s fail %d \n", MUXER_SAMPLE_SRC_AAC, s32Ret);
        goto TRACK_DESTROY;
    }

    /* start audio/video read and mux thread */
    pthread_t ProresThread;
    pthread_t AACThread;
    s_bChunkedVideo = HI_TRUE;

    pthread_create(&ProresThread, HI_NULL, Sample_ReadProresThread, (void*)&stProresTrackInfo);
    pthread_create(&AACThread, HI_NULL, Sample_ReadAACThread, (void*)&stAACTrackInfo);

    usleep(100*1000);

    pthread_mutex_lock(&s_AudioChunkMutex);
    pthread_cond_signal(&s_AudioChunkWriteCond);
    pthread_mutex_unlock(&s_AudioChunkMutex);

    pthread_join(ProresThread, HI_NULL);
    pthread_join(AACThread, HI_NULL);

    pthread_mutex_destroy(&s_AudioChunkMutex);
    pthread_cond_destroy(&s_AudioChunkWriteCond);
    pthread_mutex_destroy(&s_VideoChunkMutex);
    pthread_cond_destroy(&s_VideoChunkWriteCond);

    PRORES_Close(stProresTrackInfo.pHandle);
    AAC_Close(stAACTrackInfo.pHandle);
    /*write tail and destroy stream*/
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }

    /*destroy muxer*/
    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail  %d \n", s32Ret);
    }
    printf("destroy muxer duration %lld \n", u64Duration);

    return s32Ret;
TRACK_DESTROY:
    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, SAMPLE_SetDuration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 SAMPLE_MP4_MuxerHEVC()
{
    HI_U64 u64Duration = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_TRACK_INFO_S stHEVCTrackInfo0;
    HI_TRACK_INFO_S stHEVCTrackInfo1;
    HI_TRACK_INFO_S stAACTrackInfo;

    HEVC_Handle_S stHevcHandle0;
    HEVC_Handle_S stHevcHandle1;
    AAC_Handle_S stAACHandle1;
    stHEVCTrackInfo0.pHandle = &stHevcHandle0;
    stHEVCTrackInfo1.pHandle = &stHevcHandle1;
    stAACTrackInfo.pHandle = &stAACHandle1;

    HI_MP4_TRACK_INFO_S stVideo0;
    memset(&stVideo0, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo0.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo0.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
    stVideo0.fSpeed = 1.0f;
    stVideo0.stVideoInfo.u32BitRate = 3000;
    stVideo0.stVideoInfo.u32FrameRate = 30;
    stVideo0.stVideoInfo.u32Height = 1080;
    stVideo0.stVideoInfo.u32Width = 1920;
    stVideo0.u32TimeScale = 120000;
    snprintf(stVideo0.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stVideo1;
    memset(&stVideo1, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stVideo1.enTrackType = HI_MP4_STREAM_VIDEO;
    stVideo1.stVideoInfo.enCodecID = HI_MP4_CODEC_ID_H265;
    stVideo1.fSpeed = 1.0f;
    stVideo1.stVideoInfo.u32BitRate = 3000;
    stVideo1.stVideoInfo.u32FrameRate = 30;
    stVideo1.stVideoInfo.u32Height = 576;
    stVideo1.stVideoInfo.u32Width = 1024;
    stVideo1.u32TimeScale = 120000;
    snprintf(stVideo1.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon VIDEO");

    HI_MP4_TRACK_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    stAudio.fSpeed = 1.0f;
    stAudio.enTrackType = HI_MP4_STREAM_AUDIO;
    stAudio.stAudioInfo.enCodecID = HI_MP4_CODEC_ID_AACLC;
    stAudio.stAudioInfo.u16SampleSize = 16;
    stAudio.stAudioInfo.u32Channels = 1;
    stAudio.stAudioInfo.u32SamplePerFrame = 1024;
    stAudio.stAudioInfo.u32SampleRate = 48000;
    stAudio.u32TimeScale = 48000;
    snprintf(stAudio.aszHdlrName, HI_MP4_MAX_HDLR_NAME, "%s", "Hisilicon AUDIO");

    HI_MP4_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, "%s/%s", MOUNT_POINT, "test_hevc.mp4");

    stMuxerCfg.enConfigType = HI_MP4_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 20*1024*1024;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024 * 1024;
    stMuxerCfg.stMuxerConfig.bConstantFps = HI_TRUE;
    stMuxerCfg.stMuxerConfig.bCo64Flag = HI_FALSE;
    stMuxerCfg.stMuxerConfig.u32BackupUnit = 4 * 1024 * 1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_MP4_Create(&pMP4Muxer, &stMuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Create fail %d \n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stHEVCTrackInfo0.hTrack, &stVideo0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }
#if 0
    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stHEVCTrackInfo1.hTrack, &stVideo1);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }
#endif
    s32Ret = HI_MP4_CreateTrack(pMP4Muxer, &stAACTrackInfo.hTrack, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_CreateTrack fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    if (-1 == HEVC_Open(stHEVCTrackInfo0.pHandle, MUXER_SAMPLE_SRC_H265))
    {
        printf("HEVC_Open %s fail %d \n", s32Ret, MUXER_SAMPLE_SRC_H265);
        goto TRACK_DESTROY;
    }
#if 0
    if (-1 == HEVC_Open(stHEVCTrackInfo1.pHandle, "21.h265"))
    {
        printf("PRORES_Open 21.h265 fail %d \n", s32Ret);
        goto TRACK_DESTROY;
    }
#endif
    if (-1 == AAC_Open(stAACTrackInfo.pHandle, MUXER_SAMPLE_SRC_AAC))
    {

        printf("AVC_Open %s fail %d \n", MUXER_SAMPLE_SRC_AAC, s32Ret);
        goto TRACK_DESTROY;
    }

    /* start audio/video read and mux thread */
    pthread_t HEVCThread0;
    pthread_t HEVCThread1;
    pthread_t AACThread;
    pthread_create(&HEVCThread0, HI_NULL, Sample_ReadHEVCThread, (void*)&stHEVCTrackInfo0);
//    pthread_create(&HEVCThread1, HI_NULL, Sample_ReadHEVCThread, (void*)&stHEVCTrackInfo1);
    pthread_create(&AACThread, HI_NULL, Sample_ReadAACThread, (void*)&stAACTrackInfo);
    pthread_join(HEVCThread0, HI_NULL);
//    pthread_join(HEVCThread1, HI_NULL);
    pthread_join(AACThread, HI_NULL);

    HEVC_Close(stHEVCTrackInfo0.pHandle);
//    HEVC_Close(stHEVCTrackInfo1.pHandle);
    AAC_Close(stAACTrackInfo.pHandle);
    printf("destroy streams\n");

TRACK_DESTROY:

    s32Ret = HI_MP4_DestroyAllTracks(pMP4Muxer, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_DestroyAllTracks fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    printf("destroy muxer\n");
    s32Ret = HI_MP4_Destroy(pMP4Muxer, &u64Duration);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MP4_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
}

HI_S32 SAMPLE_MP4_RepairFile(HI_CHAR *path)
{
    printf("start repair:%s\n", path);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32DirCnt = 0;
    struct dirent** ppNameList = HI_NULL;
    struct stat s_buf;
    HI_CHAR filePath[256];

    // repair single file
    if (HI_SUCCESS != stat(path, &s_buf))
    {
        printf("stat failed[%s]\n", path);
        return -1;
    }

    if (!S_ISDIR(s_buf.st_mode))
    {
        s32Ret = HI_MP4_RepairFile(path, HI_TRUE);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MP4_RepairFile[%s] fail\n", path);
        }
        else
        {
            printf("HI_MP4_RepairFile[%s] success!\n", path);
        }
        return 0;
    }

    // repair all file in dir
    s32DirCnt = scandir(path, &ppNameList, 0, HI_NULL);
    if (HI_FAILURE == s32DirCnt)
    {
        printf("errno=%d\n", errno);
        return -1;
    }

    while (s32DirCnt-- && s32DirCnt >= 0)
    {
        snprintf_s(filePath, 256, 255, "%s/%s", path, ppNameList[s32DirCnt]->d_name);

        if (HI_SUCCESS != stat(filePath, &s_buf))
        {
            printf("stat failed[%s]\n", filePath);
            free(ppNameList[s32DirCnt]);
            continue;
        }

        if (S_ISDIR(s_buf.st_mode))
        {
            printf("skip:[%s]\n", filePath);
            free(ppNameList[s32DirCnt]);
            continue;
        }

        s32Ret = HI_MP4_RepairFile(filePath, HI_TRUE);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MP4_RepairFile[%s] fail\n", filePath);
        }
        else
        {
            printf("HI_MP4_RepairFile[%s] success!\n", filePath);
        }
        free(ppNameList[s32DirCnt]);
    }

    free(ppNameList);
    ppNameList = HI_NULL;

    return HI_SUCCESS;
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
    HI_CHAR   InputCmd[32];
    HI_CHAR path[256];
    HI_S32 s32InputNumber = 0, i = 0;

    printf("please input 'h' to get help or 'q' to quit!\n");
    if (HI_SUCCESS != SAMPLE_mount_tmp_dir())
    {
        return HI_FAILURE;
    }

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
            case 1:/* sample avc muxer  */
                SAMPLE_MP4_MuxerAVC();
                break;
            case 2:/* hevc muxer */
                SAMPLE_MP4_MuxerHEVC();
                break;
            case 3:/* avc plus hevc muxer*/
                SAMPLE_MP4_MuxerAVCPlusHEVC();
                break;
            case 4:/* avc plus metadata muxer */
                SAMPLE_MP4_MuxerAVCPlusData();
                break;
            case 5:/* prores muxer */
                SAMPLE_MP4_MuxerPRORES();
                break;
            case 6:/* repair */
                printf("Please input the file or dir name to repair:\n");
                fscanf(stdin, "%s", path);
                SAMPLE_MP4_RepairFile(path);
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
