#include <string.h>
#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

//#define WIDGET_BIND_TOUCH

extern HIGV_HANDLE s_LayerId;

static HI_BOOL isTouchButton = HI_FALSE;

HI_S32 BUTTON_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#ifndef WIDGET_BIND_TOUCH
    HI_S32 point_x, point_y;
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;
    HIGV_HANDLE hHandle;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    point_x = touchEvent.last.x;
    point_y = touchEvent.last.y;

    type = touchEvent.last.type;


    switch (type)
    {
        case HIGV_TOUCH_START:
        {

            (HI_VOID)HI_GV_Widget_GetWidgetByPos_TouchDevice(point_x, point_y, &hHandle);

            if (BUTTON_WIN_BUTTON1 == hHandle)
            {
                isTouchButton = HI_TRUE;
            }

            break;
        }

        case HIGV_TOUCH_END:
        {

            if (HI_TRUE == isTouchButton)
            {
                isTouchButton = HI_FALSE;
                printf("[Func: %s, Line: %d] button action\n\n", __FUNCTION__, __LINE__);
            }

            break;
        }

        default:
            break;
    }

#endif
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_BUTTON1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#ifdef WIDGET_BIND_TOUCH
    HI_S32 point_x, point_y;
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;
    HIGV_HANDLE hHandle;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    point_x = touchEvent.last.x;
    point_y = touchEvent.last.y;

    type = touchEvent.last.type;


    switch (type)
    {
        case HIGV_TOUCH_START:
        {
            break;
        }

        case HIGV_TOUCH_END:
        {
            printf("[Func: %s, Line: %d] button action\n\n", __FUNCTION__, __LINE__);
            break;
        }

        return HIGV_PROC_GOON;
    }

#endif
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(BUTTON_SCENE_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#ifdef WIDGET_BIND_TOUCH
    HI_S32 Ret = HI_SUCCESS;

    Ret = HI_GV_WM_BindTouchMsg(s_LayerId, HI_TRUE);

    if (Ret != HI_SUCCESS)
    {
        printf("[Func: %s, Line: %d] HI_GV_WM_BindTouchMsg Fail\n\n", __FUNCTION__, __LINE__);
    }

#endif
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#ifdef WIDGET_BIND_TOUCH
    HI_S32 Ret = HI_SUCCESS;
    HI_U32 IsBind = 0;

    Ret = HI_GV_WM_GetBindTouchStatus(s_LayerId, &IsBind);

    if (HI_FALSE == (HI_BOOL)IsBind)
    {
        return HIGV_PROC_GOON;
    }

    Ret = HI_GV_WM_BindTouchMsg(s_LayerId, HI_FALSE);

    if (Ret != HI_SUCCESS)
    {
        printf("[Func: %s, Line: %d] HI_GV_WM_BindTouchMsg Fail\n\n", __FUNCTION__, __LINE__);
    }

#endif
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
