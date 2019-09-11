#include "securec.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include "hi_mw_type.h"
#include "stg_log.h"
#include "hi_storage.h"
#include "stg_common.h"
#include "stg_devinfo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef __HuaweiLite__
#define STG_DEVINFO_CID_PROC_PATH_TOP       "/sys/block/mmcblk"
#define STG_DEVINFO_CID_PROC_PATH_BOTTOM    "/device/cid"
#endif
#define STG_DEVINFO_MCI_PROC_PATH           "/proc/mci/mci_info"
#define STG_DEVINFO_MCI                     "MCI"

#ifdef __HuaweiLite__
#define STG_DEVINFO_PROC_LINES              (10)
#define STG_DEVINFO_CID_LINE_NO             (9)
#else
#define STG_DEVINFO_PROC_LINES              (9)
#endif
#define STG_DEVINFO_PROC_LINE_LEN           (64)
#define STG_DEVINFO_DEV_TYPE_LINE_NO        (1)
#define STG_DEVINFO_WORK_MODE_LINE_NO       (2)
#define STG_DEVINFO_SPEED_CLASS_LINE_NO     (3)
#define STG_DEVINFO_SPEED_GRADE_LINE_NO     (4)
#define STG_DEVINFO_WORK_CLOCK_LINE_NO      (7)
#define STG_DEVINFO_ERROR_COUNT_LINE_NO     (8)


#define STG_DEVINFO_PORT_INVALID      "invalid"
#define STG_DEVINFO_STATE_UNPLUGGED   "unplugged_disconnected"
#if ((defined __HI3518EV300__)&&(defined __HuaweiLite__))
#define STG_DEVINFO_STATE_CONNECTING  "plugged_disconnected"
#define STG_DEVINFO_STATE_CONNECTED   "plugged_connected"
#else
#define STG_DEVINFO_STATE_CONNECTING  "pluged_disconnected"
#define STG_DEVINFO_STATE_CONNECTED   "pluged_connected"
#endif
#define STG_DEVINFO_VALUE_RESERVED    "Reserved"
#define STG_DEVINFO_VALUE_PREFIX      ":"

#define STG_DEVINFO_HS                "HS"
#define STG_DEVINFO_UHS_SDR12         "SDR12"
#define STG_DEVINFO_UHS_SDR25         "SDR25"
#define STG_DEVINFO_UHS_SDR50         "SDR50"
#define STG_DEVINFO_UHS_DDR50         "DDR50"
#define STG_DEVINFO_UHS_SDR104        "SDR104"
#define STG_DEVINFO_UHS_DDR200        "DDR200"

#define STG_DEVINFO_SPEED_GRADE_LOW   "Less than 10MB/sec(0h)"
#define STG_DEVINFO_SPEED_GRADE_HIGH  "10MB/sec and above(1h)"
#define STG_DEVINFO_SPEED_CLASS_10    "Class 10"

typedef struct hiSTG_DEVINFO_S
{
    HI_S32 s32ErrCurrentCnt;
    HI_U8 u8DevErrCountThr;
    pthread_mutex_t mDevMCIProcLock;
    HI_CHAR aszDevPort[STG_DEVINFO_PROC_LINE_LEN];
#ifndef __HuaweiLite__
    HI_CHAR aszDevCIDPorcPath[PATH_MAX + 1];
#endif
    HI_CHAR aszDevMCIPorcPath[PATH_MAX + 1];
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX];
} STG_DEVINFO_S;

typedef struct tagSTG_DEVINFO_VALID_INFO_S
{
    STG_DEV_STATE_E enState;
    HI_CHAR aszWorkMode[STG_DEVINFO_PROC_LINE_LEN];
    HI_CHAR aszSpeedClass[STG_DEVINFO_PROC_LINE_LEN];
    HI_CHAR aszSpeedGrade[STG_DEVINFO_PROC_LINE_LEN];
    HI_CHAR aszWorkClock[STG_DEVINFO_PROC_LINE_LEN];
    HI_CHAR aszErrCnt[STG_DEVINFO_PROC_LINE_LEN];
#ifdef __HuaweiLite__
    HI_CHAR aszCID[HI_STORAGE_PATH_LEN_MAX];
#endif
    HI_CHAR aszDevType[STG_DEVINFO_PROC_LINE_LEN];
} STG_DEVINFO_VALID_INFO_S;

