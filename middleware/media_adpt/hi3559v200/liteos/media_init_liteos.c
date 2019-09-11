#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi_audio.h"
#include "hi_comm_aio.h"
#include "acodec.h"
#include "hi_type.h"
#include "mpi_vpss.h"
#include "mpi_vo.h"
#include "mpi_hdmi.h"
#include "hi_buffer.h"
#include "fcntl.h"
#include <sys/ioctl.h>
#include "hi_comm_vb.h"
#include "mpi_sys.h"
#include "mpi_vb.h"

#include "hi_mapi_sys.h"
#include "common_ipcmsg_server.h"

#define AUDIO_LEFT_CHN (0)
#define AUDIO_RIGHT_CHN (1)
#define AUDIO_SYS_CHN (2)

#define ACODEC_FILE     "/dev/acodec"
#define VPSS_MAX_WIDTH (3840)
#define VPSS_MAX_HEIGHT (2160)
#define VO_MAX_WIDTH (1920)
#define VO_MAX_HEIGHT (1080)

static DYNAMIC_RANGE_E s_genDynRange = DYNAMIC_RANGE_SDR8;

static HI_S32 PLAYER_AUDIO_CODEC_CFG(AUDIO_SAMPLE_RATE_E enSample)
{
    HI_S32 fdAcodec = -1;
    HI_S32 s32Ret = HI_SUCCESS;
    ACODEC_FS_E i2s_fs_sel = (ACODEC_FS_E)0;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        printf( "can't open Acodec,%s\n", ACODEC_FILE);
        return HI_FAILURE;
    }

    if (ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
        printf("Reset audio codec error\n");
    }

    switch (enSample)
    {
        case AUDIO_SAMPLE_RATE_8000:
            i2s_fs_sel = ACODEC_FS_8000;
            break;

        case AUDIO_SAMPLE_RATE_16000:
            i2s_fs_sel = ACODEC_FS_16000;
            break;

        case AUDIO_SAMPLE_RATE_48000:
            i2s_fs_sel = ACODEC_FS_48000;
            break;

        default:
            printf( "not support enSample:%d\n", enSample);
            s32Ret = HI_FAILURE;
            break;
    }

    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
    {
        printf( "set acodec sample rate failed\n");
        s32Ret = HI_FAILURE;
    }

    close(fdAcodec);
    return s32Ret;
}

static AUDIO_SAMPLE_RATE_E PLAYER_SampleRate_Convert(HI_S32 s32SampleRate)
{
    AUDIO_SAMPLE_RATE_E enSampleRate;

    switch (s32SampleRate)
    {
        case 8000:
            enSampleRate = AUDIO_SAMPLE_RATE_8000;
            break;

        case 16000:
            enSampleRate = AUDIO_SAMPLE_RATE_16000;
            break;

        case 24000:
            enSampleRate = AUDIO_SAMPLE_RATE_24000;
            break;

        case 32000:
            enSampleRate = AUDIO_SAMPLE_RATE_32000;
            break;

        case 44100:
            enSampleRate = AUDIO_SAMPLE_RATE_44100;
            break;

        case 48000:
            enSampleRate = AUDIO_SAMPLE_RATE_48000;
            break;

        default:
            enSampleRate = AUDIO_SAMPLE_RATE_BUTT;
            printf("unsupported audio samplerate: %d\n", s32SampleRate);
            break;
    }

    return enSampleRate;
}

