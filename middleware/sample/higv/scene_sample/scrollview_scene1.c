#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "higv_cextfile.h"
#include "higv_language.h"
#include "hi_gv_graphiccontext.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define INVALID_HANDLE             (0)

#define HIGV_CHECK(Function, Ret)  \
    if (Ret != HI_SUCCESS) {printf("[Func: %s, Line: %d] %s fail Ret(x):%x, Ret(d): %d\n", __FUNCTION__, __LINE__, Function, Ret, Ret); \
        return Ret;}

#define TIME_STAMP \
    do \
    { \
        struct timespec Nowtime = {0}; \
        clock_gettime(CLOCK_MONOTONIC, &Nowtime); \
        printf("[Func: %s, Line: %d] tv_sec: %ld, tv_usec: %ld\n\n", __FUNCTION__, __LINE__, Nowtime.tv_sec, (Nowtime.tv_nsec / 1000)); \
    } \
    while(0);

typedef struct
{
    HIGV_HANDLE lastFocus;
    HIGV_HANDLE currentFocus;
} HIGV_FOCUS_S;

typedef struct
{
    HI_S32 beforeAdjustX;
    HI_S32 beforeAdjustY;
    HI_S32 afterAdjustX;
    HI_S32 afterAdjustY;
} HIGV_ADJUST_S;

static HIGV_FOCUS_S  s_FocusInfo = {0};
static HIGV_HANDLE s_AnimHandle = 0;
static HIGV_ADJUST_S s_Adjust = {0};
static HI_BOOL  s_IsFling = HI_FALSE;
static HI_BOOL s_IsFirstEnter = HI_FALSE;

static HI_S32 Change_To_Focus(HIGV_HANDLE hWidget)
{
    HI_S32 Ret = HI_SUCCESS;

    if (INVALID_HANDLE == hWidget)
    {
        return HI_FAILURE;
    }

    Ret = HI_GV_Widget_SetFont(hWidget, simhei_font_text_40);
    HIGV_CHECK("HI_GV_Widget_SetFont", Ret);

    Ret = HI_GV_Widget_SetSkin(hWidget, HIGV_SKIN_NORMAL, button_focus_skin);
    HIGV_CHECK("HI_GV_Widget_SetSkin", Ret);

    Ret = HI_GV_Widget_Paint(hWidget, HI_NULL);
    HIGV_CHECK("HI_GV_Widget_Paint", Ret);

    return HI_SUCCESS;
}

static HI_S32 Change_To_Normal(HIGV_HANDLE hWidget)
{
    HI_S32 Ret = HI_SUCCESS;

    if (INVALID_HANDLE == hWidget)
    {
        return HI_FAILURE;
    }

    Ret = HI_GV_Widget_SetFont(hWidget, simhei_font_text_20);
    HIGV_CHECK("HI_GV_Widget_SetFont", Ret);

    Ret = HI_GV_Widget_SetSkin(hWidget, HIGV_SKIN_NORMAL, common_skin);
    HIGV_CHECK("HI_GV_Widget_SetSkin", Ret);

    Ret = HI_GV_Widget_Paint(hWidget, HI_NULL);
    HIGV_CHECK("HI_GV_Widget_Paint", Ret);

    return HI_SUCCESS;
}

static HI_S32 Handle_Move()
{
    HI_S32 Ret = HI_SUCCESS;
    HIGV_HANDLE hFocusWidget;

    Ret = HI_GV_Widget_GetWidgetByPos_TouchDevice(160, 120, &hFocusWidget);  //(160, 120) is the focus widget coordinate
    HIGV_CHECK("HI_GV_Widget_GetWidgetByPos_TouchDevice", Ret);

    //printf("[Func: %s, Line: %d] hFocusWidget: %u\n\n", __FUNCTION__, __LINE__, hFocusWidget);
    if (s_FocusInfo.currentFocus != hFocusWidget)
    {
        s_FocusInfo.lastFocus = s_FocusInfo.currentFocus;
        s_FocusInfo.currentFocus = hFocusWidget;
        (HI_VOID)Change_To_Focus(hFocusWidget);
        (HI_VOID)Change_To_Normal(s_FocusInfo.lastFocus);
    }

    return HI_SUCCESS;
}

void anim_update_handle(HIGV_LISTENER_S* listener, void* data)
{
    HI_GV_ScrollView_SetViewCoordinate(SCROLLVIEW_SCENE1_WIN_SCROLLVIEW,
                                       HI_GV_TweenAnimGetTweenValue(s_AnimHandle, 0), s_Adjust.beforeAdjustY);
}

