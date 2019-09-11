/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_main.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/01/16
  Description   :
  History       :
  1.Date        : 2013/01/16
    Author      :
    Modification: Created file

******************************************************************************/

#include "isp_main.h"
#include "isp_ext_config.h"
#include "isp_defaults.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_statistics.h"
#include "isp_regcfg.h"
#include "isp_config.h"
#include "isp_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 ISP_UpdateInfo(VI_PIPE ViPipe)
{
    HI_S32 i, s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    ISP_CHECK_OPEN(ViPipe);

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (pstIspCtx->astAlgs[i].bUsed) {
            if (HI_NULL != pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl) {
                pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl(ViPipe, ISP_UPDATE_INFO_GET, &pstIspCtx->stDcfUpdateInfo);
            }
        }
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_UPDATE_INFO_SET, &pstIspCtx->stDcfUpdateInfo);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set dcf updateinfo failed\n", ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateFrameInfo(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_FRAME_INFO_SET, &pstIspCtx->stFrameInfo);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set frameinfo failed\n", ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateAttachInfo(VI_PIPE ViPipe)
{
    HI_S32 i;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    if (HI_NULL == pstIspCtx->stAttachInfoCtrl.pstAttachInfo) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d the Isp attach Info hasn't init!\n", ViPipe);
        return HI_FAILURE;
    }

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (pstIspCtx->astAlgs[i].bUsed) {
            if (HI_NULL != pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl) {
                pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl(ViPipe, ISP_ATTACHINFO_GET, pstIspCtx->stAttachInfoCtrl.pstAttachInfo);
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateColorGamutinfo(VI_PIPE ViPipe)
{
    HI_S32 i;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    if (HI_NULL == pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d the Isp colorGamut Info hasn't init!\n", ViPipe);
        return HI_FAILURE;
    }

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (pstIspCtx->astAlgs[i].bUsed) {
            if (HI_NULL != pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl) {
                pstIspCtx->astAlgs[i].stAlgFunc.pfn_alg_ctrl(ViPipe, ISP_COLORGAMUTINFO_GET,
                                                             pstIspCtx->stGamutInfoCtrl.pstColorGamutInfo);
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_ReadExtregs(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_U64 u64PhyAddrHigh;
    HI_U64 u64PhyAddrTemp;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u64PhyAddrHigh = (HI_U64)hi_ext_system_sys_debug_high_addr_read(ViPipe);
    u64PhyAddrTemp = (HI_U64)hi_ext_system_sys_debug_low_addr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    pstIspCtx->bFreezeFw = hi_ext_system_freeze_firmware_read(ViPipe);

    pstIspCtx->stIspDbg.bDebugEn = hi_ext_system_sys_debug_enable_read(ViPipe);
    pstIspCtx->stIspDbg.u64PhyAddr = u64PhyAddrTemp;

    pstIspCtx->stIspDbg.u32Depth = hi_ext_system_sys_debug_depth_read(ViPipe);
    pstIspCtx->stIspDbg.u32Size = hi_ext_system_sys_debug_size_read(ViPipe);

    return HI_SUCCESS;
}

HI_S32 ISP_UpdateExtRegs(VI_PIPE ViPipe)
{
    return HI_SUCCESS;
}

HI_S32 ISP_SwitchImageMode(VI_PIPE ViPipe)
{
    HI_U8 u8WDRMode;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Value = 0;
    HI_VOID *pValue = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;
    ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = NULL;
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsImageMode;
    ISP_CMOS_SENSOR_IMAGE_MODE_S stIspImageMode;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    pstIspCtx->stSnsImageMode.u16Width  = hi_ext_top_sensor_width_read(ViPipe);
    pstIspCtx->stSnsImageMode.u16Height = hi_ext_top_sensor_height_read(ViPipe);
    u32Value = hi_ext_system_fps_base_read(ViPipe);
    pValue = (HI_VOID *)&u32Value;
    pstIspCtx->stSnsImageMode.f32Fps    = *(HI_FLOAT *)pValue;
    pstIspCtx->stSnsImageMode.u8SnsMode = hi_ext_system_sensor_mode_read(ViPipe);

    pstIspCtx->stSysRect.s32X      = hi_ext_system_corp_pos_x_read(ViPipe);
    pstIspCtx->stSysRect.s32Y      = hi_ext_system_corp_pos_y_read(ViPipe);
    pstIspCtx->stSysRect.u32Width  = hi_ext_sync_total_width_read(ViPipe);
    pstIspCtx->stSysRect.u32Height = hi_ext_sync_total_height_read(ViPipe);

    stIspImageMode.u16Width  = hi_ext_sync_total_width_read(ViPipe);
    stIspImageMode.u16Height = hi_ext_sync_total_height_read(ViPipe);
    stIspImageMode.f32Fps    = pstIspCtx->stSnsImageMode.f32Fps;
    stIspImageMode.u8SnsMode = pstIspCtx->stSnsImageMode.u8SnsMode;

    ioctl(g_as32IspFd[ViPipe], ISP_GET_HDR_ATTR, &pstIspCtx->stHdrAttr);

    if (0 == pstIspCtx->u8IspImageModeFlag) {
        ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET,
                     (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);

        memcpy(&pstIspCtx->stPreSnsImageMode, &pstIspCtx->stSnsImageMode, sizeof(ISP_SENSOR_IMAGE_MODE_S));

        pstIspCtx->u8IspImageModeFlag = 1;
    } else {
        if ((pstIspCtx->stSnsImageMode.u16Width  != pstIspCtx->stPreSnsImageMode.u16Width) ||
            (pstIspCtx->stSnsImageMode.u16Height != pstIspCtx->stPreSnsImageMode.u16Height) ||
            (pstIspCtx->stSnsImageMode.f32Fps    != pstIspCtx->stPreSnsImageMode.f32Fps) ||
            (pstIspCtx->stSnsImageMode.u8SnsMode != pstIspCtx->stPreSnsImageMode.u8SnsMode)) {
            s32Ret = ISP_BlockUpdate(ViPipe, &pstIspCtx->stBlockAttr);
            if (HI_SUCCESS != s32Ret) {
                ISP_TRACE(HI_DBG_ERR, "pipe:%d Update ISP Block Attr Failed !\n", ViPipe);
                return s32Ret;
            }

            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_PIPE_SIZE, &pstIspCtx->stPipeSize);
            if (HI_SUCCESS != s32Ret) {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Pipe size failed\n", ViPipe);
                return s32Ret;
            }

            /* p2en info */
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_P2EN_INFO_GET, &pstIspCtx->bIsp0P2En);
            if (HI_SUCCESS != s32Ret) {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] get p2en info failed\n", ViPipe);
                return s32Ret;
            }

            stSnsImageMode.u16Width  = pstIspCtx->stSnsImageMode.u16Width;
            stSnsImageMode.u16Height = pstIspCtx->stSnsImageMode.u16Height;
            stSnsImageMode.f32Fps    = pstIspCtx->stSnsImageMode.f32Fps;
            stSnsImageMode.u8SnsMode = pstIspCtx->stSnsImageMode.u8SnsMode;

            memcpy(&pstIspCtx->stPreSnsImageMode, &pstIspCtx->stSnsImageMode, sizeof(ISP_SENSOR_IMAGE_MODE_S));

            s32Ret = ISP_SensorSetImageMode(ViPipe, &stSnsImageMode);

            if (HI_SUCCESS == s32Ret) {  /* Need to init sensor */
                u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);

                s32Ret = ISP_SensorSetWDRMode(ViPipe, u8WDRMode);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor wdr mode failed\n", ViPipe);
                    return s32Ret;
                }

                pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
                s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
                }

                if (pstIspCtx->stStitchAttr.bStitchEnable == HI_TRUE) {
                    ioctl(g_as32IspFd[ViPipe], ISP_SYNC_STITCH_PARAM_INIT);
                }

                s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_RES_SWITCH_SET);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "pipe:%d Config ISP Res Switch Failed with ec %#x!\n", ViPipe, s32Ret);
                    return s32Ret;
                }

                s32Ret = ISP_RegCfgCtrl(ViPipe);
                if (HI_SUCCESS != s32Ret) {
                    return s32Ret;
                }

                s32Ret = ISP_GetBeLastBuf(ViPipe);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "Pipe:%d Get be last bufs failed %x!\n", ViPipe, s32Ret);
                    return s32Ret;
                }

                ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_CHANGE_IMAGE_MODE_SET, (HI_VOID *)&stIspImageMode);
                ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET,
                             (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);
                ISP_SensorUpdateSnsReg(ViPipe);

                if (IS_FS_WDR_MODE(u8WDRMode)) {
                    s32Ret = ISP_WDRCfgSet(ViPipe);
                    if (HI_SUCCESS != s32Ret) {
                        ISP_TRACE(HI_DBG_ERR, "pipe:%d ISP_WDRCfgSet ec %#x!\n", ViPipe, s32Ret);
                        return s32Ret;
                    }
                }

                s32Ret = ISP_SwitchRegSet(ViPipe);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "pipe:%d set reg config failed!\n", ViPipe);
                    return HI_FAILURE;
                }

                s32Ret = ISP_SensorSwitch(ViPipe);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "pipe:%d init sensor failed!\n", ViPipe);
                    return HI_FAILURE;
                }
            } else {
                ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET, (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);
            }

            hi_ext_top_res_switch_write(ViPipe, HI_TRUE);

            ISP_SensorUpdateDefault(ViPipe);
            ISP_SensorUpdateBlc(ViPipe);
            ISP_SensorGetDefault(ViPipe, &pstSnsDft);

            pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorID = pstSnsDft->stSensorMode.u32SensorID;
            pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorMode = pstSnsDft->stSensorMode.u8SensorMode;

            ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);

            memcpy(&pstIspCtx->stDngInfoCtrl.pstIspDng->stDngRawFormat, &pstSnsDft->stSensorMode.stDngRawFormat,
                   sizeof(DNG_RAW_FORMAT_S));

            if ((pstSnsRegsInfo->u8Cfg2ValidDelayMax > CFG2VLD_DLY_LIMIT) || (pstSnsRegsInfo->u8Cfg2ValidDelayMax < 1)) {
                ISP_TRACE(HI_DBG_ERR, "pipe:%d Delay of config to invalid is:0x%x\n", ViPipe, pstSnsRegsInfo->u8Cfg2ValidDelayMax);
                pstSnsRegsInfo->u8Cfg2ValidDelayMax = 1;
            }

            pstIspCtx->stLinkage.u8Cfg2ValidDelayMax = pstSnsRegsInfo->u8Cfg2ValidDelayMax;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_WDRCfgSet(VI_PIPE ViPipe)
{
    HI_U8  i;
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;
    ISP_REGCFG_S       *pstRegCfg = HI_NULL;
    ISP_WDR_CFG_S      stWDRCfg;

    ISP_REGCFG_GET_CTX(ViPipe, pstRegCfg);
    ISP_CHECK_POINTER(pstRegCfg);

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    memset(&stWDRCfg, 0, sizeof(ISP_WDR_CFG_S));

    stWDRCfg.u8WDRMode = pstIspCtx->u8SnsWDRMode;

    for (i = 0; i < EXP_RATIO_NUM; i++) {
        stWDRCfg.au32ExpRatio[i] = CLIP3(pstSnsDft->stWdrSwitchAttr.au32ExpRatio[i], 0x40, 0xFFF);
    }

    memcpy(&stWDRCfg.stWdrRegCfg, &pstRegCfg->stRegCfg.stAlgRegCfg[0].stWdrRegCfg.stSyncRegCfg,
           sizeof(ISP_FSWDR_SYNC_CFG_S));

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_WDR_CFG_SET, &stWDRCfg);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d Config WDR register Failed with ec %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SwitchWDR(VI_PIPE ViPipe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8  u8SensorWDRMode;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;
    ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = HI_NULL;
    ISP_CMOS_BLACK_LEVEL_S *pstSnsBlackLevel = HI_NULL;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    s32Ret = ISP_SttAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init stt address failed!\n", ViPipe);
        return s32Ret;
    }

    /* 0. block attr change */
    s32Ret = ISP_BlockUpdate(ViPipe, &pstIspCtx->stBlockAttr);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Update ISP Block Attr Failed !\n", ViPipe);
        return s32Ret;
    }

    s32Ret = ISP_RegCfgCtrl(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    /* 1. switch sensor to WDR mode */
    u8SensorWDRMode = pstIspCtx->u8SnsWDRMode;
    s32Ret = ISP_SensorSetWDRMode(ViPipe, u8SensorWDRMode);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor WDR mode failed!\n", ViPipe);
        return HI_FAILURE;
    }

    /* 2. get last buffer, note: when sensor timing change, callback */
    if (pstIspCtx->stStitchAttr.bStitchEnable == HI_TRUE) {
        ioctl(g_as32IspFd[ViPipe], ISP_SYNC_STITCH_PARAM_INIT);
    }

    s32Ret = ISP_GetBeLastBuf(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get be last bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    ISP_SensorUpdateDefault(ViPipe);
    ISP_SensorUpdateBlc(ViPipe);
    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);

    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_BLC_SET, (HI_VOID *)&pstSnsBlackLevel->au16BlackLevel[1]);

    /* 3. init the common part of extern registers and real registers */
    ISP_ExtRegsInitialize(ViPipe);
    ISP_RegsInitialize(ViPipe);

    /* 4. isp algs global variable initialize */
    // ISP_GlobalInitialize(ViPipe);

    /* 5. notify algs to switch WDR mode */
    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_WDR_MODE_SET, (HI_VOID *)&u8SensorWDRMode);

    s32Ret = ISP_WDRCfgSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] ISP_WDRCfgSet error \n", ViPipe);
        return s32Ret;
    }

    s32Ret = ISP_SwitchRegSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set reg config failed!\n", ViPipe);
        return HI_FAILURE;
    }

    /* 6. switch sensor to WDR mode */
    ISP_SensorUpdateSnsReg(ViPipe);
    s32Ret = ISP_SensorSwitch(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init sensor failed!\n", ViPipe);
        return HI_FAILURE;
    }

    /* 7. get new sensor default param */
    ISP_SensorGetDefault(ViPipe, &pstSnsDft);

    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorID = pstSnsDft->stSensorMode.u32SensorID;
    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorMode = pstSnsDft->stSensorMode.u8SensorMode;

    ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);

    if ((pstSnsRegsInfo->u8Cfg2ValidDelayMax > CFG2VLD_DLY_LIMIT) || (pstSnsRegsInfo->u8Cfg2ValidDelayMax < 1)) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Delay of config to invalid is:0x%x\n", ViPipe, pstSnsRegsInfo->u8Cfg2ValidDelayMax);
        pstSnsRegsInfo->u8Cfg2ValidDelayMax = 1;
    }

    pstIspCtx->stLinkage.u8Cfg2ValidDelayMax = pstSnsRegsInfo->u8Cfg2ValidDelayMax;

    pstIspCtx->u8PreSnsWDRMode = pstIspCtx->u8SnsWDRMode;
    hi_ext_top_wdr_switch_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}

