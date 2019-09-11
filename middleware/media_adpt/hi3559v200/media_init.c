#include <stdio.h>
#include <string.h>
#include "comm_define.h"
#include "../include/hi_mw_media_intf.h"
#include "common_ipcmsg_client.h"
#ifdef ENABLE_AUDIO
#include "hi_mapi_aenc_adpt.h"
#endif
#include "lcd_mw_adapt.h"

HI_HANDLE g_hACapHdl = 0;
HI_HANDLE g_hACapChnHdl = 0;

static HI_BOOL s_abVPortStarted[5] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
static HI_BOOL s_abVProcStarted[5] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
static HI_BOOL s_abVProcInited[5] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
static HI_BOOL s_abVProcBinded[5] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
static HI_BOOL s_abVEncPrepared[5] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};


HI_S32 AcapGetChnHandle(HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode, HI_HANDLE* pAcapChn)
{
    switch(enSoundMode)
    {
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
            *pAcapChn = 0;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            *pAcapChn = 1;
            break;
        default:
            printf("not support sound mode!\n");
            return HI_FAILURE;
        }
    return HI_SUCCESS;
}

static HI_S32 VPortPrepare(HI_HANDLE hVProcHandle, HI_MW_VPortAttr* pstVPortAttr)
{
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    stVPortAttr.u32Width = pstVPortAttr->WIDTH;
    stVPortAttr.u32Height = pstVPortAttr->HEIGHT;
    stVPortAttr.stFrameRate.s32SrcFrameRate = pstVPortAttr->FRAMERATE;
    stVPortAttr.stFrameRate.s32DstFrameRate = pstVPortAttr->FRAMERATE;
    stVPortAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVPortAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVPortAttr.enCompressMode = COMPRESS_MODE_NONE;

    if (HI_MAPI_VPROC_SetPortAttr(hVProcHandle, pstVPortAttr->HANDLE, &stVPortAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VPROC_SetPortAttr fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_VPROC_StartPort(hVProcHandle, pstVPortAttr->HANDLE) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VPROC_StartPort fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_MAPI_PAYLOAD_TYPE_E convertPayloadType(HI_MW_PAYLOAD_TYPE_E enMWPayload)
{
    if (HI_MW_PAYLOAD_TYPE_H264 == enMWPayload)
    {
        return HI_MAPI_PAYLOAD_TYPE_H264;
    }
    else if (HI_MW_PAYLOAD_TYPE_H265 == enMWPayload)
    {
        return HI_MAPI_PAYLOAD_TYPE_H265;
    }
    else if (HI_MW_PAYLOAD_TYPE_JPEG == enMWPayload)
    {
        return HI_MAPI_PAYLOAD_TYPE_JPEG;
    }

    return HI_MAPI_PAYLOAD_TYPE_H264;
}

HI_S32 VEncPrepare(HI_HANDLE hVProcHandle, HI_MW_VPortAttr* pstVPortAttr, HI_MW_VEncAttr* pstVEncAttr)
{
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    memset(&stVencAttr, 0, sizeof(HI_MAPI_VENC_ATTR_S));
    stVencAttr.stVencPloadTypeAttr.enType = convertPayloadType(pstVEncAttr->PAYLOAD_TYPE);
    stVencAttr.stVencPloadTypeAttr.enSceneMode = HI_MAPI_VENC_SCENE_MODE_DV;
    stVencAttr.stVencPloadTypeAttr.u32Width = pstVEncAttr->WIDTH;
    stVencAttr.stVencPloadTypeAttr.u32Height = pstVEncAttr->HEIGHT;
    stVencAttr.stVencPloadTypeAttr.u32BufSize = pstVEncAttr->BUFSIZE;
    stVencAttr.stVencPloadTypeAttr.u32Profile = pstVEncAttr->PROFILE;

    if (HI_MAPI_PAYLOAD_TYPE_JPEG == stVencAttr.stVencPloadTypeAttr.enType)
    {
        stVencAttr.stVencPloadTypeAttr.stAttrJpege.u32Qfactor = 90;
    }

    stVencAttr.stRcAttr.enRcMode = HI_MAPI_VENC_RC_MODE_CBR;
    stVencAttr.stRcAttr.stAttrCbr.u32BitRate = pstVEncAttr->BITRATE;
    stVencAttr.stRcAttr.stAttrCbr.u32Gop = pstVEncAttr->GOP;
    stVencAttr.stRcAttr.stAttrCbr.u32SrcFrameRate = pstVEncAttr->FRAMERATE;
    stVencAttr.stRcAttr.stAttrCbr.fr32DstFrameRate = pstVEncAttr->FRAMERATE;
    stVencAttr.stRcAttr.stAttrCbr.u32StatTime = 3;
    HI_MAPI_DATAFIFO_CONFIG_S stDatafifoCfg;
    stDatafifoCfg.u32VencDatafifeNodeNum = 200;
    if (HI_MAPI_Sys_SetDatafifoNodeNum(&stDatafifoCfg) != HI_SUCCESS)
    {
        printf("HI_MAPI_Sys_SetDatafifoNodeNum fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_VENC_Init(pstVEncAttr->HANDLE, &stVencAttr) != HI_SUCCESS)
    {
        printf("call HI_MAIN_VEnc_Init fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_VENC_BindVProc(hVProcHandle, pstVPortAttr->HANDLE, pstVEncAttr->HANDLE, bStitch) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VEnc_Bind_VProc fail\n");
        HI_MAPI_VENC_Deinit(pstVEncAttr->HANDLE);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MW_Media_Init(HI_MW_SensorAttr_S* pstMWSensorAttr,
                        HI_MW_VCapAttr_S* pstMWVCapAttr,
                        HI_MW_VideoBindMap_S astVideoBindMap[], HI_U32 u32BindMapSize,
                        HI_HANDLE hACapHdl, HI_HANDLE hAEncHdl)
{
    HI_U32 i = 0;
    HI_BOOL bError = HI_FALSE;

    HI_HANDLE  VcapDev0 = 0;
    HI_HANDLE  VcapPipe0 = 0;
    HI_HANDLE  PipeChn0 = 0;

    HI_MAPI_SENSOR_ATTR_S stSensorAttr;
    memset(&stSensorAttr, 0x00, sizeof(HI_MAPI_SENSOR_ATTR_S));
    stSensorAttr.u8SnsMode = 0;
    stSensorAttr.stSize.u32Width = pstMWSensorAttr->WIDTH;
    stSensorAttr.stSize.u32Height = pstMWSensorAttr->HEIGHT;
    stSensorAttr.enWdrMode = WDR_MODE_NONE;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    memset(&stVcapAttr, 0x00, sizeof(HI_MAPI_VCAP_ATTR_S));
    //dev attr
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe0;
    stVcapAttr.stVcapDevAttr.enWdrMode = WDR_MODE_NONE;
    stVcapAttr.stVcapDevAttr.stBasSize.u32Height = pstMWVCapAttr->HEIGHT;
    stVcapAttr.stVcapDevAttr.stBasSize.u32Width = pstMWVCapAttr->WIDTH;

    //pipe attr
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].bIspBypass = HI_FALSE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stFrameRate.s32DstFrameRate = -1;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stFrameRate.s32SrcFrameRate = -1;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enCompressMode = WDR_MODE_NONE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeBypassMode = VI_PIPE_BYPASS_NONE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stFrameIntAttr.enIntType = FRAME_INTERRUPT_EARLY_END;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stFrameIntAttr.u32EarlyLine = pstMWVCapAttr->HEIGHT - 10;
    //pipechn attr
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].stFrameRate.s32DstFrameRate = pstMWVCapAttr->FRAMERATE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].stFrameRate.s32SrcFrameRate = pstMWVCapAttr->FRAMERATE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].enCompressMode = WDR_MODE_NONE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].stDestSize.u32Height = pstMWVCapAttr->HEIGHT;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0].stDestSize.u32Width = pstMWVCapAttr->WIDTH;

    //pipe isp attr
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stIspPubAttr.f32FrameRate = pstMWVCapAttr->FRAMERATE;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stIspPubAttr.stSize.u32Width = pstMWVCapAttr->WIDTH;
    stVcapAttr.astVcapPipeAttr[VcapPipe0].stIspPubAttr.stSize.u32Height = pstMWVCapAttr->HEIGHT;

    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0x00, sizeof(HI_MAPI_MEDIA_ATTR_S));
    stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;
    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize  = pstMWVCapAttr->WIDTH * pstMWVCapAttr->HEIGHT * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt   = 10;

    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize  = pstMWVCapAttr->WIDTH * pstMWVCapAttr->HEIGHT * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt   = 4;

    if (HI_MAPI_Sys_Init() != HI_SUCCESS)
    {
        printf("call HI_MAPI_Sys_Init fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_Media_Init(&stMediaAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_Media_Init fail\n");
        goto SYS_DEINIT;
    }

    /*vcap*/
    if (HI_MAPI_VCAP_InitSensor(VcapDev0, &stSensorAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VCAP_InitSensor fail\n");
        goto MEDIA_DEINIT;
    }

    if (HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VCAP_SetAttr fail\n");
        goto MEDIA_DEINIT;
    }

    if (HI_MAPI_VCAP_StartDev(VcapDev0) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VCAP_StartDev fail\n");
        goto MEDIA_DEINIT;
    }

    if (HI_MAPI_VCAP_StartChn(VcapPipe0, PipeChn0) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VCAP_StartChn fail\n");
        goto ISP_STOP;
    }

    if (HI_MAPI_VCAP_StartISP(VcapPipe0) != HI_SUCCESS)
    {
        printf("call HI_MAPI_VCAP_StartChn fail\n");
        goto ISP_STOP;
    }

    HI_HANDLE  VpssHdl0 = 0;
    HI_HANDLE  VPortHdl0 = 0;
    HI_BOOL    bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    for (i = 0; i < u32BindMapSize; i++)
    {
        if (HI_FALSE == s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            HI_MAPI_VPSS_ATTR_S stVpssAttr;
            memset(&stVpssAttr, 0x00, sizeof(HI_MAPI_VPSS_ATTR_S));
            stVpssAttr.u32MaxW = astVideoBindMap[i].pstVProcAttr->WIDTH;
            stVpssAttr.u32MaxH = astVideoBindMap[i].pstVProcAttr->HEIGHT;
            stVpssAttr.stFrameRate.s32SrcFrameRate = 30;
            stVpssAttr.stFrameRate.s32DstFrameRate = 30;
            stVpssAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            stVpssAttr.bNrEn = HI_TRUE;
            stVpssAttr.stNrAttr.enNrType = 0;
            stVpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_NONE;
            stVpssAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;

            if (HI_MAPI_VPROC_InitVpss(astVideoBindMap[i].pstVProcAttr->HANDLE, &stVpssAttr) != HI_SUCCESS)
            {
                printf("call HI_MAPI_VProc_Init fail\n");
                bError = HI_TRUE;
                break;
            }

            s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_TRUE;
        }

        if (HI_FALSE == s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            if (HI_MAPI_VPROC_BindVcap(VcapPipe0, PipeChn0, astVideoBindMap[i].pstVProcAttr->HANDLE) != HI_SUCCESS)
            {
                printf("call HI_MAPI_VPROC_BindVcap fail\n");
                bError = HI_TRUE;
                break;
            }

            s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_TRUE;
        }

        if (HI_FALSE == s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE])
        {
            if (HI_SUCCESS != VPortPrepare(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr))
            {
                printf("VPortPrepare for %lu fail\n", i);
                bError = HI_TRUE;
                break;
            }

            s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE] = HI_TRUE;
        }

        if (HI_FALSE == s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_TRUE;
        }

        if (HI_FALSE == s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE])
        {
            if (HI_SUCCESS != VEncPrepare(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr, astVideoBindMap[i].pstVEncAttr))
            {
                printf("call VEncPrepare for %lu fail\n", i);
                bError = HI_TRUE;
                break;
            }

            s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE] = HI_TRUE;
        }
    }

    if (bError)
    {
        printf("bError happened \n");
        goto VENC_DEINIT;
    }

