#include "higv_cextfile.h"
#include "sample_utils.h"
#include "hi_gv_input.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_U32 s_TimerId = 0;

static HI_S32 KeyTestBegin()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Create(SCROLLVIEW_WIN, s_TimerId, 1000);
    HIGV_CHECK("HI_GV_Timer_Create", s32Ret);

    s32Ret = HI_GV_Timer_Start(SCROLLVIEW_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Start", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 KeyTestEnd()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(SCROLLVIEW_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Stop", s32Ret);

    s32Ret = HI_GV_Timer_Destroy(SCROLLVIEW_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Destory", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 KeyTestSend()
{
    HI_S32 s32Ret = HI_SUCCESS;

    HIGV_INPUTEVENT_S inputevent;
    inputevent.msg = HIGV_MSG_KEYDOWN;
    inputevent.value = 0;
    inputevent.dx = 0;
    inputevent.dy = 0;
    inputevent.step = 0;
    s32Ret = HI_GV_SendInputEvent(&inputevent);
    HIGV_CHECK("HI_GV_SendInputEvent", s32Ret);

    s32Ret = HI_GV_Msg_SendAsync(ChildButton_02, HIGV_MSG_KEYDOWN, 0, 0);
    HIGV_CHECK("HI_GV_Msg_SendAsync", s32Ret);

    return HI_SUCCESS;
}

HI_S32 SCROLLVIEW_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(SCROLLVIEW_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_SCROLLVIEW1_onviewmove(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_SCROLLVIEW1_onfinishfling(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 ChildButton_01_onkeydown(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 ChildButton_02_onkeydown(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)KeyTestEnd();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_onrefresh(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)KeyTestBegin();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLVIEW_WIN_ontimer(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)KeyTestSend();

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
