/**
* Copyright (C), 2016-2032, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_storage.h
* @brief     storage struct and interface
* @author    HiMobileCam Develop Team
* @date      2017.11.30
*/

#ifndef __HI_STORAGE_H__
#define __HI_STORAGE_H__
#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/** \addtogroup     STORAGE */
/** @{ */  /** <!-- [STORAGE] */
typedef enum hiAPP_STORAGE_ERR_CODE_E
{
    APP_STORAGE_ERR_SYSTEM_CALL_FAILURE            = 0x40,
    APP_STORAGE_ERR_NULL_PTR                       = 0x41,

    APP_STORAGE_ERR_ILLEGAL_PARAM                  = 0x50,
    APP_STORAGE_ERR_INVALID_HANDLE                  = 0x51,
    APP_STORAGE_ERR_INST_CNT_UPPER_LIMIT           = 0x52,

    APP_STORAGE_ERR_DEV_DISCONNECT                 = 0x60,
    APP_STORAGE_ERR_DEV_ERROR                      = 0x61,

    APP_STORAGE_ERR_FS_CHECKING                    = 0x70,
    APP_STORAGE_ERR_FS_EXCEPTION                   = 0x71,
    APP_STORAGE_ERR_FS_NOT_MOUNTED                 = 0x72,

    APP_STORAGE_ERR_RUNNING                        = 0x80,
    APP_STORAGE_ERR_PAUSED                         = 0x81,

    APP_STORAGE_ERR_BUTT = 0xFF
} HI_APP_STORAGE_ERR_CODE_E;

#define HI_ERR_STORAGE_SYSTEM_CALL_FAILURE                 HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_SYSTEM_CALL_FAILURE)
#define HI_ERR_STORAGE_NULL_PTR                            HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_NULL_PTR)
#define HI_ERR_STORAGE_ILLEGAL_PARAM                       HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_ILLEGAL_PARAM)
#define HI_ERR_STORAGE_INVALID_HANDLE                       HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_INVALID_HANDLE)
#define HI_ERR_STORAGE_INST_CNT_UPPER_LIMIT                HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_INST_CNT_UPPER_LIMIT)

#define HI_ERR_STORAGE_DEV_DISCONNECT                      HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_DEV_DISCONNECT)
#define HI_ERR_STORAGE_DEV_ERROR                           HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_DEV_ERROR)

#define HI_ERR_STORAGE_FS_CHECKING                         HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_FS_CHECKING)
#define HI_ERR_STORAGE_FS_EXCEPTION                        HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_FS_EXCEPTION)
#define HI_ERR_STORAGE_FS_NOT_MOUNTED                      HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_FS_NOT_MOUNTED)

#define HI_ERR_STORAGE_RUNNING                             HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_RUNNING)
#define HI_ERR_STORAGE_PAUSED                              HI_APP_DEF_ERR(HI_APPID_STORAGE,APP_ERR_LEVEL_ERROR,APP_STORAGE_ERR_PAUSED)

#define HI_STORAGE_PATH_LEN_MAX        (256)
#define HI_STORAGE_INST_CNT_MAX        (1)

/** storage instance configuration*/
typedef struct hiSTORAGE_CFG_S
{
    HI_U8 u8DevPortNo;                                  /**Device Port Number, such as 0 identifying board host port MCI0*/
    HI_U8 u8DevErrCountThr;                             /**Device error count threshold, if the device error count over u8DevErrCountThr, the device is  considered as  broken*/
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX];  /**<storage partition path */
    HI_CHAR aszMountPath[HI_STORAGE_PATH_LEN_MAX];      /**<storage mounted path */
} HI_STORAGE_CFG_S;


