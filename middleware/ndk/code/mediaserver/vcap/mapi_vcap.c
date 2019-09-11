/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap.c
 * @brief   server vcap module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "mapi_vcap_inner.h"
#include "hi_mapi_vcap_define.h"
#include "mpi_isp.h"
#include "mpi_vi.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "adpt_vcap_inner_config_define.h"
#include "sensor_interface_cfg_params.h"
#include "mpi_snap.h"
#include "hi_sns_ctrl.h"
#include "hi_ae_comm.h"
#include "hi_awb_comm.h"
#include "hi_comm_region.h"
#include "hi_mipi.h"
#include "hi_common.h"
#include "sensor_comm.h"
#include "adpt_vcap_inner_config_define.h"
#include "mpi_sys.h"
#include "hi_comm_region.h"
#include "mpi_region.h"
#include "mapi_comm_inner.h"
#include "sensor_interface_cfg_adapt.h"

/* USEC_UNIT is the unit of seconds to microseconds,
Used to calculate the time of a frame 1(s) = 1000000(us) */
#define USEC_UNIT                            (1000 * 1000)
#define MIPI_DEV_NODE                        "/dev/hi_mipi"
#define TRIGGER_DELAY_FRAME_CNT              3
#define VCAP_ISP_QUERY_SWITCHSTATE_MAX_TIMES 5000 /* max: 5000ms */
#define WDR_DUMP_REPEATE_MAX_CNT             10

#if defined (__HI3556AV100__) || defined (__HI3519AV100__)
#define GPS_NUMBERATOR_DENOMINATOR 2
#define GPS_LATITUDE_COORDINATE    3
#elif defined __HI3559AV100__
#define GPS_NUMBERATOR_DENOMINATOR 2
#define GPS_LATITUDE_COORDINATE    3
#elif defined __HI3559V200__
#define GPS_NUMBERATOR_DENOMINATOR 2
#define GPS_LATITUDE_COORDINATE    3
#elif defined __HI3518EV300__
#define GPS_NUMBERATOR_DENOMINATOR 2
#define GPS_LATITUDE_COORDINATE    3
#endif

static MAPI_VCAP_GLOB_ATTR_S g_stVcapGlobAttr;
static HI_MAPI_SENSOR_COMM_CFG_S g_stCommSnsIntfCfg;
static HI_S32 g_as32SensorType[HI_MAPI_VCAP_MAX_DEV_NUM];

static HI_VOID MAPI_VCAP_ResetStatus(HI_VOID)
{
    HI_S32 i;
    HI_S32 j;
    HI_S32 k;

    /* init vcap dev golbal pararm */
    for (i = 0; i < HI_MAPI_VCAP_MAX_DEV_NUM; i++) {
        g_stVcapGlobAttr.astGlobDevAttr[i].enDevStatus = VCAP_STATUS_NOTINIT;
        g_stVcapGlobAttr.astGlobDevAttr[i].bSensorInit = HI_FALSE;
        g_stVcapGlobAttr.astGlobDevAttr[i].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;
        g_stVcapGlobAttr.astGlobDevAttr[i].enSensorScene = HI_MAPI_SENSOR_SCENE_NORMAL;
        g_stVcapGlobAttr.astGlobDevAttr[i].u32PipeBindNum = 0;

        for (j = 0; j < HI_MAPI_VCAP_MAX_PIPE_NUM; j++) {
            g_stVcapGlobAttr.astGlobDevAttr[i].aPipeIdBind[j] = HI_INVALID_HANDLE;
        }
    }

    /* init vcap pipe and chn golbal pararm */
    for (i = 0; i < HI_MAPI_VCAP_MAX_PIPE_NUM; i++) {
        g_stVcapGlobAttr.astGlobPipeAttr[i].bIspRun = HI_FALSE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].bPipeCreate = HI_FALSE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].bPipeStart = HI_FALSE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].BindDevHdl = HI_INVALID_HANDLE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].bSnapPipeEnable = HI_FALSE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].ISPHdl = VCAP_PIPE_0 + i;

        memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].stVcapISPInitAttr, 0, sizeof(VCAP_ISP_INIT_ATTR_S));
        g_stVcapGlobAttr.astGlobPipeAttr[i].stVcapISPInitAttr.bISPInitAttr = HI_FALSE;
        g_stVcapGlobAttr.astGlobPipeAttr[i].stVcapISPWbAttr.enOpType = HI_MAPI_VCAP_OP_TYPE_AUTO;
        g_stVcapGlobAttr.astGlobPipeAttr[i].stVcapISPWbAttr.stWBMode.u32ColorTemp = 5000;
        memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].stPipeAttr, 0, sizeof(PIPE_ATTR_S));

        memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].stDumpRawInfo, 0, sizeof(VCAP_DUMP_RAWDATA_INFO_S));
        memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].stSnapAttr, 0, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S));
        g_stVcapGlobAttr.astGlobPipeAttr[i].stSnapAttr.enSnapType = SNAP_TYPE_BUTT;

        for (j = 0; j < HI_MAPI_PIPE_MAX_CHN_NUM; j++) {
            g_stVcapGlobAttr.astGlobPipeAttr[i].astPipeChnAttr[j].bChnEnable = HI_FALSE;
            memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].astPipeChnAttr[j].stChnAttr, 0,
                   sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

#ifdef SUPPORT_DIS
            g_stVcapGlobAttr.astGlobPipeAttr[i].astPipeChnAttr[j].stVcapDisAttr.enDisStatus = DIS_STATUS_STOP;
#endif

            for (k = 0; k < HI_MAPI_VCAP_OSD_MAX_NUM; k++) {
                g_stVcapGlobAttr.astGlobPipeAttr[i].astPipeChnAttr[j].astVcapOSDAttr[k].enOSDStatus =
                    VCAP_STATUS_NOTINIT;
                memset(&g_stVcapGlobAttr.astGlobPipeAttr[i].astPipeChnAttr[j].astVcapOSDAttr[k].stOSDAttr, 0,
                       sizeof(HI_MAPI_OSD_ATTR_S));
            }
        }
    }
}

HI_S32 MAPI_VCAP_Init(HI_VOID)
{
    HI_S32 s32Ret;

    memset(&g_stVcapGlobAttr, 0, sizeof(MAPI_VCAP_GLOB_ATTR_S));

    g_stVcapGlobAttr.bVcapInited = HI_FALSE;

    /* load inner default sensor param and add list */
    s32Ret = MAPI_SENSOR_Load();
    CHECK_MAPI_VCAP_RET(s32Ret, "Sensor Load Param fail\n");
    MAPI_SENSOR_LoadSnsType(g_as32SensorType);
    MAPI_SENSOR_LoadCommCfg(&g_stCommSnsIntfCfg);

    MAPI_VCAP_ResetStatus();

    g_stVcapGlobAttr.bVcapInited = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_Deinit(HI_VOID)
{
    MAPI_VCAP_ResetStatus();

    g_stVcapGlobAttr.bVcapInited = HI_FALSE;

    return HI_SUCCESS;
}

static HI_VOID MAPI_VCAP_CheckSwitchMode(HI_HANDLE VcapDevHdl, const HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    /* only in the sensor scece change, it will be need to check the switch mode */
    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSensorScene != HI_MAPI_SENSOR_SCENE_NORMAL) {
        /* The switch mode is SWITCH_MODE_SELF_NORMAL_SWITCH in default */
        g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;

        if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.enWdrMode != pstSensorAttr->enWdrMode) {
            /* wdr and liner mode change */
            g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSwitchMode = SWITCH_MODE_WDR_LINER_SWITCH;
        } else {
            /* no wdr and liner mode change */
            if ((g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Width !=
                    pstSensorAttr->stSize.u32Width)
                || (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Height !=
                          pstSensorAttr->stSize.u32Height)
                || (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.u8SnsMode != pstSensorAttr->u8SnsMode)) {
                g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSwitchMode = SWITCH_MODE_RESOLUTION_SWITCH;
            }
        }
    } else {
        g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;
    }
}

static HI_S32 MAPI_VCAP_CheckViVpssWorkMode(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_VPSS_MODE_S stVIVPSSMode;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    s32Ret = HI_MPI_SYS_GetVIVPSSMode(&stVIVPSSMode);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SYS_GetVIVPSSMode failed,VcapPipeHdl[%d]\n", VcapPipeHdl);

#ifdef __HI3559AV100__

    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_ONLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_PARALLEL_VPSS_PARALLEL) {
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

#else

    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_ONLINE_VPSS_ONLINE) {
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

#endif
    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetCurrentSensorModeSeq(const HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg,
                                                const HI_MAPI_SENSOR_ATTR_S *pstSensorAttr, HI_S32 *ps32CurrSnsSeq)
{
    HI_S32 i;
    HI_S32 s32SensorModeCnt;
    HI_S32 s32SnsModeSeq = 0;
    const HI_MAPI_SENSOR_MODE_S *pstSensorAttrs;

    s32SensorModeCnt = pstSensorCfg->stSensorMode.s32SensorModeCnt;
    pstSensorAttrs = pstSensorCfg->stSensorMode.pSensorMode;

    for (i = 0; i < s32SensorModeCnt; i++) {
        if ((HI_U32)pstSensorAttrs[i].s32Width == pstSensorAttr->stSize.u32Width
            && (HI_U32) pstSensorAttrs[i].s32Height == pstSensorAttr->stSize.u32Height
            && pstSensorAttrs[i].u8SnsMode == pstSensorAttr->u8SnsMode
            && pstSensorAttrs[i].enWdrMode == pstSensorAttr->enWdrMode) {
            s32SnsModeSeq = i;
            break;
        }
    }

    if (i == s32SensorModeCnt) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "this sensor mode (Width:%d,Height:%d,SnsMode:%d,WdrMode:%d) is unexist\n",
                       pstSensorAttr->stSize.u32Width,
                       pstSensorAttr->stSize.u32Height,
                       pstSensorAttr->u8SnsMode,
                       pstSensorAttr->enWdrMode);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    *ps32CurrSnsSeq = s32SnsModeSeq;
    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_StartMipi(HI_HANDLE VcapDevHdl, const combo_dev_attr_t *pstComboDevAttr)
{
    HI_S32 fd;
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    lane_divide_mode_t enHsMode;
    sns_clk_source_t *SnsClkSourceDev;
    sns_rst_source_t *SnsRstSourceDev;

    enHsMode = g_stCommSnsIntfCfg.enLaneIdMode; /* Temporarily put here, to be determined */
    SnsClkSourceDev = (sns_clk_source_t *)g_stCommSnsIntfCfg.aSnsClkSource;
    SnsRstSourceDev = (sns_clk_source_t *)g_stCommSnsIntfCfg.aSnsRstSource;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "open mipi dev fail\n");
        return HI_MAPI_VCAP_EINVALID_FD;
    }

    /* 1.Setting Lane Distribution Mode for MIPI Rx has no effect on SLVS */
    for (i = 0; i < HI_MAPI_VCAP_MAX_DEV_NUM; i++) {
        if (g_stVcapGlobAttr.astGlobDevAttr[i].bSensorInit) {
            /* there are some sensors already inited, so no need to set lane mode again */
            break;
        }

        /* need to set lane mode in the first time */
        if (i == HI_MAPI_VCAP_MAX_DEV_NUM - 1) {
            s32Ret = ioctl(fd, HI_MIPI_SET_HS_MODE, &enHsMode);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "set mipi divide mode failed\n");
                goto CLOSE_MIPI_DEV_NODE_ERR;
            }
        }
    }

    /* 2.Enable MIPI_Rx/slvds clock  and reset MIPI_Rx/slvds, mipi and lvds same clock */
    s32Ret = HAL_MAPI_VCAP_EnableMipiClock(fd, pstComboDevAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "enable mipi clock failed\n");
        goto CLOSE_MIPI_DEV_NODE_ERR;
    }

    /* 3.Enable sensor clock */
    s32Ret = ioctl(fd, HI_MIPI_ENABLE_SENSOR_CLOCK, &SnsClkSourceDev[VcapDevHdl]);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "enable sensor clock failed\n");
        goto CLOSE_MIPI_DEV_NODE_ERR;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSensorScene == HI_MAPI_SENSOR_SCENE_NORMAL) {
        /* 4.Reset the sensor */
        s32Ret = ioctl(fd, HI_MIPI_RESET_SENSOR, &SnsRstSourceDev[VcapDevHdl]);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "reset sensor failed\n");
            goto CLOSE_MIPI_DEV_NODE_ERR;
        }
    }

    /* 5.Set MIPI properties */
    s32Ret = ioctl(fd, HI_MIPI_SET_DEV_ATTR, pstComboDevAttr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "set mipi attr failed\n");
        goto CLOSE_MIPI_DEV_NODE_ERR;
    }

    /* 6.unreset MIPI_Rx/slvds */
    s32Ret = HAL_MAPI_VCAP_ResetMipiClock(fd, pstComboDevAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "enable mipi clock failed\n");
        goto CLOSE_MIPI_DEV_NODE_ERR;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSensorScene == HI_MAPI_SENSOR_SCENE_NORMAL) {
        /* 7.unreset sensor */
        s32Ret = ioctl(fd, HI_MIPI_UNRESET_SENSOR, &SnsRstSourceDev[VcapDevHdl]);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ureset sensor failed\n");
            goto CLOSE_MIPI_DEV_NODE_ERR;
        }
    }

    close(fd);
    return HI_SUCCESS;

CLOSE_MIPI_DEV_NODE_ERR:
    close(fd);
    return s32Ret;
}

static HI_S32 MAPI_VCAP_StopMipi(HI_HANDLE VcapDevHdl)
{
    HI_S32 fd;
    HI_S32 s32Ret;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    HI_HANDLE devno = (HI_HANDLE)VcapDevHdl;

    sns_clk_source_t *SnsClkSourceDev = (sns_clk_source_t *)g_stCommSnsIntfCfg.aSnsClkSource;
    sns_rst_source_t *SnsRstSourceDev = (sns_clk_source_t *)g_stCommSnsIntfCfg.aSnsRstSource;

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "no this sensor[%d] type\n", VcapDevHdl);

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "open mipi dev fail\n");
        return HI_MAPI_VCAP_EINVALID_FD;
    }

    if (stSensorCfg.input_mode == INPUT_MODE_BT656) {
        close(fd);
        return HI_SUCCESS;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSensorScene == HI_MAPI_SENSOR_SCENE_NORMAL) {
        /* 2.reset sensor */
        s32Ret = ioctl(fd, HI_MIPI_RESET_SENSOR, &SnsRstSourceDev[VcapDevHdl]);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "reset sensor failed\n");
            goto CLOSE_MIPI_DEV_NODE_ERR;
        }

        /* 3.disable sensor clock */
        s32Ret = ioctl(fd, HI_MIPI_DISABLE_SENSOR_CLOCK, &SnsClkSourceDev[VcapDevHdl]);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "disable sensor clock failed\n");
            goto CLOSE_MIPI_DEV_NODE_ERR;
        }
    }

    /* 4.reset MIPI_Rx/slvds and disable MIPI_Rx clock */
    s32Ret = HAI_MAPI_VCAP_DisableMipiClock(devno, fd, stSensorCfg.input_mode);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, " disable mipi clock failed\n");
        goto CLOSE_MIPI_DEV_NODE_ERR;
    }

    close(fd);
    return HI_SUCCESS;

CLOSE_MIPI_DEV_NODE_ERR:
    close(fd);
    return s32Ret;
}

