/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_frameinfo.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        :
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __ISP_DNGINFO_H__
#define __ISP_DNGINFO_H__

#include "hi_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_DNG_INFO_CTRL_S {
    DNG_IMAGE_STATIC_INFO_S *pstIspDng;
} ISP_DNG_INFO_CTRL_S;

typedef struct hiDNG_MATRIX_S {
    HI_DOUBLE adAColorMatrix[9];
    HI_DOUBLE adD50ColorMatrix[9];
    HI_DOUBLE adAForwardMatrix[9];
    HI_DOUBLE adD50ForwardMatrix[9];
} DNG_MATRIX_S;

HI_S32 ISP_UpdateDngImageDynamicInfo(VI_PIPE ViPipe);

HI_S32 ISP_DngInfoInit(VI_PIPE ViPipe);
HI_S32 ISP_DngInfoExit(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