static HI_S32 ISP_SwitchModeProcess(VI_PIPE ViPipe)
{
    HI_S32  s32Ret   = HI_SUCCESS;
    ISP_CMOS_SENSOR_IMAGE_MODE_S  stIspImageMode;
    ISP_CTX_S              *pstIspCtx        = HI_NULL;
    ISP_CMOS_BLACK_LEVEL_S *pstSnsBlackLevel = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    stIspImageMode.u16Width  = hi_ext_sync_total_width_read(ViPipe);
    stIspImageMode.u16Height = hi_ext_sync_total_height_read(ViPipe);
    stIspImageMode.f32Fps    = pstIspCtx->stSnsImageMode.f32Fps;
    stIspImageMode.u8SnsMode = pstIspCtx->stSnsImageMode.u8SnsMode;

    /* 1. if switch wdr mode, should disable festt enable first */
    if (pstIspCtx->bChangeWdrMode == HI_TRUE) {
        s32Ret = ISP_ResetFeSttEn(ViPipe);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Reset FeStt enable failed\n", ViPipe);
            return s32Ret;
        }
    }

    /* 2.Get info: WDR, HDR, Stitch */
    /* WDR attribute */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_WDR_ATTR, &pstIspCtx->stWdrAttr);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get WDR attr failed\n", ViPipe);
        return s32Ret;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_PIPE_SIZE, &pstIspCtx->stPipeSize);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Pipe size failed\n", ViPipe);
        return s32Ret;
    }

    /* p2en info */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_P2EN_INFO_GET, &pstIspCtx->bIsp0P2En);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get p2en info failed\n", ViPipe);
        return s32Ret;
    }

    if (pstIspCtx->bChangeWdrMode == HI_TRUE) {
        s32Ret = ISP_SttAddrInit(ViPipe);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] init stt address failed!\n", ViPipe);
            return s32Ret;
        }
    }

    s32Ret = ISP_BlockUpdate(ViPipe, &pstIspCtx->stBlockAttr);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Update ISP Block Attr Failed !\n", ViPipe);
        return s32Ret;
    }

    pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
    }

    if (pstIspCtx->stStitchAttr.bStitchEnable == HI_TRUE) {
        ioctl(g_as32IspFd[ViPipe], ISP_SYNC_STITCH_PARAM_INIT);
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_RES_SWITCH_SET);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Config ISP Res Switch Failed with ec %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    s32Ret = ISP_RegCfgCtrl(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    s32Ret = ISP_GetBeLastBuf(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get be last bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    ISP_SensorUpdateDefault(ViPipe);
    ISP_SensorUpdateBlc(ViPipe);
    ISP_SensorGetBlc(ViPipe, &pstSnsBlackLevel);

    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_BLC_SET, (HI_VOID *)&pstSnsBlackLevel->au16BlackLevel[1]);

    /* init the common part of extern registers and real registers */
    ISP_ExtRegsInitialize(ViPipe);

    if (HI_TRUE == pstIspCtx->bChangeWdrMode) {
        ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_WDR_MODE_SET, (HI_VOID *)&pstIspCtx->u8SnsWDRMode);
    }

    if ((HI_TRUE == pstIspCtx->bChangeIspRes) || (HI_TRUE == pstIspCtx->bChangeImageMode)) {
        ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_CHANGE_IMAGE_MODE_SET, (HI_VOID *)&stIspImageMode);
    }
    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET,
                 (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);

    s32Ret = ISP_WDRCfgSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] ISP_WDRCfgSet error \n", ViPipe);
        return s32Ret;
    }

    s32Ret = ISP_SwitchRegSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set reg config failed!\n", ViPipe);
        return HI_FAILURE;
    }

    ISP_SensorUpdateSnsReg(ViPipe);
    s32Ret = ISP_SensorSwitch(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init sensor failed!\n", ViPipe);
        return HI_FAILURE;
    }

    pstIspCtx->u8PreSnsWDRMode = pstIspCtx->u8SnsWDRMode;

    if (pstIspCtx->bChangeWdrMode == HI_TRUE) {
        hi_ext_top_wdr_switch_write(ViPipe, HI_TRUE);
    }

    if ((pstIspCtx->bChangeIspRes == HI_TRUE) || (HI_TRUE == pstIspCtx->bChangeImageMode)) {
        hi_ext_top_res_switch_write(ViPipe, HI_TRUE);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SwitchRes(VI_PIPE ViPipe)
{
    HI_U8  u8WDRMode;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Value = 0;
    HI_VOID *pValue = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CMOS_SENSOR_IMAGE_MODE_S  stIspImageMode;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    u32Value  = hi_ext_system_fps_base_read(ViPipe);
    pValue    = (HI_VOID *)&u32Value;

    stIspImageMode.u16Width  = hi_ext_sync_total_width_read(ViPipe);
    stIspImageMode.u16Height = hi_ext_sync_total_height_read(ViPipe);
    stIspImageMode.f32Fps    = *(HI_FLOAT *)pValue;
    stIspImageMode.u8SnsMode = hi_ext_system_sensor_mode_read(ViPipe);

    s32Ret = ISP_BlockUpdate(ViPipe, &pstIspCtx->stBlockAttr);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Update ISP Block Attr Failed !\n", ViPipe);
        return s32Ret;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_PIPE_SIZE, &pstIspCtx->stPipeSize);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Pipe size failed\n", ViPipe);
        return s32Ret;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_P2EN_INFO_GET, &pstIspCtx->bIsp0P2En);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get p2en info failed\n", ViPipe);
        return s32Ret;
    }

    pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
    }

    if (pstIspCtx->stStitchAttr.bStitchEnable == HI_TRUE) {
        ioctl(g_as32IspFd[ViPipe], ISP_SYNC_STITCH_PARAM_INIT);
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_RES_SWITCH_SET);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Config ISP Res Switch Failed with ec %#x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    s32Ret = ISP_RegCfgCtrl(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    s32Ret = ISP_GetBeLastBuf(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get be last bufs failed %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    ISP_SensorUpdateDefault(ViPipe);
    ISP_SensorUpdateBlc(ViPipe);

    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_CHANGE_IMAGE_MODE_SET, (HI_VOID *)&stIspImageMode);
    ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET,
                 (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);

    if (IS_FS_WDR_MODE(u8WDRMode)) {
        s32Ret = ISP_WDRCfgSet(ViPipe);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] ISP_WDRCfgSet ec %#x!\n", ViPipe, s32Ret);
            return s32Ret;
        }
    }

    s32Ret = ISP_SwitchRegSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set reg config failed!\n", ViPipe);
        return HI_FAILURE;
    }

    ISP_SensorUpdateSnsReg(ViPipe);
    s32Ret = ISP_SensorSwitch(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d init sensor failed!\n", ViPipe);
        return HI_FAILURE;
    }

    hi_ext_top_res_switch_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}