static HI_S32 PLAYER_AO_OPEN(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, AUDIO_SOUND_MODE_E enSoundMode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    AUDIO_SAMPLE_RATE_E enSampleRate = PLAYER_SampleRate_Convert(s32SampleRate);

    if (enSampleRate == AUDIO_SAMPLE_RATE_BUTT)
    {
        printf( "unsupported audio samplerate: %d\n", s32SampleRate);
        return HI_FAILURE;
    }
    //comment for mapi media init will open.
    //s32Ret = PLAYER_AUDIO_CODEC_CFG(enSampleRate);
    //if (HI_SUCCESS != s32Ret)
    //{
    //    printf( "PLAYER_AUDIO_CODEC_CFG err\n");
    //}

    AIO_ATTR_S stAioAttr;
    memset(&stAioAttr, 0x00, sizeof(AIO_ATTR_S));


    stAioAttr.enSamplerate   = enSampleRate;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = enSoundMode;
    stAioAttr.u32EXFlag = 0;
    stAioAttr.u32FrmNum = 2;
    stAioAttr.u32PtNumPerFrm = 1024;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 0;
    stAioAttr.enI2sType = AIO_I2STYPE_INNERHDMI;

    printf( "sampleRate: %d  soundmode: %d\n", s32SampleRate, enSoundMode);

    s32Ret = HI_MPI_AO_SetPubAttr(hAoHdl, &stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_SetPubAttr err %x \n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AO_Enable(hAoHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_Enable err %x \n", s32Ret);
        return HI_FAILURE;
    }

    printf( "HI_MPI_AO_Enable OK...\n");
    printf( "HI_MPI_AO_EnableChn AUDIO_LEFT_CHN \n");

    s32Ret = HI_MPI_AO_EnableChn(hAoHdl, AUDIO_LEFT_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_EnableChn AUDIO_LEFT_CHN err %x \n", s32Ret);
        (HI_VOID)HI_MPI_AO_Disable(hAoHdl);
        return HI_FAILURE;
    }

    printf( "HI_MPI_AO_EnableChn AUDIO_SYS_CHN \n");

    s32Ret = HI_MPI_AO_EnableChn(hAoHdl, AUDIO_SYS_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_EnableChn AUDIO_SYS_CHN err %x \n", s32Ret);
        (HI_VOID)HI_MPI_AO_Disable(hAoHdl);
        return HI_FAILURE;
    }

    HI_S32 s32VolumeDb = 6;
    s32Ret = HI_MPI_AO_SetVolume(hAoHdl, s32VolumeDb);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_SetVolume err\n");
    }

    printf( "HI_MPI_AO_EnableChn OK...\n");
    return s32Ret;

}

HI_S32 PLAYER_AO_CLOSE(HI_HANDLE hAoHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_AO_DisableChn(hAoHdl, AUDIO_LEFT_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_DisableChn AUDIO_LEFT_CHN err %x \n", s32Ret);
    }

    s32Ret = HI_MPI_AO_DisableChn(hAoHdl, AUDIO_RIGHT_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_DisableChn AUDIO_RIGHT_CHN err %x \n", s32Ret);
    }
    s32Ret = HI_MPI_AO_DisableChn(hAoHdl, AUDIO_SYS_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_DisableChn AUDIO_SYS_CHN err %x \n", s32Ret);
    }

    s32Ret = HI_MPI_AO_Disable(hAoHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_Disable return ERR, %x\n", s32Ret);
        return HI_FAILURE;
    }

    printf( "HI_MPI_AO_Disable OK...\n");
    return HI_SUCCESS;
}

static AUDIO_SOUND_MODE_E PLAYER_SoundMode_Convert(HI_S32 s32Channel)
{
    AUDIO_SOUND_MODE_E enSoundMode;

    switch (s32Channel)
    {
        case 1:
            enSoundMode = AUDIO_SOUND_MODE_MONO;
            break;

        case 2:
            enSoundMode = AUDIO_SOUND_MODE_STEREO;
            break;

        default:
            enSoundMode = AUDIO_SOUND_MODE_BUTT;
            printf("unsupported audio chnNum: %d\n", s32Channel);
            break;
    }

    return enSoundMode;
}

