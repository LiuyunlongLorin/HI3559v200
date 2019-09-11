/**
 * @file    hi_filemng_dcf.c
 * @brief   DCF file manager function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#include "hi_eventhub.h"
#include "hi_appcomm_util.h"
#include "hi_filemng_dcf.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static HI_DCF_WRAPPER_INDEX s_DCFIdx;
static pthread_mutex_t s_FILEMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_FILEMNGCond = PTHREAD_COND_INITIALIZER;
static HI_FILEMNG_COMM_CFG_S s_stFILEMNGCfg;
static HI_FILEMNG_DCF_CFG_S s_stFILEMNGDCF_Cfg;
static HI_BOOL s_bFILEMNGInit = HI_FALSE;
static FILEMNG_DISK_STATE_E s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE; /**<0-not available;1-available;2-scan completed */

/**-------------------------internal function interface------------------------- */
/**
 * @brief    scan evnet callback function.
 * @param[in] iDCF:DCF instance handle.
 * @param[in] enState:disk scan state.
 * @param[in] s32ScanErrCode:scan error code.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 */
static HI_VOID FILEMNG_ScanEvent(HI_DCF_WRAPPER_INDEX iDCF, HI_DCF_WRAPPER_SCAN_STATE_E enState, HI_S32 s32ScanErrCode)
{
    if (HI_DCF_WRAPPER_SCAN_STATE_LAST_DIR_DONE == enState) {
        MLOGD("The final directory scan done\n");
    } else if (HI_DCF_WRAPPER_SCAN_STATE_FIRST_DIR_DONE == enState) {
        /**this signal corresponds to the wait in function HI_FILEMNG_SetDiskState. */
        HI_MUTEX_LOCK(s_FILEMNGMutex);
        HI_COND_SIGNAL(s_FILEMNGCond);
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGD("The first directory scan done\n");
    } else if (HI_DCF_WRAPPER_SCAN_STATE_ALL_DIR_DONE == enState) {
        HI_MUTEX_LOCK(s_FILEMNGMutex);
        s_enFILEMNGDiskState = FILEMNG_DISK_STATE_SCAN_COMPLETED;
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGI("The whole directory scan done\n");
        HI_EVENT_S stEvent;
        stEvent.EventID = HI_EVENT_FILEMNG_SCAN_COMPLETED;
        HI_EVTHUB_Publish(&stEvent);
    } else {
        /**this signal corresponds to the wait in function HI_FILEMNG_SetDiskState. */
        HI_MUTEX_LOCK(s_FILEMNGMutex);
        HI_COND_SIGNAL(s_FILEMNGCond);
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_EVENT_S stEvent;
        stEvent.EventID = HI_EVENT_FILEMNG_SCAN_FAIL;
        HI_EVTHUB_Publish(&stEvent);
        MLOGE("Scan failed:[0x%08X]\n", s32ScanErrCode);
    }
    return;
}

static HI_S32 FILEMNG_DelCurGrp(const HI_DCF_WRAPPER_GRP_S *pstGrp, HI_VOID *pvUserData)
{
    HI_S32 s32Ret = 0;
    HI_DCF_WRAPPER_OBJ_S stObj;
    s32Ret = HI_DCF_WRAPPER_GetCurGrpFirstObj(s_DCFIdx, &stObj);
    if (HI_SUCCESS == s32Ret) {
        FILEMNG_MARKER_CleanFlag(stObj.aszFilePath[0]);
    }

    while (HI_SUCCESS == s32Ret) {
        s32Ret = HI_DCF_WRAPPER_GetCurGrpNextObj(s_DCFIdx, &stObj);
        if (HI_SUCCESS == s32Ret) {
            FILEMNG_MARKER_CleanFlag(stObj.aszFilePath[0]);
        }
    }

    s32Ret = HI_DCF_WRAPPER_DelCurGrp(s_DCFIdx);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_DelCurGrp, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    return HI_SUCCESS;
}

static HI_S32 FILEMNG_Disable(HI_VOID)
{
    HI_S32 s32Ret = HI_DCF_WRAPPER_Destroy(s_DCFIdx);
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_Destroy, s32Ret);
    }

    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;
    return HI_SUCCESS;
}

