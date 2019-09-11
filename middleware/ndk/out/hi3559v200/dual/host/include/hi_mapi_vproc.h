/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_vproc.h
 * @brief   vproc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_VPROC_H__
#define __HI_MAPI_VPROC_H__
#include "hi_mapi_vproc_define.h"

#ifdef SUPPORT_PHOTO_POST_PROCESS
#include "hi_comm_photo.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*********Vpss*********/

/**
 * @brief Init vpss group attribute.
 * @param[in] VpssHdl HI_HANDLE:handle of vpss. It's integer from 0 to 31.
 * @param[in] pstSensorMode HI_MAPI_VPSS_ATTR_S*:pointer of vpss attribute.Static attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_InitVpss(HI_HANDLE VpssHdl, HI_MAPI_VPSS_ATTR_S *pstVpssAttr);

/**
 * @brief Deinit vpss. It must be init first.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_DeinitVpss(HI_HANDLE VpssHdl);

/**
 * @brief Bind vcap to vproc.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_BindVcap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl);

/**
 * @brief Unbind vcap from vproc.
 * @param[in] VcapPipeHdl HI_HANDLE:handle of video capture PIPE.
 * @param[in] PipeChnHdl HI_HANDLE:handle of video capture PIPE's channel.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_UnBindVCap(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE VpssHdl);

/*********VPort*********/

/**
 * @brief Set attribute of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport. It's integer from 0 to 3.
 * @param[in] pstVPortAttr HI_MAPI_VPORT_ATTR_S*: pointer of vport attribute. Dynamic attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr);

/**
 * @brief Get attribute of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[out] pstVPortAttr HI_VPORT_ATTR_S*: pointer of vport attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetPortAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPORT_ATTR_S *pstVPortAttr);

/**
 * @brief Start a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief Stop a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopPort(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief Set corresponding attributes of a vpss,including mirror, flip, rotate etc.
 * @param[in] VpssHdl HI_HANDLE :handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] enCMD HI_MAPI_VPROC_CMD_E : Setting command.
 * @param[in] pAttr HI_VOID:The struct of corresponding attributes.Dynamic attribute.
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPROC_CMD_E enCMD,
                                   HI_VOID *pAttr, HI_U32 u32Len);

/**
 * @brief Get corresponding attributes of a vproc,including mirror, flip, rotate etc.
 * @param[in] VpssHdl HI_HANDLE :handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] enCMD HI_MAPI_VPROC_CMD_E : Getting command.
 * @param[out] pAttr HI_VOID:The struct of corresponding attributes.
 * @param[in] u32Len HI_U32: length of the extend attribute
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetPortAttrEx(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_VPROC_CMD_E enCMD,
                                   HI_VOID *pAttr, HI_U32 u32Len);

/*********Stitch*********/
#ifdef SUPPORT_STITCH
/**
 * @brief Create Stitch group.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch. It's integer from 0 to 31.
 * @param[in] pstStitchAttr HI_MAPI_STITCH_ATTR_S*:pointer of Stitch attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_CreateStitch(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr);

/**
 * @brief Destroy Stitch group.It must be Create first.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_DestroyStitch(HI_HANDLE StitchHdl);

/**
 * @brief Set the dynamic attributes of the Stitch.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch.
 * @param[in] pstStitchAttr HI_MAPI_STITCH_ATTR_S*:pointer of Stitch attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr);

/**
 * @brief Get the attributes of the Stitch.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch.
 * @param[out] pstStitchAttr HI_MAPI_STITCH_ATTR_S*:pointer of Stitch attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetStitchAttr(HI_HANDLE StitchHdl, HI_MAPI_STITCH_ATTR_S *pstStitchAttr);

/*********StitchPort*********/