HI_S32 AUDIO_StartHdmi(AUDIO_SAMPLE_RATE_E enSamplerate)
{
    HI_S32 s32Ret;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_ID_E enHdmi = HI_HDMI_ID_0;

    //TODO: must insure hdmi device already started by video or reference
    s32Ret = HI_MPI_HDMI_Stop(enHdmi);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_HDMI_Stop Ret: %08x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_HDMI_GetAttr(enHdmi, &stHdmiAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_HDMI_GetAttr Ret: %08x\n", s32Ret);
        (HI_VOID)HI_MPI_HDMI_Start(enHdmi);
        return HI_FAILURE;
    }

    stHdmiAttr.bEnableAudio = HI_TRUE;        /**< if enable audio */
    stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S; /**< source of HDMI audio, HI_HDMI_SND_INTERFACE_I2S suggested.the parameter must be consistent with the input of AO*/
    stHdmiAttr.enSampleRate = enSamplerate;        /**< sampling rate of PCM audio,the parameter must be consistent with the input of AO */
    stHdmiAttr.u8DownSampleParm = HI_FALSE;    /**< parameter of downsampling  rate of PCM audio, default :0 */

    stHdmiAttr.enBitDepth = 8 * (AUDIO_BIT_WIDTH_16 + 1); /**< bitwidth of audio,default :16,the parameter must be consistent with the config of AO */
    stHdmiAttr.u8I2SCtlVbit = 0;        /**< reserved, should be 0, I2S control (0x7A:0x1D) */

    stHdmiAttr.bEnableAviInfoFrame = HI_TRUE; /**< if enable  AVI InfoFrame*/
    stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;; /**< if enable AUDIO InfoFrame*/
    stHdmiAttr.bIsMultiChannel       = HI_FALSE;

    s32Ret = HI_MPI_HDMI_SetAttr(enHdmi, &stHdmiAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_SetAttr Ret: %08x\n", s32Ret);
        (HI_VOID)HI_MPI_HDMI_Start(enHdmi);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_HDMI_Start(enHdmi);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_Start Ret: %08x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VO_HdmiStartByDyRg(DYNAMIC_RANGE_E enDyRg)
{
    HI_HDMI_ATTR_S          stAttr;
    HI_HDMI_ID_E            enHdmiId    = HI_HDMI_ID_0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_HDMI_VIDEO_FMT_E enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;

    s32Ret = HI_MPI_HDMI_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_Start Ret: %08x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_HDMI_Open(enHdmiId);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_Open Ret: %08x\n", s32Ret);
        HI_MPI_HDMI_DeInit();
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_HDMI_GetAttr(enHdmiId, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_GetAttr Ret: %08x\n", s32Ret);
        HI_MPI_HDMI_Close(enHdmiId);
        HI_MPI_HDMI_DeInit();
        return HI_FAILURE;
    }

    stAttr.bEnableHdmi           = HI_TRUE;
    stAttr.bEnableVideo          = HI_TRUE;
    stAttr.enVideoFmt            = enVideoFmt;
    stAttr.enVidOutMode          = HI_HDMI_VIDEO_MODE_YCBCR444;

    switch (enDyRg)
    {
        case DYNAMIC_RANGE_SDR8:
            stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
        case DYNAMIC_RANGE_HDR10:
            stAttr.enVidOutMode    = HI_HDMI_VIDEO_MODE_YCBCR422;
            break;
        default:
            stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_24BIT;
            break;
    }
    stAttr.bxvYCCMode            = HI_FALSE;
    stAttr.enOutCscQuantization  = HDMI_QUANTIZATION_LIMITED_RANGE;

    stAttr.bEnableAudio          = HI_TRUE;
    stAttr.enSoundIntf           = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel       = HI_FALSE;
    stAttr.enSampleRate          = HI_HDMI_SAMPLE_RATE_48K;

    stAttr.enBitDepth            = HI_HDMI_BIT_DEPTH_16;

    stAttr.bEnableAviInfoFrame   = HI_TRUE;
    stAttr.bEnableAudInfoFrame   = HI_TRUE;
    stAttr.bEnableSpdInfoFrame   = HI_FALSE;
    stAttr.bEnableMpegInfoFrame  = HI_FALSE;

    stAttr.bDebugFlag            = HI_FALSE;
    stAttr.bHDCPEnable           = HI_FALSE;

    stAttr.b3DEnable             = HI_FALSE;
    stAttr.enDefaultMode         = HI_HDMI_FORCE_HDMI;

    s32Ret = HI_MPI_HDMI_SetAttr(enHdmiId, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_SetAttr Ret: %08x\n", s32Ret);
        HI_MPI_HDMI_Close(enHdmiId);
        HI_MPI_HDMI_DeInit();
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_HDMI_Start(enHdmiId);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_Start Ret: %08x\n", s32Ret);
        HI_MPI_HDMI_Close(enHdmiId);
        HI_MPI_HDMI_DeInit();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 VO_HdmiStop()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_ID_E            enHdmiId    = HI_HDMI_ID_0;

    s32Ret = HI_MPI_HDMI_Close(enHdmiId);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_Close Ret: %08x\n", s32Ret);
    }

    s32Ret = HI_MPI_HDMI_DeInit();
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_HDMI_DeInit Ret: %08x\n", s32Ret);
    }
    return HI_SUCCESS;
}


HI_S32 PLAYER_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    printf("PLAYER_AO_Open s32SampleRate: %d u32ChnCnt: %d\n", s32SampleRate, u32ChnCnt);
    AUDIO_SOUND_MODE_E enSoundMode = PLAYER_SoundMode_Convert(u32ChnCnt);
    AUDIO_SAMPLE_RATE_E enSampleRate = PLAYER_SampleRate_Convert(s32SampleRate);

    s32Ret = AUDIO_StartHdmi(enSampleRate);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "AUDIO_StartHdmi return ERR, %x\n", s32Ret);
        return HI_FAILURE;
    }


    s32Ret = PLAYER_AO_OPEN(hAoHdl,  s32SampleRate,  enSoundMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_AO_Disable return ERR, %x\n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 PLAYER_AO_Close(HI_HANDLE hAoHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PLAYER_AO_CLOSE(hAoHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "PLAYER_AO_CLOSE return ERR, %x\n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 PLAYER_VPSS_Chn_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                            HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHN_ATTR_S stChnAttr;
    memset(&stChnAttr, 0x00, sizeof(VPSS_CHN_ATTR_S));

    s32Ret = HI_MPI_VPSS_GetChnAttr(hVpssGrpHdl, hVpssChnHdl, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VPSS_GetChnAttr(%d,%d) s32Ret=%x\n", (HI_U32)hVpssGrpHdl, (HI_U32)hVpssChnHdl, s32Ret);
        return s32Ret;
    }


    stChnAttr.enChnMode = VPSS_CHN_MODE_USER;
    stChnAttr.u32Width = u32Width;
    stChnAttr.u32Height = u32Height;
    stChnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stChnAttr.u32Depth                    = 0;
    stChnAttr.bMirror                     = HI_FALSE;
    stChnAttr.bFlip                       = HI_FALSE;
    stChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    stChnAttr.enDynamicRange              = s_genDynRange;

    stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stChnAttr.stFrameRate.s32DstFrameRate = -1;

    s32Ret = HI_MPI_VPSS_SetChnAttr(hVpssGrpHdl, hVpssChnHdl, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf(
            "HI_MPI_VPSS_SetChnAttr(%d,%d) s32Ret=%x\n", (HI_U32)hVpssGrpHdl, (HI_U32)hVpssChnHdl, s32Ret);
        return s32Ret;
    }

    printf( "vpss dst width: %d height: %d\n", u32Width, u32Height);

    s32Ret = HI_MPI_VPSS_EnableChn(hVpssGrpHdl, hVpssChnHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VPSS_EnableChn fail s32Ret:%x\n", s32Ret);
        return s32Ret;
    }

    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    memset(&stSrcChn, 0x00, sizeof(MPP_CHN_S));
    memset(&stDestChn, 0x00, sizeof(MPP_CHN_S));

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = hVpssGrpHdl;
    stSrcChn.s32ChnId = hVpssChnHdl;

    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = hDispHdl;
    stDestChn.s32ChnId = hWndHdl;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "vo(%d, %d) bind to vpss (%d, %d) ERR.\n"
                , hDispHdl, hWndHdl, hVpssGrpHdl, hVpssChnHdl);
        (HI_VOID)HI_MPI_VPSS_DisableChn(hVpssGrpHdl, hVpssChnHdl);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 PLAYER_VPSS_Chn_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                             HI_HANDLE hDispHdl, HI_HANDLE hWndHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    memset(&stSrcChn, 0x00, sizeof(MPP_CHN_S));
    memset(&stDestChn, 0x00, sizeof(MPP_CHN_S));

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = hVpssGrpHdl;
    stSrcChn.s32ChnId = hVpssChnHdl;

    stDestChn.enModId = HI_ID_VO;
    stDestChn.s32DevId = hDispHdl;
    stDestChn.s32ChnId = hWndHdl;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf(  "vo(%lld, %lld) unbind to Vpss (%d, %d) ERR.\n", hDispHdl, hWndHdl, hVpssGrpHdl, hVpssChnHdl);
    }

    s32Ret = HI_MPI_VPSS_DisableChn(hVpssGrpHdl, hVpssChnHdl);

    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VPSS_DisableChn %d vport %d error.\n", hVpssGrpHdl, hVpssChnHdl);
    }

    return HI_SUCCESS;
}


