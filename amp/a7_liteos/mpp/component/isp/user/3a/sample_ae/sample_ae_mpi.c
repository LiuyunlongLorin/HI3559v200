/******************************************************************************

  Copyright (C), 2016, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sample_ae_mpi.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2014/04/01
  Description   :
  History       :
  1.Date        : 2014/04/01
    Author      :
    Modification: Created file

******************************************************************************/
#include <string.h>
#include <stdio.h>

#include "hi_comm_isp.h"
#include "hi_comm_3a.h"
#include "sample_ae_ext_config.h"
//#include "isp_metering_mem_config.h"
//#include "isp_histogram_mem_config.h"
#include "isp_config.h"
#include "sample_ae_adp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


HI_S32 SAMPLE_HI_MPI_ISP_SetExposureAttr(VI_PIPE ViPipe, const ISP_EXPOSURE_ATTR_S *pstExpAttr)
{
    ALG_LIB_S stAeLib;
    stAeLib.s32Id = 4;
    strncpy(stAeLib.acLibName, "hisi_ae_lib", sizeof("hisi_ae_lib"));

    if (pstExpAttr->stAuto.enAEMode >= AE_MODE_BUTT)
    {
        AE_TRACE(HI_DBG_ERR, "Invalid AE mode!\n");
        return HI_ERR_ISP_ILLEGAL_PARAM;
    }

    hi_ext_system_ae_mode_write((HI_U8)stAeLib.s32Id, (HI_U8)pstExpAttr->stAuto.enAEMode);


    return HI_SUCCESS;
}

HI_S32 SAMPLE_HI_MPI_ISP_GetExposureAttr(VI_PIPE ViPipe, ISP_EXPOSURE_ATTR_S *pstExpAttr)
{

    return HI_SUCCESS;
}


HI_S32 SAMPLE_HI_MPI_ISP_SetWDRExposureAttr(VI_PIPE ViPipe, const ISP_WDR_EXPOSURE_ATTR_S *pstWDRExpAttr)
{

    return HI_SUCCESS;
}

HI_S32 SAMPLE_HI_MPI_ISP_GetWDRExposureAttr(VI_PIPE ViPipe, ISP_WDR_EXPOSURE_ATTR_S *pstWDRExpAttr)
{

    return HI_SUCCESS;
}


HI_S32 SAMPLE_ISP_RouteCheck(HI_U8 u8Id, const ISP_AE_ROUTE_S *pstRoute)
{

    return HI_SUCCESS;
}


HI_S32 SAMPLE_HI_MPI_ISP_SetAERouteAttr(VI_PIPE ViPipe, const ISP_AE_ROUTE_S *pstAERouteAttr)
{

    return HI_SUCCESS;
}

HI_S32 SAMPLE_HI_MPI_ISP_GetAERouteAttr(VI_PIPE ViPipe, ISP_AE_ROUTE_S *pstAERouteAttr)
{

    return HI_SUCCESS;
}


HI_S32 SAMPLE_HI_MPI_ISP_QueryExposureInfo(VI_PIPE ViPipe, ISP_EXP_INFO_S *pstExpInfo)
{

    ALG_LIB_S stAeLib;
    stAeLib.s32Id = 4;
    strncpy(stAeLib.acLibName, "hisi_ae_lib", sizeof("hisi_ae_lib"));

    pstExpInfo->u32AGain = hi_ext_system_query_exposure_again_read((HI_U8)stAeLib.s32Id);

    return HI_SUCCESS;

}


HI_S32 SAMPLE_HI_MPI_ISP_SetIrisAttr(VI_PIPE ViPipe, const ISP_IRIS_ATTR_S *pstIrisAttr)
{


    return HI_SUCCESS;
}


HI_S32 SAMPLE_HI_MPI_ISP_GetIrisAttr(VI_PIPE ViPipe, ISP_IRIS_ATTR_S *pstIrisAttr)
{

    return HI_SUCCESS;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
