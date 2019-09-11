#include <string.h>
#include "higv_cextfile.h"
#include "higv_language.h"
#include "hi_gv_parser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define HIGV_CONTROL_PAGENUM   (15)

static HI_VOID MainWinList_AddData()
{
    HI_S32 s32Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;

    HI_U32 StrSet[HIGV_CONTROL_PAGENUM] = {STR_BUTTON_SCENE, STR_FLING_SCENE, STR_SCROLLBAR_SKIN_SCENE, STR_ASY_MSG_SCENE,
                                           STR_MULTI_LANGUAGE_SCENE, STR_HISTOGRAM_SCENE, STR_SCROLLVIEW_SCENE1, STR_SCROLLVIEW_SCENE2,
                                           STR_ANIM_INOUT, STR_ANIM_WIN_MOVE, STR_ANIM_TAB_INOUT, STR_WIN_CLOCK, STR_WIN_MENU_ROLL,
                                           STR_IMAGEEX_SCENE, STR_FOCUS_CHANGE
                                          };

    /** Get DDB (default data base) handle*/
    s32Ret = HI_GV_ADM_GetDDBHandle(ADM_SET_STRINGID, &hDDB);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Failed to add data, function:<%s>, retval %#x.\n", __FUNCTION__, s32Ret);
        return;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < HIGV_CONTROL_PAGENUM; u32Index++)
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(MAIN_WIN_LISTBOX1);

    return;
}

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
            (HI_VOID)Load_Show_Active(BUTTON_SCENE_WIN);
            break;
        }

        case 1:
        {
            (HI_VOID)Load_Show_Active(FLING_SCENE_WIN);
            break;
        }

        case 2:
        {
            (HI_VOID)Load_Show_Active(SCROLLBAR_SKIN_SCENE_WIN);
            break;
        }

        case 3:
        {
            (HI_VOID)Load_Show_Active(MSG_SCENE_WIN);
            break;
        }


        case 4:
        {
            (HI_VOID)Load_Show_Active(MULTI_LANGUAGE_SCENE_WIN);
            break;
        }

        case 5:
        {
            (HI_VOID)Load_Show_Active(HISTOGRAM_SCENE_WIN);
            break;
        }

        case 6:
        {
            (HI_VOID)Load_Show_Active(SCROLLVIEW_SCENE1_WIN);
            break;
        }

        case 7:
        {
            (HI_VOID)Load_Show_Active(SCROLLVIEW_SCENE2_WIN);
            break;
        }

        case 8:
        {
            (HI_VOID)Load_Show_Active(ANIM_IN_OUT);
            break;
        }

        case 9:
        {
            (HI_VOID)Load_Show_Active(ANIM_WIN_MOVE);
            break;
        }

        case 10:
        {
            (HI_VOID)Load_Show_Active(ANIM_TAB_INOUT);
            break;
        }

        case 11:
        {
            (HI_VOID)Load_Show_Active(WIN_CLOCK);
            break;
        }

        case 12:
        {
            HI_GV_ScrollGrid_RegisterWidget();
            (HI_VOID)Load_Show_Active(SCROLLGRID_WIN);
            HI_GV_ADM_Sync(ADM_IMAGE);
            break;
        }

        case 13:
        {
            (HI_VOID)Load_Show_Active(IMAGEEX_SCENE_WIN);
            break;
        }

        case 14:
        {
            (HI_VOID)Load_Show_Active(FOCUS_CHANGE_WIN);
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

HI_S32 MAIN_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    MainWinList_AddData();

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
