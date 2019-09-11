#include <stdio.h>
#include <string.h>
#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

extern HIGV_HANDLE s_LayerId;
HI_S32 TRACKBAR_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(TRACKBAR_WIN);

    return HIGV_PROC_GOON;
}
HI_S32 HORIZONTAL_TRACKBAR_onvaluechange(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 HORIZONTAL_TRACKBAR_onmouseout(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 HORIZONTAL_TRACKBAR_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    type = touchEvent.last.type;

    switch (type)
    {
        case HIGV_TOUCH_END:
        {
            break;
        }

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 VERTICAL_TRACKBAR_onvaluechange(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 TRACKBAR_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;

    Ret = HI_GV_WM_BindTouchMsg(s_LayerId, HI_TRUE);

    if (Ret != HI_SUCCESS)
    {
    }

    return HIGV_PROC_GOON;
}

HI_S32 TRACKBAR_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;
    HI_U32 IsBind = 0;

    Ret = HI_GV_WM_GetBindTouchStatus(s_LayerId, &IsBind);

    if (HI_FALSE == (HI_BOOL)IsBind)
    {
        return HIGV_PROC_GOON;
    }

    Ret = HI_GV_WM_BindTouchMsg(s_LayerId, HI_FALSE);

    if (Ret != HI_SUCCESS)
    {
        printf("[Func: %s, Line: %d] HI_GV_WM_BindTouchMsg Fail\n\n", __FUNCTION__, __LINE__);
    }

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