/**
 * @brief Set attribute of a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE:handle of StitchPort. It's integer from 0 to 3.
 * @param[in] pstStitchPortAttr HI_MAPI_STITCH_PORT_ATTR_S*:pointer of StitchPort attribute.Dynamic attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr);

/**
 * @brief Get attribute of a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE:handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE:handle of StitchPort. It's integer from 0 to 3.
 * @param[out] pstStitchPortAttr HI_MAPI_STITCH_PORT_ATTR_S*:pointer of StitchPort attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetStitchPortAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                       HI_MAPI_STITCH_PORT_ATTR_S *pstStitchPortAttr);

/**
 * @brief Start a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl);

/**
 * @brief Stop a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopStitchPort(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl);
#endif

/*********Photo*********/
#ifdef SUPPORT_PHOTO_POST_PROCESS
/**
 * @brief Init Photo of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] enPhotoType HI_MAPI_PHOTO_TYPE_E: Photo type.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_InitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_MAPI_PHOTO_TYPE_E enPhotoType);

/**
 * @brief Deinit Photo of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_DeinitPhoto(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief UnRegister Photo Process callback of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] pstDumpCB HI_PHOTO_DUMP_CALLBACK_FUNC_S : callback.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_RegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                    HI_PHOTO_DUMP_CALLBACK_FUNC_S *pstDumpCB);

/**
 * @brief UnRegister Photo Process callback of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_UnRegPhotoDumpCB(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief Start Photo Process of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_PhotoProcess(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief Get current photo type of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[out] penPhotoType HI_MAPI_PHOTO_TYPE_E*: Photo type.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetCurrentPhotoType(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                         HI_MAPI_PHOTO_TYPE_E *penPhotoType);

/**
 * @brief Set the photo algorithm parameter.
 * @param[in] enAlgType PHOTO_ALG_TYPE_E: Photo Alg type.
 * @param[in] pstAlgCoef const PHOTO_ALG_COEF_S*: photo algorithm parameter.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, const PHOTO_ALG_COEF_S *pstAlgCoef);

/**
 * @brief Get the photo algorithm parameter.
 * @param[in] enAlgType PHOTO_ALG_TYPE_E: Photo Alg type.
 * @param[out] pstAlgCoef const PHOTO_ALG_COEF_S*: photo algorithm parameter.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetPhotoAlgCoef(PHOTO_ALG_TYPE_E enAlgType, PHOTO_ALG_COEF_S *pstAlgCoef);
#endif

/*********DumpYUV*********/
/**
 * @brief get frame from vport of vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] bStitch HI_BOOL: flag of stitch.
 * @param[out] pstYUVFrameData HI_MAPI_FRAME_DATA_S* : pointer of framedata.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetPortFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                                  HI_MAPI_FRAME_DATA_S *pstYUVFrameData);

/**
 * @brief release frame from vport of vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] bStitch HI_BOOL: flag of stitch.
 * @param[out] pstYUVFrameData HI_MAPI_FRAME_DATA_S* : pointer of framedata.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_ReleasePortFrame(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_BOOL bStitch,
                                      HI_MAPI_FRAME_DATA_S *pstYUVFrameData);

/**
 * @brief set dump YUV attribute form vport of vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] pstDumpYUVAttr HI_MAPI_DUMP_YUV_ATTR_S* : pointer of DumpYUV attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr);

/**
 * @brief  get dump YUV attribute form vport of vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[out] pstDumpYUVAttr HI_MAPI_DUMP_YUV_ATTR_S* : pointer of DumpYUV attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetVpssDumpYUVAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl,
                                        HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr);

/**
 * @brief start dump YUV data form port of vproc.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] s32Count HI_S32 : the number of dump YUV data.
 * @param[in] pstCallbackFun HI_DUMP_YUV_CALLBACK_FUNC_S : The pointer of callback function of dump YUV data.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_S32 s32Count,
                                      HI_DUMP_YUV_CALLBACK_FUNC_S *pstCallbackFun);

/**
 * @brief stop dump YUV data form vport of vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopVpssDumpYUV(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl);

/**
 * @brief set dump YUV attribute form StitchPort of Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] pstDumpYUVAttr HI_MAPI_DUMP_YUV_ATTR_S* : pointer of DumpYUV attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetStitchDumpYUVAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                          HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr);

/**
 * @brief get dump YUV attribute form StitchPort of Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[out] pstDumpYUVAttr HI_MAPI_DUMP_YUV_ATTR_S* : pointer of DumpYUV attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetStitchDumpYUVAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl,
                                          HI_MAPI_DUMP_YUV_ATTR_S *pstDumpYUVAttr);

/**
 * @brief start dump YUV data form port of vproc.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] s32Count HI_S32 : the number of dump YUV data.
 * @param[in] pstCallbackFun HI_DUMP_YUV_CALLBACK_FUNC_S : The pointer of callback function of dump YUV data.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartStitchDumpYUV(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_S32 s32Count,
                                        HI_DUMP_YUV_CALLBACK_FUNC_S *pstCallbackFun);

/**
 * @brief stop dump YUV data form StitchPort of Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopStitchDumpYUV(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl);

/*********OSD*********/

/**
 * @brief Set OSD attribute of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.It's integer from 0 to 3.
 * @param[in] pstOSDAttr HI_MAPI_OSD_ATTR_S*: pointer of OSD attribute. Dynamic attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief Get OSD attribute of a port in vpss.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @param[out] pstOSDAttr HI_MAPI_OSD_ATTR_S*: pointer of OSD attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetVpssOSDAttr(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl,
                                    HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief start OSD.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl);

/**
 * @brief stop OSD.
 * @param[in] VpssHdl HI_HANDLE : handle of vpss.
 * @param[in] VPortHdl HI_HANDLE : handle of vport.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopVpssOSD(HI_HANDLE VpssHdl, HI_HANDLE VPortHdl, HI_HANDLE OSDHdl);

/**
 * @brief Set OSD attribute of a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.It's integer from 0 to 3.
 * @param[in] pstOSDAttr HI_MAPI_OSD_ATTR_S*: pointer of OSD attribute. Dynamic attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_SetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief Get OSD attribute of a StitchPort in Stitch.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @param[out] pstOSDAttr HI_MAPI_OSD_ATTR_S*: pointer of OSD attribute.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_GetStitchOSDAttr(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl,
                                      HI_MAPI_OSD_ATTR_S *pstOSDAttr);

/**
 * @brief start OSD.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StartStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl);

/**
 * @brief stop OSD.
 * @param[in] StitchHdl HI_HANDLE : handle of Stitch.
 * @param[in] StitchPortHdl HI_HANDLE : handle of StitchPort.
 * @param[in] OSDHdl HI_HANDLE : handle of OSD channel.
 * @return 0  successful.
 * @return non 0  fail.
 */
HI_S32 HI_MAPI_VPROC_StopStitchOSD(HI_HANDLE StitchHdl, HI_HANDLE StitchPortHdl, HI_HANDLE OSDHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_VPROC_H__ */