static HI_S32 Anim_Init()
{
    HI_U32 interval = 0;

    interval = abs(s_Adjust.afterAdjustX - s_Adjust.beforeAdjustX) * 10;

    if (HI_TRUE == s_IsFirstEnter)
    {
        s_AnimHandle = HI_GV_TweenAnimCreate();
        HI_GV_TweenAnimSetDelay(s_AnimHandle, 0);
        HI_GV_TweenAnimSetDuration(s_AnimHandle, interval);
        HI_GV_TweenAnimAddTween(s_AnimHandle, HIGV_TWEEN_TRANSITION_QUART, HIGV_TWEEN_EASE_OUT,
                                s_Adjust.beforeAdjustX, s_Adjust.afterAdjustX);
        HI_GV_TweenAnimSetExecListener(s_AnimHandle, anim_update_handle);
        s_IsFirstEnter = HI_FALSE;
    }
    else
    {
        HI_GV_TweenAnimSetDuration(s_AnimHandle, interval);
        HI_GV_TweenAnimSetTweenRange(s_AnimHandle, 0, s_Adjust.beforeAdjustX, s_Adjust.afterAdjustX);
    }

    HI_GV_TweenAnimStart(s_AnimHandle);

    return HI_SUCCESS;
}

static HI_S32 Anim_DeInit()
{
    HI_GV_TweenAnimStop(s_AnimHandle);
    HI_GV_TweenAnimDestroy(s_AnimHandle);
    s_AnimHandle = 0;
    return HI_SUCCESS;
}

static HI_S32 Adjust_Widget()
{
    HI_S32 Ret = HI_SUCCESS;
    HI_S32 x, y;
    HI_S32 Delta;

    Ret = HI_GV_ScrollView_GetViewCoordinate(SCROLLVIEW_SCENE1_WIN_SCROLLVIEW, &x, &y);
    HIGV_CHECK("HI_GV_ScrollView_GetViewCoordinate", Ret);

    Delta = (10 + x) % 100; // 10 is value of scrollview left margin , 100 is width of  button widget.

    if (0 == Delta)
    {
        return HI_SUCCESS;
    }

    s_Adjust.beforeAdjustX = x;
    s_Adjust.beforeAdjustY = y;

    if (Delta <= 50)
    {
        x = x - Delta;
    }
    else
    {
        x = x + (100 - Delta);
    }

    s_Adjust.afterAdjustX = x;

    Anim_Init();

    return HI_SUCCESS;
}

static HI_S32 Fling_Adjust_Widget()
{
    HI_S32 Ret = HI_SUCCESS;
    HI_S32 x, y;
    HI_S32 Delta;

    Ret = HI_GV_ScrollView_GetViewCoordinate(SCROLLVIEW_SCENE1_WIN_SCROLLVIEW, &x, &y);
    HIGV_CHECK("HI_GV_ScrollView_GetViewCoordinate", Ret);

    Delta = (10 + x) % 100; // 10 is value of scrollview left margin , 100 is width of  button widget.

    if (0 == Delta)
    {
        return HI_SUCCESS;
    }

    if (Delta <= 50)
    {
        x = x - Delta;
    }
    else
    {
        x = x + (100 - Delta);
    }

    Ret = HI_GV_ScrollView_SetViewCoordinate(SCROLLVIEW_SCENE1_WIN_SCROLLVIEW, x, y);
    HIGV_CHECK("HI_GV_ScrollView_SetViewCoordinate", Ret);

    return HI_SUCCESS;
}


HI_S32 SCROLLVIEW_SCENE1_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(SCROLLVIEW_SCENE1_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    s_IsFirstEnter = HI_TRUE;
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_onrefresh(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)Handle_Move();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)Anim_DeInit();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_onfinishfling(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)Handle_Move();
    (HI_VOID)Fling_Adjust_Widget();

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_ongesturefling(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    s_IsFling = HI_TRUE;

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_onmouseout(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    Adjust_Widget();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    type = touchEvent.last.type;

    if (HIGV_TOUCH_START == type)
    {
        HI_GV_TweenAnimStop(s_AnimHandle);
    }

    (HI_VOID)Handle_Move();

    if (HIGV_TOUCH_END == type)
    {
        if (HI_FALSE == s_IsFling)
        {
            Adjust_Widget();
        }
        else
        {
            s_IsFling = HI_FALSE;
        }
    }

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
