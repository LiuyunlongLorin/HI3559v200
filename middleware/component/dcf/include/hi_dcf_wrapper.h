/**
* Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_dcf_wrapper.h
* @brief     dcf module header file
* @author    HiMobileCam middleware develop team
* @date      2017.11.07
*/
#ifndef _HI_DCF_WRAPPER_H_
#define _HI_DCF_WRAPPER_H_
#include "hi_mw_type.h"
#include "hi_defs.h"
#include "hi_error_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
typedef enum hiAPP_DCF_WRAPPER_ERR_CODE_E {
    APP_DCF_WRAPPER_ERR_NULL_PTR = 0x40,
    APP_DCF_WRAPPER_ERR_ILLEGAL_PARAM = 0x41,
    APP_DCF_WRAPPER_ERR_REINIT = 0x42,
    APP_DCF_WRAPPER_ERR_NOT_INIT = 0x43,

    APP_DCF_WRAPPER_ERR_INVALID_INDEX = 0x50,
    APP_DCF_WRAPPER_ERR_NOT_ENOUGH_INDEX = 0x51,
    APP_DCF_WRAPPER_ERR_INSTANCE_NOT_DESTROY = 0x52,
    APP_DCF_WRAPPER_ERR_SCAN_THREAD_CREATE_FAILURE = 0x53,
    APP_DCF_WRAPPER_ERR_LAST_DIR_NOT_SCANDONE = 0x54,
    APP_DCF_WRAPPER_ERR_FIRST_DIR_NOT_SCANDONE = 0x55,
    APP_DCF_WRAPPER_ERR_ALL_DIR_NOT_SCANDONE = 0x56,
    APP_DCF_WRAPPER_ERR_SCAN_FAILURE = 0x57,

    APP_DCF_WRAPPER_ERR_ROOT_ENOTDIR = 0x60,
    APP_DCF_WRAPPER_ERR_ROOT_ENOENT = 0x61,
    APP_DCF_WRAPPER_ERR_ROOT_ALREADY_OCCUPIED = 0x62,
    APP_DCF_WRAPPER_ERR_IMAGEROOT_ENOTDIR = 0x63,
    APP_DCF_WRAPPER_ERR_IMAGEROOT_ENOENT = 0x64,
    APP_DCF_WRAPPER_ERR_DIR_ENOTDIR = 0x65,
    APP_DCF_WRAPPER_ERR_DIR_ENOENT = 0x66,
    APP_DCF_WRAPPER_ERR_DIR_NOT_EXIST = 0x67,

    APP_DCF_WRAPPER_ERR_MAKE_DIR_FAILURE = 0x71,
    APP_DCF_WRAPPER_ERR_NOT_ENOUGH_DIR = 0x72,

    APP_DCF_WRAPPER_ERR_NOT_ENOUGH_SIZE = 0x80,
    APP_DCF_WRAPPER_ERR_NOT_ENOUGH_MEMORY = 0x81,
    APP_DCF_WRAPPER_ERR_SYSTEM_CALL_FAILURE = 0x82,

    APP_DCF_WRAPPER_ERR_EMPTY_DB = 0x90,
    APP_DCF_WRAPPER_ERR_NO_NEXT = 0x91,
    APP_DCF_WRAPPER_ERR_NO_PREV = 0x92,

    APP_DCF_WRAPPER_ERR_EMPTY_GRP = 0xa0,
    APP_DCF_WRAPPER_ERR_UNKNOW_CUR_GRP = 0xa1,

    APP_DCF_WRAPPER_ERR_UNKNOW_CUR_OBJ = 0xb0,
    APP_DCF_WRAPPER_ERR_INVALID_OBJ_TYPE_INDEX = 0xb1,
    APP_DCF_WRAPPER_ERR_BUSY_OBJ = 0xb2,
    APP_DCF_WRAPPER_ERR_OBJ_NOT_EXIST = 0xb3,
    APP_DCF_WRAPPER_ERR_OBJ_ALREADY_EXIST = 0xb4,

    APP_DCF_WRAPPER_ERR_INVALID_FILEPATH = 0xc1,
    APP_DCF_WRAPPER_ERR_NOT_MAIN_FILE = 0xc2,

    APP_DCF_WRAPPER_ERR_BUTT = 0xFF
} HI_APP_DCF_WRAPPER_ERR_CODE_E;