static HI_S32 MAPI_VCAP_LoadMipiConfig(HI_HANDLE VcapDevHdl, const HI_MAPI_SENSOR_ATTR_S *pstSensorAttr,
                                       combo_dev_attr_t *pComboDevAttr)
{
    HI_S32 s32Ret;
    HI_S32 i;
    HI_S32 s32SnsModeSeq = 0;
    HI_S32 s32sensorsNo = (HI_S32)VcapDevHdl;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    const HI_MAPI_MIPI_INTF_S *pstSenIntf = HI_NULL;

    /* 1.According to the sensor type to find which sensor is the use of the configuration file */
    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "no this sensor[%d] type\n", VcapDevHdl);

    s32Ret = MAPI_VCAP_GetCurrentSensorModeSeq(&stSensorCfg, pstSensorAttr, &s32SnsModeSeq);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor Current Sensor Mode Seq fail,sensor[%d]\n", VcapDevHdl);

    pstSenIntf = stSensorCfg.stSensorMode.pstIntf;

    if (pstSenIntf == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sensor interface config is NULL pointer\n");
        return HI_MAPI_VCAP_ENULL_PTR;
    }

    pComboDevAttr->devno = VcapDevHdl;
    pComboDevAttr->data_rate = stSensorCfg.data_rate;
    pComboDevAttr->input_mode = stSensorCfg.input_mode;
    pComboDevAttr->img_rect.x = pstSenIntf[s32SnsModeSeq].img_rect.x;
    pComboDevAttr->img_rect.y = pstSenIntf[s32SnsModeSeq].img_rect.y;
    pComboDevAttr->img_rect.width = pstSenIntf[s32SnsModeSeq].img_rect.width;
    pComboDevAttr->img_rect.height = pstSenIntf[s32SnsModeSeq].img_rect.height;

    s32Ret = HAI_MAPI_VCAP_LoadMipiConfig(pComboDevAttr, &pstSenIntf[s32SnsModeSeq], stSensorCfg.input_mode,
                                          stSensorCfg.aLaneId[s32sensorsNo][s32SnsModeSeq]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "VcapDevHdl %d sensor interface config error ,ret:%x \n", VcapDevHdl, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_BOOL MAPI_VCAP_IsDevPipeBindChange(HI_HANDLE VcapDevHdl, const HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_U32 i;

    if (pstVCapAttr->u32PipeBindNum != g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum) {
        return HI_TRUE;
    }

    for (i = 0; i < pstVCapAttr->u32PipeBindNum; i++) {
        if (pstVCapAttr->aPipeIdBind[i] != g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i]) {
            return HI_TRUE;
        }
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsDevAttrChange(HI_HANDLE VcapDevHdl, const HI_MAPI_VCAP_DEV_ATTR_S *pstDevAttr)
{
    HI_MAPI_VCAP_DEV_ATTR_S *pstGlobDevAttr;

    pstGlobDevAttr = (HI_MAPI_VCAP_DEV_ATTR_S *)&g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stDevAttr;

    if (pstGlobDevAttr->enWdrMode != pstDevAttr->enWdrMode
        || pstGlobDevAttr->stBasSize.u32Height != pstDevAttr->stBasSize.u32Height
        || pstGlobDevAttr->stBasSize.u32Width != pstDevAttr->stBasSize.u32Width) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsPipeStaticAttrChange(HI_HANDLE VcapPipeHdl,
                                                const HI_MAPI_VCAP_PIPE_ATTR_S *pstPipeAttr)
{
    PIPE_ATTR_S *pstGlobPipeAttr;

    pstGlobPipeAttr = (PIPE_ATTR_S *)&g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stPipeAttr;

    if (pstGlobPipeAttr->enPipeType != pstPipeAttr->enPipeType) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsPipeDynamicAttrChange(HI_HANDLE VcapPipeHdl,
                                                 const HI_MAPI_VCAP_PIPE_ATTR_S *pstPipeAttr)
{
    PIPE_ATTR_S *pstGlobPipeAttr;

    pstGlobPipeAttr = (PIPE_ATTR_S *)&g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stPipeAttr;

    if (pstGlobPipeAttr->enCompressMode != pstPipeAttr->enCompressMode
        || pstGlobPipeAttr->stFrameRateCtrl.s32SrcFrameRate != pstPipeAttr->stFrameRate.s32SrcFrameRate
        || pstGlobPipeAttr->stFrameRateCtrl.s32DstFrameRate != pstPipeAttr->stFrameRate.s32DstFrameRate) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsChnStaticAttrChange(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                               const HI_MAPI_PIPE_CHN_ATTR_S *pstChnAttr)
{
    HI_MAPI_PIPE_CHN_ATTR_S *pstGlobChnAttr;

    pstGlobChnAttr = (HI_MAPI_PIPE_CHN_ATTR_S *)
                     &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stChnAttr;

    if (pstGlobChnAttr->enPixelFormat != pstChnAttr->enPixelFormat) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsChnDynamicAttrChange(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                                HI_MAPI_PIPE_CHN_ATTR_S *pstChnAttr)
{
    HI_MAPI_PIPE_CHN_ATTR_S *pstGlobChnAttr;

    pstGlobChnAttr = (HI_MAPI_PIPE_CHN_ATTR_S *)
                     &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stChnAttr;

    if (pstGlobChnAttr->stDestSize.u32Height != pstChnAttr->stDestSize.u32Height
        || pstGlobChnAttr->stDestSize.u32Width != pstChnAttr->stDestSize.u32Width
        || pstGlobChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32SrcFrameRate
        || pstGlobChnAttr->stFrameRate.s32DstFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate
        || pstGlobChnAttr->enCompressMode != pstChnAttr->enCompressMode) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsISPStaticPubAttrChange(HI_HANDLE VcapPipeHdl,
                                                  HI_MAPI_PIPE_ISP_ATTR_S *pstIspAttr)
{
    HI_MAPI_PIPE_ISP_ATTR_S *pstGlobIspAttr;

    pstGlobIspAttr = &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stIspPubAttr;

    if (pstGlobIspAttr->stSize.u32Width != pstIspAttr->stSize.u32Width
        || pstGlobIspAttr->stSize.u32Height != pstIspAttr->stSize.u32Height
        || pstGlobIspAttr->enBayer != pstIspAttr->enBayer) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_BOOL MAPI_VCAP_IsISPDynamicPubAttrChange(HI_HANDLE VcapPipeHdl,
                                                   const HI_MAPI_PIPE_ISP_ATTR_S *pstIspAttr)
{
    HI_MAPI_PIPE_ISP_ATTR_S *pstGlobIspAttr;

    pstGlobIspAttr = &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stIspPubAttr;

    if (pstGlobIspAttr->f32FrameRate != pstIspAttr->f32FrameRate) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

static HI_S32 MAPI_VCAP_ISP_SnsRegisterCallback(HI_HANDLE VcapPipeHdl, const ISP_SNS_OBJ_S *pstSnsObj)
{
    HI_S32 s32Ret;
    ALG_LIB_S stAeLib;
    ALG_LIB_S stAwbLib;
    VI_PIPE ViPipe;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSnsObj);
    CHECK_MAPI_VCAP_NULL_PTR(pstSnsObj->pfnRegisterCallback);

    ViPipe = (VI_PIPE)VcapPipeHdl;

    stAeLib.s32Id = ViPipe;
    stAwbLib.s32Id = ViPipe;

    if (sizeof(HI_AE_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AE_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (sizeof(HI_AWB_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AWB_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* The '\0' must be copied together,so Use sizeof instead of strlen */
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

    s32Ret = pstSnsObj->pfnRegisterCallback(ViPipe, &stAeLib, &stAwbLib);
    CHECK_MAPI_VCAP_RET(s32Ret, "sensor register callback fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ISP_3A_Register(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    VI_PIPE ViPipe;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));

    ViPipe = (VI_PIPE)VcapPipeHdl;

    if (sizeof(HI_AE_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AE_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (sizeof(HI_AWB_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AWB_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    stLib.s32Id = ViPipe;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

    s32Ret = HI_MPI_AE_Register(ViPipe, &stLib);
    CHECK_MAPI_VCAP_RET(s32Ret, "register ae lib fail, VcapPipeHdl[%d]\n", VcapPipeHdl);

    stLib.s32Id = ViPipe;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
    s32Ret = HI_MPI_AWB_Register(ViPipe, &stLib);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "register awb lib failed! s32Ret:%x\n", s32Ret);

        strncpy(stLib.acLibName, HI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
        s32Ret = HI_MPI_AE_UnRegister(ViPipe, &stLib);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "3A Register failed!,ret:%x\n", s32Ret);
            return s32Ret;
        }

        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ISP_3A_UnRegister(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    ALG_LIB_S stLib;
    VI_PIPE ViPipe;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    if (sizeof(HI_AE_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AE_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (sizeof(HI_AWB_LIB_NAME) > ALG_LIB_NAME_SIZE_MAX) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "HI_AWB_LIB_NAME string is longger then %d\n", ALG_LIB_NAME_SIZE_MAX);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    stLib.s32Id = ViPipe;
    strncpy(stLib.acLibName, HI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

    s32Ret = HI_MPI_AE_UnRegister(ViPipe, &stLib);
    CHECK_MAPI_VCAP_RET(s32Ret, "unregister ae lib fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stLib.s32Id = ViPipe;
    strncpy(stLib.acLibName, HI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

    s32Ret = HI_MPI_AWB_UnRegister(ViPipe, &stLib);
    CHECK_MAPI_VCAP_RET(s32Ret, "unregister awb lib fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ISP_BindSns(HI_HANDLE VcapDevHdl, HI_HANDLE VcapPipeHdl,
                                    const ISP_SNS_OBJ_S *pstSnsObj, HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg)
{
    HI_MAPI_SENSOR_COMMBUS_TYPE_E encommBus;
    ISP_SNS_COMMBUS_U stCommBus;
    VI_PIPE ViPipe;
    HI_S32 s32SnsBusId;
    HI_HANDLE MasterISPId;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    MasterISPId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].MasterISPId;
    encommBus = pstSensorCfg->stSensorInputAttr.enSensorCommBusType;

    if (encommBus != HI_SENSOR_COMMBUS_TYPE_I2C && encommBus != HI_SENSOR_COMMBUS_TYPE_SPI) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "wrong sensor commbus type[%d]\n", encommBus);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    /* tips:
      1)Multiple PIPE with the same sensor, only one isp (video) can control the sensor, the other incoming -1,
       otherwise there will be exposure problems.
      2)The ISP witch control the sensor is master ISP,Only the master ISP can init sensor.
    */
    s32SnsBusId = (MasterISPId == VcapPipeHdl) ? g_stCommSnsIntfCfg.aBusId[VcapDevHdl] : HI_INVALID_HANDLE;

    if (encommBus == HI_SENSOR_COMMBUS_TYPE_I2C) {
        stCommBus.s8I2cDev = s32SnsBusId;
    } else if (encommBus == HI_SENSOR_COMMBUS_TYPE_SPI) {
        stCommBus.s8SspDev.bit4SspDev = s32SnsBusId;
        stCommBus.s8SspDev.bit4SspCs = 0;
    }

    pstSnsObj->pfnSetBusInfo(ViPipe, stCommBus);
    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_DEV_AdaptParamFromSensorCfg(HI_HANDLE VcapDevHdl,
                                                    VI_DEV_ATTR_S *pstDevAttr)
{
    HI_S32 s32Ret;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    HI_MAPI_SENSOR_ATTR_S *pstSensorAttr;
    HI_MAPI_MIPI_INTF_S *pstSensorInf;
    mipi_dev_attr_t *pstMipiDevAttr;
    data_type_t stSnsDataType;
    HI_S32 s32CurrSnsSeq;

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Get sensor Config failed,sesnor No:[%d]\n", VcapDevHdl);
        return s32Ret;
    }

    pstSensorAttr = &g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr;

    s32Ret = MAPI_VCAP_GetCurrentSensorModeSeq(&stSensorCfg, pstSensorAttr, &s32CurrSnsSeq);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor[%d] mode seq fail\n", VcapDevHdl);

    pstSensorInf = (HI_MAPI_MIPI_INTF_S *)(stSensorCfg.stSensorMode.pstIntf);

    /* 1.Match the DataRate */
    if (stSensorCfg.data_rate >= MIPI_DATA_RATE_BUTT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "error DataRate %d\n", stSensorCfg.data_rate);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    if ((stSensorCfg.data_rate == MIPI_DATA_RATE_X2) && (VcapDevHdl != VCAP_DEV_0)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "only dev[0] support MIPI_DATA_RATE_X2,now the dev is [%d]\n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    if (stSensorCfg.data_rate == MIPI_DATA_RATE_X1) {
        pstDevAttr->enDataRate = DATA_RATE_X1;
    } else if (stSensorCfg.data_rate == MIPI_DATA_RATE_X2) {
        pstDevAttr->enDataRate = DATA_RATE_X2;
    }

    if (stSensorCfg.input_mode == INPUT_MODE_BT656) {
        pstDevAttr->au32ComponentMask[0] = 0x00FF0000;
        pstDevAttr->au32ComponentMask[1] = 0x0;
        return HI_SUCCESS;
    }

    /* 2.Match the interface mode */
    pstDevAttr->enIntfMode = HAI_MAPI_VCAP_GetIntfMode(stSensorCfg.input_mode);
    stSnsDataType = HAI_MAPI_VCAP_GetSnsDataType(&pstSensorInf[s32CurrSnsSeq], stSensorCfg.input_mode);

    /* 3.match bit width ComponentMask,8bit corresponds to FF (1111 1111), starting from the high position
       so 8bit au32ComponentMask is 0xFF000000 */

    switch (stSnsDataType) {
        case DATA_TYPE_RAW_8BIT:
            pstDevAttr->au32ComponentMask[0] = 0xFF000000;
            pstDevAttr->au32ComponentMask[1] = 0x0;
            break;

        case DATA_TYPE_RAW_10BIT:
            pstDevAttr->au32ComponentMask[0] = 0xFFC00000;
            pstDevAttr->au32ComponentMask[1] = 0x0;
            break;

        case DATA_TYPE_RAW_12BIT:
            pstDevAttr->au32ComponentMask[0] = 0xFFF00000;
            pstDevAttr->au32ComponentMask[1] = 0x0;
            break;

        case DATA_TYPE_RAW_14BIT:
            pstDevAttr->au32ComponentMask[0] = 0xFFFC0000;
            pstDevAttr->au32ComponentMask[1] = 0x0;
            break;

        case DATA_TYPE_RAW_16BIT:
            pstDevAttr->au32ComponentMask[0] = 0xFFFF0000;
            pstDevAttr->au32ComponentMask[1] = 0x0;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sensor input data type(%d) error!\n", stSnsDataType);
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_AdaptPipePixFmtFromSensorCfg(HI_HANDLE VcapDevHdl,
                                                     VI_PIPE_ATTR_S *pstPipeAttr)
{
    HI_S32 s32Ret;
    VI_DEV ViDev;
    HI_S32 s32CurrSnsSeq;
    HI_MAPI_SENSOR_ATTR_S *pstSensorAttr;
    HI_MAPI_MIPI_INTF_S *pstSensorInf;
    data_type_t stSnsDataType;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;

    ViDev = (VI_DEV)VcapDevHdl;

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[ViDev], &stSensorCfg);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Get sensor Config failed,sesnor No:[%d]\n", ViDev);
        return s32Ret;
    }

    pstSensorAttr = &g_stVcapGlobAttr.astGlobDevAttr[ViDev].stSensorAttr;

    s32Ret = MAPI_VCAP_GetCurrentSensorModeSeq(&stSensorCfg, pstSensorAttr, &s32CurrSnsSeq);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor[%d] mode seq fail\n", ViDev);

    if (stSensorCfg.input_mode == INPUT_MODE_BT656) {
        pstPipeAttr->enBitWidth = DATA_BITWIDTH_8;
        pstPipeAttr->enPixFmt = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        pstPipeAttr->bYuvSkip = HI_TRUE;
        return HI_SUCCESS;
    }

    pstSensorInf = (HI_MAPI_MIPI_INTF_S *)(stSensorCfg.stSensorMode.pstIntf);
    stSnsDataType = HAI_MAPI_VCAP_GetSnsDataType(&pstSensorInf[s32CurrSnsSeq], stSensorCfg.input_mode);

    switch (stSnsDataType) {
        case DATA_TYPE_RAW_8BIT:
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_8BPP;
            break;

        case DATA_TYPE_RAW_10BIT:
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_10BPP;
            break;

        case DATA_TYPE_RAW_12BIT:
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
            break;

        case DATA_TYPE_RAW_14BIT:
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_14BPP;
            break;

        case DATA_TYPE_RAW_16BIT:
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_16BPP;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sensor input data type error!\n");
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ISP_SetCommParam(HI_HANDLE VcapPipeHdl, const ISP_PUB_ATTR_S *pstPubAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    HI_HANDLE BindDevHdl;
    HI_S32 i = 0;
    ISP_CTRL_PARAM_S stIspCtrlParam;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, pstPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetPubAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* set isp pubattr in isp running status, call HI_MPI_ISP_QueryInnerStateInfo make sure the set operation is finished. */
    if ((g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene == HI_MAPI_SENSOR_SCENE_STANDBY)
        && (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bIspRun == HI_TRUE)) {
        memset(&stInnerStateInfo, 0x00, sizeof(ISP_INNER_STATE_INFO_S));
        while (i < VCAP_ISP_QUERY_SWITCHSTATE_MAX_TIMES) {
            s32Ret = HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_QueryInnerStateInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSwitchMode == SWITCH_MODE_WDR_LINER_SWITCH) {
                /* wdr switch to liner */
                if (stInnerStateInfo.bWDRSwitchFinish) {
                    g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;
                    break;
                }
            } else if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSwitchMode == SWITCH_MODE_RESOLUTION_SWITCH) {
                /* res switch */
                if (stInnerStateInfo.bResSwitchFinish) {
                    g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;
                    break;
                }
            } else if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSwitchMode == SWITCH_MODE_NO_SENSOR_SWITCH) {
                /* switch not res and wdr */
                break;
            } else {
                return HI_MAPI_VCAP_ENOTSUPPORT;
            }

            usleep(2000); /* delay 2ms */
            i++;
        }

        if (i == VCAP_ISP_QUERY_SWITCHSTATE_MAX_TIMES) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Switch operation time out (%d)us!!\n", (2000 * i));
            return HI_MAPI_VCAP_ENOT_PERM;
        }
    }

    s32Ret = HI_MPI_ISP_GetCtrlParam(ViPipe, &stIspCtrlParam);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetCtrlParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* When the high frame rate function is used, for example, greater than 120 fps,
      the ISP statistics update frequency can be reduced by setting the u32StatIntvl parameter,
      thereby reducing the CPU usage. */

    if (pstPubAttr->f32FrameRate >= 240) {
        stIspCtrlParam.u32StatIntvl = 4;
    } else if (pstPubAttr->f32FrameRate >= 180) {
        stIspCtrlParam.u32StatIntvl = 3;
    } else if (pstPubAttr->f32FrameRate > 120) {
        stIspCtrlParam.u32StatIntvl = 2;
    } else {
        stIspCtrlParam.u32StatIntvl = 1;
    }

    s32Ret = HI_MPI_ISP_SetCtrlParam(ViPipe, &stIspCtrlParam);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetCtrlParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetISPPubAttr(HI_HANDLE VcapDevHdl, HI_HANDLE VcapPipeHdl,
                                      HI_MAPI_COMBO_DEV_ATTR_S *pstSensorCfg)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    ISP_PUB_ATTR_S stPubAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)VcapDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorCfg);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].bSensorInit, "sensor has not been inited",
                         HI_MAPI_VCAP_ENOT_INITED);

    /* 2.get sensor mode info */
    HI_MAPI_SENSOR_ATTR_S *pstSensorAttr;
    HI_MAPI_MIPI_INTF_S *pstSensorInf;
    HI_MAPI_SENSOR_MODE_S *pSensorMode;
    HI_S32 s32CurrSnsSeq;

    pstSensorAttr = &g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stSensorAttr;

    s32Ret = MAPI_VCAP_GetCurrentSensorModeSeq(pstSensorCfg, pstSensorAttr, &s32CurrSnsSeq);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor[%d] mode seq fail,VcapPipeHdl[%d]\n", VcapDevHdl, VcapPipeHdl);

    pstSensorInf = (HI_MAPI_MIPI_INTF_S *)(pstSensorCfg->stSensorMode.pstIntf);
    pSensorMode = (HI_MAPI_SENSOR_MODE_S *)pstSensorCfg->stSensorMode.pSensorMode;

    /* 4.set isp pub attr,BayerFormat parameter If the user does not set or the setting is wrong,
        then the sensor default value is used, otherwise the user-set value is used. */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.enBayer >= BAYER_BUTT) {
        stPubAttr.enBayer = pstSensorCfg->stSensorInputAttr.enBayerFormat;
    } else {
        stPubAttr.enBayer = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.enBayer;
    }

    stPubAttr.u8SnsMode = pSensorMode[s32CurrSnsSeq].u8SnsMode;
    stPubAttr.enWDRMode = g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stSensorAttr.enWdrMode;
    stPubAttr.f32FrameRate = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.f32FrameRate;
    stPubAttr.stWndRect.s32X = 0;
    stPubAttr.stWndRect.s32Y = 0;
    stPubAttr.stWndRect.u32Height = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.stSize.u32Height;
    stPubAttr.stWndRect.u32Width = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.stSize.u32Width;
    stPubAttr.stSnsSize.u32Height = pstSensorInf[s32CurrSnsSeq].stSnsSize.u32Height;
    stPubAttr.stSnsSize.u32Width = pstSensorInf[s32CurrSnsSeq].stSnsSize.u32Width;

    s32Ret = MAPI_VCAP_ISP_SetCommParam(VcapPipeHdl, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "set isp[%d] pub attr fail!\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ResetISP(HI_HANDLE VcapDevHdl, HI_HANDLE VcapPipeHdl)
{
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    HI_S32 s32Ret;
    ISP_SNS_OBJ_S *pstSnsObj;
    ISP_INIT_ATTR_S stISPInitAttr;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit, "sensor has not been inited",
                         HI_MAPI_VCAP_ENOT_INITED);

    /* 1.get sensor config */
    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Get sensor Config failed,sesnor No:[%d], ViPipe:[%d],\n", VcapDevHdl,
                       VcapPipeHdl);
        return s32Ret;
    }

    /* 2.sensor_set_init,reset AWB\AE */
    pstSnsObj = (ISP_SNS_OBJ_S *)stSensorCfg.pSensorObj;
    memcpy(&stISPInitAttr, &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stVcapISPInitAttr.stISPInitAttr,
           sizeof(ISP_INIT_ATTR_S));

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stVcapISPInitAttr.bISPInitAttr) {
        s32Ret = pstSnsObj->pfnSetInit(VcapPipeHdl, &stISPInitAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "pstSnsObj->pfnSetInit failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    /* 3.reset pub attr */
    s32Ret = MAPI_VCAP_SetISPPubAttr(VcapDevHdl, VcapPipeHdl, &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetISPPubAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_VOID *MAPI_VCAP_ISPRunProc(HI_VOID *pPipeHandl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    HI_CHAR aszThreadName[HI_MAPI_PTHREAD_NAME_LEN] = {0};

    ViPipe = *((HI_S32 *)pPipeHandl);

    snprintf(aszThreadName, HI_MAPI_PTHREAD_NAME_LEN, "Hi_pTIspRun%d", ViPipe);
    prctl(PR_SET_NAME, (unsigned long)aszThreadName, 0, 0, 0);

    s32Ret = HI_MPI_ISP_Run(ViPipe);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ISP[%d],firmware abnormally exit", ViPipe);
        return HI_NULL;
    }

    return HI_NULL;
}

static HI_S32 MAPI_VCAP_GetMasterISPId(HI_HANDLE VcapDevHdl)
{
    VI_DEV_BIND_PIPE_S stDevBindPipe;
    VI_PIPE MasterISP = HI_INVALID_HANDLE;
    HI_U32 i = 0;

    stDevBindPipe.u32Num = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum;
    memcpy(stDevBindPipe.PipeId, g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind,
           sizeof(HI_HANDLE) * HI_MAPI_VCAP_MAX_PIPE_NUM);

    /* 1.check dev bind */
    if (stDevBindPipe.u32Num < 1 || stDevBindPipe.u32Num > HI_MAPI_VCAP_MAX_PIPE_NUM) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "vcap dev[%u] bind is wrong\n", VcapDevHdl);
        return HI_INVALID_HANDLE;
    }

    /* 2.get master isp,
       1)In Scenes of single PIPE, the ID of the master ISP is the ID of this PIPE.
       2)In Scenes of double PIPE, the ID of the video PIPE is the ID of the master ISP.
     */
    if (stDevBindPipe.u32Num == 1) {
        MasterISP = (HI_HANDLE)stDevBindPipe.PipeId[0];
        return MasterISP;
    } else if (stDevBindPipe.u32Num > 1 && stDevBindPipe.u32Num <= HI_MAPI_VCAP_MAX_PIPE_NUM) {
        for (i = 0; i < stDevBindPipe.u32Num; i++) {
            MasterISP = (HI_HANDLE)stDevBindPipe.PipeId[i];

            if (g_stVcapGlobAttr.astGlobPipeAttr[MasterISP].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO) {
                return MasterISP;
            }
        }
    }

    MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "vcap dev[%d] bind is wrong\n", VcapDevHdl);

    return HI_INVALID_HANDLE;
}

static HI_S32 MAPI_VCAP_DynamicSetPipeAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_PIPE_ATTR_S stPipeAttr;
    HI_MAPI_VCAP_PIPE_ATTR_S *pstPipeAttr = HI_NULL;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    pstPipeAttr = &pstVCapAttr->astVcapPipeAttr[VcapPipeHdl];

    CHECK_MAPI_VCAP_NULL_PTR(pstPipeAttr);
    CHECK_MAPI_VCAP_TRUE(!MAPI_VCAP_IsPipeStaticAttrChange(VcapPipeHdl, pstPipeAttr),
                         "don't support modify PIPE's static Attributes in the running status", HI_MAPI_VCAP_ENOT_PERM);

    if (!MAPI_VCAP_IsPipeDynamicAttrChange(VcapPipeHdl, pstPipeAttr)) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "PIPE[%d]'s Dynamic Attrs is no change!\n", VcapPipeHdl);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stPipeAttr.enCompressMode = pstPipeAttr->enCompressMode;
    stPipeAttr.stFrameRate.s32SrcFrameRate = pstPipeAttr->stFrameRate.s32SrcFrameRate;
    stPipeAttr.stFrameRate.s32DstFrameRate = pstPipeAttr->stFrameRate.s32DstFrameRate;

    s32Ret = HI_MPI_VI_SetPipeAttr(ViPipe, &stPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* update dynamic pipe attr */
    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stPipeAttr.enCompressMode = pstPipeAttr->enCompressMode;
    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stPipeAttr.stFrameRateCtrl.s32SrcFrameRate =
        pstPipeAttr->stFrameRate.s32SrcFrameRate;
    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stPipeAttr.stFrameRateCtrl.s32DstFrameRate =
        pstPipeAttr->stFrameRate.s32DstFrameRate;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_DynamicSetISPAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_PUB_ATTR_S stPubAttr;
    HI_MAPI_PIPE_ISP_ATTR_S *psIspAttr = HI_NULL;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    psIspAttr = &pstVCapAttr->astVcapPipeAttr[VcapPipeHdl].stIspPubAttr;

    CHECK_MAPI_VCAP_NULL_PTR(psIspAttr);
    CHECK_MAPI_VCAP_TRUE(!MAPI_VCAP_IsISPStaticPubAttrChange(VcapPipeHdl, psIspAttr),
                         "don't support modify ISP pub static Attributes in the dev running status",
                         HI_MAPI_VCAP_ENOT_PERM);

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bIspRun == HI_FALSE) {
        g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stIspPubAttr.f32FrameRate = psIspAttr->f32FrameRate;
        return HI_SUCCESS;
    }

    if (!MAPI_VCAP_IsISPDynamicPubAttrChange(VcapPipeHdl, psIspAttr)) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "ISP's Dynamic Attrs is no change!\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPubAttr fail,VcapPipeHdl[%u]\n", VcapPipeHdl);

    stPubAttr.f32FrameRate = psIspAttr->f32FrameRate;
    s32Ret = MAPI_VCAP_ISP_SetCommParam(VcapPipeHdl, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ISP_SetCommParam fail,VcapPipeHdl[%u]\n", VcapPipeHdl);

    /* update dynamic isp pub attr */
    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stIspPubAttr.f32FrameRate = psIspAttr->f32FrameRate;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_DynamicSetChnAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                          HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_CHN_ATTR_S stChnAttr;
    HI_MAPI_PIPE_CHN_ATTR_S *pstChnAttr = HI_NULL;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    pstChnAttr = &pstVCapAttr->astVcapPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl];

    CHECK_MAPI_VCAP_NULL_PTR(pstChnAttr);

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].bChnEnable == HI_FALSE) {
        memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stChnAttr, pstChnAttr,
               sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
        return HI_SUCCESS;
    }

    CHECK_MAPI_VCAP_TRUE(!MAPI_VCAP_IsChnStaticAttrChange(VcapPipeHdl, PipeChnHdl, pstChnAttr),
                         "don't support modify chn's static Attributes in the running status", HI_MAPI_VCAP_ENOT_PERM);

    if (!MAPI_VCAP_IsChnDynamicAttrChange(VcapPipeHdl, PipeChnHdl, pstChnAttr)) {
        MAPI_INFO_TRACE(HI_MAPI_MOD_VCAP, "CHN's Dynamic Attrs is no change!\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stChnAttr.enCompressMode = pstChnAttr->enCompressMode;
    stChnAttr.stSize.u32Width = pstChnAttr->stDestSize.u32Width;
    stChnAttr.stSize.u32Height = pstChnAttr->stDestSize.u32Height;
    stChnAttr.stFrameRate.s32SrcFrameRate = pstChnAttr->stFrameRate.s32SrcFrameRate;
    stChnAttr.stFrameRate.s32DstFrameRate = pstChnAttr->stFrameRate.s32DstFrameRate;

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* update dynamic chn attr */
    memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stChnAttr, pstChnAttr,
           sizeof(HI_MAPI_PIPE_CHN_ATTR_S));

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_StaticSetAttr(HI_HANDLE VcapDevHdl, const HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret;
    VI_DEV ViDev;
    VI_PIPE ViPipe;
    HI_U32 i, j;
    HI_S32 s32CurrSnsSeq = 0;
    VI_DEV_ATTR_S stDevAttr;
    VCAP_INNER_CONFIG_ATTR_S stVcapCfg;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    HI_MAPI_MIPI_INTF_S *pstMipiIntf = HI_NULL;
    HI_MAPI_SENSOR_ATTR_S *pstSensorAttr = HI_NULL;

    ViDev = (VI_DEV)VcapDevHdl;

    /************************set vi dev*************************/

    pstSensorAttr = &g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr;
    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor[%d] config fail\n", VcapDevHdl);

    s32Ret = MAPI_VCAP_GetCurrentSensorModeSeq(&stSensorCfg, pstSensorAttr, &s32CurrSnsSeq);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor[%d] mode seq fail\n", VcapDevHdl);

    pstMipiIntf = (HI_MAPI_MIPI_INTF_S *)stSensorCfg.stSensorMode.pstIntf;

    /* load dev default config */
    MAPI_VCAP_LoadParam(&stVcapCfg, g_as32SensorType);
    memcpy(&stDevAttr, &stVcapCfg.astDevAttr[ViDev], sizeof(VI_DEV_ATTR_S));
    stDevAttr.stSize.u32Width = pstMipiIntf[s32CurrSnsSeq].img_rect.width;
    stDevAttr.stSize.u32Height = pstMipiIntf[s32CurrSnsSeq].img_rect.height;
    stDevAttr.stWDRAttr.enWDRMode = pstVCapAttr->stVcapDevAttr.enWdrMode;
    stDevAttr.stWDRAttr.u32CacheLine = pstVCapAttr->stVcapDevAttr.u32CacheLine;  // check at SDK
    stDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Width = pstVCapAttr->stVcapDevAttr.stBasSize.u32Width;
    stDevAttr.stBasAttr.stSacleAttr.stBasSize.u32Height =
        pstVCapAttr->stVcapDevAttr.stBasSize.u32Height;  // check at SDK

    /* load dev config from sensor config */
    s32Ret = MAPI_VCAP_DEV_AdaptParamFromSensorCfg(VcapDevHdl, &stDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "get Datarate and inputmode fail,VcapDevHdl[%d]\n", VcapDevHdl);

    /* set vi dev attr */
    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "HI_MPI_VI_SetDevAttr, set vcap[%d] attr fail\n", VcapDevHdl);

    /***************save vcap attr and flag**********************************/
    if (pstVCapAttr->u32PipeBindNum > HI_MAPI_VCAP_MAX_PIPE_NUM) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PipeBindNum[%u] is error\n", pstVCapAttr->u32PipeBindNum);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum = pstVCapAttr->u32PipeBindNum;
    memcpy(g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind, pstVCapAttr->aPipeIdBind,
           sizeof(HI_HANDLE) * HI_MAPI_VCAP_MAX_PIPE_NUM);
    memcpy(&g_stVcapGlobAttr.astGlobDevAttr[ViDev].stDevAttr, &pstVCapAttr->stVcapDevAttr,
           sizeof(HI_MAPI_VCAP_DEV_ATTR_S));

    for (i = 0; i < pstVCapAttr->u32PipeBindNum; i++) {
        ViPipe = (VI_PIPE)pstVCapAttr->aPipeIdBind[i];

        /* 1. save pipe user attr and flag */
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enCompressMode =
            pstVCapAttr->astVcapPipeAttr[ViPipe].enCompressMode;
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType =
            pstVCapAttr->astVcapPipeAttr[ViPipe].enPipeType;
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.bIspBypass =
            pstVCapAttr->astVcapPipeAttr[ViPipe].bIspBypass;
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeBypassMode =
            pstVCapAttr->astVcapPipeAttr[ViPipe].enPipeBypassMode;
#ifndef __HI3559AV100__
        memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.stFrameIntAttr,
               &pstVCapAttr->astVcapPipeAttr[ViPipe].stFrameIntAttr, sizeof(FRAME_INTERRUPT_ATTR_S));
#endif
        memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.stFrameRateCtrl,
               &pstVCapAttr->astVcapPipeAttr[ViPipe].stFrameRate, sizeof(FRAME_RATE_CTRL_S));
        memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr,
               &pstVCapAttr->astVcapPipeAttr[ViPipe].stIspPubAttr, sizeof(HI_MAPI_PIPE_ISP_ATTR_S));

        /* 2. save chn user attr and flag */
        for (j = 0; j < HI_MAPI_PIPE_MAX_CHN_NUM; j++) {
            memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[j].stChnAttr,
                   &pstVCapAttr->astVcapPipeAttr[ViPipe].astPipeChnAttr[j], sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
        }
    }

    g_stVcapGlobAttr.astGlobDevAttr[ViDev].MasterISPId = MAPI_VCAP_GetMasterISPId(ViDev);

    return HI_SUCCESS;
}
static HI_S32 MAPI_VCAP_SwitchTypeToISP(HI_MAPI_VCAP_OP_TYPE_E enSrcType, ISP_OP_TYPE_E *penDstType)
{
    CHECK_MAPI_VCAP_NULL_PTR(penDstType);

    switch (enSrcType) {
        case HI_MAPI_VCAP_OP_TYPE_AUTO: /* 0 */
            *penDstType = OP_TYPE_AUTO;
            break;

        case HI_MAPI_VCAP_OP_TYPE_MANUAL: /* 1 */
            *penDstType = OP_TYPE_MANUAL;
            break;

        case HI_MAPI_VCAP_OP_TYPE_DISABLE: /* 2 */
        case HI_MAPI_VCAP_OP_TYPE_BUTT:    /* 3 */
            *penDstType = OP_TYPE_BUTT;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Unsupport MAPI VCAP operate Type\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SwitchTypeToMapi(ISP_OP_TYPE_E enSrcType, HI_MAPI_VCAP_OP_TYPE_E *penDstType)
{
    CHECK_MAPI_VCAP_NULL_PTR(penDstType);

    switch (enSrcType) {
        case OP_TYPE_AUTO: /* 0 */
            *penDstType = HI_MAPI_VCAP_OP_TYPE_AUTO;
            break;

        case OP_TYPE_MANUAL: /* 1 */
            *penDstType = HI_MAPI_VCAP_OP_TYPE_MANUAL;
            break;

        case OP_TYPE_BUTT: /* 2 */
            *penDstType = HI_MAPI_VCAP_OP_TYPE_BUTT;
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Unsupport ISP op Type\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetExposure(HI_HANDLE VcapPipeHdl, const HI_MAPI_VCAP_EXPOSURE_ATTR_S *pstExposure)
{
    HI_S32 s32Ret;
    ISP_EXPOSURE_ATTR_S stExposure;
    VI_PIPE MasterISP;
    VI_DEV BindDevHdl;
    VI_PIPE ViPipe;
    ISP_OP_TYPE_E enDstISPType = OP_TYPE_BUTT;

    CHECK_MAPI_VCAP_NULL_PTR(pstExposure);

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;
    MasterISP = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].MasterISPId;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    s32Ret = HI_MPI_ISP_GetExposureAttr(MasterISP, &stExposure);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetExposureAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    switch (pstExposure->enOpType) {
        case HI_MAPI_VCAP_OP_TYPE_DISABLE: {
            stExposure.bByPass = HI_TRUE;
            break;
        }

        case HI_MAPI_VCAP_OP_TYPE_AUTO: {
            if (pstExposure->stAExposureMode.stSysGainRange.u32Min < HI_MAPI_VCAP_SYSGAIN_MIN
                || pstExposure->stAExposureMode.stSysGainRange.u32Max < HI_MAPI_VCAP_SYSGAIN_MIN) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SysGainRange Min or Max less than %d\n", HI_MAPI_VCAP_SYSGAIN_MIN);
                return HI_MAPI_VCAP_EILLEGAL_PARA;
            }

            if (pstExposure->stAExposureMode.stSysGainRange.u32Max <
                pstExposure->stAExposureMode.stSysGainRange.u32Min) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "SysGainRange Max:%d can not less than Min:%d\n",
                               pstExposure->stAExposureMode.stSysGainRange.u32Max,
                               pstExposure->stAExposureMode.stSysGainRange.u32Min);
                return HI_MAPI_VCAP_EILLEGAL_PARA;
            }

            stExposure.bByPass = HI_FALSE;
            stExposure.enOpType = OP_TYPE_AUTO;
            stExposure.stAuto.u16EVBias = pstExposure->stAExposureMode.u32EVBias;
            stExposure.stAuto.stSysGainRange.u32Min = pstExposure->stAExposureMode.stSysGainRange.u32Min;
            stExposure.stAuto.stSysGainRange.u32Max = pstExposure->stAExposureMode.stSysGainRange.u32Max;
            break;
        }

        case HI_MAPI_VCAP_OP_TYPE_MANUAL: {
            ISP_PUB_ATTR_S stIspPubAttr;

            /* get the realtime frame rate */
            s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stIspPubAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPubAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            /* check para, check exposure time of AE */
            if (pstExposure->stMExposureMode.u32ExposureTime > (USEC_UNIT / stIspPubAttr.f32FrameRate)) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Manual exposure time can not greater than %f\n",
                               (USEC_UNIT / stIspPubAttr.f32FrameRate));
                return HI_MAPI_VCAP_EILLEGAL_PARA;
            }

            stExposure.bByPass = HI_FALSE;
            stExposure.enOpType = OP_TYPE_MANUAL;
            s32Ret = MAPI_VCAP_SwitchTypeToISP(pstExposure->stMExposureMode.enExpTimeOpType, &enDstISPType);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToISP fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            stExposure.stManual.enExpTimeOpType = enDstISPType;

            s32Ret = MAPI_VCAP_SwitchTypeToISP(pstExposure->stMExposureMode.enAGainOpType, &enDstISPType);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToISP fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            stExposure.stManual.enAGainOpType = enDstISPType;

            s32Ret = MAPI_VCAP_SwitchTypeToISP(pstExposure->stMExposureMode.enDGainOpType, &enDstISPType);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToISP fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            stExposure.stManual.enDGainOpType = enDstISPType;

            s32Ret = MAPI_VCAP_SwitchTypeToISP(pstExposure->stMExposureMode.enISPDGainOpType, &enDstISPType);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToISP fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            stExposure.stManual.enISPDGainOpType = enDstISPType;

            stExposure.stManual.u32ExpTime = pstExposure->stMExposureMode.u32ExposureTime;
            stExposure.stManual.u32AGain = pstExposure->stMExposureMode.u32AGain;
            stExposure.stManual.u32DGain = pstExposure->stMExposureMode.u32DGain;
            stExposure.stManual.u32ISPDGain = pstExposure->stMExposureMode.u32ISPDGain;
            break;
        }

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "not support exposure op type\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
        }
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(MasterISP, &stExposure);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetExposureAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetExposure(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_EXPOSURE_ATTR_S *pstExposure)
{
    HI_S32 s32Ret;
    ISP_EXPOSURE_ATTR_S stExposure;
    VI_PIPE MasterISP;
    VI_DEV BindDevHdl;
    HI_MAPI_VCAP_OP_TYPE_E enDstType = HI_MAPI_VCAP_OP_TYPE_BUTT;

    CHECK_MAPI_VCAP_NULL_PTR(pstExposure);

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;
    MasterISP = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].MasterISPId;

    memset(&stExposure, 0, sizeof(ISP_EXPOSURE_ATTR_S));

    s32Ret = HI_MPI_ISP_GetExposureAttr(MasterISP, &stExposure);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetExposureAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    if (stExposure.bByPass == HI_TRUE) {
        pstExposure->enOpType = HI_MAPI_VCAP_OP_TYPE_DISABLE;
    } else {
        if (stExposure.enOpType == OP_TYPE_AUTO) {
            pstExposure->enOpType = HI_MAPI_VCAP_OP_TYPE_AUTO;
        } else if (stExposure.enOpType == OP_TYPE_MANUAL) {
            pstExposure->enOpType = HI_MAPI_VCAP_OP_TYPE_MANUAL;
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "not support exposure op type\n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }
    }

    s32Ret = MAPI_VCAP_SwitchTypeToMapi(stExposure.stManual.enExpTimeOpType, &enDstType);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToMapi fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstExposure->stMExposureMode.enExpTimeOpType = enDstType;
    s32Ret = MAPI_VCAP_SwitchTypeToMapi(stExposure.stManual.enAGainOpType, &enDstType);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToMapi fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstExposure->stMExposureMode.enAGainOpType = enDstType;
    s32Ret = MAPI_VCAP_SwitchTypeToMapi(stExposure.stManual.enDGainOpType, &enDstType);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToMapi fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstExposure->stMExposureMode.enDGainOpType = enDstType;
    s32Ret = MAPI_VCAP_SwitchTypeToMapi(stExposure.stManual.enISPDGainOpType, &enDstType);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SwitchTypeToMapi fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    pstExposure->stMExposureMode.enISPDGainOpType = enDstType;

    pstExposure->stMExposureMode.u32ExposureTime = stExposure.stManual.u32ExpTime;
    pstExposure->stMExposureMode.u32AGain = stExposure.stManual.u32AGain;
    pstExposure->stMExposureMode.u32DGain = stExposure.stManual.u32DGain;
    pstExposure->stMExposureMode.u32ISPDGain = stExposure.stManual.u32ISPDGain;

    pstExposure->stAExposureMode.u32EVBias = stExposure.stAuto.u16EVBias;
    pstExposure->stAExposureMode.stSysGainRange.u32Min = stExposure.stAuto.stSysGainRange.u32Min;
    pstExposure->stAExposureMode.stSysGainRange.u32Max = stExposure.stAuto.stSysGainRange.u32Max;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetWB(HI_HANDLE VcapPipeHdl, const HI_MAPI_VCAP_WB_ATTR_S *pstWBAttr)
{
    HI_S32 s32Ret;
    ISP_WB_ATTR_S stWbAttr;
    VI_PIPE ViPipe;

    CHECK_MAPI_VCAP_NULL_PTR(pstWBAttr);

    ViPipe = (VI_PIPE)VcapPipeHdl;

    memset(&stWbAttr, 0, sizeof(ISP_WB_ATTR_S));

    s32Ret = HI_MPI_ISP_GetWBAttr(ViPipe, &stWbAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetWBAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    switch (pstWBAttr->enOpType) {
        case HI_MAPI_VCAP_OP_TYPE_DISABLE: {
            stWbAttr.bByPass = HI_TRUE;
            break;
        }

        case HI_MAPI_VCAP_OP_TYPE_AUTO: {
            stWbAttr.bByPass = HI_FALSE;
            stWbAttr.enOpType = OP_TYPE_AUTO;
            break;
        }

        case HI_MAPI_VCAP_OP_TYPE_MANUAL: {
            HI_U16 au16AwbGain[4];

            /* check para */
            if (pstWBAttr->stWBMode.u32ColorTemp < HI_MAPI_VCAP_MIN_COLORTEMP ||
                pstWBAttr->stWBMode.u32ColorTemp > HI_MAPI_VCAP_MAX_COLORTEMP) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "WBColorValue:%d,[%d,%d]\n", pstWBAttr->stWBMode.u32ColorTemp,
                               HI_MAPI_VCAP_MIN_COLORTEMP, HI_MAPI_VCAP_MAX_COLORTEMP);
                return HI_MAPI_VCAP_EILLEGAL_PARA;
            }

            s32Ret = HI_MPI_ISP_CalGainByTemp(ViPipe, &stWbAttr, pstWBAttr->stWBMode.u32ColorTemp, 0, au16AwbGain);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetWBAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            stWbAttr.stManual.u16Rgain = au16AwbGain[0];
            stWbAttr.stManual.u16Grgain = au16AwbGain[1];
            stWbAttr.stManual.u16Gbgain = au16AwbGain[2];
            stWbAttr.stManual.u16Bgain = au16AwbGain[3];
            stWbAttr.bByPass = HI_FALSE;
            stWbAttr.enOpType = OP_TYPE_MANUAL;
            break;
        }

        default:
        {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "not support wb op type");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }
    }

    s32Ret = HI_MPI_ISP_SetWBAttr(ViPipe, &stWbAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetWBAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.enOpType = pstWBAttr->enOpType;

    if (pstWBAttr->enOpType == HI_MAPI_VCAP_OP_TYPE_MANUAL) {
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.stWBMode.u32ColorTemp =
            pstWBAttr->stWBMode.u32ColorTemp;
    } else if (pstWBAttr->enOpType == HI_MAPI_VCAP_OP_TYPE_AUTO) {
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.stWBMode.u32ColorTemp = 5000;
    } else {
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.stWBMode.u32ColorTemp = 0xFFFF;  // The value is invalid
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_ReleaseOneRawFrame(HI_HANDLE VcapPipeHdl, const HI_MAPI_FRAME_DATA_S *pstFrameData,
                                    HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    VI_PIPE ViPipe;
    VIDEO_FRAME_INFO_S stVideoFrame;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);

    stVideoFrame.stVFrame.enPixelFormat = pstFrameData->enPixelFormat;
    stVideoFrame.stVFrame.u32Height = pstFrameData->u32Height;
    stVideoFrame.stVFrame.u32Width = pstFrameData->u32Width;
    stVideoFrame.u32PoolId = pstFrameData->u32PoolId;
    stVideoFrame.stVFrame.u64PTS = pstFrameData->u64pts;

    for (i = 0; i < 3; i++) {
        stVideoFrame.stVFrame.u64PhyAddr[i] = pstFrameData->u64PhyAddr[i];
        stVideoFrame.stVFrame.u32Stride[i] = pstFrameData->u32Stride[i];
        stVideoFrame.stVFrame.u64VirAddr[i] = pstFrameData->u64VirAddr[i];
    }

    if (bBNRraw) {
#ifndef __HI3559AV100__
        s32Ret = HI_MPI_VI_ReleasePipeBNRRaw(ViPipe, &stVideoFrame);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_ReleasePipeBNRRaw failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#else
        s32Ret = HI_MPI_SNAP_ReleaseBNRRaw(ViPipe, &stVideoFrame);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_ReleaseBNRRaw failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#endif
    } else {
        s32Ret = HI_MPI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_ReleasePipeFrame failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_GetOneRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
                                HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    VI_PIPE ViPipe;
    VIDEO_FRAME_INFO_S stVideoFrame;
    VCAP_DUMP_RAWDATA_INFO_S *pstDumpRawInfo = &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stDumpRawInfo;
    /* 10000 is our estimate excute max time */
    HI_U32 u32GetFrameTimeout = pstDumpRawInfo->u32ExposureTime + 10000;
    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);

    if (bBNRraw) {
#ifndef __HI3559AV100__
        /* the timeout value need */
        s32Ret = HI_MPI_VI_GetPipeBNRRaw(ViPipe, &stVideoFrame, u32GetFrameTimeout);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeBNRRaw failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#else
        s32Ret = HI_MPI_SNAP_GetBNRRaw(ViPipe, &stVideoFrame, u32GetFrameTimeout);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_GetBNRRaw failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#endif
    } else {
        s32Ret = HI_MPI_VI_GetPipeFrame(ViPipe, &stVideoFrame, u32GetFrameTimeout);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeFrame failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    memcpy(&pstFrameData->stVideoSupplement, &stVideoFrame.stVFrame.stSupplement, sizeof(VIDEO_SUPPLEMENT_S));
    pstFrameData->enCompressMode = stVideoFrame.stVFrame.enCompressMode;
    pstFrameData->enFrameDataType = HI_FRAME_DATA_TYPE_RAW;
    pstFrameData->enPixelFormat = stVideoFrame.stVFrame.enPixelFormat;
    pstFrameData->u32Height = stVideoFrame.stVFrame.u32Height;
    pstFrameData->u32Width = stVideoFrame.stVFrame.u32Width;
    pstFrameData->u32PoolId = stVideoFrame.u32PoolId;
    pstFrameData->u64pts = stVideoFrame.stVFrame.u64PTS;

    for (i = 0; i < 3; i++) {
        pstFrameData->u64PhyAddr[i] = stVideoFrame.stVFrame.u64PhyAddr[i];
        pstFrameData->u32Stride[i] = stVideoFrame.stVFrame.u32Stride[i];
        pstFrameData->u64VirAddr[i] = stVideoFrame.stVFrame.u64VirAddr[i];
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_ReleaseWdrRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
                                    HI_S32 ps32FrameCnt, HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    HI_U32 u32DumpPipeCnt;
    HI_HANDLE VcapDevHdl;
    HI_HANDLE PipeId;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);

    VcapDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;
    u32DumpPipeCnt = ps32FrameCnt;

    for (i = 0; i < u32DumpPipeCnt; i++) {
        PipeId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];
        s32Ret = MAPI_VCAP_ReleaseOneRawFrame(PipeId, &pstFrameData[i], bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ReleaseOneRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_GetWdrRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
                                HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i, j;
    HI_U32 u32DumpPipeCnt;
    HI_U64 u64TimeDiff;
    HI_HANDLE VcapDevHdl;
    HI_HANDLE PipeId;
    HI_HANDLE MasterPipeId;
    HI_MAPI_FRAME_DATA_S astOneGroupWdrRaw[HI_VCAP_RAW_EACHGROUP_MAX_NUM];
    HI_U32 u32WdrRawVcNum[HI_VCAP_RAW_EACHGROUP_MAX_NUM];
    HI_MAPI_FRAME_DATA_S stTempWdrRaw;
    HI_U32 u32ReapeteCnt = 0;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);

    VcapDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;
    u32DumpPipeCnt = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum;
    MasterPipeId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[0];
    u64TimeDiff = (HI_U64)((1000 * 1000) / g_stVcapGlobAttr.astGlobPipeAttr[MasterPipeId].stIspPubAttr.f32FrameRate);

    /* 1.get a group wdr raw,and save VC number */
    for (i = 0; i < u32DumpPipeCnt; i++) {
        PipeId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];
        s32Ret = HI_MPI_VI_GetPipeVCNumber(PipeId, &u32WdrRawVcNum[i]);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeVCNumber fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

        s32Ret = MAPI_VCAP_GetOneRawFrame(PipeId, &astOneGroupWdrRaw[i], bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetOneRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

        if (i >= 1) {
            /* Determine if it is a WDR paired frame */
            if ((HI_U32)abs(astOneGroupWdrRaw[i].u64pts - astOneGroupWdrRaw[i - 1].u64pts) > u64TimeDiff) {
                MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP,
                                "not a group wdr frame,release all and restart to get a new group wdr frame\n");

                s32Ret = MAPI_VCAP_ReleaseWdrRawFrame(VcapPipeHdl, astOneGroupWdrRaw, i, bBNRraw);
                CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ReleaseWdrRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

                /* Prevent the occurrence of die loops caused by never getting data */
                if (++u32ReapeteCnt > WDR_DUMP_REPEATE_MAX_CNT) {
                    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Still unable to get a group wdr raw after trying %d times\n",
                                   u32ReapeteCnt);
                    return HI_MAPI_VCAP_EOPERATE_FAIL;
                }

                i = 0;
                continue;
            }
        }
    }

    /* 2.Sort frames by exposure ,The larger the VC number is the short exposure frame,
        In order to adapt to PQ the short exposure frame must save first */
    for (i = 0; i < u32DumpPipeCnt; i++) {
        for (j = i + 1; j < u32DumpPipeCnt; j++) {
            if (u32WdrRawVcNum[i] < u32WdrRawVcNum[j]) {
                memcpy(&stTempWdrRaw, &astOneGroupWdrRaw[i], sizeof(HI_MAPI_FRAME_DATA_S));
                memcpy(&astOneGroupWdrRaw[i], &astOneGroupWdrRaw[j], sizeof(HI_MAPI_FRAME_DATA_S));
                memcpy(&astOneGroupWdrRaw[j], &stTempWdrRaw, sizeof(HI_MAPI_FRAME_DATA_S));
            }
        }
    }

    memcpy(pstFrameData, astOneGroupWdrRaw, sizeof(HI_MAPI_FRAME_DATA_S) * u32DumpPipeCnt);

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_ReleaseRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
                                 HI_S32 s32DataNum, HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    if (bBNRraw) {
        CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bSnapPipeEnable,
                             "Only snap supports dump BNR raw", HI_MAPI_VCAP_ENOTSUPPORT);
    }

    VcapDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    if (s32DataNum <= 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Release Raw data, error dump number.\n");
        return HI_FAILURE;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stDevAttr.enWdrMode == WDR_MODE_NONE) {
        s32Ret = MAPI_VCAP_ReleaseOneRawFrame(VcapPipeHdl, pstFrameData, bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ReleaseOneRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    } else {
        s32Ret = MAPI_VCAP_ReleaseWdrRawFrame(VcapPipeHdl, pstFrameData, s32DataNum, bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ReleaseWdrRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    return HI_SUCCESS;
}

HI_S32 MAPI_VCAP_GetRawFrame(HI_HANDLE VcapPipeHdl, HI_MAPI_FRAME_DATA_S *pstFrameData,
                             HI_S32 *ps32FrameCnt, HI_BOOL bBNRraw)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstFrameData);
    CHECK_MAPI_VCAP_NULL_PTR(ps32FrameCnt);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    if (bBNRraw) {
        CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bSnapPipeEnable,
                             "Only snap supports dump BNR raw", HI_MAPI_VCAP_ENOTSUPPORT);
    }

    VcapDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stDevAttr.enWdrMode == WDR_MODE_NONE) {
        s32Ret = MAPI_VCAP_GetOneRawFrame(VcapPipeHdl, pstFrameData, bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetOneRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
        *ps32FrameCnt = 1;
    } else {
        s32Ret = MAPI_VCAP_GetWdrRawFrame(VcapPipeHdl, pstFrameData, bBNRraw);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetWdrRawFrame fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
        *ps32FrameCnt = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_DEV ViDev;
    VI_PIPE_ATTR_S stPipeAttr;

    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    memset(&stPipeAttr, 0, sizeof(VI_PIPE_ATTR_S));

    s32Ret = HI_MPI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* judge whether to set pipe pixformat */
    if (pstDumpAttr->stDumpAttr.bEnable || pstDumpAttr->stDumpBNRAttr.bEnable) {
        if (pstDumpAttr->enPixFmt == PIXEL_FORMAT_RGB_BAYER_16BPP
            && stPipeAttr.enCompressMode != COMPRESS_MODE_NONE) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP,
                           "when the enPixFmt is BAYER 16BPP,the Compression mode(%d) can only be COMPRESS_MODE_NONE\n",
                           stPipeAttr.enCompressMode);
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }

        if (stPipeAttr.enPixFmt != pstDumpAttr->enPixFmt
            && pstDumpAttr->enPixFmt != PIXEL_FORMAT_BUTT) {
            stPipeAttr.enPixFmt = pstDumpAttr->enPixFmt;

            s32Ret = HI_MPI_VI_SetPipeAttr(ViPipe, &stPipeAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
        }
    } else {
        s32Ret = MAPI_VCAP_AdaptPipePixFmtFromSensorCfg(ViDev, &stPipeAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "get PIPE pixfmt fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
        s32Ret = HI_MPI_VI_SetPipeAttr(ViPipe, &stPipeAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    if (pstDumpAttr->stDumpAttr.bEnable) {
        s32Ret = HI_MPI_VI_SetPipeDumpAttr(ViPipe, &pstDumpAttr->stDumpAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeDumpAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    if (pstDumpAttr->stDumpBNRAttr.bEnable) {
#ifndef __HI3559AV100__
        s32Ret = HI_MPI_VI_SetPipeBNRRawDumpAttr(ViPipe, &pstDumpAttr->stDumpBNRAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeBNRRawDumpAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#else
        s32Ret = HI_MPI_SNAP_SetBNRRawDumpAttr(ViPipe, &pstDumpAttr->stDumpBNRAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_SetBNRRawDumpAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
#endif
    }

    memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stDumpRawInfo.stVcapDumpAttr, pstDumpAttr,
           sizeof(HI_MAPI_DUMP_ATTR_S));

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetWdrDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
    HI_HANDLE PipeId;
    HI_U32 u32DumpPipeCnt;
    HI_HANDLE VcapDevHdl;

    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    VcapDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;
    u32DumpPipeCnt = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum;

    for (i = 0; i < u32DumpPipeCnt; i++) {
        PipeId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];

        s32Ret = MAPI_VCAP_SetDumpRawAttr(PipeId, pstDumpAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetDumpRawAttr failed,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, const VI_LDC_ATTR_S *pstLdcAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_CHN_ATTR_S stChnAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    // para check
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable,
                         "chn has not been start", HI_MAPI_VCAP_ENOT_PERM);
    CHECK_MAPI_VCAP_MIN_VAL("s32CenterXOffset", pstLdcAttr->stAttr.s32CenterXOffset,
                            HI_MAPI_VCAP_MIN_LDC_CENTERX_OFFSET);
    CHECK_MAPI_VCAP_MAX_VAL("s32CenterXOffset", pstLdcAttr->stAttr.s32CenterXOffset,
                            HI_MAPI_VCAP_MAX_LDC_CENTERX_OFFSET);
    CHECK_MAPI_VCAP_MIN_VAL("s32CenterYOffset", pstLdcAttr->stAttr.s32CenterYOffset,
                            HI_MAPI_VCAP_MIN_LDC_CENTERY_OFFSET);
    CHECK_MAPI_VCAP_MAX_VAL("s32CenterYOffset", pstLdcAttr->stAttr.s32CenterYOffset,
                            HI_MAPI_VCAP_MAX_LDC_CENTERY_OFFSET);
    CHECK_MAPI_VCAP_MIN_VAL("s32XYRatio", pstLdcAttr->stAttr.s32XYRatio, HI_MAPI_VCAP_MIN_LDC_RATIO_XY);
    CHECK_MAPI_VCAP_MAX_VAL("s32XYRatio", pstLdcAttr->stAttr.s32XYRatio, HI_MAPI_VCAP_MAX_LDC_RATIO_XY);
    CHECK_MAPI_VCAP_MIN_VAL("s32XRatio", pstLdcAttr->stAttr.s32XRatio, HI_MAPI_VCAP_MIN_LDC_RATIO_X);
    CHECK_MAPI_VCAP_MAX_VAL("s32XRatio", pstLdcAttr->stAttr.s32XRatio, HI_MAPI_VCAP_MAX_LDC_RATIO_X);
    CHECK_MAPI_VCAP_MIN_VAL("s32YRatio", pstLdcAttr->stAttr.s32YRatio, HI_MAPI_VCAP_MIN_LDC_RATIO_Y);
    CHECK_MAPI_VCAP_MAX_VAL("s32YRatio", pstLdcAttr->stAttr.s32YRatio, HI_MAPI_VCAP_MAX_LDC_RATIO_Y);
    CHECK_MAPI_VCAP_MIN_VAL("s32YRatio", pstLdcAttr->stAttr.s32YRatio, HI_MAPI_VCAP_MIN_LDC_RATIO_DISTORTION);
    CHECK_MAPI_VCAP_MAX_VAL("s32YRatio", pstLdcAttr->stAttr.s32YRatio, HI_MAPI_VCAP_MAX_LDC_RATIO_DISTORTION);

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "don't support LDC when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n",
                        VcapPipeHdl);

    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr failed,VcapPipeHdl[%d],PipeChn[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    if (stChnAttr.enVideoFormat == VIDEO_FORMAT_TILE_16x8) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "don't support LDC when the VideoFormat is VIDEO_FORMAT_TILE_16x8\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    s32Ret = HI_MPI_VI_SetChnLDCAttr(ViPipe, ViChn, pstLdcAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnLDCAttr failed,VcapPipeHdl[%d],PipeChn[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetLDC(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, VI_LDC_ATTR_S *pstLdcAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "don't support LDC when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n",
                        VcapPipeHdl);
    s32Ret = HI_MPI_VI_GetChnLDCAttr(ViPipe, ViChn, pstLdcAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnLDCAttr failed,VcapPipeHdl[%d],PipeChn[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetISPDebug(HI_HANDLE VcapPipeHdl, ISP_DEBUG_INFO_S *pstDebugInfo,
                                    HI_MAPI_VCAP_CMD_E enCMD)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    ALG_LIB_S stLib;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    stLib.s32Id = ViPipe;

    switch (enCMD) {
        case HI_MAPI_VCAP_CMD_ISP_SetAEDebug:
            strcpy(stLib.acLibName, HI_AE_LIB_NAME);
            s32Ret = HI_MPI_AE_Ctrl(&stLib, AE_DEBUG_ATTR_SET, (HI_VOID *)pstDebugInfo);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_AE_Ctrl fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;

        case HI_MAPI_VCAP_CMD_ISP_SetAWBDebug:
            strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
            s32Ret = HI_MPI_AWB_Ctrl(&stLib, AWB_DEBUG_ATTR_SET, (HI_VOID *)pstDebugInfo);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_AWB_Ctrl fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "error CMD\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetLuma(HI_HANDLE VcapPipeHdl, const HI_MAPI_VCAP_LUMA_S *pstLuma)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_CSC_ATTR_S stISPCSCAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_NULL_PTR(pstLuma);
    CHECK_MAPI_VCAP_MAX_VAL("pstLuma->u8Luma", pstLuma->u8Luma, HI_MAPI_VCAP_MAX_LUMA);

    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stISPCSCAttr.u8Luma = pstLuma->u8Luma;
    s32Ret = HI_MPI_ISP_SetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetLuma(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_LUMA_S *pstLuma)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_CSC_ATTR_S stISPCSCAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_NULL_PTR(pstLuma);

    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstLuma->u8Luma = stISPCSCAttr.u8Luma;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetSaturation(HI_HANDLE VcapPipeHdl, const HI_MAPI_VCAP_SATURATION_S *pstSaturation)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_CSC_ATTR_S stISPCSCAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    // para check
    CHECK_MAPI_VCAP_NULL_PTR(pstSaturation);
    CHECK_MAPI_VCAP_MAX_VAL("Saturation value", pstSaturation->u8Saturation, HI_MAPI_VCAP_MAX_SATURATION);

    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stISPCSCAttr.u8Satu = pstSaturation->u8Saturation;
    s32Ret = HI_MPI_ISP_SetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetSaturation(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SATURATION_S *pstSaturation)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_CSC_ATTR_S stISPCSCAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    // para check
    CHECK_MAPI_VCAP_NULL_PTR(pstSaturation);

    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stISPCSCAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetCSCAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstSaturation->u8Saturation = stISPCSCAttr.u8Satu;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetSharpen(HI_HANDLE VcapPipeHdl, const HI_MAPI_VCAP_SHARPEN_S *pstSharpen)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_SHARPEN_ATTR_S stIspShpAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    if (ISP_SHARPEN_GAIN_NUM != HI_MAPI_ISP_SHARPEN_GAIN_NUM) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP,
                       "HI_MAPI_ISP_SHARPEN_GAIN_NUM != ISP_SHARPEN_GAIN_NUM,please check,VcapPipeHdl[%u]\n",
                       VcapPipeHdl);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = HI_MPI_ISP_GetIspSharpenAttr(ViPipe, &stIspShpAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetIspSharpenAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    switch (pstSharpen->enOpType) {
        case HI_MAPI_VCAP_OP_TYPE_DISABLE:
            stIspShpAttr.bEnable = HI_FALSE;
            break;

        case HI_MAPI_VCAP_OP_TYPE_AUTO:
            stIspShpAttr.bEnable = HI_TRUE;
            stIspShpAttr.enOpType = OP_TYPE_AUTO;
            break;

        case HI_MAPI_VCAP_OP_TYPE_MANUAL:
            stIspShpAttr.bEnable = HI_TRUE;
            stIspShpAttr.enOpType = OP_TYPE_MANUAL;
            memcpy(stIspShpAttr.stManual.au16EdgeStr, pstSharpen->stSharpenManualAttr.au16EdgeStr,
                   sizeof(HI_U16) * ISP_SHARPEN_GAIN_NUM);
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "error enOpType\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = HI_MPI_ISP_SetIspSharpenAttr(ViPipe, &stIspShpAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetIspSharpenAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetSharpen(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SHARPEN_S *pstSharpen)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_SHARPEN_ATTR_S stIspShpAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    if (ISP_SHARPEN_GAIN_NUM != HI_MAPI_ISP_SHARPEN_GAIN_NUM) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP,
                       "HI_MAPI_ISP_SHARPEN_GAIN_NUM != ISP_SHARPEN_GAIN_NUM,please check,VcapPipeHdl[%u]\n",
                       VcapPipeHdl);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    CHECK_MAPI_VCAP_NULL_PTR(pstSharpen);

    s32Ret = HI_MPI_ISP_GetIspSharpenAttr(ViPipe, &stIspShpAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetIspSharpenAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    if (!stIspShpAttr.bEnable) {
        pstSharpen->enOpType = HI_MAPI_VCAP_OP_TYPE_DISABLE;
        memcpy(pstSharpen->stSharpenManualAttr.au16EdgeStr, stIspShpAttr.stManual.au16EdgeStr,
               sizeof(HI_U16) * ISP_SHARPEN_GAIN_NUM);
        return HI_SUCCESS;
    }

    switch (stIspShpAttr.enOpType) {
        case OP_TYPE_AUTO:
            pstSharpen->enOpType = HI_MAPI_VCAP_OP_TYPE_AUTO;
            break;

        case OP_TYPE_MANUAL:
            pstSharpen->enOpType = HI_MAPI_VCAP_OP_TYPE_MANUAL;
            memcpy(pstSharpen->stSharpenManualAttr.au16EdgeStr, stIspShpAttr.stManual.au16EdgeStr,
                   sizeof(HI_U16) * ISP_SHARPEN_GAIN_NUM);
            break;

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "error enOpType\n");
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetMeteringMode(HI_HANDLE VcapPipeHdl,
                                        const HI_MAPI_VCAP_METERINGMODE_S *pstMeteringMode)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_STATISTICS_CFG_S stStatCfg;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_NULL_PTR(pstMeteringMode);

    if (HI_MAPI_VCAP_AE_ZONE_ROW != AE_ZONE_ROW
        || HI_MAPI_VCAP_AE_ZONE_COLUMN != AE_ZONE_COLUMN) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ZONE_ROW or ZONE_COLUMN Does not match ISP,please check,VcapPipeHdl[%u]\n",
                       VcapPipeHdl);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stStatCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetStatisticsConfig fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    memcpy(stStatCfg.stAECfg.au8Weight, pstMeteringMode->au8Weight, sizeof(HI_U8) * AE_ZONE_ROW * AE_ZONE_COLUMN);

    s32Ret = HI_MPI_ISP_SetStatisticsConfig(ViPipe, &stStatCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetStatisticsConfig fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_GetMeteringMode(HI_HANDLE VcapPipeHdl,
                                        HI_MAPI_VCAP_METERINGMODE_S *pstMeteringMode)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    ISP_STATISTICS_CFG_S stStatCfg;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_NULL_PTR(pstMeteringMode);

    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stStatCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetStatisticsConfig fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    memcpy(pstMeteringMode->au8Weight, stStatCfg.stAECfg.au8Weight, sizeof(HI_U8) * AE_ZONE_ROW * AE_ZONE_COLUMN);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetMirrorFilp(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                      const HI_MAPI_VCAP_MIRRORFLIP_ATTR_S *pstAttr)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stViChnAttr;
    VI_PIPE ViPipe;
    VI_CHN ViChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support mirror\flip when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
        VcapPipeHdl, PipeChnHdl);

#ifdef SUPPORT_DIS

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stVcapDisAttr.enDisStatus ==
        DIS_STATUS_RUNNING) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "Mirror and Flip not supported after enabling DIS\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

#endif
    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stViChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    if ((stViChnAttr.bMirror == pstAttr->bMirror) && (stViChnAttr.bFlip == pstAttr->bFlip)) {
        return HI_SUCCESS;
    }

    stViChnAttr.bMirror = pstAttr->bMirror;
    stViChnAttr.bFlip = pstAttr->bFlip;

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stViChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_InitISP(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret;
    ISP_CTRL_PARAM_S stIspCtrlParam;
    VI_PIPE ViPipe;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    memset(&stIspCtrlParam, 0x00, sizeof(ISP_CTRL_PARAM_S));

    s32Ret = HI_MPI_ISP_Init(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP[%u] Init failed\n", VcapPipeHdl);

    s32Ret = HI_MPI_ISP_GetCtrlParam(ViPipe, &stIspCtrlParam);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP [%u] GetCtrlParam failed\n", VcapPipeHdl);

    stIspCtrlParam.u32IntTimeOut = 30;
    s32Ret = HI_MPI_ISP_SetCtrlParam(ViPipe, &stIspCtrlParam);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP [%u]SetCtrlParam failed\n", VcapPipeHdl);

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_EnterStandBy(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    HI_S32 s32Ret;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    const ISP_SNS_OBJ_S *pstSnsObj;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "vcap has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1))
        || (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "you must set Vcap Attrs and start dev[%d] first\n", BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "pipe:[%d] isp is not started, no need to enter standby mode\n", ViPipe);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene == HI_MAPI_SENSOR_SCENE_STANDBY) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "you have enter standby mode already\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene = HI_MAPI_SENSOR_SCENE_STANDBY;
    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[BindDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "Get sensor Config failed, sesnor[%d],VcapPipeHdl[%d]\n", BindDevHdl, VcapPipeHdl);

    pstSnsObj = (ISP_SNS_OBJ_S *)stSensorCfg.pSensorObj;

    if (pstSnsObj->pfnStandby != NULL) {
        pstSnsObj->pfnStandby(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_ExitStandBy(HI_HANDLE VcapPipeHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    HI_S32 i = 0;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "vcap has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1))
        || (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "you must set Vcap Attrs and start dev[%d] first\n", BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun != HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ViPipe:[%d] isp is not started, no need to exit standby scene\n", ViPipe);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene != HI_MAPI_SENSOR_SCENE_STANDBY) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "you should enter standby mode first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene = HI_MAPI_SENSOR_SCENE_NORMAL;

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_RestartSensor(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    HI_S32 s32Ret;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    const ISP_SNS_OBJ_S *pstSnsObj;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "vcap has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1))
        || (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "you must set Vcap Attrs and start dev[%d] first\n", BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun != HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "ViPipe:[%d] isp is not started, no need to enter restart scene\n", ViPipe);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enSensorScene != HI_MAPI_SENSOR_SCENE_STANDBY) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "you should enter standby mode first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[BindDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "Get sensor Config failed, sesnor[%d],VcapPipeHdl[%d]\n", BindDevHdl, VcapPipeHdl);

    pstSnsObj = (ISP_SNS_OBJ_S *)stSensorCfg.pSensorObj;

    if (pstSnsObj->pfnRestart != NULL) {
        pstSnsObj->pfnRestart(ViPipe);
    }

    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_SetLDCv2(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                 HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
#ifdef SUPPORT_GYRO
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_CHN_ATTR_S stChnAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    // para check
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable,
                         "chn has not been start", HI_MAPI_VCAP_ENOT_PERM);

    if (MAPI_VCAP_GetMotionSensorStatus() == HI_FALSE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "need to Init GYRO first!\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "don't support LDC when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n",
                        VcapPipeHdl);

    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr failed,VcapPipeHdl[%d],PipeChn[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    if (stChnAttr.enCompressMode != COMPRESS_MODE_NONE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "only support COMPRESS_MODE_NONE when the LDC is enable\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    VI_LDCV2_ATTR_S *pstLdcv2Attr = &pstDisAttr->stLDCV2Attr;
    s32Ret = HI_MPI_VI_SetChnLDCV2Attr(ViPipe, ViChn, pstLdcv2Attr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnLDCV2Attr failed,VcapPipeHdl[%d],PipeChn[%d]\n", VcapPipeHdl,
                        PipeChnHdl);
#else
    MAPI_UNUSED(VcapPipeHdl);
    MAPI_UNUSED(PipeChnHdl);
    MAPI_UNUSED(pstDisAttr);

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support setLDCV2, when not support gyro.\n");
#endif
    return HI_SUCCESS;
}

static HI_S32 MAPI_VCAP_EnableChnDIS(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                     const HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
#ifdef SUPPORT_DIS
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    HI_FLOAT f32FrameRate = 0.0f;
    HI_S32 i = 0;
    HI_S32 s32DisCfgSeq = 0;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    /* 1.check param */
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDisAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stVcapDisAttr.enDisStatus ==
        DIS_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must stop dis first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* 3.set LDC, Gyro mode */
#ifdef SUPPORT_GYRO
    if (pstDisAttr->stDISConfig.enMode == DIS_MODE_GYRO) {
        if (MAPI_VCAP_GetMotionSensorStatus() == HI_FALSE) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "need to Init GYRO first!\n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }

        VI_LDCV2_ATTR_S stLDCV2Attr;
        VI_CHN_ATTR_S stChnAttr;

        s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);

        memset(&stLDCV2Attr, 0, sizeof(VI_LDCV2_ATTR_S));
        memcpy(&stLDCV2Attr, &pstDisAttr->stLDCV2Attr, sizeof(VI_LDCV2_ATTR_S));

        s32Ret = HI_MPI_VI_SetChnLDCV2Attr(ViPipe, ViChn, &stLDCV2Attr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnLDCV2Attr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);
    }
#endif

    /* 4.set DIS config param (static attr) */
    DIS_CONFIG_S stDISConfig;
    memset(&stDISConfig, 0, sizeof(DIS_CONFIG_S));
    memcpy(&stDISConfig, &(pstDisAttr->stDISConfig), sizeof(DIS_CONFIG_S));

    s32Ret = HI_MPI_VI_SetChnDISConfig(ViPipe, ViChn, &stDISConfig);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnDISConfig fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    /* 5.set DIS attr (dynamic attr) */
    DIS_ATTR_S stDISAttr;
    memset(&stDISAttr, 0, sizeof(DIS_ATTR_S));
    memcpy(&stDISAttr, &pstDisAttr->stDisAttr, sizeof(DIS_ATTR_S));
    stDISAttr.bEnable = HI_TRUE;

    s32Ret = HI_MPI_VI_SetChnDISAttr(ViPipe, ViChn, &stDISAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnDISAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

#ifdef SUPPORT_GYRO
    if (pstDisAttr->stDISConfig.enMode == DIS_MODE_GYRO) {
        if (MAPI_VCAP_GetMotionSensorStatus() == HI_FALSE) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "need to Init GYRO first!\n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }

        s32Ret = MAPI_VCAP_SetMotionSensor(&pstDisAttr->stMotionSensorInfo);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HAL_MAPI_VCAP_SetMotionSensor fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                            VcapPipeHdl, PipeChnHdl);
    }
#endif

    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stVcapDisAttr.enDisStatus =
        DIS_STATUS_RUNNING;
    g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].astPipeChnAttr[PipeChnHdl].stVcapDisAttr.enDisMotionType =
        pstDisAttr->stDISConfig.enMode;

    return HI_SUCCESS;

#else
    MAPI_UNUSED(pstDisAttr);

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support dis.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}

MAPI_GLOB_DEV_ATTR_S *GetVcapDevAttr(HI_HANDLE VcapDevHdl)
{
    return &g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl];
}

static HI_S32 MAPI_VCAP_DisableChnDIS(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                      const HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
#ifdef SUPPORT_DIS
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    DIS_ATTR_S stDISAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 1.check dis status */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stVcapDisAttr.enDisStatus == DIS_STATUS_STOP) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "DIS is already been stopped.\n");
        return HI_SUCCESS;
    }

#ifdef SUPPORT_GYRO
    if (pstDisAttr->stDISConfig.enMode == DIS_MODE_GYRO) {
        if (MAPI_VCAP_GetMotionSensorStatus() == HI_FALSE) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "need to Init GYRO first!\n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }

        /* 1.5 stop LDCV2 */
        VI_LDCV2_ATTR_S stLDCV2Attr;
        memset(&stLDCV2Attr, 0, sizeof(VI_LDCV2_ATTR_S));
        memcpy(&stLDCV2Attr, &pstDisAttr->stLDCV2Attr, sizeof(VI_LDCV2_ATTR_S));

        s32Ret = HI_MPI_VI_SetChnLDCV2Attr(ViPipe, ViChn, &stLDCV2Attr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnLDCV2Attr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);
    }
#else
    MAPI_UNUSED(pstDisAttr);

#endif

    /* 2.stop DIS */
    s32Ret = HI_MPI_VI_GetChnDISAttr(ViPipe, ViChn, &stDISAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnDISAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    stDISAttr.bEnable = HI_FALSE;
    s32Ret = HI_MPI_VI_SetChnDISAttr(ViPipe, ViChn, &stDISAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnDISAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stVcapDisAttr.enDisStatus = DIS_STATUS_STOP;

    return HI_SUCCESS;

#else
    MAPI_UNUSED(pstDisAttr);

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support dis.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}

#ifndef __HI3559AV100__
static HI_S32 MAPI_VCAP_SetFrameIntAttr(HI_HANDLE VcapPipeHdl, const FRAME_INTERRUPT_ATTR_S *pstFrameIntAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_VPSS_MODE_S stVIVPSSMode;
    VI_DEV ViDev;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    /* in Wdr Mode, the pipe not master no need to be set */
    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].stDevAttr.enWdrMode != WDR_MODE_NONE
        && (HI_HANDLE) ViPipe != g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind[0]) {
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_SYS_GetVIVPSSMode(&stVIVPSSMode);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SYS_GetVIVPSSMode failed,VcapPipeHdl[%d]\n", VcapPipeHdl);

#ifdef __HI3559V200__
    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_OFFLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_ONLINE_VPSS_OFFLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_PARALLEL_VPSS_PARALLEL) {
        s32Ret = HI_MPI_VI_SetPipeFrameInterruptAttr(ViPipe, pstFrameIntAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeFrameInterruptAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }
#else
    /* VI_OFFLINE_VPSS_ONLINE will be support in the future 56a version */
    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_OFFLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_PARALLEL_VPSS_PARALLEL) {
        s32Ret = HI_MPI_VI_SetPipeFrameInterruptAttr(ViPipe, pstFrameIntAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetPipeFrameInterruptAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }
#endif

    return HI_SUCCESS;
}
#endif

HI_S32 HI_MAPI_VCAP_InitSensor(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    HI_S32 s32Ret;
    combo_dev_attr_t stComboDevAttr;

    /* 1 .check paramas */
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if ((g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.enWdrMode == pstSensorAttr->enWdrMode)
        && (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.u8SnsMode == pstSensorAttr->u8SnsMode)
        && (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Width ==
                  pstSensorAttr->stSize.u32Width)
        && (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Height ==
                  pstSensorAttr->stSize.u32Height)) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "sensor[%u] mode not change\n", VcapDevHdl);
        return HI_SUCCESS;
    }

    /* 2.if vi dev is been started, then can't change sensor attrs */
    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enDevStatus == VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "You must stop vcap dev[%u] first\n", VcapDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* 4.Access to the mipi property settings of the sensor interface configuration file, get the current sequence, check the sensor attr passed over is correct. */
    s32Ret = MAPI_VCAP_LoadMipiConfig(VcapDevHdl, pstSensorAttr, &stComboDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "load mipi and sensor config fail,sensor[%d]\n", VcapDevHdl);

    if (g_as32SensorType[VcapDevHdl] == BT656) {
        /* the ahd driver has done reseting sensor clock */
        g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSensorScene = HI_MAPI_SENSOR_SCENE_STANDBY;
    }

    /* 5.Set mipi interface attrs */
    s32Ret = MAPI_VCAP_StartMipi(VcapDevHdl, &stComboDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "start mipi and sensor fail,sensor[%d]\n", VcapDevHdl);

    /* 6.set Peripheral attr */
    s32Ret = PeripheralMngSetAttr(VcapDevHdl, g_as32SensorType[VcapDevHdl], pstSensorAttr);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PeripheralMngSetAttr failed\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    /* 7.check switch mode */
    MAPI_VCAP_CheckSwitchMode(VcapDevHdl, pstSensorAttr);

    /* 7.save sensor mode and flag */
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit = HI_TRUE;
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.enWdrMode = pstSensorAttr->enWdrMode;
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.u8SnsMode = pstSensorAttr->u8SnsMode;
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Width = pstSensorAttr->stSize.u32Width;
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Height = pstSensorAttr->stSize.u32Height;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_DeinitSensor(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret;
    HI_HANDLE VcapPipeId;
    HI_S32 i;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 1. check dev is stoped */
    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enDevStatus == VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "vcap is running,you should stop vcap dev first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* 2. unreset mipi */
    s32Ret = MAPI_VCAP_StopMipi(VcapDevHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "unreset mipi fail,Sensor[%u]\n", VcapDevHdl);

    memset(&g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr, 0, sizeof(HI_MAPI_SENSOR_ATTR_S));
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit = HI_FALSE;
    g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enSwitchMode = SWITCH_MODE_NO_SENSOR_SWITCH;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate)
{
    HI_S32 s32Ret;
    ISP_PUB_ATTR_S stIspPubAttr;
    VI_PIPE ViPipe;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pf32Framerate);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    ViPipe = (VI_PIPE)MAPI_VCAP_GetMasterISPId(VcapDevHdl);
    CHECK_MAPI_VCAP_HANDLE("the id of isp which control sensor", ViPipe, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));

    memset(&stIspPubAttr, 0, sizeof(ISP_PUB_ATTR_S));
    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stIspPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPubAttr fail,VcapDevHdl[%d],VcapPipeHdl[%d]\n", VcapDevHdl,
                        ViPipe);

    /* 1.set mater isp pub attr */
    stIspPubAttr.f32FrameRate = *pf32Framerate;

    s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, &stIspPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetPubAttr fail,VcapDevHdl[%d],VcapPipeHdl[%d]\n", VcapDevHdl,
                        ViPipe);

    /* the frame of isp pub attr can not be 0, which is checked in sdk, so ndk not check repeatly */
    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.f32FrameRate = *pf32Framerate;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorFrameRate(HI_HANDLE VcapDevHdl, HI_FLOAT *pf32Framerate)
{
    VI_PIPE ViPipe;
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_PUB_ATTR_S stIspPubAttr;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pf32Framerate);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    ViPipe = (VI_PIPE)MAPI_VCAP_GetMasterISPId(VcapDevHdl);
    CHECK_MAPI_VCAP_HANDLE("the id of isp which control sensor", ViPipe, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));

    *pf32Framerate = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.f32FrameRate;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorAttr(HI_HANDLE VcapDevHdl, HI_MAPI_SENSOR_ATTR_S *pstSensorAttr)
{
    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    pstSensorAttr->enWdrMode = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.enWdrMode;
    pstSensorAttr->u8SnsMode = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.u8SnsMode;
    pstSensorAttr->stSize.u32Width = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Width;
    pstSensorAttr->stSize.u32Height = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stSensorAttr.stSize.u32Height;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorModesCnt(HI_HANDLE VcapDevHdl, HI_S32 *ps32ModesCnt)
{
    HI_S32 s32Ret;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    HI_S32 s32SensorModeCnt;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(ps32ModesCnt);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor config failed,sensor[%d]\n", VcapDevHdl);

    s32SensorModeCnt = stSensorCfg.stSensorMode.s32SensorModeCnt;

    *ps32ModesCnt = s32SensorModeCnt;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSensorAllModes(HI_HANDLE VcapDevHdl, HI_S32 s32ModesCnt,
                                      HI_MAPI_SENSOR_MODE_S *pstSensorModes)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSensorModes);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (s32ModesCnt < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Get Sensor All Modes input count is error.\n");
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[VcapDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "get sensor config failed,sensor[%d]\n", VcapDevHdl);

    const HI_MAPI_SENSOR_MODE_S *pstAllSensorModes = stSensorCfg.stSensorMode.pSensorMode;

    /* the ache memory size is need to be promised by user. */
    for (i = 0; i < s32ModesCnt; i++) {
        pstSensorModes[i].enWdrMode = pstAllSensorModes[i].enWdrMode;
        pstSensorModes[i].u8SnsMode = pstAllSensorModes[i].u8SnsMode;
        pstSensorModes[i].s32Width = pstAllSensorModes[i].s32Width;
        pstSensorModes[i].s32Height = pstAllSensorModes[i].s32Height;
        pstSensorModes[i].f32SnsMaxFrameRate = pstAllSensorModes[i].f32SnsMaxFrameRate;
    }

    return HI_SUCCESS;
}

/*
 1. Resolution constraints: sensor >=  Vi dev >= ISP >= PIPE >= CHN
 2. Frame rate constraints: sensor >=  Vi dev >= ISP >= PIPE >= CHN
*/
HI_S32 HI_MAPI_VCAP_SetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapPipeHdl;
    HI_HANDLE PipeChnHdl;
    HI_U32 i, j;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    /* 1 .set some dynamic attrs at dev running status */
    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enDevStatus == VCAP_STATUS_RUNNING) {
        /* check DEV */
        CHECK_MAPI_VCAP_TRUE(!MAPI_VCAP_IsDevAttrChange(VcapDevHdl, &pstVCapAttr->stVcapDevAttr),
                             "don't support modify vcap dev Attributes in the running status", HI_MAPI_VCAP_ENOT_PERM);
        /* check bind */
        CHECK_MAPI_VCAP_TRUE(!MAPI_VCAP_IsDevPipeBindChange(VcapDevHdl, pstVCapAttr),
                             "don't support modify vcap dev and pipebind in the running status",
                             HI_MAPI_VCAP_ENOT_PERM);

        /* Dynamic Set PIPE\ISP\CHN attr */
        if (pstVCapAttr->u32PipeBindNum > HI_MAPI_VCAP_MAX_PIPE_NUM) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PipeBindNum[%u] is error\n", pstVCapAttr->u32PipeBindNum);
            return HI_MAPI_VCAP_EILLEGAL_PARA;
        }

        for (i = 0; i < g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum; i++) {
            VcapPipeHdl = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];

            s32Ret = MAPI_VCAP_DynamicSetPipeAttr(VcapPipeHdl, pstVCapAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "set vcap pipe Dynamic attr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            s32Ret = MAPI_VCAP_DynamicSetISPAttr(VcapPipeHdl, pstVCapAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "set ISP pub Dynamic attr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            for (j = 0; j < HI_MAPI_PIPE_MAX_CHN_NUM; j++) {
                PipeChnHdl = PIPE_CHN_0 + j;

                s32Ret = MAPI_VCAP_DynamicSetChnAttr(VcapPipeHdl, PipeChnHdl, pstVCapAttr);
                CHECK_MAPI_VCAP_RET(s32Ret, "set ISP pub attr fail,VcapPipeHdl[%d], PipeChnHdl[%d]\n", VcapPipeHdl,
                                    PipeChnHdl);
            }
        }
    } else {
        /* 2 .static set all vcap attrs at dev INIT or NOTINIT status */
        s32Ret = MAPI_VCAP_StaticSetAttr(VcapDevHdl, pstVCapAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "set vcap attr fail,VcapDevHdl[%d]\n", VcapDevHdl);
        g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enDevStatus = VCAP_STATUS_INIT;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetAttr(HI_HANDLE VcapDevHdl, HI_MAPI_VCAP_ATTR_S *pstVCapAttr)
{
    HI_HANDLE VcapPipeId;
    HI_HANDLE PipeChnId;
    HI_U32 i, j;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstVCapAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].enDevStatus == VCAP_STATUS_NOTINIT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "please config VCAP attr first\n");
        return HI_MAPI_VCAP_ENOT_CONFIG;
    }

    pstVCapAttr->u32PipeBindNum = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum;
    memcpy(pstVCapAttr->aPipeIdBind, g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind,
           sizeof(HI_HANDLE) * HI_MAPI_VCAP_MAX_PIPE_NUM);
    memcpy(&pstVCapAttr->stVcapDevAttr, &g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].stDevAttr,
           sizeof(HI_MAPI_VCAP_DEV_ATTR_S));

    for (i = 0; i < g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum; i++) {
        VcapPipeId = g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];

        pstVCapAttr->astVcapPipeAttr[VcapPipeId].bIspBypass =
            g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stPipeAttr.bIspBypass;
        pstVCapAttr->astVcapPipeAttr[VcapPipeId].enPipeBypassMode =
            g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stPipeAttr.enPipeBypassMode;
        pstVCapAttr->astVcapPipeAttr[VcapPipeId].enCompressMode =
            g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stPipeAttr.enCompressMode;
        pstVCapAttr->astVcapPipeAttr[VcapPipeId].enPipeType =
            g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stPipeAttr.enPipeType;
        memcpy(&pstVCapAttr->astVcapPipeAttr[VcapPipeId].stFrameRate,
               &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stPipeAttr.stFrameRateCtrl, sizeof(FRAME_RATE_CTRL_S));
        memcpy(&pstVCapAttr->astVcapPipeAttr[VcapPipeId].stIspPubAttr,
               &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].stIspPubAttr, sizeof(HI_MAPI_PIPE_ISP_ATTR_S));

        for (j = 0; j < HI_MAPI_PIPE_MAX_CHN_NUM; j++) {
            PipeChnId = PIPE_CHN_0 + j;
            memcpy(&pstVCapAttr->astVcapPipeAttr[VcapPipeId].astPipeChnAttr[PipeChnId],
                   &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeId].astPipeChnAttr[PipeChnId].stChnAttr,
                   sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartDev(HI_HANDLE VcapDevHdl)
{
    VI_DEV_BIND_PIPE_S stDevBindPipe;
    VI_PIPE_ATTR_S stPipeAttr;
    VI_DEV ViDev;
    VI_PIPE ViPipe;
    HI_U32 i;
    HI_S32 s32Ret;

    ViDev = (VI_DEV)VcapDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[ViDev].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    /* 1. check vcap current status,you must call "HI_MAPI_VCAP_SetAttr" first then call "HI_MAPI_VCAP_StartDev" */
    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus == VCAP_STATUS_NOTINIT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "please config VCAP attr first,current dev is[%d]\n", ViDev);
        return HI_MAPI_VCAP_ENOT_CONFIG;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus == VCAP_STATUS_RUNNING) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "vcap dev[%d] is already been started\n", ViDev);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    CHECK_MAPI_VCAP_RET(s32Ret, "set vcap attr fail,VcapDevHdl[%d]", VcapDevHdl);

    /* 2. check bind attr
         tips:
         1)Bind already created PIPE is not supported
         2)Does not support the same PIPE in the binding group
         3)Does not support binding of the same PIPE to different VCAP DEV
         4)Parametric detection, constraint conditions are detected in the SDK, NDK only handles errors
    */

    memset(&stDevBindPipe, 0, sizeof(VI_DEV_BIND_PIPE_S));

    for (i = 0; i < HI_MAPI_VCAP_MAX_PIPE_NUM; i++) {
        if (i < g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum) {
            stDevBindPipe.PipeId[i] = g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind[i];
        } else {
            stDevBindPipe.PipeId[i] = HI_INVALID_HANDLE;
        }
    }

    /* 3 Bind vi and PIPE */
    stDevBindPipe.u32Num = g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum;

    s32Ret = HI_MPI_VI_SetDevBindPipe(ViDev, &stDevBindPipe);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call HI_MPI_VI_SetDevBindPipe fail,ret:%#x\n", s32Ret);
        goto VCAP_START_DEV_ERR;
    }

    /* 4 Create PIPE, and set properties, do not support duplicate create */
    PIPE_ATTR_S *pstPipeAttr = HI_NULL;
    VCAP_INNER_CONFIG_ATTR_S stVcapCfg;

    MAPI_VCAP_LoadParam(&stVcapCfg, g_as32SensorType);

    for (i = 0; i < stDevBindPipe.u32Num; i++) {
        ViPipe = (VI_PIPE)stDevBindPipe.PipeId[i];

        memcpy(&stPipeAttr, &stVcapCfg.astPipeAttr[ViPipe], sizeof(VI_PIPE_ATTR_S));
        pstPipeAttr = &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr;

        stPipeAttr.u32MaxH = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.stSize.u32Height;
        stPipeAttr.u32MaxW = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.stSize.u32Width;
        stPipeAttr.bIspBypass = pstPipeAttr->bIspBypass;
        stPipeAttr.enPipeBypassMode = pstPipeAttr->enPipeBypassMode;
        stPipeAttr.enCompressMode = pstPipeAttr->enCompressMode;
        stPipeAttr.stFrameRate.s32SrcFrameRate = pstPipeAttr->stFrameRateCtrl.s32SrcFrameRate;
        stPipeAttr.stFrameRate.s32DstFrameRate = pstPipeAttr->stFrameRateCtrl.s32DstFrameRate;

        s32Ret = MAPI_VCAP_AdaptPipePixFmtFromSensorCfg(ViDev, &stPipeAttr);

        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "get PIPE pixfmt fail,ret:%#x!\n", s32Ret);
            goto VCAP_CREATE_PIPE_ERR;
        }

        s32Ret = HI_MPI_VI_CreatePipe(ViPipe, &stPipeAttr);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "start vcap device fail,ret:%#x!\n", s32Ret);
            goto VCAP_CREATE_PIPE_ERR;
        }

        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate = HI_TRUE;
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl = ViDev;
    }

    g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus = VCAP_STATUS_RUNNING;

    s32Ret = PeripheralMngStart(VcapDevHdl, g_as32SensorType[VcapDevHdl]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PeripheralMngStart failed\n");
        goto VCAP_CREATE_PIPE_ERR;
    }

    return HI_SUCCESS;