/**-------------------------external function interface-------------------------*/
/**
 * @brief    register filemng event.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/29
 */
HI_S32 HI_FILEMNG_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = 0;
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SCAN_COMPLETED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SCAN_FAIL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SPACE_FULL);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_SPACE_ENOUGH);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_BEGIN);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_END);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_REPAIR_FAILED);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_FILEMNG_UNIDENTIFICATION);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FILEMNG_EINTER);
    return HI_SUCCESS;
}

/**
 * @brief    init file manager.
 * @param[in] pstCfg:file manager comm configuration.
 * @param[in] pstDCF_Cfg:file manager DCF configuration.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Init(const HI_FILEMNG_COMM_CFG_S *pstCfg, const HI_FILEMNG_DCF_CFG_S *pstDCF_Cfg)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDCF_Cfg, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_EXPR(HI_DCF_WRAPPER_MAX_OBJ_TYPE_CNT >= pstDCF_Cfg->u8ObjTypeCnt, HI_FILEMNG_EINVAL);

    for (i = 0; i < pstDCF_Cfg->u8ObjTypeCnt; i++) {
        HI_APPCOMM_CHECK_EXPR(HI_DCF_WRAPPER_MAX_FILE_CNT_IN_OBJ >= pstDCF_Cfg->astObjCfgTbl[i].u8FileCnt, HI_FILEMNG_EINVAL);
    }

    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_TRUE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EINITIALIZED));
        return HI_FILEMNG_EINITIALIZED;
    }

    s32Ret = HI_DCF_WRAPPER_Init();
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_Init, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    if (HI_TRUE == pstCfg->stRepairCfg.bEnable) {
        s32Ret = FILEMNG_REPAIRER_Init(&pstCfg->stRepairCfg);
        if (HI_SUCCESS != s32Ret) {
            HI_DCF_WRAPPER_DeInit();
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
            return s32Ret;
        }
    }

    SPACEMONITOR_CFG_S stConfig;
    snprintf(stConfig.szMntPath, HI_APPCOMM_MAX_PATH_LEN, pstCfg->szMntPath);
    stConfig.u32WarningStage = pstDCF_Cfg->u32WarningStage;
    stConfig.u32GuaranteedStage = 0;
    stConfig.u32Interval = 0;
    stConfig.u32MaxCheckDelay = 0;
    stConfig.pfnCoverCB = NULL;
    s32Ret = FILEMNG_SPACEMONITOR_Create(&stConfig);
    if (HI_SUCCESS != s32Ret) {
        FILEMNG_REPAIRER_Deinit();
        HI_DCF_WRAPPER_DeInit();
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
        return s32Ret;
    }

    memcpy(&s_stFILEMNGCfg, pstCfg, sizeof(HI_FILEMNG_COMM_CFG_S));
    memcpy(&s_stFILEMNGDCF_Cfg, pstDCF_Cfg, sizeof(HI_FILEMNG_DCF_CFG_S));
    s_enFILEMNGDiskState = FILEMNG_DISK_STATE_NOT_AVAILABLE;
    s_bFILEMNGInit = HI_TRUE;
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    deinit file manager.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        FILEMNG_Disable();
        HI_MUTEX_LOCK(s_FILEMNGMutex);
    }

    s32Ret = FILEMNG_SPACEMONITOR_Destroy();
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(FILEMNG_SPACEMONITOR_Destroy, s32Ret);
    }

    if (HI_TRUE == s_stFILEMNGCfg.stRepairCfg.bEnable) {
        s32Ret = FILEMNG_REPAIRER_Deinit();
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(FILEMNG_REPAIRER_Deinit, s32Ret);
        }
    }

    s32Ret = HI_DCF_WRAPPER_DeInit();
    if (HI_SUCCESS != s32Ret) {
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_DeInit, s32Ret);
    }

    s_bFILEMNGInit = HI_FALSE;
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    call this function to set the disk state.
 * @param[in] bAvailable:disk state.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_SetDiskState(HI_BOOL bAvailable)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0, j = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if ((HI_TRUE == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE != s_enFILEMNGDiskState)
            || (HI_FALSE == bAvailable && FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState)) {
        /**disk available status not change,do nothing. */
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    } else {
        if (bAvailable) {
            s32Ret = FILEMNG_CheckPath(s_stFILEMNGCfg.szMntPath, "DCIM");
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
                return s32Ret;
            }

            HI_DCF_WRAPPER_NAMERULE_CFG_S stCfg;
            snprintf(stCfg.aszRoot, HI_DCF_WRAPPER_MAX_ROOT_PATH_LENGTH, s_stFILEMNGCfg.szMntPath);
            snprintf(stCfg.aszDirFreeChar, HI_DCF_WRAPPER_DIR_FREE_CHAR_LENGTH, s_stFILEMNGDCF_Cfg.szDirFreeChar);
            stCfg.u8ObjTypCnt = s_stFILEMNGDCF_Cfg.u8ObjTypeCnt;

            for (i = 0; i < s_stFILEMNGDCF_Cfg.u8ObjTypeCnt; i++) {
                stCfg.astObjTypTbl[i].u8FileCnt = s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].u8FileCnt;
                for (j = 0; j < s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].u8FileCnt; j++) {
                    snprintf(stCfg.astObjTypTbl[i].astFileTbl[j].aszTypChar,
                             HI_DCF_WRAPPER_FILE_FREE_CHAR_LENGTH,
                             s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].szFileFreeChar);
                    snprintf(stCfg.astObjTypTbl[i].astFileTbl[j].aszExtension,
                             HI_DCF_WRAPPER_FILE_EXTENSION_LENGTH,
                             s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].aszExtension[j]);
                }
            }

            s_enFILEMNGDiskState = FILEMNG_DISK_STATE_AVAILABLE;
            s32Ret = HI_DCF_WRAPPER_Create(FILEMNG_ScanEvent, &stCfg, &s_DCFIdx);
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_Create, s32Ret);
                return HI_FILEMNG_EINTER;
            }

            s32Ret = FILEMNG_MARKER_Init(s_stFILEMNGCfg.szMntPath);
            if (HI_SUCCESS != s32Ret) {
                HI_LOG_PrintFuncErr(FILEMNG_MARKER_Init, s32Ret);
            }
            /**here wait for the first directory scan done before HI_FILEMNG_SetDiskState return. */
            HI_COND_WAIT(s_FILEMNGCond, s_FILEMNGMutex);
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return s32Ret;
        } else {
            FILEMNG_Disable();
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return HI_SUCCESS;
        }
    }
}