HI_S32 ISP_SwitchMode(VI_PIPE ViPipe)
{
    HI_BOOL bHDREnable;
    HI_U8   u8WDRMode, u8HDRMode;
    HI_U32  u32Value = 0;
    HI_S32  s32Ret   = HI_SUCCESS;
    HI_VOID   *pValue    = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CMOS_DEFAULT_S  *pstSnsDft      = HI_NULL;
    ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = NULL;
    ISP_CMOS_SENSOR_IMAGE_MODE_S  stSnsImageMode;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    u32Value  = hi_ext_system_fps_base_read(ViPipe);
    pValue    = (HI_VOID *)&u32Value;

    pstIspCtx->stSnsImageMode.f32Fps    = *(HI_FLOAT *)pValue;
    pstIspCtx->stSnsImageMode.u16Width  = hi_ext_top_sensor_width_read(ViPipe);
    pstIspCtx->stSnsImageMode.u16Height = hi_ext_top_sensor_height_read(ViPipe);
    pstIspCtx->stSnsImageMode.u8SnsMode = hi_ext_system_sensor_mode_read(ViPipe);
    pstIspCtx->stSysRect.s32X           = hi_ext_system_corp_pos_x_read(ViPipe);
    pstIspCtx->stSysRect.s32Y           = hi_ext_system_corp_pos_y_read(ViPipe);

    if (0 == pstIspCtx->u8IspImageModeFlag) {
        ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET,
                     (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);

        memcpy(&pstIspCtx->stPreSnsImageMode, &pstIspCtx->stSnsImageMode, sizeof(ISP_SENSOR_IMAGE_MODE_S));

        pstIspCtx->u8IspImageModeFlag = 1;
    } else {
        if (pstIspCtx->u8SnsWDRMode != u8WDRMode) {
            pstIspCtx->bChangeWdrMode = HI_TRUE;
        }

        if ((pstIspCtx->stSnsImageMode.u16Width  != pstIspCtx->stPreSnsImageMode.u16Width ) ||
            (pstIspCtx->stSnsImageMode.u16Height != pstIspCtx->stPreSnsImageMode.u16Height) ||
            (pstIspCtx->stSnsImageMode.f32Fps    != pstIspCtx->stPreSnsImageMode.f32Fps) ||
            (pstIspCtx->stSnsImageMode.u8SnsMode != pstIspCtx->stPreSnsImageMode.u8SnsMode)) {
            pstIspCtx->bChangeImageMode = HI_TRUE;
        }

        if (pstIspCtx->stSysRect.u32Width  != hi_ext_sync_total_width_read(ViPipe) ||
            pstIspCtx->stSysRect.u32Height != hi_ext_sync_total_height_read(ViPipe)) {
            pstIspCtx->stSysRect.u32Width  = hi_ext_sync_total_width_read(ViPipe);
            pstIspCtx->stSysRect.u32Height = hi_ext_sync_total_height_read(ViPipe);

            pstIspCtx->bChangeIspRes = HI_TRUE;
        }

        if ((HI_TRUE == pstIspCtx->bChangeWdrMode) ||
            (HI_TRUE == pstIspCtx->bChangeImageMode) ||
            (HI_TRUE == pstIspCtx->bChangeIspRes)) {
            /* HDR attribute */
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_HDR_ATTR, &pstIspCtx->stHdrAttr);
            if (HI_SUCCESS != s32Ret) {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] get HDR attr failed\n", ViPipe);
                return s32Ret;
            }

            if (pstIspCtx->bChangeWdrMode == HI_TRUE) {
                bHDREnable = (DYNAMIC_RANGE_XDR == pstIspCtx->stHdrAttr.enDynamicRange) ? 1 : 0;
                u8HDRMode  = (((bHDREnable) & 0x1) << 0x6);
                u8HDRMode  = u8HDRMode | hi_ext_system_sensor_wdr_mode_read(ViPipe);

                s32Ret = ISP_SensorSetWDRMode(ViPipe, u8HDRMode);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor wdr mode failed\n", ViPipe);
                    return s32Ret;
                }
            }

            stSnsImageMode.u16Width  = pstIspCtx->stSnsImageMode.u16Width;
            stSnsImageMode.u16Height = pstIspCtx->stSnsImageMode.u16Height;
            stSnsImageMode.f32Fps    = pstIspCtx->stSnsImageMode.f32Fps;
            stSnsImageMode.u8SnsMode = pstIspCtx->stSnsImageMode.u8SnsMode;

            memcpy(&pstIspCtx->stPreSnsImageMode, &pstIspCtx->stSnsImageMode, sizeof(ISP_SENSOR_IMAGE_MODE_S));

            s32Ret = ISP_SensorSetImageMode(ViPipe, &stSnsImageMode);
            if (HI_SUCCESS == s32Ret) {  /* Need to init sensor */
                pstIspCtx->u8SnsWDRMode = u8WDRMode;
                pstIspCtx->u8Count      = 0;
                pstIspCtx->stSpecialOpt.bFeSttUpdate = HI_TRUE; /* used for fe statistics */

                s32Ret = ISP_SwitchModeProcess(ViPipe);
                if (HI_SUCCESS != s32Ret) {
                    ISP_TRACE(HI_DBG_ERR, "ISP[%d] Switch Mode failed!\n", ViPipe);
                    return s32Ret;
                }
                //} else if (s32Ret == ISP_DO_NOT_NEED_SWITCH_IMAGEMODE) {
            } else {
                if (pstIspCtx->bChangeIspRes == HI_TRUE) {
                    ISP_SwitchRes(ViPipe);
                }
                ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_AE_FPS_BASE_SET, (HI_VOID *)&pstIspCtx->stSnsImageMode.f32Fps);
            }
                // else
                // {
                //     return s32Ret;
                // }


            ISP_SensorGetDefault(ViPipe, &pstSnsDft);

            pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorID = pstSnsDft->stSensorMode.u32SensorID;
            pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorMode = pstSnsDft->stSensorMode.u8SensorMode;

            ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);

            memcpy(&pstIspCtx->stDngInfoCtrl.pstIspDng->stDngRawFormat, &pstSnsDft->stSensorMode.stDngRawFormat,
                   sizeof(DNG_RAW_FORMAT_S));

            if ((pstSnsRegsInfo->u8Cfg2ValidDelayMax > CFG2VLD_DLY_LIMIT) || (pstSnsRegsInfo->u8Cfg2ValidDelayMax < 1)) {
                ISP_TRACE(HI_DBG_ERR, "ISP[%d] Delay of config to invalid is:0x%x\n", ViPipe, pstSnsRegsInfo->u8Cfg2ValidDelayMax);
                pstSnsRegsInfo->u8Cfg2ValidDelayMax = 1;
            }

            pstIspCtx->stLinkage.u8Cfg2ValidDelayMax = pstSnsRegsInfo->u8Cfg2ValidDelayMax;
        }

        pstIspCtx->bChangeWdrMode   = HI_FALSE;
        pstIspCtx->bChangeImageMode = HI_FALSE;
        pstIspCtx->bChangeIspRes    = HI_FALSE;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SwitchWDRMode(VI_PIPE ViPipe)
{
    HI_U8  u8WDRMode;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32WDRmode;
    HI_U32 u32Value = 0;
    HI_BOOL bHDREnable;
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsImageMode;
    HI_VOID *pValue = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    u32WDRmode = hi_ext_system_sensor_wdr_mode_read(ViPipe);

    /* swtich linear/WDR mode, width/height, fps  */
    if (pstIspCtx->u8SnsWDRMode != u32WDRmode) {
        pstIspCtx->u8SnsWDRMode = u32WDRmode;
        // pstIspCtx->bChangeMode  = HI_TRUE;
        pstIspCtx->u8Count = 0;

        s32Ret = ISP_ResetFeSttEn(ViPipe);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Reset FeStt enable failed\n", ViPipe);
            return s32Ret;
        }

        /* Get info: PipeBindDev, RunningMode, WDR, HDR, Stitch */
        /* WDR attribute */
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_WDR_ATTR, &pstIspCtx->stWdrAttr);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] get WDR attr failed\n", ViPipe);
            return s32Ret;
        }

        /* HDR attribute */
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_HDR_ATTR, &pstIspCtx->stHdrAttr);

        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] get HDR attr failed\n", ViPipe);
            return s32Ret;
        }

        bHDREnable = (DYNAMIC_RANGE_XDR == pstIspCtx->stHdrAttr.enDynamicRange) ? 1 : 0;
        u8WDRMode = (((bHDREnable) & 0x1 ) << 0x6);
        u8WDRMode = u8WDRMode | hi_ext_system_sensor_wdr_mode_read(ViPipe);

        s32Ret = ISP_SensorSetWDRMode(ViPipe, u8WDRMode);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor wdr mode failed\n", ViPipe);
            return s32Ret;
        }

        stSnsImageMode.u16Width = hi_ext_top_sensor_width_read(ViPipe);
        stSnsImageMode.u16Height = hi_ext_top_sensor_height_read(ViPipe);
        u32Value = hi_ext_system_fps_base_read(ViPipe);
        pValue = (HI_VOID *)&u32Value;
        stSnsImageMode.f32Fps = *(HI_FLOAT *)pValue;
        stSnsImageMode.u8SnsMode = hi_ext_system_sensor_mode_read(ViPipe);
        s32Ret = ISP_SensorSetImageMode(ViPipe, &stSnsImageMode);

        pstIspCtx->bMemInit = HI_TRUE;
        s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_MEM_INFO_SET, &pstIspCtx->bMemInit);
        if (HI_SUCCESS != s32Ret) {
            pstIspCtx->bMemInit = HI_FALSE;
            VReg_ReleaseAll(ViPipe);

            ISP_TRACE(HI_DBG_ERR, "ISP[%d] set Mem info failed!\n", ViPipe);
            return s32Ret;
        }

        pstIspCtx->u8SnsWDRMode              = u32WDRmode;
        pstIspCtx->stSpecialOpt.bFeSttUpdate = HI_TRUE; /* used for fe statistics */

        s32Ret = ISP_SwitchWDR(ViPipe);
        if (HI_SUCCESS != s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Switch WDR failed!\n", ViPipe);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_UpdatePosGet(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_S32 s32Ret;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_UPDATE_POS_GET, &pstIspCtx->stLinkage.u32UpdatePos);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d get update pos %x!\n", ViPipe, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_U32 ISP_FrameCntGet(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_S32 s32Ret;
    HI_U32 u32FrmCnt = 0;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_FRAME_CNT_GET, &u32FrmCnt);
    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "pipe:%d get update pos %x!\n", ViPipe, s32Ret);
        return pstIspCtx->stLinkage.u32IsoDoneFrmcnt;
    }

    return u32FrmCnt;
}

