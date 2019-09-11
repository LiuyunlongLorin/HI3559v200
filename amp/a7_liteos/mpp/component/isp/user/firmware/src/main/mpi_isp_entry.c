/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_isp_entry.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2011/01/05
  Description   :
  History       :
  1.Date        : 2011/01/05
    Author      :
    Modification: Created file

******************************************************************************/

#include <stdio.h>
#include <string.h>
#if 0
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <sys/ioctl.h>
#include <signal.h>

#include "mpi_sys.h"
#include "mkp_isp.h"
#include "hi_isp_debug.h"
#include "isp_debug.h"
#include "isp_defaults.h"
#include "isp_main.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_statistics.h"
#include "isp_regcfg.h"
#include "isp_proc.h"
#include "isp_vreg.h"
#include "isp_config.h"
#include "isp_ext_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* Isp Version Proc will be Hi3518_ISP_V1.0.4.x, ISP_LIB_VERSION stands for x */
#define ISP_LIB_VERSION "0"    //[0~F]

/****************************************************************************
 * GLOBAL VARIABLES                                                         *
 ****************************************************************************/

ISP_CTX_S  g_astIspCtx[ISP_MAX_PIPE_NUM] = {{0}};
HI_S32     g_as32IspFd[ISP_MAX_PIPE_NUM] = {[0 ... (ISP_MAX_PIPE_NUM - 1)] = -1};

HI_U16     g_au16ResWMax[ISP_MAX_PIPE_NUM] = {ISP_RES_WIDTH_MAX};
HI_U16     g_au16ResHMax[ISP_MAX_PIPE_NUM] = {ISP_RES_HEIGHT_MAX};

#ifdef _MSC_VER
#define MUTEX_INIT_LOCK(mutex) InitializeCriticalSection(&mutex)
#define MUTEX_LOCK(mutex) EnterCriticalSection(&mutex)
#define MUTEX_UNLOCK(mutex) LeaveCriticalSection(&mutex)
#define MUTEX_DESTROY(mutex) DeleteCriticalSection(&mutex)
#else
#define MUTEX_INIT_LOCK(mutex) \
    do \
    { \
        (void)pthread_mutex_init(&mutex, NULL); \
    }while(0)
#define MUTEX_LOCK(mutex) \
    do \
    { \
        (void)pthread_mutex_lock(&mutex); \
    }while(0)
#define MUTEX_UNLOCK(mutex) \
    do \
    { \
        (void)pthread_mutex_unlock(&mutex); \
    }while(0)
#define MUTEX_DESTROY(mutex) \
    do \
    { \
        (void)pthread_mutex_destroy(&mutex); \
    }while(0)
#endif

static ISP_VERSION_S gs_stVersion =
{
    .au8MppVersion = ISP_LIB_VERSION,
    .au8Date = __DATE__,
    .au8Time = __TIME__,
    .u32Magic = 0,
};

/****************************************************************************
 * MACRO DEFINITION                                                         *
 ****************************************************************************/

#define ISP_CHECK_SENSOR_REGISTER(dev)\
    do {\
        if ((g_astIspCtx[dev].bSnsReg != HI_TRUE) && (g_astIspCtx[dev].bISPYuvMode != HI_TRUE))\
        {\
            ISP_TRACE(HI_DBG_ERR, "Sensor doesn't register to ISP[%d]!\n", dev);\
            return HI_ERR_ISP_SNS_UNREGISTER;\
        }\
    }while(0)

#define ISP_VERSION_MAGIC    20130305

/****************************************************************************
 * static functions
 ****************************************************************************/

HI_S32 ISP_CheckDevOpen(ISP_DEV dev)
{
    if (g_as32IspFd[dev] <= 0)
    {
        HI_U32 arg = (dev);
        g_as32IspFd[dev] = open("/dev/isp_dev", O_RDONLY, S_IRUSR);
        if (g_as32IspFd[dev] < 0)
        {
            perror("open isp device error!\n");
            return HI_ERR_ISP_NOT_INIT;
        }
        if (ioctl(g_as32IspFd[dev], ISP_DEV_SET_FD, &arg))
        {
            close(g_as32IspFd[dev]);
            g_as32IspFd[dev] = -1;
            return HI_ERR_ISP_NOT_INIT;
        }
    }

    return HI_SUCCESS;
}

HI_S32 ISP_CheckMemInit(ISP_DEV dev)
{
    if (g_astIspCtx[dev].bMemInit != HI_TRUE)
    {
        if (ioctl(g_as32IspFd[dev], ISP_MEM_INFO_GET, &g_astIspCtx[dev].bMemInit))
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Mem info failed!\n", dev);
            return HI_ERR_ISP_MEM_NOT_INIT;
        }
        if (g_astIspCtx[dev].bMemInit != HI_TRUE)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Mem NOT Init %d!\n", dev, g_astIspCtx[dev].bMemInit);
            return HI_ERR_ISP_MEM_NOT_INIT;
        }
    }

    return HI_SUCCESS;
}

static HI_BOOL ISP_GetVersion(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;

    //ISP_CHECK_OPEN(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_VERSION, &gs_stVersion);
    if (s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "register ISP[%d] lib info ec %x!\n", ViPipe, s32Ret);
        return HI_FALSE;
    }

    /* disable isp magic version check */
#if 0
    if (gs_stVersion.u32Magic != ISP_VERSION_MAGIC)
    {
        ISP_TRACE(HI_DBG_ERR, "isp lib version doesn't match with sdk!\n");
        return HI_FALSE;
    }
#endif

    return HI_TRUE;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_SetModParam
 Description     : isp module parameter
 Input           : I_VOID  **
 Output          : None
 Return Value    :
 Process         :
 Note            :

  History
  1.Date         : 2018/2/3
    Author       :
    Modification : Created function

*****************************************************************************/

HI_S32 HI_MPI_ISP_SetModParam(const ISP_MOD_PARAM_S *pstModParam)
{
    ISP_CHECK_PIPE(0);
    ISP_CHECK_POINTER(pstModParam);
    ISP_CHECK_OPEN(0);

    return  ioctl(g_as32IspFd[0], ISP_SET_MOD_PARAM, pstModParam);
}

HI_S32 HI_MPI_ISP_GetModParam(ISP_MOD_PARAM_S *pstModParam)
{
    ISP_CHECK_PIPE(0);
    ISP_CHECK_POINTER(pstModParam);
    ISP_CHECK_OPEN(0);

    return ioctl(g_as32IspFd[0], ISP_GET_MOD_PARAM, pstModParam);
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_MemInit
 Description     : isp initial extent memory
 Input           :
 Output          :
 Return Value    :
 Process         :
 Note            :

  History
  1.Date         : 2017/8/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_MemInit(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_WORKING_MODE_S stIspWorkMode;

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    if (ioctl(g_as32IspFd[ViPipe], ISP_MEM_INFO_GET, &pstIspCtx->bMemInit))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Mem info failed!\n", ViPipe);
        return HI_ERR_ISP_MEM_NOT_INIT;
    }

    if (HI_TRUE == pstIspCtx->bMemInit)
    {
        s32Ret = ISP_Exit(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Exit failed!\n", ViPipe);
            return s32Ret;
        }
    }

    /* WDR attribute */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_WDR_ATTR, &pstIspCtx->stWdrAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get WDR attr failed\n", ViPipe);
        return s32Ret;
    }

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    /* Get pipe size */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_PIPE_SIZE, &pstIspCtx->stPipeSize);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Pipe size failed\n", ViPipe);
        return s32Ret;
    }

    /* HDR attribute */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_HDR_ATTR, &pstIspCtx->stHdrAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get HDR attr failed\n", ViPipe);
        return s32Ret;
    }

    /* Stitch attribute */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_STITCH_ATTR, &pstIspCtx->stStitchAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Stitch attr failed\n", ViPipe);
        return s32Ret;
    }

