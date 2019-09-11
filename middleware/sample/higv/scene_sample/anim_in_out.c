#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*  __cplusplus  */

#define INVALID_HANDLE 0

//*四个按钮动画都显示，调试使用;去掉此宏定义则只显示一个按钮动画*/
#define SHOW_ALL_BUTTON_ANIM

//*按钮飞出动画句柄*/
HIGV_HANDLE qhTopOut = INVALID_HANDLE;
HIGV_HANDLE qhLeftOut = INVALID_HANDLE;
HIGV_HANDLE qhRightOut = INVALID_HANDLE;
HIGV_HANDLE qhBottomOut = INVALID_HANDLE;

//*按钮飞入动画句柄*/
HIGV_HANDLE qhTopIn = INVALID_HANDLE;
HIGV_HANDLE qhLeftIn = INVALID_HANDLE;
HIGV_HANDLE qhRightIn = INVALID_HANDLE;
HIGV_HANDLE qhBottomIn = INVALID_HANDLE;

//*当前按钮是否隐藏,根据此标识切换启动动画类型*/
HI_BOOL gBtnIsHide = HI_FALSE;
//*窗口被触屏次数，模拟真实场景，屏蔽前两次触屏事件再启动动画;*/
HI_S32 gTouchWndCount = 0;

HI_S32 StartAnim(HIGV_HANDLE hAnim,HIGV_HANDLE hWidget,HIGV_HANDLE & hAnimObj)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (INVALID_HANDLE == hAnimObj)
    {
        s32Ret = HI_GV_Anim_CreateInstance(hAnim,hWidget,&hAnimObj);
        printf("[%s][%d] OK HI_GV_Anim_CreateInstance s32Ret:%d,qBtnOk1=%d \n", __FILE__, __LINE__, s32Ret,hAnimObj);
    }

    s32Ret = HI_GV_Anim_Start(ANIM_IN_OUT,hAnimObj);

    return HI_SUCCESS;
}
HI_S32 ANIM_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_EVENT_S * touchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    if (HIGV_TOUCH_END != touchEvent->last.type)
    {
        return s32Ret;
    }
    gTouchWndCount++;
    if (gTouchWndCount < 2)
    {
        return s32Ret;
    }

    if (INVALID_HANDLE != qhTopOut)
    {
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhTopOut);
#ifdef SHOW_ALL_BUTTON_ANIM
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhLeftOut);
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhRightOut);
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhBottomOut);
#endif
    }

    if (INVALID_HANDLE != qhTopIn)
    {
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhTopIn);
#ifdef SHOW_ALL_BUTTON_ANIM
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhLeftIn);
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhRightIn);
        HI_GV_Anim_Stop(ANIM_IN_OUT,qhBottomIn);
#endif

    }

    if(HI_TRUE == gBtnIsHide)
    {
        StartAnim(trs_inout_top_out,ANIM_GRP_TOP,qhTopOut);
#ifdef SHOW_ALL_BUTTON_ANIM
        StartAnim(trs_inout_right_out,ANIM_GRP_RIGHT,qhRightOut);
        StartAnim(trs_inout_left_out,ANIM_GRP_LEFT,qhLeftOut);
        StartAnim(trs_inout_bottom_out,ANIM_GRP_BOTTON,qhBottomOut);
#endif
        gBtnIsHide = HI_FALSE;
    }
    else
    {
        StartAnim(trs_inout_top_in,ANIM_GRP_TOP,qhTopIn);
#ifdef SHOW_ALL_BUTTON_ANIM
        StartAnim(trs_inout_right_in,ANIM_GRP_RIGHT,qhRightIn);
        StartAnim(trs_inout_left_in,ANIM_GRP_LEFT,qhLeftIn);
        StartAnim(trs_inout_bottom_in,ANIM_GRP_BOTTON,qhBottomIn);
#endif
        //StartAnim(trs_inout_top_in,ANIM_GRP_TOP,qhTopIn);
        gBtnIsHide = HI_TRUE;
    }
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_IN_OUT_HIDE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_IN_OUT);
    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
