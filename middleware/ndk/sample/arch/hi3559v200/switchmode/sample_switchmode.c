/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_switch.c
 * @brief   sample switch function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define SAMPLE_IMX477_MAX_MODE_NUM          2
#define SAMPLE_IMX477_MAX_PIPE_ATTR_NUM     2
#define SAMPLE_IMX477_MAX_PIPE_CHN_ATTR_NUM 2
#define SAMPLE_IMX477_MAX_VPSS_ATTR_NUM     2
#define SAMPLE_IMX477_MAX_VPORT_ATTR_NUM    2
#define SAMPLE_IMX477_MAX_VENC_ATTR_NUM     2

static HI_MAPI_SENSOR_ATTR_S g_stSensor4KMode1 = {
    .u8SnsMode = 1,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    }
};

static HI_MAPI_SENSOR_ATTR_S g_stSensor12MMode0 = {
    .u8SnsMode = 0,
    .enWdrMode = WDR_MODE_NONE,
    .stSize = {
        .u32Width = 4000,
        .u32Height = 3000,
    }
};

static HI_MAPI_VCAP_DEV_ATTR_S g_stDev4K = {
    .stBasSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    },
    .enWdrMode = WDR_MODE_NONE,
};

static HI_MAPI_VCAP_DEV_ATTR_S g_stDev12M = {
    .stBasSize = {
        .u32Width = 4000,
        .u32Height = 3000,
    },
    .enWdrMode = WDR_MODE_NONE,
};

static HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe4K30Fps = {
    .stFrameRate = {
        .s32SrcFrameRate = -1,
        .s32DstFrameRate = -1,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .bIspBypass = HI_FALSE,
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .stIspPubAttr = {
        .f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE,
        .stSize = {
            .u32Width = 3840,
            .u32Height = 2160,
        },
        .enBayer = BAYER_RGGB,
    }
};

static HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe12M30Fps = {
    .stFrameRate = {
        .s32SrcFrameRate = -1,
        .s32DstFrameRate = -1,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .bIspBypass = HI_FALSE,
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .stIspPubAttr = {
        .f32FrameRate = 20.0f,
        .stSize = {
            .u32Width = 4000,
            .u32Height = 3000,
        },
        .enBayer = BAYER_RGGB,
    }
};

static HI_MAPI_PIPE_CHN_ATTR_S g_stChn4K30Fps = {
    .stDestSize = {
        .u32Width = 3840,
        .u32Height = 2160,
    },
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
};

static HI_MAPI_PIPE_CHN_ATTR_S s_stChn12M30Fps = {
    .stDestSize = {
        .u32Width = 4000,
        .u32Height = 3000,
    },
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enCompressMode = COMPRESS_MODE_NONE,
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
};

static HI_MAPI_VPSS_ATTR_S g_stVpss4K30Fps = {
    .u32MaxW = 3840,
    .u32MaxH = 2160,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .bNrEn = HI_TRUE,
    .stNrAttr = {
        .enCompressMode = COMPRESS_MODE_NONE,
        .enNrMotionMode = NR_MOTION_MODE_NORMAL,
    },
};

static HI_MAPI_VPSS_ATTR_S g_stVpss12M30Fps = {
    .u32MaxW = 4000,
    .u32MaxH = 3000,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .bNrEn = HI_TRUE,
    .stNrAttr = {
        .enCompressMode = COMPRESS_MODE_NONE,
        .enNrMotionMode = NR_MOTION_MODE_NORMAL,
    },
};

static HI_MAPI_VPORT_ATTR_S g_stVport4K30Fps = {
    .u32Width = 3840,
    .u32Height = 2160,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
};

static HI_MAPI_VPORT_ATTR_S g_stVport12M30Fps = {
    .u32Width = 4000,
    .u32Height = 3000,
    .stFrameRate = {
        .s32SrcFrameRate = 30,
        .s32DstFrameRate = 30,
    },
    .enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_NONE,
};

static HI_MAPI_VENC_ATTR_S g_stVenc4K30FpsVideoH264 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H264,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32BufSize = 3840 * 2160 * 2,
        .u32Profile = 1,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_NORMAL,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 30,
            .u32StatTime = 2,
            .u32SrcFrameRate = 30,
            .fr32DstFrameRate = 30,
            .u32BitRate = 10000,
        }
    }
};

