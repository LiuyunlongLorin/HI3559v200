/**
 * @file      ui_filelist_page.c
 * @brief     ui filelist page. show thumbnails.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "ui_common.h"
#include "ui_common_filelist.h"
#include "ui_common_playback.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


static HI_BOOL s_bSelectState = HI_FALSE;
static HI_BOOL s_bSelectAll = HI_FALSE;
static HI_BOOL s_bMediaReady = HI_FALSE;
extern HIGV_HANDLE s_u32WinCallDialog;


static HI_S32 PDT_UI_FILELIST_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if(pstEvent->s32Result)
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(HOME_PAGE, HI_TRUE);
                s32Ret |= PDT_UI_WINMNG_FinishWindow(FILELIST_PAGE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            else
            {
                MLOGE("Quit filelist failed\n");
            }
            break;

        default:
            MLOGD("Event ID %u not need to process\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_FILELIST_Exit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_WORKMODE_E enWorkMode = HI_PDT_WORKMODE_BUTT;

    if(s_bSelectState)
    {
        s_bSelectState = HI_FALSE;
        s_bSelectAll = HI_FALSE;
        PDT_UI_COMM_FILELIST_ListClear();
        HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        return HI_SUCCESS;
    }

    s_bMediaReady  = HI_FALSE;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_COMM_FILELIST_Exit(enWorkMode, PDT_UI_FILELIST_OnReceiveMsgResult);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_PLAYBACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl)
{
    if(pstEvent->s32Result && HI_PDT_WORKMODE_PLAYBACK == pstEvent->arg2)
    {
        s_bMediaReady = HI_TRUE;
        return HI_SUCCESS;
    }

    return PDT_UI_COMM_FILELIST_OnEventProc(pstEvent, DATAMODEL_FILELIST, FILELIST_BUTTON_BACK);
}


HI_S32 PDT_UI_FILELIST_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32CurGroupCount = 0;

    s_bSelectState = HI_FALSE;
    s_bSelectAll = HI_FALSE;

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32CurGroupCount);
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    if (u32CurGroupCount > 0)
    {
        s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }


    if (INVALID_GROUP_INDEX == u32CurGroupIndex || 0 == u32CurGroupCount)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
    }
    else
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
        s32Ret |= HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, u32CurGroupIndex);

    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_FILELIST_OnGridSelect(HI_U32 u32GridIndex)
{
    MLOGD("GridIndex = %u\n", u32GridIndex);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32GroupCount = 0;

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
    PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32GridIndex);

    if(0 == u32GroupCount || u32GridIndex >= u32GroupCount)
    {
        return HI_SUCCESS;
    }

    if (s_bSelectState)
    {
        s32Ret = PDT_UI_COMM_FILELIST_ListFindGrpIdx(u32GridIndex);

        if (HI_SUCCESS == s32Ret)
        {
            s32Ret = PDT_UI_COMM_FILELIST_ListDelGrpIdx(u32GridIndex); /** unselect it */
        }
        else
        {
            s32Ret = PDT_UI_COMM_FILELIST_ListAddGrpIdx(u32GridIndex); /** select it */
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, u32GridIndex);
        s32Ret |= HI_GV_ADM_Sync(DATAMODEL_FILELIST);  /** update filelist page */
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(PLAYBACK_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_FILELIST_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);

    if (FILELIST_BUTTON_BACK == WidgetHdl)
    {
        s32Ret = PDT_UI_FILELIST_Exit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (FILELIST_BUTTON_SELECTALL == WidgetHdl)
    {
        if (!s_bSelectAll)
        {
            s_bSelectState = HI_TRUE;
            s_bSelectAll = HI_TRUE;

            for(s32Idx = 0; s32Idx < u32GroupCount; s32Idx++)
            {
                s32Ret = PDT_UI_COMM_FILELIST_ListFindGrpIdx(s32Idx);
                if(HI_SUCCESS != s32Ret)
                {
                    s32Ret = PDT_UI_COMM_FILELIST_ListAddGrpIdx(s32Idx);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }
        else
        {
            s_bSelectState = HI_FALSE;
            s_bSelectAll = HI_FALSE;
            PDT_UI_COMM_FILELIST_ListClear();
        }

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        if (s_bSelectState && (HI_FALSE == PDT_UI_COMM_FILELIST_ListIsNull()))
        {
            s_u32WinCallDialog = FILELIST_PAGE;

            s32Ret = HI_GV_Widget_SetTextByID(DIALOG_WINDOW_LABEL_ASK, STRING_DIALOG_Delete_ASK);
            if(s32Ret)
            {
                MLOGE("SetTextByID failed. %x\n", s32Ret);
            }

            HI_GV_Widget_Active(DIALOG_WINDOW_BUTTON_CANCEL);
            s32Ret = PDT_UI_WINMNG_StartWindow(DIALOG_WINDOW, HI_FALSE);
        }
        else
        {
            s_bSelectState = HI_TRUE;

            s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else
    {
        if (HI_FALSE == s_bMediaReady)
        {
            return HI_SUCCESS;
        }
        PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);
        s32Ret = PDT_UI_FILELIST_OnGridSelect(u32CurGroupIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_FILELIST_Exit();
    }
    else if (PDT_UI_KEY_MENU == wParam && PDT_UI_COMM_SdIsReady())
    {
        PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
        PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);
        if (FILELIST_BUTTON_BACK == WidgetHdl)
        {
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECTALL);
        }
        else if (FILELIST_BUTTON_SELECTALL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DELETE);
        }
        else if (FILELIST_BUTTON_DELETE == WidgetHdl)
        {
            if(u32GroupCount > 0)
            {
                s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(0);
                s32Ret |= HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, 0);
            }
            else
            {
                s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
            }
        }
        else
        {
            if(0 == u32GroupCount || u32CurGroupIndex >= u32GroupCount - 1)
            {
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
                s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
                s32Ret |= HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
                return HIGV_PROC_GOON;
            }
            else
            {
                u32CurGroupIndex += 1;
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32CurGroupIndex);
                s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, u32CurGroupIndex);
            }
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_UI_FILELIST_OnClick(WidgetHdl);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(FILELIST_BUTTON_BACK == WidgetHdl || PDT_UI_COMM_SdIsReady())
    {
        HI_GV_Widget_Active(WidgetHdl);

        s32Ret = PDT_UI_FILELIST_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_VOID PDT_UI_FILELIST_DIALOG_Delete(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32GroupCount = 0;

    s_bSelectState = HI_FALSE;

    s32Ret = PDT_UI_COMM_FILELIST_ListDelNodeAndGrp();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ListDelNodeAndGrp");

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ADM_Sync");

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
    if(u32GroupCount <= 4)
    {
        HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
    }
}

HI_S32 PDT_UI_FILELIST_GRID_OnCellColSelect(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM u32ChildIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_UI_FILELIST_OnGridSelect(u32GridIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_GRID_GetTypeIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                        const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    if(HI_NULL == pSrcStr || HI_NULL == pDstStr)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }

    if (0 == strcasecmp(FILE_GROUP_TYPE_NORMMP4, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_record_normal.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_SLOWMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_record_slow.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LPSEMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_record_lapse.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LOOPMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_record_loop.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_SINGJPG, pSrcStr)
        || 0 == strcasecmp(FILE_GROUP_TYPE_SINRJPG, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_photo_normal.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_DLAYJPG, pSrcStr)
        || 0 == strcasecmp(FILE_GROUP_TYPE_DLYRJPG, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_photo_delay.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LPSEJPG, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_multi_lapse.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_BURSTJPG, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/home_multi_burst.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "");
    }

    return HI_SUCCESS;
}

/**
 * @brief  given data from row value,  output  SELECT state image's path.
 * @param u32GridChild  grid's child's index.
 * @param u32Grid  grid index
 * @param pSrcStr   in XML file, coldbindex="3", so we get a row's 3rd column value.
 * @param pDstStr  output TYPE image's path
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_FILELIST_GRID_GetSelIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                       const HI_CHAR* pSrcStr, HI_CHAR* pDstStr,HI_U32 u32Length)
{
    if(HI_NULL == pSrcStr || HI_NULL == pDstStr)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }
    if (HI_FALSE == s_bSelectState)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        return HI_SUCCESS;
    }

    if (0 == strncmp("0", pSrcStr, HI_APPCOMM_MAX_PATH_LEN))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_selecting.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_selected.png");
    }

    return HI_SUCCESS;
}

/**
 * @brief  given data from row value,  output  FOCUS state image's path.
 * @param u32GridChild  grid's child's index.
 * @param u32Grid  grid index
 * @param pSrcStr  we don't use it.
 * @param pDstStr  output FOCUS box image's path
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_FILELIST_GRID_GetFocusIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                        const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    if (u32CurGroupIndex == u32Grid)
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_focus_box.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "");
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

