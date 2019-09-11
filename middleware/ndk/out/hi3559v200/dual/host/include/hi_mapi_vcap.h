/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_vcap.h
 * @brief   vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_VCAP_H__
#define __HI_MAPI_VCAP_H__

#include "hi_mapi_vcap_define.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


/** \addtogroup     VCAP */
/** @{ */ /** <!-- [VCAP] */

/**
 * @brief initialize sensor attribute.
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] pstSensorAttr HI_MAPI_SENSOR_ATTR_S*:sensor attribute
 * @return 0  initialize sensor attribute success.
 * @return Non 0  initialize sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_InitSensor(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr);

/**
 * @brief Deinit sensor,close mipi and sensor clock.
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @return 0  Deinit sensor success.
 * @return Non 0  Deinit sensor.
 */
HI_S32 HI_MAPI_VCAP_DeinitSensor(HI_HANDLE VcapDevHdl);

/**
 * @brief Get sensor attribute.
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] pstSensorAttr HI_MAPI_SENSOR_ATTR_S*:sensor attribute
 * @return 0  Get sensor attribute success.
 * @return Non 0  Get sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSensorAttr(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr);

/**
 * @brief Set sensor framerate,Called after isp start
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] ps32Framerate HI_S32*:sensor framerate
 * @return 0  Set sensor attribute success.
 * @return Non 0 Set sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate);

/**
 * @brief Get sensor framerate
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] ps32Framerate HI_S32*:sensor framerate
 * @return 0  Get sensor attribute success.
 * @return Non 0 Get sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate);

/**
 * @brief Get sensor modes count
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] ps32ModesCnt HI_S32 *:sensor mode counts
 * @return 0  Get sensor attribute success.
 * @return Non 0 Get sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSensorModesCnt(HI_HANDLE VcapDevHdl, HI_S32 *ps32ModesCnt);

/**
 * @brief Get sensor All modes
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] ppstSensorAttrs HI_MAPI_SENSOR_ATTR_S **:sensor modes
 * @param[in] ps32ModesCnt HI_S32 *:sensor mode counts
 * @return 0  Get sensor attribute success.
 * @return Non 0 Get sensor attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSensorAllModes(HI_HANDLE VcapDevHdl, HI_S32 s32ModesCnt,
                                      HI_MAPI_SENSOR_MODE_S *pstSensorModes);

/**
 * @brief Start VCAP Device
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @return 0  Start VCAP Device success.
 * @return Non 0 Start VCAP Device fail.
 */
HI_S32 HI_MAPI_VCAP_StartDev(HI_HANDLE VcapDevHdl);

/**
 * @brief Stop VCAP Device
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @return 0  Stop VCAP Device success.
 * @return Non 0 Stop VCAP Device fail.
 */
HI_S32 HI_MAPI_VCAP_StopDev(HI_HANDLE VcapDevHdl);

/**
 * @brief Start VCAP ISP
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  Start VCAP ISP success.
 * @return Non 0 Start VCAP ISP fail.
 */
HI_S32 HI_MAPI_VCAP_StartISP(HI_HANDLE VcapPipeHdl);

/**
 * @brief Stop VCAP ISP
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  Stop VCAP ISP success.
 * @return Non 0 Stop VCAP ISP fail.
 */
HI_S32 HI_MAPI_VCAP_StopISP(HI_HANDLE VcapPipeHdl);

/**
 * @brief Set VCAP attribute
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] pstVCapAttr HI_MAPI_VCAP_ATTR_S *:video capture attribute
 * @return 0  Set VCAP attribute success.
 * @return Non 0 Set VCAP attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr);

/**
 * @brief Get VCAP attribute
 * @param[in] VcapDevHdl HI_HANDLE:handle of video capture device.
 * @param[in] pstVCapAttr HI_MAPI_VCAP_ATTR_S *:video capture attribute
 * @return 0  Get VCAP attribute success.
 * @return Non 0 Get VCAP attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr);

/**
 * @brief set VCAP expand attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's CHN.
 * @param[in] enCMD HI_MAPI_VCAP_CMD_E:control CMD.
 * @param[in] pAttr HI_VOID*:expand attribute struct.
 * @param[in] u32Len HI_U32:expand attribute struct length.
 * @return 0  set VCAP expand attribute success.
 * @return Non 0  set VCAP expand attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief get VCAP expand attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] enCMD HI_MAPI_VCAP_CMD_E:control CMD.
 * @param[in] pAttr HI_VOID*:expand attribute struct.
 * @param[in] u32Len HI_U32:expand attribute struct length.
 * @return 0  get VCAP expand attribute success.
 * @return Non 0  get VCAP expand attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Start VCAP channel
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @return 0  Start VCAP CHN success.
 * @return Non 0 Start VCAP CHN fail.
 */
