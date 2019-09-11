/******************************* Include Files *******************************/

/* Sys headers */
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>

#include "hi_type.h"
#include "hi_userproc.h"
#include "hi_common_userproc.h"
#include "mpi_userproc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define MODULE_NAME_UPROC  "UPROC"

#define HI_FATAL_UPROC(fmt, ...)    printf("[%s][%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define HI_ERR_UPROC(fmt, ...)  printf("[%s][%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define HI_WARN_UPROC(fmt, ...)  printf("[%s][%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define HI_INFO_UPROC(fmt, ...)  printf("[%s][%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define UPROC_LOCK(Mutex)\
do{\
    HI_S32 s32lockRet = pthread_mutex_lock(&Mutex);\
    if ( s32lockRet != 0 )\
    {\
        HI_ERR_UPROC("UPROC_LOCK error\n");\
    }\
}while(0)

#define UPROC_UNLOCK(Mutex)\
do{\
    HI_S32 s32unlockRet = pthread_mutex_unlock(&Mutex);\
    if ( s32unlockRet != 0 )\
    {\
        HI_ERR_UPROC("UPROC_UNLOCK error\n");\
    }\
}while(0)


#define UPROC_CHECK_INIT\
    UPROC_LOCK(g_stUprocParam.stMutex);\
    if (-1 == g_stUprocParam.s32Fd)\
    {\
        UPROC_UNLOCK(g_stUprocParam.stMutex);\
        HI_ERR_UPROC("USERPROC not init!\n");\
        return HI_FAILURE;\
    }\
    UPROC_UNLOCK(g_stUprocParam.stMutex);

#define CHECK_UPROC_NULL_PTR(ptr)\
    do{\
        if(HI_NULL == ptr)\
        {\
            printf("UPROC NULL pointer\n" );\
            return HI_UPROC_ERR_NULL_PTR;\
        }\
    }while(0)

/*************************** Structure Definition ****************************/

typedef struct tagUSERPROC_PARAM_S
{
    HI_S32      s32Inited;
    pthread_mutex_t stMutex;
}USERPROC_PARAM_S;


/***************************** Global Definition *****************************/


/***************************** Static Definition *****************************/

static USERPROC_PARAM_S g_stUprocParam =
{
    .s32Inited = 0,
    .stMutex = PTHREAD_MUTEX_INITIALIZER,
};

static HI_BOOL s_bEnableUproc = HI_TRUE;
HI_VOID HI_UPROC_Enable(HI_BOOL bEnable)
{
    UPROC_LOCK(g_stUprocParam.stMutex);
    s_bEnableUproc = bEnable;
    UPROC_UNLOCK(g_stUprocParam.stMutex);
}

HI_S32 HI_MW_EnableDebug(HI_BOOL bDebug)
{
    HI_UPROC_Enable(bDebug);
    /*
    if (bDebug)
    {
        return HI_LOG_SetEnabledLevel(HI_LOG_LEVEL_DEBUG);
    }
    else
    {
        return HI_LOG_SetEnabledLevel(HI_LOG_LEVEL_ERR);
    }
    */
    return HI_SUCCESS;
}

HI_S32 HI_UPROC_Printf(HI_UPROC_SHOW_BUFFER_S *pstBuf, const HI_CHAR *pFmt, ...)
{
    HI_U32 u32Len = 0;
    va_list args;

    if ((HI_NULL == pstBuf) || (HI_NULL == pstBuf->pu8Buf) || (HI_NULL == pFmt))
    {
        return HI_FAILURE;
    }

    /* log buffer overflow */
    if (pstBuf->u32Offset >= pstBuf->u32Size)
    {
        HI_ERR_UPROC("userproc log buffer(size:%d) overflow.\n", pstBuf->u32Size);
        return HI_FAILURE;
    }

    va_start(args, pFmt);
    u32Len = (HI_U32)vsnprintf((HI_CHAR*)pstBuf->pu8Buf + pstBuf->u32Offset,
                            pstBuf->u32Size - pstBuf->u32Offset, pFmt, args);
    va_end(args);

    pstBuf->u32Offset += u32Len;

    return HI_SUCCESS;
}