HI_S32 PLAYER_VPSS_Grp_Open(HI_HANDLE hVpssGrpHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    VPSS_GRP_ATTR_S stGrpAttr;
    memset(&stGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
    stGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stGrpAttr.enDynamicRange = s_genDynRange;
    stGrpAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stGrpAttr.u32MaxW = VPSS_MAX_WIDTH;
    stGrpAttr.u32MaxH = VPSS_MAX_HEIGHT;

    s32Ret = HI_MPI_VPSS_CreateGrp(hVpssGrpHdl, &stGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        printf( "Creat vproc %d %#xerror.\n", hVpssGrpHdl, s32Ret);
        goto Failed;
    }

    s32Ret = HI_MPI_VPSS_StartGrp(hVpssGrpHdl);

    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VPSS_StartGrp(%d) s32Ret=%x\n", (HI_U32)hVpssGrpHdl, s32Ret);
        goto Vpss_Destory;
    }

    return HI_SUCCESS;

Vpss_Destory:
    if (HI_SUCCESS != HI_MPI_VPSS_DestroyGrp(hVpssGrpHdl))
    {
        printf( "HI_MPI_VPSS_DestroyGrp fail\n");
    }
Failed:
    return s32Ret;
}

HI_S32 PLAYER_VPSS_Grp_Close(HI_HANDLE hVpssGrpHdl)
{
    if (HI_SUCCESS != HI_MPI_VPSS_StopGrp(hVpssGrpHdl))
    {
        printf( "HI_MPI_VPSS_StopGrp fail\n");
    }

    if (HI_SUCCESS != HI_MPI_VPSS_DestroyGrp(hVpssGrpHdl))
    {
        printf( "HI_MPI_VPSS_DestroyGrp fail\n");
    }

    return HI_SUCCESS;
}


