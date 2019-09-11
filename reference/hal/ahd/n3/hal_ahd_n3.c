/**
* @file    hal_ahd_n3.c
* @brief   hal ahd n3 implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

#include "hi_appcomm.h"
#include "hi_type.h"
#include "hi_appcomm_util.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"
#include "hi_mapi_hal_ahd_define.h"
#include "n3_drv.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AHD_N3 */
/** @{ */  /** <!-- [AHD_N3] */
extern HI_HAL_AHD_DEV_S g_halAhdN3Obj;
#define HAL_AHD_DEV "/dev/n3_dev"
typedef struct tagHALAHD_N3_CTX_S
{
    HI_S32 halAhdfd;
    HI_HAL_AHD_CFG_S ahdCfg;
} HALAHD_N3_CTX_S;

static HALAHD_N3_CTX_S g_halAhdN3Ctx={HAL_FD_INITIALIZATION_VAL,{HI_HAL_AHD_RES_BUIT,HI_HAL_AHD_FPS_BUIT}};

static HI_VOID HAL_AHD_PinoutInit(HI_VOID)
{
#ifdef HI3559V200
#define PER1_CRG60 0x120100f0
    HI_U32 value;

#ifdef BOARD_DEMB
        /**0 is maximum driving capability */
        himm(0x114F0060,0x602);/** used for sensor1_clk_out*/
        himm(0x114F0064,0x602);/** used for sensor1_rst*/
#elif BOARD_DASHCAM_REFB
        /**driving capability:6 for EMI compatible*/
        himm(0x114F0060,0x662);/** used for sensor1_clk_out*/
        himm(0x114F0064,0x662);/** used for sensor1_rst*/
#else
        MLOGE("please point board type,fail\n\n");
#endif

    himd(PER1_CRG60, &value);
    value |= (0x1 << 6);   /**<open sensor1 clk*/
    /**<sensor1 clk:27M*/
    value &= ~(0xf << 8);
    value |= (0xa << 8);

    value |= (0x1 << 7);/**<do sensor1 reset*/
    himm(PER1_CRG60,value);

    HI_usleep(50*1000);
    value &= ~(0x1 << 7);   /**<undo sensor1 reset*/
    himm(PER1_CRG60,value);

#ifdef BOARD_DEMB
    /**I2C1 pinout*/
    himm(0x114F007C,0x432);
    himm(0x114F0080,0x532);
#elif BOARD_DASHCAM_REFB
    /**I2C4 pinout*/
    himm(0x114F0008,0x433);
    himm(0x114F000C,0x533);
#else
    MLOGE("please point board type,fail\n\n");
#endif

#else
    MLOGE("please point chip type,fail\n\n");
#endif
}

HI_S32 HAL_CHECK_ParamValid(HI_HAL_AHD_DEV_S* ahdDev)
{
    HI_APPCOMM_CHECK_POINTER(ahdDev,HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR((ahdDev->id == g_halAhdN3Obj.id),HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR(memcmp(ahdDev->name,g_halAhdN3Obj.name,sizeof(ahdDev->name)) == 0,HI_FAILURE);
    return HI_SUCCESS;
}
extern int n3_module_init(unsigned int i2c_num);
extern void n3_module_exit();


HI_S32 HAL_AHD_N3_Init(HI_HAL_AHD_DEV_S* ahdDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i2cNum = 0;
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd != HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("already init");
        return HI_HAL_EINITIALIZED;
    }
    HAL_AHD_PinoutInit();
#ifdef HI3559V200
#ifdef BOARD_DEMB
    i2cNum = 1;
#elif BOARD_DASHCAM_REFB
    i2cNum = 4;
#else
    MLOGE("please point board type,fail\n\n");
    return HI_HAL_EINTER;
#endif
#else
    MLOGE("please point chip type,fail\n\n");
    return HI_HAL_EINTER;
#endif
    s32Ret = n3_module_init(i2cNum);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("insmod n3: failed\n");
        return HI_HAL_EINTER;
    }
    g_halAhdN3Ctx.halAhdfd = open(HAL_AHD_DEV, O_RDWR);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("open [%s] failed\n",HAL_AHD_DEV);
        return HI_HAL_EINVOKESYS;
    }

    n3_detect_param  n3DetectParam;
    memset(&n3DetectParam,'\0',sizeof(n3_detect_param));
    s32Ret = ioctl(g_halAhdN3Ctx.halAhdfd, IOC_VDEC_DETECT_CHIP, &n3DetectParam);/**chip detect check*/
    if((-1 == s32Ret) || (VDEC_N3_CHIP_N3 != n3DetectParam.chip_def))
    {
        MLOGE("ioctl failed or chip_def[%d] is not N3\n",n3DetectParam.chip_def);
        close(g_halAhdN3Ctx.halAhdfd);
        g_halAhdN3Ctx.halAhdfd = HAL_FD_INITIALIZATION_VAL;
        n3_module_exit();
        return HI_HAL_EINTER;
    }
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_Start(HI_HAL_AHD_DEV_S* ahdDev)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_StartChn(HI_HAL_AHD_DEV_S* ahdDev, HI_U32 u32Chn)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    ;//todo
    return HI_SUCCESS;
}


