#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mount.h>

#include "hi_mw_type.h"
#include "hi_ts_format.h"
#include "avc.h"
#include "aac.h"
#include "jpg.h"
#include "ts.h"
#include "hevc.h"
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define SMAPLE_ROOT_DIR "/tmp/sample/"
#ifdef __HuaweiLite__
#define TS_MUXER_SAMPLE_SRC_H264    SMAPLE_ROOT_DIR"1.h264"
#define TS_MUXER_SAMPLE_SRC_AAC_1   SMAPLE_ROOT_DIR"1.aac"
#define TS_MUXER_SAMPLE_SRC_H265    SMAPLE_ROOT_DIR"2.h265"
#define TS_MUXER_SAMPLE_SRC_AAC_2   SMAPLE_ROOT_DIR"2.aac"
#else
#define TS_MUXER_SAMPLE_SRC_H264    "1.h264"
#define TS_MUXER_SAMPLE_SRC_AAC_1   "1.aac"
#define TS_MUXER_SAMPLE_SRC_H265    "2.h265"
#define TS_MUXER_SAMPLE_SRC_AAC_2   "2.aac"
#endif
static HI_S32 SAMPLE_mount_tmp_dir()
{
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
    return s32Ret;
}

static HI_VOID SAMPLE_umount_tmp_dir()
{
    HI_S32 s32Ret = umount("/tmp");
    if (HI_SUCCESS != s32Ret)
    {
        printf("umount tmp failed, errno:%d \n", errno);
    }
    return;
}

static HI_VOID SAMPLE_make_sampleDir()
{
    (HI_VOID)mkdir(SMAPLE_ROOT_DIR, 0770);
    return;
}

static HI_VOID print_help_info(HI_VOID)
{
    printf("List all testtool command\n");
    printf("h             list all command we provide\n");
    printf("q             quit sample test\n");
    printf("1             start h264 record from read file\n");
    printf("2             start h265 record from read file \n");
    printf("3             write no frame sample \n");
    //printf("4             Demuxer Thm from 1 \n");
}

HI_S32 SAMPLE_TS_WriteNoFrame()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR hTSMuxer = 0;
    HI_MW_PTR hVideo = 0;
    HI_TS_STREAM_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stVideo.enCodecID = HI_TS_CODEC_ID_H265;

    HI_MW_PTR hAudio = 0;
    HI_TS_STREAM_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stAudio.enCodecID = HI_TS_CODEC_ID_AAC;


    HI_TS_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_TS_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_TS_MAX_FILE_NAME, SMAPLE_ROOT_DIR"test_avc_no_frame.ts");
    stMuxerCfg.enConfigType = HI_TS_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024*1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_TS_Create(&stMuxerCfg, &hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Create fail %d \n", s32Ret);
        return s32Ret;
    }

    /*create stream */
    printf("create video\n");
    s32Ret = HI_TS_CreateStream(hTSMuxer, &hVideo, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }
    printf("create audio\n");
    s32Ret = HI_TS_CreateStream(hTSMuxer, &hAudio, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %d \n", s32Ret);
        return s32Ret;
    }
    /*add user define box */
    printf("destroy all stream\n");
    s32Ret = HI_TS_DestroyAllStreams(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_DestroyAllStreams fail %d \n", s32Ret);
    }

    /*destroy muxer*/
    printf("destroy muxer\n");
    s32Ret = HI_TS_Destroy(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
TRACK_DESTROY:
    s32Ret = HI_TS_DestroyAllStreams(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_DestroyAllStreams fail %d \n", s32Ret);
    }
MUXER_DESTROY:
    s32Ret = HI_TS_Destroy(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Destroy fail %d \n", s32Ret);
    }
    return s32Ret;

}