#ifdef CONFIG_HI_VI_MONO_COLOR_FUSION
    /* Fusion attribute */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_FUSION_ATTR, &pstIspCtx->stFusionAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get Fusion attr failed\n", ViPipe);
        return s32Ret;
    }
#endif

    /* p2en info */
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_P2EN_INFO_GET, &pstIspCtx->bIsp0P2En);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] get p2en info failed\n", ViPipe);
        return s32Ret;
    }

    if (HI_TRUE != ISP_GetVersion(ViPipe))
    {
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* Creat extern registers */
    s32Ret = VReg_Init(ViPipe, ISP_VIR_REG_BASE(ViPipe), ISP_VREG_SIZE);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init Mem failed\n", ViPipe);
        return s32Ret;
    }

    /* Init block attribute */
    s32Ret = ISP_BlockInit(ViPipe, &pstIspCtx->stBlockAttr);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init block failed!\n", ViPipe);
        goto Fail0;
    }

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] work mode get failed!\n", ViPipe);
        goto Fail0;
    }

    if(ISP_MODE_BT1120_YUV == stIspWorkMode.enIspDataInputMode)
    {
       pstIspCtx->bISPYuvMode = HI_TRUE;
       s32Ret = ISP_SensorCtxInit(ViPipe);
       if (HI_SUCCESS != s32Ret)
       {
            //return s32Ret;
            goto Fail0;
       }
    }
    else if(ISP_MODE_RAW == stIspWorkMode.enIspDataInputMode)
    {
       pstIspCtx->bISPYuvMode = HI_FALSE;
    }

    if ((g_astIspCtx[ViPipe].bSnsReg != HI_TRUE) && (g_astIspCtx[ViPipe].bISPYuvMode != HI_TRUE))
    {
        ISP_TRACE(HI_DBG_ERR, "Sensor doesn't register to ISP[%d]!\n", ViPipe);
        s32Ret = HI_ERR_ISP_SNS_UNREGISTER;
        goto Fail1;
    }


    memset(&pstIspCtx->stIspParaRec, 0, sizeof(ISP_PARA_REC_S));
    hi_ext_top_wdr_cfg_write(ViPipe, pstIspCtx->stIspParaRec.bWDRCfg);
    hi_ext_top_pub_attr_cfg_write(ViPipe, pstIspCtx->stIspParaRec.bPubCfg);

    hi_ext_top_wdr_switch_write(ViPipe, HI_FALSE);
    hi_ext_top_res_switch_write(ViPipe, HI_FALSE);

    pstIspCtx->bMemInit = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_MEM_INFO_SET, &pstIspCtx->bMemInit);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->bMemInit = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set Mem info failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail1;
    }

    return HI_SUCCESS;
Fail1:
    ISP_SensorUnRegCallBack(ViPipe);
Fail0:
    VReg_Exit(ViPipe, ISP_VIR_REG_BASE(ViPipe), ISP_VREG_SIZE);
    return s32Ret;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_Init
 Description     : isp initial process, include extent memory, top structure,
                    default value, etc.
 Input           :
 Output          :
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2017/1/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_ISP_Init(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_VOID *pRegCfg = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_U8 u8WDRMode;
    HI_U32 u32Value = 0;
    const HI_U64 u64Handsignal = ISP_INIT_HAND_SIGNAL;
    HI_VOID *pValue = HI_NULL;
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsImageMode;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;
    ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = NULL;
    ISP_MOD_PARAM_S stModParam = {0};

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_SENSOR_REGISTER(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    pstIspCtx->stIspParaRec.bWDRCfg = hi_ext_top_wdr_cfg_read(ViPipe);
    ISP_CHECK_ISP_WDR_CFG(ViPipe);

    pstIspCtx->stIspParaRec.bPubCfg = hi_ext_top_pub_attr_cfg_read(ViPipe);
    ISP_CHECK_ISP_PUB_ATTR_CFG(ViPipe);

    pstIspCtx->stSysRect.s32X      = hi_ext_system_corp_pos_x_read(ViPipe);
    pstIspCtx->stSysRect.s32Y      = hi_ext_system_corp_pos_y_read(ViPipe);
    pstIspCtx->stSysRect.u32Width  = hi_ext_sync_total_width_read(ViPipe);
    pstIspCtx->stSysRect.u32Height = hi_ext_sync_total_height_read(ViPipe);

    if (HI_TRUE == pstIspCtx->stIspParaRec.bInit)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Init failed!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    s32Ret = ISP_CfgBeBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail00;
    }

    s32Ret = ISP_GetBeBufFirst(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_BeVregAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_SttBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_SttAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail02;
    }

    s32Ret = ISP_ClutBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail02;
    }

    s32Ret = ISP_SpecAwbBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail03;
    }

    /* init statistics bufs. */
    s32Ret = ISP_StatisticsInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail04;
    }

    /* init proc bufs. */
    s32Ret = ISP_ProcInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail05;
    }

    /* init trans info bufs. */
    s32Ret = ISP_TransInfoInit(ViPipe, &pstIspCtx->stIspTransInfo);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail06;
    }

    s32Ret = ISP_UpdateInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail07;
    }

    s32Ret = ISP_FrameInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail08;
    }

    s32Ret = ISP_AttachInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail09;
    }

    s32Ret = ISP_ColorGamutInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail10;
    }

    s32Ret = ISP_DngInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail11;
    }

    s32Ret = ISP_ProInfoInit(ViPipe, &pstIspCtx->stIspProInfo);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail12;
    }

    s32Ret = ISP_ProNrParamInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail13;
    }

    s32Ret = ISP_ProShpParamInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail14;
    }

    /* regcfg init */
    s32Ret = ISP_RegCfgInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail15;
    }

    /* set sensor wdr mode */
    u8WDRMode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    s32Ret    = ISP_SensorSetWDRMode(ViPipe, u8WDRMode);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor WDR mode failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail16;
    }

    stSnsImageMode.u16Width  = hi_ext_top_sensor_width_read(ViPipe);
    stSnsImageMode.u16Height = hi_ext_top_sensor_height_read(ViPipe);
    u32Value = hi_ext_system_fps_base_read(ViPipe);
    pValue   = (HI_VOID *)&u32Value;
    stSnsImageMode.f32Fps    = *(HI_FLOAT *)pValue;
    stSnsImageMode.u8SnsMode = hi_ext_system_sensor_mode_read(ViPipe);

    s32Ret = ISP_SensorSetImageMode(ViPipe, &stSnsImageMode);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set sensor image mode failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail16;
    }

    s32Ret = ISP_SensorUpdateAll(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] update sensor all failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail16;
    }

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorID   = pstSnsDft->stSensorMode.u32SensorID;
    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorMode = pstSnsDft->stSensorMode.u8SensorMode;

    /* Get Dng paramters from CMOS.c*/
    ISP_DngExtRegsInitialize(ViPipe, (ISP_DNG_COLORPARAM_S *)&pstSnsDft->stDngColorParam);
    hi_ext_system_dng_static_info_valid_write(ViPipe, pstSnsDft->stSensorMode.bValidDngRawFormat);

    if (HI_TRUE == pstSnsDft->stSensorMode.bValidDngRawFormat)
    {
        memcpy(&pstIspCtx->stDngInfoCtrl.pstIspDng->stDngRawFormat, \
               &pstSnsDft->stSensorMode.stDngRawFormat, sizeof(DNG_RAW_FORMAT_S));
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] DngInfo not initialized in Cmos.c!\n", ViPipe);
    }

    /* init the common part of extern registers and real registers */
    ISP_ExtRegsDefault(ViPipe);
    ISP_RegsDefault(ViPipe);
    ISP_ExtRegsInitialize(ViPipe);
    ISP_RegsInitialize(ViPipe);

    /* isp algs global variable initialize */
    ISP_GlobalInitialize(ViPipe);

    /* register all algorithm to isp, and init them. */
    ISP_AlgsRegister(ViPipe);

    /* get regcfg */
    ISP_GetRegCfgCtx(ViPipe, &pRegCfg);

    ISP_AlgsInit(pstIspCtx->astAlgs, ViPipe, pRegCfg);

    s32Ret = ISP_ModParamGet(&stModParam);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail17;
    }

    //if(HI_FALSE == stModParam.u32QuickStart)
    {
        s32Ret = ISP_Lut2SttApbReg(ViPipe);
        if (s32Ret != HI_SUCCESS) {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] init lut2stt reg failed!\n", ViPipe);
            s32Ret = HI_ERR_ISP_NOT_INIT;
            goto Fail17;
        }

        s32Ret = ISP_SensorInit(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] init sensor failed!\n", ViPipe);
            s32Ret = HI_ERR_ISP_NOT_INIT;
            goto Fail17;
        }
    }

    ISP_SensorGetSnsReg(ViPipe, &pstSnsRegsInfo);
    pstIspCtx->stLinkage.u8Cfg2ValidDelayMax = pstSnsRegsInfo->u8Cfg2ValidDelayMax;

    /* regcfg info set */
    s32Ret = ISP_RegCfgInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init regcfgs info failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* set WDR mode to kernel. */
    s32Ret = ISP_WDRCfgSet(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set WDR mode to kernel failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    pstIspCtx->stIspParaRec.bStitchSync = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* init isp be cfgs all buffer */
    s32Ret = ISP_AllCfgsBeBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init Becfgs buffer failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* init isp global variables */
    MUTEX_INIT_LOCK(pstIspCtx->stLock);
    MUTEX_LOCK(pstIspCtx->stLock);

    pstIspCtx->stIspParaRec.bInit = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_INIT_INFO_SET, &pstIspCtx->stIspParaRec.bInit);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stIspParaRec.bInit = HI_FALSE;
        MUTEX_UNLOCK(pstIspCtx->stLock);
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp init info failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail19;
    }
    pstIspCtx->stIspParaRec.bRunEn = HI_TRUE;

    /* set handshake signal */
    if (ioctl(g_as32IspFd[ViPipe], ISP_RUN_STATE_SET, &u64Handsignal))
    {
        s32Ret = HI_ERR_ISP_NOT_INIT;
        MUTEX_UNLOCK(pstIspCtx->stLock);
        goto Fail19;
    }

    MUTEX_UNLOCK(pstIspCtx->stLock);

    return HI_SUCCESS;

Fail19:
    MUTEX_DESTROY(pstIspCtx->stLock);
Fail18:
    if (HI_FALSE == stModParam.u32QuickStart)
    {
        ISP_SensorExit(ViPipe);
    }
Fail17:
    ISP_AlgsExit(ViPipe, pstIspCtx->astAlgs);
    ISP_AlgsUnRegister(ViPipe);
Fail16:
    ISP_RegCfgExit(ViPipe);
Fail15:
    ISP_ProShpParamExit(ViPipe);
Fail14:
    ISP_ProNrParamExit(ViPipe);
Fail13:
    ISP_ProInfoExit(ViPipe);
Fail12:
    ISP_DngInfoExit(ViPipe);
Fail11:
    ISP_ColorGamutInfoExit(ViPipe);
Fail10:
    ISP_AttachInfoExit(ViPipe);
Fail09:
    ISP_FrameInfoExit(ViPipe);
Fail08:
    ISP_UpdateInfoExit(ViPipe);
Fail07:
    ISP_TransInfoExit(ViPipe);
Fail06:
    ISP_ProcExit(ViPipe);
Fail05:
    ISP_StatisticsExit(ViPipe);
Fail04:
    ISP_SpecAwbBufExit(ViPipe);
Fail03:
    ISP_ClutBufExit(ViPipe);
Fail02:
    ISP_SttBufExit(ViPipe);
Fail01:
    ISP_CfgBeBufExit(ViPipe);
Fail00:

    return s32Ret;
}

HI_S32 HI_ISP_YUV_Init(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_VOID *pRegCfg = HI_NULL;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    const HI_U64 u64Handsignal = ISP_INIT_HAND_SIGNAL;
    ISP_CMOS_DEFAULT_S *pstSnsDft = HI_NULL;

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);
    //ISP_CHECK_SENSOR_REGISTER(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    pstIspCtx->stIspParaRec.bWDRCfg = hi_ext_top_wdr_cfg_read(ViPipe);
    ISP_CHECK_ISP_WDR_CFG(ViPipe);

    pstIspCtx->stIspParaRec.bPubCfg = hi_ext_top_pub_attr_cfg_read(ViPipe);
    ISP_CHECK_ISP_PUB_ATTR_CFG(ViPipe);

    pstIspCtx->stSysRect.s32X      = hi_ext_system_corp_pos_x_read(ViPipe);
    pstIspCtx->stSysRect.s32Y      = hi_ext_system_corp_pos_y_read(ViPipe);
    pstIspCtx->stSysRect.u32Width  = hi_ext_sync_total_width_read(ViPipe);
    pstIspCtx->stSysRect.u32Height = hi_ext_sync_total_height_read(ViPipe);

    if (HI_TRUE == pstIspCtx->stIspParaRec.bInit)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Init failed!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    s32Ret = ISP_CfgBeBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail00;
    }

    s32Ret = ISP_GetBeBufFirst(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_BeVregAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_SttBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail01;
    }

    s32Ret = ISP_SttAddrInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail02;
    }
#if 0
    s32Ret = ISP_ClutBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail02;

    }

    s32Ret = ISP_SpecAwbBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail03;
    }