static HI_MAPI_VENC_ATTR_S g_stVenc4K30FpsJpeg = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_JPEG,
        .u32Width = 3840,
        .u32Height = 2160,
        .u32BufSize = 3840 * 2160 * 2,
        .stAttrJpege = {
            .bEnableDCF = HI_TRUE,
            .u32Qfactor = 90,
            .enJpegEncodeMode = JPEG_ENCODE_SNAP,
            .stAttrMPF = {
                .u8LargeThumbNailNum = 0,
            },
        },
    },
};

static HI_MAPI_VENC_ATTR_S g_stVenc12M15FpsVideoH264 = {
    .stVencPloadTypeAttr = {
        .enType = HI_MAPI_PAYLOAD_TYPE_H264,
        .u32Width = 4000,
        .u32Height = 3000,
        .u32BufSize = 4000 * 3000 * 2,
        .u32Profile = 1,
        .enSceneMode = HI_MAPI_VENC_SCENE_MODE_NORMAL,
    },
    .stRcAttr = {
        .stAttrCbr = {
            .u32Gop = 15,
            .u32StatTime = 2,
            .u32SrcFrameRate = 15,
            .fr32DstFrameRate = 15,
            .u32BitRate = 10000,
        }
    }
};

/* all the cfg */
static HI_MAPI_SENSOR_ATTR_S *g_pstSensorAttr[SAMPLE_IMX477_MAX_MODE_NUM] = {
    &g_stSensor4KMode1,
    &g_stSensor12MMode0,
};
static HI_MAPI_VCAP_DEV_ATTR_S *g_pstDevAttr[SAMPLE_IMX477_MAX_MODE_NUM] = {
    &g_stDev4K,
    &g_stDev12M,
};

static HI_MAPI_VCAP_PIPE_ATTR_S *g_pstPipeAttr[SAMPLE_IMX477_MAX_MODE_NUM][SAMPLE_IMX477_MAX_PIPE_ATTR_NUM] = {
    { &g_stPipe4K30Fps, NULL },
    { &g_stPipe12M30Fps, NULL },
};

static HI_MAPI_PIPE_CHN_ATTR_S *g_pstChnAttr[SAMPLE_IMX477_MAX_MODE_NUM][SAMPLE_IMX477_MAX_PIPE_CHN_ATTR_NUM] = {
    { &g_stChn4K30Fps, NULL },
    { &s_stChn12M30Fps, NULL },
};

static HI_MAPI_VPSS_ATTR_S *g_pstVpssAttr[SAMPLE_IMX477_MAX_MODE_NUM][SAMPLE_IMX477_MAX_VPSS_ATTR_NUM] = {
    { &g_stVpss4K30Fps, NULL },
    { &g_stVpss12M30Fps, NULL },
};

static HI_MAPI_VPORT_ATTR_S *g_pstVportAttr[SAMPLE_IMX477_MAX_MODE_NUM][SAMPLE_IMX477_MAX_VPORT_ATTR_NUM] = {
    { &g_stVport4K30Fps, NULL },
    { &g_stVport12M30Fps, NULL },
};

static HI_MAPI_VENC_ATTR_S *g_pstVencAttr[SAMPLE_IMX477_MAX_MODE_NUM][SAMPLE_IMX477_MAX_VENC_ATTR_NUM] = {
    { &g_stVenc4K30FpsVideoH264, &g_stVenc4K30FpsJpeg },
    { &g_stVenc12M15FpsVideoH264, NULL },
};

