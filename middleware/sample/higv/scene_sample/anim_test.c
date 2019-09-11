#include "higv_cextfile.h"
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*  __cplusplus  */

#define MAX_INSTANCE_NUM 1000

extern HIGV_HANDLE g_hApp;
int g_AnimTestTouchCnt = 0;
HI_S32 ANIM_TEST_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(ANIM_TEST);
    return HIGV_PROC_GOON;
}

HI_S32 ANIM_TEST_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    /*
    if ( g_AnimTestTouchCnt > 3)
    {
        HIGV_HANDLE hAnimObj = 0;

        HI_S32 s32Ret = 0;
        HIGV_HANDLE hWidget = SHOW_IMAGE;
        HIGV_HANDLE hAnimInfo = trs_anim_test;
        HIGV_HANDLE hWnd = ANIM_TEST;
        HIGV_HANDLE ahAnimObj[MAX_INSTANCE_NUM]={0};

        int i = 0;
        for(i=0;i<MAX_INSTANCE_NUM;i++)
        {
            s32Ret = HI_GV_Anim_CreateInstance(hAnimInfo,hWidget,&(ahAnimObj[i]));
            if (HI_SUCCESS != s32Ret)
            {
               printf("create error !\n");
            }

        }
        usleep(1000 * 1000);

        for(i=0;i<MAX_INSTANCE_NUM;i++)
        {
            printf("start to destory obj: %d \n",ahAnimObj[i]);

            s32Ret = HI_GV_Anim_DestoryInstance(hWnd,ahAnimObj[i]);
            printf("destroy obj:%d \n",ahAnimObj[i]);
        }
        usleep(5000 * 1000);
        (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
        (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
        (HI_VOID)HI_GV_Widget_Hide(ANIM_TEST);

        s32Ret = HI_GV_Anim_CreateInstance(trs_anim_test,SHOW_IMAGE,&hAnimObj);
        printf("[%s][%d] OK HI_GV_Anim_CreateInstance s32Ret:%d,qBtnOk1=%d \n", __FILE__, __LINE__, s32Ret,hAnimObj);
        usleep(1000 * 1000);
        HI_GV_App_Stop(g_hApp);
    }
    g_AnimTestTouchCnt++;
    printf("g_AnimTestTouchCnt = %d\n", g_AnimTestTouchCnt);
    */
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
