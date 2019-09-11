/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_dcfinfo.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2014/06/17
  Description   :
  History       :
  1.Date        : 2014/06/17
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __ISP_DCFINFO_H__
#define __ISP_DCFINFO_H__

#include "hi_comm_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_UPDATE_INFO_CTRL_S {
    ISP_DCF_CONST_INFO_S  *pstIspDCFConstInfo;
    ISP_DCF_UPDATE_INFO_S *pstIspUpdateInfo;
} ISP_UPDATE_INFO_CTRL_S;

HI_S32 ISP_UpdateInfoInit(VI_PIPE ViPipe);
HI_S32 ISP_UpdateInfoExit(VI_PIPE ViPipe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
