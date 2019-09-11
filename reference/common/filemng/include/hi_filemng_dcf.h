/**
 * @file    hi_filemng_dcf.h
 * @brief   DCF file manager struct and interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

 */
#ifndef __HI_FILEMNG_DCF_H__
#define __HI_FILEMNG_DCF_H__

#include "hi_dcf_wrapper.h"
#include "hi_filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     FILEMNG */
/** @{ */  /** <!-- [FILEMNG] */

/** struct define */
/** DCF file object configuration */
typedef struct hiFILEMNG_DCF_OBJ_CFG_S {
    HI_FILEMNG_FILE_TYPE_E enType;/**<type of the basic file in object */
    HI_U8 u8FileCnt;/**<file count in object */
    HI_CHAR szFileFreeChar[HI_DCF_WRAPPER_FILE_FREE_CHAR_LENGTH];/**<file name free characters */
    HI_CHAR aszExtension[HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ][HI_DCF_WRAPPER_FILE_EXTENSION_LENGTH];/**<file name extensions list */
    HI_BOOL abNeedRepair[HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ];/**<file need repair or not */
} HI_FILEMNG_DCF_OBJ_CFG_S;

/** DCF configuration */
typedef  struct hiFILEMNG_DCF_CFG_S {
    HI_U32 u32WarningStage;/**<the stage of space full warning,unit:MB */
    HI_U8 u8ObjTypeCnt;/**<DCF file object type count. eg.9 */
    HI_CHAR szDirFreeChar[HI_DCF_WRAPPER_DIR_FREE_CHAR_LENGTH];/**<directory name free characterseg."HSCAM" */
    HI_FILEMNG_DCF_OBJ_CFG_S astObjCfgTbl[HI_DCF_WRAPPER_MAX_OBJ_TYPE_CNT];/**<DCF file object configuration table. eg.
    {
        {HI_FILEMNG_FILE_TYPE_RECORD,3,"NORM",{"MP4","LRV","THM"},{1,1,0}},
        {HI_FILEMNG_FILE_TYPE_RECORD,3,"SLOW",{"MP4","LRV","THM"},{1,1,0}},
        {HI_FILEMNG_FILE_TYPE_RECORD,3,"L",{"MP4","LRV","THM"},{1,1,0}},
        {HI_FILEMNG_FILE_TYPE_RECORD,3,"LPSE",{"MP4","LRV","THM"},{1,1,0}},
        {HI_FILEMNG_FILE_TYPE_PHOTO,2,"SING",{"JPG","DNG"},{0,0}},
        {HI_FILEMNG_FILE_TYPE_PHOTO,2,"DLAY",{"JPG","DNG"},{0,0}},
        {HI_FILEMNG_FILE_TYPE_PHOTO,1,"B",{"JPG"},{0}},
        {HI_FILEMNG_FILE_TYPE_PHOTO,1,"C",{"JPG"},{0}},
        {HI_FILEMNG_FILE_TYPE_PHOTO,1,"L",{"JPG"},{0}},
    }
    */
} HI_FILEMNG_DCF_CFG_S;

/** callback function for search group */
typedef HI_S32(*HI_FILEMNG_SRCH_GRP_CALLBACK_FN_PTR)(const HI_DCF_WRAPPER_GRP_S *pstGrp, HI_VOID *pvUserData);

/** callback function for search file object */
typedef HI_S32(*HI_FILEMNG_SRCH_FILEOBJ_CALLBACK_FN_PTR)(const HI_FILEMNG_FILEOBJ_S *pstFileObjInfo,
        HI_VOID *pvUserData);


/** function interface */
/**
 * @brief    register filemng event.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/29
 */
HI_S32 HI_FILEMNG_RegisterEvent(HI_VOID);

