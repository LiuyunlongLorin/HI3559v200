/**
* @file    hi_photomng_testcase.c
* @brief
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/22
* @version 1.0

*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>


#include "hi_buffer.h"
#include "hi_mapi_sys.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_disp.h"
#include "hi_mapi_hdmi.h"
#include "hi_appcomm_util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define RES_WIDTH  (1920)
#define RES_HEIGHT (1080)

#define RAWCAP_RAW_DEPTH  (3)


/**Dicided by MediaCfg*/
#define UT_VCAPPIPE_HANDLE_SNAP (1)
#define UT_VCAPPIPECHN_HANDLE_SNAP (0)
#define UT_VPROC_HANDLE_SNAP (1)
#define UT_VPORT_HANDLE_SNAP (0)

#define VcapVideoPipeHdl (0)
#define VcapSnapPipeHdl  (1)
#define VpssVideoHdl     (0)
#define VpssSnapHdl      (1)
#define VencSanpHdl      (0)
#define VencVideoHdl     (1)
#define VencThmbHdl      (2)
#define VoDevHdl         (0)
#define VoDevWndHdl      (0)


#ifndef CHECK_RET
#define CHECK_RET(express)\
    do{\
        s32Ret = express;\
        if (HI_SUCCESS != s32Ret)\
        {\
            printf("return [0x%08X]\n",s32Ret);\
            goto exit;\
        }\
    }while(0)
#endif

static HI_VOID SAMPLE_DoublePipeSnap_Deinit(HI_VOID)
{
    HI_HANDLE VcapDev0 = 0;
    HI_HANDLE PipeChn0 = 0;

    HI_HANDLE VPortHdl0 = 0;

    HI_BOOL   bStitch = HI_FALSE;

    /* stop venc*/
    HI_MAPI_VENC_Stop(VencSanpHdl);
    HI_MAPI_VENC_UnBindVProc(VpssSnapHdl, VPortHdl0, VencSanpHdl, bStitch);
    HI_MAPI_VENC_Deinit(VencSanpHdl);

    HI_MAPI_VENC_Stop(VencVideoHdl);
    HI_MAPI_VENC_UnBindVProc(VpssVideoHdl, VPortHdl0, VencVideoHdl, bStitch);
    HI_MAPI_VENC_Deinit(VencVideoHdl);

    HI_MAPI_VENC_Stop(VencThmbHdl);
    HI_MAPI_VENC_UnBindVProc(VpssVideoHdl, VPortHdl0, VencThmbHdl, bStitch);
    HI_MAPI_VENC_Deinit(VencThmbHdl);

    HI_MAPI_DISP_StopWindow(VoDevHdl, VoDevWndHdl);
    HI_MAPI_DISP_UnBind_VProc(VpssVideoHdl, VPortHdl0, VoDevHdl, VoDevWndHdl, 0);
    HI_MAPI_DISP_Stop(VoDevHdl);
    HI_MAPI_DISP_Deinit(VoDevHdl);
    /* stop vproc */
    HI_MAPI_VPROC_StopPort(VpssVideoHdl, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapVideoPipeHdl, PipeChn0, VpssVideoHdl);
    HI_MAPI_VPROC_DeinitVpss(VpssVideoHdl);

    HI_MAPI_VPROC_StopPort(VpssSnapHdl, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapVideoPipeHdl, PipeChn0, VpssSnapHdl);
    HI_MAPI_VPROC_DeinitVpss(VpssSnapHdl);

    /* stop vcap*/
    HI_MAPI_VCAP_StopISP(VcapVideoPipeHdl);
    HI_MAPI_VCAP_StopChn(VcapVideoPipeHdl, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);

    /* stop media */
    HI_MAPI_Media_Deinit();
    HI_MAPI_Sys_Deinit();

}