#endif
    /* init statistics bufs. */
    s32Ret = ISP_StatisticsInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        s32Ret = HI_ERR_ISP_MEM_NOT_INIT;
        goto Fail04;
    }

    /* init proc bufs. */
    s32Ret = ISP_ProcInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail05;
    }

    /* init trans info bufs. */
    s32Ret = ISP_TransInfoInit(ViPipe, &pstIspCtx->stIspTransInfo);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail06;
    }

    s32Ret = ISP_UpdateInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail07;
    }

    s32Ret = ISP_FrameInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail08;
    }

    s32Ret = ISP_AttachInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail09;
    }

    s32Ret = ISP_ColorGamutInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail10;
    }

    s32Ret = ISP_DngInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail11;
    }

    s32Ret = ISP_ProInfoInit(ViPipe, &pstIspCtx->stIspProInfo);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail12;
    }

    s32Ret = ISP_ProNrParamInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail13;
    }

    s32Ret = ISP_ProShpParamInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail14;
    }

    /* regcfg init */
    s32Ret = ISP_RegCfgInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail15;
    }

    s32Ret = ISP_SensorUpdateAll_YUV(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] update sensor all failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail16;
    }

    ISP_SensorGetDefault(ViPipe, &pstSnsDft);
    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorID = pstSnsDft->stSensorMode.u32SensorID;
    pstIspCtx->stFrameInfoCtrl.pstIspFrame->u32SensorMode = pstSnsDft->stSensorMode.u8SensorMode;

    /* Get Dng paramters from CMOS.c*/
    ISP_DngExtRegsInitialize(ViPipe, (ISP_DNG_COLORPARAM_S *)&pstSnsDft->stDngColorParam);
    hi_ext_system_dng_static_info_valid_write(ViPipe, pstSnsDft->stSensorMode.bValidDngRawFormat);

    if (HI_TRUE == pstSnsDft->stSensorMode.bValidDngRawFormat)
    {
        memcpy(&pstIspCtx->stDngInfoCtrl.pstIspDng->stDngRawFormat, \
               &pstSnsDft->stSensorMode.stDngRawFormat, sizeof(DNG_RAW_FORMAT_S));
    }
    else
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] DngInfo not initialized in Cmos.c!\n", ViPipe);
    }

    /* init the common part of extern registers and real registers */
    ISP_ExtRegsDefault(ViPipe);
    ISP_RegsDefault(ViPipe);
    ISP_ExtRegsInitialize(ViPipe);
    ISP_RegsInitialize(ViPipe);

    /* isp algs global variable initialize */
    ISP_GlobalInitialize(ViPipe);

    /* register all algorithm to isp, and init them. */
    ISP_YUVAlgsRegister(ViPipe);

    /* get regcfg */
    ISP_GetRegCfgCtx(ViPipe, &pRegCfg);

    ISP_AlgsInit(pstIspCtx->astAlgs, ViPipe, pRegCfg);

    pstIspCtx->stLinkage.u8Cfg2ValidDelayMax = 2;

    /* regcfg info set */
    s32Ret = ISP_RegCfgInfoInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init regcfgs info failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* set WDR mode to kernel. */
    s32Ret = ISP_WDRCfgSet(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set WDR mode to kernel failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    pstIspCtx->stIspParaRec.bStitchSync = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_SYNC_INIT_SET, &pstIspCtx->stIspParaRec.bStitchSync);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stIspParaRec.bStitchSync = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp stitch sync failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* init isp be cfgs all buffer */
    s32Ret = ISP_AllCfgsBeBufInit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] init Becfgs buffer failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail18;
    }

    /* init isp global variables */
    MUTEX_INIT_LOCK(pstIspCtx->stLock);
    MUTEX_LOCK(pstIspCtx->stLock);

    pstIspCtx->stIspParaRec.bInit = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_INIT_INFO_SET, &pstIspCtx->stIspParaRec.bInit);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stIspParaRec.bInit = HI_FALSE;
        MUTEX_UNLOCK(pstIspCtx->stLock);
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set isp init info failed!\n", ViPipe);
        s32Ret = HI_ERR_ISP_NOT_INIT;
        goto Fail19;
    }
    pstIspCtx->stIspParaRec.bRunEn = HI_TRUE;

    /* set handshake signal */
    if (ioctl(g_as32IspFd[ViPipe], ISP_RUN_STATE_SET, &u64Handsignal))
    {
        s32Ret = HI_ERR_ISP_NOT_INIT;
        MUTEX_UNLOCK(pstIspCtx->stLock);
        goto Fail19;
    }

    MUTEX_UNLOCK(pstIspCtx->stLock);

    return HI_SUCCESS;

Fail19:
    MUTEX_DESTROY(pstIspCtx->stLock);
Fail18:
    ISP_SensorExit(ViPipe);
//Fail17:
//    ISP_AlgsExit(ViPipe, pstIspCtx->astAlgs);
//    ISP_AlgsUnRegister(ViPipe);
Fail16:
    ISP_RegCfgExit(ViPipe);
Fail15:
    ISP_ProShpParamExit(ViPipe);
Fail14:
    ISP_ProNrParamExit(ViPipe);
Fail13:
    ISP_ProInfoExit(ViPipe);
Fail12:
    ISP_DngInfoExit(ViPipe);
Fail11:
    ISP_ColorGamutInfoExit(ViPipe);
Fail10:
    ISP_AttachInfoExit(ViPipe);
Fail09:
    ISP_FrameInfoExit(ViPipe);
Fail08:
    ISP_UpdateInfoExit(ViPipe);
Fail07:
    ISP_TransInfoExit(ViPipe);
Fail06:
    ISP_ProcExit(ViPipe);
Fail05:
    ISP_StatisticsExit(ViPipe);
Fail04:
    //ISP_SpecAwbBufExit(ViPipe);
//Fail03:
    //ISP_ClutBufExit(ViPipe);
Fail02:
    ISP_SttBufExit(ViPipe);
Fail01:
    ISP_CfgBeBufExit(ViPipe);
Fail00:

    return s32Ret;
}

HI_S32  HI_MPI_ISP_Init(VI_PIPE ViPipe)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_S32 s32Ret;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if (HI_FALSE == pstIspCtx->bISPYuvMode)
    {
        s32Ret = HI_ISP_Init(ViPipe);
    }
    else
    {
        s32Ret = HI_ISP_YUV_Init(ViPipe);
    }

    return s32Ret;
}

HI_S32  HI_ISP_YUV_RunOnce(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    HI_U32 u32WDRmode;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);

    ISP_CHECK_MEM_INIT(ViPipe);

    ISP_CHECK_ISP_INIT(ViPipe);

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    MUTEX_LOCK(pstIspCtx->stLock);

    if (HI_TRUE == pstIspCtx->stIspParaRec.bRun)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d]  is already running !\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* Sometimes HI_MPI_ISP_Run thread is not scheduled to run before calling HI_MPI_ISP_Exit. */
    if (HI_FALSE == pstIspCtx->stIspParaRec.bRunEn)
    {
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_SUCCESS;
    }

    pstIspCtx->stIspParaRec.bRun = HI_TRUE;

    u32WDRmode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    pstIspCtx->u8SnsWDRMode = u32WDRmode;

    pstIspCtx->stLinkage.bRunOnce = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_YUV_RUNONCE_INFO, &pstIspCtx->stLinkage.bRunOnce);
    if (HI_SUCCESS != s32Ret)
    {
        pstIspCtx->stLinkage.bRunOnce = HI_FALSE;
        pstIspCtx->stIspParaRec.bRun  = HI_FALSE;
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set runonce info failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return s32Ret;
    }

    ISP_Run(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_KERNEL_YUV_RUNONCE);

    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] kernel runonce  failed!\n", ViPipe);
        pstIspCtx->stIspParaRec.bRun  = HI_FALSE;
        pstIspCtx->stLinkage.bRunOnce = HI_FALSE;
        ioctl(g_as32IspFd[ViPipe], ISP_YUV_RUNONCE_INFO, &pstIspCtx->stLinkage.bRunOnce);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return s32Ret;
    }

    pstIspCtx->stIspParaRec.bRun = HI_FALSE;

    MUTEX_UNLOCK(pstIspCtx->stLock);

    return HI_SUCCESS;
}