/**
 * @brief    init file manager.
 * @param[in] pstCfg:file manager comm configuration.
 * @param[in] pstDCF_Cfg:file manager DCF configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Init(const HI_FILEMNG_COMM_CFG_S *pstCfg, const HI_FILEMNG_DCF_CFG_S *pstDCF_Cfg);

/**
 * @brief    deinit file manager.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Deinit(HI_VOID);

/**
 * @brief    call this function to set the disk state.
 * @param[in] bAvailable:disk state.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SetDiskState(HI_BOOL bAvailable);

/**
 * @brief    call this function to check the disk space.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_CheckDiskSpace(HI_VOID);

/**
 * @brief    add a file to the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_AddFile(const HI_CHAR *pszFilePath);

/**
 * @brief    remove a file from the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_RemoveFile(const HI_CHAR *pszFilePath);

/**
 * @brief    mark a file with a symbol.
 * @param[in] pszFilePath:the file absolute path.
 * @param[in] u8Flag:the flag marked to the file.range:[0,255],0 unmark,1-255 special flag.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_MarkFile(const HI_CHAR *pszFilePath, HI_U8 u8Flag);

/**
 * @brief    create new file name to DCF.
 * @param[in] u8ObjCfgIdx:the object index according to the order in struct HI_APP_DCF_OBJ_CFG_S,range:[0,u8ObjTypeCnt].
 * @param[in] bNewGrp:is a new group or not.
 * @param[out] pstFileName:new file object name.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GenerateFileName(HI_U8 u8ObjCfgIdx, HI_BOOL bNewGrp, HI_FILEMNG_OBJ_FILENAME_S *pstFileName);

/**
 * @brief    repair the damaged files.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE).
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Repair(HI_VOID);

/**
 * @brief    search groups forward from the final group.
 * @param[in] u32Start:start position,0 means the final group.
 * @param[in] u32GrpNum:the number of groups to search.
 * @param[in] pfnSrchGrpCB:the callback function for search group.
 * @param[in] pvUserData:user data pointer.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SrchGrpsFwd(HI_U32 u32Start, HI_U32 u32GrpNum, HI_FILEMNG_SRCH_GRP_CALLBACK_FN_PTR pfnSrchGrpCB,
                              HI_VOID *pvUserData);

/**
 * @brief    search groups backward from the first group.
 * @param[in] u32Start:start position,0 means the first group.
 * @param[in] u32GrpNum:the number of groups to search.
 * @param[in] pfnSrchGrpCB:the callback function for search group.
 * @param[in] pvUserData:user data pointer.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SrchGrpsBwd(HI_U32 u32Start, HI_U32 u32GrpNum, HI_FILEMNG_SRCH_GRP_CALLBACK_FN_PTR pfnSrchGrpCB,
                              HI_VOID *pvUserData);

/**
 * @brief    search file objects forward from the final file object.
 * @param[in] u32Start:start position,0 means the final file object.
 * @param[in] u32FileNum:the number of file objects to search.
 * @param[in] pfnSrchFileObjCB:the callback function for search file object.
 * @param[in] pvUserData:user data pointer.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SrchFileObjsFwd(HI_U32 u32Start, HI_U32 u32FileObjNum,
                                  HI_FILEMNG_SRCH_FILEOBJ_CALLBACK_FN_PTR pfnSrchFileObjCB, HI_VOID *pvUserData);

/**
 * @brief    search file objects backward from the first file object.
 * @param[in] u32Start:start position,0 means the first file object.
 * @param[in] u32FileNum:the number of file objects to search.
 * @param[in] pfnSrchFileObjCB:the callback function for search file object.
 * @param[in] pvUserData:user data pointer.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SrchFileObjsBwd(HI_U32 u32Start, HI_U32 u32FileObjNum,
                                  HI_FILEMNG_SRCH_FILEOBJ_CALLBACK_FN_PTR pfnSrchFileObjCB, HI_VOID *pvUserData);

/**
 * @brief    delete groups forward from the final group.
 * @param[in] u32Start:start position,0 means the final group.
 * @param[in] u32GrpNum:the number of groups to delete.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_DelGrpsFwd(HI_U32 u32Start, HI_U32 u32GrpNum);

/**
 * @brief   delete groups backward from the first group.
 * @param[in] u32Start:start position,0 means the first group.
 * @param[in] u32GrpNum:the number of groups to delete.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_DelGrpsBwd(HI_U32 u32Start, HI_U32 u32GrpNum);

/**
 * @brief    get the file object filenames from current group.
 * @param[in] u32FileIdx: file object index in group,range:[0,u16ObjCnt).
 * @param[out] pstFileNames:the file object filenames.
 * @return 0 success,non-zero error code.
 * @exception    call this function in HI_FILEMNG_SRCH_GRP_CALLBACK_FN_PTR.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetObjFilenames(HI_U32 u32FileIdx, HI_FILEMNG_OBJ_FILENAME_S *pstFileNames);

/**
 * @brief    get the total group count.
 * @param[out] pu32GrpCnt:the total group count.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_EVENT_FILEMNG_SCAN_COMPLETED published
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetGrpCnt(HI_U32 *pu32GrpCnt);

/**
 * @brief    get the specified file object count.
 * @param[in] enType:specified type,HI_FILEMNG_FILE_TYPE_BUTT means all type.
 * @param[out] pu32FileObjCnt:the specified file object count.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_EVENT_FILEMNG_SCAN_COMPLETED published
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileObjCnt(HI_FILEMNG_FILE_TYPE_E enType, HI_U32 *pu32FileObjCnt);

/**
 * @brief    get the file information by file name.
 * @param[in] pszFilePath:the file absolute path.
 * @param[out] pstFileInfo:the file information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileInfoByName(const HI_CHAR *pszFilePath, HI_FILEMNG_FILE_INFO_S *pstFileInfo);

/** @}*/  /** <!-- ==== FILEMNG End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_FILEMNG_DCF_H__*/

