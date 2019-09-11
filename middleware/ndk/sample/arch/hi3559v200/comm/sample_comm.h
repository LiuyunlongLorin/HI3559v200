/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    sample_comm.h
 * @brief   sample comm header
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */

#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__

#include "comm_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/* in sensor 477 modes, the frame rate of 4k30 snsmode is 29.97fps, else it is 30.0fps */
#ifdef SENSOR_IMX477
#define SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE (29.97f)
#else
#define SAMPLE_4K30FPS_ISPPUB_MAX_FRAMERATE (30.0f)
#endif

#define SAMPLE_MAPI_AI_CHN (0) /** stereo sound mode */

extern HI_BOOL g_bSnap_finished;
extern HI_CHAR g_aszDumpFrameName[HI_MAPI_VCAP_MAX_PIPE_NUM][64];
extern HI_S32 g_as32RawFrameCnt[HI_MAPI_VCAP_MAX_PIPE_NUM];

extern const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_1080P_Mode0;
extern const HI_MAPI_SENSOR_ATTR_S g_stSensor_4M_Mode0;
extern const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_4K_Mode0;
extern const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_4K_Mode1;
extern const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_12M_Mode0;
extern const HI_MAPI_SENSOR_ATTR_S g_stSample_Sensor_12M_Mode1;
extern const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_1080P30FPS;
extern const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_4M30FPS;
extern const HI_MAPI_VCAP_PIPE_ATTR_S g_stPipe_4K30FPS;
extern const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_1080P;
extern const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_4M;
extern const HI_MAPI_VCAP_DEV_ATTR_S g_stDev_4K;
extern const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_1080P30FPS;
extern const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_4M30FPS;
extern const HI_MAPI_PIPE_CHN_ATTR_S g_stChn_4K30FPS;
extern const HI_MAPI_VPSS_ATTR_S g_stVpss_1080P30FPS;
extern const HI_MAPI_VPSS_ATTR_S g_stVpss_4M30FPS;
extern const HI_MAPI_VPSS_ATTR_S g_stVpss_4K30FPS;
extern const HI_MAPI_VPORT_ATTR_S g_stVport_1080P30FPS;
extern const HI_MAPI_VPORT_ATTR_S g_stVport_4M30FPS;
extern const HI_MAPI_VPORT_ATTR_S g_stVport_4K30FPS;
extern const HI_MAPI_VPORT_ATTR_S g_stVport_1080P30FPS_16BPP;
extern const HI_MAPI_VPORT_ATTR_S g_stVport_4K30FPS_16BPP;
extern const HI_MAPI_VENC_ATTR_S g_stVenc_1080P30FPS_JPEG;
extern const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_JPEG;
extern const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_VIDEO_H264;
extern const HI_MAPI_VENC_ATTR_S g_stVenc_4K30FPS_VIDEO_H265;
extern const HI_MAPI_VENC_ATTR_S g_stVenc_1080P30FPS_VIDEO_H265;

extern const HI_MAPI_VENC_ATTR_S g_stVenc_720P30FPS_H264;
extern const VO_PUB_ATTR_S g_stDisp_HDMI_1080P30;
extern const HI_MAPI_DISP_WINDOW_ATTR_S g_stWnd_1920x1080;
#ifdef SUPPORT_HDMI
extern const HI_MAPI_HDMI_ATTR_S g_sthdmi_1080P30;
HI_VOID SAMPLE_COMM_EventPro(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData);
#endif
HI_S32 SAMPLE_COMM_GetSensorCfg(SAMPLE_SENSOR_SEQ_E enSensorSeq, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr);

HI_S32 COMM_SnapTrigger(HI_S32 VCapPipeHdl, HI_S32 VEncHdl, HI_S32 s32Framecnt);
HI_S32 SAMPLE_ExitMapi(void);
void SAMPLE_VENC_HandleSig(HI_S32 signo);
HI_S32 VENC_DataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData);
HI_S32 SAMPLE_COMM_SaveJpegFile(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData);
HI_S32 SAMPLE_COMM_SaveStreamFile(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData);
HI_S32 SMAPLE_COMM_VENC_DataProc(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData, HI_VOID *pPrivateData);
HI_S32 SAMPLE_COMM_StartPreview(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch);
HI_VOID SAMPLE_COMM_StopPreview(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch);
HI_S32 SAMPLE_COMM_Init(SAMPLE_COMM_INITATTR_S stSampleCommAttr, SAMPLE_VB_CFG_MODE_E enVbMode);
HI_S32 SAMPLE_COMM_Deinit(HI_VOID);
HI_S32 YUV_DataProc(HI_HANDLE VProcHdl, HI_HANDLE VPortHdl, HI_MAPI_FRAME_DATA_S *pVPortYUV,
                    HI_VOID *pPrivateData);
HI_S32 RAW_DataProc(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pVCapRawData, HI_S32 s32DataNum,
                    HI_VOID *pPrivateData);
HI_S32 Fastboot_Start_Audio_Aenc(HI_VOID);
HI_S32 Fastboot_Start_Vcap(HI_HANDLE VcapDev, HI_HANDLE VcapPipe, HI_HANDLE PipeChn);
HI_S32 Fastboot_Start_DispAndHdmi(HI_HANDLE DispHdl, HI_HANDLE WndHdl, HI_HANDLE HdmiHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