HI_S32 HI_MAPI_VCAP_StartChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl);

/**
 * @brief Stop VCAP channel
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @return 0  Stop VCAP CHN success.
 * @return Non 0 Stop VCAP CHN fail.
 */
HI_S32 HI_MAPI_VCAP_StopChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl);

/**
 * @brief set channel crop attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] pstCropInfo VI_CROP_INFO_S*:channel crop attribute.
 * @return 0  set channel crop attribute success.
 * @return Non 0  set channel crop attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, VI_CROP_INFO_S *pstCropInfo);

/**
 * @brief get channel crop attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] pstCropInfo VI_CROP_INFO_S*:channel crop attribute.
 * @return 0  get channel crop attribute success.
 * @return Non 0  get channel crop attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, VI_CROP_INFO_S *pstCropInfo);

/**
 * @brief set channel OSD attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture OSD channel.
 * @param[in] pstOsdAttr HI_MAPI_OSD_ATTR_S*:channel OSD attribute.
 * @return 0  set channel OSD attribute success.
 * @return Non 0  set channel OSD attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
                                  HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief Get channel OSD attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture OSD channel.
 * @param[in] pstOsdAttr HI_MAPI_OSD_ATTR_S*:channel OSD attribute.
 * @return 0  Get channel OSD attribute success.
 * @return Non 0  Get channel OSD attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
                                  HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief start OSD .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture OSD channel.
 * @return 0  start OSD attribute success.
 * @return Non 0  start OSD fail.
 */
HI_S32 HI_MAPI_VCAP_StartChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl);

/**
 * @brief stop OSD .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture OSD channel.
 * @return 0  start OSD attribute success.
 * @return Non 0  stop OSD fail.
 */
HI_S32 HI_MAPI_VCAP_StopChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl);

/**
 * @brief set DIS attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] pstDisAttr HI_MAPI_VCAP_DIS_ATTR_S*:DIS attribute.
 * @return 0  set DIS attribute success.
 * @return Non 0  set DIS attribute DIS fail.
 */
HI_S32 HI_MAPI_VCAP_SetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                  HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr);

/**
 * @brief get DIS attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] pstDisAttr HI_MAPI_VCAP_DIS_ATTR_S*:DIS attribute.
 * @return 0  get DIS attribute success.
 * @return Non 0  get DIS attribute DIS fail.
 */
HI_S32 HI_MAPI_VCAP_GetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                  HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr);

/**
 * @brief Set snap attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstSnapAttr HI_MAPI_VCAP_SNAP_ATTR_S*:pstSnapAttr
 * @return 0  Set snap attribute success.
 * @return Non 0  Set snap attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr);

/**
 * @brief Get snap attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstSnapAttr HI_MAPI_VCAP_SNAP_ATTR_S*:pstSnapAttr
 * @return 0  Get snap attribute success.
 * @return Non 0  Get snap attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr);

/**
 * @brief Set snap Expand attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstSnapAttr HI_MAPI_VCAP_SNAP_ATTR_S*:pstSnapAttr
 * @return 0  Set snap Expand attribute success.
 * @return Non 0  Set snap Expand attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
                                  HI_U32 u32Len);

/**
 * @brief Get snap Expand attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstSnapAttr HI_MAPI_VCAP_SNAP_ATTR_S*:pstSnapAttr
 * @return 0  Get snap Expand attribute success.
 * @return Non 0  Get snap Expand attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
                                  HI_U32 u32Len);

/**
 * @brief snap Start Trigger.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  snap Start Trigger success.
 * @return Non 0  snap Start Trigger fail.
 */
HI_S32 HI_MAPI_VCAP_StartTrigger(HI_HANDLE VcapPipeHdl);

/**
 * @brief snap Stop Trigger.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  snap Stop Trigger success.
 * @return Non 0  snap Stop Trigger fail.
 */
