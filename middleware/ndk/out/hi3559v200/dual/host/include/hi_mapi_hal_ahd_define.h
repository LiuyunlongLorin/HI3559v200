/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_hal_ahd_define.h
 * @brief   ahd module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_HAL_AHD_DEFINE_H__
#define __HI_MAPI_HAL_AHD_DEFINE_H__

#include "hi_mapi_comm_define.h"
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AHD */
/** @{ */ /** <!-- [AHD] */

#define HI_HAL_AHD_CHN_MAX 4

/* struct */

typedef enum hiHAL_AHD_RES_E {
    HI_HAL_AHD_RES_720P = 0,
    HI_HAL_AHD_RES_1080P,
    HI_HAL_AHD_RES_BUIT
} HI_HAL_AHD_RES_E;

typedef enum hiHAL_AHD_FPS_E {
    HI_HAL_AHD_FPS_25 = 0,
    HI_HAL_AHD_FPS_30,
    HI_HAL_AHD_FPS_BUIT
} HI_HAL_AHD_FPS_E;

typedef struct hiHAL_AHD_CFG_S {
    HI_HAL_AHD_RES_E enRes;
    HI_HAL_AHD_FPS_E enFps;
} HI_HAL_AHD_CFG_S;

typedef enum hiHAL_AHD_STATUS_E {
    HI_HAL_AHD_STATUS_CONNECT = 0,
    HI_HAL_AHD_STATUS_DISCONNECT,
    HI_HAL_AHD_STATUS_BUIT
} HI_HAL_AHD_STATUS_E;

typedef struct hi_HAL_AHD_DEV_S {
    HI_U32 id;
    HI_CHAR name[20];
    HI_U32 u32ChnMax;
    HI_BOOL bUsedChn[HI_HAL_AHD_CHN_MAX]; /* HI_TRUE means used the chn, and only change chn will update it's value */
    HI_S32 (*pfnAhdInit)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev);
    HI_S32 (*pfnAhdDeinit)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev);
    HI_S32 (*pfnAhdStart)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev);
    HI_S32 (*pfnAhdStop)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev);
    HI_S32 (*pfnAhdStartChn)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev, HI_U32 u32Chn);
    HI_S32 (*pfnAhdStopChn)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev, HI_U32 u32Chn);
    HI_S32 (*pfnAhdSetAttr)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev, const HI_HAL_AHD_CFG_S *pstAHDCfg);
    HI_S32 (*pfnAhdGetAttr)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev, HI_HAL_AHD_CFG_S *pstAHDCfg);
    HI_S32 (*pfnAhdGetStatus)(struct hi_HAL_AHD_DEV_S *pstHalAhdDev, HI_HAL_AHD_STATUS_E astAhdStatus[],
           HI_U32 u32Cnt);
} HI_HAL_AHD_DEV_S;

/** @} */ /** <!-- ==== AHD End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_MAPI_HAL_AHD_DEFINE_H__ */