HI_S32 Sample_SwitchSensorMode(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;
    HI_S32 s32ModeCnt;
    HI_CHAR aszSaveName[64] = {0};
    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video\033[0;39m\n\n");

    /* 8M30fps */
    s32ModeCnt = 0;
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc callback */
    sprintf(aszSaveName, "%s_8M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    sleep(3);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    /* stop vcap 0 */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_DeinitSensor(hVcapDev0));

    /* 12M Mode */
    s32ModeCnt = 1;
    printf("\n\r\033[1;34m--->>>sony_imx477 12M@30 Video\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc callback */
    sprintf(aszSaveName, "%s_12M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));
    sleep(3);

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_AddVideoPipe(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Single Video\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_1", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    sleep(5);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));

    /* stop vcap 0 not need to stop isp */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 double Video\033[0;39m\n\n");
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    /* restart Sensor */
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_RESTART_SENSOR, HI_NULL, 0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));
    /* exit standby */
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    memset(aszSaveName, 0x00, 64);
    sprintf(aszSaveName, "%s_Scene_2", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, -1));

    sleep(5);
exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);
    /* stop venc 1 */
    HI_MAPI_VENC_Stop(hVencHdl1);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl1);
    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);
    /* stop vproc 1 */
    HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl1);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopISP(hVcapPipe1);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_DelVideoPipe(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32SnapCnt = 0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 double Video\033[0;39m\n\n");
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_1", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, -1));
    sleep(5);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop venc 1 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl1));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl1));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    /* stop vproc 1 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl1));

    CHECK_RET(HI_MAPI_VCAP_StopISP(hVcapPipe1));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0));

    /* stop vcap 0 */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 single Video\033[0;39m\n\n");
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* init VB and media */
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_RESTART_SENSOR, HI_NULL, 0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc callback */
    memset(aszSaveName, 0x00, 64);
    sprintf(aszSaveName, "%s_Scene_2", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));
    sleep(5);
exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_AddSnapPipe(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32SnapCnt = 0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    /* SET VI */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_1", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));
    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    sleep(3);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    /* stop vcap 0 */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video + Snap\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_SNAP;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_RESTART_SENSOR, HI_NULL, 0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_2", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    memcpy(&stVencAttr, &g_stVenc_4K30FPS_JPEG, sizeof(HI_MAPI_VENC_ATTR_S));
    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, 1));

    stSnapAttr.bLoadCCM = HI_FALSE;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;

    CHECK_RET(HI_MAPI_VCAP_SetSnapAttr(hVcapPipe1, &stSnapAttr));
    CHECK_RET(COMM_SnapTrigger(hVcapPipe1, hVencHdl1, stSnapAttr.stNormalAttr.u32FrameCnt));
    sleep(3);

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop venc 1 */
    HI_MAPI_VENC_Stop(hVencHdl1);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl1);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vproc 1 */
    HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl1);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl1);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopISP(hVcapPipe1);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0);
    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_DelSnapPipe(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32SnapCnt = 0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video + Snap\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_SNAP;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));

    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_1", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    memcpy(&stVencAttr, &g_stVenc_4K30FPS_JPEG, sizeof(HI_MAPI_VENC_ATTR_S));
    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, 1));

    stSnapAttr.bLoadCCM = HI_FALSE;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;

    CHECK_RET(HI_MAPI_VCAP_SetSnapAttr(hVcapPipe1, &stSnapAttr));
    CHECK_RET(COMM_SnapTrigger(hVcapPipe1, hVencHdl1, stSnapAttr.stNormalAttr.u32FrameCnt));
    sleep(3);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop venc 1 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl1));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl1));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    /* stop vproc 1 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl1));

    /* Stop vi */
    CHECK_RET(HI_MAPI_VCAP_StopISP(hVcapPipe1));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, &g_stSensor_4K30FPS_imx477, sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].stIspPubAttr.f32FrameRate = SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE;
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_RESTART_SENSOR, HI_NULL, 0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    /* venc callback */
    sprintf(aszSaveName, "%s_Scene_2", __FUNCTION__);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));
    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));
    sleep(3);

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_Standby_8MTo12M(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_S32 s32SnapCnt = 0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;
    HI_S32 s32ModeCnt;
    HI_CHAR aszSaveName[64] = {0};

    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video + Snap\033[0;39m\n\n");
    /* 8M */
    s32ModeCnt = 0;
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_SNAP;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc callback */
    sprintf(aszSaveName, "%s_8M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl1], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, 1));

    stSnapAttr.bLoadCCM = HI_FALSE;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;

    CHECK_RET(HI_MAPI_VCAP_SetSnapAttr(hVcapPipe1, &stSnapAttr));
    CHECK_RET(COMM_SnapTrigger(hVcapPipe1, hVencHdl1, stSnapAttr.stNormalAttr.u32FrameCnt));
    sleep(3);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop venc 1 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl1));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl1));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    /* stop vproc 1 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl1));

    CHECK_RET(HI_MAPI_VCAP_StopISP(hVcapPipe1));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0));
    /* stop vcap 0 */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_DeinitSensor(hVcapDev0));

    s32ModeCnt = 1;
    printf("\n\r\033[1;34m--->>>sony_imx477 12M@30 Video\033[0;39m\n\n");

    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));
    /* venc callback */
    sprintf(aszSaveName, "%s_12M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));
    sleep(3);

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_S32 Sample_Standby_12MTo8M(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    HI_HANDLE hVcapPipe1 = VCAP_PIPE_1;
    HI_HANDLE hPipeChn1 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start vproc 1 */
    HI_HANDLE hVpssHdl1 = 1;
    HI_HANDLE hVportHdl1 = 1;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;
    /* start venc 1 */
    HI_HANDLE hVencHdl1 = 1;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;

    HI_S32 s32ModeCnt;
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};
    s32ModeCnt = 1;
    printf("\n\r\033[1;34m--->>>sony_imx477 12M@30 Video\033[0;39m\n\n");
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe1] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = 1;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));
    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));
    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* venc callback */
    sprintf(aszSaveName, "%s_12M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    sleep(3);
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_ENTER_STANDBY, HI_NULL, 0));

    /* stop venc 0 */
    CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
    CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

    /* stop vproc 0 */
    CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
    CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

    CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
    /* stop vcap 0 */
    CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_DeinitSensor(hVcapDev0));
    s32ModeCnt = 0;
    printf("\n\r\033[1;34m--->>>sony_imx477 8M@30 Video + Snap\033[0;39m\n\n");
    /* sensor0 attr */
    memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 2;
    stVcapAttr.aPipeIdBind[0] = hVcapPipe0;
    stVcapAttr.aPipeIdBind[1] = hVcapPipe1;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Pipe1 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
           sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[hVcapPipe1].enPipeType = HI_MAPI_PIPE_TYPE_SNAP;

    /* pipe0-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* pipe1-Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe1].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe1, hPipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe1));
    CHECK_RET(HI_MAPI_VCAP_SetAttrEx(hVcapPipe0, hPipeChn0, HI_MAPI_VCAP_CMD_EXIT_STANDBY, HI_NULL, 0));

    memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
    memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

    /* set vport0 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

    /* set vport1 attr */
    CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl1, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe1, hPipeChn0, hVpssHdl1));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl1, hVportHdl1, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl1, hVportHdl1));

    /* venc callback */
    sprintf(aszSaveName, "%s_12M_%d", __FUNCTION__, s32ModeCnt);
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)aszSaveName;
    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc0 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

    memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl1], sizeof(HI_MAPI_VENC_ATTR_S));

    /* venc1 */
    CHECK_RET(HI_MAPI_VENC_Init(hVencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE));
    CHECK_RET(HI_MAPI_VENC_Start(hVencHdl1, 1));

    stSnapAttr.bLoadCCM = HI_FALSE;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;

    CHECK_RET(HI_MAPI_VCAP_SetSnapAttr(hVcapPipe1, &stSnapAttr));
    CHECK_RET(COMM_SnapTrigger(hVcapPipe1, hVencHdl1, stSnapAttr.stNormalAttr.u32FrameCnt));
    sleep(3);

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop venc 1 */
    HI_MAPI_VENC_Stop(hVencHdl1);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl1, hVportHdl1, hVencHdl1, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl1, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl1);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    /* stop vproc 1 */
    HI_MAPI_VPROC_StopPort(hVpssHdl1, hVportHdl1);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe1, hPipeChn0, hVpssHdl1);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl1);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopISP(hVcapPipe1);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);
    HI_MAPI_VCAP_StopChn(hVcapPipe1, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();
    return s32Ret;
}
HI_S32 Sample_SwitchVIVPSSMode()
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* start vcap 0 */
    HI_HANDLE hVcapDev0 = VCAP_DEV_0;
    HI_HANDLE hVcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE hPipeChn0 = PIPE_CHN_0;
    /* start vproc 0 */
    HI_HANDLE hVpssHdl0 = 0;
    HI_HANDLE hVportHdl0 = 0;
    /* start venc 0 */
    HI_HANDLE hVencHdl0 = 0;

    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB = { HI_NULL, HI_NULL };
    /* flag */
    HI_BOOL bFirst = HI_TRUE;
    HI_BOOL bStandBy = HI_FALSE;
    HI_S32 s32ModeCnt = 0;
    /* File Name */
    HI_CHAR aszSaveName[64] = {0};

    while (1) {
        /* sensor0 attr */
        memcpy(&stVcapSensorAttr, g_pstSensorAttr[s32ModeCnt], sizeof(HI_MAPI_SENSOR_ATTR_S));

        /* dev0 attr */
        memcpy(&stVcapAttr.stVcapDevAttr, g_pstDevAttr[s32ModeCnt], sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
        stVcapAttr.u32PipeBindNum = 1;
        stVcapAttr.aPipeIdBind[0] = hVcapPipe0;

        /* Pipe0 attr */
        memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0], g_pstPipeAttr[s32ModeCnt][hVcapPipe0],
               sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
        stVcapAttr.astVcapPipeAttr[hVcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

        /* Chn0 attr */
        memcpy(&stVcapAttr.astVcapPipeAttr[hVcapPipe0].astPipeChnAttr[hPipeChn0], g_pstChnAttr[s32ModeCnt][hPipeChn0],
               sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

        if (bFirst) {
            /* VB and media Attr */
            memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
            memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
            stSampleCommAttr.stResolution.u32Width = 4000;
            stSampleCommAttr.stResolution.u32Height = 3000;
            stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
            stSampleCommAttr.u8SnsCnt = 1;
        } else {
            printf("\n\r\033[1;34m--->>>sony_imx477 8M@30fps sitch VI-VPSS Mode\033[0;39m\n\n");
            /* VB and media Attr */
            memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
            memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
            stSampleCommAttr.stResolution.u32Width = 4000;
            stSampleCommAttr.stResolution.u32Height = 3000;
            stSampleCommAttr.stViVpssMode.aenMode[hVcapPipe0] = VI_ONLINE_VPSS_OFFLINE;
            stSampleCommAttr.u8SnsCnt = 1;
        }

        /* init VB and media */
        CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

        CHECK_RET(HI_MAPI_VCAP_InitSensor(hVcapDev0, &stVcapSensorAttr));
        CHECK_RET(HI_MAPI_VCAP_SetAttr(hVcapDev0, &stVcapAttr));
        CHECK_RET(HI_MAPI_VCAP_StartDev(hVcapDev0));
        CHECK_RET(HI_MAPI_VCAP_StartChn(hVcapPipe0, hPipeChn0));
        CHECK_RET(HI_MAPI_VCAP_StartISP(hVcapPipe0));

        /* venc callback */
        sprintf(aszSaveName, "%s_Mode_%d", __FUNCTION__, s32ModeCnt);
        stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
        stVencCB.pPrivateData = (HI_VOID *)aszSaveName;

        memcpy(&stVpssAttr, g_pstVpssAttr[s32ModeCnt][hVpssHdl0], sizeof(HI_MAPI_VPSS_ATTR_S));
        memcpy(&stVPortAttr, g_pstVportAttr[s32ModeCnt][hVportHdl0], sizeof(HI_MAPI_VPORT_ATTR_S));

        /* set vport0 attr */
        CHECK_RET(HI_MAPI_VPROC_InitVpss(hVpssHdl0, &stVpssAttr));
        CHECK_RET(HI_MAPI_VPROC_BindVcap(hVcapPipe0, hPipeChn0, hVpssHdl0));
        CHECK_RET(HI_MAPI_VPROC_SetPortAttr(hVpssHdl0, hVportHdl0, &stVPortAttr));
        CHECK_RET(HI_MAPI_VPROC_StartPort(hVpssHdl0, hVportHdl0));

        memcpy(&stVencAttr, g_pstVencAttr[s32ModeCnt][hVencHdl0], sizeof(HI_MAPI_VENC_ATTR_S));
        /* venc0 */
        CHECK_RET(HI_MAPI_VENC_Init(hVencHdl0, &stVencAttr));
        CHECK_RET(HI_MAPI_VENC_RegisterCallback(hVencHdl0, &stVencCB));
        CHECK_RET(HI_MAPI_VENC_BindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
        CHECK_RET(HI_MAPI_VENC_Start(hVencHdl0, -1));

        sleep(5);

        if (!bFirst) {
            printf("ready to exit scene\n");
            break;
        }
        s32ModeCnt++;
        bFirst = HI_FALSE;

        /* stop venc 0 */
        CHECK_RET(HI_MAPI_VENC_Stop(hVencHdl0));
        CHECK_RET(HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE));
        CHECK_RET(HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB));
        CHECK_RET(HI_MAPI_VENC_Deinit(hVencHdl0));

        /* stop vproc 0 */
        CHECK_RET(HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0));
        CHECK_RET(HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0));
        CHECK_RET(HI_MAPI_VPROC_DeinitVpss(hVpssHdl0));

        CHECK_RET(HI_MAPI_VCAP_StopISP(hVcapPipe0));
        CHECK_RET(HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0));
        CHECK_RET(HI_MAPI_VCAP_StopDev(hVcapDev0));
        CHECK_RET(HI_MAPI_VCAP_DeinitSensor(hVcapDev0));
        /* stop media */
        CHECK_RET(SAMPLE_COMM_Deinit());
    }

