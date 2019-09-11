/**
* @file    ui_filemng_page.c
* @brief   ui_filemng_page.c to enter different file directory
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/5
* @version   1.0

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ui_common.h"
#include "hi_gv_listbox.h"
#include "hi_filemng_dtcf.h"
#include "hi_playback.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define UI_ALBUM_INVALID_ROW (0xFFFFFFFF)

typedef struct tagPDT_UI_ALBUM_LEVEL_S
{
    HI_DTCF_DIR_E enDirType;
    HI_U32 u32StartRowIndex;
    HI_U32 u32ActiveRowIndex;
} PDT_UI_ALBUM_LEVEL_S;

/** used by DDB, for show filemng rows*/
typedef struct tagPDT_UI_ALBUM_ROW_S
{
    HI_CHAR szTitle[HI_DIR_LEN_MAX];   /**< show text */
    HI_DTCF_DIR_E enDirType;       /**< show what */
} PDT_UI_ALBUM_ROW_S;

static HIGV_HANDLE s_DataBufferHdl = INVALID_HANDLE;
static PDT_UI_ALBUM_LEVEL_S s_stLevel = {DTCF_DIR_NORM_FRONT, UI_ALBUM_INVALID_ROW, UI_ALBUM_INVALID_ROW};

static HI_VOID UI_ALBUM_UpdateLevelInfo(HI_DTCF_DIR_E enDirType, HI_U32 u32StartRow, HI_U32 u32ActiveRow)
{
    s_stLevel.enDirType = enDirType;
    s_stLevel.u32StartRowIndex = u32StartRow;
    s_stLevel.u32ActiveRowIndex= u32ActiveRow;
}

static HI_S32 PDT_UI_ALBUM_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    if(HI_SUCCESS != pstEvent->s32Result)
    {
        MLOGE(" event result failed \n");
        return HI_FAILURE;
    }

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_StartWindow");

            s32Ret = PDT_UI_WINMNG_FinishWindow(ALBUM_PAGE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_FinishWindow");
            break;
        default:
            MLOGD("Event ID %u not need to be processed here\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_ALBUM_OnEventProc(HI_EVENT_S * pstEvent)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);

    MLOGD("EventID:%#x, arg1:%#x, arg2:%#x, result:%d\n", pstEvent->EventID, pstEvent->arg1,
          pstEvent->arg2,pstEvent->s32Result);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_ALBUM_Exit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

    HI_PDT_STATEMNG_WORKMODE_STATE_S stWorkModeState = {.bStateMngInProgress = HI_FALSE,};
    s32Ret = HI_PDT_STATEMNG_GetState(&stWorkModeState);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetState");
    if(stWorkModeState.bStateMngInProgress)
    {
        MLOGE("BUSY\n");
        return HI_SUCCESS;
    }

    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetPoweronWorkmode");
    MLOGD("enPoweronWorkmode(%d)\n", enPoweronWorkmode);

    HI_U32 u32Index = 0;
    HI_U32 u32DirCount = 0;
    HI_PDT_FILEMNG_CFG_S stCfg = {};
    HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
    {
        if ( 0 < strlen(stCfg.stDtcfCfg.aszDirNames[u32Index]))
        {
            aenDirs[u32DirCount++] = u32Index;
        }
    }

    HI_U32 u32Temp = 0;
    s32Ret = HI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32Temp);

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = enPoweronWorkmode;

    HI_PLAYBACK_Deinit();
    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_ALBUM_OnReceiveMsgResult);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_FAILURE;
    }

    s_stLevel.enDirType = DTCF_DIR_NORM_FRONT;
    s_stLevel.u32ActiveRowIndex = UI_ALBUM_INVALID_ROW;
    s_stLevel.u32StartRowIndex = UI_ALBUM_INVALID_ROW;

    return HI_SUCCESS;
}

static HI_VOID UI_ALBUM_ShowList(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_U32 u32Index = 0;
    HIGV_DBROW_S stDbRow = {};
    PDT_UI_ALBUM_ROW_S stSetRow = {};

    HI_U32 u32StartIndex = s_stLevel.u32StartRowIndex;
    HI_U32 u32SelectIndex = s_stLevel.u32ActiveRowIndex;

    HI_PDT_FILEMNG_CFG_S stCfg = {};
    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "HI_PDT_PARAM_GetFileMngCfg");

    MLOGD("u32StartIndex = %u, u32SelectIndex = %u\n", u32StartIndex, u32SelectIndex);

    s32Ret = HI_GV_DDB_EnableDataChange(s_DataBufferHdl, HI_FALSE);
    s32Ret |= HI_GV_DDB_Clear(s_DataBufferHdl);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "DDB_Clear");

    for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
    {
        if ( 0 >= strlen(stCfg.stDtcfCfg.aszDirNames[u32Index]))
        {
            continue;
        }
        memset(&stDbRow, 0, sizeof(stDbRow));
        memset(&stSetRow, 0, sizeof(stSetRow));

        stSetRow.enDirType = u32Index;
        s32Ret = PDT_UI_COMM_Translate(stCfg.stDtcfCfg.aszDirNames[stSetRow.enDirType], stSetRow.szTitle, HI_DIR_LEN_MAX);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "DTCF_DIR_Translate");
        stDbRow.pData = &stSetRow;
        stDbRow.Size = sizeof(PDT_UI_ALBUM_ROW_S);

        s32Ret = HI_GV_DDB_Append(s_DataBufferHdl, &stDbRow);
        PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "DDB_Append");
    }

    s32Ret = HI_GV_DDB_EnableDataChange(s_DataBufferHdl, HI_TRUE);
    s32Ret |= HI_GV_Widget_SyncDB(ALBUM_LISTBOX);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SyncDB");

    /*s32Ret = HI_GV_List_SetStartItem(ALBUM_LISTBOX, u32StartIndex);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetStartItem");

    s32Ret = HI_GV_List_SetSelItem(ALBUM_LISTBOX, u32SelectIndex);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "SetSelItem");*/

    s32Ret = HI_GV_Widget_Paint(ALBUM_LISTBOX, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "Widget_Paint");
}

