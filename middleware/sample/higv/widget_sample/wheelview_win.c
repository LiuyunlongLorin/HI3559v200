#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "hi_type.h"

#include "hi_go.h"
#include "hi_gv.h"
#include "higv_cextfile.h"
#include "higv_language.h"
#include "hi_gv_parser.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define MONTH_NUM (12)
#define DAY_NUM   (31)
#define YEAR_NUM  (12)

HI_S32 WHEELVIEW_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(WHEELVIEW_WIN);

    return HIGV_PROC_GOON;
}

static HI_S32 UpdateMonthData()
{
    HI_S32 Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;
    HI_U32 StrSet[MONTH_NUM]
        = {Month_1, Month_2, Month_3, Month_4, Month_5, Month_6, Month_7, Month_8, Month_9, Month_10, Month_11, Month_12};

    /** Get DDB (default data base) handle*/
    Ret = HI_GV_ADM_GetDDBHandle(ADM_MONTH, &hDDB);

    if (HI_SUCCESS != Ret)
    {
        return -1;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < MONTH_NUM; u32Index++ )
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(WHEELVIEW_MONTH);

    return HI_SUCCESS;
}

static HI_S32 UpdateDayData()
{
    HI_S32 Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;
    HI_U32 StrSet[DAY_NUM] = {Day_1, Day_2, Day_3, Day_4, Day_5, Day_6, Day_7, Day_8, Day_9, Day_10,
                              Day_11, Day_12, Day_13, Day_14, Day_15, Day_16, Day_17, Day_18, Day_19, Day_20,
                              Day_21, Day_22, Day_23, Day_24, Day_25, Day_26, Day_27, Day_28, Day_29, Day_30,
                              Day_31
                             };

    /** Get DDB (default data base) handle*/
    Ret = HI_GV_ADM_GetDDBHandle(ADM_DAY, &hDDB);

    if (HI_SUCCESS != Ret)
    {
        return -1;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < DAY_NUM; u32Index++)
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(WHEELVIEW_DAY);

    return HI_SUCCESS;
}

static HI_S32 UpdateYearData()
{
    HI_S32 Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;
    HI_U32 StrSet[YEAR_NUM] = {Year_1, Year_2, Year_3, Year_4, Year_5, Year_6, Year_7, Year_8, Year_9, Year_10, Year_11, Year_12};

    /** Get DDB (default data base) handle*/
    Ret = HI_GV_ADM_GetDDBHandle(ADM_YEAR, &hDDB);

    if (HI_SUCCESS != Ret)
    {
        return -1;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < YEAR_NUM; u32Index++)
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(WHEELVIEW_YEAR);

    return HI_SUCCESS;
}


HI_S32 WheelViewOnShowListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    UpdateMonthData();
    UpdateDayData();
    UpdateYearData();

    HI_GV_WheelView_SetSelItem(WHEELVIEW_MONTH, 3);
    HI_GV_WheelView_SetSelItem(WHEELVIEW_DAY, 9);
    HI_GV_WheelView_SetSelItem(WHEELVIEW_YEAR, 10);

    return HIGV_PROC_GOON;
}

HI_S32 OnDayFocusSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 OnMonthFocusSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 OnYearFocusSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 OnMonthListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    type = touchEvent.last.type;

    if (HIGV_TOUCH_START == type)
    {
        HI_GV_WheelView_SetUpImage(WHEELVIEW_MONTH, up_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_MONTH, down_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_DAY, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_DAY, down_no_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_YEAR, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_YEAR, down_no_focus_cover_skin);

        HI_GV_Widget_Paint(WHEELVIEW_MONTH, 0);
        HI_GV_Widget_Paint(WHEELVIEW_DAY, 0);
        HI_GV_Widget_Paint(WHEELVIEW_YEAR, 0);
    }

    return HIGV_PROC_GOON;
}

HI_S32 OnDayListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    type = touchEvent.last.type;

    if (HIGV_TOUCH_START == type)
    {
        HI_GV_WheelView_SetUpImage(WHEELVIEW_MONTH, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_MONTH, down_no_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_DAY, up_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_DAY, down_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_YEAR, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_YEAR, down_no_focus_cover_skin);

        HI_GV_Widget_Paint(WHEELVIEW_MONTH, 0);
        HI_GV_Widget_Paint(WHEELVIEW_DAY, 0);
        HI_GV_Widget_Paint(WHEELVIEW_YEAR, 0);
    }

    return HIGV_PROC_GOON;
}

HI_S32 OnYearListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    type = touchEvent.last.type;

    if (HIGV_TOUCH_START == type)
    {
        HI_GV_WheelView_SetUpImage(WHEELVIEW_MONTH, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_MONTH, down_no_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_DAY, up_no_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_DAY, down_no_focus_cover_skin);
        HI_GV_WheelView_SetUpImage(WHEELVIEW_YEAR, up_focus_cover_skin);
        HI_GV_WheelView_SetDownImage(WHEELVIEW_YEAR, down_focus_cover_skin);

        HI_GV_Widget_Paint(WHEELVIEW_MONTH, 0);
        HI_GV_Widget_Paint(WHEELVIEW_DAY, 0);
        HI_GV_Widget_Paint(WHEELVIEW_YEAR, 0);
    }

    return HIGV_PROC_GOON;
}

HI_S32 OnMonthScrollListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    //HI_GV_Msg_SendAsync(WHEELVIEW_DAY, HIGV_MSG_GESTURE_SCROLL, wParam, lParam);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