HI_S32 HI_FILEMNG_CheckDiskSpace(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }
    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }
    HI_CHAR szRootPath[HI_APPCOMM_MAX_PATH_LEN];
    snprintf(szRootPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", s_stFILEMNGCfg.szMntPath, "DCIM");
    HI_U64 u64RealUsedSize = 0;
    HI_du(szRootPath, &u64RealUsedSize);
    u64RealUsedSize = u64RealUsedSize >> 10;
    s32Ret = FILEMNG_SPACEMONITOR_JudgeStage(u64RealUsedSize);
    if (HI_SUCCESS != s32Ret) {
        MLOGE("%s\n", FILEMNG_Strerror(s32Ret));
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

/**
 * @brief    add a file to the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_AddFile(const HI_CHAR *pszFilePath)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32Ret = HI_PathIsDirectory(pszFilePath);
    if (0 == s32Ret) {
        s32Ret = HI_DCF_WRAPPER_AddObj(s_DCFIdx, pszFilePath);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            if (HI_ERR_DCF_WRAPPER_OBJ_ALREADY_EXIST == s32Ret) {
                return HI_FILEMNG_EEXIST;
            } else if (HI_ERR_DCF_WRAPPER_NOT_MAIN_FILE == s32Ret) {
                return HI_FILEMNG_ENOTMAIN;
            }

            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_AddObj, s32Ret);
            return HI_FILEMNG_EINVAL;
        } else {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            return HI_SUCCESS;
        }
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
    return HI_FILEMNG_ELOST;
}

/**
 * @brief    remove a file from the file manager.
 * @param[in] pszFilePath:the file absolute path.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_RemoveFile(const HI_CHAR *pszFilePath)
{
    HI_S32 s32Ret = 0;
    HI_S32 s32IsDir = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32IsDir = HI_PathIsDirectory(pszFilePath);
    s32Ret = HI_DCF_WRAPPER_DelObj(s_DCFIdx, pszFilePath);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        if (HI_ERR_DCF_WRAPPER_NOT_MAIN_FILE == s32Ret) {
            return HI_FILEMNG_ENOTMAIN;
        }
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_DelObj, s32Ret);
        return HI_FILEMNG_EINVAL;
    }

    if (0 == s32IsDir) {
        FILEMNG_MARKER_CleanFlag(pszFilePath);
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    } else {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
        return HI_FILEMNG_ELOST;
    }
}

/**
 * @brief    mark a file with a symbol.
 * @param[in] pszFilePath:the file absolute path.
 * @param[in] u8Flag:the flag marked to the file.range:[0,255],0 unmark,1-255 special flag.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_MarkFile(const HI_CHAR *pszFilePath, HI_U8 u8Flag)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    s32Ret = HI_PathIsDirectory(pszFilePath);
    if (0 != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s:%s\n", FILEMNG_Strerror(HI_FILEMNG_ELOST), pszFilePath);
        return HI_FILEMNG_ELOST;
    }

    if (0 == u8Flag) {
        s32Ret = FILEMNG_MARKER_CleanFlag(pszFilePath);
    } else {
        s32Ret = FILEMNG_MARKER_SetFlag(pszFilePath, u8Flag);
    }
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return s32Ret;
}

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
HI_S32 HI_FILEMNG_GenerateFileName(HI_U8 u8ObjCfgIdx, HI_BOOL bNewGrp, HI_FILEMNG_OBJ_FILENAME_S *pstFileName)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstFileName, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    HI_DCF_WRAPPER_OBJ_S stObj;
    s32Ret = HI_DCF_WRAPPER_GetNewFilePaths(s_DCFIdx, bNewGrp, u8ObjCfgIdx, &stObj);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetNewFilePaths, s32Ret);
        if (HI_ERR_DCF_WRAPPER_NOT_ENOUGH_DIR == s32Ret) {
            return HI_FILEMNG_ENORES;
        }
        return HI_FILEMNG_EINVAL;
    } else {
        for (i = 0; i < stObj.u8FileCnt; i++) {
            snprintf(pstFileName->szFileName[i], HI_APPCOMM_MAX_PATH_LEN, stObj.aszFilePath[i]);
            if (HI_TRUE == s_stFILEMNGDCF_Cfg.astObjCfgTbl[u8ObjCfgIdx].abNeedRepair[i]
                    && HI_TRUE == s_stFILEMNGCfg.stRepairCfg.bEnable) {
                FILEMNG_REPAIRER_Backup(pstFileName->szFileName[i]);
            }
        }

        pstFileName->u8FileCnt = stObj.u8FileCnt;
        FILEMNG_SPACEMONITOR_CheckSpace();
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        return HI_SUCCESS;
    }
}

/**
 * @brief    repair the damaged files.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE).
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_Repair(HI_VOID)
{
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    FILEMNG_REPAIRER_Repair();
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

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
                              HI_VOID *pvUserData)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32GrpCnt = 0;
    HI_APPCOMM_CHECK_POINTER(pfnSrchGrpCB, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    HI_DCF_WRAPPER_GRP_S stGrp;
    s32Ret = HI_DCF_WRAPPER_GetLastGrp(s_DCFIdx, &stGrp);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetLastGrp, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    if (0 == u32Start) {
        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    } else {
        s32Ret = HI_DCF_WRAPPER_GetPrevGrp(s_DCFIdx, u32Start, &stGrp);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetPrevGrp, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    }

    while (u32GrpCnt < u32GrpNum) {
        s32Ret = HI_DCF_WRAPPER_GetPrevGrp(s_DCFIdx, 1, &stGrp);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            if (HI_ERR_DCF_WRAPPER_NO_PREV == s32Ret) {
                return HI_SUCCESS;
            }
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetPrevGrp, s32Ret);
            return HI_FILEMNG_EINTER;
        }
        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    }
    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

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
                              HI_VOID *pvUserData)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32GrpCnt = 0;
    HI_APPCOMM_CHECK_POINTER(pfnSrchGrpCB, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    HI_DCF_WRAPPER_GRP_S stGrp;
    s32Ret = HI_DCF_WRAPPER_GetFirstGrp(s_DCFIdx, &stGrp);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetFirstGrp, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    if (0 == u32Start) {
        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    } else {
        s32Ret = HI_DCF_WRAPPER_GetNextGrp(s_DCFIdx, u32Start, &stGrp);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetNextGrp, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    }

    while (u32GrpCnt < u32GrpNum) {
        s32Ret = HI_DCF_WRAPPER_GetNextGrp(s_DCFIdx, 1, &stGrp);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            if (HI_ERR_DCF_WRAPPER_NO_NEXT == s32Ret) {
                return HI_SUCCESS;
            }

            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetNextGrp, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        u32GrpCnt++;
        pfnSrchGrpCB(&stGrp, pvUserData);
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

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
                                  HI_FILEMNG_SRCH_FILEOBJ_CALLBACK_FN_PTR pfnSrchFileObjCB, HI_VOID *pvUserData)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_U32 u32FileObjCnt = 0;
    HI_APPCOMM_CHECK_POINTER(pfnSrchFileObjCB, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    HI_DCF_WRAPPER_GRP_S stGrp;
    HI_DCF_WRAPPER_OBJ_S stObj;
    HI_DCF_WRAPPER_OBJ_LIST_S stObjList;
    HI_FILEMNG_FILEOBJ_S stFileObjInfo;
    s32Ret = HI_DCF_WRAPPER_GetLastGrp(s_DCFIdx, &stGrp);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetLastGrp, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    s32Ret = HI_DCF_WRAPPER_GetCurGrpLastObj(s_DCFIdx, &stObj);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetCurGrpLastObj, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    if (0 == u32Start) {
        stFileObjInfo.stFileNames.u8FileCnt = stObj.u8FileCnt;
        for (i = 0; i < stObj.u8FileCnt; i++) {
            snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN, stObj.aszFilePath[i]);
        }

        snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
        HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
        u32FileObjCnt++;
        pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
    } else {
        stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S *)malloc(u32Start * sizeof(HI_DCF_WRAPPER_OBJ_S));
        s32Ret = HI_DCF_WRAPPER_GetPrevObjList(s_DCFIdx, stObj.aszFilePath[0], u32Start, &stObjList);
        if (HI_SUCCESS != s32Ret) {
            HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetPrevObjList, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        u32Start = stObjList.u32ObjCnt;
        stFileObjInfo.stFileNames.u8FileCnt = stObjList.apstObj[u32Start - 1].u8FileCnt;
        for (i = 0; i < stObjList.apstObj[u32Start - 1].u8FileCnt; i++) {
            snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN,
                     stObjList.apstObj[u32Start - 1].aszFilePath[i]);
        }

        snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
        HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
        u32FileObjCnt++;
        pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
        HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
    }

    if (u32FileObjNum > 1) {
        stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S *)malloc((u32FileObjNum - 1) * sizeof(HI_DCF_WRAPPER_OBJ_S));
        s32Ret = HI_DCF_WRAPPER_GetPrevObjList(s_DCFIdx, stFileObjInfo.stFileNames.szFileName[0], (u32FileObjNum - 1),
                                               &stObjList);
        if (HI_SUCCESS != s32Ret) {
            HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetPrevObjList, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        while ((u32FileObjCnt - 1) < stObjList.u32ObjCnt) {
            stFileObjInfo.stFileNames.u8FileCnt = stObjList.apstObj[u32FileObjCnt - 1].u8FileCnt;
            for (i = 0; i < stObjList.apstObj[u32FileObjCnt - 1].u8FileCnt; i++) {
                snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN,
                         stObjList.apstObj[u32FileObjCnt - 1].aszFilePath[i]);
            }

            snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
            HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
            u32FileObjCnt++;
            pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
        }

        HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

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
                                  HI_FILEMNG_SRCH_FILEOBJ_CALLBACK_FN_PTR pfnSrchFileObjCB, HI_VOID *pvUserData)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_U32 u32FileObjCnt = 0;
    HI_APPCOMM_CHECK_POINTER(pfnSrchFileObjCB, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    }

    HI_DCF_WRAPPER_GRP_S stGrp;
    HI_DCF_WRAPPER_OBJ_S stObj;
    HI_DCF_WRAPPER_OBJ_LIST_S stObjList;
    HI_FILEMNG_FILEOBJ_S stFileObjInfo;
    s32Ret = HI_DCF_WRAPPER_GetFirstGrp(s_DCFIdx, &stGrp);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetFirstGrp, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    s32Ret = HI_DCF_WRAPPER_GetCurGrpFirstObj(s_DCFIdx, &stObj);
    if (HI_SUCCESS != s32Ret) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetCurGrpFirstObj, s32Ret);
        return HI_FILEMNG_EINTER;
    }

    if (0 == u32Start) {
        stFileObjInfo.stFileNames.u8FileCnt = stObj.u8FileCnt;
        for (i = 0; i < stObj.u8FileCnt; i++) {
            snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN, stObj.aszFilePath[i]);
        }

        snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
        HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
        u32FileObjCnt++;
        pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
    } else {
        stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S *)malloc(u32Start * sizeof(HI_DCF_WRAPPER_OBJ_S));
        s32Ret = HI_DCF_WRAPPER_GetNextObjList(s_DCFIdx, stObj.aszFilePath[0], u32Start, &stObjList);
        if (HI_SUCCESS != s32Ret) {
            HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetNextObjList, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        u32Start = stObjList.u32ObjCnt;
        stFileObjInfo.stFileNames.u8FileCnt = stObjList.apstObj[u32Start - 1].u8FileCnt;
        for (i = 0; i < stObjList.apstObj[u32Start - 1].u8FileCnt; i++) {
            snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN,
                     stObjList.apstObj[u32Start - 1].aszFilePath[i]);
        }

        snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
        HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
        u32FileObjCnt++;
        pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
        HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
    }

    if (u32FileObjNum > 1) {
        stObjList.apstObj = (HI_DCF_WRAPPER_OBJ_S *)malloc((u32FileObjNum - 1) * sizeof(HI_DCF_WRAPPER_OBJ_S));
        s32Ret = HI_DCF_WRAPPER_GetNextObjList(s_DCFIdx, stFileObjInfo.stFileNames.szFileName[0], (u32FileObjNum - 1),
                                               &stObjList);
        if (HI_SUCCESS != s32Ret) {
            HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetNextObjList, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        while ((u32FileObjCnt - 1) < stObjList.u32ObjCnt) {
            stFileObjInfo.stFileNames.u8FileCnt = stObjList.apstObj[u32FileObjCnt - 1].u8FileCnt;
            for (i = 0; i < stObjList.apstObj[u32FileObjCnt - 1].u8FileCnt; i++) {
                snprintf(stFileObjInfo.stFileNames.szFileName[i], HI_APPCOMM_MAX_PATH_LEN,
                         stObjList.apstObj[u32FileObjCnt - 1].aszFilePath[i]);
            }

            snprintf(stFileObjInfo.stBasicFile.szAbsPath, HI_APPCOMM_MAX_PATH_LEN, stFileObjInfo.stFileNames.szFileName[0]);
            HI_FILEMNG_GetFileInfoByName(stFileObjInfo.stBasicFile.szAbsPath, &stFileObjInfo.stBasicFile);
            u32FileObjCnt++;
            pfnSrchFileObjCB(&stFileObjInfo, pvUserData);
        }

        HI_APPCOMM_SAFE_FREE(stObjList.apstObj);
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    delete groups forward from the final group.
 * @param[in] u32Start:start position,0 means the final group.
 * @param[in] u32GrpNum:the number of groups to delete.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_DelGrpsFwd(HI_U32 u32Start, HI_U32 u32GrpNum)
{
    return HI_FILEMNG_SrchGrpsFwd(u32Start, u32GrpNum, FILEMNG_DelCurGrp, NULL);
}

/**
 * @brief   delete groups backward from the first group.
 * @param[in] u32Start:start position,0 means the first group.
 * @param[in] u32GrpNum:the number of groups to delete.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_FILEMNG_SetDiskState(TRUE)
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_DelGrpsBwd(HI_U32 u32Start, HI_U32 u32GrpNum)
{
    return HI_FILEMNG_SrchGrpsBwd(u32Start, u32GrpNum, FILEMNG_DelCurGrp, NULL);
}

/**
 * @brief    get the file object filenames from current group.
 * @param[in] u32FileIdx: file object index in group,range:[0,u16ObjCnt).
 * @param[out] pstFileNames:the file object filenames.
 * @return 0 success,non-zero error code.
 * @exception    call this function in HI_FILEMNG_SRCH_GRP_CALLBACK_FN_PTR.
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetObjFilenames(HI_U32 u32FileIdx, HI_FILEMNG_OBJ_FILENAME_S *pstFileNames)
{
    HI_S32 s32Ret = 0;
    HI_U32 i = 0;
    HI_APPCOMM_CHECK_POINTER(pstFileNames, HI_FILEMNG_EINVAL);

    if (HI_FALSE == s_bFILEMNGInit) {
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    if (FILEMNG_DISK_STATE_NOT_AVAILABLE == s_enFILEMNGDiskState) {
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTREADY));
        return HI_FILEMNG_ENOTREADY;
    } else {
        HI_DCF_WRAPPER_OBJ_S stObj;
        s32Ret = HI_DCF_WRAPPER_GetCurGrpFirstObj(s_DCFIdx, &stObj);
        if (HI_SUCCESS != s32Ret) {
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetCurGrpFirstObj, s32Ret);
            return HI_FILEMNG_EINTER;
        }

        if (0 < u32FileIdx) {
            for (i = 0; i < u32FileIdx; i++) {
                s32Ret = HI_DCF_WRAPPER_GetCurGrpNextObj(s_DCFIdx, &stObj);
                if (HI_SUCCESS != s32Ret) {
                    HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetCurGrpNextObj, s32Ret);
                    return HI_FILEMNG_EINVAL;
                }
            }
        }

        pstFileNames->u8FileCnt = stObj.u8FileCnt;

        for (i = 0; i < stObj.u8FileCnt; i++) {
            snprintf(pstFileNames->szFileName[i], HI_APPCOMM_MAX_PATH_LEN, stObj.aszFilePath[i]);
        }
    }

    return HI_SUCCESS;
}

/**
 * @brief    get the total group count.
 * @param[out] pu32GrpCnt:the total group count.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_EVENT_FILEMNG_SCAN_COMPLETED published
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetGrpCnt(HI_U32 *pu32GrpCnt)
{
    HI_S32 s32Ret = 0;
    HI_APPCOMM_CHECK_POINTER(pu32GrpCnt, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    *pu32GrpCnt = 0;

    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EBUSY));
        return HI_FILEMNG_EBUSY;
    } else if (FILEMNG_DISK_STATE_SCAN_COMPLETED == s_enFILEMNGDiskState) {
        s32Ret = HI_DCF_WRAPPER_GetTotalGrpCnt(s_DCFIdx, pu32GrpCnt);
        if (HI_SUCCESS != s32Ret) {
            HI_MUTEX_UNLOCK(s_FILEMNGMutex);
            HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetTotalGrpCnt, s32Ret);
            return HI_FILEMNG_EINTER;
        }
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    MLOGD("TotalGrpCnt %d \n", *pu32GrpCnt);
    return HI_SUCCESS;
}

/**
 * @brief    get the specified file object count.
 * @param[in] enType:specified type,HI_FILEMNG_FILE_TYPE_BUTT means all type.
 * @param[out] pu32FileObjCnt:the specified file object count.
 * @return 0 success,non-zero error code.
 * @exception    call this function after HI_EVENT_FILEMNG_SCAN_COMPLETED published
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileObjCnt(HI_FILEMNG_FILE_TYPE_E enType, HI_U32 *pu32FileObjCnt)
{
    HI_S32 s32Ret = 0;
    HI_S32 i = 0;
    HI_U32 u32RecObjCnt = 0;
    HI_U32 u32PhotoObjCnt = 0;
    HI_APPCOMM_CHECK_POINTER(pu32FileObjCnt, HI_FILEMNG_EINVAL);
    HI_MUTEX_LOCK(s_FILEMNGMutex);

    if (HI_FALSE == s_bFILEMNGInit) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_ENOTINIT));
        return HI_FILEMNG_ENOTINIT;
    }

    *pu32FileObjCnt = 0;

    if (FILEMNG_DISK_STATE_AVAILABLE == s_enFILEMNGDiskState) {
        HI_MUTEX_UNLOCK(s_FILEMNGMutex);
        MLOGE("%s\n", FILEMNG_Strerror(HI_FILEMNG_EBUSY));
        return HI_FILEMNG_EBUSY;
    } else if (FILEMNG_DISK_STATE_SCAN_COMPLETED == s_enFILEMNGDiskState) {
        if (HI_FILEMNG_FILE_TYPE_RECORD != enType && HI_FILEMNG_FILE_TYPE_PHOTO != enType) {
            s32Ret = HI_DCF_WRAPPER_GetTotalObjCnt(s_DCFIdx, pu32FileObjCnt);
            if (HI_SUCCESS != s32Ret) {
                HI_MUTEX_UNLOCK(s_FILEMNGMutex);
                HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetTotalObjCnt, s32Ret);
                return HI_FILEMNG_EINTER;
            }
            MLOGD("TotalFileObjCount %d\n", *pu32FileObjCnt);
        } else {
            for (i = 0; i < s_stFILEMNGDCF_Cfg.u8ObjTypeCnt; i++) {
                HI_U32 u32Cnt = 0;
                s32Ret = HI_DCF_WRAPPER_GetObjCnt(s_DCFIdx, i, &u32Cnt);
                if (HI_SUCCESS == s32Ret) {
                    MLOGD("ObjTypIndex = %d,count = %d\n", i, u32Cnt);
                    if (HI_FILEMNG_FILE_TYPE_RECORD == s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].enType) {
                        u32RecObjCnt += u32Cnt;
                    } else if (HI_FILEMNG_FILE_TYPE_PHOTO == s_stFILEMNGDCF_Cfg.astObjCfgTbl[i].enType) {
                        u32PhotoObjCnt += u32Cnt;
                    }
                } else {
                    HI_LOG_PrintFuncErr(HI_DCF_WRAPPER_GetObjCnt, s32Ret);
                }
            }

            if (HI_FILEMNG_FILE_TYPE_RECORD == enType) {
                *pu32FileObjCnt = u32RecObjCnt;
            } else {
                *pu32FileObjCnt = u32PhotoObjCnt;
            }
            MLOGD("TotalFileObjCount %d(record:%d+photo:%d)\n",
                  (u32RecObjCnt + u32PhotoObjCnt),
                  u32RecObjCnt,
                  u32PhotoObjCnt);
        }
    }

    HI_MUTEX_UNLOCK(s_FILEMNGMutex);
    return HI_SUCCESS;
}

/**
 * @brief    get the file information by file name.
 * @param[in] pszFilePath:the file absolute path.
 * @param[out] pstFileInfo:the file information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/12
 */
HI_S32 HI_FILEMNG_GetFileInfoByName(const HI_CHAR *pszFilePath, HI_FILEMNG_FILE_INFO_S *pstFileInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszFilePath, HI_FILEMNG_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstFileInfo, HI_FILEMNG_EINVAL);
    return FILEMNG_GetFileInfo(pszFilePath, pstFileInfo);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

