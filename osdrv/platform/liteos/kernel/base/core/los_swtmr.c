/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "los_swtmr.inc"
#include "los_base.ph"
#include "los_sortlink.ph"
#include "los_membox.ph"
#include "los_memory.ph"
#include "los_queue.ph"
#include "los_task.ph"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#if (LOSCFG_BASE_CORE_SWTMR == YES)

LITE_OS_SEC_BSS UINT32                  m_uwSwTmrHandlerQueue; /*Software Timer timeout queue ID*/
LITE_OS_SEC_BSS SWTMR_CTRL_S     *m_pstSwtmrCBArray;          /*First address in Timer memory space  */
LITE_OS_SEC_BSS LOS_DL_LIST     m_pstSwtmrFreeList;          /*Free list of Software Timer*/
LITE_OS_SEC_BSS  SORTLINK_ATTRIBUTE_S               g_stSwtmrSortLink;/*The software timer count list*/
LITE_OS_SEC_BSS UINT8 *m_aucSwTmrHandlerPool = NULL;

__attribute__((section(".sysswtcb"))) SWTMR_CTRL_S m_pstSwtmrCBArray_0;
LITE_OS_SEC_DATA_INIT UINT32 g_uwSwtmrNumCfg;

extern CHAR LOS_CB_START(sysswtcb);
extern CHAR LOS_CB_END(sysswtcb);
extern CHAR LOS_BLOCK_END(sysswtcb);
extern CHAR LOS_BLOCK_START(sysswtcb);
/*****************************************************************************
Function   : osSwTmrTask
Description: SwTmr task
Input      : None
Output     : None
Return     : None
*****************************************************************************/
LITE_OS_SEC_TEXT VOID osSwTmrTask(VOID)
{
    SWTMR_HANDLER_ITEM_P pstSwtmrHandle = (SWTMR_HANDLER_ITEM_P)NULL;
    SWTMR_HANDLER_ITEM_S stSwtmrHandle;
    UINT32 uwRet;

    for ( ; ; )
    {
        uwRet = LOS_QueueRead(m_uwSwTmrHandlerQueue, &pstSwtmrHandle, sizeof(SWTMR_HANDLER_ITEM_P), LOS_WAIT_FOREVER);
        if (uwRet == LOS_OK)
        {
            if (pstSwtmrHandle != NULL)
            {
                stSwtmrHandle.pfnHandler = pstSwtmrHandle->pfnHandler;
                stSwtmrHandle.uwArg = pstSwtmrHandle->uwArg;
                (VOID)LOS_MemboxFree(m_aucSwTmrHandlerPool, pstSwtmrHandle);
                if (stSwtmrHandle.pfnHandler != NULL)
                {
                    stSwtmrHandle.pfnHandler(stSwtmrHandle.uwArg);
                }
            }
        }
    }//end of for
}

/*****************************************************************************
Function   : osSwTmrTaskCreate
Description: Create Software Timer
Input      : None
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 osSwTmrTaskCreate(VOID)
{
    UINT32 uwRet;
    TSK_INIT_PARAM_S stSwTmrTask;

    (VOID)memset(&stSwTmrTask, 0, sizeof(TSK_INIT_PARAM_S));
    stSwTmrTask.pfnTaskEntry    = (TSK_ENTRY_FUNC)osSwTmrTask;
    stSwTmrTask.uwStackSize     = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stSwTmrTask.pcName          = "Swt_Task";
    stSwTmrTask.usTaskPrio      = 0;
    stSwTmrTask.uwResved        = LOS_TASK_STATUS_DETACHED;
    uwRet = LOS_TaskCreate(&g_uwSwtmrTaskID, &stSwTmrTask);
    return uwRet;
}

/*****************************************************************************
Function   : osSwTmrInit
Description: Initializes Software Timer
Input      : None
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 osSwTmrInit(VOID)
{
    UINT16 usIndex;
    UINT32 uwRet;
    SWTMR_CTRL_S *pstSwtmr;
    UINT32 uwSwtmrHandlePoolSize;

    g_uwSwtmrNumCfg = (&LOS_BLOCK_END(sysswtcb) - &LOS_BLOCK_START(sysswtcb)) / sizeof(SWTMR_CTRL_S); /*lint !e573*/
    pstSwtmr = (SWTMR_CTRL_S *)&LOS_BLOCK_START(sysswtcb);
    if (0 == g_uwSwtmrNumCfg)
    {
        return LOS_ERRNO_SWTMR_MAXSIZE_INVALID;
    }
    (VOID)memset(pstSwtmr,0,(&LOS_BLOCK_END(sysswtcb) - &LOS_BLOCK_START(sysswtcb)));
    m_pstSwtmrCBArray = pstSwtmr;
    LOS_ListInit(&m_pstSwtmrFreeList);
    for (usIndex = 0; usIndex < LOSCFG_BASE_CORE_SWTMR_LIMIT; usIndex++, pstSwtmr++)
    {
        pstSwtmr->usTimerID = usIndex;
        LOS_ListTailInsert(&m_pstSwtmrFreeList, &pstSwtmr->stSortList.stList);
    }

    uwSwtmrHandlePoolSize = LOS_MEMBOX_SIZE(sizeof(SWTMR_HANDLER_ITEM_S), OS_SWTMR_HANDLE_QUEUE_SIZE);

    m_aucSwTmrHandlerPool = (UINT8 *)LOS_MemAlloc(m_aucSysMem1, uwSwtmrHandlePoolSize);

    uwRet = LOS_MemboxInit(m_aucSwTmrHandlerPool, uwSwtmrHandlePoolSize, sizeof(SWTMR_HANDLER_ITEM_S));
    if (uwRet != LOS_OK)
    {
        return LOS_ERRNO_SWTMR_HANDLER_POOL_NO_MEM;
    }

    uwRet = LOS_QueueCreate((CHAR *)NULL, OS_SWTMR_HANDLE_QUEUE_SIZE, &m_uwSwTmrHandlerQueue, 0, sizeof(SWTMR_HANDLER_ITEM_P));
    if (uwRet != LOS_OK)
    {
        return LOS_ERRNO_SWTMR_QUEUE_CREATE_FAILED;
    }

    uwRet = osSwTmrTaskCreate();
    if (LOS_OK != uwRet)
    {
        return LOS_ERRNO_SWTMR_TASK_CREATE_FAILED;
    }

    uwRet = osSortLinkInit(&g_stSwtmrSortLink);
    if(uwRet!= LOS_OK)
    {
        return LOS_ERRNO_SWTMR_SORTLINK_CREATE_FAILED;
    }

    return LOS_OK;
}

/*****************************************************************************
Function   : osSwTmrStart
Description: Start Software Timer
Input      : pstSwtmr ---------- Need to start Software Timer
Output     : None
Return     : None
*****************************************************************************/
LITE_OS_SEC_TEXT VOID osSwTmrStart(SWTMR_CTRL_S *pstSwtmr)
{
    if (pstSwtmr->ucOverrun == 0 && (pstSwtmr->ucMode == LOS_SWTMR_MODE_ONCE || pstSwtmr->ucMode == LOS_SWTMR_MODE_OPP ||
        pstSwtmr->ucMode == LOS_SWTMR_MODE_NO_SELFDELETE))
    {
        SET_SORTLIST_VALUE(&(pstSwtmr->stSortList), pstSwtmr->uwExpiry);
    }
    else
    {
        SET_SORTLIST_VALUE(&(pstSwtmr->stSortList), pstSwtmr->uwInterval);
    }

    osAdd2SortLink(&g_stSwtmrSortLink, &pstSwtmr->stSortList);
    pstSwtmr->ucState = OS_SWTMR_STATUS_TICKING;

    return;
}

/*****************************************************************************
Function   : osSwTmrDelete
Description: Delete Software Timer
Input      : pstSwtmr --- Need to delete Software Timer, When using, Ensure that it can't be NULL.
Output     : None
Return     : None
*****************************************************************************/
STATIC INLINE VOID osSwtmrDelete(SWTMR_CTRL_S *pstSwtmr)
{
    /**insert to free list **/
    LOS_ListTailInsert(&m_pstSwtmrFreeList, &pstSwtmr->stSortList.stList);
    pstSwtmr->ucState = OS_SWTMR_STATUS_UNUSED;
}

/*****************************************************************************
Function   : osSwtmrScan
Description: Tick interrupt interface module of Software Timer
Input      : None
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 osSwtmrScan(VOID)
{
    SORTLINK_LIST *pstSortList;
    SWTMR_CTRL_S  *pstSwtmr;
    SWTMR_HANDLER_ITEM_P pstSwtmrHandler;
    LOS_DL_LIST *pstListObject;

    g_stSwtmrSortLink.usCursor = (g_stSwtmrSortLink.usCursor + 1) & OS_TSK_SORTLINK_MASK;
    pstListObject = g_stSwtmrSortLink.pstSortLink + g_stSwtmrSortLink.usCursor;

    if(LOS_ListEmpty(pstListObject))
    {
        return LOS_OK;
    }
    pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
    UWROLLNUMDEC(pstSortList->uwIdxRollNum);

    while (UWROLLNUM(pstSortList->uwIdxRollNum) == 0)
    {
        pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
        LOS_ListDelete(&pstSortList->stList);
        pstSwtmr = LOS_DL_LIST_ENTRY(pstSortList, SWTMR_CTRL_S, stSortList);

        pstSwtmrHandler = (SWTMR_HANDLER_ITEM_P)LOS_MemboxAlloc(m_aucSwTmrHandlerPool);

        if (pstSwtmrHandler != NULL)
        {
            pstSwtmrHandler->pfnHandler = pstSwtmr->pfnHandler;
            pstSwtmrHandler->uwArg = pstSwtmr->uwArg;

            if (LOS_QueueWrite(m_uwSwTmrHandlerQueue, pstSwtmrHandler, sizeof(SWTMR_HANDLER_ITEM_P), LOS_NO_WAIT))
            {
                (VOID)LOS_MemboxFree(m_aucSwTmrHandlerPool, pstSwtmrHandler);
            }
        }

        if (pstSwtmr->ucMode == LOS_SWTMR_MODE_ONCE)
        {
            osSwtmrDelete(pstSwtmr);

            if (pstSwtmr->usTimerID < OS_SWTMR_MAX_TIMERID - LOSCFG_BASE_CORE_SWTMR_LIMIT)
                pstSwtmr->usTimerID += LOSCFG_BASE_CORE_SWTMR_LIMIT;
            else
                pstSwtmr->usTimerID %= LOSCFG_BASE_CORE_SWTMR_LIMIT;
        }
        else if (pstSwtmr->ucMode == LOS_SWTMR_MODE_NO_SELFDELETE)
        {
            pstSwtmr->ucState = OS_SWTMR_STATUS_CREATED;
        }
        else
        {
            pstSwtmr->ucOverrun++;
            osSwTmrStart(pstSwtmr);
        }

        pstSortList = LOS_DL_LIST_ENTRY((pstListObject)->pstNext, SORTLINK_LIST, stList);
    }

    return LOS_OK;
}

/*****************************************************************************
Function   : osSwTmrGetNextTimeout
Description: Get next timeout
Input      : None
Output     : None
Return     : Count of the Timer list
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 osSwTmrGetNextTimeout(VOID)
{
    return osSortLinkGetNextExpireTime(&g_stSwtmrSortLink);
}

/*****************************************************************************
Function   : osSwtmrStop
Description: Stop of Software Timer interface
Input      : pstSwtmr
Output     : None
Return     : None
*****************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID osSwtmrStop(SWTMR_CTRL_S *pstSwtmr)
{
    osDeleteSortLink(&g_stSwtmrSortLink, &pstSwtmr->stSortList);

    pstSwtmr->ucState = OS_SWTMR_STATUS_CREATED;
    pstSwtmr->ucOverrun = 0;
}

/*****************************************************************************
Function   : osSwtmrTimeGet
Description:
Input      : pstSwtmr
Output     : None
Return     : None
Other      : None
*****************************************************************************/
LITE_OS_SEC_TEXT STATIC UINT32 osSwtmrTimeGet(SWTMR_CTRL_S *pstSwtmr)
{
    return osSortLinkGetTargetExpireTime(&g_stSwtmrSortLink, &pstSwtmr->stSortList);
}

/*****************************************************************************
Function   : LOS_SwtmrCreate
Description: Create software timer
Input      : uwInterval
             usMode
             pfnHandler
             uwArg
Output     : pusSwTmrID
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SwtmrCreate(UINT32  uwInterval,
                                        UINT8           ucMode,
                                        SWTMR_PROC_FUNC pfnHandler,
                                        UINT16          *pusSwTmrID,
                                        AARCHPTR     uwArg)
{
    SWTMR_CTRL_S  *pstSwtmr;
    UINTPTR  uvIntSave;
    SORTLINK_LIST *pstSortList;

    if (0 == uwInterval)
    {
        return LOS_ERRNO_SWTMR_INTERVAL_NOT_SUITED;
    }

    if ((LOS_SWTMR_MODE_ONCE != ucMode) && (LOS_SWTMR_MODE_PERIOD != ucMode) &&
        (LOS_SWTMR_MODE_NO_SELFDELETE != ucMode))
    {
        return LOS_ERRNO_SWTMR_MODE_INVALID;
    }

    if (NULL == pfnHandler)
    {
        return LOS_ERRNO_SWTMR_PTR_NULL;
    }

    if (NULL == pusSwTmrID)
    {
        return LOS_ERRNO_SWTMR_RET_PTR_NULL;
    }

    uvIntSave = LOS_IntLock();
    if (LOS_ListEmpty(&m_pstSwtmrFreeList))
    {
        LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_SWTMR_MAXSIZE;
    }

    pstSortList = LOS_DL_LIST_ENTRY(m_pstSwtmrFreeList.pstNext, SORTLINK_LIST, stList);
    pstSwtmr = LOS_DL_LIST_ENTRY(pstSortList, SWTMR_CTRL_S, stSortList);
    LOS_ListDelete(LOS_DL_LIST_FIRST(&m_pstSwtmrFreeList));

    LOS_IntRestore(uvIntSave);
    pstSwtmr->pfnHandler    = pfnHandler;
    pstSwtmr->ucMode        = ucMode;
    pstSwtmr->ucOverrun     = 0;
    pstSwtmr->uwInterval    = uwInterval;
    pstSwtmr->uwExpiry      = uwInterval;
    pstSwtmr->uwArg         = uwArg;
    pstSwtmr->ucState       = OS_SWTMR_STATUS_CREATED;
    SET_SORTLIST_VALUE(&(pstSwtmr->stSortList), 0);
    *pusSwTmrID = pstSwtmr->usTimerID;

    return LOS_OK;
}

/*****************************************************************************
Function   : LOS_SwtmrStart
Description: Start software timer
Input      : usSwTmrID ------- Software timer ID
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrStart(UINT16 usSwTmrID)
{
    SWTMR_CTRL_S  *pstSwtmr;
    UINTPTR  uvIntSave;
    UINT32 uwRet = LOS_OK;
    UINT16 usSwTmrCBID;

    if (usSwTmrID >= OS_SWTMR_MAX_TIMERID)
    {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    uvIntSave = LOS_IntLock();
    usSwTmrCBID = usSwTmrID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    pstSwtmr = m_pstSwtmrCBArray + usSwTmrCBID;

    if (pstSwtmr->usTimerID != usSwTmrID)
    {
        LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    switch (pstSwtmr->ucState)
    {
    case OS_SWTMR_STATUS_UNUSED:
        uwRet = LOS_ERRNO_SWTMR_NOT_CREATED;
        break;
    /*If the status of swtmr is timing, it should stop the swtmr first,
        then start the swtmr again.*/
    case OS_SWTMR_STATUS_TICKING:
        osSwtmrStop(pstSwtmr);
    case OS_SWTMR_STATUS_CREATED: /*lint !e616*/
        osSwTmrStart(pstSwtmr);
        break;
    default:
        uwRet = LOS_ERRNO_SWTMR_STATUS_INVALID;
        break;
    }

    LOS_IntRestore(uvIntSave);
    return uwRet;
}

/*****************************************************************************
Function   : LOS_SwtmrStop
Description: Stop software timer
Input      : usSwTmrID ------- Software timer ID
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrStop(UINT16 usSwTmrID)
{
    SWTMR_CTRL_S *pstSwtmr;
    UINTPTR uvIntSave;
    UINT32 uwRet = LOS_OK;
    UINT16 usSwTmrCBID;

    if (usSwTmrID >= OS_SWTMR_MAX_TIMERID)
    {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    uvIntSave = LOS_IntLock();
    usSwTmrCBID = usSwTmrID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    pstSwtmr = m_pstSwtmrCBArray + usSwTmrCBID;

    if (pstSwtmr->usTimerID != usSwTmrID)
    {
        LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    switch (pstSwtmr->ucState)
    {
    case OS_SWTMR_STATUS_UNUSED:
        uwRet = LOS_ERRNO_SWTMR_NOT_CREATED;
        break;
    case OS_SWTMR_STATUS_CREATED:
        uwRet = LOS_ERRNO_SWTMR_NOT_STARTED;
        break;
    case OS_SWTMR_STATUS_TICKING:
        osSwtmrStop(pstSwtmr);
        break;
    default:
        uwRet = LOS_ERRNO_SWTMR_STATUS_INVALID;
        break;
    }

    LOS_IntRestore(uvIntSave);
    return uwRet;
}

LITE_OS_SEC_TEXT UINT32 LOS_SwtmrTimeGet(UINT16 usSwTmrID, UINT32 *uwTick)
{
    SWTMR_CTRL_S  *pstSwtmr;
    UINTPTR  uvIntSave;
    UINT32 uwRet = LOS_OK;
    UINT16 usSwTmrCBID;

    if (usSwTmrID >= OS_SWTMR_MAX_TIMERID)
    {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    if (uwTick == NULL)
    {
        return LOS_ERRNO_SWTMR_TICK_PTR_NULL;
    }

    uvIntSave = LOS_IntLock();
    usSwTmrCBID = usSwTmrID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    pstSwtmr = m_pstSwtmrCBArray + usSwTmrCBID;

    if (pstSwtmr->usTimerID != usSwTmrID)
    {
        LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }
    switch (pstSwtmr->ucState)
    {
    case OS_SWTMR_STATUS_UNUSED:
        uwRet = LOS_ERRNO_SWTMR_NOT_CREATED;
        break;
    case OS_SWTMR_STATUS_CREATED:
        uwRet = LOS_ERRNO_SWTMR_NOT_STARTED;
        break;
    case OS_SWTMR_STATUS_TICKING:
        *uwTick = osSwtmrTimeGet(pstSwtmr);
        break;
    default:
        uwRet = LOS_ERRNO_SWTMR_STATUS_INVALID;
        break;
    }
    LOS_IntRestore(uvIntSave);
    return uwRet;
}

/*****************************************************************************
Function   : LOS_SwtmrDelete
Description: Delete software timer
Input      : usSwTmrID ------- Software timer ID
Output     : None
Return     : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrDelete(UINT16 usSwTmrID)
{
    SWTMR_CTRL_S  *pstSwtmr;
    UINTPTR  uvIntSave;
    UINT32 uwRet = LOS_OK;
    UINT16 usSwTmrCBID;

    if (usSwTmrID >= OS_SWTMR_MAX_TIMERID)
    {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    uvIntSave = LOS_IntLock();
    usSwTmrCBID = usSwTmrID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    pstSwtmr = m_pstSwtmrCBArray + usSwTmrCBID;

    if (pstSwtmr->usTimerID != usSwTmrID)
    {
        LOS_IntRestore(uvIntSave);
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    switch (pstSwtmr->ucState)
    {
    case OS_SWTMR_STATUS_UNUSED:
        uwRet = LOS_ERRNO_SWTMR_NOT_CREATED;
        break;
    case OS_SWTMR_STATUS_TICKING:
        osSwtmrStop(pstSwtmr);
    case OS_SWTMR_STATUS_CREATED:  /*lint !e616*/
        osSwtmrDelete(pstSwtmr);
        break;
    default:
        uwRet = LOS_ERRNO_SWTMR_STATUS_INVALID;
        break;
    }

    LOS_IntRestore(uvIntSave);
    return uwRet;
}

#endif /*(LOSCFG_BASE_CORE_SWTMR == YES)*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
