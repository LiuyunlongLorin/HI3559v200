#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

HI_S32 BUTTON_WIN_BUTTON1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Show(BUTTON_WIN_BUTTON2);
    s32Ret |= HI_GV_Widget_Active(BUTTON_WIN_BUTTON2);
    s32Ret |= HI_GV_Widget_Hide(BUTTON_WIN_BUTTON1);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] ERROR s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_BUTTON2_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Show(BUTTON_WIN_BUTTON1);
    s32Ret |= HI_GV_Widget_Active(BUTTON_WIN_BUTTON1);
    s32Ret |= HI_GV_Widget_Hide(BUTTON_WIN_BUTTON2);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] ERROR s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret |= HI_GV_Widget_Show(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Active(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Hide(BUTTON_WIN);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] ERROR s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_SWITCH_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_TOGGLE_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_radio1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_radio2_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_radio3_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_OnShowListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_OnHideListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 BUTTON_WIN_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