/** storage instance state enum */
typedef enum hiSTORAGE_STATE_E
{
    HI_STORAGE_STATE_DEV_UNPLUGGED = 0x00,      /**<device already plugout */
    HI_STORAGE_STATE_DEV_CONNECTING ,           /**<device connecting */
    HI_STORAGE_STATE_DEV_ERROR,                 /**<sd card error */
    HI_STORAGE_STATE_FS_CHECKING,               /**<device doing fscheck */
    HI_STORAGE_STATE_FS_CHECK_FAILED,           /**<device fscheck failed */
    HI_STORAGE_STATE_FS_EXCEPTION,              /**<device file system exception */
    HI_STORAGE_STATE_MOUNTED,                   /**<device mounted */
    HI_STORAGE_STATE_MOUNT_FAILED,              /**<device mount fail*/
    HI_STORAGE_STATE_IDEL                       /**init state*/
} HI_STORAGE_STATE_E;

/**storage device transmission speed enum*/
typedef enum hiSTORAGE_TRANSMISSION_SPEED_E
{
    HI_STORAGE_TRANSMISSION_SPEED_1_4M = 0x00,      /**device transfer speed above 1-4  MB/s */
    HI_STORAGE_TRANSMISSION_SPEED_4_10M  ,          /**device transfer speed above 4-10     MB/s */
    HI_STORAGE_TRANSMISSION_SPEED_10_30M ,          /**device transfer speed above 10-30 MB/s */
    HI_STORAGE_TRANSMISSION_SPEED_30_50M ,          /**device transfer speed above 30-50 MB/s */
    HI_STORAGE_TRANSMISSION_SPEED_50_100M ,         /**device transfer speed above 50-100MB/s */
    HI_STORAGE_TRANSMISSION_SPEED_EXCEED_100M ,     /**device transfer speed above 100MB/s and faster*/
    HI_STORAGE_TRANSMISSION_SPEED_BUTT              /***others**/
} HI_STORAGE_TRANSMISSION_SPEED_E;


typedef struct hiSTORAGE_DEV_INFO_S
{
    HI_CHAR aszDevType[HI_STORAGE_PATH_LEN_MAX];         /**device type,such as SD or MMC*/
    HI_CHAR aszCID[HI_STORAGE_PATH_LEN_MAX];             /**device id info */
    HI_CHAR aszWorkMode[HI_STORAGE_PATH_LEN_MAX];        /**device work mode*/
    HI_CHAR aszWorkClock[HI_STORAGE_PATH_LEN_MAX];       /**device actual work clock*/
    HI_STORAGE_TRANSMISSION_SPEED_E enTranSpeed;         /**device transmission rate info */
} HI_STORAGE_DEV_INFO_S;

/**storage file system  information */
typedef struct hiSTORAGE_FS_INFO_S
{
    HI_U64 u64ClusterSize;                      /**<DEV partition cluster size(unit bytes) */
    HI_U64 u64TotalSize;                        /**<DEV partition total space size(unit bytes) */
    HI_U64 u64AvailableSize;                    /**<DEV partition free space size(unit bytes) */
    HI_U64 u64UsedSize;                         /**<DEV partition used space size(unit bytes) */
} HI_STORAGE_FS_INFO_S;

//errorcode should accord with fstool.
typedef enum hiSTORAGE_FSTOOL_ERR_E
{
    HI_STORAGE_FSTOOL_SUCCESS=0x00,             /* success                                      */
    HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM=0x201 ,       /* Illegal param                               */
    HI_STORAGE_FSTOOL_ERR_OPEN_FAIL=0x202,            /* Open operation error                     */
    HI_STORAGE_FSTOOL_ERR_READ_FAIL=0x203,            /* Read FATs informations error         */
    HI_STORAGE_FSTOOL_ERR_WRITE_FAIL=0x204 ,          /* Update FATs informations error      */
    HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY=0x205,    /* Not enough memory when fsck       */
    HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT=0x206,     /* Fs type unsupport  */
    HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION=0x207,         /* FS exception and cannot repair  */
    HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY=0x208,    /* FATs Fragment severely                */
    HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION=0x209,     /* Device is exception so can not format a FAT32 file system on it */
    HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE=0x210,  /* System call failure */
    HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER=0x211,  /* Cluster size is not 64K */
    HI_STORAGE_FSTOOL_ERR_ERR_BUTT
}HI_STORAGE_FSTOOL_ERR_E;

