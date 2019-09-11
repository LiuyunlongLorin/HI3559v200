/**
 * @file      ui_hdmi_filelist.c
 * @brief     ui filelist in HDMI scene. show thumbnails.
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


static HI_S32 PDT_HDMI_FILELIST_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if(pstEvent->s32Result)
            {
                s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_HOME, HI_TRUE);
                s32Ret |= PDT_UI_WINMNG_FinishWindow(HDMI_FILELIST);
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

static HI_S32 PDT_HDMI_FILELIST_Exit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(s_bSelectState)
    {
        s_bSelectState = HI_FALSE;
        s_bSelectAll = HI_FALSE;
        PDT_UI_COMM_FILELIST_ListClear();
        HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);
        return HI_SUCCESS;
    }

    s_bMediaReady  = HI_FALSE;

    s32Ret = PDT_UI_COMM_FILELIST_Exit(HI_PDT_WORKMODE_HDMI_PREVIEW, PDT_HDMI_FILELIST_OnReceiveMsgResult);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    s32Ret = PDT_UI_COMM_PLAYBACK_SyncParamVolume(HI_PDT_WORKMODE_HDMI_PLAYBACK);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_HDMI_FILELIST_OnEventProc(HI_EVENT_S * pstEvent, HIGV_HANDLE ADMHdl, HIGV_HANDLE BackBtnHdl)
{
    if(pstEvent->s32Result && HI_PDT_WORKMODE_HDMI_PLAYBACK == pstEvent->arg2)
    {
        s_bMediaReady = HI_TRUE;
        return HI_SUCCESS;
    }

    return PDT_UI_COMM_FILELIST_OnEventProc(pstEvent, DATAMODEL_HDMI_FILELIST, HDMI_FILELIST_BUTTON_BACK);
}

HI_S32 PDT_HDMI_FILELIST_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32CurGroupCount = 0;

    s_bSelectState = HI_FALSE;
    s_bSelectAll = HI_FALSE;

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32CurGroupCount);
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    if ((INVALID_GROUP_INDEX == u32CurGroupIndex) || (BACK_GROUP_INDEX == u32CurGroupIndex))
    {
        PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
    }
    else if(SELLECT_ALL_GROUP_INDEX == u32CurGroupIndex)
    {
        PDT_UI_COMM_FILELIST_SetCurGrpIdx(SELLECT_ALL_GROUP_INDEX);
        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_SELECTALL);
    }
    else if(DELETE_GROUP_INDEX == u32CurGroupIndex)
    {
        PDT_UI_COMM_FILELIST_SetCurGrpIdx(DELETE_GROUP_INDEX);
        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_DELETE);
    }
    else
    {
        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_GRID);
        s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32CurGroupIndex);
    }

    s32Ret |= HI_GV_ScrollGrid_EnableGesture(HDMI_FILELIST_GRID, HI_FALSE);

    s32Ret |= HI_GV_Widget_Paint(HDMI_FILELIST, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_HDMI_FILELIST_ParseTouchActionTopLable(HI_S32 s32StartX,
                                                 HI_S32 s32EndX)
{
    HI_U32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    MLOGD("u32GroupCount = 0x%x, u32CurGroupIndex = 0x%x\n", u32GroupCount, u32CurGroupIndex);

    if (0 == u32CurGroupIndex)
    {
        if (s32StartX > s32EndX) /* left */
        {
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(DELETE_GROUP_INDEX);
            s32Ret = HI_GV_Widget_Paint(HDMI_FILELIST,NULL);
            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(HDMI_FILELIST_GRID);
            s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_DELETE);
        }
        else                     /* right */
        {
            if(1 == u32GroupCount)/*for one file special case*/
            {
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
                s32Ret = HI_GV_Widget_Paint(HDMI_FILELIST,NULL);
                s32Ret = HI_GV_ScrollGrid_MoveToOrigin(HDMI_FILELIST_GRID);
                s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
            }
            else
            {
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32CurGroupIndex + 1);
                s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32CurGroupIndex + 1);
            }
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        return HI_SUCCESS;
    }
    else if(u32CurGroupIndex == ( u32GroupCount - 1))
    {
        if (s32StartX > s32EndX) /* left */
        {
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32CurGroupIndex - 1);
            s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32CurGroupIndex - 1);
        }
        else                     /* right */
        {
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(HDMI_FILELIST_GRID);
            s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        return HI_SUCCESS;
    }
    else
    {
        switch(u32CurGroupIndex)
        {
            case BACK_GROUP_INDEX:
            {
                if (s32StartX > s32EndX) /* left */
                {
                    if(0 == u32GroupCount) /* no file*/
                    {
                        PDT_UI_COMM_FILELIST_SetCurGrpIdx(DELETE_GROUP_INDEX);
                        s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_DELETE);
                    }
                    else
                    {
                        PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32GroupCount - 1);
                        s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32GroupCount - 1);
                        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_GRID);
                    }
                }
                else                    /* right */
                {
                    PDT_UI_COMM_FILELIST_SetCurGrpIdx(SELLECT_ALL_GROUP_INDEX);
                    s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_SELECTALL);
                }
                break;
            }
            case SELLECT_ALL_GROUP_INDEX:
            {
                if (s32StartX > s32EndX) /* left */
                {
                    PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
                    s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
                }
                else                    /* right */
                {
                    PDT_UI_COMM_FILELIST_SetCurGrpIdx(DELETE_GROUP_INDEX);
                    s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_DELETE);
                }
                break;
            }
            case DELETE_GROUP_INDEX:
            {
                if (s32StartX > s32EndX) /* left */
                {
                    PDT_UI_COMM_FILELIST_SetCurGrpIdx(SELLECT_ALL_GROUP_INDEX);
                    s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_SELECTALL);
                }
                else                    /* right */
                {
                    if(0 == u32GroupCount) /* no file*/
                    {
                        PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
                        s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
                    }
                    else
                    {
                        PDT_UI_COMM_FILELIST_SetCurGrpIdx(0);
                        s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, 0);
                        s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_GRID);
                    }
                }
                break;
            }
            case INVALID_GROUP_INDEX:
            {
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(BACK_GROUP_INDEX);
                break;
            }
            default:
                break;
        }
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    return HI_SUCCESS;
}