#define HI_ERR_DCF_WRAPPER_NULL_PTR                   HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NULL_PTR)
#define HI_ERR_DCF_WRAPPER_ILLEGAL_PARAM              HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_ILLEGAL_PARAM)
#define HI_ERR_DCF_WRAPPER_REINIT                     HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_REINIT)
#define HI_ERR_DCF_WRAPPER_NOT_INIT                   HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_INIT)
#define HI_ERR_DCF_WRAPPER_INVALID_INDEX              HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_INVALID_INDEX)
#define HI_ERR_DCF_WRAPPER_NOT_ENOUGH_INDEX           HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_ENOUGH_INDEX)
#define HI_ERR_DCF_WRAPPER_INSTANCE_NOT_DESTROY       HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_INSTANCE_NOT_DESTROY)
#define HI_ERR_DCF_WRAPPER_SCAN_THREAD_CREATE_FAILURE HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_SCAN_THREAD_CREATE_FAILURE)
#define HI_ERR_DCF_WRAPPER_LAST_DIR_NOT_SCANDONE      HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_LAST_DIR_NOT_SCANDONE)
#define HI_ERR_DCF_WRAPPER_FIRST_DIR_NOT_SCANDONE     HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_FIRST_DIR_NOT_SCANDONE)
#define HI_ERR_DCF_WRAPPER_ALL_DIR_NOT_SCANDONE       HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_ALL_DIR_NOT_SCANDONE)
#define HI_ERR_DCF_WRAPPER_SCAN_FAILURE               HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_SCAN_FAILURE)

#define HI_ERR_DCF_WRAPPER_ROOT_ENOTDIR          HI_APP_DEF_ERR(HI_APPID_DCF, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_ROOT_ENOTDIR)
#define HI_ERR_DCF_WRAPPER_ROOT_ENOENT           HI_APP_DEF_ERR(HI_APPID_DCF, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_ROOT_ENOENT)
#define HI_ERR_DCF_WRAPPER_ROOT_ALREADY_OCCUPIED HI_APP_DEF_ERR(HI_APPID_DCF, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_ROOT_ALREADY_OCCUPIED)
#define HI_ERR_DCF_WRAPPER_IMAGEROOT_ENOTDIR     HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_IMAGEROOT_ENOTDIR)
#define HI_ERR_DCF_WRAPPER_IMAGEROOT_ENOENT      HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_IMAGEROOT_ENOENT)
#define HI_ERR_DCF_WRAPPER_DIR_ENOTDIR           HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_DIR_ENOTDIR)
#define HI_ERR_DCF_WRAPPER_DIR_ENOENT            HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_DIR_ENOENT)
#define HI_ERR_DCF_WRAPPER_DIR_NOT_EXIST         HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_DIR_NOT_EXIST)

#define HI_ERR_DCF_WRAPPER_MAKE_DIR_FAILURE    HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_MAKE_DIR_FAILURE)
#define HI_ERR_DCF_WRAPPER_NOT_ENOUGH_DIR      HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_ENOUGH_DIR)
#define HI_ERR_DCF_WRAPPER_NOT_ENOUGH_SIZE     HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_ENOUGH_SIZE)
#define HI_ERR_DCF_WRAPPER_NOT_ENOUGH_MEMORY   HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_ENOUGH_MEMORY)
#define HI_ERR_DCF_WRAPPER_SYSTEM_CALL_FAILURE HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_SYSTEM_CALL_FAILURE)

