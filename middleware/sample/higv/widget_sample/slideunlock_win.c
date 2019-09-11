#include "higv_cextfile.h"
#include "hi_gv_slideunlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define INTERFACE_TEST  0

#define SLIDE_LOCK    "./res/pic/slideunlock/pop_slid_lock.png"
#define SLIDE_UNLOCK  "./res/pic/slideunlock/pop_slid_unlock.gif"
#define SLIDE_KEY     "./res/pic/slideunlock/pop_slid_key.png"

static HI_BOOL s_ShowImageExBG = HI_TRUE;
static HI_BOOL s_HideUnlockImg = HI_FALSE;

static HI_RESID s_hResID = 0;

HI_S32 OnKickBackListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (HI_FALSE == s_ShowImageExBG)
    {
        HI_GV_Widget_Show(IMAGEEX_BG);
        s_ShowImageExBG = HI_TRUE;
        HI_GV_ImageEx_SetImage(LOCK_IMG, SLIDE_LOCK);
    }

    return HIGV_PROC_GOON;
}

HI_S32 OnMoveListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (HI_TRUE == s_ShowImageExBG)
    {
        HI_GV_Widget_Hide(IMAGEEX_BG);
        s_ShowImageExBG = HI_FALSE;
        HI_GV_ImageEx_SetImage(LOCK_IMG, SLIDE_UNLOCK);
    }

    if (SLIDEUNLOCK_LENGTH == wParam)
    {
        HI_GV_ImageEx_SetImage(LOCK_IMG, SLIDE_KEY);
        s_HideUnlockImg = HI_TRUE;
    }

    if (HI_TRUE == s_HideUnlockImg)
    {
        if (wParam < SLIDEUNLOCK_LENGTH)
        {
            HI_GV_ImageEx_SetImage(LOCK_IMG, SLIDE_UNLOCK);
            s_HideUnlockImg = HI_FALSE;
        }
    }

    return HIGV_PROC_GOON;
}

HI_S32 OnShowListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_GV_Widget_Show(IMAGEEX_BG);
    return HIGV_PROC_GOON;
}

HI_S32 SLIDEUNLOCK_WIN_onrefresh(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#if INTERFACE_TEST
    HI_S32  s32Ret = HI_SUCCESS;

    (HI_VOID)HI_GV_SlideUnlock_GetStatus(HORIZONTAL_SLIDEUNLOCK, HI_NULL);

    (HI_VOID)HI_GV_Res_CreateID(SLIDE_KEY, HIGV_RESTYPE_IMG, &s_hResID);

    s32Ret = HI_GV_SlideUnlock_SetTraImage(HORIZONTAL_SLIDEUNLOCK, HIGV_SLIDEUNLOCK_NORMAL, s_hResID);
    s32Ret |= HI_GV_SlideUnlock_SetTraImage(HORIZONTAL_SLIDEUNLOCK, HIGV_SLIDEUNLOCK_TOUCH, s_hResID);
    s32Ret |= HI_GV_SlideUnlock_SetTraImage(HORIZONTAL_SLIDEUNLOCK, HIGV_SLIDEUNLOCK_DONE, s_hResID);

    if (s32Ret != HI_SUCCESS)
    {
        printf("[Func: %s, Line: %d] s32Ret: %d\n\n", __FUNCTION__, __LINE__, s32Ret);
    }

#endif
    return HIGV_PROC_GOON;
}

HI_S32 SLIDEUNLOCK_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#if INTERFACE_TEST
    (HI_VOID)HI_GV_Res_DestroyID(s_hResID);
#endif
}

HI_S32 OnUnlockListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_GV_Widget_Show(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Active(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Hide(SLIDEUNLOCK_WIN);

    if (s32Ret != HI_SUCCESS)
    {
        printf("OnUnlockListener failed! s32Ret: %x\n", s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 SLIDEUNLOCK_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(SLIDEUNLOCK_WIN);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