HI_S32 ISP_SnapAttrGet(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_SNAP_ATTR_S stIspSnapAttr;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_WORKING_MODE_S stIspWorkMode;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SNAP_ATTR_GET, &stIspSnapAttr);
    if (HI_SUCCESS != s32Ret) {
        // ISP_TRACE(HI_DBG_ERR, "Get snap attr failed with ec %#x!\n", s32Ret);
        return s32Ret;
    }
    memcpy(&pstIspCtx->stProParam, &stIspSnapAttr.stProParam, sizeof(ISP_PRO_PARAM_S));
    pstIspCtx->stLinkage.enSnapType = stIspSnapAttr.enSnapType;
    if ((stIspSnapAttr.s32PicturePipeId == stIspSnapAttr.s32PreviewPipeId) && (stIspSnapAttr.s32PreviewPipeId != -1)) {
        pstIspCtx->stLinkage.enSnapPipeMode = ISP_SNAP_PREVIEW_PICTURE;
    } else if (stIspSnapAttr.s32PicturePipeId == ViPipe) {
        pstIspCtx->stLinkage.enSnapPipeMode = ISP_SNAP_PICTURE;
    } else if (stIspSnapAttr.s32PreviewPipeId == ViPipe) {
        pstIspCtx->stLinkage.enSnapPipeMode = ISP_SNAP_PREVIEW;
    } else {
        pstIspCtx->stLinkage.enSnapPipeMode = ISP_SNAP_NONE;
    }
    pstIspCtx->stLinkage.bLoadCCM = stIspSnapAttr.bLoadCCM;
    pstIspCtx->stLinkage.s32PicturePipeId = stIspSnapAttr.s32PicturePipeId;
    pstIspCtx->stLinkage.s32PreviewPipeId = stIspSnapAttr.s32PreviewPipeId;

    if (stIspSnapAttr.s32PicturePipeId != -1) {
        s32Ret = ioctl(g_as32IspFd[stIspSnapAttr.s32PicturePipeId], ISP_WORK_MODE_GET, &stIspWorkMode);

        if (s32Ret) {
            ISP_TRACE(HI_DBG_ERR, "get isp work mode failed!\n");
            return s32Ret;
        }
        pstIspCtx->stLinkage.enPictureRunningMode = stIspWorkMode.enIspRunningMode;
    } else {
        pstIspCtx->stLinkage.enPictureRunningMode = ISP_MODE_RUNNING_BUTT;
    }

    if (stIspSnapAttr.s32PicturePipeId == stIspSnapAttr.s32PreviewPipeId) {
        pstIspCtx->stLinkage.enPreviewRunningMode = pstIspCtx->stLinkage.enPictureRunningMode;
    } else {
        if (stIspSnapAttr.s32PreviewPipeId != -1) {
            s32Ret = ioctl(g_as32IspFd[stIspSnapAttr.s32PreviewPipeId], ISP_WORK_MODE_GET, &stIspWorkMode);

            if (s32Ret) {
                ISP_TRACE(HI_DBG_ERR, "get isp work mode failed!\n");
                return s32Ret;
            }
            pstIspCtx->stLinkage.enPreviewRunningMode = stIspWorkMode.enIspRunningMode;
        } else {
            pstIspCtx->stLinkage.enPreviewRunningMode = ISP_MODE_RUNNING_BUTT;
        }
    }

    return HI_SUCCESS;
}

