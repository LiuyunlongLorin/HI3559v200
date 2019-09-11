#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HIGV_HANDLE s_gAnimHandle = 0;

void anim_update_handle(HIGV_LISTENER_S* listener, void* data)
{
    HI_GV_Widget_Move(ANIM_BUTTON, HI_GV_TweenAnimGetTweenValue(s_gAnimHandle, 0), 80);
}

void anim_start_handle(HIGV_LISTENER_S* listener, void* data)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
}

void anim_end_handle(HIGV_LISTENER_S* listener, void* data)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
}

static HI_S32 Anim_Init()
{
    s_gAnimHandle = HI_GV_TweenAnimCreate();
    HI_GV_TweenAnimSetDelay(s_gAnimHandle, 0);
    HI_GV_TweenAnimSetDuration(s_gAnimHandle, 2000);
    HI_GV_TweenAnimSetLoops(s_gAnimHandle, 5);
    HI_GV_TweenAnimAddTween(s_gAnimHandle, HIGV_TWEEN_TRANSITION_LINEAR, HIGV_TWEEN_EASE_IN_OUT, 0, 320);

    HI_GV_TweenAnimSetStartedListener(s_gAnimHandle, anim_start_handle);
    HI_GV_TweenAnimSetExecListener(s_gAnimHandle, anim_update_handle);
    HI_GV_TweenAnimSetFinishedListener(s_gAnimHandle, anim_end_handle);

    HI_GV_TweenAnimStart(s_gAnimHandle);

    return HI_SUCCESS;
}

static HI_S32 Anim_DeInit()
{
    HI_GV_TweenAnimStop(s_gAnimHandle);
    HI_GV_TweenAnimDestroy(s_gAnimHandle);
    return HI_SUCCESS;
}

HI_S32 ANIM_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    Anim_DeInit();
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 AnimOnShowListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    Anim_Init();

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
