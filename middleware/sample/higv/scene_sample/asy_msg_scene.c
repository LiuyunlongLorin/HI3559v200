#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

static pthread_t s_ThreadId = 0;
static HI_BOOL   s_IsThreadCreate = HI_FALSE;

static HI_VOID* thread_send_msg(HI_VOID*)
{
    HI_S32 Ret = HI_SUCCESS;

    Ret = HI_GV_Msg_SendAsync(MSG_SCENE_WIN, HIGV_MSG_EVENT, 20, 0);

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] HI_GV_Msg_SendAsync error.\n\n", __FUNCTION__, __LINE__);
    }

    usleep(2 * 1000 * 1000);

    Ret = HI_GV_Msg_SendAsync(MSG_SCENE_WIN, HIGV_MSG_EVENT, 30, 0);

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] HI_GV_Msg_SendAsync error.\n\n", __FUNCTION__, __LINE__);
    }

    pthread_exit(0);
}

HI_S32 MSG_SCENE_BUTTON1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;

    Ret = HI_GV_Msg_SendAsync(MSG_SCENE_WIN, HIGV_MSG_EVENT, 10, 0);

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] HI_GV_Msg_SendAsync error.\n\n", __FUNCTION__, __LINE__);
    }

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_BUTTON2_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;

    Ret = pthread_create(&s_ThreadId, HI_NULL, thread_send_msg, HI_NULL);

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] Create pthread error!\n\n", __FUNCTION__, __LINE__);
    }

    s_IsThreadCreate = HI_TRUE;

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_BUTTON3_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;
    HIGV_GESTURE_TAP_S tap;

    tap.pointer.id = 0;
    tap.pointer.pressure = 0;
    tap.pointer.timeStamp = 0;
    tap.pointer.type = HIGV_TOUCH_END;
    tap.pointer.x = 25;
    tap.pointer.y = 105;

    Ret = HI_GV_Msg_SendAsyncWithData(MSG_SCENE_BUTTON1, HIGV_MSG_GESTURE_TAP, &tap, sizeof(tap));

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] HI_GV_Msg_SendAsyncWithData error.\n\n", __FUNCTION__, __LINE__);
    }

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_BUTTON4_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;
    HIGV_GESTURE_SCROLL_S scroll;

    scroll.start.id = 0;
    scroll.start.pressure = 1;
    scroll.start.timeStamp = 0;
    scroll.start.type = HIGV_TOUCH_MOVE;
    scroll.start.x = 100;
    scroll.start.y = 50;

    scroll.end.id = 0;
    scroll.end.pressure = 1;
    scroll.end.timeStamp = 0;
    scroll.end.type = HIGV_TOUCH_MOVE;
    scroll.end.x = 105;
    scroll.end.y = 50;

    scroll.distanceX = 5;
    scroll.distanceY = 0;

    Ret = HI_GV_Msg_SendAsyncWithData(MSG_SCENE_WIN, HIGV_MSG_GESTURE_SCROLL, &scroll, sizeof(scroll));

    if (HI_SUCCESS != Ret)
    {
        printf("[Func: %s, Line: %d] HI_GV_Msg_SendAsyncWithData error.\n\n", __FUNCTION__, __LINE__);
    }

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    if (HI_TRUE == s_IsThreadCreate)
    {
        pthread_join(s_ThreadId, HI_NULL);
        s_IsThreadCreate = HI_FALSE;
    }

    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(MSG_SCENE_WIN);

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_WIN_onevent(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 Ret = HI_SUCCESS;

    //printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    switch (wParam)
    {
        case 10:
        {
            Ret = HI_GV_Widget_SetText(MSG_SCENE_WIN_LABEL2, "123456789!");

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_SetText error.\n\n", __FUNCTION__, __LINE__);
            }

            Ret = HI_GV_Widget_Paint(MSG_SCENE_WIN_LABEL2, HI_NULL);

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_Paint error.\n\n", __FUNCTION__, __LINE__);
            }

            break;
        }

        case 20:
        {
            Ret = HI_GV_Widget_SetText(MSG_SCENE_WIN_LABEL2, "thread msg 20");

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_SetText error.\n\n", __FUNCTION__, __LINE__);
            }

            Ret = HI_GV_Widget_Paint(MSG_SCENE_WIN_LABEL2, HI_NULL);

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_Paint error.\n\n", __FUNCTION__, __LINE__);
            }

            break;
        }

        case 30:
        {
            Ret = HI_GV_Widget_SetText(MSG_SCENE_WIN_LABEL2, "thread msg 30");

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_SetText error.\n\n", __FUNCTION__, __LINE__);
            }

            Ret = HI_GV_Widget_Paint(MSG_SCENE_WIN_LABEL2, HI_NULL);

            if (HI_SUCCESS != Ret)
            {
                printf("[Func: %s, Line: %d] HI_GV_Widget_Paint error.\n\n", __FUNCTION__, __LINE__);
            }

            break;
        }

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 MSG_SCENE_WIN_ongesturescroll(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_SCROLL_S gestureScroll;

    memset(&gestureScroll, 0x0, sizeof(HIGV_GESTURE_SCROLL_S));
    memcpy(&gestureScroll, (HIGV_GESTURE_SCROLL_S*)lParam, wParam);

    printf("[Func: %s, Line: %d] start_x: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.start.x);
    printf("[Func: %s, Line: %d] start_y: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.start.y);

    printf("[Func: %s, Line: %d] end_x: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.end.x);
    printf("[Func: %s, Line: %d] end_y: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.end.y);

    printf("[Func: %s, Line: %d] distance_x: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.distanceX);
    printf("[Func: %s, Line: %d] distance_y: %d\n\n", __FUNCTION__, __LINE__, gestureScroll.distanceY);

    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
