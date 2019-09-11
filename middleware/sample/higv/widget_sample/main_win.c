#include "higv_cextfile.h"
#include "hi_gv_parser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static HI_S32 Load_Show_Active(HI_U32 view)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_PARSER_LoadViewById(view);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] HI_GV_PARSER_LoadViewById failed! s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_GV_Widget_Show(view);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] HI_GV_Widget_Show failed! s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_GV_Widget_Active(view);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] HI_GV_Widget_Active failed! s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 Main_Win_Listbox1_OnCellSelect(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (wParam)
    {
        case 0:
        {
            (HI_VOID)Load_Show_Active(LISTBOX_WIN);
            break;
        }

        case 1:
        {
            (HI_VOID)Load_Show_Active(BUTTON_WIN);
            break;
        }

        case 2:
        {
            (HI_VOID)Load_Show_Active(SCROLLVIEW_WIN);
            break;
        }

        case 3:
        {
            (HI_VOID)HI_GV_ScrollGrid_RegisterWidget();
            (HI_VOID)Load_Show_Active(SCROLLGRID_WIN);
            HI_GV_ADM_Sync(ADM_IMAGE);
            break;
        }


        case 4:
        {
            (HI_VOID)Load_Show_Active(TRACKBAR_WIN);
            break;
        }

        case 5:
        {
            (HI_VOID)Load_Show_Active(WHEELVIEW_WIN);
            break;
        }

        case 6:
        {
            (HI_VOID)Load_Show_Active(SLIDEUNLOCK_WIN);
            break;
        }

        case 7:
        {
            (HI_VOID)Load_Show_Active(IMAGE_WIN);
            break;
        }

        case 8:
        {
            (HI_VOID)Load_Show_Active(PROGRESSBAR_WIN);
            break;
        }

        case 9:
        {
            (HI_VOID)Load_Show_Active(ANIM_WIN);
            break;
        }

        case 10:
        {
            (HI_VOID)Load_Show_Active(SCROLLTEXT_WIN);
            break;
        }

        default:
            break;
    }

    s32Ret = HI_GV_Widget_Hide(MAIN_WIN);

    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] HI_GV_Widget_Hide failed! s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 Main_Win_Listbox1_OnSelect(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
