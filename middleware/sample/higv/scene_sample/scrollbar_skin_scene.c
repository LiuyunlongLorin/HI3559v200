#include <string.h>
#include "higv_language.h"
#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

#define PAGENUM (6)

HI_S32 SCROLLBAR_SKIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(SCROLLBAR_SKIN_SCENE_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLBAR_SKIN_SCENE_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;
    HI_U32 StrSet[PAGENUM] = {STR_SCENE_SAMPLE, STR_BUTTON_SCENE, STR_FLING_SCENE, STR_SCROLLBAR_SKIN_SCENE, STR_SCROLLBAR_SUSPEND_SCENE,
                              STR_MULTI_LANGUAGE_SCENE};

    /** Get DDB (default data base) handle*/
    Ret = HI_GV_ADM_GetDDBHandle(ADM_SET_STRINGID, &hDDB);

    if (HI_SUCCESS != Ret)
    {
        return -1;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < PAGENUM; u32Index++ )
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);
    HI_GV_Widget_SyncDB(SCROLLBAR_SKIN_LISTBOX);

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