HI_U8 ISP_IsoIndexCal(VI_PIPE ViPipe, HI_U32 u32AeDoneFrmCnt)
{
    HI_U8 u8Index;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    u8Index = pstIspCtx->stLinkage.u8Cfg2ValidDelayMax;

    if (pstIspCtx->stLinkage.u32IsoDoneFrmcnt > u32AeDoneFrmCnt) {  // preview pipe the last frame dont run complete.
        u8Index = u8Index - (pstIspCtx->stLinkage.u32IsoDoneFrmcnt - u32AeDoneFrmCnt);
    } else if (pstIspCtx->stLinkage.u32IsoDoneFrmcnt < u32AeDoneFrmCnt)  // the preview pipe run first.
    {
        u8Index = u8Index + ((u32AeDoneFrmCnt - pstIspCtx->stLinkage.u32IsoDoneFrmcnt) - 1);
    } else if (pstIspCtx->stLinkage.u32IsoDoneFrmcnt == u32AeDoneFrmCnt)  // the picture pipe run first.
    {
        u8Index = u8Index - 1;
    }

    if (0 < pstIspCtx->stLinkage.u32UpdatePos) {
        u8Index = u8Index - 1;
    }

    u8Index = MIN2(u8Index, ISP_SYNC_ISO_BUF_MAX - 1);

    return u8Index;
}

