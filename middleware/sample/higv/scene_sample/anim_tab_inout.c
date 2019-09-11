#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*  __cplusplus  */

#define INVALID_HANDLE 0
#define MAX_COUNT 3

//all line move handle
HI_U32 qhLineAnim[MAX_COUNT][MAX_COUNT] = {{INVALID_HANDLE, trs_tab_122,trs_tab_123},
                                           {trs_tab_221, INVALID_HANDLE,trs_tab_223},
                                           {trs_tab_321, trs_tab_322,INVALID_HANDLE},
                                          };

HI_U32 qhTabAnim[2] = {trs_tab_left,trs_tab_right};

HI_U32 qhTab[MAX_COUNT] = {ANIM_TAB_INOUT_TAB1, ANIM_TAB_INOUT_TAB2, ANIM_TAB_INOUT_TAB3};

HIGV_HANDLE qhTabMove[MAX_COUNT][MAX_COUNT] = {INVALID_HANDLE};
HIGV_HANDLE qhLineMove[MAX_COUNT][MAX_COUNT] = {INVALID_HANDLE};

HIGV_HANDLE qCrtLineRuning = INVALID_HANDLE;
HIGV_HANDLE qCrtTabRuning = INVALID_HANDLE;

//current index of tab view
HI_U32 qCrtIndex = 0;

HI_S32 MoveTab2Tab(HI_U32 crt, HI_U32 move)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (INVALID_HANDLE == qhLineMove[crt][move])
    {
        HI_U32 isBig = crt < move;
        s32Ret = HI_GV_Anim_CreateInstance(qhLineAnim[crt][move],ANIM_TAB_INOUT_TAB1_LBL,&qhLineMove[crt][move]);

        s32Ret = HI_GV_Anim_CreateInstance(qhTabAnim[isBig],qhTab[move],&qhTabMove[crt][move]);

        printf("[%s][%d] OK HI_GV_Anim_CreateInstance s32Ret:%d,qBtnOk1=%d \n", __FILE__, __LINE__, s32Ret,qhLineMove[crt][move]);
    }

    (HI_VOID)HI_GV_Widget_Hide(qhTab[crt]);
    (HI_VOID)HI_GV_Widget_Show(qhTab[move]);

    HI_GV_Anim_Stop(ANIM_TAB_INOUT,qCrtLineRuning);
    HI_GV_Anim_Stop(ANIM_TAB_INOUT,qCrtTabRuning);

    s32Ret = HI_GV_Anim_Start(ANIM_TAB_INOUT,qhLineMove[crt][move]);
    qCrtLineRuning = qhLineMove[crt][move];
    printf("[%s][%d] ok HI_GV_Anim_Start s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);

    s32Ret = HI_GV_Anim_Start(ANIM_TAB_INOUT,qhTabMove[crt][move]);
    qCrtTabRuning = qhTabMove[crt][move];
    printf("[%s][%d] ok HI_GV_Anim_Start qhTabMove s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    return HI_SUCCESS;
}

HI_S32 ANIM_TAB_INOUT_HIDE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_TAB_INOUT);
    return HIGV_PROC_STOP;
}

HI_S32 ANIM_TAB_INOUT_TAB1_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    if (0 != qCrtIndex){
        MoveTab2Tab(qCrtIndex,0);
        qCrtIndex = 0;
    }
    return HIGV_PROC_STOP;
}

HI_S32 ANIM_TAB_INOUT_TAB2_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    if (1 != qCrtIndex){
        MoveTab2Tab(qCrtIndex,1);
        qCrtIndex = 1;
    }
    return HIGV_PROC_STOP;
}

HI_S32 ANIM_TAB_INOUT_TAB3_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    if (2 != qCrtIndex){
        MoveTab2Tab(qCrtIndex,2);
        qCrtIndex = 2;
    }
    return HIGV_PROC_STOP;
}

HI_S32 ANIM_TAB_INOUT_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Hide(ANIM_TAB_INOUT_TAB2);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_TAB_INOUT_TAB3);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_TAB_INOUT_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    HI_U32 i = 0,j = 0;
    for (i = 0; i < MAX_COUNT; i++)
    {
        for (j = 0; j < MAX_COUNT; j++)
        {
            if (INVALID_HANDLE != qhLineMove[i][j])
            {
                HI_GV_Anim_DestroyInstance(ANIM_TAB_INOUT, qhLineMove[i][j]);
                qhLineMove[i][j] = INVALID_HANDLE;
            }

            if (INVALID_HANDLE != qhLineMove[i][j])
            {
                HI_GV_Anim_DestroyInstance(ANIM_TAB_INOUT, qhLineMove[i][j]);
                qhLineMove[i][j] = INVALID_HANDLE;
            }
        }
    }
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

