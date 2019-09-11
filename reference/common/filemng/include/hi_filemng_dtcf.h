/**
 * @file    hi_filemng_dtcf.h
 * @brief   DTCF file manager struct and interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

 */
#ifndef __HI_FILEMNG_DTCF_H__
#define __HI_FILEMNG_DTCF_H__

#include "hi_dtcf.h"
#include "hi_filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     FILEMNG */
/** @{ */  /** <!-- [FILEMNG] */

#define HI_FILEMNG_DTCF_MAX_PHOTO_DIR (2)

/** struct define */
/** DTCF configuration */
typedef  struct hiFILEMNG_DTCF_CFG_S {
    HI_U32 u32PreAllocCnt[HI_FILEMNG_DTCF_MAX_PHOTO_DIR];/**<pre allocate file count */
    HI_U32 u32PreAllocUnit[HI_FILEMNG_DTCF_MAX_PHOTO_DIR];/**<pre allocate file size ,suggest 2M, unit:byte */
    HI_U8  u8SharePercent; /**<0:share 1~100:movie space ratio;eg.80 */
    HI_U32 u32WarningStage;/**<unit:MB;filemanager will publish a warning when the remaining space is less than u32WarningStage */
    HI_U32 u32GuaranteedStage;/**<unit:MB;u32GuaranteedStage should NOT be less than u32WarningStage;the loop coverage is disable when u32GuaranteedStage is 0 */
    HI_CHAR szRootDir[HI_APPCOMM_MAX_PATH_LEN];/**<file manager top directory name in mount path. eg."HICAM"*/
    HI_CHAR aszDirNames[DTCF_DIR_BUTT][HI_DIR_LEN_MAX];/**<eg.{"EMR", "EMR_s", "Movie", "Movie_s", "", "", "", "", "Photo"} */
} HI_FILEMNG_DTCF_CFG_S;


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
HI_S32 HI_FILEMNG_Init(const HI_FILEMNG_COMM_CFG_S *pstCfg, const HI_FILEMNG_DTCF_CFG_S *pstDTCF_Cfg);

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
 * @brief    create new file name to DTCF.
 * @param[in] enType:file type
 * @param[in] enDir:dir type,range:[0,DTCF_DIR_BUTT)
 * @param[in] bPreAlloc:is pre alloc file or not
 * @param[out] pstFileName:new file object name.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GenerateFileName(HI_DTCF_FILE_TYPE_E enType, HI_DTCF_DIR_E enDir, HI_BOOL bPreAlloc,
                                   HI_FILEMNG_OBJ_FILENAME_S *pstFileName);

/**
 * @brief    repair the damaged files.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE).
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Repair(HI_VOID);

/**
 * @brief    set the search scope
 * @param[in] enDirs[DTCF_DIR_BUTT]:directory collection to search
 * @param[in] u32DirCount:directory collection count
 * @param[out] pu32FileObjCnt:file object count in the selected directorys
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SetSearchScope(HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT], HI_U32 u32DirCount, HI_U32 *pu32FileObjCnt);

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
 * @brief    get the file information by file index.
 * @param[in] u32FileIdx:the file absolute index,range[0,FileObjCnt).
 * @param[out] pazFileName:the file name.
 * @param[in] u32Length:the file path length.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileByIndex(HI_U32 u32FileIdx, HI_CHAR *pazFileName, HI_U32 u32Length);

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

#endif /* End of __HI_FILEMNG_DTCF_H__*/

