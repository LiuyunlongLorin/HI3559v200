#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static int s_pos = 0;

HI_S32 OnProgressBarBackListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (0 == s_pos)
    {
        return HIGV_PROC_GOON;
    }

    s_pos = s_pos - 1;

    HI_GV_ProgressBar_SetPos(HORIZONTAL_PROGRESSBAR, s_pos);
    HI_GV_Widget_Paint(HORIZONTAL_PROGRESSBAR, 0);

    return HIGV_PROC_GOON;
}

HI_S32 OnProgressBarForwardListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (15 == s_pos)
    {
        return HIGV_PROC_GOON;
    }

    s_pos = s_pos + 1;

    HI_GV_ProgressBar_SetPos(HORIZONTAL_PROGRESSBAR, s_pos);
    HI_GV_Widget_Paint(HORIZONTAL_PROGRESSBAR, 0);

    return HIGV_PROC_GOON;
}

HI_S32 onProgressBarSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 PROGRESSBAR_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(PROGRESSBAR_WIN);

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
