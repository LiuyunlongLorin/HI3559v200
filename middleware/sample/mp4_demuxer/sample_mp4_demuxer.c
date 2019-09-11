#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "hi_mw_type.h"
#include "hi_mp4_format.h"
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#define ATOM_BUF_LEN (64*1024)

static HI_U64 getCurRelativeTime()
{
    struct timespec ts = {0, 0};
    HI_U64 curT = 0;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    curT = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    return curT;
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
    if(argc==1)
    {
        printf("no filename\n");
        return -1;
    }
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pDemuxer = 0;
    HI_MP4_ATOM_INFO_S stAtomInfo;
    HI_MW_PTR pTrack = 0;
    HI_MP4_ATOM_INFO_S stAtomInfo0;
    HI_U32 u32BoxLen = ATOM_BUF_LEN;
    HI_U32 i = 0;
    HI_MP4_TRACK_INFO_S stTrackInfo;

    HI_U64 curT1 = getCurRelativeTime();
    HI_MP4_CONFIG_S stDemuxerCfg;
    memset(&stDemuxerCfg, 0x00, sizeof(HI_MP4_CONFIG_S));

    memset(&stTrackInfo, 0x00, sizeof(HI_MP4_TRACK_INFO_S));
    snprintf(stDemuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, argv[1]);

    stDemuxerCfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
    stDemuxerCfg.stDemuxerConfig.u32VBufSize = 1024 * 1024;
    memset(&stAtomInfo, 0x00, sizeof(HI_MP4_ATOM_INFO_S));
    memset(&stAtomInfo0, 0x00, sizeof(HI_MP4_ATOM_INFO_S));

    s32Ret = HI_MP4_Create(&pDemuxer, &stDemuxerCfg);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MP4_Create failed\n");
        return HI_FAILURE;
    }
    HI_U64 curT2 = getCurRelativeTime();
    printf("use time: %d Ms\n", (HI_U32)(curT2 - curT1));

    HI_MP4_FILE_INFO_S stfileinfo;
    s32Ret = HI_MP4_GetFileInfo(pDemuxer, &stfileinfo);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MP4_GetFileInfo failed\n");
        goto DES_MP4;
    }
    else
    {
        printf("file duration: %u ms trak[0] handlr name: %s bco64Flag: %d trackNum: %d\n",
               stfileinfo.u32Duration, stfileinfo.aszHandlrName, stfileinfo.bCo64Flag,
               stfileinfo.u32TrackNum);
    }

    for (; i < stfileinfo.u32TrackNum; i++)
    {
        s32Ret = HI_MP4_GetTrackByIndex(pDemuxer, i, &stTrackInfo,&pTrack);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MP4_GetAtom index: %d failed\n", i);
            goto DES_MP4;
        }
        else
        {
            HI_MP4_TRACK_INFO_S* pTrackInfo = &stTrackInfo;
            printf("track index %d type: %d timescale: %d hdlrname: %s\n", i, pTrackInfo->enTrackType,
                   pTrackInfo->u32TimeScale, pTrackInfo->aszHdlrName);
            if (pTrackInfo->enTrackType == HI_MP4_STREAM_VIDEO)
            {
                printf("video width: %d height: %d enCodecID: %d framerate %d \n", pTrackInfo->stVideoInfo.u32Width,
                       pTrackInfo->stVideoInfo.u32Height, pTrackInfo->stVideoInfo.enCodecID,pTrackInfo->stVideoInfo.u32FrameRate);
            }
            else if (pTrackInfo->enTrackType == HI_MP4_STREAM_AUDIO)
            {
                printf("audio u32SampleRate: %d u32Channels: %d Bitwidth: %d codecId: %d\n", pTrackInfo->stAudioInfo.u32SampleRate,
                       pTrackInfo->stAudioInfo.u32Channels, pTrackInfo->stAudioInfo.u16SampleSize, pTrackInfo->stAudioInfo.enCodecID);
            }
            else
            {
                printf("data width: %d height: %d framerate %d  enCodecID: %d \n", pTrackInfo->stDataInfo.u32Width,
                       pTrackInfo->stDataInfo.u32Height, pTrackInfo->stDataInfo.u32FrameRate,pTrackInfo->stDataInfo.enCodecID);
            }
        }
    }



    s32Ret = HI_MP4_GetAtom(pDemuxer, "/moov/trak[0]/mdia/minf/stbl/stco", &stAtomInfo);
    if (s32Ret != HI_SUCCESS && s32Ret != HI_ERR_MP4_ATOM_LEN)
    {
        printf("HI_MP4_GetAtom failed\n");
        goto DES_MP4;
    }
    else
    {
        printf("read stco atom content len: %u \n", stAtomInfo.u32DataLen);
    }

    stAtomInfo.pu8DataBuf = (HI_U8*)malloc(stAtomInfo.u32DataLen);
    if (NULL == stAtomInfo.pu8DataBuf)
    {
        printf("HI_MP4_GetAtom malloc stAtomInfo.pu8DataBuf failed\n");
        goto DES_MP4;
    }
    memset(stAtomInfo.pu8DataBuf, 0x00, stAtomInfo.u32DataLen);
    s32Ret = HI_MP4_GetAtom(pDemuxer, "/moov/trak[0]/mdia/minf/stbl/stco", &stAtomInfo);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MP4_GetAtom failed\n");
        goto FREE_ATOM;
    }

    printf("HI_MP4_GetAtom stco  size %d \n",  stAtomInfo.u32DataLen);
    stAtomInfo0.u32DataLen = 16;
    stAtomInfo0.pu8DataBuf = (HI_U8*)malloc(stAtomInfo0.u32DataLen);
    if (NULL == stAtomInfo0.pu8DataBuf)
    {
        printf("HI_MP4_GetAtom malloc stAtomInfo0.pu8DataBuf failed\n");
        goto FREE_ATOM;
    }
    /*
    s32Ret = HI_MP4_GetAtom(pDemuxer, "/moov/dddd", &stAtomInfo0);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MP4_GetAtom0 failed\n");

    }
    printf("HI_MP4_GetAtom0 type %s size %d \n", stAtomInfo0.aszType, stAtomInfo0.u32DataLen);*/

FREE_ATOM0:
    if (stAtomInfo0.pu8DataBuf)
    {
        free(stAtomInfo0.pu8DataBuf);
        stAtomInfo0.pu8DataBuf = NULL;
    }
FREE_ATOM:
    if (stAtomInfo.pu8DataBuf)
    {
        free(stAtomInfo.pu8DataBuf);
        stAtomInfo.pu8DataBuf = NULL;
    }
DES_MP4:
    s32Ret = HI_MP4_Destroy(pDemuxer, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MP4_Destroy failed\n");
        goto FREE_ATOM0;
    }

    return s32Ret;
}