typedef struct hi_STORAGE_FS_TOOL_S
{
    HI_STORAGE_FSTOOL_ERR_E (*pfnCheck)(const HI_CHAR *pszPartitionPath);//ps32FSSitutaion such as FS Exception, FS fragment serious and so on
    HI_STORAGE_FSTOOL_ERR_E (*pfnFormat)(const HI_CHAR *pszPartitionPath, HI_U64 u64ClusterSize); //u64ClusterSize unit is bytes, should be 512*2^n, such as 1024B
    HI_CHAR aszFSType[HI_STORAGE_PATH_LEN_MAX]; /**such as vfat/exfat*/
}HI_STORAGE_FS_TOOL_S;

typedef HI_VOID (*HI_STORAGE_ON_STATE_CHANGE_PFN)(HI_MW_PTR pStorage, const HI_CHAR* pszPartitionPath, HI_STORAGE_STATE_E enState, HI_S32 s32ErrCode);

/**
*   @brief  register fstool(fscheck & fsformat) to storage.
*   @user should register at last one filesystem type(such as vfat) fstool before create storage.
*   @param[in] pstFSTool : HI_STORAGE_FS_TOOL_S*: fstool
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_RegisterFSTool(const HI_STORAGE_FS_TOOL_S* pstFSTool);

/**
*   @brief  unregister FSTool(FSCheck & FSFormat) to storage.
*   @user should unregister fstools after destroy all storage intance.
*   @param[in] pszFSType : HI_CHAR*: filesystem type of fstool
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_UnregisterFSTool(const HI_CHAR* pszFSType);

/**
*   @brief  create a storage intance.
*   @param[in] pstCfg : HI_STORAGE_CFG_S*: configuration of storage intance
*   @param[in] pfnOnStateChange : HI_STORAGE_ON_STATE_CHANGE_PFN: storage state change callback func
*   @param[out] ppStorage : HI_MW_PTR*: point to the handler of new storage intance
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_Create(const HI_STORAGE_CFG_S* pstCfg, const HI_STORAGE_ON_STATE_CHANGE_PFN pfnOnStateChange, HI_MW_PTR* ppStorage);

/**
*   @brief  destroy a storage intance.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_Destroy(HI_MW_PTR pStorage);

/**
*   @brief  get state of storage intance specified with pStorage.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @param[out] penState : HI_STORAGE_STATE_E*: the storage intance state
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_GetState(HI_MW_PTR pStorage, HI_STORAGE_STATE_E* penState);

/**
*   @brief  get file system info of storage intance specified with pStorage.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @param[out] pstFSInfo : HI_STORAGE_FS_INFO_S*: the storage intance file system info
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_GetFSInfo(HI_MW_PTR pStorage, HI_STORAGE_FS_INFO_S* pstFSInfo);

/**
*   @brief  get device info of storage intance specified with pStorage.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @param[out] pstDevInfo : HI_STORAGE_DEV_INFO_S*: the storage intance device info
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_GetDevInfo(HI_MW_PTR pStorage, HI_STORAGE_DEV_INFO_S* pstDevInfo);

/**
*   @brief  fomat a new file system on the device(partition) of storage intance specified with pStorage.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @param[in] pszFSType : HI_CHAR*: file system type
*   @param[in] u64ClusterSize : HI_U64: file system cluster size
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_Format(HI_MW_PTR pStorage, const HI_CHAR* pszFSType, HI_U64 u64ClusterSize);

/**
*   @brief  pause storage intance specified with pStorage. if paused, the storage intance will not monitor and notify the device state change.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_Pause(HI_MW_PTR pStorage);

/**
*   @brief  resume storage intance specified with pStorage. the paused storage intance will monitor and notify the device state change after resumed.
*   @param[in] pStorage : HI_MW_PTR: handler of the storage intance
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_STORAGE_Resume(HI_MW_PTR pStorage);

/** @}*/  /** <!-- ==== STORAGE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /* End of #ifndef __HI_STORAGE_H__ */