HI_S32 HAL_AHD_N3_StopChn(HI_HAL_AHD_DEV_S* ahdDev, HI_U32 u32Chn)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    ;//todo
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_AhdGetStatus(HI_HAL_AHD_DEV_S* ahdDev, HI_HAL_AHD_STATUS_E aAhdStatus[], HI_U32 u32Cnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    n3_novideo_param  n3NoVideoParam;
    memset(&n3NoVideoParam,'\0',sizeof(n3NoVideoParam));
    s32Ret = ioctl(g_halAhdN3Ctx.halAhdfd, IOC_VDEC_READ_B1_D8, &n3NoVideoParam);
    if(-1 == s32Ret)
    {
        MLOGE("ioctrl : failed\n");
        return HI_HAL_EINTER;
    }
    for(HI_S32 s32Chn = 0; (s32Chn < ahdDev->u32ChnMax) && (s32Chn < u32Cnt); s32Chn++)
    {
        if(ahdDev->bUsedChn[s32Chn] == HI_TRUE)
        {
            if(n3NoVideoParam.b1_d8 & (0x01<<s32Chn))
            {
                aAhdStatus[s32Chn] = HI_HAL_AHD_STATUS_DISCONNECT;

            }
            else
            {
                aAhdStatus[s32Chn] = HI_HAL_AHD_STATUS_CONNECT;
            }
        }
        else
        {
            aAhdStatus[s32Chn] = HI_HAL_AHD_STATUS_BUIT;
        }
    }
    for(HI_S32 s32Chn = ahdDev->u32ChnMax; (s32Chn < u32Cnt)&&(s32Chn < HI_HAL_AHD_CHN_MAX); s32Chn++)
    {
        aAhdStatus[s32Chn] = HI_HAL_AHD_STATUS_BUIT;
    }
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_GetAttr( HI_HAL_AHD_DEV_S* ahdDev, HI_HAL_AHD_CFG_S* ahdCfg)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ahdCfg,HI_FAILURE);
    if(g_halAhdN3Ctx.ahdCfg.enRes == HI_HAL_AHD_RES_BUIT)
    {
        MLOGE("not init");
        return HI_FAILURE;
    }
    memcpy(ahdCfg,&g_halAhdN3Ctx.ahdCfg,sizeof(HI_HAL_AHD_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_SetAttr(HI_HAL_AHD_DEV_S* ahdDev, const HI_HAL_AHD_CFG_S* ahdCfg)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(ahdCfg,HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR((ahdCfg->enRes >=HI_HAL_AHD_RES_720P)&&(ahdCfg->enRes < HI_HAL_AHD_RES_BUIT),HI_FAILURE);
    HI_APPCOMM_CHECK_EXPR((ahdCfg->enFps >=HI_HAL_AHD_FPS_25)&&(ahdCfg->enFps <=HI_HAL_AHD_FPS_BUIT),HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    n3_init_param n3InitParam;
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    if(memcmp(&g_halAhdN3Ctx.ahdCfg,ahdCfg,sizeof(HI_HAL_AHD_CFG_S)) == 0)
    {
        MLOGW("the same config enRes[%d],enFps[%d]\n",ahdCfg->enRes,ahdCfg->enFps);
        return HI_SUCCESS;
    }
    memset(&n3InitParam,'\0',sizeof(n3_init_param));
    if(HI_HAL_AHD_RES_720P == ahdCfg->enRes)
        n3InitParam.fmt_res = VDEC_N3_FMT_720P;
    else
        n3InitParam.fmt_res = VDEC_N3_FMT_1080P;

    if(HI_HAL_AHD_FPS_25 == ahdCfg->enFps)
        n3InitParam.fmt_fps = VDEC_N3_FMT_25P;
    else
        n3InitParam.fmt_fps = VDEC_N3_FMT_30P;

    MLOGI("AHD config enRes[%d],enFps[%d]\n",ahdCfg->enRes,ahdCfg->enFps);
    s32Ret = ioctl(g_halAhdN3Ctx.halAhdfd, IOC_VDEC_INIT, &n3InitParam);
    if (s32Ret < 0)
    {
        MLOGE("ioctl IOC_VDEC_INIT error");
        return HI_HAL_EINTER;
    }
    for(HI_S32 s32Chn = 0; s32Chn < ahdDev->u32ChnMax; s32Chn++)
    {
        if(ahdDev->bUsedChn[s32Chn] == HI_TRUE)
        {
            s32Ret = ioctl(g_halAhdN3Ctx.halAhdfd, IOC_VDEC_SELECT_CHANNEL, &s32Chn);
            if (s32Ret < 0)
            {
                MLOGE("ioctl IOC_VDEC_SELECT_CHANNEL error");
                return HI_HAL_EINTER;
            }
        }
        else
        {
            ;//todo
        }
    }
    HI_usleep(100*1000);
    memcpy(&g_halAhdN3Ctx.ahdCfg,ahdCfg,sizeof(HI_HAL_AHD_CFG_S));
    return HI_SUCCESS;

}


HI_S32 HAL_AHD_N3_Stop(HI_HAL_AHD_DEV_S* ahdDev)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    if (g_halAhdN3Ctx.halAhdfd < 0)
    {
        MLOGE("not init");
        return HI_HAL_ENOINIT;
    }
    ;//todo
    return HI_SUCCESS;
}

HI_S32 HAL_AHD_N3_Deinit(HI_HAL_AHD_DEV_S* ahdDev)
{
    HI_APPCOMM_CHECK_RETURN(HAL_CHECK_ParamValid(ahdDev),HI_FAILURE);
    HI_S32 s32Ret;
    if (g_halAhdN3Ctx.halAhdfd == HAL_FD_INITIALIZATION_VAL)
    {
        MLOGE("ahd not initialized,no need to close\n");
        return HI_HAL_ENOINIT;
    }
    s32Ret = close(g_halAhdN3Ctx.halAhdfd);
    if (0 > s32Ret)
    {
        MLOGE("halAhdfd[%d] close,fail\n",g_halAhdN3Ctx.halAhdfd);
        return HI_HAL_EINVOKESYS;
    }
    g_halAhdN3Ctx.halAhdfd = HAL_FD_INITIALIZATION_VAL;
    g_halAhdN3Ctx.ahdCfg.enRes = HI_HAL_AHD_RES_BUIT;
    g_halAhdN3Ctx.ahdCfg.enFps = HI_HAL_AHD_FPS_BUIT;
    n3_module_exit();
#ifdef HI3559V200
    HI_U32 value;
    himd(PER1_CRG60, &value);
    value |= (0x1 << 7);/**<do sensor1 reset*/
    himm(PER1_CRG60,value);
#endif
    return HI_SUCCESS;
}

HI_HAL_AHD_DEV_S g_halAhdN3Obj =
{
    .id = 0,
    .name = "HALN3",
    .u32ChnMax= 1,
    .bUsedChn={HI_TRUE,HI_FALSE,HI_FALSE,HI_FALSE}, /*HI_TRUE means used the chn, and only change chn will update*/
    .pfnAhdInit = HAL_AHD_N3_Init,
    .pfnAhdStart = HAL_AHD_N3_Start,
    .pfnAhdGetStatus = HAL_AHD_N3_AhdGetStatus,
    .pfnAhdStartChn = HAL_AHD_N3_StartChn,
    .pfnAhdStopChn = HAL_AHD_N3_StopChn,
    .pfnAhdGetAttr = HAL_AHD_N3_GetAttr,
    .pfnAhdSetAttr = HAL_AHD_N3_SetAttr,
    .pfnAhdStop = HAL_AHD_N3_Stop,
    .pfnAhdDeinit = HAL_AHD_N3_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