HI_S32 HI_MAPI_VCAP_StopTrigger(HI_HANDLE VcapPipeHdl);

/**
 * @brief set stitch attribute.
 * @param[in] VcapStitchHdl HI_HANDLE:handle of video capture stitch mode.
 * @param[in] pstStitchAttr VI_STITCH_GRP_ATTR_S*:stitch attribute.
 * @return 0  set stitch attribute success.
 * @return Non 0  set stitch attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr);

/**
 * @brief get stitch attribute.
 * @param[in] VcapStitchHdl HI_HANDLE:handle of video capture stitch mode.
 * @param[in] pstStitchAttr VI_STITCH_GRP_ATTR_S*:stitch attribute.
 * @return 0  get stitch attribute success.
 * @return Non 0  get stitch attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr);

/**
 * @brief stitch snap trigger.
 * @param[in] VCapHdl HI_HANDLE:handle of video capture.
 * @return 0  stitch snap trigger success.
 * @return Non 0  stitch snap trigger fail.
 */
HI_S32 HI_MAPI_VCAP_StitchTrigger(HI_HANDLE VcapStitchHdl);

/**
 * @brief set VCAP exif attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstExifInfo HI_MAPI_SNAP_EXIF_INFO_S*:exif attribute.
 * @return 0  set VCAP exif attribute success.
 * @return Non 0  set VCAP exif attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo);

/**
 * @brief get VCAP exif attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstExifInfo HI_MAPI_SNAP_EXIF_INFO_S*:exif attribute.
 * @return 0  get VCAP exif attribute success.
 * @return Non 0  get VCAP exif attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo);

/**
 * @brief set dump normal and BNR raw attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstDumpAttr VI_DUMP_ATTR_S*:dump raw attribute .
 * @return 0  set dump normal and BNR raw attribute success.
 * @return Non 0  set dump normal raw attribute fail.
 */
HI_S32 HI_MAPI_VCAP_SetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr);

/**
 * @brief get dump normal and BNR raw attribute.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] pstDumpAttr VI_DUMP_ATTR_S*:dump raw attribute .
 * @return 0  get dump normal and BNR raw attribute success.
 * @return Non 0  get dump normal and BNR raw attribute fail.
 */
HI_S32 HI_MAPI_VCAP_GetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr);

/**
 * @brief start dump normal raw .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] u32Count HI_U32:dump frame count .
 * @param[in] pstVCapRawData:struct of write normal raw .
 * @return 0  start dump normal raw success.
 * @return Non 0  start dump normal raw fail.
 */
HI_S32 HI_MAPI_VCAP_StartDumpRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count, HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData);

/**
 * @brief stop dump normal raw .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  stop dump normal raw success.
 * @return Non 0  stop dump normal raw fail.
 */
HI_S32 HI_MAPI_VCAP_StopDumpRaw(HI_HANDLE VcapPipeHdl);

/**
 * @brief start dump BNR raw .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] u32Count HI_U32:dump frame count .
 * @param[in] pstVCapRawData:struct of write BNR raw .
 * @return 0  start dump BNR raw success.
 * @return Non 0  start dump BNR raw fail.
 */
HI_S32 HI_MAPI_VCAP_StartDumpBNRRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
                                    HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData);

/**
 * @brief stop dump BNR raw .
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @return 0  stop dump BNR raw success.
 * @return Non 0  stop dump BNR raw fail.
 */
HI_S32 HI_MAPI_VCAP_StopDumpBNRRaw(HI_HANDLE VcapPipeHdl);

/**
 * @brief init Motion Sensor .
 * @param[in] pstMotionAttr HI_MAPI_MOTIONSENSOR_INFO_S:info of Motionsensor.
 * @return 0  init Motion Sensor success.
 * @return Non 0  init Motion Sensor fail.
 */
HI_S32 HI_MAPI_VCAP_InitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr);

/**
 * @brief deinit Motion Sensor .
 * @param[out] pstMotionAttr HI_MAPI_MOTIONSENSOR_INFO_S:info of Motionsensor.
 * @return 0  deinit Motion Sensor success.
 * @return Non 0  deinit Motion Sensor fail.
 */
HI_S32 HI_MAPI_VCAP_DeInitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr);

// 5.others

/** @} */ /** <!-- ==== VCAP End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_VCAP_H__ */
