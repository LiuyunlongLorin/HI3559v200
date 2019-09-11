#include <stdio.h>
#include <string.h>
#include "stg_log.h"
#include "hi_userproc.h"
#include "hi_storage.h"
#include "stg_proc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static STG_PROC_S s_stStgProcHeader;

static HI_S32 STG_PROC_Show(HI_UPROC_SHOW_BUFFER_S* pstBuf, __attribute__((unused))HI_VOID* pPrivData)
{
    HI_U32 i = 0;
    if (!s_stStgProcHeader.bValid)
    {
        return HI_SUCCESS;
    }
    HI_UPROC_Printf(pstBuf, "Storage\n");

    for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)
    {
        if (s_stStgProcHeader.astInst[i].bValid)
        {
            HI_UPROC_Printf(pstBuf, "\taszPartition : %s\n", s_stStgProcHeader.astInst[i].aszPartition);
            HI_UPROC_Printf(pstBuf, "\tbRunning                 :%d\n", s_stStgProcHeader.astInst[i].bRunning);
            HI_UPROC_Printf(pstBuf, "\tenCurState               :%d\n", s_stStgProcHeader.astInst[i].enCurState);
            HI_UPROC_Printf(pstBuf, "\tu64FSCheckDuration(us)   :%llu\n", s_stStgProcHeader.astInst[i].u64FSCheckDuration);
            HI_UPROC_Printf(pstBuf, "\tu64MountDuration(us)     :%llu\n", s_stStgProcHeader.astInst[i].u64MountDuration);
            HI_UPROC_Printf(pstBuf, "\tu64FormatDuration(us)    :%llu\n", s_stStgProcHeader.astInst[i].u64FormatDuration);
            HI_UPROC_Printf(pstBuf, "\tbFragmentSeverely        :%d\n", s_stStgProcHeader.astInst[i].bFragmentSeverely);
        }
    }
    return HI_SUCCESS;
}


static HI_UPROC_ENTRY_S s_stProcEntry;

static HI_VOID STG_PROC_Enable()
{
    s_stProcEntry.pszDirectory = (HI_CHAR*)HI_UPROC_MW_DIR_NAME;
    s_stProcEntry.pszEntryName = (HI_CHAR*)STG_PROC_DIR_NAME;
    s_stProcEntry.pfnShowProc = STG_PROC_Show;
    s_stProcEntry.pfnCmdProc = NULL;
    s_stProcEntry.pPrivData = NULL;
#ifdef ENABLE_PROC
    if (s_stStgProcHeader.bValid)
    {
        return;
    }
    if ((HI_SUCCESS == HI_UPROC_Init())
        && (HI_SUCCESS == HI_UPROC_AddEntry(HI_APPID_DCF, &s_stProcEntry)))
    {
        s_stStgProcHeader.bValid = HI_TRUE;
    }
#endif

    return;
}

static HI_VOID STG_PROC_Disable()
{
#ifdef ENABLE_PROC
    HI_S32 s32Ret = EOK;
    if (!s_stStgProcHeader.bValid)
    {
        return;
    }

    HI_UPROC_RemoveEntry(0, &s_stProcEntry);
    HI_UPROC_DeInit();
    s32Ret = memset_s(&s_stStgProcHeader, sizeof(STG_PROC_S), 0x00, sizeof(STG_PROC_S));
    if ( EOK != s32Ret )
    {
        STG_LOG_Printf("STG_PROC", STG_LOG_LEVEL_ERR, "memset_s s_stStgProcHeader failed, s32Ret=0x%x\n", s32Ret);
    }
#endif

    return;
}

static STG_PROC_INST_INFO_S* STG_PROC_GetFreeInst()
{
    HI_U8 i = 0;
    for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)
    {
        if (!s_stStgProcHeader.astInst[i].bValid)
        {
            s_stStgProcHeader.astInst[i].bValid = HI_TRUE;
            return &s_stStgProcHeader.astInst[i];
        }
    }

    return HI_NULL;
}

static STG_PROC_INST_INFO_S* STG_PROC_GetBusyInst(HI_MW_PTR hInst)
{
    HI_U8 i = 0;
    for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)
    {
        if (s_stStgProcHeader.astInst[i].bValid)
        {
            if (s_stStgProcHeader.astInst[i].hInst == hInst)
            {
                return &s_stStgProcHeader.astInst[i];
            }
        }
    }

    return HI_NULL;
}

static HI_BOOL STG_PROC_IsAllInstDisabled()
{
    HI_U8 i = 0;
    for (i = 0; i < HI_STORAGE_INST_CNT_MAX; i++)
    {
        if (s_stStgProcHeader.astInst[i].bValid)
        {
            return HI_FALSE;
        }
    }

    return HI_TRUE;
}