VCAP_CREATE_PIPE_ERR:

    for (i = 0; i < stDevBindPipe.u32Num; i++) {
        ViPipe = (VI_PIPE)stDevBindPipe.PipeId[i];

        if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate == HI_TRUE) {
            s32Ret = HI_MPI_VI_DestroyPipe(ViPipe);

            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call HI_MPI_VI_DestroyPipe fail,ret:%#x!\n", s32Ret);
                continue;
            }

            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate = HI_FALSE;
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl = HI_INVALID_HANDLE;
        }
    }

VCAP_START_DEV_ERR:

    s32Ret = HI_MPI_VI_DisableDev(ViDev);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call HI_MPI_VI_DisableDev fail,ret:%#x!\n", s32Ret);
    }

    return HI_MAPI_VCAP_EOPERATE_FAIL;
}

/*
<1> Turn off the constraints of the path:
A. If you want to disable dev, then to ensure that the pipe bound to dev stop and destroy
B. If you want to destroy the pipe, then to ensure that the pipe is closed, and the two CHNs on the PIPE have been closed
C.pipe start (enable) / stop (disable) does not depend on the chn enable and disable

<2>Closing is done by closing all CHNs on the pipe-> closing all pipes bound to dev (VcapDevHdl) -> deactivating the specified (VcapDevHdl)dev
*/
HI_S32 HI_MAPI_VCAP_StopDev(HI_HANDLE VcapDevHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    HI_U32 i, j;

    ViDev = (VI_DEV)VcapDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapDevHdl", VcapDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "vcap dev[%d] is already been stopped,or in the stop state\n", ViDev);
        return HI_SUCCESS;
    }

    for (i = 0; i < g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].u32PipeBindNum; i++) {
        ViPipe = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[VcapDevHdl].aPipeIdBind[i];

        /* 1.Check if the channel on the PIPE is off */
        for (j = 0; j < HI_MAPI_PIPE_MAX_CHN_NUM; j++) {
            ViChn = PIPE_CHN_0 + j;

            if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable == HI_TRUE) {
                MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "CHN[%d] has not been disable yet\n", ViChn);

                s32Ret = HI_MPI_VI_DisableChn(ViPipe, ViChn);
                CHECK_MAPI_VCAP_RET(s32Ret,
                                    "call HI_MPI_VI_DisableChn fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                    VcapDevHdl, ViPipe, ViChn);
                g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable = HI_FALSE;
            }

            memset(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stChnAttr, 0,
                   sizeof(HI_MAPI_PIPE_CHN_ATTR_S));
#ifdef SUPPORT_DIS
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stVcapDisAttr.enDisStatus = DIS_STATUS_STOP;
#endif
        }

        /* 2.Check if all the Video PIPEs bound to the device are off */
        if ((g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO)
            && (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart == HI_TRUE)) {
            MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "PIPE[%d] has not been disable yet\n", ViPipe);

            s32Ret = HI_MPI_VI_StopPipe(ViPipe);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_StopPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d]\n", VcapDevHdl,
                                ViPipe);
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart = HI_FALSE;
        }

        /* 3.Check if all the Snap PIPEs bound to the device are off */
        if ((g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP)
            && (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bSnapPipeEnable == HI_TRUE)) {
            MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "PIPE[%d] has not been disable yet\n", ViPipe);

            s32Ret = HI_MPI_SNAP_DisablePipe(ViPipe);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_DisablePipe fail,VcapDevHdl[%d],VcapPipeHdl[%d]\n",
                                VcapDevHdl, ViPipe);
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bSnapPipeEnable = HI_FALSE;
        }

        /* 4.destroy PIPE */
        s32Ret = HI_MPI_VI_DestroyPipe(ViPipe);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_DestroyPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d]\n", VcapDevHdl,
                            ViPipe);

        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl = HI_INVALID_HANDLE;
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate = HI_FALSE;
        memset(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr, 0, sizeof(PIPE_ATTR_S));
        memset(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stSnapAttr, 0, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S));
    }

    /* 6.disable dev */
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_DisableDev fail,VcapDevHdl[%d]\n", VcapDevHdl);

    /* 7.clear dev global param,you must set attr again */
    g_stVcapGlobAttr.astGlobDevAttr[ViDev].MasterISPId = HI_INVALID_HANDLE;
    g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus = VCAP_STATUS_NOTINIT;
    g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum = 0;
    memset(g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind, 0, sizeof(HI_HANDLE) * HI_MAPI_VCAP_MAX_PIPE_NUM);
    memset(&g_stVcapGlobAttr.astGlobDevAttr[ViDev].stDevAttr, 0, sizeof(HI_MAPI_VCAP_DEV_ATTR_S));

    s32Ret = PeripheralMngStop(VcapDevHdl, g_as32SensorType[VcapDevHdl]);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PeripheralMngStop failed\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    HI_HANDLE BindDevHdl;
    VI_DEV ViDev;
    VI_CHN_ATTR_S stChnAttr;
    VI_VPSS_MODE_S stVIVPSSMode;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;
    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