#ifdef ENABLE_AUDIO
    HI_MAPI_ACAP_ATTR_S stACapAttr;
    stACapAttr.enSampleRate = AUDIO_SAMPLE_RATE_48000;
    stACapAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stACapAttr.enSoundMode = HI_MAPI_AUDIO_SOUND_MODE_LEFT;
    stACapAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stACapAttr.u32PtNumPerFrm = 1024;
    stACapAttr.enMixerMicMode = ACODEC_MIXER_IN0;
    stACapAttr.enResampleRate = AUDIO_SAMPLE_RATE_48000;
    HI_HANDLE hACapChnHdl;
    if (HI_SUCCESS != AcapGetChnHandle(stACapAttr.enSoundMode, &hACapChnHdl))
    {
        printf("call AcapGetChnHandle fail\n");
        goto VENC_DEINIT;
    }
    g_hACapChnHdl = hACapChnHdl;

    hACapHdl = 0;
    g_hACapHdl = hACapHdl;

    if (HI_MAPI_ACAP_Init(hACapHdl, &stACapAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_ACap_Init fail\n");
        goto VENC_DEINIT;
    }

    if (HI_MAPI_ACAP_Start(hACapHdl, hACapHdl) != HI_SUCCESS)
    {
        printf("call HI_MAPI_ACap_Start fail\n");
        goto ACAP_DEINIT;
    }

    HI_S32 s32Gain = 50;

    if (HI_MAPI_ACAP_SetVolume(g_hACapHdl , s32Gain) != HI_SUCCESS)
    {
        printf("call HI_MAPI_ACap_SetVolume fail\n");
        goto ACAP_STOP;
    }

    if (HI_MAPI_AENC_RegisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_AACLC) != HI_SUCCESS)
    {
        printf("call HI_MAPI_Register_AudioEncoder fail\n");
        goto ACAP_STOP;
    }

    HI_MAPI_AENC_ATTR_S stAencAttr;
    HI_MAPI_AENC_ATTR_AAC_S stAacAencAttr;
    stAacAencAttr.enAACType = AAC_TYPE_AACLC;
    stAacAencAttr.enBitRate = AAC_BPS_48K;
    stAacAencAttr.enBitWidth = AUDIO_BIT_WIDTH_16;
    stAacAencAttr.enSmpRate = AUDIO_SAMPLE_RATE_48000;
    stAacAencAttr.enSoundMode = AUDIO_SOUND_MODE_MONO;
    stAacAencAttr.enTransType = AAC_TRANS_TYPE_ADTS;
    stAacAencAttr.s16BandWidth = 0;
    stAencAttr.enAencFormat = HI_MAPI_AUDIO_FORMAT_AACLC;
    stAencAttr.u32PtNumPerFrm = 1024;
    stAencAttr.pValue = &stAacAencAttr;
    stAencAttr.u32Len = sizeof(HI_MAPI_AENC_ATTR_AAC_S);

    if (HI_MAPI_AENC_Init(hAEncHdl, &stAencAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_AENC_Init fail\n");
        goto UNREG_AUDIO_ENCODER;
    }

    return HI_SUCCESS;
UNREG_AUDIO_ENCODER:
    HI_MAPI_AENC_UnregisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_AACLC);
