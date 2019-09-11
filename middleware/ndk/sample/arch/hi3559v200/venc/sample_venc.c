/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_venc.c
 * @brief   sample venc function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_VOID SAMPLE_VENC_Usage()
{
    printf("Usage : \n");
    printf("\t 0) 4K@30fps H.264 record + 1080p@30fps H.265 record + 1080p HDMI Preview.\n");
    printf("\t 1) 4K@30fps JPEG single snap + 1080p HDMI Preview.\n");
    printf("\t 2) record adapt sensor config + 1080p HDMI Preview.\n");
    printf("\t q/Q) quit\n");
    return;
}

HI_S32 SAMPLE_ExitMapi(void)
{
    HI_S32 s32Ret = 0;
    HI_HANDLE VencHdl0 = 0;
    HI_HANDLE VpssHdl0 = 0;
    HI_HANDLE VPortHdl0 = 0;
    HI_HANDLE VcapPipe0 = 0;
    HI_HANDLE PipeChn0 = 0;
    HI_HANDLE VcapDev0 = 0;

    /* stop venc 0 */
    HI_MAPI_VENC_Stop(VencHdl0);
    HI_MAPI_VENC_UnBindVProc(VpssHdl0, VPortHdl0, VencHdl0, HI_FALSE);
    HI_MAPI_VENC_Deinit(VencHdl0);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe0, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);

    /* stop media */
    HI_MAPI_Media_Deinit();

    return 0;
}

void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo) {
        SAMPLE_ExitMapi();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }

    exit(-1);
}