#ifdef __HI3559AV100__ /* todo: Set Frame Interupt Attr in the next 59a version */
    /* 1.StartPipe MPI is called when the pipe type is video, when snap is not called in hi3559av100 version */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO
        && g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart == HI_FALSE) {

        s32Ret = HI_MPI_VI_StartPipe(ViPipe);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_StartPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                            ViDev, VcapPipeHdl, PipeChnHdl);
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart = HI_TRUE;
    }
#else
    /* 1.Support single snap pipe in hi3559v200/hi3556av100 version */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart == HI_FALSE) {
        if ((g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum == 1)
            || (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO)) {
            s32Ret = MAPI_VCAP_SetFrameIntAttr(VcapPipeHdl,
                                               &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.stFrameIntAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "Set Frame Int Attr fail,Pipe[%d]\n", VcapPipeHdl);

            s32Ret = HI_MPI_VI_StartPipe(ViPipe);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_StartPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n", ViDev,
                                VcapPipeHdl, PipeChnHdl);
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart = HI_TRUE;

            s32Ret = PeripheralMngStartChn(ViDev, g_as32SensorType[ViDev], (HI_U32)ViPipe);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PeripheralMngStartChn failed\n");
                return HI_MAPI_VCAP_EOPERATE_FAIL;
            }
        }
    }