static HI_S32 PDT_HDMI_FILELIST_ParseTouchAction(HI_S32 s32StartX, HI_S32 s32StartY,
                                                 HI_S32 s32EndX, HI_S32 s32EndY)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    if(abs(s32StartX - s32EndX) > abs(s32StartY - s32EndY))
    {
        if((u32GroupCount == 0) || (u32CurGroupIndex >= (u32GroupCount -1 )) || (0 == u32CurGroupIndex))/*first or last grid or top lable*/
        {
            s32Ret = PDT_HDMI_FILELIST_ParseTouchActionTopLable(s32StartX,s32EndX);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            return HI_SUCCESS;
        }
        PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

        if (s32StartX > s32EndX)
        {
            MLOGD("fling left\n");
            u32CurGroupIndex = (u32CurGroupIndex + u32GroupCount - 1) % u32GroupCount;
        }
        else
        {
            MLOGD("fling right\n");
            u32CurGroupIndex = (u32CurGroupIndex + 1) % u32GroupCount;
        }

    }
    else if(abs(s32StartX - s32EndX) < abs(s32StartY - s32EndY))
    {
        if((u32GroupCount == 0) || (u32CurGroupIndex > (u32GroupCount -1 )))/*top lable, up or down do nothing*/
        {
            return HI_SUCCESS;
        }
        if(s32StartY > s32EndY)
        {
            u32CurGroupIndex = u32CurGroupIndex / 16 * 16 + 16;
            u32CurGroupIndex = u32CurGroupIndex >= u32GroupCount ? (u32GroupCount - 1) : u32CurGroupIndex;
        }
        else
        {
            u32CurGroupIndex = u32CurGroupIndex / 16 * 16;
            u32CurGroupIndex = u32CurGroupIndex <= 16 ? 0 : u32CurGroupIndex - 16;
        }
    }

    PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32CurGroupIndex);
    MLOGD("u32GroupCount = 0x%x, u32CurGroupIndex = 0x%x\n", u32GroupCount, u32CurGroupIndex);

    s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32CurGroupIndex);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_FILELIST_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32PointX, s32PointY;
    HIGV_TOUCH_EVENT_S stTouchEvent;
    HIGV_TOUCH_E type;
    static HI_S32 TOUCH_START_X = 0, TOUCH_START_Y = 0;

    memset(&stTouchEvent, 0x0, sizeof(stTouchEvent));
    memcpy(&stTouchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(stTouchEvent));

    s32PointX = stTouchEvent.last.x;
    s32PointY = stTouchEvent.last.y;
    type = stTouchEvent.last.type;


    switch (type)
    {
        case HIGV_TOUCH_START:
        {
            TOUCH_START_X = s32PointX;
            TOUCH_START_Y = s32PointY;
            break;
        }

        case HIGV_TOUCH_END:
        {
            PDT_HDMI_FILELIST_ParseTouchAction(TOUCH_START_X, TOUCH_START_Y, s32PointX, s32PointY);
            break;
        }

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_HDMI_FILELIST_OnGridSelect(HI_U32 u32GridIndex)
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
            s32Ret = PDT_UI_COMM_FILELIST_ListDelGrpIdx(u32GridIndex);  /** unselect it */
        }
        else
        {
            s32Ret = PDT_UI_COMM_FILELIST_ListAddGrpIdx(u32GridIndex);  /** select it */

        }
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        s32Ret = HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32GridIndex);
        s32Ret |= HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);  /** update filelist page */
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_PLAYBACK_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_FILELIST_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret;
    HI_S32 s32Idx;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    if (HDMI_FILELIST_BUTTON_BACK == WidgetHdl)
    {
        s32Ret = PDT_HDMI_FILELIST_Exit();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (HDMI_FILELIST_BUTTON_SELECTALL == WidgetHdl)
    {
        PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
        if (!s_bSelectAll)
        {
            s_bSelectState = HI_TRUE;
            s_bSelectAll = HI_TRUE;

            for(s32Idx=0; s32Idx< u32GroupCount; s32Idx++)
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

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    else if (HDMI_FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        if (s_bSelectState && (HI_FALSE == PDT_UI_COMM_FILELIST_ListIsNull()))
        {
            s_u32WinCallDialog = HDMI_FILELIST;

            s32Ret = PDT_UI_WINMNG_StartWindow(HDMI_DIALOG_WINDOW, HI_FALSE);
        }
        else
        {
            s_bSelectState = HI_TRUE;
            s32Ret = HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);
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
        s32Ret = PDT_HDMI_FILELIST_OnGridSelect(u32CurGroupIndex);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_FILELIST_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %lx, lParam = %lx\n", (HI_UL)wParam, (HI_UL)lParam);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    HI_U32 u32GroupCount = 0;

    if(PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_HDMI_FILELIST_Exit();
    }
    else if(PDT_UI_KEY_MENU == wParam && PDT_UI_COMM_SdIsReady())
    {
        PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);
        PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);
        if(HDMI_FILELIST_BUTTON_BACK == WidgetHdl)
        {
            PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
            s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_SELECTALL);
        }
        else if (HDMI_FILELIST_BUTTON_SELECTALL == WidgetHdl)
        {
            s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_DELETE);
        }
        else if(HDMI_FILELIST_BUTTON_DELETE == WidgetHdl)
        {
            if(u32GroupCount > 0)
            {
                s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_GRID);
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(0);
                s32Ret |= HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, 0);
            }
            else
            {
                s32Ret = HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
            }
        }
        else if(HDMI_FILELIST_GRID == WidgetHdl)
        {
            if((0 == u32GroupCount) || (u32CurGroupIndex >= u32GroupCount - 1))
            {
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(INVALID_GROUP_INDEX);
                s32Ret = HI_GV_ScrollGrid_MoveToOrigin(HDMI_FILELIST_GRID);
                s32Ret |= HI_GV_Widget_Active(HDMI_FILELIST_BUTTON_BACK);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
                return HIGV_PROC_GOON;
            }
            else
            {
                u32CurGroupIndex += 1;
                PDT_UI_COMM_FILELIST_SetCurGrpIdx(u32CurGroupIndex);
                s32Ret = HI_GV_ScrollGrid_SetSelCell(HDMI_FILELIST_GRID, u32CurGroupIndex);
            }
        }
    }
    else if (PDT_UI_KEY_ENTER == wParam)
    {
        s32Ret = PDT_HDMI_FILELIST_OnClick(WidgetHdl);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_VOID PDT_HDMI_FILELIST_DIALOG_Delete(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32GroupCount = 0;

    s_bSelectState = HI_FALSE;

    s32Ret = PDT_UI_COMM_FILELIST_ListDelNodeAndGrp();
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ListDelNodeAndGrp");

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_HDMI_FILELIST);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "ADM_Sync");
    PDT_UI_COMM_FILELIST_GetCurGrpCnt(&u32GroupCount);

    if(u32GroupCount <= 16)
    {
        HI_GV_ScrollGrid_MoveToOrigin(HDMI_FILELIST_GRID);
    }
}

