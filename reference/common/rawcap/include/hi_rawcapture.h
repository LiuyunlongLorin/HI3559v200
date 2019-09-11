/**
 * @file    hi_rawcapture.h
 * @brief   raw capture module struct and interface declaration
 *
 * Copyright (c) 2018 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/8
 * @version   1.0

 */
#ifndef __HI_RAWCAP_H__
#define __HI_RAWCAP_H__

#include "hi_mapi_comm_define.h"
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**\addtogroup     RAWCAP */
/**@{ */  /**<!-- [RAWCAP] */

/** error code define */
#define HI_RAWCAP_EINVAL                     HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_EINVAL)                  /**<illegal parameter */
#define HI_RAWCAP_ENOINIT                    HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_ENOINIT)                 /**<module not inited */
#define HI_RAWCAP_EBUSY                      HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_EBUSY)                   /**<task is busy */
#define HI_RAWCAP_EINITIALIZED               HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_EINITIALIZED)            /**<already initialized */
#define HI_RAWCAP_ETASK_NOT_CREATED          HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_ERRNO_CUSTOM_BOTTOM + 1) /**<task not created */
#define HI_RAWCAP_ETASK_NOT_STARTED          HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_ERRNO_CUSTOM_BOTTOM + 2) /**<task not started */
#define HI_RAWCAP_ETIMEOUT                   HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_ERRNO_CUSTOM_BOTTOM + 3) /**<timeout */
#define HI_RAWCAP_EMAXTASK                   HI_APPCOMM_ERR_ID(HI_APP_MOD_RAWCAP, HI_ERRNO_CUSTOM_BOTTOM + 4) /**<beyond maximum task */

#define HI_RAWCAP_MAX_FILENAME_LEN           (160)

typedef struct tagRAWCAP_FILENAME_S
{
    HI_CHAR szRaw_FileName[HI_RAWCAP_MAX_FILENAME_LEN];
    HI_CHAR szYUV_FileName[HI_RAWCAP_MAX_FILENAME_LEN];
    HI_CHAR szDebugInfo_FileName[HI_RAWCAP_MAX_FILENAME_LEN];
} HI_RAWCAP_FILENAME_S;

/** rawcap callback: get file name */
typedef HI_S32 (*HI_RAWCAP_GET_FILENAME_CALLBACK_FN_PTR)(HI_RAWCAP_FILENAME_S* pstFilename, HI_VOID* pvPrivateData);

/** filename struct */
typedef struct hiRAWCAP_GET_FILENAME_S
{
    HI_RAWCAP_GET_FILENAME_CALLBACK_FN_PTR pfnGetFileNameCb; /**<get filename callback */
    HI_VOID *pvPrivateData; /**<get filename callback private data */
} HI_RAWCAP_GET_FILENAME_S;

/** data source struct */
typedef struct hiRAWCAP_DATA_SOURCE_S
{
    HI_HANDLE VcapPipeHdl;    /**<handle of vcap pipe, HI_INVALID_HANDLE: not support */
    HI_HANDLE VcapPipeChnHdl; /**<handle of vcap pipe's channel, HI_INVALID_HANDLE: not support */
    HI_HANDLE VpssHdl;        /**<handle of vpss, HI_INVALID_HANDLE: not support */
    HI_HANDLE VPortHdl;       /**<handle of vport, HI_INVALID_HANDLE: not support */
    HI_BOOL bDumpYUV;         /**<DumpYUV or not */
    HI_BOOL bDumpDebugInfo;   /**<DumpDebugInfo or not */
} HI_RAWCAP_DATA_SOURCE_S;

typedef struct hiRAWCAP_TRIGGER_MODE_S
{
    HI_BOOL bDumpYUV;
    HI_BOOL bDumpBNR;
} HI_RAWCAP_TRIGGER_MODE_S;

/** rawcap task struct */
typedef struct hiRAWCAP_CFG_S
{
    HI_RAWCAP_GET_FILENAME_S stGetFilenameCb;
    HI_RAWCAP_DATA_SOURCE_S stDataSource;
    HI_U32 u32RawDepth;       /**<range [0,8] */
    HI_BOOL bRawFileSeparated;    /**<all raws once dump saved in one file or each. If true, save separated.*/
} HI_RAWCAP_CFG_S;

/**-------------------------------rawcap fuction declare-------------------------------- */
/**
* @brief          rawcap initialization
* @return 0     success,non-zero error code
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_Init(HI_VOID);

/**
* @brief          rawcap deinitialization
* @return 0     success,non-zero error code
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_Deinit(HI_VOID);

/**
* @brief          create rawcap task
* @param[in]   pstRawCapCfg: rawcap task attribute, e.g. filenameCb, datasource, rawdepth
* @param[out] pHdl: rawcap task handle
* @return 0     success,non-zero error code
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_CreateTask(const HI_RAWCAP_CFG_S *pstRawCapCfg, HI_HANDLE *pHdl);

/**
* @brief          destroy rawcap task
* @param[in]   Hdl: rawcap task handle
* @return 0     success,non-zero error code
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_DestroyTask(HI_HANDLE Hdl);

/**
* @brief          start periodical rawcap task
* @param[in]   Hdl: rawcap task handle
* @param[in]   u32Interval_ms: Interval_ms, The minimum is 10000ms.
* @param[in]   u32Count: the framecount of YUV and raw when dump once
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_StartTask(HI_HANDLE Hdl, HI_U32 u32Interval_ms, HI_U32 u32Count);

/**
* @brief          stop periodical rawcap task
* @param[in]   Hdl: rawcap task handle
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_StopTask(HI_HANDLE Hdl);

/**
* @brief          start rawcap task, dump once
* @param[in]   Hdl: rawcap task handle
* @param[in]   u32Count: the framecount of YUV and raw when dump once
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_Trigger(HI_HANDLE Hdl, HI_U32 u32Count, HI_RAWCAP_TRIGGER_MODE_S* pstTriggerMode);

/**
* @brief          Save PostProcess Function Callback
* @param[in]   pVPortYUV: YUV Data
* @param[in]   pPrivateData: Private Data
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_SaveYUV(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S* pVPortYUV, HI_VOID* pPrivateData);

/**
* @brief          Save PostProcess Function Callback
* @param[in]   pVPortRaw: Raw Data
* @param[in]   pPrivateData: Private Data
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2018/05/07
*/
HI_S32 HI_RAWCAP_SaveBNR(HI_HANDLE GrpHdl, HI_HANDLE PortHdl, HI_MAPI_FRAME_DATA_S* pVPortRaw, HI_VOID* pPrivateData);


/**@}*/  /**<!-- ==== RAWCAP End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_RAWCAP_H__ */
