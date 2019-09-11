#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/prctl.h>

#include "hi_liteplayer.h"
#include "hi_mw_media_intf.h"

#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif


#ifndef __HuaweiLite__
#define FFMPEG_DEMUX_SO "../../../component/liteplayer/lib/libffmpeg_demuxer.so"
#define AAC_DECODER "/app/sharefs/libhiaacdec.so"

#else
#define FFMPEG_DEMUX_SO "/home/libffmpeg_demuxer.so"
#define AAC_DECODER "/home/libhiaacdec.so"

#endif
#define BOARD_FFMPEG_DEMUX_SO "/usr/lib/libffmpeg_demuxer.so"
//#define BOARD_MPF_DEMUX_SO   "/usr/lib/libmpf_demuxer.so"


static HI_LITEPLAYER_STATE_E g_enState = HI_LITEPLAYER_STATE_BUTT;
static HI_MW_PTR g_pPlayer = NULL;
static int g_isHasError = 0;
static char* in_filename = NULL;

static HI_HANDLE g_hVpssGrpHandle = 0;
static HI_HANDLE g_hVpssGrpChnHandle = 0;
static HI_HANDLE g_hDispHandle = 0;
static HI_HANDLE g_hDispWndHandle = 0;

static int DISPLAY_WIDTH  = 1920;
static int DISPLAY_HEIGHT = 1080;

#ifndef __HuaweiLite__
static HI_LITEPLAYER_VOUT_TYPE_E g_enVoutType = HI_LITEPLAYER_VOUT_TYPE_VPSS;
static HI_HANDLE g_hAoTrackHandle = 0;
static HI_HANDLE g_hAoDevHandle = 2;
#else
static HI_LITEPLAYER_VOUT_TYPE_E g_enVoutType = HI_LITEPLAYER_VOUT_TYPE_VO;
static HI_HANDLE g_hAoTrackHandle = 0;
static HI_HANDLE g_hAoDevHandle = 0;
#endif

#define DOFUNC_STR_NORET( func , str)  \
    do { \
        HI_S32 s32Ret = 0; \
        s32Ret = func; \
        if(s32Ret != HI_SUCCESS) \
        { \
            printf("[liteplayer_sample][%s:%d] ret:%d,  %s \n", __FILE__, __LINE__, s32Ret,str); \
            return NULL;\
        } \
    } while(0)

#define DOFUNC_STR_RET( func , str)  \
    do { \
        HI_S32 s32Ret = 0; \
        s32Ret = func; \
        if(s32Ret != HI_SUCCESS ) \
        { \
            printf("[liteplayer_sample][%s:%d] ret:%d,  %s \n", __FILE__, __LINE__, s32Ret,str); \
            return HI_FAILURE;\
        } \
    } while(0)


void sample_evect_cb(HI_MW_PTR pPlayer, HI_LITEPLAYER_EVENT_E enEvent, void* pData)
{
    HI_LITEPLAYER_ERROR_E subErr = HI_LITEPLAYER_ERROR_BUTT;

    if (pPlayer == 0)
    {
        printf("the handle is error\n");
        return;
    }

    switch (enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
            g_enState = *(HI_LITEPLAYER_STATE_E*)pData;
            printf("@@player: %p, state:%d\n", pPlayer, g_enState);
            break;

        case HI_LITEPLAYER_EVENT_EOF:
            printf("@@\nend of file\n");
            break;
        case HI_LITEPLAYER_EVENT_SOF:
            printf("@@start of file\n");
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
            g_isHasError = 1;
            subErr = *(HI_LITEPLAYER_ERROR_E*)pData;
            printf("@@error: %d\n", subErr);
            break;

        case HI_LITEPLAYER_EVENT_PROGRESS:
            printf("@@player: %p, progress: %d\n", pPlayer, *(HI_U32*)pData);
            break;

        case HI_LITEPLAYER_EVENT_SEEK_END:
            printf("@@seek action end, time is %lld\n",  *(HI_S64*)pData);
            break;

        default:
            break;
    }

}

static HI_S32 tplay_speed_convert(HI_S32 s32Speed, HI_LITEPLAYER_PLAY_SPEED_E* penTSpeed)
{
    switch (s32Speed)
    {
        case HI_LITEPLAYER_PLAY_SPEED_2X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_2X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_4X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_4X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_8X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_8X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_16X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_16X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_32X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_32X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_64X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_64X_FAST;
            break;

        case HI_LITEPLAYER_PLAY_SPEED_128X_FAST:
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_128X_FAST;
            break;

        default:
            printf("unsupported tplay speed : %d\n", s32Speed);
            *penTSpeed = HI_LITEPLAYER_PLAY_SPEED_BUTT;
            break;
    }

    if (*penTSpeed == HI_LITEPLAYER_PLAY_SPEED_BUTT)
    {
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 HI_LITEPLAYER_OnVbPoolGet(HI_VOID* pPlayer, HI_U32 u32FrameBufSize, HI_U32 u32FrameBufCnt, HI_U32* pVbPoolId, HI_VOID* pPriv)
{
    HI_S32 s32Ret = HI_SUCCESS;

    printf("on req vb size: %d cnt: %d\n", u32FrameBufSize, u32FrameBufCnt);
    s32Ret = HI_MW_VB_Alloc(pVbPoolId, u32FrameBufSize, u32FrameBufCnt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("PLAYER_Vb_Alloc failed \n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_LITEPLAYER_OnVbPoolPut(HI_VOID* pPlayer, HI_U32 u32VbPoolId, HI_VOID* pPriv)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MW_VB_Free(u32VbPoolId);
    if (s32Ret != HI_SUCCESS)
    {
        printf("PLAYER_Vb_Free failed \n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


static HI_S32 player_unregisterDemuxer(const HI_CHAR* pszMainFile, const HI_CHAR* pszBakFile)
{
    struct stat stFileState = {0};
    HI_S32 s32Ret = HI_SUCCESS;

    memset(&stFileState, 0 , sizeof(stFileState));

    s32Ret = lstat(pszMainFile, &stFileState);

    if (s32Ret != HI_SUCCESS)
    {
        s32Ret = lstat(pszBakFile, &stFileState);

        if (s32Ret != HI_SUCCESS)
        {
            printf("lstat %s failed, please check the file \n", pszBakFile);
        }
        else
        {
            s32Ret = HI_LITEPLAYER_UnRegisterDemuxer(pszBakFile);
            if (s32Ret != HI_SUCCESS)
            {
                printf(" unreg libmpfdemux_demuxer.so exec failed\n");
            }
        }

    }
    else
    {
        s32Ret = HI_LITEPLAYER_UnRegisterDemuxer(pszMainFile);
        if (s32Ret != HI_SUCCESS)
        {
            printf(" unreg libmpfdemux_demuxer.so exec failed\n");
        }
    }

    return s32Ret;
}

static HI_S32 player_registerDemuxer(const HI_CHAR* pszMainFile, const HI_CHAR* pszBakFile)
{
    struct stat stFileState = {0};
    HI_S32 s32Ret = HI_SUCCESS;

    memset(&stFileState, 0 , sizeof(stFileState));

    s32Ret = lstat(pszMainFile, &stFileState);

    if (s32Ret != HI_SUCCESS)
    {
        s32Ret = lstat(pszBakFile, &stFileState);
        if (s32Ret != HI_SUCCESS)
        {
            printf("lstat %s failed, please check the file exist\n", pszMainFile);
        }
        else
        {
            s32Ret = HI_LITEPLAYER_RegisterDemuxer(pszBakFile, NULL);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_LITEPLAYER_RegisterDemuxer libffmpeg_demuxer.so  failed\n");
            }
        }

    }
    else
    {
        s32Ret = HI_LITEPLAYER_RegisterDemuxer(pszMainFile, NULL);
        if (s32Ret != HI_SUCCESS)
        {
            printf(" reg %s exec failed\n",pszMainFile);
        }
    }
    return s32Ret;
}

static void sample_play(void)
{
    int ret = 0;
    HI_LITEPLAYER_ATTR_S stMediaAttr = {0};
    HI_FORMAT_FILE_INFO_S stFormatFileInfo;
    memset(&stFormatFileInfo, 0 , sizeof(stFormatFileInfo));

    ret = HI_LITEPLAYER_SetDataSource(g_pPlayer, in_filename);
    if (ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_SetDataSource  exec failed \n");
        goto EXIT_SAMPLE_PLAY;
    }

    ret = HI_LITEPLAYER_Prepare(g_pPlayer);
    if (ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Prepare  exec failed \n");
        goto EXIT_SAMPLE_PLAY;
    }

    ret = HI_LITEPLAYER_GetFileInfo(g_pPlayer, &stFormatFileInfo);
    if (ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_GetFileInfo  exec failed \n");
        goto EXIT_SAMPLE_PLAY;
    }

    for (int i = 0; i < HI_DEMUXER_RESOLUTION_CNT; i++) {
        printf("video[%d],w=%d,h=%d,index=%d \n",i,
                stFormatFileInfo.stSteamResolution[i].u32Width,stFormatFileInfo.stSteamResolution[i].u32Height,
                stFormatFileInfo.stSteamResolution[i].s32VideoStreamIndex);
    }

    printf("stFormatFileInfo.s32UsedVideoStreamIndex  %d\n", stFormatFileInfo.s32UsedVideoStreamIndex);
    stMediaAttr.s32VidStreamId = stFormatFileInfo.s32UsedVideoStreamIndex;
    stMediaAttr.s32AudStreamId = stFormatFileInfo.s32UsedAudioStreamIndex;
    ret = HI_LITEPLAYER_SetMedia(g_pPlayer, &stMediaAttr);
    if (ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_SetMedia  exec failed\n");
        goto EXIT_SAMPLE_PLAY;
    }

    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    if (stFormatFileInfo.s32UsedVideoStreamIndex != HI_DEMUXER_NO_MEDIA_STREAM)
    {
        u32Width = stFormatFileInfo.stSteamResolution[stFormatFileInfo.s32UsedVideoStreamIndex].u32Width;
        u32Height = stFormatFileInfo.stSteamResolution[stFormatFileInfo.s32UsedVideoStreamIndex].u32Height;
    }
    else if (stFormatFileInfo.s32UsedVideoStreamIndex == HI_DEMUXER_NO_MEDIA_STREAM)
    {
        u32Width = 1920;
        u32Height = 1080;
    }

    ret = HI_LITEPLAYER_Play(g_pPlayer);
    if (ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Play exec failed %x\n", ret);
    }

EXIT_SAMPLE_PLAY:
    return;
}

#ifdef __HuaweiLite__
int app_main(int argc , char** argv)
#else
int main(int argc , char** argv)
#endif
{
    if (argc < 2)
    {
        printf("usage: %s filename  (or vpss channel index)\n", argv[0]);
        return -1;
    }
    if (argc >= 3) {
        HI_HANDLE  index = 0;
        index = atoi(argv[2]);
        if ((index < 5) && (index >= 0)) {
            g_hVpssGrpChnHandle = index;
        }
    }
#ifdef __HuaweiLite__
    /*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
    /*init cpp running context on liteos end*/
#endif

    in_filename  = argv[1];
    HI_CHAR cmd[64];
    HI_S32 s32Ret = 0;
    HI_LITEPLAYER_VOUT_OPT_S stVoutOpt;
    HI_LITEPLAYER_AOUT_OPT_S stAoutOpt;
    HI_LITEPLAYER_EXTRA_FILESURFIX_S stExtraSufixes;
    HI_LITEPLAYER_PARAM_S stPlayerParam;
    memset(&stAoutOpt, 0x00, sizeof(stAoutOpt));
    memset(&stPlayerParam, 0x00, sizeof(stPlayerParam));
    memset(&stVoutOpt, 0x00, sizeof(stVoutOpt));

    s32Ret = HI_MW_Sys_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MW_Sys_Init failed Ret: %x\n", s32Ret);
        return HI_FAILURE;
    }
    HI_MW_AO_GetDevHandle(&g_hAoDevHandle);

    HI_MW_EnableDebug(HI_TRUE);

    stExtraSufixes.u32SurfixCnt = 1;
    strncpy(stExtraSufixes.aszFileSurfix[0], "sec", HI_LITEPLAYER_MAX_FILE_SURFIX_LEN);
    s32Ret = HI_LITEPLAYER_Init();
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Init exec failed \n");
        return -1;
    }

    s32Ret = player_registerDemuxer(FFMPEG_DEMUX_SO, BOARD_FFMPEG_DEMUX_SO);
    if (s32Ret != HI_SUCCESS)
    {
        printf("player_registerDemuxer failed \n");
        goto Deinit;
    }

    s32Ret = HI_LITEPLAYER_RegisterAudioDecoder(AAC_DECODER);
    if (s32Ret != HI_SUCCESS)
    {
        printf(" reg libhiaacdec.so exec failed %x\n", s32Ret);
        goto DeregMPF;
    }

    stPlayerParam.u32PlayPosNotifyIntervalMs = 300;
    s32Ret = HI_LITEPLAYER_Create(&g_pPlayer, &stPlayerParam);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Create  exec failed \n");
        goto DeregADec;
    }

    stVoutOpt.enVoutType = g_enVoutType;
    if (g_enVoutType == HI_LITEPLAYER_VOUT_TYPE_VO)
    {
        g_hVpssGrpHandle = HI_MW_INVALID_HANDLE;
        g_hVpssGrpChnHandle = HI_MW_INVALID_HANDLE;
        stVoutOpt.hModHdl = g_hDispHandle;
        stVoutOpt.hChnHdl = g_hDispWndHandle;
    }
    else if (g_enVoutType == HI_LITEPLAYER_VOUT_TYPE_VPSS)
    {
        stVoutOpt.hModHdl = g_hVpssGrpHandle;
        stVoutOpt.hChnHdl = g_hVpssGrpChnHandle;
    }
    else
    {
        printf("invalid  g_enVoutType: %d \n", g_enVoutType);
    }

    s32Ret = HI_LITEPLAYER_SetVOHandle(g_pPlayer, &stVoutOpt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_SetVOHandle  exec failed \n");
        goto Stop;
    }

    stAoutOpt.hAudTrackHdl = g_hAoTrackHandle;
    stAoutOpt.hAudDevHdl = g_hAoDevHandle;

    s32Ret = HI_LITEPLAYER_SetAOHandle(g_pPlayer, &stAoutOpt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_SetAOHandle  exec failed \n");
        goto Stop;
    }

    s32Ret = HI_LITEPLAYER_RegCallback(g_pPlayer, sample_evect_cb);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_RegCallback exec failed \n");
        goto Stop;
    }

    s32Ret = HI_MW_VO_Open(g_hVpssGrpHandle, g_hVpssGrpChnHandle, g_hDispHandle, g_hDispWndHandle, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    printf("g_hVpssGrpChnHandle = %d \n",g_hVpssGrpChnHandle);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MW_VO_Open failed Ret: %x\n", s32Ret);
    }

    s32Ret = HI_MW_AO_Open(g_hAoDevHandle, 48000, 1);
    if (s32Ret != HI_SUCCESS)
    {
        printf("PLAYER_AO_Open fail!\n");
    }

    sample_play();

    printf("Input CMD: quit, pause(pu), replay(re), play(pl), seek(sk), ff [speed] , bw [speed]\n");

    while (fgets(cmd, 10, stdin))
    {
        cmd[10] = '\0';

        if (strncmp(cmd, "quit", 4) == 0 )
        {
            break;
        }
        else if (strncmp(cmd, "pause", 5) == 0 || strncmp(cmd, "pu", 2) == 0 )
        {
            s32Ret = HI_LITEPLAYER_Pause(g_pPlayer);
            printf("HI_LITEPLAYER_Pause() ret=0x%x\n", s32Ret);
        }
        else if (strncmp(cmd, "replay", 4) == 0 || strncmp(cmd, "re", 2) == 0)
        {
            s32Ret = HI_LITEPLAYER_Stop(g_pPlayer);
            printf("HI_LITEPLAYER_Stop() ret=0x%x\n", s32Ret);
//            sleep(3);
            sample_play();
        }
        else if (strncmp(cmd, "play", 4) == 0 || strncmp(cmd, "pl", 2) == 0)
        {
            s32Ret = HI_LITEPLAYER_Play(g_pPlayer);
            printf("HI_LITEPLAYER_Play() ret=0x%x\n", s32Ret);
        }
        else if (strncmp(cmd, "seek", 4) == 0 || strncmp(cmd, "sk", 2) == 0)
        {
            HI_S64 s64TimeInMs = 0;
            if (1 != sscanf(cmd, "sk %lld", &s64TimeInMs))
            {
                if (1 != sscanf(cmd, "seek %lld", &s64TimeInMs))
                {
                    printf("\e[31m ERR: not input seek position, seek to 0 \e[0m \n");
                    s64TimeInMs = 0;
                }
            }
            s32Ret = HI_LITEPLAYER_Seek(g_pPlayer, s64TimeInMs);
            printf("HI_LITEPLAYER_Seek() ret=0x%x\n", s32Ret);
        }
        else if (strncmp(cmd, "ff", 2) == 0)
        {
            HI_S32 s32Speed = 0;
            if (1 != sscanf(cmd, "ff %d", &s32Speed))
            {
                printf("\e[31m ERR: not input tplay speed, example: ff 4! \e[0m \n");
                continue;
            }
            printf("forward speed = %d \n", s32Speed);

            HI_LITEPLAYER_TPLAY_ATTR_S stTPlayAttr;
            stTPlayAttr.enDirection = HI_LITEPLAYER_TPLAY_FORWARD;
            s32Ret = tplay_speed_convert(s32Speed * HI_LITEPLAYER_PLAY_SPEED_BASE, &stTPlayAttr.enSpeed);
            if (s32Ret != HI_SUCCESS)
            {
                continue;
            }

            s32Ret = HI_LITEPLAYER_TPlay(g_pPlayer, &stTPlayAttr);
            printf("HI_LITEPLAYER_TPlay() ret=0x%x\n", s32Ret);
        }
        else if (strncmp(cmd, "bw", 2) == 0)
        {
            HI_S32 s32Speed = 0;

            if (1 != sscanf(cmd, "bw %d", &s32Speed))
            {
                printf("\e[31m ERR: not input tplay speed, example: bw 4! \e[0m \n");
                continue;
            }
            printf("backward speed = %d \n", s32Speed);

            HI_LITEPLAYER_TPLAY_ATTR_S stTPlayAttr;
            stTPlayAttr.enDirection = HI_LITEPLAYER_TPLAY_BACKWARD;
            s32Ret = tplay_speed_convert(s32Speed * HI_LITEPLAYER_PLAY_SPEED_BASE, &stTPlayAttr.enSpeed);
            if (s32Ret != HI_SUCCESS)
            {
                continue;
            }

            s32Ret = HI_LITEPLAYER_TPlay(g_pPlayer, &stTPlayAttr);
            printf("HI_LITEPLAYER_TPlay() ret=0x%x\n", s32Ret);
        }
        else
        {
            printf("Input CMD: quit, pause(pu), replay(re), play(pl), seek(sk), ff [speed] , bw [speed]\n");
        }
    }

Stop:
    s32Ret = HI_LITEPLAYER_Stop(g_pPlayer);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Stop  exec failed \n");
    }

    s32Ret = HI_LITEPLAYER_Destroy(g_pPlayer);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_Destroy  exec failed \n");
    }

DeregADec:
    s32Ret = HI_LITEPLAYER_UnRegisterAudioDecoder(AAC_DECODER);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_UnRegisterAudioDecoder failed\n");
    }
DeregMPF:

    player_unregisterDemuxer(FFMPEG_DEMUX_SO, BOARD_FFMPEG_DEMUX_SO);

Deinit:
    s32Ret = HI_LITEPLAYER_DeInit();
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_LITEPLAYER_DeInit  exec failed \n");
    }

    HI_MW_VO_Close(g_hVpssGrpHandle, g_hVpssGrpChnHandle, g_hDispHandle,  g_hDispWndHandle);
    HI_MW_AO_Close(g_hAoDevHandle);

    s32Ret = HI_MW_Sys_Deinit();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MW_Sys_Deinit failed Ret: %x\n", s32Ret);
    }

    return 0;
}

#ifdef __HuaweiLite__
void __pthread_key_create()
{
    printf("__pthread_key_create not support\n");
}

void __register_atfork()
{
    printf("__register_atfork not support\n");
}

FILE* tmpfile()
{
    printf("tmpfile not support\n");
    return NULL;
}
#endif