static HI_S32 STG_DEVINFO_Route(HI_U32 procLineNo, HI_CHAR* value, STG_DEVINFO_VALID_INFO_S* pstInfo)
{
    HI_S32 s32FmtCount = 0;
    HI_CHAR *end = NULL;
    switch (procLineNo)
    {
        case STG_DEVINFO_DEV_TYPE_LINE_NO :
            end = strchr(value,' ');
            if(end != NULL && (end -value) > 0)
            {
                s32FmtCount = strncpy_s(pstInfo->aszDevType, STG_DEVINFO_PROC_LINE_LEN, value, (end - value));
            }
            break;
        case STG_DEVINFO_WORK_MODE_LINE_NO :
            s32FmtCount = snprintf_s(pstInfo->aszWorkMode, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
        case STG_DEVINFO_SPEED_CLASS_LINE_NO :
            s32FmtCount = snprintf_s(pstInfo->aszSpeedClass, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
        case STG_DEVINFO_SPEED_GRADE_LINE_NO:
            s32FmtCount = snprintf_s(pstInfo->aszSpeedGrade, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
        case STG_DEVINFO_WORK_CLOCK_LINE_NO :
            s32FmtCount = snprintf_s(pstInfo->aszWorkClock, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
        case STG_DEVINFO_ERROR_COUNT_LINE_NO :
            s32FmtCount = snprintf_s(pstInfo->aszErrCnt, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
#ifdef __HuaweiLite__
        case STG_DEVINFO_CID_LINE_NO :
            s32FmtCount = snprintf_s(pstInfo->aszCID, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s", value);
            break;
#endif
        default :
            break;
    }

    if ( s32FmtCount < 0 )
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "snprintf_s failed, procLineNo = %d\n", procLineNo);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 STG_DEVINFO_ReadMCI(STG_DEVINFO_S* pstDev, STG_DEVINFO_VALID_INFO_S* pstDevInfo)
{
    HI_U8 u8LineNo = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* pFDevMCIProc = HI_NULL;
    size_t szProcLen = 0;
    HI_CHAR* pTmp = HI_NULL;
    HI_CHAR aszProcLineStr[STG_DEVINFO_PROC_LINE_LEN] = {0};

    STG_THREAD_MUTEX_LOCK(&pstDev->mDevMCIProcLock);
    pFDevMCIProc = fopen(pstDev->aszDevMCIPorcPath, "r");
    if (HI_NULL == pFDevMCIProc)
    {
        (HI_VOID)pthread_mutex_unlock(&pstDev->mDevMCIProcLock);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    while (HI_NULL != fgets(aszProcLineStr, STG_DEVINFO_PROC_LINE_LEN - 1, pFDevMCIProc))
    {
        pTmp = strstr(aszProcLineStr, pstDev->aszDevPort);
        if (pTmp)
        {
            break;
        }
    }

    if (!pTmp)
    {
        s32Ret = HI_ERR_STORAGE_ILLEGAL_PARAM;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev port is not found, and ret:%x!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
        goto CLOSE_FILE;
    }

    if (strstr(aszProcLineStr, STG_DEVINFO_STATE_CONNECTED))
    {
        pstDevInfo->enState = STG_DEV_STATE_CONNECTED;
        for (u8LineNo = 1; u8LineNo < STG_DEVINFO_PROC_LINES; u8LineNo++)
        {
            if (HI_NULL == fgets(aszProcLineStr, STG_DEVINFO_PROC_LINE_LEN - 1, pFDevMCIProc))
            {
                s32Ret = HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
                STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s %d read file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
                goto CLOSE_FILE;
            }

            szProcLen = strlen(aszProcLineStr);
            if ( szProcLen >= 1 && (szProcLen - 1) < STG_DEVINFO_PROC_LINE_LEN )
            {
                aszProcLineStr[szProcLen - 1] = '\0';
            }
            else
            {
                aszProcLineStr[0] = '\0';
            }
            pTmp = strstr(aszProcLineStr, STG_DEVINFO_VALUE_PREFIX);
            if (pTmp && (pTmp = pTmp + 2))// case ": " has occupied two Bit
            {
                s32Ret = STG_DEVINFO_Route(u8LineNo, pTmp, pstDevInfo);
                if ( HI_SUCCESS != s32Ret )
                {
                    goto CLOSE_FILE;
                }
            }
        }
    }
    else if (strstr(aszProcLineStr, STG_DEVINFO_STATE_UNPLUGGED))
    {
        pstDevInfo->enState = STG_DEV_STATE_UNPLUGGED;
    }
    else
    {
        pstDevInfo->enState = STG_DEV_STATE_CONNECTING;
    }

CLOSE_FILE:
    if (0 != fclose(pFDevMCIProc))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  close file failure, system errno(%d)!\n", __FUNCTION__, __LINE__, errno);
    }
    (HI_VOID)pthread_mutex_unlock(&pstDev->mDevMCIProcLock);
    return s32Ret;
}

#ifndef __HuaweiLite__
static HI_S32 STG_DEVINFO_ReadCID(const HI_CHAR* pszPath, HI_CHAR* pszCID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    FILE* pFile = HI_NULL;
    size_t szStrLen = 0;
    HI_CHAR aszProcPath[PATH_MAX + 1] = {0};

    if (NULL == pszPath || NULL == pszCID)
    {
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    if (HI_NULL == realpath(pszPath, aszProcPath))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    pFile = fopen(aszProcPath, "r");
    if (HI_NULL == pFile)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }


    if (HI_NULL == fgets(pszCID, HI_STORAGE_PATH_LEN_MAX - 1, pFile))
    {
        s32Ret = HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
    }
    else
    {
        szStrLen = strlen(pszCID);
        if ((szStrLen > 1) && (szStrLen < HI_STORAGE_PATH_LEN_MAX))
        {
            pszCID[szStrLen - 1] = '\0';
        }
        else
        {
            pszCID[0] = '\0';
        }
    }

    if (0 != fclose(pFile))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  close file failure, system errno(%d)!\n", __FUNCTION__, __LINE__, errno);
    }

    return s32Ret;
}
#endif

static HI_VOID STG_DEVINFO_CalcTransSpeed(const HI_CHAR* aszWorkMode, const HI_CHAR* aszSpeedClass, const HI_CHAR* aszSpeedGrade, HI_STORAGE_TRANSMISSION_SPEED_E* penSpeed)
{
    if (HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_SDR104))
    {
        *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_10_30M;
        return;
    }
    else if (HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_DDR200))
    {
        *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_30_50M;
        return;
    }

    if (HI_NULL != strstr(aszSpeedClass, STG_DEVINFO_VALUE_RESERVED))
    {
        *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_BUTT;
        return;
    }
    else if (HI_NULL == strstr(aszSpeedClass, STG_DEVINFO_SPEED_CLASS_10))
    {
        *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_1_4M;
        return;
    }
    else
    {
        if ((HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_SDR12))
            || (HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_SDR25))
            || (HI_NULL != strstr(aszWorkMode, STG_DEVINFO_HS)))
        {
            *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_4_10M;
            return;
        }
        else if ((HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_SDR50))
                 || (HI_NULL != strstr(aszWorkMode, STG_DEVINFO_UHS_DDR50)))
        {
            if (HI_NULL != strstr(aszSpeedGrade, STG_DEVINFO_VALUE_RESERVED))
            {
                *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_BUTT;
                return;
            }
            else if (0 == strncmp(aszSpeedGrade, STG_DEVINFO_SPEED_GRADE_LOW, strlen(aszSpeedGrade)))
            {
                *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_4_10M;
                return;
            }
            else
            {
                *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_10_30M;
                return;
            }
        }
        else
        {
            *penSpeed = HI_STORAGE_TRANSMISSION_SPEED_BUTT;
            return;
        }
    }
}

