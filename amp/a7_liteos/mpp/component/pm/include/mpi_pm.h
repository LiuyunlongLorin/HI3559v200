/******************************************************************************

  Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mpi_pm.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2018/09/17
  Description   :
  History       :


******************************************************************************/
#ifndef __MPI_PM_H__
#define __MPI_PM_H__

#include "hi_comm_video.h"
#include "hi_comm_pm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */


HI_S32 HI_MPI_PM_SetMediaParam(HI_MPI_PM_MEDIA_CFG_S* pstPmParam);//
HI_S32 HI_MPI_PM_GetMediaParam(HI_MPI_PM_MEDIA_CFG_S* pstPmParam);//
HI_S32 HI_MPI_PM_SetSingleMediaParam(HI_MPI_PM_SIGLE_MEDIA_CFG_S* pstPmParam);
HI_S32 HI_MPI_PM_GetTemperature(HI_S32 * ps32PmTempe);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__MPI_SNAP_H__ */