#define HI_ERR_DCF_WRAPPER_EMPTY_DB               HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_EMPTY_DB)
#define HI_ERR_DCF_WRAPPER_NO_NEXT                HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NO_NEXT)
#define HI_ERR_DCF_WRAPPER_NO_PREV                HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NO_PREV)
#define HI_ERR_DCF_WRAPPER_EMPTY_GRP              HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_EMPTY_GRP)
#define HI_ERR_DCF_WRAPPER_UNKNOW_CUR_GRP         HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_UNKNOW_CUR_GRP)
#define HI_ERR_DCF_WRAPPER_UNKNOW_CUR_OBJ         HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_UNKNOW_CUR_OBJ)
#define HI_ERR_DCF_WRAPPER_INVALID_OBJ_TYPE_INDEX HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_INVALID_OBJ_TYPE_INDEX)
#define HI_ERR_DCF_WRAPPER_BUSY_OBJ               HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_BUSY_OBJ)
#define HI_ERR_DCF_WRAPPER_OBJ_NOT_EXIST          HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_OBJ_NOT_EXIST)
#define HI_ERR_DCF_WRAPPER_OBJ_ALREADY_EXIST      HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_OBJ_ALREADY_EXIST)

#define HI_ERR_DCF_WRAPPER_INVALID_FILEPATH HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_INVALID_FILEPATH)
#define HI_ERR_DCF_WRAPPER_NOT_MAIN_FILE    HI_APP_DEF_ERR(HI_APPID_DCF_WRAPPER, APP_ERR_LEVEL_ERROR, APP_DCF_WRAPPER_ERR_NOT_MAIN_FILE)

#define HI_DCF_WRAPPER_MAX_INSTANCE          (4)
#define HI_DCF_WRAPPER_MAX_OBJ_TYPE_CNT      (16)
#define HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ   (6)
#define HI_DCF_WRAPPER_MAX_FILE_PATH_LENGTH  (256)
#define HI_DCF_WRAPPER_MAX_ROOT_PATH_LENGTH  (229) /* "/DCIM/XXXHSCAM/NOMR0001.MP4" has occupied 27bit */
#define HI_DCF_WRAPPER_DIR_FREE_CHAR_LENGTH  (6)
#define HI_DCF_WRAPPER_FILE_FREE_CHAR_LENGTH (5)
#define HI_DCF_WRAPPER_FILE_EXTENSION_LENGTH (4)

typedef HI_U8 HI_DCF_WRAPPER_INDEX;

typedef struct hiDCF_WRAPPER_OBJ_S {
    HI_U8 u8FileCnt;
    HI_CHAR aszFilePath[HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ][HI_DCF_WRAPPER_MAX_FILE_PATH_LENGTH];
} HI_DCF_WRAPPER_OBJ_S;

typedef struct hiDCF_WRAPPER_OBJ_LIST_S {
    HI_U32 u32ObjCnt;
    HI_DCF_WRAPPER_OBJ_S *apstObj;
} HI_DCF_WRAPPER_OBJ_LIST_S;

typedef struct hiDCF_WRAPPER_GRP_S {
    HI_U8 u8ObjTypIdx;  // index of object type set in HI_DCF_WRAPPER_NAMERULE_CFG_S
    HI_U16 u16Gnum;     // group number, and it will be invalid when the free char has no group number field
    HI_U16 u16ObjCnt;   // DCF object count in this group
} HI_DCF_WRAPPER_GRP_S;

typedef enum hiDCF_WRAPPER_SCAN_STATE_E {
    HI_DCF_WRAPPER_SCAN_STATE_LAST_DIR_DONE = 0x01,
    HI_DCF_WRAPPER_SCAN_STATE_FIRST_DIR_DONE = 0x02,
    HI_DCF_WRAPPER_SCAN_STATE_ALL_DIR_DONE = 0x03,
    HI_DCF_WRAPPER_SCAN_STATE_FAILURE = 0x04,
    HI_DCF_WRAPPER_SCAN_STATE_BUTT
} HI_DCF_WRAPPER_SCAN_STATE_E;

typedef struct hiDCF_WRAPPER_FILE_TYPE_S {
    HI_CHAR aszTypChar[HI_DCF_WRAPPER_FILE_FREE_CHAR_LENGTH];
    HI_CHAR aszExtension[HI_DCF_WRAPPER_FILE_EXTENSION_LENGTH];
} HI_DCF_WRAPPER_FILE_TYPE_S;

