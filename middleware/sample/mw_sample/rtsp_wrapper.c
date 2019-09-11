#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "rtsp_wrapper.h"
#include "hi_rtsp_server.h"
#include "hi_track_source.h"
#include "venc_wrapper.h"
HI_MW_PTR s_hRtspServerHandle;
HI_RTSP_SOURCE_S g_stStreamSrc = {0};
HI_Track_Source_S stVideoSrc = {0};
HI_Track_Source_S stAudioSrc = {0};


static HI_S32 audio_start(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return Aenc4RtspStart(pTrackSource, pCaller);
}

static HI_S32 audio_stop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return Aenc4RtspStop(pTrackSource, pCaller);
}


static HI_S32 video_start(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return Venc4RtspStart(pTrackSource, pCaller);
}

static HI_S32 video_stop(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    return Venc4RtspStop(pTrackSource, pCaller);
}

static HI_VOID onServerError(HI_SERVER_STATE_LISTENER_S* handle, HI_S32 errId, HI_CHAR* msg)
{
    printf("server error handle:%p, id:%d, msd:%s\n", handle, errId, msg);
    return;
}

static HI_VOID onClientConnect(HI_SERVER_STATE_LISTENER_S* handle, HI_CHAR* ip)
{
    printf("onClientConnect handle:%p, ip:%s\n", handle, ip);
    return;
}

static HI_VOID onClientDisconnect(HI_SERVER_STATE_LISTENER_S* handle, HI_CHAR* ip)
{
    printf("onClientDisconnect handle:%p, ip:%s \n", handle, ip);
    return;
}
static HI_S32 video_requestKeyFrame(HI_Track_Source_Handle pTrackSource, HI_MW_PTR pCaller)
{
    printf("video_requestKeyFrame \n");

    return HI_SUCCESS;
}
/*add the statelistener of rtspserver*/

static HI_SERVER_STATE_LISTENER_S  statelistener =
{
    .onClientConnect = onClientConnect,
    .onClientDisconnect = onClientDisconnect,
    .onServerError = onServerError,
};


HI_S32 RTSPSERVER_Start(HI_S32 s32Port)
{
    HI_RTSP_CONFIG_S stRtspConfig;
    HI_MW_VEncAttr stVEncAttr;
    HI_HANDLE hAEncHdl;

    GetLiveVEncAttr(&stVEncAttr);
    memset(&stVideoSrc, 0x00, sizeof(stVideoSrc));
    stVideoSrc.enTrackType = HI_TRACK_SOURCE_TYPE_VIDEO;
    stVideoSrc.pfnSourceStart  = Venc4RtspStart;
    stVideoSrc.pfnSourceStop = Venc4RtspStop;
    stVideoSrc.pfnRequestKeyFrame = video_requestKeyFrame;
    stVideoSrc.s32PrivateHandle = stVEncAttr.HANDLE;
    if (HI_MW_PAYLOAD_TYPE_H264 == stVEncAttr.PAYLOAD_TYPE)
    {
        stVideoSrc.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == stVEncAttr.PAYLOAD_TYPE)
    {
        stVideoSrc.unTrackSourceAttr.stVideoInfo.enCodecType = HI_TRACK_VIDEO_CODEC_H265;
    }

    stVideoSrc.unTrackSourceAttr.stVideoInfo.u32BitRate = stVEncAttr.BITRATE;
    stVideoSrc.unTrackSourceAttr.stVideoInfo.u32FrameRate = stVEncAttr.FRAMERATE;
    stVideoSrc.unTrackSourceAttr.stVideoInfo.u32Height = stVEncAttr.HEIGHT;
    stVideoSrc.unTrackSourceAttr.stVideoInfo.u32Width = stVEncAttr.WIDTH;

    GetAEncAttr(&hAEncHdl);
    memset(&stAudioSrc, 0x00, sizeof(stAudioSrc));
    stAudioSrc.enTrackType = HI_TRACK_SOURCE_TYPE_AUDIO;
    stAudioSrc.pfnSourceStart  = Aenc4RtspStart;
    stAudioSrc.pfnSourceStop = Aenc4RtspStop;
    stAudioSrc.pfnRequestKeyFrame = NULL;
    stAudioSrc.s32PrivateHandle = hAEncHdl;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.enCodecType = HI_TRACK_AUDIO_CODEC_AAC;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.u16SampleBitWidth = 16;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.u32AvgBytesPerSec = 48000;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.u32ChnCnt = 1;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.u32SampleRate = 48000;
    stAudioSrc.unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = 1024;

    g_stStreamSrc.pstVideoSrc = &stVideoSrc;
    g_stStreamSrc.pstAudioSrc = &stAudioSrc;

    stRtspConfig.s32PacketLen = 1500;
    stRtspConfig.s32MaxConnNum = 2;
    stRtspConfig.s32ListenPort = s32Port;
    stRtspConfig.s32MaxPayload = 2;
    stRtspConfig.s32Timeout = 10;
    if (HI_SUCCESS != HI_RTSPSVR_Create(&s_hRtspServerHandle, &stRtspConfig))
    {
        printf("call HI_RTSPSVR_Create fail\n");
        return HI_FAILURE;
    }
    HI_RTSPSVR_SetListener(s_hRtspServerHandle, &statelistener);
    if (HI_SUCCESS != HI_RTSPSVR_Start(s_hRtspServerHandle))
    {
        printf("call HI_RTSPSVR_Start fail\n");
        HI_RTSPSVR_Destroy(s_hRtspServerHandle);
    }

    if (HI_SUCCESS != HI_RTSPSVR_AddMediaStream(s_hRtspServerHandle, &g_stStreamSrc, "12", 1024 * 1024))
    {
        printf("call HI_RTSPSVR_AddMediaStream fail\n");
        HI_RTSPSVR_Stop(s_hRtspServerHandle);
        HI_RTSPSVR_Destroy(s_hRtspServerHandle);
    }

    return HI_SUCCESS;
}

HI_S32 RTSPSERVER_Stop()
{

    if (HI_SUCCESS != HI_RTSPSVR_RemoveMediaStream(s_hRtspServerHandle, "12"))
    {
        printf("call HI_RTSPSVR_RemoveMediaStream fail\n");
    }

    if (HI_SUCCESS != HI_RTSPSVR_Stop(s_hRtspServerHandle))
    {
        printf("call HI_RTSPSVR_Stop fail\n");
    }

    if (HI_SUCCESS != HI_RTSPSVR_Destroy(s_hRtspServerHandle))
    {
        printf("call HI_RTSPSVR_Destroy fail\n");
    }


    return HI_SUCCESS;
}