HI_S32 PDT_UI_ALBUM_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_ADM_GetDDBHandle(DATAMODEL_ALBUM, &s_DataBufferHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "GetDDBHandle");

    UI_ALBUM_ShowList();

    s32Ret = HI_GV_Widget_Active(ALBUM_LISTBOX);

    if((DTCF_DIR_NORM_FRONT == s_stLevel.enDirType) && (UI_ALBUM_INVALID_ROW == s_stLevel.u32StartRowIndex)
        && (UI_ALBUM_INVALID_ROW == s_stLevel.u32ActiveRowIndex))
    {
        s32Ret = HI_GV_List_SetSelItem(ALBUM_LISTBOX, 0);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_List_SetSelItem");
    }
    else
    {
        s32Ret = HI_GV_List_SetStartItem(ALBUM_LISTBOX, s_stLevel.u32StartRowIndex);
        s32Ret |= HI_GV_List_SetSelItem(ALBUM_LISTBOX, s_stLevel.u32ActiveRowIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_List_SetSelItem");
    }

    PDT_UI_PLAYBACK_VideoPlayerInit();

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_end();
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALBUM_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALBUM_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALBUM_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALBUM_OnKeyUp(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}

static HI_VOID UI_ALBUM_OnClickRow(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ActiveRow = 0;
    HI_U32 u32StartRow = 0;
    HI_U32 u32ItemNum = 0;
    PDT_UI_ALBUM_ROW_S stSetRow = {};
    HIGV_DBROW_S stDbRow = {};

    stDbRow.Size = sizeof(PDT_UI_ALBUM_ROW_S);
    stDbRow.pData = &stSetRow;

    s32Ret = HI_GV_List_GetStartItem(ALBUM_LISTBOX, &u32StartRow);
    s32Ret |= HI_GV_List_GetSelItem(ALBUM_LISTBOX, &u32ActiveRow);
    s32Ret |= HI_GV_List_GetItemNum(ALBUM_LISTBOX, &u32ItemNum);
    if (s32Ret != HI_SUCCESS)
    {
        MLOGE("Error:%#x\n", s32Ret);
    }

    MLOGD("u32StartRow = %u, u32ActiveRow = %u\n", u32StartRow, u32ActiveRow);

    s32Ret = HI_GV_DDB_GetRow(s_DataBufferHdl, u32ActiveRow, &stDbRow);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("DDB GetRow failed. %#x\n", s32Ret);
        return;
    }

    UI_ALBUM_UpdateLevelInfo(stSetRow.enDirType, u32StartRow, u32ActiveRow);
    PDT_UI_FILELIST_SetSearchScope(s_stLevel.enDirType);
    MLOGD("DirType:%d\n",s_stLevel.enDirType);

    s32Ret = PDT_UI_WINMNG_StartWindow(FILELIST_PAGE, HI_TRUE);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "StartWindow");

}

static HI_S32 PDT_UI_ALBUM_FocusToNext(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_List_GetSelItem(ALBUM_LISTBOX, &u32CurrentRow);
    s32Ret |= HI_GV_List_GetItemNum(ALBUM_LISTBOX, &u32ItemNum);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s32Ret = HI_GV_List_SetSelItem(ALBUM_LISTBOX, (u32CurrentRow+1)%u32ItemNum );
    s32Ret |=HI_GV_Widget_Paint(ALBUM_LISTBOX, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, (u32CurrentRow+1)%u32ItemNum);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_ALBUM_FocusGoBack(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_List_GetSelItem(ALBUM_LISTBOX, &u32CurrentRow);
    s32Ret |= HI_GV_List_GetItemNum(ALBUM_LISTBOX, &u32ItemNum);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    u32CurrentRow = u32CurrentRow ? ( u32CurrentRow - 1): (u32ItemNum - 1);
    s32Ret = HI_GV_List_SetSelItem(ALBUM_LISTBOX, u32CurrentRow);
    s32Ret |=HI_GV_Widget_Paint(ALBUM_LISTBOX, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_ALBUM_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
#ifdef CFG_TIME_STAMP_ON
        PDT_UI_COMM_SwitchPageTime_begin();
#endif
        s32Ret = PDT_UI_ALBUM_Exit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_ALBUM_FocusToNext(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
#ifdef CFG_TIME_STAMP_ON
        PDT_UI_COMM_SwitchPageTime_begin();
#endif
        UI_ALBUM_OnClickRow();
    }
    else if(PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_ALBUM_FocusGoBack(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else
    {
        MLOGE("KEY CODE %#lx not processed\n", (HI_UL)wParam);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_ALBUM_OnCellSelect(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_List_GetSelItem(ALBUM_LISTBOX, &u32CurrentRow);
    s32Ret |= HI_GV_List_GetItemNum(ALBUM_LISTBOX, &u32ItemNum);
    MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

    s32Ret = HI_GV_Widget_Active(ALBUM_LISTBOX);
    if(s32Ret)
    {
        MLOGE("Active %x\n", s32Ret);
    }

    UI_ALBUM_OnClickRow();

    return HIGV_PROC_GOON;
}




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

