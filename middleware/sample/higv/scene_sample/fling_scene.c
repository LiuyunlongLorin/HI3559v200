#include <stdlib.h>
#include <string.h>

#include "higv_cextfile.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

typedef struct
{
    HI_S32 x;
    HI_S32 y;
} HI_COORD;

static HI_BOOL isTouchWin = HI_FALSE;
static HI_COORD s_TouchStartCoord = {0};
static HI_COORD s_TouchEndCoord = {0};

static HI_S32 ParseDirection()
{
    HI_S32 interval_x, interval_y;

    interval_x = abs(s_TouchStartCoord.x - s_TouchEndCoord.x);
    interval_y = abs(s_TouchStartCoord.y - s_TouchEndCoord.y);

    if ((0 == interval_x) && (0 == interval_y))
    {
        printf("[Func: %s, Line: %d] start is equal to end, same point.\n\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }

    if (interval_x > interval_y)
    {
        if (s_TouchStartCoord.x > s_TouchEndCoord.x)
        {
            printf("[Func: %s, Line: %d] from right to left.\n\n", __FUNCTION__, __LINE__);
        }
        else
        {
            printf("[Func: %s, Line: %d] from left to right.\n\n", __FUNCTION__, __LINE__);
        }

        return HI_SUCCESS;
    }

    if (s_TouchStartCoord.y > s_TouchEndCoord.y)
    {
        printf("[Func: %s, Line: %d] from bottom to top.\n\n", __FUNCTION__, __LINE__);
    }
    else
    {
        printf("[Func: %s, Line: %d] from top to bottom.\n", __FUNCTION__, __LINE__);
    }

    return HI_SUCCESS;
}

HI_S32 FLING_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 point_x, point_y;
    HIGV_TOUCH_EVENT_S touchEvent;
    HIGV_TOUCH_E type;
    HIGV_HANDLE hHandle;

    memset(&touchEvent, 0x0, sizeof(touchEvent));
    memcpy(&touchEvent, (HIGV_TOUCH_EVENT_S*)lParam, sizeof(touchEvent));

    point_x = touchEvent.last.x;
    point_y = touchEvent.last.y;

    type = touchEvent.last.type;


    switch (type)
    {
        case HIGV_TOUCH_START:
        {
            (HI_VOID)HI_GV_Widget_GetWidgetByPos_TouchDevice(point_x, point_y, &hHandle);

            if (FLING_SCENE_WIN == hHandle)
            {
                isTouchWin = HI_TRUE;
                s_TouchStartCoord.x = point_x;
                s_TouchStartCoord.y = point_y;
            }

            break;
        }

        case HIGV_TOUCH_END:
        {
            if (HI_TRUE == isTouchWin)
            {
                isTouchWin = HI_FALSE;
                s_TouchEndCoord.x = point_x;
                s_TouchEndCoord.y = point_y;
                (HI_VOID)ParseDirection();
            }

            break;
        }

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 FLING_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    (HI_VOID)HI_GV_Widget_Show(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Active(MAIN_WIN);
    (HI_VOID)HI_GV_Widget_Hide(FLING_SCENE_WIN);

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
