#ifndef _STG_PROC_H_
#define _STG_PROC_H_

#include "securec.h"
#include <sys/time.h>
#include "hi_mw_type.h"
#include "hi_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STG_PROC_DIR_NAME    "storage"

typedef enum tagSTG_PROC_DURATION_TYPE_E
{
    STG_PROC_DURATION_TYPE_FSCHECK = 0x00,
    STG_PROC_DURATION_TYPE_FORMAT,
    STG_PROC_DURATION_TYPE_MOUNT,
    STG_PROC_DURATION_TYPE_BUTT
}STG_PROC_DURATION_TYPE_E;

typedef struct tagSTG_PROC_INST_INFO_S
{
    HI_BOOL bValid;
    HI_MW_PTR hInst;
    HI_CHAR aszPartition[HI_STORAGE_PATH_LEN_MAX];
    HI_STORAGE_STATE_E enCurState;
    struct timeval stFSChkStartTime;
    struct timeval stFmtStartTime;
    struct timeval stMntStartTime;
    HI_U64 u64FSCheckDuration;
    HI_U64 u64FormatDuration;
    HI_U64 u64MountDuration;
    HI_BOOL bFragmentSeverely;
    HI_BOOL bRunning;
}STG_PROC_INST_INFO_S;

typedef struct tagSTG_PROC_S
{
    HI_BOOL bValid;
    STG_PROC_INST_INFO_S astInst[HI_STORAGE_INST_CNT_MAX];
}STG_PROC_S;

HI_VOID STG_PROC_EnableInst(HI_MW_PTR hInst, const HI_CHAR* pszPartition);
HI_VOID STG_PROC_DisableInst(HI_MW_PTR hInst);
HI_VOID STG_PROC_RefRunningState(HI_MW_PTR hInst, HI_BOOL bRunning);
HI_VOID STG_PROC_RefCurState(HI_MW_PTR hInst, HI_STORAGE_STATE_E enState);
HI_VOID STG_PROC_SetStartTime(HI_MW_PTR hInst, STG_PROC_DURATION_TYPE_E enState);
HI_VOID STG_PROC_RefDuration(HI_MW_PTR hInst, STG_PROC_DURATION_TYPE_E enState);
HI_VOID STG_PROC_RefFragment(HI_MW_PTR hInst, HI_STORAGE_FSTOOL_ERR_E enErr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*_STG_PROC_H_*/