static HI_S32 SAMPLE_DoublePipeSnap_Init(HI_U32 u32Width, HI_U32 u32Height, HI_S32 s32Fps)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    printf("\n----double PIPE Snap + HDMI preview----\n\n");
    CHECK_RET(HI_MAPI_Sys_Init());
    /*init VB and media*/
    HI_MAPI_MEDIA_ATTR_S stMediaAttr;
    memset(&stMediaAttr, 0, sizeof(HI_MAPI_MEDIA_ATTR_S));
    HI_S32 s32SnsCnt = 1;
    HI_U32 u32BlkSize;
    u32BlkSize = COMMON_GetPicBufferSize(u32Width, u32Height, \
                                         enPixelFormat, DATA_BITWIDTH_10, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stMediaAttr.stMediaConfig.stVbConfig.u32MaxPoolCnt = 3;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u64BlkSize  = u32BlkSize;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[0].u32BlkCnt   = 10 * s32SnsCnt;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u64BlkSize  = u32BlkSize;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[1].u32BlkCnt   = 20 * s32SnsCnt;
    u32BlkSize = VI_GetRawBufferSize(u32Width, u32Height, \
                                     PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u64BlkSize  = u32BlkSize;
    stMediaAttr.stMediaConfig.stVbConfig.astCommPool[2].u32BlkCnt   = 10;
    stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;
    stMediaAttr.stMediaConfig.stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE;
    stMediaAttr.stVencModPara.u32H264eLowPowerMode = 1;
    stMediaAttr.stVencModPara.u32H265eLowPowerMode = 1;

    CHECK_RET(HI_MAPI_Media_Init(&stMediaAttr));
    printf("\n----Media Init----\n\n");
    /**************************start vcap *******************************/
    HI_HANDLE  VcapDev0 = 0;
    HI_HANDLE  PipeChn0 = 0;

    /* start sensor*/
    HI_MAPI_SENSOR_ATTR_S stSnsAttr;
    stSnsAttr.u8SnsMode = 0;
    stSnsAttr.enWdrMode = WDR_MODE_NONE;
    stSnsAttr.stSize.u32Width = u32Width;
    stSnsAttr.stSize.u32Height = u32Height;
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev0, &stSnsAttr));

    HI_MAPI_VCAP_ATTR_S  stVcapAttr = {0};
    /* dev0 attr*/
    stVcapAttr.stVcapDevAttr.stBasSize.u32Width = u32Width;
    stVcapAttr.stVcapDevAttr.stBasSize.u32Height = u32Height;
    stVcapAttr.stVcapDevAttr.enWdrMode = stSnsAttr.enWdrMode;
    stVcapAttr.stVcapDevAttr.u32CacheLine = 0; /**not wdr online, the value not used*/

    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapVideoPipeHdl;
    stVcapAttr.aPipeIdBind[1] = VcapSnapPipeHdl;
    /* Pipe0 attr*/
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stFrameRate.s32SrcFrameRate = -1;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stFrameRate.s32DstFrameRate = -1;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].enCompressMode = COMPRESS_MODE_NONE;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].enPipeBypassMode = VI_PIPE_BYPASS_NONE;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].bIspBypass = HI_FALSE;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stIspPubAttr.f32FrameRate = s32Fps;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stIspPubAttr.enBayer = BAYER_RGGB;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stIspPubAttr.enSnsMirrorFlip = ISP_SNS_NORMAL;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stIspPubAttr.stSize.u32Width = u32Width;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].stIspPubAttr.stSize.u32Height = u32Height;
    /* pipe0-Chn0 attr*/
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].stDestSize.u32Width = u32Width;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].stDestSize.u32Height = u32Height;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].stFrameRate.s32SrcFrameRate = s32Fps;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].stFrameRate.s32DstFrameRate = s32Fps;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].enCompressMode = COMPRESS_MODE_NONE;
    stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl].astPipeChnAttr[PipeChn0].enPixelFormat = enPixelFormat;

    /* Pipe1 attr*/
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapSnapPipeHdl], &stVcapAttr.astVcapPipeAttr[VcapVideoPipeHdl], sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapSnapPipeHdl].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr));

    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev0));
    printf("\n----HI_MAPI_VCAP_StartDev----\n\n");
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapVideoPipeHdl, PipeChn0));
    printf("\n----HI_MAPI_VCAP_StartChn----\n\n");
    CHECK_RET(HI_MAPI_VCAP_StartISP(VcapVideoPipeHdl));
    printf("\n----double PIPE Snap + preview--StartVproc-\n\n");
    /**************************start vproc *******************************/
    /*vproc 0*/
    HI_HANDLE  VPortHdl0 = 0;
    HI_BOOL    bStitch = HI_FALSE;

    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    stVpssAttr.u32MaxW = u32Width;
    stVpssAttr.u32MaxH = u32Height;
    stVpssAttr.stFrameRate.s32SrcFrameRate = s32Fps;
    stVpssAttr.stFrameRate.s32DstFrameRate = s32Fps;
    stVpssAttr.enPixelFormat = enPixelFormat;
    stVpssAttr.bNrEn = HI_FALSE;
    stVpssAttr.stNrAttr.enCompressMode = COMPRESS_MODE_NONE;
    stVpssAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;

    HI_MAPI_VPORT_ATTR_S stVPortAttr;
    stVPortAttr.u32Width = u32Width;
    stVPortAttr.u32Height = u32Height;
    stVPortAttr.stFrameRate.s32SrcFrameRate = s32Fps;
    stVPortAttr.stFrameRate.s32DstFrameRate = s32Fps;
    stVPortAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVPortAttr.enPixelFormat = enPixelFormat;
    stVPortAttr.enCompressMode = COMPRESS_MODE_NONE;

    /*vproc 0*/
    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssVideoHdl, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapVideoPipeHdl, PipeChn0, VpssVideoHdl));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssVideoHdl, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssVideoHdl, VPortHdl0));
    /*vproc 1*/
    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssSnapHdl, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapVideoPipeHdl, PipeChn0, VpssSnapHdl));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssSnapHdl, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssSnapHdl, VPortHdl0));

    printf("\n----double PIPE Snap + HDMI preview--StartVo-\n\n");
    /**************************start preview *******************************/
    HI_MAPI_DISP_ATTR_S stDispAttr = {0};
    HI_MAPI_DISP_WINDOW_ATTR_S stDispWindowAttr = {0};
    HI_MAPI_DISP_VIDEOLAYER_ATTR_S stDispVideoLayer = {0};

    stDispAttr.stUserInfoAttr.u32DevFrameRate = 60;
    stDispAttr.stUserInfoAttr.stUserInfo.bClkReverse = HI_FALSE;
    stDispAttr.stUserInfoAttr.stUserInfo.u32DevDiv = 3;
    stDispAttr.stUserInfoAttr.stUserInfo.u32PreDiv = 1;

    stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.enClkSource = VO_CLK_SOURCE_LCDMCLK;
    stDispAttr.stUserInfoAttr.stUserInfo.stUserIntfSyncAttr.u32LcdMClkDiv = 0x1CF62C;

    stDispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    stDispAttr.stPubAttr.enIntfType = VO_INTF_LCD_6BIT;
    stDispAttr.stPubAttr.stSyncInfo.bSynm = 1; /**<sync mode: signal */
    stDispAttr.stPubAttr.stSyncInfo.bIop  = 1; /**<progressive display */
    stDispAttr.stPubAttr.stSyncInfo.u16Vact = 320;
    stDispAttr.stPubAttr.stSyncInfo.u16Vbb  = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Vfb  = 4;
    stDispAttr.stPubAttr.stSyncInfo.u16Hact = 240;
    stDispAttr.stPubAttr.stSyncInfo.u16Hbb  = 30;
    stDispAttr.stPubAttr.stSyncInfo.u16Hfb  = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Hpw  = 10;
    stDispAttr.stPubAttr.stSyncInfo.u16Vpw  = 2;
    stDispAttr.stPubAttr.stSyncInfo.bIdv = 0;
    stDispAttr.stPubAttr.stSyncInfo.bIhs = 0;
    stDispAttr.stPubAttr.stSyncInfo.bIvs = 0;

    stDispWindowAttr.stRect.s32X = 0;
    stDispWindowAttr.stRect.s32Y = 0;
    stDispWindowAttr.stRect.u32Width = 240;
    stDispWindowAttr.stRect.u32Height = 320;
    stDispWindowAttr.u32Priority = 0;

    stDispVideoLayer.stImageSize.u32Width = 240;
    stDispVideoLayer.stImageSize.u32Height = 320;
    stDispVideoLayer.u32BufLen = 3;
    stDispVideoLayer.u32VLFrameRate = 30;
    CHECK_RET(HI_MAPI_DISP_Init(VoDevHdl, &stDispAttr));
    CHECK_RET(HI_MAPI_DISP_Start(VoDevHdl, &stDispVideoLayer));
    CHECK_RET(HI_MAPI_DISP_SetWindowAttr(VoDevHdl, VoDevWndHdl, &stDispWindowAttr));
    CHECK_RET(HI_MAPI_DISP_Bind_VProc(VpssVideoHdl, VPortHdl0, VoDevHdl, VoDevWndHdl, 0));
    CHECK_RET(HI_MAPI_DISP_StartWindow(VoDevHdl, VoDevWndHdl));

    /**************************start venc *******************************/
    HI_MAPI_VENC_ATTR_S stVencAttr;
    stVencAttr.stVencPloadTypeAttr.enType = HI_MAPI_PAYLOAD_TYPE_JPEG;
    stVencAttr.stVencPloadTypeAttr.u32Width = u32Width;
    stVencAttr.stVencPloadTypeAttr.u32Height = u32Height;
    stVencAttr.stVencPloadTypeAttr.u32BufSize = u32Width * u32Height * 2;
    stVencAttr.stVencPloadTypeAttr.u32Profile = 0;
    stVencAttr.stVencPloadTypeAttr.enSceneMode = HI_MAPI_VENC_SCENE_MODE_NORMAL;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.bEnableDCF = HI_TRUE;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum = 2;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[0].u32Width = 1280;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[0].u32Height = 720;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[1].u32Width = 1024;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.astLargeThumbNailSize[1].u32Height = 576;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.u32Qfactor = 90;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.enJpegEncodeMode = JPEG_ENCODE_SNAP;
    stVencAttr.stRcAttr.enRcMode = HI_MAPI_VENC_RC_MODE_CBR;
    stVencAttr.stRcAttr.stAttrCbr.u32Gop = s32Fps;
    stVencAttr.stRcAttr.stAttrCbr.u32SrcFrameRate = s32Fps;
    stVencAttr.stRcAttr.stAttrCbr.fr32DstFrameRate = s32Fps;
    stVencAttr.stRcAttr.stAttrCbr.u32StatTime = 2;
    stVencAttr.stRcAttr.stAttrCbr.u32BitRate = 5120;

    /*venc 0*/
    CHECK_RET(HI_MAPI_VENC_Init(VencSanpHdl, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssSnapHdl, VPortHdl0, VencSanpHdl, bStitch));

    /*venc 1*/
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.enJpegEncodeMode = JPEG_ENCODE_ALL;
    CHECK_RET(HI_MAPI_VENC_Init(VencVideoHdl, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssVideoHdl, VPortHdl0, VencVideoHdl, bStitch));

    /*venc 2 for thm*/
    stVencAttr.stVencPloadTypeAttr.u32Width = 160;
    stVencAttr.stVencPloadTypeAttr.u32Height = 120;
    stVencAttr.stVencPloadTypeAttr.u32BufSize = 160 * 120 * 2;
    stVencAttr.stVencPloadTypeAttr.stAttrJpege.stAttrMPF.u8LargeThumbNailNum = 0;
    CHECK_RET(HI_MAPI_VENC_Init(VencThmbHdl, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssVideoHdl, VPortHdl0, VencThmbHdl, bStitch));
    printf("\n----double PIPE Snap + LCD preview--End--\n\n");
    return s32Ret;
exit:
    SAMPLE_DoublePipeSnap_Deinit();
    return s32Ret;
}

HI_VOID SAMPLE_VIO_Usage()
{
    printf("Usage : \n");
    printf("\t 0) VIO RGB LCD preview.\n");
    return;
}

int main(int argc, char* argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 SampleId;

    SAMPLE_VIO_Usage();
    SampleId = (char) getchar();
    (void)getchar();
    switch (SampleId)
    {
        case '0':
            s32Ret = SAMPLE_DoublePipeSnap_Init(RES_WIDTH, RES_HEIGHT, 30);
            break;

        //case '1':
        //    s32Ret = SAMPLE_DoublePipeSnap_Init(RES_WIDTH, RES_HEIGHT, 30);
        //    break;

        default:
            printf("the index is invaild!\n");
            SAMPLE_VIO_Usage();
            return HI_FAILURE;
    }
     printf("Press Enter key to stop preview...\n");
    (void)getchar();
    SAMPLE_DoublePipeSnap_Deinit();
    return s32Ret;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

