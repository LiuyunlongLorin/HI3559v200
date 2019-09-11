#include <string.h>

#include "higv_cextfile.h"
#include "higv_language.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define ICON_HAVE_SUB_MENU_PATH "./res/pic/set_have_sub_menu.png"

#define PAGENUM (10)
#define CONTENT_SIZE  (32)

typedef struct hiUI_SET_ROW_S
{
    HI_CHAR szTitle[CONTENT_SIZE];   /* show text, left */
    HI_CHAR szContent[CONTENT_SIZE]; /* show text, right */
    HI_RESID resIcon;         /* has sub menu or others */
    HI_U32 handle;
} UI_SET_ROW_S;

static HI_RESID s_resIcon = 0;
static HIGV_HANDLE s_resHandle = 0;

static HI_S32 LoadSetList()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Index = 0;
    HI_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;
    UI_SET_ROW_S stSetRow;

    /** Get DDB (default data base) handle*/
    s32Ret = HI_GV_ADM_GetDDBHandle(ADM_SET_STRINGID2, &hDDB);

    if (HI_SUCCESS != s32Ret)
    {
        return -1;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < PAGENUM; u32Index++)
    {
        memset(&stDbRow, 0x0, sizeof(stDbRow));
        memset(&stSetRow, 0x0, sizeof(stSetRow));

        snprintf(stSetRow.szTitle, sizeof(HI_U32), "%d", u32Index);
        snprintf(stSetRow.szContent, sizeof(HI_U32), "%d", u32Index + 1);
        stSetRow.resIcon = s_resIcon;
        stSetRow.handle = s_resHandle;

        stDbRow.pData = &stSetRow;
        stDbRow.Size  = sizeof(UI_SET_ROW_S);

        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(MENUSET_LISTBOX);
    HI_GV_Widget_Active(MENUSET_LISTBOX);

    return HI_SUCCESS;
}

HI_S32 LISTBOX_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(LISTBOX_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 listboxOnCellSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n", __FUNCTION__, __LINE__);

    return HIGV_PROC_GOON;
}
HI_S32 listboxOnDateChangeListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}


HI_S32 listboxOnSelectListener(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 LISTBOX_WIN_onshow(HI_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = 0;

    s32Ret = HI_GV_Res_CreateID(ICON_HAVE_SUB_MENU_PATH, HIGV_RESTYPE_IMG, &s_resIcon);

    if (HI_SUCCESS != s32Ret)
    {
        printf("CreateID failed, s32Ret:%d\n", s32Ret);
    }

    s32Ret = HI_GV_Res_GetResInfo(s_resIcon, &s_resHandle);

    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_GV_Res_GetResInfo failed, s32Ret:%d\n", s32Ret);
    }

    LoadSetList();

    return HIGV_PROC_GOON;
}

HI_S32 LISTBOX_WIN_onhide(HI_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Res_ReleaseResInfo(s_resIcon);
    (HI_VOID)HI_GV_Res_DestroyID(s_resIcon);

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
