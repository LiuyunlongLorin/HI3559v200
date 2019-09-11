#include "higv_cextfile.h"
#include "hi_gv_parser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_U32 s_TimerID = 20;
static HI_U32 s_DelayTime = 300; //300ms
static HI_PARAM s_EventID = 10;

HI_S32 FOCUS_CHANGE_WIN2_BUTTON1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(FOCUS_CHANGE_WIN);
    (HI_VOID)HI_GV_Widget_Active(FOCUS_CHANGE_WIN);
    (HI_VOID)HI_GV_Widget_Hide(FOCUS_CHANGE_WIN2);

    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN2_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN2_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_BUTTON1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Active(hWidget);
    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_BUTTON2_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Active(hWidget);
    HI_GV_Msg_SendAsync(FOCUS_CHANGE_WIN, HIGV_MSG_EVENT, 0, s_EventID);

    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(FOCUS_CHANGE_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_onevent(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (s_EventID == lParam)
    {
        HI_GV_Timer_Start(hWidget, s_TimerID);
    }

    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_ontimer(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (s_TimerID == wParam)
    {
        (HI_VOID)HI_GV_Widget_Show(FOCUS_CHANGE_WIN2);
        (HI_VOID)HI_GV_Widget_Active(FOCUS_CHANGE_WIN2);
        (HI_VOID)HI_GV_Widget_Hide(FOCUS_CHANGE_WIN);
        HI_GV_Timer_Stop(hWidget, s_TimerID);
    }

    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_GV_Timer_Destroy(hWidget, s_TimerID);
    return HIGV_PROC_GOON;
}

HI_S32 FOCUS_CHANGE_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_GV_Timer_Create(hWidget, s_TimerID, s_DelayTime);
    (HI_VOID)HI_GV_Widget_Active(FOCUS_CHANGE_WIN_BUTTON1);
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
