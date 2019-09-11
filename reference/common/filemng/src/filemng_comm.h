/**
 * @file    filemng_comm.h
 * @brief   define file manager comm inner struct
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

*/
#ifndef __FILEMNG_COMM_H__
#define __FILEMNG_COMM_H__

#include "hi_filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     FILEMNG */
/** @{ */  /** <!-- [FILEMNG] */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "FILEMNG"

/**define full flag mask*/
#define SPACEMONITOR_MASK_ENOUGH    (0x000)
#define SPACEMONITOR_MASK_TOTALFULL (0x001)
#define SPACEMONITOR_MASK_MOVIEFULL (0x010)
#define SPACEMONITOR_MASK_EMRFULL   (0x100)

/**disk state */
typedef enum tagFILEMNG_DISK_STATE_E {
    FILEMNG_DISK_STATE_NOT_AVAILABLE  = 0,
    FILEMNG_DISK_STATE_AVAILABLE      = 1,
    FILEMNG_DISK_STATE_SCAN_COMPLETED = 2,
    FILEMNG_DISK_STATE_BUTT
} FILEMNG_DISK_STATE_E;

/** callback function for cover file object */
typedef HI_S32(*FILEMNG_SPACEMONITOR_Cover)(HI_S32 s32SMFullFlag);
#ifdef CONFIG_FILEMNG_DTCF
typedef HI_S32(*FILEMNG_SPACEMONITOR_GetRatioSpace)(HI_U32 *pu32MovieSpace, HI_U32 *pu32EmrSpace);
#endif

typedef struct tagSPACEMONITOR_CFG_S {
    HI_CHAR szMntPath[HI_APPCOMM_MAX_PATH_LEN];
    HI_U32 u32WarningStage;
    HI_U32 u32GuaranteedStage;
    HI_U32 u32Interval;
    HI_U32 u32MaxCheckDelay;
    FILEMNG_SPACEMONITOR_Cover pfnCoverCB;
#ifdef CONFIG_FILEMNG_DTCF
    HI_U8 u8SharePercent;
    FILEMNG_SPACEMONITOR_GetRatioSpace pfnGetRatioSpace;
#endif
} SPACEMONITOR_CFG_S;

HI_CHAR *FILEMNG_Strerror(HI_S32 s32ErrorCode);
HI_S32 FILEMNG_CheckPath(const HI_CHAR *pszMntPath, const HI_CHAR *pszRootDir);
HI_BOOL FILEMNG_IsMP4(const HI_CHAR *pszFilePath);
HI_S32 FILEMNG_GetFileInfo(const HI_CHAR *pszFilePath, HI_FILEMNG_FILE_INFO_S *pstFileInfo);
HI_S32 FILEMNG_HideFile(const HI_CHAR *pFilePath, HI_BOOL bHide);
HI_S32 FILEMNG_HideDir(const HI_CHAR *pDirPath, HI_BOOL bHide);

HI_S32 FILEMNG_SPACEMONITOR_Create(const SPACEMONITOR_CFG_S *pstConfig);
HI_S32 FILEMNG_SPACEMONITOR_Destroy(HI_VOID);
HI_S32 FILEMNG_SPACEMONITOR_JudgeStage(HI_U64 u64RealUsedSize_MB);
HI_S32 FILEMNG_SPACEMONITOR_CheckSpace(HI_VOID);

HI_S32 FILEMNG_REPAIRER_Init(const HI_FILEMNG_REPAIR_CFG_S *pstRepairCfg);
HI_S32 FILEMNG_REPAIRER_Backup(const HI_CHAR *pszSrcFilePath);
HI_S32 FILEMNG_REPAIRER_Repair(HI_VOID);
HI_S32 FILEMNG_REPAIRER_Deinit(HI_VOID);

HI_S32 FILEMNG_MARKER_Init(const HI_CHAR *pszTopDir);
HI_S32 FILEMNG_MARKER_SetFlag(const HI_CHAR *pszFileName, HI_U8 u8Flag);
HI_S32 FILEMNG_MARKER_CleanFlag(const HI_CHAR *pszFileName);

/** @}*/  /** <!-- ==== FILEMNG End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __FILEMNG_COMM_H__*/