HI_S32 ISP_SnapPreProcess(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CTX_S *pstPreviewIspCtx = HI_NULL;
    SNAP_TYPE_E enSnapType;
    ISP_SNAP_PIPE_MODE_E enSnapPipeMode;
    HI_U8 u8Index = 0;
    HI_U32 u32PrePipeFrmCnt = 0;
    HI_U8 i;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    ISP_SnapAttrGet(ViPipe);

    enSnapType = pstIspCtx->stLinkage.enSnapType;
    enSnapPipeMode = pstIspCtx->stLinkage.enSnapPipeMode;

    if (ISP_SNAP_NONE == enSnapPipeMode) {
        return HI_SUCCESS;
    }

    if ((pstIspCtx->stLinkage.s32PreviewPipeId > -1) && (pstIspCtx->stLinkage.s32PreviewPipeId < ISP_MAX_PIPE_NUM)) {
        ISP_GET_CTX(pstIspCtx->stLinkage.s32PreviewPipeId, pstPreviewIspCtx);
        u32PrePipeFrmCnt = pstPreviewIspCtx->stLinkage.u32IsoDoneFrmcnt;
    }

    if (((ISP_SNAP_PICTURE == enSnapPipeMode) || (ISP_SNAP_PREVIEW_PICTURE == enSnapPipeMode)) &&
        (IS_OFFLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
         IS_STRIPING_MODE(pstIspCtx->stBlockAttr.enIspRunningMode))) {
        ISP_SnapRegCfgGet(ViPipe, &pstIspCtx->stSnapIspInfo);

        if (SNAP_STATE_CFG == pstIspCtx->stSnapIspInfo.enSnapState) {
            pstIspCtx->stLinkage.bSnapState = HI_TRUE;
            pstIspCtx->stLinkage.u32Iso = pstIspCtx->stSnapIspInfo.stIspCfgInfo.u32ISO;
            pstIspCtx->stLinkage.u32IspDgain = pstIspCtx->stSnapIspInfo.stIspCfgInfo.u32IspDgain;
            pstIspCtx->stLinkage.u32ColorTemp = pstIspCtx->stSnapIspInfo.stIspCfgInfo.u32ColorTemperature;
            pstIspCtx->stLinkage.u8ProIndex = pstIspCtx->stSnapIspInfo.u32ProIndex;
        } else {
            pstIspCtx->stLinkage.bSnapState = HI_FALSE;
            pstIspCtx->stLinkage.u8ProIndex = 0;
        }
    }

    if ((ISP_SNAP_PREVIEW == enSnapPipeMode) || (ISP_SNAP_PREVIEW_PICTURE == enSnapPipeMode)) {
        if (SNAP_TYPE_PRO == enSnapType) {
            pstIspCtx->stLinkage.bProTrigger = ISP_ProTriggerGet(ViPipe);
            if (HI_TRUE == pstIspCtx->stLinkage.bProTrigger) {
                ISP_AlgsCtrl(pstIspCtx->astAlgs, ViPipe, ISP_PROTRIGGER_SET, (HI_VOID *)&pstIspCtx->stProParam);
            }
        }

        for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
            pstIspCtx->stLinkage.au8ProIndexBuf[i] = pstIspCtx->stLinkage.au8ProIndexBuf[i - 1];
        }
        if ((HI_TRUE == pstIspCtx->stLinkage.bProTrigger) ||
            ((pstIspCtx->u8ProFrmCnt > 0) && (pstIspCtx->u8ProFrmCnt < pstIspCtx->stProParam.u8ProFrameNum))) {
            pstIspCtx->stLinkage.au8ProIndexBuf[0] = pstIspCtx->u8ProFrmCnt + 1;
        } else {
            pstIspCtx->stLinkage.au8ProIndexBuf[0] = 0;
        }
        pstIspCtx->stLinkage.u8ProIndex = pstIspCtx->stLinkage.au8ProIndexBuf[pstIspCtx->stLinkage.u8Cfg2ValidDelayMax];
        if (0 < pstIspCtx->stLinkage.u32UpdatePos) {
            pstIspCtx->stLinkage.u8ProIndex = pstIspCtx->stLinkage.au8ProIndexBuf[pstIspCtx->stLinkage.u8Cfg2ValidDelayMax - 1];
        }
    }

    if (IS_ONLINE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode) ||
        IS_SIDEBYSIDE_MODE(pstIspCtx->stBlockAttr.enIspRunningMode)) {
        if (ISP_SNAP_PICTURE == enSnapPipeMode) {
            u8Index = ISP_IsoIndexCal(ViPipe, u32PrePipeFrmCnt);
            if (HI_NULL != pstPreviewIspCtx) {
                pstIspCtx->stLinkage.u32Iso = pstPreviewIspCtx->stLinkage.au32SyncIsoBuf[u8Index];
                pstIspCtx->stLinkage.u8ProIndex = pstPreviewIspCtx->stLinkage.au8ProIndexBuf[u8Index];
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SaveSnapInfo(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CONFIG_INFO_S stSnapInfo;
    HI_U32 i;

    ISP_GET_CTX(ViPipe, pstIspCtx);

    stSnapInfo.u32ColorTemperature = pstIspCtx->stLinkage.u32ColorTemp;
    stSnapInfo.u32ISO = pstIspCtx->stLinkage.au32SyncIsoBuf[0];
    stSnapInfo.u32IspDgain = pstIspCtx->stLinkage.u32IspDgain;
    stSnapInfo.u32ExposureTime = pstIspCtx->stLinkage.u32Inttime;
    stSnapInfo.au32WhiteBalanceGain[0] = (pstIspCtx->stLinkage.au32WhiteBalanceGain[0] + 128) >> 8;
    stSnapInfo.au32WhiteBalanceGain[1] = (pstIspCtx->stLinkage.au32WhiteBalanceGain[1] + 128) >> 8;
    stSnapInfo.au32WhiteBalanceGain[2] = (pstIspCtx->stLinkage.au32WhiteBalanceGain[2] + 128) >> 8;
    stSnapInfo.au32WhiteBalanceGain[3] = (pstIspCtx->stLinkage.au32WhiteBalanceGain[3] + 128) >> 8;

    for (i = 0; i < ISP_CAP_CCM_NUM; i++) {
        stSnapInfo.au16CapCCM[i] = pstIspCtx->stLinkage.au16CCM[i];
    }

    ISP_SnapRegCfgSet(ViPipe, &stSnapInfo);

    return HI_SUCCESS;
}

HI_S32 ISP_SetSmartInfo(VI_PIPE ViPipe, const ISP_SMART_INFO_S *pstSmartInfo)
{
    HI_U32 i;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstSmartInfo);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    for (i = 0; i < SMART_CLASS_MAX; i++) {
        hi_ext_system_smart_enable_write(ViPipe, i, pstSmartInfo->stROI[i].bEnable);
        hi_ext_system_smart_available_write(ViPipe, i, pstSmartInfo->stROI[i].bAvailable);
        hi_ext_system_smart_luma_write(ViPipe, i, pstSmartInfo->stROI[i].u8Luma);
    }

	hi_ext_system_smart_update_write(ViPipe, HI_TRUE);

    return HI_SUCCESS;
}

HI_S32 ISP_GetSmartInfo(VI_PIPE ViPipe, ISP_SMART_INFO_S *pstSmartInfo)
{
    HI_U32 i;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstSmartInfo);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    for (i = 0; i < SMART_CLASS_MAX; i++) {
        pstSmartInfo->stROI[i].bEnable    = hi_ext_system_smart_enable_read(ViPipe, i);
        pstSmartInfo->stROI[i].bAvailable = hi_ext_system_smart_available_read(ViPipe, i);
        pstSmartInfo->stROI[i].u8Luma     = hi_ext_system_smart_luma_read(ViPipe, i);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_SetProCalcDone(VI_PIPE ViPipe)
{
    HI_S32 s32Ret = 0;

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SET_PROCALCDONE);

    if (s32Ret) {
        ISP_TRACE(HI_DBG_ERR, "set isp pro calculate done failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 ISP_StitchSyncExit(VI_PIPE ViPipe)
{
    HI_U32  k;
    VI_PIPE ViPipeS;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    /* 1. check status */
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if ((HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable) && (HI_TRUE != pstIspCtx->stStitchAttr.bMainPipe)) {
        for (k = 0; k < pstIspCtx->stStitchAttr.u8StitchPipeNum; k++) {
            ViPipeS = pstIspCtx->stStitchAttr.as8StitchBindId[k];
            ISP_GET_CTX(ViPipeS, pstIspCtx);

            while ((HI_TRUE == pstIspCtx->stStitchAttr.bMainPipe) && (HI_FALSE != pstIspCtx->bMemInit)) {
                usleep(10);
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_StitchSyncRun(VI_PIPE ViPipe)
{
    HI_U8 k;
    VI_PIPE ViPipeId;
    ISP_CTX_S *pstIspCtx  = HI_NULL;
    ISP_CTX_S *pstIspCtxS = HI_NULL;

    /* 1. check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if (HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable) {
        if (HI_TRUE == pstIspCtx->stStitchAttr.bMainPipe) {
            for (k = 0; k < pstIspCtx->stStitchAttr.u8StitchPipeNum; k++) {
                ViPipeId = pstIspCtx->stStitchAttr.as8StitchBindId[k];
                ISP_GET_CTX(ViPipeId, pstIspCtxS);

                if (HI_TRUE != pstIspCtxS->bMemInit) {
                    return HI_FAILURE;
                }

                if (HI_TRUE != pstIspCtxS->stIspParaRec.bInit) {
                    return HI_FAILURE;
                }
            }
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_StitchIsoSync(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_CTX_S *pstMainIspCtx = HI_NULL;
    HI_U8 u8Index;
    HI_U32 u32MainPipeFrmCnt;
    VI_PIPE MainPipe;
    ISP_GET_CTX(ViPipe, pstIspCtx);

    if (HI_TRUE == pstIspCtx->stStitchAttr.bStitchEnable) {
        if (HI_TRUE != pstIspCtx->stStitchAttr.bMainPipe) {
            MainPipe = pstIspCtx->stStitchAttr.as8StitchBindId[0];
            ISP_CHECK_PIPE(MainPipe);
            ISP_GET_CTX(ViPipe, pstMainIspCtx);

            u32MainPipeFrmCnt = pstMainIspCtx->stLinkage.u32IsoDoneFrmcnt;
            u8Index = ISP_IsoIndexCal(ViPipe, u32MainPipeFrmCnt);
            pstIspCtx->stLinkage.u32Iso = pstMainIspCtx->stLinkage.au32SyncIsoBuf[u8Index];
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_Run(VI_PIPE ViPipe)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_VOID *pStat = HI_NULL;
    HI_VOID *pRegCfg = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);

    s32Ret = ISP_StitchSyncRun(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return HI_SUCCESS;
    }

    /*  get isp BeBuf info. */
    s32Ret = ISP_GetBeFreeBuf(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    /*  get statistics buf info. */
    s32Ret = ISP_StatisticsGetBuf(ViPipe, &pStat);
    if (HI_SUCCESS != s32Ret) {
        if (0 != pstIspCtx->u32FrameCnt) {
            return s32Ret;
        }

        pstIspCtx->stLinkage.bStatReady = HI_FALSE;
    } else {
        pstIspCtx->stLinkage.bStatReady = HI_TRUE;
    }

    s32Ret = ISP_SetCfgBeBufState(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    /* get regcfg */
    ISP_GetRegCfgCtx(ViPipe, &pRegCfg);

    ISP_ReadExtregs(ViPipe);

    if (pstIspCtx->bFreezeFw) {
        ISP_RegCfgInfoSet(ViPipe);
        return HI_SUCCESS;
    }

    ISP_UpdatePosGet(ViPipe);

    ISP_SnapPreProcess(ViPipe);

    ISP_StitchIsoSync(ViPipe);

    pstIspCtx->u32FrameCnt++;

    ISP_DbgRunBgn(&pstIspCtx->stIspDbg, pstIspCtx->u32FrameCnt);

    ISP_AlgsRun(pstIspCtx->astAlgs, ViPipe, pStat, pRegCfg, 0);

    /* update info */
    if (ISP_SNAP_PICTURE != pstIspCtx->stLinkage.enSnapPipeMode) {
        ISP_UpdateInfo(ViPipe);

        ISP_UpdateFrameInfo(ViPipe);
    }

    ISP_ProcWrite(pstIspCtx->astAlgs, ViPipe);

    ISP_UpdateAttachInfo(ViPipe);

    ISP_UpdateColorGamutinfo(ViPipe);

    ISP_DbgRunEnd(&pstIspCtx->stIspDbg, pstIspCtx->u32FrameCnt);

    /* release statistics buf info. */
    if (HI_TRUE == pstIspCtx->stLinkage.bStatReady) {
        ISP_StatisticsPutBuf(ViPipe);
    }

    /* record the register config infomation to fhy and kernel,and be valid in next frame. */
    s32Ret = ISP_RegCfgInfoSet(ViPipe);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }

    if (((pstIspCtx->u8ProFrmCnt > 0) && (pstIspCtx->u8ProFrmCnt < pstIspCtx->stProParam.u8ProFrameNum + 4)) ||
        (HI_TRUE == pstIspCtx->stLinkage.bProTrigger)) {
        pstIspCtx->u8ProFrmCnt++;
    } else {
        pstIspCtx->u8ProFrmCnt = 0;
    }

    if ((0 == pstIspCtx->u32FrameCnt % DIV_0_TO_1(pstIspCtx->stLinkage.u8AERunInterval)) ||
        (pstIspCtx->u8ProFrmCnt > 0)) {
        if (!pstIspCtx->stLinkage.bDefectPixel) {
            ISP_SyncCfgSet(ViPipe);
        }
    }

    ISP_UpdateExtRegs(ViPipe);

    /* save capture info */
    ISP_SaveSnapInfo(ViPipe);

    ISP_UpdateDngImageDynamicInfo(ViPipe);

    /* pro snap mode, ae calculate done */
    if (1 == pstIspCtx->u8ProFrmCnt) {
        ISP_SetProCalcDone(ViPipe);
    }

    return HI_SUCCESS;
}

HI_S32 ISP_Exit(VI_PIPE ViPipe)
{
    HI_U8  i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32RetRegCfg;
    HI_S32 s32RetShpParam;
    HI_S32 s32RetProcNrPara;
    HI_S32 s32RetProInfo;
    HI_S32 s32RetDngInfo;
    HI_S32 s32RetColorGamut;
    HI_S32 s32RetAttachInfo;
    HI_S32 s32RetFrameInfo;
    HI_S32 s32RetUpdateInof;
    HI_S32 s32RetTransInfo;
    HI_S32 s32RetProc;
    HI_S32 s32RetStatic;
    HI_S32 s32RetSpecAwbBuf;
    HI_S32 s32RetClutBuf;
    HI_S32 s32RetSttBuf;
    HI_S32 s32RetBeBuf;
    HI_S32 s32RetBloack;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    const HI_U64 u64Handsignal = ISP_EXIT_HAND_SIGNAL;

    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    /* Set handsignal */
    if (ioctl(g_as32IspFd[ViPipe], ISP_RUN_STATE_SET, &u64Handsignal)) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set run state failed!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    memset(&pstIspCtx->stIspParaRec, 0, sizeof(ISP_PARA_REC_S));

    hi_ext_top_wdr_cfg_write(ViPipe, pstIspCtx->stIspParaRec.bWDRCfg);
    hi_ext_top_pub_attr_cfg_write(ViPipe, pstIspCtx->stIspParaRec.bPubCfg);

    if (HI_FALSE == pstIspCtx->bISPYuvMode) {
        s32Ret = ISP_SensorExit(ViPipe);
    }

    ISP_AlgsExit(ViPipe, pstIspCtx->astAlgs);

    ISP_AlgsUnRegister(ViPipe);

    //ISP_LibsUnRegister(ViPipe);
    s32RetRegCfg     = ISP_RegCfgExit(ViPipe);
    s32RetShpParam   = ISP_ProShpParamExit(ViPipe);
    s32RetProcNrPara = ISP_ProNrParamExit(ViPipe);
    s32RetProInfo    = ISP_ProInfoExit(ViPipe);
    s32RetDngInfo    = ISP_DngInfoExit(ViPipe);
    s32RetColorGamut = ISP_ColorGamutInfoExit(ViPipe);
    s32RetAttachInfo = ISP_AttachInfoExit(ViPipe);
    s32RetFrameInfo  = ISP_FrameInfoExit(ViPipe);
    s32RetUpdateInof = ISP_UpdateInfoExit(ViPipe);
    s32RetTransInfo  = ISP_TransInfoExit(ViPipe);
    s32RetProc       = ISP_ProcExit(ViPipe);
    s32RetStatic     = ISP_StatisticsExit(ViPipe);
    s32RetSpecAwbBuf = ISP_SpecAwbBufExit(ViPipe);
    s32RetClutBuf    = ISP_ClutBufExit(ViPipe);
    s32RetSttBuf     = ISP_SttBufExit(ViPipe);
    s32RetBeBuf      = ISP_CfgBeBufExit(ViPipe);
    s32RetBloack     = ISP_BlockExit(ViPipe);
    /* exit global variables */
    pstIspCtx->bMemInit                 = HI_FALSE;
    pstIspCtx->stIspParaRec.bInit       = HI_FALSE;
    pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
    pstIspCtx->stLinkage.bRunOnce       = HI_FALSE;

    for (i = 0; i < ISP_STRIPING_MAX_NUM; i++) {
        pstIspCtx->pIspBeVirtAddr[i]  = HI_NULL;
        pstIspCtx->pViProcVirtAddr[i] = HI_NULL;
    }

    if (ioctl(g_as32IspFd[ViPipe], ISP_RESET_CTX)) {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] reset ctx failed!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* release vregs */
    VReg_Munmap(ISP_FE_REG_BASE(ViPipe), FE_REG_SIZE_ALIGN);
    VReg_Munmap(ISP_BE_REG_BASE(ViPipe), BE_REG_SIZE_ALIGN);
    VReg_Exit(ViPipe, ISP_VIR_REG_BASE(ViPipe), ISP_VREG_SIZE);
    VReg_ReleaseAll(ViPipe);
    if((s32RetRegCfg     != HI_SUCCESS) || (s32RetShpParam   != HI_SUCCESS) ||
       (s32RetProcNrPara != HI_SUCCESS) || (s32RetProInfo    != HI_SUCCESS) ||
       (s32RetDngInfo    != HI_SUCCESS) || (s32RetColorGamut != HI_SUCCESS) ||
       (s32RetAttachInfo != HI_SUCCESS) || (s32RetFrameInfo  != HI_SUCCESS) ||
       (s32RetUpdateInof != HI_SUCCESS) || (s32RetTransInfo  != HI_SUCCESS) ||
       (s32RetProc       != HI_SUCCESS) || (s32RetStatic     != HI_SUCCESS) ||
       (s32RetSpecAwbBuf != HI_SUCCESS) || (s32RetClutBuf    != HI_SUCCESS) ||
       (s32RetSttBuf     != HI_SUCCESS) || (s32RetBeBuf      != HI_SUCCESS) ||
       (s32RetBloack     != HI_SUCCESS)) {
        return HI_FAILURE;
    }
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