/******************************************************************************
* function : record adapt sensor config .
******************************************************************************/
HI_S32 SAMPLE_VENC_Record(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8SnsCnt = 1;
    HI_HANDLE VcapDev0 = VCAP_DEV_0;
    HI_HANDLE VcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE PipeChn0 = PIPE_CHN_0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    SAMPLE_SENSOR_SEQ_E enSensorSeq = SAMPLE_SENSOR_1080P30;

    HI_HANDLE VpssHdl0 = 0;
    HI_HANDLE VPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    HI_HANDLE VencHdl0 = 0;
    HI_HANDLE VencHdl1 = 1;
    HI_S32 s32FrameCnt = -1;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB;
    SAMPLE_MODE_COMM_CONFIG_S stCommConfig;

    memset(&stVencCB, 0, sizeof(HI_MAPI_VENC_CALLBACK_S));

    /* sensor0 and media cfg */
    CHECK_RET(SAMPLE_VENC_Get_Media_Mode(&enSensorSeq, &stVcapSensorAttr));
    CHECK_RET(MAPI_SAMPLE_COMM_GetMediaCFG(enSensorSeq, &stCommConfig));
    if (enSensorSeq == SAMPLE_SENSOR_4K30) {
        printf("\n----4K@30fps H.264 record + 1080p HDMI preview----\n\n");
    } else {
        printf("\n----1080p@30fps H.265 record + 1080p HDMI preview----\n\n");
    }

    /**************************start vcap *******************************/
    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, stCommConfig.pstVcapDevAttr, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0], stCommConfig.pstVcapPipeAttr, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0], stCommConfig.pstPipeChnAttr,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[VcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe0, PipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(VcapPipe0));

    /**************************start vproc *******************************/
    /* Vpss0 attr */
    memcpy(&stVpssAttr, stCommConfig.pstVpssAttr, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, stCommConfig.pstVPortAttr, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapPipe0, PipeChn0, VpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssHdl0, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssHdl0, VPortHdl0));

    /**************************start preview *******************************/

    CHECK_RET(SAMPLE_COMM_StartPreview(VpssHdl0, VPortHdl0, bStitch));

    /**************************start venc *******************************/
    memcpy(&stVencAttr, stCommConfig.pstRecVencAttr, sizeof(HI_MAPI_VENC_ATTR_S));
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)__FUNCTION__;

    CHECK_RET(HI_MAPI_VENC_Init(VencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(VencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch));
    CHECK_RET(HI_MAPI_VENC_Start(VencHdl0, s32FrameCnt));

    printf("Press Enter key to stop record...\n");
    (void)getchar();

exit:

    /* stop venc 0 */
    HI_MAPI_VENC_Stop(VencHdl0);
    HI_MAPI_VENC_UnBindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch);
    HI_MAPI_VENC_UnRegisterCallback(VencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(VencHdl0);

    /* stop preview */
    SAMPLE_COMM_StopPreview(VpssHdl0, VPortHdl0, bStitch);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe0, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

/******************************************************************************
* function : 4K@30fps H.264 record + 1080p@30fps H.265 record + 1080p HDMI preview.
******************************************************************************/
HI_S32 SAMPLE_VENC_TwoRecord(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8SnsCnt = 1;
    HI_HANDLE VcapDev0 = VCAP_DEV_0;
    HI_HANDLE VcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE PipeChn0 = PIPE_CHN_0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;

    HI_HANDLE VpssHdl0 = 0;
    HI_HANDLE VPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    HI_HANDLE VencHdl0 = 0;
    HI_HANDLE VencHdl1 = 1;
    HI_S32 s32FrameCnt = -1;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB;

    memset(&stVencCB, 0, sizeof(HI_MAPI_VENC_CALLBACK_S));

    printf("\n----4K@30fps H.264 record + 1080p@30fps H.265 record + 1080p HDMI preview----\n\n");

    /**************************start vcap *******************************/
    /* sensor0 attr */
    CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[VcapPipe0] = VI_OFFLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe0, PipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(VcapPipe0));

    /**************************start vproc *******************************/
    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapPipe0, PipeChn0, VpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssHdl0, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssHdl0, VPortHdl0));

    /**************************start preview *******************************/

    CHECK_RET(SAMPLE_COMM_StartPreview(VpssHdl0, VPortHdl0, bStitch));

    /**************************start venc *******************************/
    /* venc 0 H264 big stream */
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_VIDEO_H264, sizeof(HI_MAPI_VENC_ATTR_S));
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)__FUNCTION__;

    CHECK_RET(HI_MAPI_VENC_Init(VencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(VencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch));
    CHECK_RET(HI_MAPI_VENC_Start(VencHdl0, s32FrameCnt));

    /* venc 1 H265 little stream */
    memcpy(&stVencAttr, &g_stVenc_1080P30FPS_VIDEO_H265, sizeof(HI_MAPI_VENC_ATTR_S));
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)__FUNCTION__;

    CHECK_RET(HI_MAPI_VENC_Init(VencHdl1, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(VencHdl1, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssHdl0, VPortHdl0, VencHdl1, bStitch));
    CHECK_RET(HI_MAPI_VENC_Start(VencHdl1, s32FrameCnt));

    printf("Press Enter key to stop record...\n");
    (void)getchar();

exit:

    /* stop venc 1 */
    HI_MAPI_VENC_Stop(VencHdl1);
    HI_MAPI_VENC_UnBindVProc(VpssHdl0, VPortHdl0, VencHdl1, bStitch);
    HI_MAPI_VENC_UnRegisterCallback(VencHdl1, &stVencCB);
    HI_MAPI_VENC_Deinit(VencHdl1);

    /* stop venc 0 */
    HI_MAPI_VENC_Stop(VencHdl0);
    HI_MAPI_VENC_UnBindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch);
    HI_MAPI_VENC_UnRegisterCallback(VencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(VencHdl0);

    /* stop preview */
    SAMPLE_COMM_StopPreview(VpssHdl0, VPortHdl0, bStitch);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe0, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

/* 4K@30fps single PIPE Snap + 1080p HDMI preview */
HI_S32 SAMPLE_VENC_SinglePipeSnap(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8SnsCnt = 1;
    HI_U32 u32FrameCnt = 2;
    HI_HANDLE VcapDev0 = VCAP_DEV_0;
    HI_HANDLE VcapPipe0 = VCAP_PIPE_0;
    HI_HANDLE PipeChn0 = PIPE_CHN_0;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    HI_MAPI_SENSOR_ATTR_S stVcapSensorAttr;
    SAMPLE_COMM_INITATTR_S stSampleCommAttr;
    HI_MAPI_VCAP_SNAP_ATTR_S stSnapAttr;

    HI_HANDLE VpssHdl0 = 0;
    HI_HANDLE VPortHdl0 = 0;
    HI_BOOL bStitch = HI_FALSE;
    HI_MAPI_VPSS_ATTR_S stVpssAttr;
    HI_MAPI_VPORT_ATTR_S stVPortAttr;

    HI_HANDLE VencHdl0 = 0;
    HI_S32 s32FrameCnt = u32FrameCnt;
    HI_MAPI_VENC_ATTR_S stVencAttr;
    HI_MAPI_VENC_CALLBACK_S stVencCB;

    memset(&stVencCB, 0, sizeof(HI_MAPI_VENC_CALLBACK_S));

    printf("\n----4K@30fps single PIPE Snap + 1080p HDMI preview----\n\n");

    /**************************start vcap *******************************/
    /* sensor0 attr */
    CHECK_RET(SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_4K30, &stVcapSensorAttr));

    /* dev0 attr */
    memcpy(&stVcapAttr.stVcapDevAttr, &g_stDev_4K, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));
    stVcapAttr.u32PipeBindNum = 1;
    stVcapAttr.aPipeIdBind[0] = VcapPipe0;

    /* Pipe0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0], &g_stPipe_4K30FPS, sizeof(HI_MAPI_VCAP_PIPE_ATTR_S));
    stVcapAttr.astVcapPipeAttr[VcapPipe0].enPipeType = HI_MAPI_PIPE_TYPE_VIDEO;

    /* Chn0 attr */
    memcpy(&stVcapAttr.astVcapPipeAttr[VcapPipe0].astPipeChnAttr[PipeChn0], &g_stChn_4K30FPS,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    /* VB and media Attr */
    memset(&stSampleCommAttr, 0, sizeof(SAMPLE_COMM_INITATTR_S));
    memset(&stSampleCommAttr.stViVpssMode, 0, sizeof(VI_VPSS_MODE_S));
    stSampleCommAttr.stResolution.u32Width = stVcapSensorAttr.stSize.u32Width;
    stSampleCommAttr.stResolution.u32Height = stVcapSensorAttr.stSize.u32Height;
    stSampleCommAttr.stViVpssMode.aenMode[VcapPipe0] = VI_ONLINE_VPSS_OFFLINE;
    stSampleCommAttr.u8SnsCnt = u8SnsCnt;

    /* init VB and media */
    CHECK_RET(SAMPLE_COMM_Init(stSampleCommAttr, VB_MODE_1));

    /* vcap */
    CHECK_RET(HI_MAPI_VCAP_InitSensor(VcapDev0, &stVcapSensorAttr));
    CHECK_RET(HI_MAPI_VCAP_SetAttr(VcapDev0, &stVcapAttr));
    CHECK_RET(HI_MAPI_VCAP_StartDev(VcapDev0));
    CHECK_RET(HI_MAPI_VCAP_StartChn(VcapPipe0, PipeChn0));
    CHECK_RET(HI_MAPI_VCAP_StartISP(VcapPipe0));

    /**************************start vproc *******************************/
    /* Vpss0 attr */
    memcpy(&stVpssAttr, &g_stVpss_4K30FPS, sizeof(HI_MAPI_VPSS_ATTR_S));

    /* VPort0 attr */
    memcpy(&stVPortAttr, &g_stVport_4K30FPS, sizeof(HI_MAPI_VPORT_ATTR_S));

    CHECK_RET(HI_MAPI_VPROC_InitVpss(VpssHdl0, &stVpssAttr));
    CHECK_RET(HI_MAPI_VPROC_BindVcap(VcapPipe0, PipeChn0, VpssHdl0));
    CHECK_RET(HI_MAPI_VPROC_SetPortAttr(VpssHdl0, VPortHdl0, &stVPortAttr));
    CHECK_RET(HI_MAPI_VPROC_StartPort(VpssHdl0, VPortHdl0));

    /**************************start preview *******************************/

    CHECK_RET(SAMPLE_COMM_StartPreview(VpssHdl0, VPortHdl0, bStitch));

    /**************************start venc *******************************/
    memcpy(&stVencAttr, &g_stVenc_4K30FPS_JPEG, sizeof(HI_MAPI_VENC_ATTR_S));
    stVencCB.pfnDataCB = SMAPLE_COMM_VENC_DataProc;
    stVencCB.pPrivateData = (HI_VOID *)__FUNCTION__;

    CHECK_RET(HI_MAPI_VENC_Init(VencHdl0, &stVencAttr));
    CHECK_RET(HI_MAPI_VENC_RegisterCallback(VencHdl0, &stVencCB));
    CHECK_RET(HI_MAPI_VENC_BindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch));
    CHECK_RET(HI_MAPI_VENC_Start(VencHdl0, s32FrameCnt));

    /**************************start trigger *******************************/
    stSnapAttr.bLoadCCM = HI_FALSE;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.stNormalAttr.u32FrameCnt = u32FrameCnt;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;

    CHECK_RET(HI_MAPI_VCAP_SetSnapAttr(VcapPipe0, &stSnapAttr));

    printf("Press Enter key to snap,press 'q' exit snap ...\n");

    while (getchar() != 'q') {
        CHECK_RET(COMM_SnapTrigger(VcapPipe0, VencHdl0, stSnapAttr.stNormalAttr.u32FrameCnt));
    }

