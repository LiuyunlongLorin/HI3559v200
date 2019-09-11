/**
 * @file      product_media_inner.h
 * @brief     product media inner inferface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 * @version   1.0

 */

#ifndef __PRODUCT_MEDIA_INNER_H__
#define __PRODUCT_MEDIA_INNER_H__

#include "hi_product_media.h"
#include "hi_mapi_hdmi.h"
#include "hi_osd.h"
#include "hi_timestamp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define PDT_MEDIA_HDMI_MAX_WIDTH  (1920)
#define PDT_MEDIA_HDMI_MAX_HEIGHT  (1080)


HI_PDT_MEDIA_CFG_S* PDT_MEDIA_GetMediaCfg(HI_VOID);


/* ++++++++++++++++++++++++ OSD ++++++++++++++++++++++++ */

/** media osd vcapture pipechn information */
typedef struct hiPDT_MEDIA_OSD_VCAP_PIPECHN_INFO_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  PipeChnHdl;
    SIZE_S     stDestResolution;
} HI_PDT_MEDIA_OSD_VCAP_PIPECHN_INFO_S;

/** media osd vcapture pipe information */
typedef struct hiPDT_MEDIA_OSD_VCAP_PIPE_INFO_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VcapPipeHdl;
    HI_PDT_MEDIA_OSD_VCAP_PIPECHN_INFO_S astPipeChnInfo[HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
} HI_PDT_MEDIA_OSD_VCAP_PIPE_INFO_S;

/** media osd vcapture device information */
typedef struct hiPDT_MEDIA_OSD_VCAP_DEV_INFO_S
{
    HI_BOOL    bEnable;
    HI_PDT_MEDIA_OSD_VCAP_PIPE_INFO_S astPipeInfo[HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT];
} HI_PDT_MEDAI_OSD_VCAP_DEV_INFO_S;

/** media osd vpss port information */
typedef struct hiPDT_MEDIA_OSD_VPSS_PORT_INFO_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VportHdl;
    SIZE_S     stResolution;
} HI_PDT_MEDIA_OSD_VPSS_PORT_INFO_S;

/** media osd vpss information */
typedef struct hiPDT_MEDIA_OSD_VPSS_INFO_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VpssHdl;
    HI_HANDLE  VcapPipeHdl;       /**<binded video capture pipe handle */
    HI_HANDLE  VcapPipeChnHdl;    /**<binded video capture pipe chn handle */
    HI_PDT_MEDIA_OSD_VPSS_PORT_INFO_S astPortInfo[HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
} HI_PDT_MEDIA_OSD_VPSS_INFO_S;

/** media osd venc information */
typedef struct hiPDT_MEDIA_OSD_VENC_INFO_S
{
    HI_BOOL    bEnable;
    HI_HANDLE  VencHdl;
    SIZE_S     stResolution;
    HI_PDT_MEDIA_VIDEOMOD_E enBindedMod; /** binded video module: only support vcap/vpss */
    HI_HANDLE  ModHdl;       /**<binded module     handle: vcap-pipe    | vproc-vpss */
    HI_HANDLE  ChnHdl;       /**<binded module chn handle: vcap-pipechn | vproc-vpssport */
} HI_PDT_MEDIA_OSD_VENC_INFO_S;

/** media osd video information */
typedef struct hiPDT_MEDIA_OSD_VIDEO_INFO_S
{
    HI_PDT_MEDAI_OSD_VCAP_DEV_INFO_S astVcapDevInfo[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_MEDIA_OSD_VPSS_INFO_S     astVpssInfo[HI_PDT_MEDIA_VPSS_MAX_CNT];
    HI_PDT_MEDIA_OSD_VENC_INFO_S     astVencInfo[HI_PDT_MEDIA_VENC_MAX_CNT];
    HI_PDT_MEDIA_OSD_CFG_S           stOsdCfg;
} HI_PDT_MEDIA_OSD_VIDEO_INFO_S;


/**
 * @brief     init osd module
 * @param[in] pstVideoAttr : video attribute for osd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 PDT_MEDIA_OSD_Init(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg);

/**
 * @brief     start time osd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 PDT_MEDIA_OSD_StartTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo);

/**
 * @brief     stop time osd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 PDT_MEDIA_OSD_StopTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo);

/**
 * @brief     deinit osd module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 PDT_MEDIA_OSD_Deinit(HI_VOID);


/* ++++++++++++++++++++++++ HDMI ++++++++++++++++++++++++ */

/**
 * @brief     init hdmi module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 PDT_MEDIA_HDMI_Init(HI_VOID);

/**
 * @brief     deinit hdmi module
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 PDT_MEDIA_HDMI_Deinit(HI_VOID);

/**
 * @brief     update hdmi media configure
 * @param[in/out] pstMediaCfg : media configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 PDT_MEDIA_HDMI_UpdateMediaCfg(HI_PDT_MEDIA_CFG_S* pstMediaCfg);

/**
 * @brief     start hdmi, including attribute set
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 PDT_MEDIA_HDMI_Start(HI_VOID);

/**
 * @brief     stop hdmi
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
HI_S32 PDT_MEDIA_HDMI_Stop(HI_VOID);

/**
 * @brief     get hdmi state
 * @param[out]penState : hdmi state
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/21
 */
HI_S32 PDT_MEDIA_HDMI_GetState(HI_PDT_MEDIA_HDMI_STATE_E* penState);

/**
 * @brief     get hdmi display size
 * @param[out]pstSize : hdmi display size
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/21
 */
HI_S32 PDT_MEDIA_HDMI_GetDispSize(SIZE_S *pstSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __PRODUCT_MEDIA_INNER_H__ */


