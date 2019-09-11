/**
 * @file      ui_windowmng.c
 * @brief     ui window manager. use a list to remember windows.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <pthread.h>

#include "ui_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define UI_WINLIST_SIZE (20)

typedef struct tagUI_WINDOW_S
{
    HIGV_HANDLE WinHdl;
    HI_BOOL bWinIsShow;
}UI_WINDOW_S;

static pthread_mutex_t s_UIWinMngMutex = PTHREAD_MUTEX_INITIALIZER;
static UI_WINDOW_S s_astUIWin[UI_WINLIST_SIZE];
static HI_S32 s_s32UIWinMngTopIndex = -1;

static HI_VOID PDT_UI_WINMNG_ListAddWin(HIGV_HANDLE WinHdl)
{
    HI_S32 i =  0;

    HI_MUTEX_LOCK(s_UIWinMngMutex);

    if(s_s32UIWinMngTopIndex >= 0)
    {
        i = s_s32UIWinMngTopIndex;
    }

    while(i<UI_WINLIST_SIZE)
    {

        if(INVALID_HANDLE == s_astUIWin[i].WinHdl)
        {
            s_astUIWin[i].WinHdl = WinHdl;
            s_astUIWin[i].bWinIsShow = HI_TRUE;
            s_s32UIWinMngTopIndex = i;
            HI_MUTEX_UNLOCK(s_UIWinMngMutex);
            return;
        }

        i++;
    }

    if (i >= UI_WINLIST_SIZE)
    {
        MLOGE("Window list is full!\n");
    }

    HI_MUTEX_UNLOCK(s_UIWinMngMutex);
}

static HI_VOID PDT_UI_WINMNG_ListDelWin(HIGV_HANDLE WinHdl)
{
    HI_S32 i = 0;

    HI_MUTEX_LOCK(s_UIWinMngMutex);

    if (s_s32UIWinMngTopIndex < 0)
    {
        MLOGD("Window list is empty.\n");
        HI_MUTEX_UNLOCK(s_UIWinMngMutex);
        return;
    }

    while(i <= s_s32UIWinMngTopIndex)
    {
        if (WinHdl == s_astUIWin[i].WinHdl)
        {
            while(i < s_s32UIWinMngTopIndex)
            {
                s_astUIWin[i].WinHdl = s_astUIWin[i+1].WinHdl;
                s_astUIWin[i].bWinIsShow = s_astUIWin[i+1].bWinIsShow;
                i++;
            }

            s_astUIWin[s_s32UIWinMngTopIndex].WinHdl = INVALID_HANDLE;
            s_s32UIWinMngTopIndex = s_s32UIWinMngTopIndex-1;
            HI_MUTEX_UNLOCK(s_UIWinMngMutex);
            return;
        }

        i++;
    }
    MLOGD("Delete window %x, not in list\n", WinHdl);

    HI_MUTEX_UNLOCK(s_UIWinMngMutex);
}

static HI_VOID PDT_UI_WINMNG_ListDump(HI_VOID)
{
    HI_S32 i = 0;

    if (s_s32UIWinMngTopIndex < 0)
    {
        MLOGD("Window list is empty.\n");
        return;
    }

    i = s_s32UIWinMngTopIndex;
    MLOGD("(%s): Window ID list: {",__FUNCTION__);
    while(i >= 0)
    {
        MLOGD("%d, ",s_astUIWin[i].WinHdl);
        i--;
    }
    MLOGD("} MaxCount:%d, CurMaxCount:%d\n\n",UI_WINLIST_SIZE,s_s32UIWinMngTopIndex+1);
}

HI_S32 PDT_UI_WINMNG_Init(HI_VOID)
{
    HI_MUTEX_LOCK(s_UIWinMngMutex);

    s_s32UIWinMngTopIndex = -1;
    memset( s_astUIWin, 0, sizeof(s_astUIWin));

    HI_MUTEX_UNLOCK(s_UIWinMngMutex);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_WINMNG_Deinit(HI_VOID)
{
    HI_S32 s32Ret = 0;

    while(s_s32UIWinMngTopIndex >= 0)
    {
        MLOGD("Finishing Window:%u\n", s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
        s32Ret = PDT_UI_WINMNG_FinishWindow(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }

    s_s32UIWinMngTopIndex = -1;
    memset( s_astUIWin, 0, sizeof(s_astUIWin));

    return HI_SUCCESS;
}

HI_BOOL PDT_UI_WINMNG_WindowIsShow(HIGV_HANDLE WinHdl)
{
    HI_S32 i = 0;

    if (s_s32UIWinMngTopIndex < 0)
    {
        return HI_FALSE;
    }

    while(i <= s_s32UIWinMngTopIndex)
    {
        if ((WinHdl == s_astUIWin[i].WinHdl) && s_astUIWin[i].bWinIsShow)
        {
            return HI_TRUE;
        }

        i++;
    }
    return HI_FALSE;
}


HI_S32 PDT_UI_WINMNG_StartWindow(HIGV_HANDLE WinHdl,HI_BOOL bHideCurWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;

    MLOGD("Show Window ID: %d\n",WinHdl);

    /** start a window that winHdl is in this windownMng array */
    while(i <= s_s32UIWinMngTopIndex)
    {
        if (WinHdl == s_astUIWin[i].WinHdl)
        {
            s32Ret = HI_GV_PARSER_LoadViewById(WinHdl);/** no need to judge the ret val */
            s32Ret = HI_GV_Widget_Show(WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s32Ret = HI_GV_Widget_Active(WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            while(s_s32UIWinMngTopIndex > i)
            {
                if (s_astUIWin[s_s32UIWinMngTopIndex].bWinIsShow)
                {
                    s32Ret = HI_GV_Widget_Hide(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }

                PDT_UI_WINMNG_ListDelWin(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
            }

            s_astUIWin[s_s32UIWinMngTopIndex].bWinIsShow = HI_TRUE;
            PDT_UI_WINMNG_ListDump();
            return HI_SUCCESS;
        }

        i++;
    }

    /** start a new window that winHdl is not in this windownMng array */
    s32Ret = HI_GV_PARSER_LoadViewById(WinHdl);/** no need to judge the ret val */
    s32Ret = HI_GV_Widget_Show(WinHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_GV_Widget_Active(WinHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    if( bHideCurWin && s_s32UIWinMngTopIndex >= 0)
    {
        s32Ret = HI_GV_Widget_Hide(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        s_astUIWin[s_s32UIWinMngTopIndex].bWinIsShow = HI_FALSE;
    }

    PDT_UI_WINMNG_ListAddWin(WinHdl);
    PDT_UI_WINMNG_ListDump();
    return HI_SUCCESS;
}

HI_S32 PDT_UI_WINMNG_FinishWindow(HIGV_HANDLE WinHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(s_s32UIWinMngTopIndex < 0)
    {
        MLOGE("Finish window %u, but list is empty!\n", WinHdl);
        return HI_FAILURE;
    }

    MLOGD("Hide Window ID: %d\n",WinHdl);

    if (WinHdl == s_astUIWin[s_s32UIWinMngTopIndex].WinHdl)
    {
        PDT_UI_WINMNG_ListDelWin(WinHdl);
        if(s_s32UIWinMngTopIndex >= 0)
        {
            /*show last window*/
            s32Ret = HI_GV_Widget_Show(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s32Ret = HI_GV_Widget_Active(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s_astUIWin[s_s32UIWinMngTopIndex].bWinIsShow = HI_TRUE;
            s32Ret = HI_GV_Widget_Hide(WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    else
    {
        if (PDT_UI_WINMNG_WindowIsShow(WinHdl) && (s_s32UIWinMngTopIndex >= 1))
        {
            s32Ret = HI_GV_Widget_Hide(WinHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        PDT_UI_WINMNG_ListDelWin(WinHdl);
    }

    PDT_UI_WINMNG_ListDump();
    return HI_SUCCESS;
}


HI_S32 PDT_UI_WINMNG_HideAllWindow(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    while(s_s32UIWinMngTopIndex >= 0)
    {
        s32Ret = HI_GV_Widget_Hide(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGD("HideWidget[%u] failed[0x%08X]\n", s_astUIWin[s_s32UIWinMngTopIndex].WinHdl, s32Ret);
        }

        PDT_UI_WINMNG_ListDelWin(s_astUIWin[s_s32UIWinMngTopIndex].WinHdl);
        PDT_UI_WINMNG_ListDump();
    }

    s_s32UIWinMngTopIndex = -1;
    memset( s_astUIWin, 0, sizeof(s_astUIWin));

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