exit:

    /* stop venc 0 */
    HI_MAPI_VCAP_StopTrigger(VcapPipe0);
    HI_MAPI_VENC_Stop(VencHdl0);
    HI_MAPI_VENC_UnBindVProc(VpssHdl0, VPortHdl0, VencHdl0, bStitch);
    HI_MAPI_VENC_UnRegisterCallback(VencHdl0, &stVencCB);
    HI_MAPI_VENC_Deinit(VencHdl0);

    /* stop preview */
    SAMPLE_COMM_StopPreview(VpssHdl0, VPortHdl0, bStitch);

    /* stop vproc 0 */
    HI_MAPI_VPROC_StopPort(VpssHdl0, VPortHdl0);
    HI_MAPI_VPROC_UnBindVCap(VcapPipe0, PipeChn0, VpssHdl0);
    HI_MAPI_VPROC_DeinitVpss(VpssHdl0);

    /* stop vcap 0 */
    HI_MAPI_VCAP_StopISP(VcapPipe0);
    HI_MAPI_VCAP_StopChn(VcapPipe0, PipeChn0);
    HI_MAPI_VCAP_StopDev(VcapDev0);
    HI_MAPI_VCAP_DeinitSensor(VcapDev0);

    /* stop media */
    SAMPLE_COMM_Deinit();

    return s32Ret;
}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret;
    HI_CHAR ch;

    s32Ret = HI_MAPI_Sys_Init();

    if (s32Ret != HI_SUCCESS) {
        printf("HI_MAPI_Sys_Init fail s32Ret:%x\n", s32Ret);
        return HI_FAILURE;
    }

    SAMPLE_VENC_Usage();
    ch = (char)getchar();
    (void)getchar();

    switch (ch) {
        case '0':
            s32Ret = SAMPLE_VENC_TwoRecord();
            break;

        case '1':
            s32Ret = SAMPLE_VENC_SinglePipeSnap();
            break;

        case '2':
            s32Ret = SAMPLE_VENC_Record();

        default:
            printf("the index is invaild!\n");
            SAMPLE_VENC_Usage();
            return HI_FAILURE;
    }

    s32Ret = HI_MAPI_Sys_Deinit();
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