HI_S32 SAMPLE_TS_MuxerAVC()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR hTSMuxer = 0;
    HI_MW_PTR hVideo = 0;
    HI_TS_STREAM_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stVideo.enCodecID = HI_TS_CODEC_ID_H264;

    HI_MW_PTR hAudio = 0;
    HI_TS_STREAM_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stAudio.enCodecID = HI_TS_CODEC_ID_AAC;


    HI_TS_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_TS_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_TS_MAX_FILE_NAME, SMAPLE_ROOT_DIR"test_avc.ts");
    stMuxerCfg.enConfigType = HI_TS_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 20*1024*1024;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024*1024;
    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_TS_Create(&stMuxerCfg, &hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("create muxer fail %d \n", s32Ret);
        return s32Ret;
    }
    /*create stream */
    printf("create video\n");
    s32Ret = HI_TS_CreateStream(hTSMuxer, &hVideo, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %x \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_TS_CreateStream(hTSMuxer, &hAudio, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %x \n", s32Ret);
        goto TRACK_DESTROY;
    }
    printf("create audio hAudio:%p\n", hAudio);


    HI_U8* u8Frame;
    HI_U32 u32FrameLen;
    HI_U8 u8KeyFrame;
    struct timeval tv;

    /*start write frame data*/
    AVC_Handle_S avcHandle;
    AAC_Handle_S aacHandle;

    s32Ret = AVC_Open(&avcHandle, TS_MUXER_SAMPLE_SRC_H264);
    if (HI_SUCCESS != s32Ret)
    {
        printf("AVC_Open %s fail %x \n", TS_MUXER_SAMPLE_SRC_H264, s32Ret);
        goto TRACK_DESTROY;
    }

    s32Ret = AAC_Open(&aacHandle, TS_MUXER_SAMPLE_SRC_AAC_1);
    if (HI_SUCCESS != s32Ret)
    {
        printf("AAC_Open %s fail %x \n", TS_MUXER_SAMPLE_SRC_AAC_1, s32Ret);
        AVC_Close(&avcHandle);
        goto TRACK_DESTROY;
    }

    HI_TS_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_TS_FRAME_DATA_S));
    printf("start read frame \n");
    while (0 == AVC_ReadFrame(&avcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame))
   {
        printf("[%s][%d]\n",__func__,__LINE__);
        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL) u8KeyFrame;
        stFrameData.u64TimeStamp = (HI_U64) tv.tv_sec * 1000000 + tv.tv_usec;
        printf("[%s][%d]\n",__func__,__LINE__);
        s32Ret = HI_TS_WriteFrame(hTSMuxer, hVideo, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
		   printf("HI_TS_WriteFrame fail %d \n", s32Ret);
        }
        printf("[%s][%d]\n",__func__,__LINE__);
        if (0 == AAC_ReadFrame(&aacHandle, &u8Frame, &u32FrameLen))
        {
            stFrameData.pu8DataBuffer = u8Frame;
            stFrameData.u32DataLength = u32FrameLen;
            stFrameData.bKeyFrameFlag = HI_FALSE;
            stFrameData.u64TimeStamp = (HI_U64) tv.tv_sec * 1000000 + tv.tv_usec;
            printf("[%s][%d]\n",__func__,__LINE__);

            s32Ret = HI_TS_WriteFrame(hTSMuxer, hAudio, &stFrameData);
            if (HI_SUCCESS != s32Ret)
            {
                printf("HI_TS_WriteFrame fail %d \n", s32Ret);
            }
        }
        printf("[%s][%d]\n",__func__,__LINE__);
        usleep(33000);

    }
    AVC_Close(&avcHandle);
    AAC_Close(&aacHandle);
    /*write tail and destroy stream*/
    s32Ret = HI_TS_DestroyAllStreams(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_DestroyAllStreams fail %d \n", s32Ret);
    }
    /*destroy muxer*/
    printf("destroy muxer\n");
    s32Ret = HI_TS_Destroy(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Destroy fail  %d \n", s32Ret);
    }
    return s32Ret;
TRACK_DESTROY:
    s32Ret = HI_TS_DestroyAllStreams(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_DestroyAllStreams fail %d \n", s32Ret);
    }
MUXER_DESTROY:
     s32Ret = HI_TS_Destroy(hTSMuxer);
     if (HI_SUCCESS != s32Ret)
     {
        printf("HI_TS_Destroy fail %d \n", s32Ret);
     }
     return s32Ret;
}