#endif

    /* 2.In WDR mode, only the CHN of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].stDevAttr.enWdrMode != WDR_MODE_NONE
        && (HI_HANDLE) ViPipe != g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind[0]) {
        return HI_SUCCESS;
    }

    /* 3.enable CHN */
    HI_MAPI_PIPE_CHN_ATTR_S *pstChnAttr;
    VCAP_INNER_CONFIG_ATTR_S stVcapCfg;

    MAPI_VCAP_LoadParam(&stVcapCfg, g_as32SensorType);
    memcpy(&stChnAttr, &stVcapCfg.astChnAttr[ViChn], sizeof(VI_CHN_ATTR_S));

    pstChnAttr = &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].stChnAttr;

    stChnAttr.enPixelFormat = pstChnAttr->enPixelFormat;
    stChnAttr.enCompressMode = pstChnAttr->enCompressMode;
    stChnAttr.stSize.u32Width = pstChnAttr->stDestSize.u32Width;
    stChnAttr.stSize.u32Height = pstChnAttr->stDestSize.u32Height;
    stChnAttr.stFrameRate.s32SrcFrameRate = pstChnAttr->stFrameRate.s32SrcFrameRate;
    stChnAttr.stFrameRate.s32DstFrameRate = pstChnAttr->stFrameRate.s32DstFrameRate;

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnAttr fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                        ViDev, VcapPipeHdl, PipeChnHdl);

    /* tips:
        1)if isp is run then reset isp,
        2)this branch will take effect when switching timing
        3)In WDR mode, only the CHN of the master PIPE need to be reset */
    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bIspRun) {
        s32Ret = MAPI_VCAP_ResetISP(BindDevHdl, VcapPipeHdl);
        CHECK_MAPI_VCAP_RET(s32Ret, "reset isp,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);
    }

    s32Ret = HI_MPI_SYS_GetVIVPSSMode(&stVIVPSSMode);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SYS_GetVIVPSSMode fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                        ViDev, VcapPipeHdl, PipeChnHdl);

    /* Only VPSS offline mode needs to enable CHN */