typedef struct hiDCF_WRAPPER_OBJ_TYPE_S {
    HI_U8 u8FileCnt;
    HI_DCF_WRAPPER_FILE_TYPE_S astFileTbl[HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ];
} HI_DCF_WRAPPER_OBJ_TYPE_S;

typedef struct hiDCF_WRAPPER_NAMERULE_CFG_S {
    HI_CHAR aszRoot[HI_DCF_WRAPPER_MAX_ROOT_PATH_LENGTH];
    HI_CHAR aszDirFreeChar[HI_DCF_WRAPPER_DIR_FREE_CHAR_LENGTH];
    HI_U8 u8ObjTypCnt;
    HI_DCF_WRAPPER_OBJ_TYPE_S astObjTypTbl[HI_DCF_WRAPPER_MAX_OBJ_TYPE_CNT];
} HI_DCF_WRAPPER_NAMERULE_CFG_S;

/**
*  @brief  DCF scan event callback, s32ScanErrCode is error code of scan failure, and only valid when enState is HI_DCF_WRAPPER_SCAN_STATE_FAILURE
*/
typedef HI_VOID (*HI_DCF_WRAPPER_ON_SCAN_EVENT_PFN)(HI_DCF_WRAPPER_INDEX iDCF,
                HI_DCF_WRAPPER_SCAN_STATE_E enState, HI_S32 s32ScanErrCode);