HI_S32 PDT_HDMI_FILELIST_GRID_GetSelIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
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
        snprintf(pDstStr, u32Length, "%s", "./res/pic/hdmi_filelist_selecting.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/hdmi_filelist_selected.png");
    }

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_FILELIST_GRID_GetFocusIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                            const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    HI_U32 u32CurGroupIndex = INVALID_GROUP_INDEX;
    PDT_UI_COMM_FILELIST_GetCurGrpIdx(&u32CurGroupIndex);

    if (u32CurGroupIndex == u32Grid)
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/hdmi_filelist_focus_box.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "");
    }

    return HI_SUCCESS;
}

HI_S32 PDT_HDMI_FILELIST_GRID_GetTypeIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                           const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    if(HI_NULL == pSrcStr || HI_NULL == pDstStr)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }

    if (0 == strcasecmp(FILE_GROUP_TYPE_NORMMP4, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_record_normal.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_SLOWMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_record_slow.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LPSEMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_record_lapse.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LOOPMP4, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_record_loop.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_SINGJPG, pSrcStr)
        || 0 == strcasecmp(FILE_GROUP_TYPE_SINRJPG, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_photo_normal.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_DLAYJPG, pSrcStr)
        || 0 == strcasecmp(FILE_GROUP_TYPE_DLYRJPG, pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_photo_delay.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_LPSEJPG, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_multi_lapse.png");
    }
    else if ( 0 == strcasecmp(FILE_GROUP_TYPE_BURSTJPG, pSrcStr) )
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/mode_multi_burst.png");
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

