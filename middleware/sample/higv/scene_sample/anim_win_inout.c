#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*  __cplusplus  */
#define INVALID_HANDLE 0

static HIGV_HANDLE ghGroupOut = INVALID_HANDLE;
static HIGV_HANDLE ghGroupIn  = INVALID_HANDLE;

HI_S32 StartAnimWin(HIGV_HANDLE hAnim,HIGV_HANDLE hWidget,HIGV_HANDLE & hAnimObj)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (INVALID_HANDLE == hAnimObj)
    {
        s32Ret = HI_GV_Anim_CreateInstance(hAnim,hWidget,&hAnimObj);
    }

    s32Ret = HI_GV_Anim_Start(ANIM_WIN_MOVE,hAnimObj);
    return HI_SUCCESS;
}

HI_S32 ANIM_WIN_MOVE_EXIT_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_WIN_MOVE);
    return HIGV_PROC_GOON;
}
HI_S32 ANIM_WIN_MOVE_BTN_1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    StartAnimWin(trs_win_out,ANIM_WIN_MOVE_GRP1,ghGroupOut);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_WIN_MOVE);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(ANIM_WIN_MOVE_GRP1);
    StartAnimWin(trs_win_in,ANIM_WIN_MOVE_GRP1,ghGroupIn);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Hide(ANIM_WIN_MOVE_GRP1);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_GRP1_onanimstart(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    //printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_GRP1_onanimupdate(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    //printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_WIN_MOVE_GRP1_onanimstop(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    //printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

