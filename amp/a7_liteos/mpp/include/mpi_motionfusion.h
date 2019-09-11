/************************************************************************
*             Copyright (C) 2018, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: mpi_motionfusion.h
* Description: init draft
*
*************************************************************************/
#ifndef  __MPI_MOTIONFUSION_H__
#define  __MPI_MOTIONFUSION_H__

#include "hi_type.h"
#include "hi_comm_motionfusion.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define MOTIONFUSION_DEVICE_ID_0 0
#define MOTIONFUSION_DEVICE_ID_1 1

//-------------------------set fusion attr  --------------------------------------------
HI_S32 HI_MPI_MOTIONFUSION_SetAttr(const HI_U32 u32FusionID, const MFUSION_ATTR_S* pstMFusionAttr);

HI_S32 HI_MPI_MOTIONFUSION_GetAttr(const HI_U32 u32FusionID, MFUSION_ATTR_S* pstMFusionAttr);

//-------------------------set calibration param --------------------------------------------
/* gyro drift cal */
HI_S32 HI_MPI_MOTIONFUSION_SetGyroDrift(const HI_U32 u32FusionID, const HI_BOOL  bEnGyroDrift, const IMU_DRIFT aGyroDrift);

HI_S32 HI_MPI_MOTIONFUSION_GetGyroDrift(const HI_U32 u32FusionID, HI_BOOL* pbEnGyroDrift, IMU_DRIFT aGyroDrift);

/* gyro six side cal */
HI_S32 HI_MPI_MOTIONFUSION_SetGyroSixSideCal(const HI_U32 u32FusionID, const HI_BOOL  bEnGyroSixSideCal, const IMU_MATRIX aGyroRotationMatrix);

HI_S32 HI_MPI_MOTIONFUSION_GetGyroSixSideCal(const HI_U32 u32FusionID, HI_BOOL* pbEnGyroSixSideCal, IMU_MATRIX aGyroRotationMatrix);

/* gyro temp cal */
HI_S32 HI_MPI_MOTIONFUSION_SetGyroTempDrift(const HI_U32 u32FusionID, const HI_BOOL  bEnGyroTempDrift, const MFUSION_TEMP_DRIFT_S* pstTempDrift);

HI_S32 HI_MPI_MOTIONFUSION_GetGyroTempDrift(const HI_U32 u32FusionID, HI_BOOL* pbEnGyroTempDrift, MFUSION_TEMP_DRIFT_S* pstTempDrift);

/* gyro online temp cal*/
HI_S32 HI_MPI_MOTIONFUSION_SetGyroOnLineTempDrift(const HI_U32 u32FusionID, const HI_BOOL  bEnGyroOnLineTempDrift, const MFUSION_TEMP_DRIFT_S* pstTempDrift);

HI_S32 HI_MPI_MOTIONFUSION_GetGyroOnLineTempDrift(const HI_U32 u32FusionID, HI_BOOL* pbEnGyroOnLineTempDrift, MFUSION_TEMP_DRIFT_S* pstTempDrift);

/* gyro online drift cal */
HI_S32 HI_MPI_MOTIONFUSION_SetGyroOnLineDrift(const HI_U32 u32FusionID, const HI_BOOL  bEnGyroOnLineDrift, const IMU_DRIFT aGyroDrift);

HI_S32 HI_MPI_MOTIONFUSION_GetGyroOnLineDrift(const HI_U32 u32FusionID, HI_BOOL* pbEnGyroOnLineDrift, IMU_DRIFT aGyroOnLineDrift);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef __MPI_MONTIONFUSION_H__*/