exit:
    /* stop venc 0 */
    HI_MAPI_VENC_Stop(hVencHdl0);
    HI_MAPI_VENC_UnBindVProc(hVpssHdl0, hVportHdl0, hVencHdl0, HI_FALSE);
    HI_MAPI_VENC_UnRegisterCallback(hVencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(hVencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(hVpssHdl0, hVportHdl0);
    HI_MAPI_VPROC_UnBindVCap(hVcapPipe0, hPipeChn0, hVpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(hVpssHdl0);

    HI_MAPI_VCAP_StopISP(hVcapPipe0);
    HI_MAPI_VCAP_StopChn(hVcapPipe0, hPipeChn0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopDev(hVcapDev0);
    HI_MAPI_VCAP_DeinitSensor(hVcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

HI_VOID Sample_StandbyScene_Usage()
{
    printf("Usage : \n");
    printf("\t 0) Sony_imx477 sensor switch mode.\n");
    printf("\t 1) Sony_imx477 8M@30fps single video pipe -> 8M@30fps double video pipe.\n");
    printf("\t 2) Sony_imx477 8M@30fps double video pipe -> 8M@30fps single video pipe.\n");
    printf("\t 3) Sony_imx477 8M@30fps single video pipe -> 8M@30fps video pipe + snap pipe.\n");
    printf("\t 4) Sony_imx477 8M@30fps video pipe + snap pipe -> 8M@30fps single video pipe.\n");
    printf("\t 5) Sony_imx477 8M@30fps snap pipe + video pipe -> 12M15fps video pipe.\n");
    printf("\t 6) Sony_imx477 12M15fps video pipe -> 8M@30fps snap pipe + video pipe.\n");
    printf("\t 7) Sony_imx477 change VI-VPSS Mode, Need to stop ISP.\n");
    printf("\t q/Q) quit\n");
    return;
}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret;
    HI_CHAR ch;

    // signal(SIGINT, SAMPLE_VENC_HandleSig);
    // signal(SIGTERM, SAMPLE_VENC_HandleSig);

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Init();

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_Sys_Init fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

#endif

    Sample_StandbyScene_Usage();
    ch = (char)getchar();
    (void)getchar();

    switch (ch) {
        case '0':
            s32Ret = Sample_SwitchSensorMode();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_SwitchSensorMode fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;

        case '1':
            s32Ret = Sample_AddVideoPipe();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_AddVideoPipe fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;
        case '2':
            s32Ret = Sample_DelVideoPipe();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_DelVideoPipe fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;

        case '3':
            s32Ret = Sample_AddSnapPipe();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_AddSnapPipe fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;
        case '4':
            s32Ret = Sample_DelSnapPipe();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_DelSnapPipe fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;

        case '5':
            s32Ret = Sample_Standby_8MTo12M();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_Standby_8MTo12M fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;
        case '6':
            s32Ret = Sample_Standby_12MTo8M();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_Standby_12MTo8M fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;

        case '7':
            s32Ret = Sample_SwitchVIVPSSMode();

            if (s32Ret != HI_SUCCESS) {
                printf("Sample_SwitchVIVPSSMod fail s32Ret:%x\n", s32Ret);
                return HI_FAILURE;
            }
            break;

        default:
            printf("the index is invaild!\n");
            Sample_StandbyScene_Usage();
            return HI_FAILURE;
    }

#ifdef __DualSys__
    s32Ret = HI_MAPI_Sys_Deinit();
#endif

    if (s32Ret == HI_SUCCESS) {
        printf("The program exited successfully!\n");
    } else {
        printf("The program exits abnormally!\n");
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