static HI_S32 STG_DEVINFO_IsDevPortValid(STG_DEVINFO_S* pstDevInfo)
{
    HI_S32 s32Vlid = HI_SUCCESS;
    FILE* pFDevMCIProc = HI_NULL;
    HI_CHAR* pTmp = HI_NULL;
    HI_CHAR aszProcLineStr[STG_DEVINFO_PROC_LINE_LEN] = {0};

    STG_THREAD_MUTEX_LOCK(&pstDevInfo->mDevMCIProcLock);
    pFDevMCIProc = fopen(pstDevInfo->aszDevMCIPorcPath, "r");
    if (HI_NULL == pFDevMCIProc)
    {
        (HI_VOID)pthread_mutex_unlock(&pstDevInfo->mDevMCIProcLock);
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  open file failure, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    while (HI_NULL != fgets(aszProcLineStr, STG_DEVINFO_PROC_LINE_LEN - 1, pFDevMCIProc))
    {
        pTmp = strstr(aszProcLineStr, pstDevInfo->aszDevPort);
        if (pTmp)
        {
            break;
        }
    }

    if (!pTmp || strstr(aszProcLineStr, STG_DEVINFO_PORT_INVALID))
    {
        s32Vlid = HI_FAILURE;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev port(%s) is invalid!\n", __FUNCTION__, __LINE__, pstDevInfo->aszDevPort);
    }

    if (0 != fclose(pFDevMCIProc))
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  close file failure, system errno(%d)!\n", __FUNCTION__, __LINE__, errno);
    }

    (HI_VOID)pthread_mutex_unlock(&pstDevInfo->mDevMCIProcLock);
    return s32Vlid;
}