HI_S32 PLAYER_Window_OPEN(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                          HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width,
                          HI_U32 u32Height)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (hVpssGrpHdl != HI_INVALID_HANDLE)
    {
        printf("start vpss chn\n");
        s32Ret = PLAYER_VPSS_Chn_Open(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl, u32Width, u32Height);
        if (HI_SUCCESS != s32Ret)
        {
            printf( "PLAYER_VPSS_Chn_Open fail s32Ret:%x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    VO_CHN_ATTR_S stVoChnAttr;

    memset(&stVoChnAttr, 0x00, sizeof(VO_CHN_ATTR_S));
    stVoChnAttr.u32Priority = 0;
    stVoChnAttr.stRect.s32X = 0;
    stVoChnAttr.stRect.s32Y = 0;
    stVoChnAttr.bDeflicker = HI_FALSE;

    stVoChnAttr.stRect.u32Width = VO_MAX_WIDTH;
    stVoChnAttr.stRect.u32Height = VO_MAX_HEIGHT;

    s32Ret = HI_MPI_VO_SetChnAttr(hDispHdl, hWndHdl, &stVoChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VO_SetChnAttr fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableChn(hDispHdl, hWndHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VO_EnableChn fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = VO_HdmiStartByDyRg(s_genDynRange);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "VO_HdmiStartByDyRg fail s32Ret:%x\n", s32Ret);
        HI_MPI_VO_DisableChn(hDispHdl, hWndHdl);
        if (hVpssGrpHdl != HI_INVALID_HANDLE)
        {
            PLAYER_VPSS_Chn_Close(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl);
        }
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 PLAYER_Window_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                           HI_HANDLE hDispHdl, HI_HANDLE hWndHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = VO_HdmiStop();
    if (HI_SUCCESS != s32Ret)
    {
        printf( "VO_HdmiStop fail s32Ret:%x\n", s32Ret);
    }

    if (hVpssGrpHdl != HI_INVALID_HANDLE)
    {
        s32Ret = PLAYER_VPSS_Chn_Close(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl);
        if (HI_SUCCESS != s32Ret)
        {
            printf( "PLAYER_VPSS_Chn_Close fail s32Ret:%x\n", s32Ret);
        }
    }

    s32Ret = HI_MPI_VO_DisableChn(hDispHdl, hWndHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf( "HI_MPI_VO_DisableChn fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 SDK_DISP_Close(HI_HANDLE hDispHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = hDispHdl;

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VO_DisableVideoLayer fail s32Ret:%x\n", s32Ret);
    }

    s32Ret = HI_MPI_VO_Disable(VoDev);;
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Disable fail s32Ret:%x\n", s32Ret);
    }

    return HI_SUCCESS;
}


static HI_S32 SDK_DISP_Open(HI_HANDLE hDispHdl)
{
    VO_DEV VoDev = 0;
    VO_LAYER VoLayer = hDispHdl;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 s32Ret = HI_SUCCESS;

    memset(&stVoPubAttr, 0x00, sizeof(stVoPubAttr));

    stVoPubAttr.enIntfType = VO_INTF_HDMI;
    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
    stVoPubAttr.u32BgColor = 0x0000ffff;

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VO_SetPubAttr fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VO_Enable fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_SetDisplayBufLen(VoLayer, 3);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetDisplayBufLen failed with %#x!\n", s32Ret);
        HI_MPI_VO_Disable(VoDev);
        return HI_FAILURE;
    }

    memset(&stLayerAttr, 0x00, sizeof(stLayerAttr));

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stLayerAttr.enDstDynamicRange = s_genDynRange;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;

    stLayerAttr.stDispRect.u32Width = VO_MAX_WIDTH;
    stLayerAttr.stDispRect.u32Height = VO_MAX_HEIGHT;
    stLayerAttr.u32DispFrmRt = 30;

    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VO_Enable fail s32Ret:%x\n", s32Ret);
        HI_MPI_VO_Disable(VoDev);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VO_Enable fail s32Ret:%x\n", s32Ret);
        HI_MPI_VO_Disable(VoDev);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



HI_S32 PLAYER_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                      HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_S32 s32Ret = HI_SUCCESS;

    printf("PLAYER_VO_Open width: %d height: %d\n", u32Width, u32Height);
    printf("vpss grp handle: %d chn handle: %d\n", hVpssGrpHdl, hVpssChnHdl);

    s32Ret = SDK_DISP_Open(hDispHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SDK_DISP_Open fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    if (hVpssGrpHdl != HI_INVALID_HANDLE)
    {
        printf("start vpss grp\n");
        s32Ret = PLAYER_VPSS_Grp_Open(hVpssGrpHdl);
        if (HI_SUCCESS != s32Ret)
        {
            printf("PLAYER_VPSS_Grp_Open fail s32Ret:%x\n", s32Ret);
            SDK_DISP_Close(hDispHdl);
            return HI_FAILURE;
        }
    }


    s32Ret = PLAYER_Window_OPEN(hVpssGrpHdl, hVpssChnHdl,
                                hDispHdl, hWndHdl, u32Width, u32Height);
    if (HI_SUCCESS != s32Ret)
    {
        printf("PLAYER_Window_OPEN failed Ret: %0x08x\n", s32Ret);
        SDK_DISP_Close(hDispHdl);
        if (hVpssGrpHdl != HI_INVALID_HANDLE)
        {
            PLAYER_VPSS_Grp_Close( hVpssGrpHdl);
        }
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 PLAYER_VO_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                       HI_HANDLE hDispHdl, HI_HANDLE hWndHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PLAYER_Window_Close(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf("PLAYER_Window_Close fail s32Ret:%x\n", s32Ret);
    }

    if (hVpssGrpHdl != HI_INVALID_HANDLE)
    {
        s32Ret = PLAYER_VPSS_Grp_Close(hVpssGrpHdl);
        if (HI_SUCCESS != s32Ret)
        {
            printf("PLAYER_Window_OPEN failed Ret: %0x08x\n", s32Ret);
        }
    }

    s32Ret = SDK_DISP_Close(hDispHdl);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SDK_DISP_Close fail s32Ret:%x\n", s32Ret);
    }
    return HI_SUCCESS;
}

HI_S32 PLAYER_SYS_Init()
{
    HI_S32 s32Ret = HI_SUCCESS;

    VB_CONFIG_S stVbConfig;
    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 1;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10;
    //stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(3840, 2160,
    //                                       PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_10, COMPRESS_MODE_SEG, 0);
    stVbConfig.astCommPool[0].u64BlkSize = 3840 * 2160 * 2;
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    s32Ret = HI_MPI_VB_SetConfig(&stVbConfig);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}



HI_S32 HI_MW_Sys_Init(HI_VOID)
{
    printf("COMMON_IPCMSG_SVR_Init begin!\n");
    HI_S32 s32Ret = COMMON_IPCMSG_SVR_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("COMMON_IPCMSG_SVR_Init 0x%x failed!\n", s32Ret);
        HI_MAPI_Sys_Deinit();
        return HI_FAILURE;
    }

    printf("HI_MAPI_Sys_Init begin!\n");
    s32Ret = HI_MAPI_Sys_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MAPI_Sys_Init 0x%x failed!\n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


HI_S32 HI_MW_Sys_Deinit(HI_VOID)
{
    HI_S32 s32Ret = COMMON_IPCMSG_SVR_Deinit();
    if (HI_SUCCESS != s32Ret)
    {
        printf("COMMON_IPCMSG_SVR_Init 0x%x failed!\n", s32Ret);
    }
    HI_MAPI_Sys_Deinit();
    return HI_SUCCESS;
}