HI_S32 SAMPLE_TS_MuxerHEVC()
{
    HI_MW_PTR hTSMuxer = 0;
    HI_MW_PTR hVideo = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TS_STREAM_INFO_S stVideo;
    memset(&stVideo, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stVideo.enCodecID = HI_TS_CODEC_ID_H265;

    HI_MW_PTR hAudio = 0;
    HI_TS_STREAM_INFO_S stAudio;
    memset(&stAudio, 0x00, sizeof(HI_TS_STREAM_INFO_S));
    stAudio.enCodecID = HI_TS_CODEC_ID_AAC;

    HI_TS_CONFIG_S stMuxerCfg;
    memset(&stMuxerCfg, 0x00, sizeof(HI_TS_CONFIG_S));
    snprintf(stMuxerCfg.aszFileName, HI_TS_MAX_FILE_NAME, SMAPLE_ROOT_DIR"test_hevc.ts");
    stMuxerCfg.enConfigType = HI_TS_CONFIG_MUXER;
    stMuxerCfg.stMuxerConfig.u32PreAllocUnit = 20*1024*1024;
    stMuxerCfg.stMuxerConfig.u32VBufSize = 1024*1024;

    /*create muxer*/
    printf("create muxer\n");
    s32Ret = HI_TS_Create(&stMuxerCfg, &hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Create fail %d \n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_TS_CreateStream(hTSMuxer, &hVideo, &stVideo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = HI_TS_CreateStream(hTSMuxer, &hAudio, &stAudio);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_CreateStream fail %d \n", s32Ret);
        goto MUXER_DESTROY;
    }

    uint8_t* u8Frame;
    uint32_t u32FrameLen;
    uint8_t u8KeyFrame;
    struct timeval tv;

    HEVC_Handle_S hevcHandle;
    AAC_Handle_S aacHandle;
    ;
    s32Ret = HEVC_Open(&hevcHandle, TS_MUXER_SAMPLE_SRC_H265);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HEVC_Open %s fail %x \n", TS_MUXER_SAMPLE_SRC_H265, s32Ret);
        goto MUXER_DESTROY;
    }

    s32Ret = AAC_Open(&aacHandle, TS_MUXER_SAMPLE_SRC_AAC_2);
    if (HI_SUCCESS != s32Ret)
    {
        printf("AAC_Open %s fail %x \n", TS_MUXER_SAMPLE_SRC_AAC_2, s32Ret);
        HEVC_Close(&hevcHandle);
        goto MUXER_DESTROY;
    }

    HI_TS_FRAME_DATA_S stFrameData;
    memset(&stFrameData, 0x00, sizeof(HI_TS_FRAME_DATA_S));
    printf("read data\n");
    while (0 == HEVC_ReadFrame(&hevcHandle, &u8Frame, &u32FrameLen, &u8KeyFrame))
    {
        gettimeofday(&tv, NULL);
        stFrameData.pu8DataBuffer = u8Frame;
        stFrameData.u32DataLength = u32FrameLen;
        stFrameData.bKeyFrameFlag = (HI_BOOL) u8KeyFrame;
        stFrameData.u64TimeStamp = (HI_U64) tv.tv_sec * 1000000 + tv.tv_usec;
        //printf("write video frame\n");
        s32Ret = HI_TS_WriteFrame(hTSMuxer, hVideo, &stFrameData);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_TS_WriteFrame fail %d \n", s32Ret);
        }

        if (0 == AAC_ReadFrame(&aacHandle, &u8Frame, &u32FrameLen))
        {
            stFrameData.pu8DataBuffer = u8Frame;
            stFrameData.u32DataLength = u32FrameLen;
            stFrameData.bKeyFrameFlag = HI_FALSE;
            stFrameData.u64TimeStamp = (HI_U64) tv.tv_sec * 1000000 + tv.tv_usec;
            //printf("write audio frame\n");
            s32Ret = HI_TS_WriteFrame(hTSMuxer, hAudio, &stFrameData);
            if (HI_SUCCESS != s32Ret)
            {
                printf("HI_TS_WriteFrame fail %d \n", s32Ret);
            }

        }
        usleep(33000);
    }
    HEVC_Close(&hevcHandle);
    AAC_Close(&aacHandle);
    printf("destroy streams\n");

    s32Ret = HI_TS_DestroyAllStreams(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_DestroyAllStreams fail %d \n", s32Ret);
    }
    printf("destroy muxer\n");
    s32Ret = HI_TS_Destroy(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;

    MUXER_DESTROY: s32Ret = HI_TS_Destroy(hTSMuxer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TS_Destroy fail %d \n", s32Ret);
    }

    return s32Ret;
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
    HI_CHAR InputCmd[32];
    HI_S32 s32InputNumber = 0, i = 0;

    SAMPLE_mount_tmp_dir();
    SAMPLE_make_sampleDir();

    printf("please input 'h' to get help or 'q' to quit!\n");
    while (1)
    {
        memset(InputCmd, 0, 30);

        printf("<input cmd:>");
        if (NULL == fgets((char*) InputCmd, 30, stdin))
        {
            print_help_info();
            continue;
        }

        /*filter backspace key*/
        for (i = 0; InputCmd[i] != '\0' && InputCmd[i] != 0xa; i++)
            ;

        if (InputCmd[i] == 0xa)
        {
            InputCmd[i] = '\0';
        }

        if ((0 == strcmp("q", (char*) InputCmd)) || (0 == strcmp("Q", (char*) InputCmd))
                || (0 == strcmp("quit", (char*) InputCmd))
                || (0 == strcmp("QUIT", (char*) InputCmd)))
        {
            printf("quit the program!\n");
            break;
        }

        if ((0 == strcmp("help", (char*) InputCmd)) || (0 == strcmp("h", (char*) InputCmd)))
        {
            print_help_info();
            continue;
        }

        s32InputNumber = atoi(InputCmd);

        switch (s32InputNumber)
        {

            case 1:/* sample avc muxer  */
                SAMPLE_TS_MuxerAVC();
                break;

            case 2:/* hevc muxer */
                SAMPLE_TS_MuxerHEVC();
                break;

            case 3:/* write no frame */
                SAMPLE_TS_WriteNoFrame();
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
