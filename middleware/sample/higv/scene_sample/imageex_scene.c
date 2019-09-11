#include "higv_cextfile.h"
#include<pthread.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*  __cplusplus  */

bool QIsRuning = false;
int  QIsStart = 0;

#define FACE_NAME_SHY   "./res/pic/shy.gif"
#define TEST_FILE_NAME   "./res/pic/image/IMG_GIF.gif"

typedef enum E_UI_FACE_CONT
{
   UI_FACE_NAME_SHY,
   UI_FACE_NAME_IMG,
   UI_FACE_BUTT
}e_UI_FACE_CONT;


void showexp(int exptype)
{
    HI_CHAR * filename;
    int repeatcount = exptype % 3 + 1;
    int ret = 0;
    switch(exptype)
    {
        case UI_FACE_NAME_SHY:
            filename = FACE_NAME_SHY;
            break;
        case UI_FACE_NAME_IMG:
            filename = TEST_FILE_NAME;
            repeatcount = -1;
            break;
        default:
            return;
    }

    ret = HI_GV_ImageEx_SetRepeatCount(SHOW_IMAGE_GIF, repeatcount);
    printf("set HI_GV_ImageEx_SetRepeatCount ret=%d \n",ret);
    //filename = TEST_FILE_NAME;
    ret = HI_GV_ImageEx_SetImage(SHOW_IMAGE_GIF, filename);
    printf("set HI_GV_ImageEx_SetImage ret=%d ,filename=%s\n",ret,filename);
    //HI_GV_Widget_Paint(SHOW_IMAGE_GIF,0);
    return;
}

int index = 0;
void* thread_func(void *arg)
{
    QIsRuning = true;
    printf("start to run index=%d\n",index);
    while(QIsRuning)
    {
        showexp(index++);
        if(index == UI_FACE_BUTT)
            index=0;
        break;
    }
    printf("end to run \n");
    return 0;
}

HI_S32 IMAGEEX_SCENE_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(IMAGEEX_SCENE_WIN);
    return HIGV_PROC_GOON;
}

HI_S32 IMAGEEX_SCENE_WIN_onrepeatfinish(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    printf("++++++ ok new repeate finsih :set count=%d,crt=%d \n",wParam,lParam);
    return HIGV_PROC_GOON;
}

HI_S32 IMAGEEX_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam)
{
    QIsStart++;
    if ((QIsStart > 3) && (0 == QIsStart % 2))
    {
        if (false == QIsRuning)
        {
            showexp(index++);
            if(index == UI_FACE_BUTT)
                index=0;
            QIsRuning = true;
        }
        else
        {
            QIsRuning = false;
        }
    }
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