HI_S32 STG_DEVINFO_Create(HI_U8 u8DevPortNo, HI_U8 u8DevErrCountThr, const HI_CHAR* pszPartitionPath, HI_MW_PTR* ppDevInfo)
{
    HI_S32 s32Ret = EOK;
    STG_DEVINFO_S* pstDevInfo = (STG_DEVINFO_S*)malloc(sizeof(STG_DEVINFO_S));
    if (HI_NULL == pstDevInfo)
    {
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = memset_s(pstDevInfo, sizeof(STG_DEVINFO_S), 0x00, sizeof(STG_DEVINFO_S));
    if ( EOK != s32Ret )
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "pstDevInfo memset_s failed, s32Ret=0x%x\n", s32Ret);
    }

    if (HI_NULL == realpath(STG_DEVINFO_MCI_PROC_PATH, pstDevInfo->aszDevMCIPorcPath))
    {
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  Device MCI Proc path is not found, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    if (HI_SUCCESS != pthread_mutex_init(&pstDevInfo->mDevMCIProcLock, HI_NULL))
    {
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  init lock failed, and ret:%x, system errno(%d)!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_SYSTEM_CALL_FAILURE, errno);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = snprintf_s(pstDevInfo->aszDevPort, STG_DEVINFO_PROC_LINE_LEN, STG_DEVINFO_PROC_LINE_LEN - 1, "%s%d", STG_DEVINFO_MCI, u8DevPortNo);
    if ( s32Ret < 0 )
    {
        (HI_VOID)pthread_mutex_destroy(&pstDevInfo->mDevMCIProcLock);
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "pstDevInfo->aszDevPort snprintf_s failed!\n");
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    if (HI_SUCCESS != STG_DEVINFO_IsDevPortValid(pstDevInfo))
    {
        (HI_VOID)pthread_mutex_destroy(&pstDevInfo->mDevMCIProcLock);
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  dev port No is invalid, and ret:%x!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_ILLEGAL_PARAM);
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

#ifndef __HuaweiLite__
    s32Ret = snprintf_s(pstDevInfo->aszDevCIDPorcPath, PATH_MAX + 1, PATH_MAX, "%s%d%s", STG_DEVINFO_CID_PROC_PATH_TOP, u8DevPortNo, STG_DEVINFO_CID_PROC_PATH_BOTTOM);
    if ( s32Ret < 0 )
    {
        (HI_VOID)pthread_mutex_destroy(&pstDevInfo->mDevMCIProcLock);
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "pstDevInfo->aszDevCIDPorcPath snprintf_s failed!\n");
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }
#endif
    s32Ret = snprintf_s(pstDevInfo->aszPartitionPath, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pszPartitionPath);
    if ( s32Ret < 0 )
    {
        (HI_VOID)pthread_mutex_destroy(&pstDevInfo->mDevMCIProcLock);
        free(pstDevInfo);
        pstDevInfo = HI_NULL;
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "pstDevInfo->aszPartitionPath snprintf_s failed!\n");
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }

    pstDevInfo->u8DevErrCountThr = u8DevErrCountThr;
    pstDevInfo->s32ErrCurrentCnt = 0;
    *ppDevInfo = pstDevInfo;
    return HI_SUCCESS;
}