#ifdef __HI3559AV100__

    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_ONLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_PARALLEL_VPSS_PARALLEL) {
        return HI_SUCCESS;
    }

#else

    if (stVIVPSSMode.aenMode[ViPipe] == VI_OFFLINE_VPSS_ONLINE
        || stVIVPSSMode.aenMode[ViPipe] == VI_ONLINE_VPSS_ONLINE) {
        return HI_SUCCESS;
    }

#endif

    s32Ret = HI_MPI_VI_EnableChn(ViPipe, ViChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_EnableChn fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n", ViDev,
                        VcapPipeHdl, PipeChnHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopChn(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;
    HI_U32 i = 0;
    HI_U32 j = 0;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 1.PIPE must have been created or it will return a failure */
    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = HI_MPI_VI_DisableChn(ViPipe, ViChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_DisableChn fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                        ViDev, VcapPipeHdl, PipeChnHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable = HI_FALSE;

#ifdef __HI3559AV100__
    /* 2.If all CHNs are off, turn off PIPE */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO) {
        for (i = 0; i < HI_MAPI_PIPE_MAX_CHN_NUM; i++) {
            if (!g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[i].bChnEnable) {
                j++;
            }
        }

        if (j == HI_MAPI_PIPE_MAX_CHN_NUM) {
            s32Ret = HI_MPI_VI_StopPipe(ViPipe);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_StopPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                ViDev, VcapPipeHdl, PipeChnHdl);
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart = HI_FALSE;
        }
    }
#else
    /* 2.If all CHNs are off, turn off PIPE */
    if ((g_stVcapGlobAttr.astGlobDevAttr[ViDev].u32PipeBindNum == 1)
        || (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_VIDEO)) {
        for (i = 0; i < HI_MAPI_PIPE_MAX_CHN_NUM; i++) {
            if (!g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[i].bChnEnable) {
                j++;
            }
        }

        if (j == HI_MAPI_PIPE_MAX_CHN_NUM) {
            s32Ret = HI_MPI_VI_StopPipe(ViPipe);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_StopPipe fail,VcapDevHdl[%d],VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                ViDev, VcapPipeHdl, PipeChnHdl);
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeStart = HI_FALSE;

            s32Ret = PeripheralMngStopChn(ViDev, g_as32SensorType[ViDev], (HI_U32)ViPipe);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PeripheralMngStopChn failed\n");
                return HI_MAPI_VCAP_EOPERATE_FAIL;
            }
        }
    }
