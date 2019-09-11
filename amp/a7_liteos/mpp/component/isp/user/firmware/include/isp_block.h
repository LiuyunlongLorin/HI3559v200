/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_block.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2016/09/01
  Description   :
  History       :
  1.Date        : 2016/09/01
    Author      :
    Modification: Created file

******************************************************************************/

#ifndef __ISP_BLOCK_CONFIG_H__
#define __ISP_BLOCK_CONFIG_H__

#include "hi_type.h"
#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_RECT_S {
    HI_S32 s32X;
    HI_S32 s32Y;
    HI_U32 u32Width;
    HI_U32 u32Height;
} ISP_RECT_S;

typedef enum hiISP_RUNNING_MODE_E {
    ISP_MODE_RUNNING_OFFLINE = 0,
    ISP_MODE_RUNNING_ONLINE,
    ISP_MODE_RUNNING_SIDEBYSIDE,
    ISP_MODE_RUNNING_STRIPING,

    ISP_MODE_RUNNING_BUTT,
} ISP_RUNNING_MODE_E;

typedef enum hiISP_DATAINPUT_MODE_E {
    ISP_MODE_RAW = 0,
    ISP_MODE_BT1120_YUV,
    ISP_MODE_DATAINPUT_BUTT,
} ISP_DATAINPUT_MODE_E;

typedef struct hiISP_WORKING_MODE_S {
    HI_U8                u8PreBlockNum;
    HI_U8                u8BlockNum;
    HI_U32               u32OverLap;
    SIZE_S               stFrameRect;  /* BE Full Frame  information */
    RECT_S               astBlockRect[ISP_STRIPING_MAX_NUM];
    ISP_RUNNING_MODE_E   enIspRunningMode;
    ISP_DATAINPUT_MODE_E enIspDataInputMode;
} ISP_WORKING_MODE_S;

typedef struct hiISP_BLOCK_S {
    HI_U8   u8BlockNum;
    HI_U8   u8PreBlockNum;
    HI_U32  u32OverLap;
    SIZE_S  stFrameRect;  /* BE Full Frame  information */
    RECT_S  astBlockRect[ISP_STRIPING_MAX_NUM];
    ISP_RUNNING_MODE_E enIspRunningMode;
} ISP_BLOCK_ATTR_S;

HI_S32 ISP_BlockInit(VI_PIPE ViPipe, ISP_BLOCK_ATTR_S *pstBlock);
HI_S32 ISP_BlockExit(VI_PIPE ViPipe);
HI_S32 ISP_BlockUpdate(VI_PIPE ViPipe, ISP_BLOCK_ATTR_S *pstBlock);
HI_S32 ISP_GetWorkMode(VI_PIPE ViPipe, ISP_WORKING_MODE_S *pstIspWorkMode);
HI_U32 ISP_GetBlockRect(ISP_RECT_S *pstBlockRect, ISP_BLOCK_ATTR_S *pstBlock, HI_U8 u8BlockId);

#define ISP_CHECK_RUNNING_MODE(eIspRunningMode)                   \
    do {                                                          \
        if (eIspRunningMode >= ISP_MODE_RUNNING_BUTT) {           \
            ISP_TRACE(HI_DBG_ERR, "Invalid Isp Running mode!\n"); \
            return HI_ERR_ISP_ILLEGAL_PARAM;                      \
        }                                                         \
    } while (0)

#define ISP_CHECK_BLOCK_ID(Blk)                                   \
    do {                                                          \
        if (((Blk) >= ISP_STRIPING_MAX_NUM)) {                    \
            ISP_TRACE(HI_DBG_ERR, "Err isp block Id %d!\n", Blk); \
            return HI_ERR_ISP_ILLEGAL_PARAM;                      \
        }                                                         \
    } while (0)

#define ISP_CHECK_BLOCK_NUM(Blk)                                   \
    do {                                                           \
        if (((Blk) > ISP_STRIPING_MAX_NUM) || ((Blk) < 1)) {       \
            ISP_TRACE(HI_DBG_ERR, "Err isp block Num %d!\n", Blk); \
            return HI_ERR_ISP_ILLEGAL_PARAM;                       \
        }                                                          \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