/**
*   @brief  init DCF module.
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_Init(HI_VOID);

/**
*   @brief  deinit DCF module.
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_DeInit(HI_VOID);

/**
*   @brief  create a DCF instance. 4 DCF instances are supported.
*   @param[in] pfnScanEvent : HI_DCF_WRAPPER_ON_SCAN_EVENT_PFN: scan event callback func
*   @param[in] pstCfg : HI_DCF_WRAPPER_NAMERULE_CFG_S*: name rule configure
*   @param[out] piDCF : HI_DCF_WRAPPER_INDEX*: DCF instance index
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_Create(const HI_DCF_WRAPPER_ON_SCAN_EVENT_PFN pfnScanEvent,
                             const HI_DCF_WRAPPER_NAMERULE_CFG_S *pstCfg, HI_DCF_WRAPPER_INDEX *piDCF);

/**
*   @brief  destroy a DCF instance.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_Destroy(HI_DCF_WRAPPER_INDEX iDCF);

/**
*   @brief  get first group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstGrp : HI_DCF_WRAPPER_GRP_S*: DCF group
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetFirstGrp(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_GRP_S *pstGrp);

/**
*   @brief  get last group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstGrp : HI_DCF_WRAPPER_GRP_S*: DCF group
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetLastGrp(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_GRP_S *pstGrp);

/**
*   @brief  get u32Interval th group after current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] u32Interval : HI_U32: the interval from current group
*   @param[out] pstGrp : HI_DCF_WRAPPER_GRP_S*: DCF group
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetNextGrp(HI_DCF_WRAPPER_INDEX iDCF, HI_U32 u32Interval, HI_DCF_WRAPPER_GRP_S *pstGrp);

/**
*   @brief  get u32Interval th group before current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] u32Interval : HI_U32: the interval from current group
*   @param[out] pstGrp : HI_DCF_WRAPPER_GRP_S*: DCF group
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetPrevGrp(HI_DCF_WRAPPER_INDEX iDCF, HI_U32 u32Interval, HI_DCF_WRAPPER_GRP_S *pstGrp);

/**
*   @brief  delete current group. Include notes in database and file entity in storage device.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_DelCurGrp(HI_DCF_WRAPPER_INDEX iDCF);

/**
*   @brief  get total group count.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pu32Cnt : HI_U32*: group count
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetTotalGrpCnt(HI_DCF_WRAPPER_INDEX iDCF, HI_U32 *pu32Cnt);

/**
*   @brief  get group count of specified object type.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] u8ObjTypIndex : HI_U8: DCF object type index
*   @param[out] pu32Cnt : HI_U32*: group count
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetGrpCnt(HI_DCF_WRAPPER_INDEX iDCF, HI_U8 u8ObjTypIndex, HI_U32 *pu32Cnt);

/**
*   @brief  get first object in current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstObj : HI_DCF_WRAPPER_OBJ_S*: DCF object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetCurGrpFirstObj(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_OBJ_S *pstObj);

/**
*   @brief  get last object in current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstObj : HI_DCF_WRAPPER_OBJ_S*: DCF object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetCurGrpLastObj(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_OBJ_S *pstObj);

/**
*   @brief  get next object from current object of current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstObj : HI_DCF_WRAPPER_OBJ_S*: DCF object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetCurGrpNextObj(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_OBJ_S *pstObj);

/**
*   @brief  get prev object from current object of current group.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pstObj : HI_DCF_WRAPPER_OBJ_S*: DCF object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetCurGrpPrevObj(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_OBJ_S *pstObj);

/**
*   @brief  get next object list from the given object.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] pszMainFile : HI_CHAR*: main file of a DCF object, and it identifies the given object
*   @param[in] u32Cnt : HI_U32: count of object to be acquired
*   @param[out] pstObjList : HI_DCF_WRAPPER_OBJ_LIST_S*: DCF object list
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetNextObjList(HI_DCF_WRAPPER_INDEX iDCF, const HI_CHAR *pszMainFile, HI_U32 u32Cnt,
                                     HI_DCF_WRAPPER_OBJ_LIST_S *pstObjList);

/**
*   @brief  get prev object list from the given object.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] pszMainFile : HI_CHAR*: main file of DCF object, and it identifies the given object
*   @param[in] u32Cnt : HI_U32: count of object to be acquired
*   @param[out] pstObjList : HI_DCF_WRAPPER_OBJ_LIST_S*: DCF object list
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetPrevObjList(HI_DCF_WRAPPER_INDEX iDCF, const HI_CHAR *pszMainFile, HI_U32 u32Cnt,
                                     HI_DCF_WRAPPER_OBJ_LIST_S *pstObjList);

/**
*   @brief  add object to a DCF instance.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] pszMainFile : HI_CHAR*: main file of DCF object, and it identifies a unique object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_AddObj(HI_DCF_WRAPPER_INDEX iDCF, const HI_CHAR *pszMainFile);

/**
*   @brief  delete object from a DCF instance. Include notes in database and file entity in storage device.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] pszMainFile : HI_CHAR*: main file of DCF object, and it identifies a unique object
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_DelObj(HI_DCF_WRAPPER_INDEX iDCF, const HI_CHAR *pszMainFile);

/**
*   @brief  get total object count.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[out] pu32Cnt : HI_U32*: object count
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetTotalObjCnt(HI_DCF_WRAPPER_INDEX iDCF, HI_U32 *pu32Cnt);

/**
*   @brief  get object count of specified object type.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] u8ObjTypIndex : HI_U8: DCF object type index
*   @param[out] pu32Cnt : HI_U32*: object count
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetObjCnt(HI_DCF_WRAPPER_INDEX iDCF, HI_U8 u8ObjTypIndex, HI_U32 *pu32Cnt);

/**
*   @brief  get file path list a new object.
*   @param[in] iDCF : HI_DCF_WRAPPER_INDEX: DCF instance index
*   @param[in] bNewGrp : HI_BOOL: whether to create a new grp
*   @param[in] u8ObjTypIndex : HI_U8: DCF object type index
*   @param[out] pstObj : HI_U32*: object file path list
*   @retval  on success, 0 is returned. On error, error code returned.
*/
HI_S32 HI_DCF_WRAPPER_GetNewFilePaths(HI_DCF_WRAPPER_INDEX iDCF, HI_BOOL bNewGrp, HI_U8 u8ObjTypIndex,
                                      HI_DCF_WRAPPER_OBJ_S *pstObj);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* _HI_DCF_WRAPPER_H_ */