/*When offline mode user send raw to BE, firstly need call this function to insure paramters ready*/
HI_S32 HI_ISP_RunOnce(VI_PIPE ViPipe)
{
    HI_S32 s32Ret;
    //HI_U32 u32WDRmode;
    ISP_RUNNING_MODE_E enIspRunMode;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_SENSOR_REGISTER(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    ISP_CHECK_ISP_INIT(ViPipe);

    /* Online mode not support */
    enIspRunMode = pstIspCtx->stBlockAttr.enIspRunningMode;
    if ((IS_ONLINE_MODE(enIspRunMode)) || \
        (IS_SIDEBYSIDE_MODE(enIspRunMode)))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] RunOnce not support for online!\n", ViPipe);
        return HI_ERR_ISP_NOT_SUPPORT;
    }

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    MUTEX_LOCK(pstIspCtx->stLock);

    if (HI_TRUE == pstIspCtx->stIspParaRec.bRun)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Run failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* Sometimes HI_MPI_ISP_Run thread is not scheduled to run before calling HI_MPI_ISP_Exit. */
    if (HI_FALSE == pstIspCtx->stIspParaRec.bRunEn)
    {
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_SUCCESS;
    }

    pstIspCtx->stIspParaRec.bRun = HI_TRUE;

#if 1
    /*change image mode (WDR mode or resolution)*/
    s32Ret = ISP_SwitchMode(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] switch mode failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return s32Ret;
    }
#else
    /* change  resolution  */
    ISP_SwitchImageMode(ViPipe);

    u32WDRmode = hi_ext_system_sensor_wdr_mode_read(ViPipe);
    /* swtich linear/WDR mode, width/height, fps  */
    if (pstIspCtx->u8SnsWDRMode != u32WDRmode)
    {
        pstIspCtx->u8SnsWDRMode = u32WDRmode;
        ISP_SwitchWDRMode(ViPipe);
    }
#endif
    pstIspCtx->stLinkage.bRunOnce = HI_TRUE;
    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_OPT_RUNONCE_INFO, &pstIspCtx->stLinkage.bRunOnce);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set runonce info failed!\n", ViPipe);
        goto Fail0;
    }

    ISP_Run(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_KERNEL_RUNONCE);
    if (HI_SUCCESS != s32Ret)
    {
        goto Fail0;
    }

    pstIspCtx->stIspParaRec.bRun = HI_FALSE;
    MUTEX_UNLOCK(pstIspCtx->stLock);

    return HI_SUCCESS;

Fail0:
    pstIspCtx->stLinkage.bRunOnce = HI_FALSE;
    if (ioctl(g_as32IspFd[ViPipe], ISP_OPT_RUNONCE_INFO, &pstIspCtx->stLinkage.bRunOnce))
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] set runonce info failed!\n", ViPipe);
    }
    pstIspCtx->stIspParaRec.bRun = HI_FALSE;
    MUTEX_UNLOCK(pstIspCtx->stLock);

    return s32Ret;
}

HI_S32 HI_MPI_ISP_RunOnce(VI_PIPE ViPipe)
{
    ISP_CTX_S* pstIspCtx = HI_NULL;
    HI_S32 s32Ret;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if (HI_FALSE == pstIspCtx->bISPYuvMode)
    {
        s32Ret = HI_ISP_RunOnce(ViPipe);
    }
    else
    {
        s32Ret = HI_ISP_YUV_RunOnce(ViPipe);
    }

    return s32Ret;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_Run
 Description     : isp firmware recurrent task, always run in a single thread.
 Input           : I_VOID  **
 Output          : None
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2011/1/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_Run(VI_PIPE ViPipe)
{
    HI_BOOL bEn;
    HI_S32 s32Ret;
    HI_U32 u32IntStatus = 0;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    /* check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_SENSOR_REGISTER(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    ISP_CHECK_ISP_INIT(ViPipe);

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    if (HI_TRUE == pstIspCtx->stIspParaRec.bRun)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Run failed!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    MUTEX_LOCK(pstIspCtx->stLock);

    /* Sometimes ISP run thread is not scheduled to run before calling ISP exit. */
    if (HI_FALSE == pstIspCtx->stIspParaRec.bRunEn)
    {
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_SUCCESS;
    }

    /* enable interrupt */
    bEn = HI_TRUE;
    if (ioctl(g_as32IspFd[ViPipe], ISP_SET_INT_ENABLE, &bEn) < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Enable ISP[%d] interrupt failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_FAILURE;
    }

    pstIspCtx->stIspParaRec.bRun = HI_TRUE;
    MUTEX_UNLOCK(pstIspCtx->stLock);

    while (1)
    {
        MUTEX_LOCK(pstIspCtx->stLock);

        if (HI_FALSE == pstIspCtx->stIspParaRec.bRunEn)
        {
            MUTEX_UNLOCK(pstIspCtx->stLock);
            break;
        }
#if 1
        /*change image mode (WDR mode or resolution)*/
        s32Ret = ISP_SwitchMode(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] switch mode failed!\n", ViPipe);
            MUTEX_UNLOCK(pstIspCtx->stLock);
            break;
        }
#else
        /* Change WDR mode  */
        s32Ret  = ISP_SwitchWDRMode(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] switch WDR mode failed!\n", ViPipe);
            MUTEX_UNLOCK(pstIspCtx->stLock);
            break;
        }

        /* Change resolution  */
        s32Ret = ISP_SwitchImageMode(ViPipe);
        if (HI_SUCCESS != s32Ret)
        {
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] switch image mode failed!\n", ViPipe);
            MUTEX_UNLOCK(pstIspCtx->stLock);
            break;
        }
#endif

        MUTEX_UNLOCK(pstIspCtx->stLock);

        {
            u32IntStatus = 0;
            /* waked up by the interrupt */
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_FRAME_EDGE, &u32IntStatus);
            if (HI_SUCCESS == s32Ret)
            {
                /* isp firmware calculate, include AE/AWB, etc. */
                if (ISP_1ST_INT & u32IntStatus)
                {
                    MUTEX_LOCK(pstIspCtx->stLock);

                    if (HI_FALSE == pstIspCtx->stIspParaRec.bRunEn)
                    {
                        MUTEX_UNLOCK(pstIspCtx->stLock);
                        break;
                    }

                    ISP_Run(ViPipe);

                    MUTEX_UNLOCK(pstIspCtx->stLock);
                }
            }
        }
    }

    /* disable interrupt */
    bEn = HI_FALSE;
    if (ioctl(g_as32IspFd[ViPipe], ISP_SET_INT_ENABLE, &bEn) < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Disable ISP[%d] interrupt failed!\n", ViPipe);
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_Exit
 Description     : control isp to exit recurrent task, always run in main process.
 Input           : I_VOID  **
 Output          : None
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2011/1/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_Exit(VI_PIPE ViPipe)
{
    HI_S32  s32Ret;
    HI_BOOL bEnable = HI_FALSE;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    /* Check status */
    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    //ISP_CHECK_OPEN(ViPipe);

    /* Wdr mode abnormal */
    if ((!pstIspCtx->stWdrAttr.bMastPipe) && \
        (IS_WDR_MODE(pstIspCtx->stWdrAttr.enWDRMode)))
    {
        return HI_SUCCESS;
    }

    ISP_StitchSyncExit(ViPipe);

    MUTEX_LOCK(pstIspCtx->stLock);

    ISP_AlgEnExit(ViPipe);

    if (ioctl(g_as32IspFd[ViPipe], ISP_SET_INT_ENABLE, &bEnable) < 0)
    {
        ISP_TRACE(HI_DBG_ERR, "Disable ISP[%d] interrupt failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    s32Ret = ISP_Exit(ViPipe);
    if (HI_SUCCESS != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Exit failed!\n", ViPipe);
        MUTEX_UNLOCK(pstIspCtx->stLock);
        return s32Ret;
    }

    ISP_LibsUnRegister(ViPipe);

    MUTEX_UNLOCK(pstIspCtx->stLock);
    MUTEX_DESTROY(pstIspCtx->stLock);

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_SetRegister
 Description     : set isp register, include extent memory.
 Input           : u32Addr   **
                   u32Value  **
 Output          : None
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2011/1/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_SetRegister(VI_PIPE ViPipe, HI_U32 u32Addr, HI_U32 u32Value)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_CHECK_PIPE(ViPipe);
    s32Ret = IO_WRITE32_Ex(u32Addr, u32Value);

    return s32Ret;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_GetRegister
 Description     : get isp register, include extent memory.
 Input           : u32Addr    **
                   pu32Value  **
 Output          : None
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2011/1/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_GetRegister(VI_PIPE ViPipe, HI_U32 u32Addr, HI_U32 *pu32Value)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pu32Value);
    s32Ret = IO_READ32_Ex(u32Addr, pu32Value);

    return s32Ret;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_SensorRegister
 Description     : sensor register callback function to isp firmware
 Input           : pstIspSensorRegister  **
 Output          : None
 Return Value    :
 Process         :
 Note             :

  History
  1.Date         : 2011/1/21
    Author       :
    Modification : Created function

*****************************************************************************/

HI_S32 HI_MPI_ISP_SetSnsSlaveAttr(SLAVE_DEV SlaveDev, const ISP_SLAVE_SNS_SYNC_S *pstSnsSync)
{
    SLAVE_CHECK_DEV(SlaveDev);
    ISP_CHECK_POINTER(pstSnsSync);

    hi_isp_slave_mode_time_cfg_select_write(SlaveDev, pstSnsSync->u32SlaveModeTime);
    hi_isp_slave_mode_configs_write(SlaveDev, pstSnsSync->unCfg.u32Bytes);
    hi_isp_slave_mode_vstime_low_write(SlaveDev, pstSnsSync->u32VsTime);
    hi_isp_slave_mode_vstime_high_write(SlaveDev, 0);
    hi_isp_slave_mode_hstime_write(SlaveDev, pstSnsSync->u32HsTime);
    hi_isp_slave_mode_vscyc_low_write(SlaveDev, pstSnsSync->u32VsCyc);
    hi_isp_slave_mode_vscyc_high_write(SlaveDev, 0);
    hi_isp_slave_mode_hscyc_write(SlaveDev, pstSnsSync->u32HsCyc);

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_GetSnsSlaveAttr(SLAVE_DEV SlaveDev, ISP_SLAVE_SNS_SYNC_S *pstSnsSync)
{
    SLAVE_CHECK_DEV(SlaveDev);
    ISP_CHECK_POINTER(pstSnsSync);

    pstSnsSync->u32SlaveModeTime = hi_isp_slave_mode_time_cfg_select_read(SlaveDev);
    pstSnsSync->unCfg.u32Bytes = hi_isp_slave_mode_configs_read(SlaveDev);
    pstSnsSync->u32VsTime = hi_isp_slave_mode_vstime_low_read(SlaveDev);
    pstSnsSync->u32HsTime = hi_isp_slave_mode_hstime_read(SlaveDev);
    pstSnsSync->u32VsCyc = hi_isp_slave_mode_vscyc_low_read(SlaveDev);
    pstSnsSync->u32HsCyc = hi_isp_slave_mode_hscyc_read(SlaveDev);

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_SensorRegCallBack(VI_PIPE ViPipe, ISP_SNS_ATTR_INFO_S *pstSnsAttrInfo, ISP_SENSOR_REGISTER_S *pstRegister)
{
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_POINTER(pstRegister);
    ISP_CHECK_POINTER(pstSnsAttrInfo);

    ISP_CHECK_POINTER(pstRegister->stSnsExp.pfn_cmos_sensor_init);
    ISP_CHECK_POINTER(pstRegister->stSnsExp.pfn_cmos_get_isp_default);
    ISP_CHECK_POINTER(pstRegister->stSnsExp.pfn_cmos_get_isp_black_level);
    ISP_CHECK_POINTER(pstRegister->stSnsExp.pfn_cmos_get_sns_reg_info);
    ISP_CHECK_POINTER(pstRegister->stSnsExp.pfn_cmos_set_pixel_detect);


    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    if (HI_TRUE == pstIspCtx->bSnsReg)
    {
        ISP_TRACE(HI_DBG_ERR, "Reg ERR! Sensor have registered to ISP[%d]!\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    s32Ret = ISP_SensorRegCallBack(ViPipe, pstSnsAttrInfo, pstRegister);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    pstIspCtx->stBindAttr.SensorId = pstSnsAttrInfo->eSensorId;
    pstIspCtx->bSnsReg = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_AELibRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib,
                                   ISP_AE_REGISTER_S *pstRegister)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_LIB_NODE_S *pstAeLibNode = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    /* check null point */
    ISP_CHECK_POINTER(pstAeLib);
    ISP_CHECK_POINTER(pstRegister);

    ISP_CHECK_POINTER(pstRegister->stAeExpFunc.pfn_ae_init);
    ISP_CHECK_POINTER(pstRegister->stAeExpFunc.pfn_ae_run);
    ISP_CHECK_POINTER(pstRegister->stAeExpFunc.pfn_ae_ctrl);
    ISP_CHECK_POINTER(pstRegister->stAeExpFunc.pfn_ae_exit);

    /* whether the lib have been registered */
    s32Ret = ISP_FindLib(pstIspCtx->stAeLibInfo.astLibs, pstAeLib);
    if (-1 != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Reg ERR! aelib have registered to ISP[%d].\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* whether can we register a new lib  */
    pstAeLibNode = ISP_SearchLib(pstIspCtx->stAeLibInfo.astLibs);
    if (HI_NULL == pstAeLibNode)
    {
        ISP_TRACE(HI_DBG_ERR, "can't register aelib to ISP[%d], there is too many libs.\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* record register info */
    memcpy(&pstAeLibNode->stAlgLib, pstAeLib, sizeof(ALG_LIB_S));
    memcpy(&pstAeLibNode->stAeRegsiter, pstRegister, sizeof(ISP_AE_REGISTER_S));
    pstAeLibNode->bUsed = HI_TRUE;

    /* set active lib */
    pstIspCtx->stAeLibInfo.u32ActiveLib = ISP_FindLib(pstIspCtx->stAeLibInfo.astLibs, pstAeLib);
    memcpy(&pstIspCtx->stBindAttr.stAeLib, pstAeLib, sizeof(ALG_LIB_S));

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_AWBLibRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAwbLib,
                                    ISP_AWB_REGISTER_S *pstRegister)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_LIB_NODE_S *pstAwbLibNode = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    /* check null point */
    ISP_CHECK_POINTER(pstAwbLib);
    ISP_CHECK_POINTER(pstRegister);

    ISP_CHECK_POINTER(pstRegister->stAwbExpFunc.pfn_awb_init);
    ISP_CHECK_POINTER(pstRegister->stAwbExpFunc.pfn_awb_run);
    ISP_CHECK_POINTER(pstRegister->stAwbExpFunc.pfn_awb_ctrl);
    ISP_CHECK_POINTER(pstRegister->stAwbExpFunc.pfn_awb_exit);

    /* whether the lib have been registered */
    s32Ret = ISP_FindLib(pstIspCtx->stAwbLibInfo.astLibs, pstAwbLib);
    if (-1 != s32Ret)
    {
        ISP_TRACE(HI_DBG_ERR, "Reg ERR! awblib have registered to ISP[%d].\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* whether can we register a new lib  */
    pstAwbLibNode = ISP_SearchLib(pstIspCtx->stAwbLibInfo.astLibs);
    if (HI_NULL == pstAwbLibNode)
    {
        ISP_TRACE(HI_DBG_ERR, "can't register awblib to ISP[%d], there is too many libs.\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* record register info */
    memcpy(&pstAwbLibNode->stAlgLib, pstAwbLib, sizeof(ALG_LIB_S));
    memcpy(&pstAwbLibNode->stAwbRegsiter, pstRegister, sizeof(ISP_AWB_REGISTER_S));
    pstAwbLibNode->bUsed = HI_TRUE;

    /* set active lib */
    pstIspCtx->stAwbLibInfo.u32ActiveLib = ISP_FindLib(pstIspCtx->stAwbLibInfo.astLibs, pstAwbLib);
    memcpy(&pstIspCtx->stBindAttr.stAwbLib, pstAwbLib, sizeof(ALG_LIB_S));

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_SensorUnRegCallBack(VI_PIPE ViPipe, SENSOR_ID SensorId)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_SENSOR_REGISTER(ViPipe);

    /* check sensor id */
    if (pstIspCtx->stBindAttr.SensorId != SensorId)
    {
        ISP_TRACE(HI_DBG_ERR, "UnReg ERR! ISP[%d] Registered sensor is %d, present sensor is %d.\n",
                  ViPipe, pstIspCtx->stBindAttr.SensorId, SensorId);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    pstIspCtx->stBindAttr.SensorId = 0;
    pstIspCtx->bSnsReg = HI_FALSE;

    ISP_SensorUnRegCallBack(ViPipe);

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_AELibUnRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib)
{
    HI_S32 s32SearchId;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    /* check null point */
    ISP_CHECK_POINTER(pstAeLib);

    s32SearchId = ISP_FindLib(pstIspCtx->stAeLibInfo.astLibs, pstAeLib);
    if (-1 == s32SearchId)
    {
        ISP_TRACE(HI_DBG_ERR, "can't find ae lib in ISP[%d].\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    memset(&pstIspCtx->stAeLibInfo.astLibs[s32SearchId], 0, sizeof(ISP_LIB_NODE_S));

    /* set active lib */
    pstIspCtx->stAeLibInfo.u32ActiveLib = 0;

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_AWBLibUnRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAwbLib)
{
    HI_S32 s32SearchId;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    /* check null point */
    ISP_CHECK_POINTER(pstAwbLib);

    s32SearchId = ISP_FindLib(pstIspCtx->stAwbLibInfo.astLibs, pstAwbLib);
    if (-1 == s32SearchId)
    {
        ISP_TRACE(HI_DBG_ERR, "can't find awb lib in ISP[%d].\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    memset(&pstIspCtx->stAwbLibInfo.astLibs[s32SearchId], 0, sizeof(ISP_LIB_NODE_S));

    /* set active lib */
    pstIspCtx->stAwbLibInfo.u32ActiveLib = 0;

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_SetBindAttr(VI_PIPE ViPipe, const ISP_BIND_ATTR_S *pstBindAttr)
{
    SENSOR_ID SensorId = 0;
    HI_S32    s32SearchId;
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    ISP_CHECK_POINTER(pstBindAttr);

    s32Ret = ISP_SensorGetId(ViPipe, &SensorId);
    if (HI_SUCCESS  != s32Ret )
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get Sensor id err\n", ViPipe);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* check sensor id */
    if (pstBindAttr->SensorId != SensorId)
    {
        ISP_TRACE(HI_DBG_ERR, "ISP[%d] Register sensor is %d, present sensor is %d.\n",
                  ViPipe, SensorId, pstBindAttr->SensorId);
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* check ae lib */
    s32SearchId = ISP_FindLib(pstIspCtx->stAeLibInfo.astLibs, &pstBindAttr->stAeLib);
    if (-1 != s32SearchId)
    {
        pstIspCtx->stAeLibInfo.u32ActiveLib = s32SearchId;
    }
    else
    {
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* check awb lib */
    s32SearchId = ISP_FindLib(pstIspCtx->stAwbLibInfo.astLibs, &pstBindAttr->stAwbLib);
    if (-1 != s32SearchId)
    {
        pstIspCtx->stAwbLibInfo.u32ActiveLib = s32SearchId;
    }
    else
    {
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* check af lib */
    s32SearchId = ISP_FindLib(pstIspCtx->stAfLibInfo.astLibs, &pstBindAttr->stAfLib);
    if (-1 != s32SearchId)
    {
        pstIspCtx->stAfLibInfo.u32ActiveLib = s32SearchId;
    }
    else
    {
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    /* save global variable */
    memcpy(&pstIspCtx->stBindAttr, pstBindAttr, sizeof(ISP_BIND_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_GetBindAttr(VI_PIPE ViPipe, ISP_BIND_ATTR_S *pstBindAttr)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);

    ISP_CHECK_POINTER(pstBindAttr);

    /* get global variable */
    memcpy(pstBindAttr, &pstIspCtx->stBindAttr, sizeof(ISP_BIND_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_GetVDTimeOut(VI_PIPE ViPipe, ISP_VD_TYPE_E enIspVDType, HI_U32 u32MilliSec)
{
    HI_S32 s32Ret;
    ISP_VD_TIMEOUT_S   stIspVdTimeOut;
    ISP_WORKING_MODE_S stIspWorkMode;

    ISP_CHECK_PIPE(ViPipe);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    stIspVdTimeOut.u32MilliSec = u32MilliSec;
    stIspVdTimeOut.u32IntStatus = 0x0;

    switch (enIspVDType) {
        case ISP_VD_FE_START:
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_VD_TIMEOUT, &stIspVdTimeOut);
            if (s32Ret != HI_SUCCESS)
            {
                return s32Ret;
            }
            break;
        case ISP_VD_FE_END:
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_VD_END_TIMEOUT, &stIspVdTimeOut);
            if (s32Ret != HI_SUCCESS) {
                return s32Ret;
            }
            break;
        case ISP_VD_BE_END:
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_WORK_MODE_GET, &stIspWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                ISP_TRACE(HI_DBG_ERR, "Get Work Mode error!\n");
                return HI_FAILURE;
            }

            if ((stIspWorkMode.enIspRunningMode == ISP_MODE_RUNNING_ONLINE) ||
                (stIspWorkMode.enIspRunningMode == ISP_MODE_RUNNING_SIDEBYSIDE)) {
                ISP_TRACE(HI_DBG_ERR, "Only support ISP_VD_BE_END under ISP offline mode!\n");
                return HI_ERR_ISP_ILLEGAL_PARAM;
            }
            s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_GET_VD_BEEND_TIMEOUT, &stIspVdTimeOut);
            if (s32Ret != HI_SUCCESS) {
                return s32Ret;
            }
            break;
        default:
            ISP_TRACE(HI_DBG_ERR, "ISP[%d] Get VD type %d not support!\n", ViPipe, enIspVDType);
            return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_SetDCFInfo
 Description     : set dcf info to isp firmware
 Input           : ISP_DCF_S *pstIspDCF
 Output          : None
 Return Value    :
 Process         :
 Note            :

  History
  1.Date         : 2014/6/13
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_SetDCFInfo(VI_PIPE ViPipe, const ISP_DCF_INFO_S *pstIspDCF)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_BOOL bTempMap = HI_FALSE;
    HI_U64 u64PhyAddrHigh;
    HI_U64 u64PhyAddrTemp;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_POINTER(pstIspDCF);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    u64PhyAddrHigh = (HI_U64)hi_ext_system_update_info_high_phyaddr_read(ViPipe);
    u64PhyAddrTemp = (HI_U64)hi_ext_system_update_info_low_phyaddr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    if (0 == pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr)
    {
        pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr = u64PhyAddrTemp;

        pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo = HI_MPI_SYS_Mmap(pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr, \
                (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ISP_DCF_CONST_INFO_S)));

        if (HI_NULL == pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo)
        {
            ISP_TRACE(HI_DBG_ERR, "isp[%d] set dcf info mmap failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }

        pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo = \
                (ISP_DCF_CONST_INFO_S *)(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo + ISP_MAX_UPDATEINFO_BUF_NUM);

        bTempMap = HI_TRUE;
    }

    memcpy(pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo, &pstIspDCF->stIspDCFConstInfo, sizeof(ISP_DCF_CONST_INFO_S));
    memcpy(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo, &pstIspDCF->stIspDCFUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));

    if (bTempMap)
    {
        HI_MPI_SYS_Munmap(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo, \
                          (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof (ISP_DCF_CONST_INFO_S)));

        pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr = 0;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : HI_MPI_ISP_GetDCFInfo
 Description     : get dcf info from isp firmware
 Input           : ISP_DCF_S *pstIspDCF
 Output          : None
 Return Value    :
 Process         :
 Note            :

  History
  1.Date         : 2014/6/16
    Author       :
    Modification : Created function

*****************************************************************************/
HI_S32 HI_MPI_ISP_GetDCFInfo(VI_PIPE ViPipe, ISP_DCF_INFO_S *pstIspDCF)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    HI_BOOL bTempMap = HI_FALSE;
    HI_U64 u64PhyAddrHigh;
    HI_U64 u64PhyAddrTemp;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_POINTER(pstIspDCF);
    ISP_CHECK_OPEN(ViPipe);
    ISP_CHECK_MEM_INIT(ViPipe);

    u64PhyAddrHigh = (HI_U64)hi_ext_system_update_info_high_phyaddr_read(ViPipe);
    u64PhyAddrTemp = (HI_U64)hi_ext_system_update_info_low_phyaddr_read(ViPipe);
    u64PhyAddrTemp |= (u64PhyAddrHigh << 32);

    if (0 == pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr)
    {
        pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr = u64PhyAddrTemp;

        pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo = HI_MPI_SYS_Mmap(pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr, \
                (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ISP_DCF_CONST_INFO_S)));

        if (HI_NULL == pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo)
        {
            ISP_TRACE(HI_DBG_ERR, "isp[%d] get dcf info mmap failed!\n", ViPipe);
            return HI_ERR_ISP_NOMEM;
        }

        pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo = \
                (ISP_DCF_CONST_INFO_S *)(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo + ISP_MAX_UPDATEINFO_BUF_NUM);

        bTempMap = HI_TRUE;
    }

    memcpy(&pstIspDCF->stIspDCFConstInfo, pstIspCtx->stUpdateInfoCtrl.pstIspDCFConstInfo, sizeof(ISP_DCF_CONST_INFO_S));
    memcpy(&pstIspDCF->stIspDCFUpdateInfo, pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo, sizeof(ISP_DCF_UPDATE_INFO_S));

    if (bTempMap)
    {
        HI_MPI_SYS_Munmap(pstIspCtx->stUpdateInfoCtrl.pstIspUpdateInfo,
                          (sizeof(ISP_DCF_UPDATE_INFO_S) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof (ISP_DCF_CONST_INFO_S)));

        pstIspCtx->stIspTransInfo.stUpdateInfo.u64PhyAddr = 0;
    }

    return HI_SUCCESS;
}


HI_S32 HI_MPI_ISP_SetFrameInfo(VI_PIPE ViPipe, const ISP_FRAME_INFO_S *pstIspFrame)
{
    HI_S32 s32Ret;
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx->stFrameInfoCtrl.pstIspFrame);
    ISP_CHECK_POINTER(pstIspFrame);
    ISP_CHECK_OPEN(ViPipe);

    s32Ret = ioctl(g_as32IspFd[ViPipe], ISP_FRAME_INFO_SET, pstIspFrame);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    memcpy(pstIspCtx->stFrameInfoCtrl.pstIspFrame, pstIspFrame, sizeof(ISP_FRAME_INFO_S));

    return HI_SUCCESS;
}

HI_S32 HI_MPI_ISP_GetFrameInfo(VI_PIPE ViPipe, ISP_FRAME_INFO_S *pstIspFrame)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;

    ISP_CHECK_PIPE(ViPipe);
    ISP_GET_CTX(ViPipe, pstIspCtx);
    ISP_CHECK_POINTER(pstIspCtx);
    ISP_CHECK_POINTER(pstIspFrame);
    ISP_CHECK_OPEN(ViPipe);

    if (ioctl(g_as32IspFd[ViPipe], ISP_FRAME_INFO_GET, pstIspFrame))
    {
        return HI_ERR_ISP_NOMEM;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
