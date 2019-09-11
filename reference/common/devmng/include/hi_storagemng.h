/**
 * @file      hi_storagemng.h
 * @brief     module err/event/interface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 * @version   1.0

 */

#ifndef _HI_STORAGEMNG_H_
#define _HI_STORAGEMNG_H_

#include "hi_appcomm.h"
#include "hi_fstool.h"
#include "hi_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     STORAGEMNG */
/** @{ */  /** <!-- [STORAGEMNG] */

/** Error Code */
#define HI_STORAGEMNG_EINVAL             HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_EINVAL) /**<Invalid argument */
#define HI_STORAGEMNG_ENOTINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_ENOINIT) /**<Not inited */
#define HI_STORAGEMNG_EINITIALIZED       HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_EINITIALIZED) /**<Already Initialized */
#define HI_STORAGEMNG_EINTER             HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_EINTER) /**<Already Initialized */
#define HI_STORAGEMNG_EREGISTER_EVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_ERRNO_CUSTOM_BOTTOM) /**<register event failed */
#define HI_STORAGEMNG_EMAXINSTANCE       HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_ERRNO_CUSTOM_BOTTOM+1) /**<beyond maximum instance */
#define HI_STORAGEMNG_ESTORAGE           HI_APPCOMM_ERR_ID(HI_APP_MOD_STORAGEMNG, HI_ERRNO_CUSTOM_BOTTOM+2) /**<storage interface error */

/** Event */
typedef enum hiEVENT_STORAGEMNG_E
{
    HI_EVENT_STORAGEMNG_DEV_UNPLUGED = HI_APPCOMM_EVENT_ID(HI_APP_MOD_STORAGEMNG, 0),
    HI_EVENT_STORAGEMNG_DEV_CONNECTING,
    HI_EVENT_STORAGEMNG_DEV_ERROR,
    HI_EVENT_STORAGEMNG_FS_CHECKING,
    HI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
    HI_EVENT_STORAGEMNG_FS_EXCEPTION,
    HI_EVENT_STORAGEMNG_MOUNTED,
    HI_EVENT_STORAGEMNG_MOUNT_FAILED,
    HI_EVENT_STORAGEMNG_BUTT
} HI_EVENT_STORAGEMNG_E;

/** StorageMng Configure */
typedef struct hiSTORAGEMNG_CFG_S
{
    HI_CHAR szMntPath[HI_APPCOMM_MAX_PATH_LEN]; /**<storage mounted path */
    HI_CHAR szDevPath[HI_APPCOMM_MAX_PATH_LEN]; /**<storage device path */
} HI_STORAGEMNG_CFG_S;

/** format preproc callback */
typedef HI_S32 (*HI_STORAGEMNG_FORMAT_PREPROC_CALLBACK_FN_PTR)(const HI_CHAR *pazMountPath);

/** storage cbset */
typedef struct hiSTORAGEMNG_CALLBACK_S
{
    HI_STORAGEMNG_FORMAT_PREPROC_CALLBACK_FN_PTR pfnFormatPreProc;
}HI_STORAGEMNG_CALLBACK_S;

/**
 * @brief     register event
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_RegisterEvent(HI_VOID);

/**
 * @brief     create storagemng instance
 * @param[in] pstCfg : storagemng configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_Create(const HI_STORAGEMNG_CFG_S *pstCfg,const HI_STORAGEMNG_CALLBACK_S *pstCallback);

/**
 * @brief     destroy storagemng instance
 * @param[in] pszMntPath : storage mount path, eg. /app/sd
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_Destroy(const HI_CHAR* pszMntPath);

/**
 * @brief     get storage instance state
 * @param[in] pszMntPath : storage mount path
 * @param[out]penState : storage state
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_GetState(const HI_CHAR* pszMntPath, HI_STORAGE_STATE_E *penState);

/**
 * @brief     get the filesystem information
 * @param[in] pszMntPath : storage mount path
 * @param[out]pstFSInfo : filesystem information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_GetFSInfo(const HI_CHAR* pszMntPath, HI_STORAGE_FS_INFO_S *pstFSInfo);

/**
 * @brief     get the device information
 * @param[in] pszMntPath : storage mount path
 * @param[out]pstDevInfo : storage device information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_GetDevInfo(const HI_CHAR* pszMntPath, HI_STORAGE_DEV_INFO_S* pstDevInfo);

/**
 * @brief     format filesystem
 * @param[in] pszMntPath : storage mount path
 * @param[in] u64ClusterSize_KB : cluster size in KB
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_Format(const HI_CHAR* pszMntPath,HI_FSTOOL_FORMAT_MODE_E enMode);

/**
 * @brief     pause storagemng instance
 * @param[in] pszMntPath : storage mount path
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_Pause(const HI_CHAR* pszMntPath);

/**
 * @brief     resume storagemng instance
 * @param[in] pszMntPath : storage mount path
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/2
 */
HI_S32 HI_STORAGEMNG_Resume(const HI_CHAR* pszMntPath);

/** @}*/  /** <!-- ==== STORAGEMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_STORAGEMNG_H_ */

