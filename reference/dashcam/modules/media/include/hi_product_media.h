/**
* @file    hi_product_media.h
* @brief   media module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/11
* @version   1.0

*/
#ifndef __HI_PRODUCT_MEDIA_H__
#define __HI_PRODUCT_MEDIA_H__

#include "hi_product_media_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     MEDIA */
/** @{ */  /** <!-- [MEDIA] */


/** error code define */
#define HI_PDT_MEDIA_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_EINVAL)       /**<Invalid argument */
#define HI_PDT_MEDIA_ENOTINIT       HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_ENOINIT)      /**<Not inited */
#define HI_PDT_MEDIA_EUNSUPPORT     HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_EPAERM)       /**<Unsupport */
#define HI_PDT_MEDIA_EINITIALIZED   HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_EINITIALIZED) /**<re-initialized */
#define HI_PDT_MEDIA_EDEINITIALIZED   HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_ERRNO_CUSTOM_BOTTOM+1) /**<re-initialized */


/**
 * @brief     module initialization
 * @param[in] pstViVpssMode : vi-vpss mode
 * @param[in] pstVBCfg : vb configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_Init(const HI_PDT_MEDIA_VI_VPSS_MODE_S* pstViVpssMode, const HI_PDT_MEDIA_VB_CFG_S* pstVBCfg);

/**
 * @brief     module deinitialization
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_Deinit(HI_VOID);

/**
 * @brief     Init sensor
 * @param[in] pstVcapDevAttr : sensor configure
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_InitSensor(const HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr);

/**
 * @brief     init video modules, vcap/vproc/venc
 * @param[in] pstVideoCfg : video configure, including vcap/vproc/venc
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoInit(const HI_PDT_MEDIA_VIDEO_CFG_S* pstVideoCfg);

/**
 * @brief     deinit video modules, vap/vproc/venc
 * @param[in] bStopIsp : all video module exit except Isp
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoDeinit(HI_BOOL bStopIsp);

/**
 * @brief     init video out modules, display
 * @param[in] pstVideoOutCfg : video out configure, including display
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoOutInit(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg);

/**
 * @brief     deinit video out modules, display
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoOutDeinit(HI_VOID);

/**
 * @brief     start display
 * @param[in] pstVideoOutCfg : video out configure, including display
 * @param[in] pstpstVprocCfg  : vpss_vport config, allow NULL
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoOutStart(const HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg,const HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg);

/**
 * @brief     set graphic layer csc
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_SetDisplayGraphicCSC(HI_VOID);

/**
 * @brief     do crop on display window(volayer channel)
 * @param[in] pstCropCfg : crop info
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_DispWindowCrop(const HI_PDT_MEDIA_DISP_WND_CROP_CFG_S* pstCropCfg);


/**
 * @brief     stop display
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VideoOutStop(HI_VOID);

/**
 * @brief     init audio modules, acap/aenc
 * @param[in] pstAudioCfg : audio configure, including acap/aenc
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AudioInit(const HI_PDT_MEDIA_AUDIO_CFG_S* pstAudioCfg);

/**
 * @brief     deinit audio modules, acap/aenc
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AudioDeinit(HI_VOID);

/**
 * @brief     start audio out
 * @param[in] pstAudioOutCfg : audio out configure, including ao
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AudioOutStart(const HI_PDT_MEDIA_AUDIOOUT_CFG_S* pstAudioOutCfg);

/**
 * @brief     stop audio out
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AudioOutStop(HI_VOID);

/**
 * @brief     reset media modules
 * @param[in] pstMediaCfg : media configure, including vb/video/videoout/audio/audioout
 * @param[in] bCompress : Compress config enable,need consider ldc rotate
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_Reset(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_BOOL bCompress);

/**
 * @brief     rebuild media modules
 * @param[in] pstMediaCfg : media configure, including vb/video/videoout/audio/audioout
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_Rebuild(const HI_PDT_MEDIA_CFG_S* pstMediaCfg);

/**
 * @brief     start aenc
 * @param[in] AencHdl : audio encoder handle
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AencStart(HI_HANDLE AencHdl);

/**
 * @brief     stop aenc
 * @param[in] AencHdl : audio encoder handle
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_AencStop(HI_HANDLE AencHdl);

/**
 * @brief     start venc
 * @param[in] VencHdl : video encoder handle
 * @param[in] s32FrameCnt : encode frame count, -1 not stop until HI_PDT_MEDIA_VencStop called
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VencStart(HI_HANDLE VencHdl, HI_S32 s32FrameCnt);

/**
 * @brief     stop venc
 * @param[in] VencHdl : video encoder handle
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VencStop(HI_HANDLE VencHdl);

/**
 * @brief     rebind venc for live
 * @param[in] pstDstSrc : venc src
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_VencReBind(HI_PDT_MEDIA_BIND_SRC_CFG_S* pstDstSrc);

/**
 * @brief     get video information
 * @param[in] VencHdl : video encoder handle
 * @param[out]pstVideoInfo : video information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_GetVideoInfo(HI_HANDLE VencHdl, HI_MEDIA_VIDEOINFO_S* pstVideoInfo);

/**
 * @brief     get audio information
 * @param[in] AencHdl : audio encoder handle
 * @param[out]pstAudioInfo : audio information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_GetAudioInfo(HI_HANDLE AencHdl, HI_MEDIA_AUDIOINFO_S* pstAudioInfo);

/**
 * @brief     set venc  attribute
 * @param[in] VencHdl : video encoder handle
 * @param[in] pstVencAttr:
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_SetVencAttr(HI_HANDLE VencHdl, const HI_MEDIA_VENC_ATTR_S* pstVencAttr);

/**
 * @brief     get venc attribute
 * @param[in] VencHdl : video encoder handle
 * @param[out]penVencFmt : video encoder format(payload type)
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/11
 */
HI_S32 HI_PDT_MEDIA_GetVencAttr(HI_HANDLE VencHdl, HI_MEDIA_VENC_ATTR_S* pstVencAttr);

/**
 * @brief     set time osd show state
 * @param[in] s32VcapDevIdx : product media osd index,range [0,HI_PDT_MEDIA_VCAP_DEV_MAX_CNT}
 * @param[in] bEnable : osd show or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetTimeOSD(HI_S32 s32VcapDevIdx,HI_BOOL bEnable);

/**
 * @brief     set logo osd show state
 * @param[in] s32VcapDevIdx : product media osd index,range [0,HI_PDT_MEDIA_VCAP_DEV_MAX_CNT}
 * @param[in] bEnable : osd show or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetLogoOSD(HI_S32 s32VcapDevIdx,HI_BOOL bEnable);

/**
 * @brief     set LDC with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[in] bEnable : ldc enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL bEnable);

/**
 * @brief     get LDC with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[out]pbEnable : ldc enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_BOOL* pbEnable);

/**
 * @brief     set flip with specified vcap pipe and pipechn
 * @param[in] pstFlipSrc : point source where to do flip
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[in] bEnable : flip enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetFlip(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL bEnable);

/**
 * @brief     get flip with specified vcap pipe and pipechn
 * @param[in] pstFlipSrc : point source where to do flip
 * @param[out]pbEnable : flip enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetFlip(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL *pbEnable);

/**
 * @brief     set mirror with specified vcap pipe and pipechn
 * @param[in] pstFlipSrc : point source where to do flip
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[in] bEnable : mirror enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetMirror(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL bEnable);

/**
 * @brief     get mirror with specified vcap pipe and pipechn
 * @param[in] pstFlipSrc : point source where to do flip
 * @param[out]pbEnable : flip enable or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetMirror(HI_PDT_MEDIA_BIND_SRC_CFG_S *pstFlipSrc, HI_BOOL *pbEnable);


/**
 * @brief     set brightness with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[in] s32Brightness : brightness value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetBrightness(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32 s32Brightness);

/**
 * @brief     get brightness with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[out]ps32Brightness : brightness value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetBrightness(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32* ps32Brightness);

/**
 * @brief     set saturation with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[in] s32Saturation : saturation value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetSaturation(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32 s32Saturation);

/**
 * @brief     get saturation with specified vcap pipe and pipechn
 * @param[in] VcapPipeHdl : vcap pipe handle
 * @param[in] VcapPipeChnHdl : vcap pipe chn handle
 * @param[out]ps32Saturation : saturation value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetSaturation(HI_HANDLE VcapPipeHdl, HI_HANDLE VcapPipeChnHdl, HI_S32* ps32Saturation);

/**
 * @brief     set ao volume
 * @param[in] AoHdl : ao handle
 * @param[in] s32Volume : volume in percent
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetAoVolume(HI_HANDLE AoHdl, HI_S32 s32Volume);

/**
 * @brief     get ao volume
 * @param[in] AoHdl : ao handle
 * @param[out]ps32Volume : volume in percent
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_GetAoVolume(HI_HANDLE AoHdl, HI_S32* ps32Volume);

/**
 * @brief     mute ao
 * @param[in] AoHdl : ao handle
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_AoMute(HI_HANDLE AoHdl);

/**
 * @brief     unmute ao
 * @param[in] AoHdl : ao handle
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_AoUnmute(HI_HANDLE AoHdl);

/**
 * @brief     venc init
 * @param[in] pstVencCfg : media venc config
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_VencInit(const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg);

/**
 * @brief     modify framerate for adaptering Flicker frequency 50HZ
 * @param[in] bEable : true mean framerate need modify,false framerate need restore before
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/3
 */
HI_S32 HI_PDT_MEDIA_SetFlickerFreq50HZ(HI_BOOL bEable);

/**
 * @brief     init osd resource, eg. bitmap memory
 * @param[in] pstOsdCfg : osd configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 HI_PDT_MEDIA_InitOSD(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg);

/**
 * @brief     deinit osd resource
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 HI_PDT_MEDIA_DeinitOSD(HI_VOID);
/**
 * @brief     update display configure
 * @param[in/out]pstDispCfg : display configure
 * @param[in/out]pstVprocCfg : vproc configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/21
 */
HI_S32 HI_PDT_Media_UpdateDispCfg(HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg,HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg,SIZE_S* pstSrcSize);


/** @}*/  /** <!-- ==== MEDIA End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_MEDIA_H__ */