#endif

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartISP(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    HI_S32 s32Ret;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 1.check dev and sensor status(flag) */
    if ((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n",
                       BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].bSensorInit,
                         "sensor has not been inited ,Operation not allowed", HI_MAPI_VCAP_ENOT_PERM);

    /* 2.In WDR mode, only the isp of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stDevAttr.enWdrMode != WDR_MODE_NONE
        && (HI_HANDLE) ViPipe != g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0]) {
        return HI_SUCCESS;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "[warning]:isp %d is already been started\n", ViPipe);
        return HI_SUCCESS;
    }

    /* 3. register sensor */
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    const ISP_SNS_OBJ_S *pstSnsObj;

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[BindDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "Get sensor Config failed,sensor[%d],VcapPipeHdl[%d]\n", BindDevHdl, VcapPipeHdl);

    pstSnsObj = (ISP_SNS_OBJ_S *)stSensorCfg.pSensorObj;
    s32Ret = MAPI_VCAP_ISP_SnsRegisterCallback(VcapPipeHdl, pstSnsObj);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP sensor[%d] Register failed,ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 4.Bind the I2C ID where SENSOR is located */
    s32Ret = MAPI_VCAP_ISP_BindSns(BindDevHdl, VcapPipeHdl, pstSnsObj, &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "bind sensor fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 5.register 3A */
    s32Ret = MAPI_VCAP_ISP_3A_Register(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_Isp_3A_Register fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 6.set Isp init attribute,include AE,AWB,must call before "HI_MPI_ISP_MemInit" */

    if (pstSnsObj->pfnSetInit && g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPInitAttr.bISPInitAttr != HI_NULL) {
        ISP_INIT_ATTR_S stISPInitAttr;
        memcpy(&stISPInitAttr, &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPInitAttr.stISPInitAttr,
               sizeof(ISP_INIT_ATTR_S));

        s32Ret = pstSnsObj->pfnSetInit(ViPipe, &stISPInitAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call pstSnsObj->pfnSetInit() fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);
    }

    /* 7.men init */
    s32Ret = HI_MPI_ISP_MemInit(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "Isp Mem Init failed,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 8.set pub attr */
    s32Ret = MAPI_VCAP_SetISPPubAttr(BindDevHdl, VcapPipeHdl, &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "set isp pub attr fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 9.isp init */
    s32Ret = MAPI_VCAP_InitISP(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "isp init fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 10.set sensor mirror and flip */
    if (pstSnsObj->pfnMirrorFlip != HI_NULL) {
        pstSnsObj->pfnMirrorFlip(ViPipe, g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.enSnsMirrorFlip);
    }

    /* 11.ISP run */
    s32Ret = pthread_create(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].pthIspRun, HI_NULL,
                            MAPI_VCAP_ISPRunProc, (HI_VOID *)&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].ISPHdl);

    CHECK_MAPI_VCAP_RET(s32Ret, "isp running thread creat failed,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopISP(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;
    ALG_LIB_S stAeLib;
    ALG_LIB_S stAwbLib;
    HI_S32 s32Ret;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    BindDevHdl = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_HANDLE("PipeBindDev", BindDevHdl, (HI_MAPI_VCAP_MAX_DEV_NUM - 1));

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun == HI_FALSE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "isp is already been stoped\n");
        return HI_SUCCESS;
    }

    /* 1. ISP exit */
    s32Ret = HI_MPI_ISP_Exit(ViPipe);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "ISP firmware abnormally exit with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = pthread_join(g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].pthIspRun, NULL);
    CHECK_MAPI_VCAP_RET(s32Ret, "call pthread_join fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 2. 3A UnRegister */
    s32Ret = MAPI_VCAP_ISP_3A_UnRegister(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "3A UnRegister fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 3. sensor UnRegister */
    HI_MAPI_COMBO_DEV_ATTR_S stSensorCfg;
    const ISP_SNS_OBJ_S *pstSnsObj;

    s32Ret = MAPI_SENSOR_GetConfig(g_as32SensorType[BindDevHdl], &stSensorCfg);
    CHECK_MAPI_VCAP_RET(s32Ret, "Get sensor Config fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    pstSnsObj = (ISP_SNS_OBJ_S *)stSensorCfg.pSensorObj;
    stAeLib.s32Id = ViPipe;
    stAwbLib.s32Id = ViPipe;
    strncpy(stAeLib.acLibName, HI_AE_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);
    strncpy(stAwbLib.acLibName, HI_AWB_LIB_NAME, ALG_LIB_NAME_SIZE_MAX);

    if (pstSnsObj == HI_NULL || pstSnsObj->pfnUnRegisterCallback == HI_NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "sensor_unregister_callback failed with HI_NULL!\n");
        return HI_MAPI_VCAP_ENULL_PTR;
    }

    s32Ret = pstSnsObj->pfnUnRegisterCallback(ViPipe, &stAeLib, &stAwbLib);
    CHECK_MAPI_VCAP_RET(s32Ret, "sensor_unregister_callback fail,sensor[%d],ISP[%d]\n", BindDevHdl, VcapPipeHdl);

    /* 4. clear flag */
    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].pthIspRun = -1;
    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bIspRun = HI_FALSE;
    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPInitAttr.bISPInitAttr = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr)
{
    VI_PIPE ViPipe;
    SNAP_ATTR_S stSnapAttr;
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_U32 u32ManExpTime = 0;
    VCAP_DUMP_RAWDATA_INFO_S *pstDumpRawInfo = NULL;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSnapAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    pstDumpRawInfo = &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stDumpRawInfo;

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bSnapPipeEnable == HI_TRUE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "stop trigger first then trigger\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (pstSnapAttr->enSnapType >= SNAP_TYPE_BUTT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "snap type error %d\n", ViPipe);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    s32Ret = HI_MPI_SNAP_GetPipeAttr(ViPipe, &stSnapAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_GetPipeAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    if (pstSnapAttr->enSnapType == SNAP_TYPE_NORMAL) {
        stSnapAttr.stNormalAttr.u32FrameCnt = pstSnapAttr->stNormalAttr.u32FrameCnt;
        stSnapAttr.stNormalAttr.u32RepeatSendTimes = pstSnapAttr->stNormalAttr.u32RepeatSendTimes;
        stSnapAttr.stNormalAttr.bZSL = HI_FALSE;
        stSnapAttr.stNormalAttr.u32FrameDepth = 0;
        stSnapAttr.stNormalAttr.u32Interval = 0;
        stSnapAttr.stNormalAttr.u32RollbackMs = 0;
    } else if (pstSnapAttr->enSnapType == SNAP_TYPE_PRO) {
        stSnapAttr.stProAttr.stProParam.enOperationMode = pstSnapAttr->stProAttr.stProParam.enOperationMode;
        stSnapAttr.stProAttr.u32FrameCnt = pstSnapAttr->stProAttr.u32FrameCnt;
        stSnapAttr.stProAttr.u32RepeatSendTimes = pstSnapAttr->stProAttr.u32RepeatSendTimes;

        if (pstSnapAttr->stProAttr.stProParam.enOperationMode == OPERATION_MODE_AUTO) {
            memcpy(&stSnapAttr.stProAttr.stProParam.stAutoParam, &pstSnapAttr->stProAttr.stProParam.stAutoParam,
                   sizeof(SNAP_PRO_AUTO_PARAM_S));
        } else if (pstSnapAttr->stProAttr.stProParam.enOperationMode == OPERATION_MODE_MANUAL) {
            memcpy(&stSnapAttr.stProAttr.stProParam.stManualParam, &pstSnapAttr->stProAttr.stProParam.stManualParam,
                   sizeof(SNAP_PRO_MANUAL_PARAM_S));
            for (i = 0; i < PRO_MAX_FRAME_NUM; i++) {
                if (u32ManExpTime < pstSnapAttr->stProAttr.stProParam.stManualParam.au32ManExpTime[i]) {
                    u32ManExpTime = pstSnapAttr->stProAttr.stProParam.stManualParam.au32ManExpTime[i];
                }
            }
            /* u32ExposureTime unit is ms, snap attr unit is us. */
            pstDumpRawInfo->u32ExposureTime = u32ManExpTime / 1000;
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "enOperationMode attr error with vcap pipe[%d]\n", ViPipe);
            return s32Ret;
        }
    }

    stSnapAttr.enSnapType = pstSnapAttr->enSnapType;
    stSnapAttr.bLoadCCM = pstSnapAttr->bLoadCCM;

    s32Ret = HI_MPI_SNAP_SetPipeAttr(ViPipe, &stSnapAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "set snap attr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stSnapAttr, pstSnapAttr, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSnapAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_ATTR_S *pstSnapAttr)
{
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstSnapAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    memcpy(pstSnapAttr, &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stSnapAttr, sizeof(HI_MAPI_VCAP_SNAP_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartTrigger(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    VI_DEV ViDev;
    HI_S32 s32Ret;
    HI_U32 u32Delay = 0;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "vcap has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stSnapAttr.enSnapType != SNAP_TYPE_NORMAL
        && g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stSnapAttr.enSnapType != SNAP_TYPE_PRO) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set snap attr first\n");
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = HAI_MAPI_VCAP_EnableSnapPipe(VcapPipeHdl, &g_stVcapGlobAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "enable snap pipe fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    /* double PIPE snap mode need delay > 2 frames, because ISP takes about two frames to process the JPEG */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP) {
        u32Delay = (HI_U32)((TRIGGER_DELAY_FRAME_CNT * 1000 * 1000) /
                            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stIspPubAttr.f32FrameRate);
        usleep(u32Delay);
    }

    s32Ret = HI_MPI_SNAP_TriggerPipe(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "trigger fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopTrigger(HI_HANDLE VcapPipeHdl)
{
    VI_PIPE ViPipe;
    HI_S32 s32Ret;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    s32Ret = HAI_MAPI_VCAP_DisableSnapPipe(VcapPipeHdl, &g_stVcapGlobAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "Disable snap pipe fail,VcapPipeHdl[%d]\n\n", VcapPipeHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    VI_STITCH_GRP_ATTR_S stVistitchAttr;
    HI_S32 i;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstStitchAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = HI_MPI_VI_GetStitchGrpAttr(VcapStitchHdl, &stVistitchAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetStitchGrpAttr fail,VcapStitchHdl[%d]\n", VcapStitchHdl);

    for (i = 0; i < HI_MAPI_VCAP_MAX_DEV_NUM; i++) {
        if (g_stVcapGlobAttr.astGlobDevAttr[i].enDevStatus != VCAP_STATUS_NOTINIT) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "please set stitch atts before Set Vcap Attr,vcap[%d]\n", i);
            return HI_MAPI_VCAP_ENOT_PERM;
        }
    }

    if (pstStitchAttr->bStitch == HI_TRUE && stVistitchAttr.bStitch == HI_TRUE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "don't repeat set stitch, VcapStitchHdl[%d]\n", VcapStitchHdl);
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VI_SetStitchGrpAttr(VcapStitchHdl, pstStitchAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "set stitch attr fail,VcapStitchHdl[%d]\n", VcapStitchHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetStitchAttr(HI_HANDLE VcapStitchHdl, VI_STITCH_GRP_ATTR_S *pstStitchAttr)
{
    HI_S32 s32Ret;
    VI_STITCH_GRP_ATTR_S stVistitchAttr;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstStitchAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = HI_MPI_VI_GetStitchGrpAttr(VcapStitchHdl, &stVistitchAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "get stitch attr fail,VcapStitchHdl[%d]\n", VcapStitchHdl);

    memcpy(pstStitchAttr, &stVistitchAttr, sizeof(VI_STITCH_GRP_ATTR_S));

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_VCAP_StitchTrigger(HI_HANDLE VcapStitchHdl)
{
    HI_S32 s32Ret;
    VI_STITCH_GRP_ATTR_S stVistitchAttr;
    HI_U32 i;
    VI_PIPE ViPipe;

    CHECK_MAPI_VCAP_HANDLE("VcapStitchHdl", VcapStitchHdl, (HI_MAPI_VCAP_STITCH_MAX_GROUP_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = HI_MPI_VI_GetStitchGrpAttr(VcapStitchHdl, &stVistitchAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetStitchGrpAttr fail,VcapStitchHdl[%d]\n", VcapStitchHdl);

    if (!stVistitchAttr.bStitch) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "stitch has not been set enable\n");
        return HI_MAPI_VCAP_ENOT_CONFIG;
    }

    for (i = 0; i < stVistitchAttr.u32PipeNum; i++) {
        ViPipe = stVistitchAttr.PipeId[i];
        CHECK_MAPI_VCAP_HANDLE("PIPE ID of the stitch binding group", ViPipe, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));

        if (!g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].bPipeCreate) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "PIPE[%d] is not created ,please start vcap device first", ViPipe);
            return HI_MAPI_VCAP_ENOT_PERM;
        }
    }

    s32Ret = HI_MPI_SNAP_MultiTrigger(VcapStitchHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_MultiTrigger fail,VcapStitchHdl[%d]\n", VcapStitchHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_S32 j = 0;
    ISP_DCF_INFO_S stIspDCFInfo;
    GPS_INFO_S stGpsInfo;
    ISP_PIRIS_ATTR_S stPirisAttr;
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstExifInfo);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    /* in double pipe, only need to set the master pipe exif info. */
    ViPipe = MAPI_VCAP_GetMasterISPId(BindDevHdl);
    if (ViPipe == HI_INVALID_HANDLE) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "Get master ISPid fail %d in setExifInfo\n", VcapPipeHdl);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    /* 1.In WDR mode, only the CHN of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stDevAttr.enWdrMode != WDR_MODE_NONE
        && (HI_HANDLE) ViPipe != g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0]) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "In WDR mode, only the ExifInfo of the master PIPE need to be set.\n");
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_ISP_GetDCFInfo(ViPipe, &stIspDCFInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetDCFInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    if (DCF_DRSCRIPTION_LENGTH != HI_MAPI_EXIF_DRSCRIPTION_LENGTH) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP,
                        "HI_MAPI_EXIF_DRSCRIPTION_LENGTH[%d] != DCF_DRSCRIPTION_LENGTH[%d],please check\n",
                        HI_MAPI_EXIF_DRSCRIPTION_LENGTH, DCF_DRSCRIPTION_LENGTH);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    strncpy((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8ImageDescription,
            (HI_CHAR *)pstExifInfo->au8ImageDescription, strlen((HI_CHAR *)pstExifInfo->au8ImageDescription));
    stIspDCFInfo.stIspDCFConstInfo.au8ImageDescription[strlen((HI_CHAR *)pstExifInfo->au8ImageDescription)] = '\0';

    strncpy((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Make, (HI_CHAR *)pstExifInfo->au8Make,
            strlen((HI_CHAR *)pstExifInfo->au8Make));
    stIspDCFInfo.stIspDCFConstInfo.au8Make[strlen((HI_CHAR *)pstExifInfo->au8Make)] = '\0';

    strncpy((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Model, (HI_CHAR *)pstExifInfo->au8Model,
            strlen((HI_CHAR *)pstExifInfo->au8Model));
    stIspDCFInfo.stIspDCFConstInfo.au8Model[strlen((HI_CHAR *)pstExifInfo->au8Model)] = '\0';

    strncpy((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Software, (HI_CHAR *)pstExifInfo->au8Software,
            strlen((HI_CHAR *)pstExifInfo->au8Software));
    stIspDCFInfo.stIspDCFConstInfo.au8Software[strlen((HI_CHAR *)pstExifInfo->au8Software)] = '\0';

    stIspDCFInfo.stIspDCFConstInfo.u8LightSource = pstExifInfo->u32LightSource;
    stIspDCFInfo.stIspDCFConstInfo.u8MeteringMode = pstExifInfo->u32MeteringMode;
    stIspDCFInfo.stIspDCFConstInfo.u32FocalLength = pstExifInfo->u32FocalLength;
    stIspDCFInfo.stIspDCFConstInfo.u8FocalLengthIn35mmFilm = pstExifInfo->u8FocalLengthIn35mmFilm;

    s32Ret = HI_MPI_ISP_SetDCFInfo(ViPipe, &stIspDCFInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetDCFInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    s32Ret = HI_MPI_ISP_GetPirisAttr(ViPipe, &stPirisAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPirisAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stPirisAttr.enMaxIrisFNOTarget = pstExifInfo->stApertureInfo.enMaxIrisFNOTarget;
    stPirisAttr.enMinIrisFNOTarget = pstExifInfo->stApertureInfo.enMinIrisFNOTarget;
    stPirisAttr.bFNOExValid = pstExifInfo->stApertureInfo.bFNOExValid;
    stPirisAttr.u32MaxIrisFNOTarget = pstExifInfo->stApertureInfo.u32MaxIrisFNOTarget;
    stPirisAttr.u32MinIrisFNOTarget = pstExifInfo->stApertureInfo.u32MinIrisFNOTarget;

    s32Ret = HI_MPI_ISP_SetPirisAttr(ViPipe, &stPirisAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetPirisAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    stGpsInfo.chGPSLatitudeRef = pstExifInfo->stGPSInfo.chGPSLatitudeRef;    // 'N'/'S'
    stGpsInfo.chGPSLongitudeRef = pstExifInfo->stGPSInfo.chGPSLongitudeRef;  // 'E'/'W'
    stGpsInfo.u8GPSAltitudeRef = pstExifInfo->stGPSInfo.u8GPSAltitudeRef;    // 0 above sea level,1 below sea level

    for (i = 0; i < GPS_LATITUDE_COORDINATE; i++) {
        for (j = 0; j < GPS_NUMBERATOR_DENOMINATOR; j++) {
            stGpsInfo.au32GPSLatitude[i][j] = pstExifInfo->stGPSInfo.au32GPSLatitude[i][j];
            stGpsInfo.au32GPSLongitude[i][j] = pstExifInfo->stGPSInfo.au32GPSLongitude[i][j];
        }
    }

    for (i = 0; i < GPS_NUMBERATOR_DENOMINATOR; i++) {
        stGpsInfo.au32GPSAltitude[i] = pstExifInfo->stGPSInfo.au32GPSAltitude[i];
    }

    s32Ret = HI_MPI_SYS_SetGPSInfo(&stGpsInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SYS_SetGPSInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetExifInfo(HI_HANDLE VcapPipeHdl, HI_MAPI_SNAP_EXIF_INFO_S *pstExifInfo)
{
    HI_S32 s32Ret;
    HI_S32 i = 0;
    HI_S32 j = 0;
    ISP_DCF_INFO_S stIspDCFInfo;
    GPS_INFO_S stGpsInfo;
    ISP_PIRIS_ATTR_S stPirisAttr;
    VI_PIPE ViPipe;
    VI_DEV BindDevHdl;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstExifInfo);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    /* 1.In WDR mode, only the CHN of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stDevAttr.enWdrMode != WDR_MODE_NONE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "In WDR mode, the CHN of the master PIPE will be get.\n");
        if ((HI_HANDLE)ViPipe != g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0]) {
            ViPipe = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0];
        }
    }

    /* 1.get the DCF information. */
    s32Ret = HI_MPI_ISP_GetDCFInfo(ViPipe, &stIspDCFInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetDCFInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    if (DCF_DRSCRIPTION_LENGTH != HI_MAPI_EXIF_DRSCRIPTION_LENGTH) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP,
                        "HI_MAPI_EXIF_DRSCRIPTION_LENGTH[%d] != DCF_DRSCRIPTION_LENGTH[%d],please check\n",
                        HI_MAPI_EXIF_DRSCRIPTION_LENGTH, DCF_DRSCRIPTION_LENGTH);
        return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    strncpy((HI_CHAR *)pstExifInfo->au8ImageDescription,
            (HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8ImageDescription,
            strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8ImageDescription));
    pstExifInfo->au8ImageDescription[strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8ImageDescription)] = '\0';

    strncpy((HI_CHAR *)pstExifInfo->au8Make, (HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Make,
            strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Make));
    pstExifInfo->au8Make[strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Make)] = '\0';

    strncpy((HI_CHAR *)pstExifInfo->au8Model, (HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Model,
            strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Model));
    pstExifInfo->au8Model[strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Model)] = '\0';

    strncpy((HI_CHAR *)pstExifInfo->au8Software, (HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Software,
            strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Software));
    pstExifInfo->au8Software[strlen((HI_CHAR *)stIspDCFInfo.stIspDCFConstInfo.au8Software)] = '\0';

    pstExifInfo->u32MeteringMode = stIspDCFInfo.stIspDCFConstInfo.u8MeteringMode;
    pstExifInfo->u32FocalLength = stIspDCFInfo.stIspDCFConstInfo.u32FocalLength;
    pstExifInfo->u32LightSource = stIspDCFInfo.stIspDCFConstInfo.u8LightSource;
    pstExifInfo->u8FocalLengthIn35mmFilm = stIspDCFInfo.stIspDCFConstInfo.u8FocalLengthIn35mmFilm;

    /* 2.get the aperture information */
    s32Ret = HI_MPI_ISP_GetPirisAttr(ViPipe, &stPirisAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPirisAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstExifInfo->stApertureInfo.enMaxIrisFNOTarget = stPirisAttr.enMaxIrisFNOTarget;
    pstExifInfo->stApertureInfo.enMinIrisFNOTarget = stPirisAttr.enMinIrisFNOTarget;
    pstExifInfo->stApertureInfo.bFNOExValid = stPirisAttr.bFNOExValid;
    pstExifInfo->stApertureInfo.u32MaxIrisFNOTarget = stPirisAttr.u32MaxIrisFNOTarget;
    pstExifInfo->stApertureInfo.u32MinIrisFNOTarget = stPirisAttr.u32MinIrisFNOTarget;

    /* 3.get the GPS information */
    s32Ret = HI_MPI_SYS_GetGPSInfo(&stGpsInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SYS_GetGPSInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

    pstExifInfo->stGPSInfo.chGPSLatitudeRef = stGpsInfo.chGPSLatitudeRef;    // 'N'/'S'
    pstExifInfo->stGPSInfo.chGPSLongitudeRef = stGpsInfo.chGPSLongitudeRef;  // 'E'/'W'
    pstExifInfo->stGPSInfo.u8GPSAltitudeRef = stGpsInfo.u8GPSAltitudeRef;    // 0 above sea level,1 below sea level

    for (i = 0; i < GPS_LATITUDE_COORDINATE; i++) {
        for (j = 0; j < GPS_NUMBERATOR_DENOMINATOR; j++) {
            pstExifInfo->stGPSInfo.au32GPSLatitude[i][j] = stGpsInfo.au32GPSLatitude[i][j];
            pstExifInfo->stGPSInfo.au32GPSLongitude[i][j] = stGpsInfo.au32GPSLongitude[i][j];
        }
    }

    for (i = 0; i < GPS_NUMBERATOR_DENOMINATOR; i++) {
        pstExifInfo->stGPSInfo.au32GPSAltitude[i] = stGpsInfo.au32GPSAltitude[i];
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_PIPE MasterISP;
    VI_DEV BindDevHdl;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_MAX_VAL("enCMD", enCMD, (HI_MAPI_VCAP_CMD_BUTT - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if (enCMD != HI_MAPI_VCAP_CMD_ENTER_STANDBY
        && enCMD != HI_MAPI_VCAP_CMD_EXIT_STANDBY
        && enCMD != HI_MAPI_VCAP_CMD_RESTART_SENSOR) {
        CHECK_MAPI_VCAP_NULL_PTR(pAttr);
        CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    }

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    /* 1.In WDR mode, only the CHN of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stDevAttr.enWdrMode != WDR_MODE_NONE
        && ViPipe != (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0]) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "In WDR mode, only the CHN of the master PIPE need to be set.\n");
        return HI_SUCCESS;
    }

    if ((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n",
                       BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    MasterISP = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].MasterISPId;

    switch (enCMD) {
        case HI_MAPI_VCAP_CMD_ISP_ExposureAttr: {
            HI_MAPI_VCAP_EXPOSURE_ATTR_S *pstVCapExposure = (HI_MAPI_VCAP_EXPOSURE_ATTR_S *)pAttr;

            s32Ret = MAPI_VCAP_SetExposure(VcapPipeHdl, pstVCapExposure);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetExposure fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_WbAttr: {
            HI_MAPI_VCAP_WB_ATTR_S *pstWBAttr = (HI_MAPI_VCAP_WB_ATTR_S *)pAttr;

            s32Ret = MAPI_VCAP_SetWB(VcapPipeHdl, pstWBAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetWB fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Set3AInit: {
            memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPInitAttr.stISPInitAttr, (ISP_INIT_ATTR_S *)pAttr,
                   sizeof(ISP_INIT_ATTR_S));
            g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPInitAttr.bISPInitAttr = HI_TRUE;
            break;
        }

        case HI_MAPI_VCAP_CMD_Rotate: {
            ROTATION_E enRotation;
            ROTATION_E *penRotationTemp;

            s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret,
                "don't support Rotate when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n", VcapPipeHdl);

            penRotationTemp = (ROTATION_E *)pAttr;
            s32Ret = HI_MPI_VI_GetChnRotation(ViPipe, ViChn, &enRotation);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnRotation fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                VcapPipeHdl, PipeChnHdl);

            if (*penRotationTemp == enRotation) {
                return HI_SUCCESS;
            }

            enRotation = *penRotationTemp;
            s32Ret = HI_MPI_VI_SetChnRotation(ViPipe, ViChn, enRotation);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnRotation fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                VcapPipeHdl, PipeChnHdl);

            break;
        }

        case HI_MAPI_VCAP_CMD_MirrorFlip: {
            HI_MAPI_VCAP_MIRRORFLIP_ATTR_S *pstMirrorFlip;

            pstMirrorFlip = (HI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr;
            s32Ret = MAPI_VCAP_SetMirrorFilp(ViPipe, ViChn, pstMirrorFlip);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetMirrorFilp fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                                VcapPipeHdl, PipeChnHdl);

            break;
        }

        case HI_MAPI_VCAP_CMD_LDC: {
            s32Ret = MAPI_VCAP_SetLDC(ViPipe, ViChn, (VI_LDC_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetLDC fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                                PipeChnHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_LDCV2: {
            s32Ret = MAPI_VCAP_SetLDCv2(ViPipe, ViChn, (HI_MAPI_VCAP_DIS_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetLDC fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                                PipeChnHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_DngColorParam: {
            s32Ret = HI_MPI_ISP_SetDngColorParam(MasterISP, (ISP_DNG_COLORPARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetDngColorParam fail,MasterISP[%d],ISP[%d]\n", MasterISP,
                                VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_SetAEDebug: {
            s32Ret = MAPI_VCAP_SetISPDebug(MasterISP, (ISP_DEBUG_INFO_S *)pAttr, enCMD);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetISPDebug fail,MasterISP[%d],ISP[%d]\n", MasterISP,
                                VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_SetAWBDebug: {
            s32Ret = MAPI_VCAP_SetISPDebug(MasterISP, (ISP_DEBUG_INFO_S *)pAttr, enCMD);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetISPDebug fail,MasterISP[%d],ISP[%d]\n", MasterISP,
                                VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Saturation: {
            s32Ret = MAPI_VCAP_SetSaturation(VcapPipeHdl, (HI_MAPI_VCAP_SATURATION_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetSaturation fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Luma: {
            s32Ret = MAPI_VCAP_SetLuma(VcapPipeHdl, (HI_MAPI_VCAP_LUMA_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetLuma fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Sharpen: {
            s32Ret = MAPI_VCAP_SetSharpen(VcapPipeHdl, (HI_MAPI_VCAP_SHARPEN_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetSharpen fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_MeteringMode: {
            s32Ret = MAPI_VCAP_SetMeteringMode(VcapPipeHdl, (HI_MAPI_VCAP_METERINGMODE_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetMeteringMode fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_WDR: {
            s32Ret = HI_MPI_ISP_SetFSWDRAttr(ViPipe, (ISP_WDR_FS_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetFSWDRAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_PipeDiffAttr: {
            s32Ret = HI_MPI_ISP_SetPipeDifferAttr(ViPipe, (ISP_PIPE_DIFF_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetPipeDifferAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Cac: {
            s32Ret = HI_MPI_ISP_SetLocalCacAttr(ViPipe, (ISP_LOCAL_CAC_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_SetLocalCacAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ENTER_STANDBY: {
            s32Ret = MAPI_VCAP_EnterStandBy(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_EnterStandBy fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_EXIT_STANDBY: {
            s32Ret = MAPI_VCAP_ExitStandBy(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_ExitStandBy fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_RESTART_SENSOR: {
            s32Ret = MAPI_VCAP_RestartSensor(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_RestartSensor fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetAttrEx(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_MAPI_VCAP_CMD_E enCMD,
                              HI_VOID *pAttr, HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE MasterISP;
    VI_DEV BindDevHdl;
    VI_PIPE ViPipe;
    VI_CHN ViChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_MAX_VAL("enCMD", enCMD, (HI_MAPI_VCAP_CMD_BUTT - 1));
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    /* 1.In WDR mode, only the CHN of the master PIPE need to be started */
    if (g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].stDevAttr.enWdrMode != WDR_MODE_NONE) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "In WDR mode, only the CHN of the master PIPE will be get.\n");
        if ((HI_HANDLE)ViPipe != g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0]) {
            ViPipe = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].aPipeIdBind[0];
        }
    }

    if ((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n",
                       BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    MasterISP = (VI_PIPE)g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].MasterISPId;

    switch (enCMD) {
        case HI_MAPI_VCAP_CMD_ISP_ExposureAttr: {
            s32Ret = MAPI_VCAP_GetExposure(VcapPipeHdl, (HI_MAPI_VCAP_EXPOSURE_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "get exposure fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_WbAttr: {
            ((HI_MAPI_VCAP_WB_ATTR_S *)pAttr)->enOpType =
                g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.enOpType;
            ((HI_MAPI_VCAP_WB_ATTR_S *)pAttr)->stWBMode.u32ColorTemp =
                g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stVcapISPWbAttr.stWBMode.u32ColorTemp;
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_GetExposureInfo: {
            ISP_EXP_INFO_S stExpInfo;

            s32Ret = HI_MPI_ISP_QueryExposureInfo(MasterISP, &stExpInfo);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_QueryExposureInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32ExpTime = stExpInfo.u32ExpTime;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32AGain = stExpInfo.u32AGain;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32DGain = stExpInfo.u32DGain;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32ISPDGain = stExpInfo.u32ISPDGain;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32Exposure = stExpInfo.u32Exposure;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32LinesPer500ms = stExpInfo.u32LinesPer500ms;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32PirisFNO = stExpInfo.u32PirisFNO;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32LongExpTime = stExpInfo.u32LongExpTime;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32ShortExpTime = stExpInfo.u32ShortExpTime;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32MedianExpTime = stExpInfo.u32MedianExpTime;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->bExposureIsMAX = stExpInfo.bExposureIsMAX;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->s16HistError = stExpInfo.s16HistError;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u8AveLum = stExpInfo.u8AveLum;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32Fps = stExpInfo.u32Fps;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32ISO = stExpInfo.u32ISO;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32RefExpRatio = stExpInfo.u32RefExpRatio;
            ((HI_MAPI_VCAP_EXPOSURE_INFO_S *)pAttr)->u32FirstStableTime = stExpInfo.u32FirstStableTime;

            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_GetAwbInfo: {
            ISP_WB_INFO_S stWBInfo;
            ISP_WB_INFO_S *pstWBInfoTemp;

            pstWBInfoTemp = (ISP_WB_INFO_S *)pAttr;

            s32Ret = HI_MPI_ISP_QueryWBInfo(MasterISP, &stWBInfo);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_QueryWBInfo fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            memcpy(pstWBInfoTemp, &stWBInfo, sizeof(ISP_WB_INFO_S));
            memcpy(pstWBInfoTemp->au16CCM, stWBInfo.au16CCM, sizeof(HI_U16) * CCM_MATRIX_SIZE);

            break;
        }

        case HI_MAPI_VCAP_CMD_Rotate: {
            s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret,
                "don't support Rotate when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n", VcapPipeHdl);

            s32Ret = HI_MPI_VI_GetChnRotation(ViPipe, ViChn, (ROTATION_E *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnRotation fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_MirrorFlip: {
            VI_CHN_ATTR_S stChnAttr;

            s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
            CHECK_MAPI_VCAP_RET(s32Ret,
                "don't support Mirror when the vpss work mode is online or parallel,VcapPipeHdl[%d]\n", VcapPipeHdl);

            s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);

            ((HI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr)->bMirror = stChnAttr.bMirror;
            ((HI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr)->bFlip = stChnAttr.bFlip;

            break;
        }

        case HI_MAPI_VCAP_CMD_LDC: {
            s32Ret = MAPI_VCAP_GetLDC(ViPipe, ViChn, (VI_LDC_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetLDC fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                                PipeChnHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_GetDngImageStaticInfo: {
            s32Ret = HI_MPI_ISP_GetDngImageStaticInfo(MasterISP, (DNG_IMAGE_STATIC_INFO_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetDngImageStaticInfo fail,MasterISP[%d],VcapPipeHdl[%d]\n",
                                MasterISP, VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_DngColorParam: {
            s32Ret = HI_MPI_ISP_GetDngColorParam(MasterISP, (ISP_DNG_COLORPARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetDngColorParam fail,MasterISP[%d],VcapPipeHdl[%d]\n",
                                MasterISP, VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Saturation: {
            s32Ret = MAPI_VCAP_GetSaturation(VcapPipeHdl, (HI_MAPI_VCAP_SATURATION_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetSaturation fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Luma: {
            s32Ret = MAPI_VCAP_GetLuma(VcapPipeHdl, (HI_MAPI_VCAP_LUMA_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetLuma fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Sharpen: {
            s32Ret = MAPI_VCAP_GetSharpen(VcapPipeHdl, (HI_MAPI_VCAP_SHARPEN_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetSharpen fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_MeteringMode: {
            s32Ret = MAPI_VCAP_GetMeteringMode(VcapPipeHdl, (HI_MAPI_VCAP_METERINGMODE_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_GetMeteringMode fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_WDR: {
            s32Ret = HI_MPI_ISP_GetFSWDRAttr(ViPipe, (ISP_WDR_FS_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetFSWDRAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_PipeDiffAttr: {
            s32Ret = HI_MPI_ISP_GetPipeDifferAttr(ViPipe, (ISP_PIPE_DIFF_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPipeDifferAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_GetPubAttr: {
            s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, (ISP_PUB_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetPubAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_ISP_Cac: {
            s32Ret = HI_MPI_ISP_GetLocalCacAttr(ViPipe, (ISP_LOCAL_CAC_ATTR_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_ISP_GetLocalCacAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_CMD_PIPE_GetCmpParam: {
            s32Ret = HI_MPI_VI_GetPipeCmpParam(ViPipe, (VI_CMP_PARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetPipeCmpParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        default:
            break;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                   VI_CROP_INFO_S *pstCropInfo)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstCropInfo);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
                        "don't support operate crop when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                        VcapPipeHdl, PipeChnHdl);
    s32Ret = HI_MPI_VI_SetChnCrop(ViPipe, ViChn, pstCropInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_SetChnCrop failed,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetChnCropAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                   VI_CROP_INFO_S *pstCropInfo)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstCropInfo);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
                        "don't support operate crop when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
                        VcapPipeHdl, PipeChnHdl);
    s32Ret = HI_MPI_VI_GetChnCrop(ViPipe, ViChn, pstCropInfo);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnCrop failed,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;
    VI_DEV ViDev;
    VI_PIPE_ATTR_S stPipeAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobDevAttr[ViDev].stDevAttr.enWdrMode == WDR_MODE_NONE) {
        s32Ret = MAPI_VCAP_SetDumpRawAttr(VcapPipeHdl, pstDumpAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetDumpRawAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    } else {
        if ((HI_HANDLE)ViPipe != g_stVcapGlobAttr.astGlobDevAttr[ViDev].aPipeIdBind[0]) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "The pipe is not the master pipe, VcapPipeHdl[%d]\n", VcapPipeHdl);
            return HI_SUCCESS;
        }

        s32Ret = MAPI_VCAP_SetWdrDumpRawAttr(VcapPipeHdl, pstDumpAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetWdrDumpRawAttr fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
    }

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_VCAP_GetDumpRawAttr(HI_HANDLE VcapPipeHdl, HI_MAPI_DUMP_ATTR_S *pstDumpAttr)
{
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDumpAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    memcpy(pstDumpAttr, &g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].stDumpRawInfo.stVcapDumpAttr,
           sizeof(HI_MAPI_DUMP_ATTR_S));

    return HI_SUCCESS;
}
HI_S32 HI_MAPI_VCAP_StartDumpRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
                                 HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData)
{
    MAPI_UNUSED(VcapPipeHdl);
    MAPI_UNUSED(u32Count);
    MAPI_UNUSED(pstVCapRawData);
    // 1 TODO:single system you can add code at this place
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopDumpRaw(HI_HANDLE VcapPipeHdl)
{
    MAPI_UNUSED(VcapPipeHdl);
    // 1 TODO:single system you can add code at this place
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartDumpBNRRaw(HI_HANDLE VcapPipeHdl, HI_U32 u32Count,
                                    HI_MAPI_VCAP_RAW_DATA_S *pstVCapRawData)
{
    MAPI_UNUSED(VcapPipeHdl);
    MAPI_UNUSED(u32Count);
    MAPI_UNUSED(pstVCapRawData);
    // 1 TODO:single system you can add code at this place
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopDumpBNRRaw(HI_HANDLE VcapPipeHdl)
{
    MAPI_UNUSED(VcapPipeHdl);
    // 1 TODO:single system you can add code at this place
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
                                  HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;
    HI_HANDLE RgnHdl;
    RGN_CHN_ATTR_S stRgnChnAttr;
    BITMAP_S stBitMap;
    MPP_CHN_S stChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_MIN_VAL("stBitmapAttr.enPixelFormat", pstOSDAttr->stBitmapAttr.enPixelFormat,
                            PIXEL_FORMAT_ARGB_1555);
    CHECK_MAPI_VCAP_MAX_VAL("stBitmapAttr.enPixelFormat", pstOSDAttr->stBitmapAttr.enPixelFormat,
                            PIXEL_FORMAT_ARGB_8888);
    CHECK_MAPI_VCAP_NULL_PTR(pstOSDAttr->stBitmapAttr.pData);
    CHECK_MAPI_VCAP_MIN_VAL("stBitmapAttr.u32Width", pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_MIN_OSD_BMP_W);
    CHECK_MAPI_VCAP_MIN_VAL("stBitmapAttr.u32Height", pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_MIN_OSD_BMP_H);
    CHECK_MAPI_VCAP_MAX_VAL("stBitmapAttr.u32Width", pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_MAX_OSD_BMP_W);
    CHECK_MAPI_VCAP_MAX_VAL("stBitmapAttr.u32Height", pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_MAX_OSD_BMP_H);
    CHECK_MAPI_VCAP_MAX_VAL("stOsdDisplayAttr.u32Albgha", pstOSDAttr->stOsdDisplayAttr.u32BgAlpha,
                            HI_MAPI_MAX_OSD_ALPHA);
    CHECK_MAPI_VCAP_MAX_VAL("stOsdDisplayAttr.u32Alfgha", pstOSDAttr->stOsdDisplayAttr.u32FgAlpha,
                            HI_MAPI_MAX_OSD_ALPHA);
    CHECK_MAPI_VCAP_MIN_VAL("stOsdDisplayAttr.s32RegionX", pstOSDAttr->stOsdDisplayAttr.s32RegionX,
                            HI_MAPI_MIN_OSD_REGION_X);
    CHECK_MAPI_VCAP_MIN_VAL("stOsdDisplayAttr.s32RegionY", pstOSDAttr->stOsdDisplayAttr.s32RegionY,
                            HI_MAPI_MIN_OSD_REGION_Y);
    CHECK_MAPI_VCAP_MAX_VAL("stOsdDisplayAttr.s32RegionX", pstOSDAttr->stOsdDisplayAttr.s32RegionX,
                            HI_MAPI_MAX_OSD_REGION_X);
    CHECK_MAPI_VCAP_MAX_VAL("stOsdDisplayAttr.s32RegionY", pstOSDAttr->stOsdDisplayAttr.s32RegionY,
                            HI_MAPI_MAX_OSD_REGION_Y);
    CHECK_MAPI_VCAP_MAX_VAL("u32CanvasNum", pstOSDAttr->u32CanvasNum, HI_MAPI_RGN_BUF_MAX_NUM);
    CHECK_MAPI_VCAP_MIN_VAL("u32CanvasNum", pstOSDAttr->u32CanvasNum, HI_MAPI_RGN_BUF_MIN_NUM);
    CHECK_MEDIA_VCAP_ALIGN(pstOSDAttr->stBitmapAttr.u32Width, HI_MAPI_OSD_BMP_W_ALIGN);
    CHECK_MEDIA_VCAP_ALIGN(pstOSDAttr->stBitmapAttr.u32Height, HI_MAPI_OSD_BMP_H_ALIGN);
    CHECK_MEDIA_VCAP_ALIGN(pstOSDAttr->stOsdDisplayAttr.s32RegionX, HI_MAPI_OSD_REGION_X_ALIGN);
    CHECK_MEDIA_VCAP_ALIGN(pstOSDAttr->stOsdDisplayAttr.s32RegionY, HI_MAPI_OSD_REGION_Y_ALIGN);

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support OSD when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n",
        VcapPipeHdl, PipeChnHdl);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus ==
        VCAP_STATUS_RUNNING) {
        RgnHdl = MAPI_GetRgnHdl(ViPipe, ViChn, OSDHdl, HI_ID_VI);

        stChn.enModId = HI_ID_VI;
        stChn.s32DevId = ViPipe;
        stChn.s32ChnId = ViChn;

        s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_GetDisplayAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);

        stRgnChnAttr.bShow = pstOSDAttr->stOsdDisplayAttr.bShow;

        if (stRgnChnAttr.enType != OVERLAYEX_RGN) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "wrong rgn type \n");
            return HI_MAPI_VCAP_ENOTSUPPORT;
        }

        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pstOSDAttr->stOsdDisplayAttr.s32RegionX;
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pstOSDAttr->stOsdDisplayAttr.s32RegionY;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstOSDAttr->stOsdDisplayAttr.u32BgAlpha;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstOSDAttr->stOsdDisplayAttr.u32FgAlpha;

        s32Ret = HI_MPI_RGN_SetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_SetDisplayAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);

        stBitMap.pData = pstOSDAttr->stBitmapAttr.pData;
        stBitMap.u32Width = pstOSDAttr->stBitmapAttr.u32Width;
        stBitMap.u32Height = pstOSDAttr->stBitmapAttr.u32Height;
        stBitMap.enPixelFormat = pstOSDAttr->stBitmapAttr.enPixelFormat;

        s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &stBitMap);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_SetBitMap fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);

    } else {
        g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus =
            VCAP_STATUS_INIT;
    }

    memcpy(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].stOSDAttr,
           pstOSDAttr, sizeof(HI_MAPI_OSD_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetChnOSDAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl,
                                  HI_MAPI_OSD_ATTR_S *pstOSDAttr)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_PIPE ViDev;
    VI_CHN ViChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstOSDAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support OSD when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
        PipeChnHdl);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus ==
        VCAP_STATUS_NOTINIT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "OSD not been config attr yet\n");
        return HI_MAPI_VCAP_ENOT_CONFIG;
    }

    memcpy(pstOSDAttr,
           &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].stOSDAttr,
           sizeof(HI_MAPI_OSD_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StartChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV BindDevHdl;
    HI_HANDLE RgnHdl;
    RGN_CHN_ATTR_S stRgnChnAttr;
    BITMAP_S stBitmap;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    BindDevHdl = g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support OSD when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
        PipeChnHdl);

    if ((HI_HANDLE)BindDevHdl > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[BindDevHdl].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n",
                       BindDevHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus ==
        VCAP_STATUS_NOTINIT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "OSD not been config attr yet\n");
        return HI_MAPI_VCAP_ENOT_CONFIG;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus ==
        VCAP_STATUS_RUNNING) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "OSD[%d]is already been started\n", OSDHdl);
        return HI_SUCCESS;
    }

    HI_MAPI_OSD_ATTR_S *pstOSDAttr;

    pstOSDAttr = &g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].stOSDAttr;

    stRgnAttr.enType = OVERLAYEX_RGN;
    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = pstOSDAttr->stBitmapAttr.enPixelFormat;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = pstOSDAttr->stOsdDisplayAttr.u32Color;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width = pstOSDAttr->stBitmapAttr.u32Width;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = pstOSDAttr->stBitmapAttr.u32Height;
    stRgnAttr.unAttr.stOverlayEx.u32CanvasNum = pstOSDAttr->u32CanvasNum;

    s32Ret = MAPI_AddRgnHdl(ViPipe, ViChn, OSDHdl, HI_ID_VI);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_AddRgnHdl fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl, PipeChnHdl);
    RgnHdl = MAPI_GetRgnHdl(ViPipe, ViChn, OSDHdl, HI_ID_VI);

    s32Ret = HI_MPI_RGN_Create(RgnHdl, &stRgnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_Create fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    memcpy(&stBitmap, &pstOSDAttr->stBitmapAttr, sizeof(BITMAP_S));

    s32Ret = HI_MPI_RGN_SetBitMap(RgnHdl, &stBitmap);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_SetBitMap fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    stChn.enModId = HI_ID_VI;
    stChn.s32DevId = ViPipe;
    stChn.s32ChnId = ViChn;

    stRgnChnAttr.bShow = pstOSDAttr->stOsdDisplayAttr.bShow;
    stRgnChnAttr.enType = OVERLAYEX_RGN;
    stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pstOSDAttr->stOsdDisplayAttr.s32RegionX;
    stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pstOSDAttr->stOsdDisplayAttr.s32RegionY;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = pstOSDAttr->stOsdDisplayAttr.u32BgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = pstOSDAttr->stOsdDisplayAttr.u32FgAlpha;
    stRgnChnAttr.unChnAttr.stOverlayExChn.u32Layer = OSDHdl;

    s32Ret = HI_MPI_RGN_AttachToChn(RgnHdl, &stChn, &stRgnChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_AttachToChn fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus =
        VCAP_STATUS_RUNNING;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_StopChnOSD(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl, HI_HANDLE OSDHdl)
{
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_PIPE ViDev;
    VI_CHN ViChn;
    HI_HANDLE RgnHdl;
    MPP_CHN_S stChn;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("OSDHdl", OSDHdl, (HI_MAPI_VCAP_OSD_MAX_NUM - 1));
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support OSD when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
        PipeChnHdl);

    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus ==
        VCAP_STATUS_NOTINIT) {
        MAPI_WARN_TRACE(HI_MAPI_MOD_VCAP, "OSD[%d]is already been stopped\n", OSDHdl);
        return HI_SUCCESS;
    }

    stChn.enModId = HI_ID_VI;
    stChn.s32DevId = ViPipe;
    stChn.s32ChnId = ViChn;

    RgnHdl = MAPI_GetRgnHdl(ViPipe, ViChn, OSDHdl, HI_ID_VI);

    s32Ret = MAPI_DelRgnHdl(ViPipe, ViChn, OSDHdl, HI_ID_VI);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_DelRgnHdl fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl, PipeChnHdl);

    s32Ret = HI_MPI_RGN_DetachFromChn(RgnHdl, &stChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_DetachFromChn fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    s32Ret = HI_MPI_RGN_Destroy(RgnHdl);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_RGN_Destroy fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    memset(&g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].stOSDAttr, 0,
           sizeof(HI_MAPI_OSD_ATTR_S));
    g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].astVcapOSDAttr[OSDHdl].enOSDStatus =
        VCAP_STATUS_NOTINIT;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
                                  HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bIspRun, "please start ISP first",
                         HI_MAPI_VCAP_ENOT_PERM);

    switch (enCMD) {
        case HI_MAPI_VCAP_SNAP_CMD_BNR: {
            s32Ret = HI_MPI_SNAP_SetProBNRParam(ViPipe, (ISP_PRO_BNR_PARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_SetProBNRParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_SNAP_CMD_Sharpen: {
            s32Ret = HI_MPI_SNAP_SetProSharpenParam(ViPipe, (ISP_PRO_SHARPEN_PARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_SetProSharpenParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "cmd illegal %d\n", enCMD);
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_GetSnapAttrEx(HI_HANDLE VcapPipeHdl, HI_MAPI_VCAP_SNAP_CMD_E enCMD, HI_VOID *pAttr,
                                  HI_U32 u32Len)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE ViPipe;

    ViPipe = (VI_PIPE)VcapPipeHdl;

    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pAttr);
    CHECK_MAPI_VCAP_ZERO_VAL("u32Len", u32Len);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bPipeCreate, "please start vcap device first",
                         HI_MAPI_VCAP_ENOT_PERM);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.astGlobPipeAttr[VcapPipeHdl].bIspRun, "please start ISP first",
                         HI_MAPI_VCAP_ENOT_PERM);

    switch (enCMD) {
        case HI_MAPI_VCAP_SNAP_CMD_BNR: {
            s32Ret = HI_MPI_SNAP_GetProBNRParam(ViPipe, (ISP_PRO_BNR_PARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_GetProBNRParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        case HI_MAPI_VCAP_SNAP_CMD_Sharpen: {
            s32Ret = HI_MPI_SNAP_GetProSharpenParam(ViPipe, (ISP_PRO_SHARPEN_PARAM_S *)pAttr);
            CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_SNAP_GetProSharpenParam fail,VcapPipeHdl[%d]\n", VcapPipeHdl);
            break;
        }

        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "cmd illegal %d\n", enCMD);
            return HI_MAPI_VCAP_EILLEGAL_PARA;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_SetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                  HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
#ifdef SUPPORT_DIS
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;
    HI_FLOAT f32FrameRate = 0.0f;
    HI_S32 i = 0;
    HI_S32 s32DisCfgSeq = 0;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    /* 1.check param */
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDisAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 2.check vpss mode and PIPE type */
    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support DIS when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
        PipeChnHdl);

    /* 3.check PIPE type */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "don't support DIS when the PIPE work mode is snap\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    /* 4.check dev status */
    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* 5.check chn status */
    if (!g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].astPipeChnAttr[ViChn].bChnEnable) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must enable chn[%d] first\n", PipeChnHdl);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    /* 6.enable and disable dis status */
    if (pstDisAttr->stDisAttr.bEnable == HI_TRUE) {
        /* Set dis attr and enable dis */
        s32Ret = MAPI_VCAP_EnableChnDIS(VcapPipeHdl, PipeChnHdl, pstDisAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "enable dis fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl, PipeChnHdl);
    } else {
        /* Disable dis */
        s32Ret = MAPI_VCAP_DisableChnDIS(VcapPipeHdl, PipeChnHdl, pstDisAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "disable dis fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl, PipeChnHdl);
    }

    return HI_SUCCESS;
#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support dis.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}

HI_S32 HI_MAPI_VCAP_GetChnDISAttr(HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                                  HI_MAPI_VCAP_DIS_ATTR_S *pstDisAttr)
{
#ifdef SUPPORT_DIS
    HI_S32 s32Ret;
    VI_PIPE ViPipe;
    VI_CHN ViChn;
    VI_DEV ViDev;
    DIS_CONFIG_S stDISConfig;
    DIS_ATTR_S stDISAttr;
    VI_LDCV2_ATTR_S stLDCV2Attr;

    ViPipe = (VI_PIPE)VcapPipeHdl;
    ViChn = (VI_CHN)PipeChnHdl;
    ViDev = (VI_DEV)g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].BindDevHdl;

    /* 1.check param */
    CHECK_MAPI_VCAP_HANDLE("VcapPipeHdl", VcapPipeHdl, (HI_MAPI_VCAP_MAX_PIPE_NUM - 1));
    CHECK_MAPI_VCAP_HANDLE("PipeChnHdl", PipeChnHdl, (HI_MAPI_PIPE_MAX_CHN_NUM - 1));
    CHECK_MAPI_VCAP_NULL_PTR(pstDisAttr);
    CHECK_MAPI_VCAP_TRUE(g_stVcapGlobAttr.bVcapInited, "media has not been inited", HI_MAPI_VCAP_ENOT_INITED);

    /* 2.check vpss mode and PIPE type */
    s32Ret = MAPI_VCAP_CheckViVpssWorkMode(VcapPipeHdl);
    CHECK_MAPI_VCAP_RET(s32Ret,
        "don't support DIS when the vpss work mode is online or parallel,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
        PipeChnHdl);

    /* 3.check PIPE type */
    if (g_stVcapGlobAttr.astGlobPipeAttr[ViPipe].stPipeAttr.enPipeType == HI_MAPI_PIPE_TYPE_SNAP) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "don't support DIS when the PIPE work mode is snap\n");
        return HI_MAPI_VCAP_ENOTSUPPORT;
    }

    /* 4.check status */
    if ((HI_HANDLE)ViDev > (HI_MAPI_VCAP_MAX_DEV_NUM - 1)
        || g_stVcapGlobAttr.astGlobDevAttr[ViDev].enDevStatus != VCAP_STATUS_RUNNING) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "dot not permit,you must set Vcap Attrs and start dev[%d] first\n", ViDev);
        return HI_MAPI_VCAP_ENOT_PERM;
    }

    s32Ret = HI_MPI_VI_GetChnDISConfig(ViPipe, ViChn, &stDISConfig);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnDISConfig fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);

    memcpy(&pstDisAttr->stDISConfig, &stDISConfig, sizeof(DIS_CONFIG_S));

    memset(&stDISAttr, 0, sizeof(DIS_ATTR_S));
    s32Ret = HI_MPI_VI_GetChnDISAttr(ViPipe, ViChn, &stDISAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnDISAttr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                        PipeChnHdl);
    memcpy(&pstDisAttr->stDisAttr, &stDISAttr, sizeof(DIS_ATTR_S));

#ifdef SUPPORT_GYRO
    if (MAPI_VCAP_GetMotionSensorStatus() == HI_TRUE) {
        memset(&stLDCV2Attr, 0, sizeof(VI_LDCV2_ATTR_S));

        s32Ret = HI_MPI_VI_GetChnLDCV2Attr(ViPipe, ViChn, &stLDCV2Attr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_VI_GetChnLDCV2Attr fail,VcapPipeHdl[%d],PipeChnHdl[%d]\n", VcapPipeHdl,
                            PipeChnHdl);

        memcpy(&pstDisAttr->stLDCV2Attr, &stLDCV2Attr, sizeof(VI_LDCV2_ATTR_S));
    }
#endif

    return HI_SUCCESS;

#else
    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support dis.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}
