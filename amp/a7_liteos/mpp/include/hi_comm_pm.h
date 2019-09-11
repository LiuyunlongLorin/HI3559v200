/******************************************************************************

  Copyright (C), 2018-2028, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : hi_comm_pm.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Last Modified :
  Description   : common struct definition for pm
  Function List :


******************************************************************************/

#ifndef __HI_COMM_PM_H__
#define __HI_COMM_PM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "hi_type.h"
#include "hi_common.h"
#include "hi_errno.h"
#include "hi_comm_video.h"


/* failure caused by malloc buffer */
#define HI_ERR_PM_NOBUF           HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define HI_ERR_PM_BUF_EMPTY       HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define HI_ERR_PM_NULL_PTR        HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_PM_ILLEGAL_PARAM   HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_PM_BUF_FULL        HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define HI_ERR_PM_SYS_NOTREADY    HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_PM_NOT_SUPPORT     HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define HI_ERR_PM_NOT_PERMITTED   HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define HI_ERR_PM_BUSY            HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define HI_ERR_PM_INVALID_CHNID   HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define HI_ERR_PM_CHN_UNEXIST     HI_DEF_ERR(HI_ID_PM, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)


typedef struct hiPM_MEDIA_VI_CFG_S
{
    HI_U32 au32VicapFreq[VI_MAX_DEV_NUM];
    HI_U32 au32VipipeSumPixel[VI_MAX_PIPE_NUM];
 } HI_MPI_PM_MEDIA_VI_CFG_S;

typedef struct hiPM_MEDIA_CFG_S
{
    HI_MPI_PM_MEDIA_VI_CFG_S stViCfg;
    HI_U32 u32VpssSumPix;
    HI_U32 u32VencSumPix;
    HI_U32 u32VdecSumPix;
    HI_U32 u32AvspSumPix;
} HI_MPI_PM_MEDIA_CFG_S;


typedef enum hiPM_MEDIA_MOD_E
{
    HI_PM_MEDIA_VI,
    HI_PM_MEDIA_PIPEBE,       
    HI_PM_MEDIA_VPSS,
    HI_PM_MEDIA_AVSP,
    HI_PM_MEDIA_VEDU,
    HI_PM_MEDIA_VDEC,
    HI_PM_MEDIA_BUIT
} HI_MPI_PM_MEDIA_MOD_E;


typedef struct hiPM_SIGLE_MEDIA_CFG_S
{
    HI_MPI_PM_MEDIA_MOD_E enMode;
    union
    {
        HI_MPI_PM_MEDIA_VI_CFG_S stViCfg;
        HI_U32 u32VpssSumPix;
        HI_U32 u32VencSumPix;
        HI_U32 u32VdecSumPix;
        HI_U32 u32AvspSumPix;
    }  unMediaCfg;
} HI_MPI_PM_SIGLE_MEDIA_CFG_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __HI_COMM_PM_H__ */

