#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

HI_S32 MULTI_LANGUAGE_BUTTON1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_EN);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON2_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_FR);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON3_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_ES);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON4_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_PT);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON5_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_DE);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON6_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_IT);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON7_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_ZH);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON8_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_ZH_TW);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON9_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_RU);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON10_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_JA);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON11_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_PO);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_BUTTON12_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Lan_Change(LAN_AR);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(MULTI_LANGUAGE_SCENE_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_SCENE_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_HANDLE hFontSet;

    if (HI_SUCCESS == HI_GV_FontSet_Create(&hFontSet))
    {
        (HI_VOID)HI_GV_FontSet_AddFont(simhei_font_text_20, "en;fr;es;pt;de;it;zh;zh_TW;ru;ja", hFontSet);
        (HI_VOID)HI_GV_FontSet_AddFont(notosans_font_text_20, "po", hFontSet);
        (HI_VOID)HI_GV_FontSet_AddFont(msuighur_font_text_30, "ar", hFontSet);
    }

    return HIGV_PROC_GOON;
}

HI_S32 MULTI_LANGUAGE_SCENE_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