HI_VOID STG_DEVINFO_Destroy(HI_MW_PTR pDevInfo)
{
    STG_DEVINFO_S* pstDevInfo = (STG_DEVINFO_S*)pDevInfo;
    (HI_VOID)pthread_mutex_destroy(&pstDevInfo->mDevMCIProcLock);
    free(pDevInfo);
    return;
}

HI_S32 STG_DEVINFO_GetEventInfo(HI_MW_PTR pDevInfo, STG_DEV_EVENT_INFO_S* pInfo)
{
    STG_DEVINFO_S* pstDevInfo = (STG_DEVINFO_S*)pDevInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32ErrCnt = 0;
    STG_DEVINFO_VALID_INFO_S stInfo;
    s32Ret = memset_s(&stInfo, sizeof(STG_DEVINFO_VALID_INFO_S), 0x00, sizeof(STG_DEVINFO_VALID_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d memset failed:%x!\n", __FUNCTION__, __LINE__, s32Ret);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_DEVINFO_ReadMCI(pstDevInfo, &stInfo);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read proc failure, and ret:%x!\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }
    pInfo->enState = stInfo.enState;
    if (STG_DEV_STATE_CONNECTED == stInfo.enState)
    {
        s32ErrCnt = atoi(stInfo.aszErrCnt);
        if ((s32ErrCnt > 0) && (pstDevInfo->u8DevErrCountThr <= (s32ErrCnt - pstDevInfo->s32ErrCurrentCnt)))
        {
            //conect report error once.
            STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d sd err cnt %d happend \n", __FUNCTION__, __LINE__, s32ErrCnt);
            pstDevInfo->s32ErrCurrentCnt = s32ErrCnt;
            pInfo->u32ErrorCnt = s32ErrCnt;
        }
        else
        {
            pInfo->u32ErrorCnt = 0;
        }
    }
    else
    {
        pstDevInfo->s32ErrCurrentCnt = 0;
        pInfo->u32ErrorCnt = 0;
    }

    return HI_SUCCESS;
}

HI_S32 STG_DEVINFO_GetInfo(HI_MW_PTR pDevInfo, HI_STORAGE_DEV_INFO_S* pInfo)
{
    STG_DEVINFO_S* pstDevInfo = (STG_DEVINFO_S*)pDevInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    STG_DEVINFO_VALID_INFO_S stInfo;
    s32Ret = memset_s(&stInfo, sizeof(STG_DEVINFO_VALID_INFO_S), 0x00, sizeof(STG_DEVINFO_VALID_INFO_S));
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d memset failed:%x!\n", __FUNCTION__, __LINE__, s32Ret);
        return HI_ERR_STORAGE_SYSTEM_CALL_FAILURE;
    }

    s32Ret = STG_DEVINFO_ReadMCI(pstDevInfo, &stInfo);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read dev info failure, and ret:%x!\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    if (STG_DEV_STATE_CONNECTED != stInfo.enState)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  get info failure, and ret:%x!\n", __FUNCTION__, __LINE__, HI_ERR_STORAGE_DEV_DISCONNECT);
        return HI_ERR_STORAGE_DEV_DISCONNECT;
    }

    STG_DEVINFO_CalcTransSpeed(stInfo.aszWorkMode, stInfo.aszSpeedClass, stInfo.aszSpeedGrade, &pInfo->enTranSpeed);
    s32Ret = snprintf_s(pInfo->aszWorkMode, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", stInfo.aszWorkMode);
    if (s32Ret < 0)
    {
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }
    s32Ret = snprintf_s(pInfo->aszWorkClock, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", stInfo.aszWorkClock);
    if (s32Ret < 0)
    {
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }
    s32Ret = snprintf_s(pInfo->aszDevType, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", stInfo.aszDevType);
    if (s32Ret < 0)
    {
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }
#ifdef __HuaweiLite__
    s32Ret = snprintf_s(pInfo->aszCID, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", stInfo.aszCID);
    if (s32Ret < 0)
    {
        return HI_ERR_STORAGE_ILLEGAL_PARAM;
    }
#else
    s32Ret = STG_DEVINFO_ReadCID(pstDevInfo->aszDevCIDPorcPath, pInfo->aszCID);
    if (HI_SUCCESS != s32Ret)
    {
        STG_LOG_Printf(MODULE_STORAGE, STG_LOG_LEVEL_ERR, "%s  %d  read CID failure, and ret:%x!\n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }
#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