/*********************************** Code ************************************/
HI_S32 HI_UPROC_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    UPROC_LOCK(g_stUprocParam.stMutex);
    if(HI_FALSE == s_bEnableUproc)
    {
        UPROC_UNLOCK(g_stUprocParam.stMutex);
        return HI_SUCCESS;
    }
    if(0 == g_stUprocParam.s32Inited)
    {
        s32Ret = HI_PROC_Init();
        if(HI_SUCCESS != s32Ret)
        {
            HI_ERR_UPROC("UPROC init error s32Ret:%d\n",s32Ret);
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            return s32Ret;
        }
        s32Ret = HI_PROC_AddDir(HI_UPROC_MW_DIR_NAME);
        if(HI_SUCCESS != s32Ret)
        {
            HI_ERR_UPROC("UPROC add Dir %s error s32Ret:%d\n", HI_UPROC_MW_DIR_NAME, s32Ret);
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            return s32Ret;
        }
        g_stUprocParam.s32Inited ++;
    }
    else if(g_stUprocParam.s32Inited > 0)
    {
        g_stUprocParam.s32Inited++;
    }
    else
    {
        HI_ERR_UPROC("UPROC init status error s32Inited:%d\n",g_stUprocParam.s32Inited);
    }
    UPROC_UNLOCK(g_stUprocParam.stMutex);
    return HI_SUCCESS;
}

HI_S32 HI_UPROC_DeInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    UPROC_LOCK(g_stUprocParam.stMutex);
    if(HI_FALSE == s_bEnableUproc)
    {
        UPROC_UNLOCK(g_stUprocParam.stMutex);
        return HI_SUCCESS;
    }
    if(1 == g_stUprocParam.s32Inited)
    {
        s32Ret = HI_PROC_RemoveDir(HI_UPROC_MW_DIR_NAME);
        if(HI_SUCCESS != s32Ret)
        {
            HI_ERR_UPROC("UPROC Remove Dir %s error s32Ret:%d\n", HI_UPROC_MW_DIR_NAME, s32Ret);
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            return s32Ret;
        }
        s32Ret = HI_PROC_DeInit();
        if(HI_SUCCESS != s32Ret)
        {
            HI_ERR_UPROC("UPROC Deinit error s32Ret:%d\n",s32Ret);
            UPROC_UNLOCK(g_stUprocParam.stMutex);
            return s32Ret;
        }
        g_stUprocParam.s32Inited--;
    }
    else if(g_stUprocParam.s32Inited > 1)
    {
        g_stUprocParam.s32Inited--;
    }
    else
    {
        HI_ERR_UPROC("UPROC is not inited status s32Inited:%d\n",g_stUprocParam.s32Inited);
    }
    UPROC_UNLOCK(g_stUprocParam.stMutex);
    return HI_SUCCESS;
}

HI_S32 HI_UPROC_AddEntry(HI_U32 u32ModuleID, const HI_UPROC_ENTRY_S* pstEntry)
{
    CHECK_UPROC_NULL_PTR(pstEntry);
    if(HI_FALSE == s_bEnableUproc)
    {
        return HI_SUCCESS;
    }
    HI_PROC_ENTRY_S stTmpEntrys = {HI_NULL,HI_NULL,HI_NULL,HI_NULL,HI_NULL};
    stTmpEntrys.pszEntryName = pstEntry->pszEntryName;
    stTmpEntrys.pszDirectory = pstEntry->pszDirectory;
    stTmpEntrys.pfnCmdProc = (HI_PROC_CMD_FN)pstEntry->pfnCmdProc;
    stTmpEntrys.pfnShowProc = (HI_PROC_SHOW_FN)pstEntry->pfnShowProc;
    stTmpEntrys.pPrivData = pstEntry->pPrivData;
    return HI_PROC_AddEntry(u32ModuleID, &stTmpEntrys);
}

HI_S32 HI_UPROC_RemoveEntry(HI_U32 u32ModuleID, const HI_UPROC_ENTRY_S* pstEntry)
{
    CHECK_UPROC_NULL_PTR(pstEntry);
    if(HI_FALSE == s_bEnableUproc)
    {
        return HI_SUCCESS;
    }
    HI_PROC_ENTRY_S stTmpEntrys = {HI_NULL,HI_NULL,HI_NULL,HI_NULL,HI_NULL};
    stTmpEntrys.pszEntryName = pstEntry->pszEntryName;
    stTmpEntrys.pszDirectory = pstEntry->pszDirectory;
    stTmpEntrys.pfnCmdProc = (HI_PROC_CMD_FN)pstEntry->pfnCmdProc;
    stTmpEntrys.pfnShowProc = (HI_PROC_SHOW_FN)pstEntry->pfnShowProc;
    stTmpEntrys.pPrivData = pstEntry->pPrivData;
    return HI_PROC_RemoveEntry(u32ModuleID, &stTmpEntrys);
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

