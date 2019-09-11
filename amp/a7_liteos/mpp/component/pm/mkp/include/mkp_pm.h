/******************************************************************************
 Copyright (C), 2016, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : mkp_pm.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2016/07/15
Last Modified :
Description   :
Function List :
******************************************************************************/

#ifndef __MKP_PM_H__
#define __MKP_PM_H__

#include "hi_math.h"
#include "hi_common.h"
//#include "mkp_ioctl.h"
//#include "pm_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/* Use 'r' as magic number */
#define PM_IOC_MAGIC  'r'
#define PM_SET_MEDIA_PARAM                  _IOW(PM_IOC_MAGIC, 1, HI_MPI_PM_MEDIA_CFG_S)
#define PM_GET_MEDIA_PARAM                  _IOR(PM_IOC_MAGIC, 2, HI_MPI_PM_MEDIA_CFG_S)
#define PM_SET_SINGLE_MEDIA_PARAM     _IOW(PM_IOC_MAGIC, 3, HI_MPI_PM_SIGLE_MEDIA_CFG_S)
#define PM_SVP_ENABLE_CTRL                    _IO(PM_IOC_MAGIC, 4)
#define PM_DVFS_ENABLE_CTRL                  _IO(PM_IOC_MAGIC, 5)
#define PM_DVFS_DISABLE_CTRL                 _IO(PM_IOC_MAGIC, 6)
#define PM_GET_TEMPERATURE                 _IOR(PM_IOC_MAGIC, 7,HI_U32)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif


