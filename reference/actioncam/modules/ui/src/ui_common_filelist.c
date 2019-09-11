/**
 * @file      ui_common_filelist.c
 * @brief     ui internal common function code
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <unistd.h>
#include "ui_common_filelist.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** used when filemng busy is scanning, load only 4 groups*/
#define UI_FILELIST_PRELOADCOUNT (4)

static HI_U32 s_u32CurGroupIndex = INVALID_GROUP_INDEX;
static HI_U32 s_u32GroupCount = 0;

/** we don't use GroupNode's u32GroupIndex, just use pstNext */
static PDT_UI_COMM_FILELIST_GROUP_NODE_S s_stListHead = {.u32GroupIndex = 0, .pstNext=NULL};

HI_VOID PDT_UI_COMM_FILELIST_GetCurGrpIdx(HI_U32* pu32CurGroupIdx)
{
    *pu32CurGroupIdx = s_u32CurGroupIndex;
}

HI_VOID PDT_UI_COMM_FILELIST_SetCurGrpIdx(HI_U32 u32CurGroupIdx)
{
    s_u32CurGroupIndex = u32CurGroupIdx;
}

HI_VOID PDT_UI_COMM_FILELIST_GetCurGrpCnt(HI_U32* pu32CurGroupCnt)
{
    *pu32CurGroupCnt = s_u32GroupCount;
}

HI_VOID PDT_UI_COMM_FILELIST_SetCurGrpCnt(HI_U32 u32CurGroupCnt)
{
    u32CurGroupCnt = u32CurGroupCnt;
}

HI_BOOL PDT_UI_COMM_FILELIST_ListIsNull(HI_VOID)
{
    HI_BOOL bRet = ((NULL == s_stListHead.pstNext) ? HI_TRUE : HI_FALSE);
    return bRet;
}

HI_S32 PDT_UI_COMM_FILELIST_ListAddGrpIdx(HI_U32 u32GroupIndex)
{
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstTemp  = NULL;
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstGridNode
        = (PDT_UI_COMM_FILELIST_GROUP_NODE_S*)malloc(sizeof(PDT_UI_COMM_FILELIST_GROUP_NODE_S));

    if (NULL == pstGridNode)
    {
        MLOGE("malloc failed\n");
        return HI_FAILURE;
    }

    pstGridNode->pstNext = NULL;
    pstGridNode->u32GroupIndex = u32GroupIndex;

    pstTemp = &s_stListHead;

    while(NULL != pstTemp->pstNext
        && u32GroupIndex < pstTemp->pstNext->u32GroupIndex)
    {
        pstTemp = pstTemp->pstNext;
    }

    pstGridNode->pstNext = pstTemp->pstNext;
    pstTemp->pstNext = pstGridNode;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_FILELIST_ListFindGrpIdx(HI_U32 u32GroupIndex)
{
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstGridNode = s_stListHead.pstNext;

    while (NULL != pstGridNode)
    {
        if (pstGridNode->u32GroupIndex == u32GroupIndex)
        {
            return HI_SUCCESS;
        }

        pstGridNode = pstGridNode->pstNext;
    }

    return HI_FAILURE;
}

HI_S32 PDT_UI_COMM_FILELIST_ListDelGrpIdx(HI_U32 u32GroupIndex)
{
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstTemp = NULL;
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstDel = NULL;

    pstTemp = &s_stListHead;
    while( NULL != pstTemp->pstNext && pstTemp->pstNext->u32GroupIndex != u32GroupIndex)
    {
        pstTemp = pstTemp->pstNext;
    }

    if(NULL == pstTemp->pstNext)
    {
        MLOGD("not found %u\n", u32GroupIndex);
        return HI_SUCCESS;
    }
    pstDel = pstTemp->pstNext;
    pstTemp->pstNext = pstDel->pstNext;

    free(pstDel);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_FILELIST_ListDelNodeAndGrp(HI_VOID)
{
    HI_S32 s32Ret = 0;

    while (NULL != s_stListHead.pstNext)
    {
        s32Ret = HI_FILEMNG_DelGrpsFwd(s_stListHead.pstNext->u32GroupIndex, 1);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        PDT_UI_COMM_FILELIST_ListDelGrpIdx(s_stListHead.pstNext->u32GroupIndex);
    }
    sync();
    return HI_SUCCESS;
}

HI_VOID PDT_UI_COMM_FILELIST_ListClear(HI_VOID)
{
    PDT_UI_COMM_FILELIST_GROUP_NODE_S* pstDelNode = NULL;

    while (NULL != s_stListHead.pstNext)
    {
        pstDelNode = s_stListHead.pstNext;
        s_stListHead.pstNext = pstDelNode->pstNext;
        free(pstDelNode);
    }
    s_stListHead.pstNext = NULL;
}

HI_S32 PDT_UI_COMM_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_FILEMNG_SCAN_COMPLETED:
            PDT_UI_COMM_FILELIST_SetCurGrpCnt(0);
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
            s32Ret = HI_GV_ADM_Sync(ADMHdl);
            break;

        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_SD_UNAVAILABLE:
        case HI_EVENT_STORAGEMNG_DEV_ERROR:
        case HI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case HI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case HI_EVENT_STORAGEMNG_MOUNT_FAILED:
            s32Ret = HI_GV_ADM_Sync(ADMHdl);
            PDT_UI_COMM_FILELIST_SetCurGrpCnt(0);
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
            s32Ret |= HI_GV_Widget_Active(BackBtnHdl);
            break;

        default:
            MLOGD("Event ID %u not need to process\n", pstEvent->EventID);
            break;
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_FILELIST_Exit(HI_PDT_WORKMODE_E enWorkMode, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
    PDT_UI_COMM_FILELIST_SetCurGrpCnt(0);

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    if(stWorkModeState.bStateMngInProgress)
    {
        MLOGE("BUSY\n");
        return HI_SUCCESS;
    }

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = enWorkMode;

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, pfnMsgResultProc);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SendAsyncMsg failed %x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


/**
 * @brief used  by datamodel, get filemgr's group count. A grid's data come from a row, which corresponds a group.
 */
HI_S32 PDT_UI_COMM_FILELIST_GRID_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt)
{
    HI_APPCOMM_CHECK_POINTER(pu32RowCnt, HI_FAILURE);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_STORAGE_STATE_E enStorageState = HI_STORAGE_STATE_IDEL;

    s32Ret = PDT_UI_COMM_GetStorageState(&enStorageState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_u32GroupCount = 0;
    *pu32RowCnt = 0;

    if (HI_STORAGE_STATE_MOUNTED == enStorageState)
    {
        s32Ret = HI_FILEMNG_GetGrpCnt(&s_u32GroupCount);

        if(HI_SUCCESS == s32Ret)
        {
            *pu32RowCnt = s_u32GroupCount;
        }
        else if(HI_FILEMNG_EBUSY == s32Ret)
        {
            s_u32GroupCount = UI_FILELIST_PRELOADCOUNT;
            *pu32RowCnt = s_u32GroupCount;
        }
        else
        {
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    MLOGI(GREEN"GroupCount[%u]\n"NONE, *pu32RowCnt);
    return HI_SUCCESS;
}

static HI_U32 s_u32CurrentIdx = 0;
static PDT_UI_COMM_FILELISTROW_S *s_pstRow = NULL;

static HI_S32 PDT_UI_COMM_FILELIST_pfnSrchGrpCB(const HI_DCF_WRAPPER_GRP_S* pstGrp, HI_VOID* pvUserData)
{
    HI_S32 s32Ret;
    HI_U8  u8ObjTypIdx;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg = {};
    HI_FILEMNG_OBJ_FILENAME_S stFileNames = {};
    PDT_UI_COMM_FILELISTROW_S stRow = {};

    snprintf(stRow.szGrpFileCount, 16, "%u", pstGrp->u16ObjCnt);
    u8ObjTypIdx = pstGrp->u8ObjTypIdx;

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = HI_FILEMNG_GetObjFilenames(0, &stFileNames);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(HI_FILEMNG_FILE_TYPE_RECORD == stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].enType)
    {
        snprintf(stRow.szThumbnail,HI_APPCOMM_MAX_PATH_LEN,"%s",stFileNames.szFileName[2]);
        snprintf(stRow.szGrpType,10,"%s%s",stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].szFileFreeChar,
                 stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].aszExtension[0]);
    }
    else if(HI_FILEMNG_FILE_TYPE_PHOTO == stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].enType)
    {
        HI_U32 u32FileIdx = 0;
        for(u32FileIdx = 0; u32FileIdx < stFileNames.u8FileCnt; ++ u32FileIdx)
        {
            if(strstr(stFileNames.szFileName[u32FileIdx], "JPG") != NULL)
            {
                break;
            }
        }

        if(u32FileIdx >= stFileNames.u8FileCnt)
        {
            u32FileIdx = 0;
            /** TODO:This group does not contain JPG file */
        }

        snprintf(stRow.szThumbnail,HI_APPCOMM_MAX_PATH_LEN,"%s",stFileNames.szFileName[u32FileIdx]);
        snprintf(stRow.szGrpType,10,"%s%s",stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].szFileFreeChar,
                 stFileMngCfg.stDcfCfg.astObjCfgTbl[u8ObjTypIdx].aszExtension[u32FileIdx]);
    }

    stRow.u32Selected = (HI_SUCCESS == PDT_UI_COMM_FILELIST_ListFindGrpIdx(s_u32CurrentIdx)) ? 1 : 0;
    ++s_u32CurrentIdx;

    memcpy(s_pstRow, &stRow, sizeof(PDT_UI_COMM_FILELISTROW_S));
    s_pstRow++;

    return HI_SUCCESS;
}

/**
 * @brief  get N row's value.  The struct FileListRow_S is consistent with datamodel(datamodel_file_list).
 * @param DBSource DataBase source
 * @param u32StartNow: start from u32StartNow.
 * @param u32RowNumber: hope to get u32RowNumber rows.
 * @param pu32RowCount:  the final gotten row count
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_COMM_FILELIST_GRID_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartIndex, HI_U32 u32ReqNumber, HI_VOID* pData, HI_U32* pu32ResCnt)
{
    HI_APPCOMM_CHECK_POINTER(pData,HI_FAILURE);
    HI_APPCOMM_CHECK_POINTER(pu32ResCnt,HI_FAILURE);

    HI_S32 s32Ret;

    s_u32CurrentIdx = u32StartIndex;
    s_pstRow = (PDT_UI_COMM_FILELISTROW_S *)pData;
    *pu32ResCnt = 0;

    s32Ret = HI_FILEMNG_SrchGrpsFwd(u32StartIndex, u32ReqNumber, PDT_UI_COMM_FILELIST_pfnSrchGrpCB, pData);

    MLOGI(GREEN"StartIndex[%u], ReqNumber[%u]\n"NONE, u32StartIndex, u32ReqNumber);

    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    *pu32ResCnt = s_u32CurrentIdx - u32StartIndex;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_COMM_FILELIST_GRID_Register(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

/** used by datamodel, unregisterdatachange */
HI_S32 PDT_UI_COMM_FILELIST_GRID_Unregister(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