ACAP_STOP:
    HI_MAPI_ACAP_Stop(hACapHdl, hACapHdl);
ACAP_DEINIT:
    HI_MAPI_ACAP_Deinit(hACapHdl);
#else
    return HI_SUCCESS;
#endif

VENC_DEINIT:

    for (i = 0; i < u32BindMapSize; i++)
    {
        if (s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE])
        {
            HI_MAPI_VENC_UnBindVProc(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr->HANDLE, astVideoBindMap[i].pstVEncAttr->HANDLE, HI_FALSE);
            HI_MAPI_VENC_Deinit(astVideoBindMap[i].pstVEncAttr->HANDLE);
        }

        s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE] = HI_FALSE;

        if (s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE])
        {
            HI_MAPI_VPROC_StopPort(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr->HANDLE);
        }

        s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE] = HI_FALSE;
    }

    for (i = 0; i < u32BindMapSize; i++)
    {
        if (s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            //HI_MAPI_VProc_Stop(astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;

        if (s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            //HI_MAPI_VProc_UnBind_VCap(pstMWVCapAttr->HANDLE, astVideoBindMap[i].pstVProcAttr->HANDLE);
            HI_MAPI_VPROC_UnBindVCap(pstMWVCapAttr->HANDLE, 0, astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;

        if (s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            HI_MAPI_VPROC_DeinitVpss(astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;
    }

ISP_STOP:
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);
MEDIA_DEINIT:
    HI_MAPI_Media_Deinit();
SYS_DEINIT:
    HI_MAPI_Sys_Deinit();
    return HI_FAILURE;
}

HI_S32 HI_MW_Media_DeInit(__attribute__((unused)) HI_MW_SensorAttr_S* pstMWSensorAttr,
                          HI_MW_VCapAttr_S* pstMWVCapAttr,
                          HI_MW_VideoBindMap_S astVideoBindMap[], HI_U32 u32BindMapSize,
                          HI_HANDLE hACapHdl, HI_HANDLE hAEncHdl)
{
    HI_HANDLE  VcapDev0 = 0;
    HI_HANDLE  VcapPipe0 = 0;
    HI_HANDLE  PipeChn0 = 0;
#ifdef ENABLE_AUDIO
    hACapHdl = g_hACapHdl;
    HI_MAPI_AENC_Deinit(hAEncHdl);
    HI_MAPI_AENC_UnregisterAudioEncoder(HI_MAPI_AUDIO_FORMAT_AACLC);
    HI_MAPI_ACAP_Stop(hACapHdl, g_hACapChnHdl);
    HI_MAPI_ACAP_Deinit(hACapHdl);
#endif
    HI_U32 i;

    for (i = 0; i < u32BindMapSize; i++)
    {
        if (s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE])
        {
            HI_MAPI_VENC_UnBindVProc(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr->HANDLE, astVideoBindMap[i].pstVEncAttr->HANDLE, HI_FALSE);
            HI_MAPI_VENC_Deinit(astVideoBindMap[i].pstVEncAttr->HANDLE);
        }

        s_abVEncPrepared[astVideoBindMap[i].pstVEncAttr->HANDLE] = HI_FALSE;

        if (s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE])
        {
            HI_MAPI_VPROC_StopPort(astVideoBindMap[i].pstVProcAttr->HANDLE, astVideoBindMap[i].pstVPortAttr->HANDLE);
        }

        s_abVPortStarted[astVideoBindMap[i].pstVPortAttr->HANDLE] = HI_FALSE;
    }

    for (i = 0; i < u32BindMapSize; i++)
    {
        if (s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            //HI_MAPI_VProc_Stop(astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcStarted[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;

        if (s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            //HI_MAPI_VProc_UnBind_VCap(pstMWVCapAttr->HANDLE, astVideoBindMap[i].pstVProcAttr->HANDLE);
            HI_MAPI_VPROC_UnBindVCap(pstMWVCapAttr->HANDLE, 0, astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcBinded[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;

        if (s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE])
        {
            HI_MAPI_VPROC_DeinitVpss(astVideoBindMap[i].pstVProcAttr->HANDLE);
        }

        s_abVProcInited[astVideoBindMap[i].pstVProcAttr->HANDLE] = HI_FALSE;
    }

    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);
    HI_MAPI_Media_Deinit();
    HI_MAPI_Sys_Deinit();

    return HI_SUCCESS;
}


HI_VOID HI_MW_HDMI_EventPro(HI_HDMI_EVENT_TYPE_E event, HI_VOID* pPrivateData)
{
    switch (event)
    {
        case HI_HDMI_EVENT_HOTPLUG:
            printf("\033[0;32mhdmi HOTPLUG event! \033[0;39m\n");
            break;

        case HI_HDMI_EVENT_NO_PLUG:
            printf("\033[0;31mhdmi NO_PLUG event! \033[0;39m\n");
            break;

        default :
            printf("\033[0;31minvalid hdmi event! \033[0;39m\n");
            break;
    }

exit:
    (-1);
}

HI_S32 HI_MW_DISP_SetDisplayGraphicCSC(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = 0;
    HI_MAPI_DISP_CSCATTREX_S stCscAttr;

    s32Ret = HI_MAPI_DISP_GetAttrEx(DispHdl, HI_MAPI_DISP_CMD_VIDEO_CSC, &stCscAttr, sizeof(HI_MAPI_DISP_CSCATTREX_S));

    stCscAttr.stVoCSC.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
    stCscAttr.stVoCSC.u32Contrast = 50;
    stCscAttr.stVoCSC.u32Hue = 50;
    stCscAttr.stVoCSC.u32Luma = 50;
    stCscAttr.stVoCSC.u32Satuature = 50;
    s32Ret = HI_MAPI_DISP_SetAttrEx(DispHdl, HI_MAPI_DISP_CMD_GRAPHIC_CSC, &stCscAttr, sizeof(HI_MAPI_DISP_CSCATTREX_S));

    return HI_SUCCESS;
}

static HI_S32 HI_MW_DISP_OpenLCD()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = 0;
    HI_MAPI_DISP_ATTR_S stDispAttr;

    memset(&stDispAttr, 0x0, sizeof(stDispAttr));
    stDispAttr.stPubAttr.u32BgColor = 0x0000FF;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_6BIT;
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    stDispAttr.stPubAttr.stSyncInfo.bSynm = 1;
    stDispAttr.stPubAttr.stSyncInfo.bIop = 1;
    stDispAttr.stPubAttr.stSyncInfo.u16Vact = 320;
    stDispAttr.stPubAttr.stSyncInfo.u16Vbb = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Vfb = 4;
    stDispAttr.stPubAttr.stSyncInfo.u16Hact = 240;
    stDispAttr.stPubAttr.stSyncInfo.u16Hbb = 30;
    stDispAttr.stPubAttr.stSyncInfo.u16Hfb = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Hpw = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Vpw = 2;
    stDispAttr.stPubAttr.stSyncInfo.bIdv = 0;
    stDispAttr.stPubAttr.stSyncInfo.bIhs = 0;
    stDispAttr.stPubAttr.stSyncInfo.bIvs = 0;
    stDispAttr.stUserInfoAttr.u32DevFrameRate = 60;

    stDispAttr.stUserInfoAttr.stUserInfo.u32DevDiv = 3;
    stDispAttr.stUserInfoAttr.stUserInfo.u32PreDiv = 1;
    stDispAttr.stUserInfoAttr.stUserInfo.bClkReverse = HI_TRUE;
    stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.enClkSource = VO_CLK_SOURCE_LCDMCLK;
    stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.u32LcdMClkDiv = 0x1CF62C;

    s32Ret = HI_MAPI_DISP_Init(DispHdl, &stDispAttr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_DISP_Init fail\n");
    }

    return HI_SUCCESS;
}

static HI_S32 HI_MW_DISP_CloseLCD()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = 0;

    s32Ret = HI_MAPI_DISP_Deinit(DispHdl);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_DISP_Deinit fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 HI_MW_DISP_OpenHDMI()
{
    /*start disp*/
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE DispHdl = 0;
    HI_MAPI_DISP_ATTR_S stDispAttr;

    memset(&stDispAttr, 0x0, sizeof(stDispAttr));
    stDispAttr.stPubAttr.u32BgColor = 0x0;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_1080P30;

    s32Ret = HI_MAPI_DISP_Init(DispHdl, &stDispAttr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_DISP_Init fail\n");
        //return HI_FAILURE;
    }

    /*starts HDMI*/
    HI_MAPI_HDMI_ATTR_S sthdmiattr;
    HI_HANDLE HdmiHdl = 0;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallBackFunc;

    memset(&sthdmiattr, 0, sizeof(HI_MAPI_HDMI_ATTR_S));

    s32Ret = HI_MAPI_HDMI_GetAttr(HdmiHdl, &sthdmiattr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_HDMI_GetAttr fail\n");
        //return HI_FAILURE;
    }

    if (HI_TRUE == sthdmiattr.bEnableHdmi)
    {
        printf("HDMI already start!!!\n");
        return HI_SUCCESS;
    }

    stHdmiCallBackFunc.pfnHdmiEventCallback = (HI_HDMI_CallBack)HI_MW_HDMI_EventPro;
    stHdmiCallBackFunc.pPrivateData = HI_NULL;

    memset(&sthdmiattr, 0, sizeof(HI_MAPI_HDMI_ATTR_S));
    sthdmiattr.bEnableHdmi = HI_TRUE;
    sthdmiattr.bEnableVideo = HI_TRUE;
    sthdmiattr.enHdmiSync = VO_OUTPUT_1080P30;
    sthdmiattr.bEnableAudio = HI_FALSE;
    sthdmiattr.enSampleRate = AUDIO_SAMPLE_RATE_48000;

    s32Ret = HI_MAPI_HDMI_Init(HdmiHdl, &stHdmiCallBackFunc);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_HDMI_Init fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MAPI_HDMI_SetAttr(HdmiHdl, &sthdmiattr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_HDMI_SetAttr fail\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MAPI_HDMI_Start(HdmiHdl);

    if (HI_SUCCESS != s32Ret)
    {
        printf("call HI_MAPI_HDMI_Start fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 HI_MW_DISP_CloseHDMI()
{
    HI_HANDLE DispHdl = 0;
    HI_HANDLE HdmiHdl = 0;

    HI_MAPI_DISP_Deinit(DispHdl);
    HI_MAPI_HDMI_Stop(HdmiHdl);
    HI_MAPI_HDMI_Deinit(HdmiHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MW_DISP_Open(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0x00, sizeof(stMediaAttr));
    stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;
    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize  = 3840 * 2160 * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt   = 2;

    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize  = 3840 * 2160 * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt   = 2;

    //config screen param
    MW_SCREEN_ST7789_Init();

    //config touchpad param
    MW_TOUCHPAD_FT6236_Init();

    //load screen driver
    system("insmod /komod/hi_ssp_st7789.ko");

    //load touchpad driver
    system("insmod /komod/ft_6236.ko");

    usleep(120 * 1000);

    if (HI_MAPI_Sys_Init() != HI_SUCCESS)
    {
        printf("call HI_MAPI_Sys_Init fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_Media_Init(&stMediaAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_Media_Init fail\n");
        return HI_FAILURE;
    }

    COMMON_IPCMSG_CLIENT_Init();

    s32Ret = HI_MW_DISP_OpenLCD();

    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    MW_SCREEN_VDP_CFG();

    return HI_SUCCESS;
}

HI_S32 HI_MW_DISP_Close(HI_VOID)
{
    HI_MW_DISP_CloseLCD();
    COMMON_IPCMSG_CLIENT_Deinit();
    HI_MAPI_Media_Deinit();
    HI_MAPI_Sys_Deinit();
    return HI_SUCCESS;
}

HI_S32 HI_MW_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                     HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height)
{
    return COMMON_Client_VO_Open(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl, u32Width, u32Height);
}

HI_S32 HI_MW_VO_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                      HI_HANDLE hDispHdl, HI_HANDLE hWndHdl)
{
    return COMMON_Client_VO_Close(hVpssGrpHdl, hVpssChnHdl, hDispHdl, hWndHdl);
}

HI_S32 HI_MW_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt)
{
    return COMMON_Client_AO_Open(hAoHdl, s32SampleRate, u32ChnCnt);
}

HI_S32 HI_MW_AO_Close(HI_HANDLE hAoHdl)
{
    return COMMON_Client_AO_Close(hAoHdl);
}

HI_S32 HI_MW_AO_GetDevHandle(HI_HANDLE* phAoHdl)
{
    *phAoHdl = 1;
    return HI_SUCCESS;
}

HI_S32 HI_MW_VB_Alloc(HI_U32* pPoolId, HI_U32 u32FrameSize, HI_U32 u32FrameCnt)
{
    return COMMON_Client_Vb_Alloc(pPoolId, u32FrameSize, u32FrameCnt);
}

HI_S32 HI_MW_VB_Free(HI_U32 poolid)
{
    return COMMON_Client_Vb_Free(poolid);
}

HI_S32 HI_MW_Sys_Init(HI_VOID)
{
    printf("call COMMON_IPCMSG_CLIENT_Init \n");
    COMMON_IPCMSG_CLIENT_Init();


    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0x00, sizeof(stMediaAttr));
    stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;
    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize  = 3840 * 2160 * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt   = 10;

    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize  = 3840 * 2160 * 2;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt   = 4;

    printf("call HI_MAPI_Sys_Init \n");

    if (HI_MAPI_Sys_Init() != HI_SUCCESS)
    {
        printf("call HI_MAPI_Sys_Init fail\n");
        return HI_FAILURE;
    }

    if (HI_MAPI_Media_Init(&stMediaAttr) != HI_SUCCESS)
    {
        printf("call HI_MAPI_Media_Init fail\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MW_Sys_Deinit(HI_VOID)
{
    COMMON_IPCMSG_CLIENT_Deinit();
    HI_MAPI_Media_Deinit();
    HI_MAPI_Sys_Deinit();
    return HI_SUCCESS;
}