HI_VOID STG_PROC_EnableInst(HI_MW_PTR hInst, const HI_CHAR* pszPartition)
{
#ifdef ENABLE_PROC
    HI_S32 s32Ret = EOK;
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    STG_PROC_Enable();
    pstInst = STG_PROC_GetFreeInst();
    if (HI_NULL == pstInst)
    {
        return;
    }
    pstInst->hInst = hInst;
    s32Ret = snprintf_s(pstInst->aszPartition, HI_STORAGE_PATH_LEN_MAX, HI_STORAGE_PATH_LEN_MAX - 1, "%s", pszPartition);
    if ( s32Ret < 0 )
    {
        STG_LOG_Printf("STG_PROC", STG_LOG_LEVEL_ERR, "snprintf_s pstInst->aszPartition failed\n");
        return;
    }
    pstInst->bRunning = HI_TRUE;
#endif

    return;
}

HI_VOID STG_PROC_DisableInst(HI_MW_PTR hInst)
{
#ifdef ENABLE_PROC
    HI_S32 s32Ret = EOK;
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }

    s32Ret = memset_s(pstInst, sizeof(STG_PROC_INST_INFO_S), 0x00, sizeof(STG_PROC_INST_INFO_S));
    if ( EOK != s32Ret )
    {
        STG_LOG_Printf("STG_PROC", STG_LOG_LEVEL_ERR, "memset_s pstInst failed, s32Ret=0x%x\n", s32Ret);
    }

    if (STG_PROC_IsAllInstDisabled())
    {
        STG_PROC_Disable();
    }
#endif

    return;
}

HI_VOID STG_PROC_RefRunningState(HI_MW_PTR hInst, HI_BOOL bRunning)
{
#ifdef ENABLE_PROC
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }
    pstInst->bRunning = bRunning;
#endif

    return;
}

HI_VOID STG_PROC_RefCurState(HI_MW_PTR hInst, HI_STORAGE_STATE_E enState)
{
#ifdef ENABLE_PROC
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }
    pstInst->enCurState = enState;
#endif

    return;
}
HI_VOID STG_PROC_SetStartTime(HI_MW_PTR hInst, STG_PROC_DURATION_TYPE_E enState)
{
#ifdef ENABLE_PROC
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }

    if (STG_PROC_DURATION_TYPE_FSCHECK == enState)
    {
        gettimeofday (&pstInst->stFSChkStartTime, HI_NULL);
    }
    else if (STG_PROC_DURATION_TYPE_FORMAT == enState)
    {
        gettimeofday (&pstInst->stFmtStartTime, HI_NULL);
    }
    else
    {
        gettimeofday (&pstInst->stMntStartTime, HI_NULL);
    }
#endif

    return;
}
HI_VOID STG_PROC_RefDuration(HI_MW_PTR hInst, STG_PROC_DURATION_TYPE_E enState)
{
#ifdef ENABLE_PROC
    struct timeval* pstStartTime = HI_NULL;
    struct timeval stEndTime = {0};

    gettimeofday (&stEndTime, HI_NULL);

    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }

    if (STG_PROC_DURATION_TYPE_FSCHECK == enState)
    {
        pstStartTime = &pstInst->stFSChkStartTime;
        pstInst->u64FSCheckDuration = (HI_U64) (stEndTime.tv_sec - pstStartTime->tv_sec) * 1000 * 1000 + (stEndTime.tv_usec - pstStartTime->tv_usec);
    }
    else if (STG_PROC_DURATION_TYPE_FORMAT == enState)
    {
        pstStartTime = &pstInst->stFmtStartTime;
        pstInst->u64FormatDuration = (HI_U64) (stEndTime.tv_sec - pstStartTime->tv_sec) * 1000 * 1000 + (stEndTime.tv_usec - pstStartTime->tv_usec);
    }
    else
    {
        pstStartTime = &pstInst->stMntStartTime;
        pstInst->u64MountDuration = (HI_U64) (stEndTime.tv_sec - pstStartTime->tv_sec) * 1000 * 1000 + (stEndTime.tv_usec - pstStartTime->tv_usec);
    }
#endif

    return;
}
HI_VOID STG_PROC_RefFragment(HI_MW_PTR hInst, HI_STORAGE_FSTOOL_ERR_E enErr)
{
#ifdef ENABLE_PROC
    STG_PROC_INST_INFO_S* pstInst = HI_NULL;
    pstInst = STG_PROC_GetBusyInst(hInst);
    if (HI_NULL == pstInst)
    {
        return;
    }

    if (HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY == enErr)
    {
        pstInst->bFragmentSeverely = HI_TRUE;
    }
    else
    {
        pstInst->bFragmentSeverely = HI_FALSE;
    }
#endif

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
