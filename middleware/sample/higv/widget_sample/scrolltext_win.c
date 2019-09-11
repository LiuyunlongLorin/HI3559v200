#include "higv_cextfile.h"
#include "sample_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

HI_S32 SCROLLTEXT_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(SCROLLTEXT_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLTEXT_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    //HI_S32 Ret = HI_SUCCESS;

    //Ret = HI_GV_ScrollText_SetContent(SCROLLTEXT_WIN_SCROLLTEXT, INVALID_RESID, "hello hiberry");
    //HIGV_CHECK("HI_GV_ScrollText_SetContent", Ret);

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLTEXT_WIN_SCROLLTEXT_onetimeover(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
