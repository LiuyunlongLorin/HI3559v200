/**
 * @file      ui_file_directory.c
 * @brief    File Directory Selector.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/15
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ui_common.h"
#include "hi_gv_listbox.h"
#include "hi_product_param.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */



HI_S32 PDT_UI_FILE_DIRECTORY_Register(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILE_DIRECTORY_Unregister(HI_U32 u32DBSource, HI_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILE_DIRECTORY_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt)
{
    HI_S32 s32Ret = 0;
    HI_U32 i = 0;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg = {};

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    *pu32RowCnt = 0;
    for(i=0; i<DTCF_DIR_BUTT; i++)
    {
        if( '\0' == stFileMngCfg.stDtcfCfg.aszDirNames[i][0])
        {
            continue;
        }
        (*pu32RowCnt)++;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILE_DIRECTORY_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartRow,
    HI_U32 u32RequestCount, HI_VOID* pData, HI_U32* pu32GotCount)
{
    HI_U32 i = 0;
    HI_U32 j = 0;
    HI_S32 s32Ret = 0;
    HI_CHAR szFileListLable[HI_DIR_LEN_MAX] = {};
    PDT_UI_DIRECTORY_INFO_S * pstDirInfo = (PDT_UI_DIRECTORY_INFO_S*)pData;
    HI_PDT_FILEMNG_CFG_S stFileMngCfg = {};

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    *pu32GotCount = 0;
    for(i=0; i<DTCF_DIR_BUTT; i++)
    {
        if( '\0' == stFileMngCfg.stDtcfCfg.aszDirNames[i][0])
        {
            /*invalid dir*/
            continue;
        }

        j++;    /*find a valid dir*/

        if( j - 1 < u32StartRow)
        {
            continue;
        }
        if( j - 1 >= u32StartRow + u32RequestCount)
        {
            break;
        }

        pstDirInfo->enDirectory = i;
        PDT_UI_COMM_GetDirName(i, pstDirInfo->szDirectoryName, sizeof(pstDirInfo->szDirectoryName));

        s32Ret = PDT_UI_COMM_Translate(pstDirInfo->szDirectoryName, szFileListLable,sizeof(pstDirInfo->szDirectoryName));
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        snprintf(pstDirInfo->szDirectoryName,sizeof(pstDirInfo->szDirectoryName), "%s", szFileListLable);

        pstDirInfo++ ;
        (*pu32GotCount)++;

    }

    return HI_SUCCESS;
}



HI_S32 PDT_UI_FILE_DIRECTORY_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILE_DIRECTORY);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Active(FILE_DIRECTORY_LISTBOX);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    s32Ret = HI_GV_List_SetSelItem(FILE_DIRECTORY_LISTBOX, 0);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    s32Ret = HI_GV_Widget_Paint(FILE_DIRECTORY_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILE_DIRECTORY_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILE_DIRECTORY_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{

    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_FILE_DIRECTORY_OnClick(HI_VOID)
{
    HI_S32  s32Ret;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32GotCount = 0;
    PDT_UI_DIRECTORY_INFO_S stDirInfo = {};

    s32Ret = HI_GV_List_GetSelItem(FILE_DIRECTORY_LISTBOX, &u32CurrentRow);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    s32Ret = PDT_UI_FILE_DIRECTORY_GetRowValue(
        DATAMODEL_FILE_DIRECTORY, u32CurrentRow, 1, &stDirInfo, &u32GotCount);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    PDT_UI_FILELIST_SetDirectory(stDirInfo.enDirectory);

    s32Ret = PDT_UI_WINMNG_FinishWindow(FILE_DIRECTORY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILE_DIRECTORY_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s32Ret = PDT_UI_WINMNG_FinishWindow(FILE_DIRECTORY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = HI_GV_List_GetSelItem(FILE_DIRECTORY_LISTBOX, &u32CurrentRow);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        s32Ret = HI_GV_List_GetItemNum(FILE_DIRECTORY_LISTBOX, &u32ItemNum);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

        s32Ret = HI_GV_List_SetSelItem(FILE_DIRECTORY_LISTBOX, (u32CurrentRow+1)%u32ItemNum);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = HI_GV_Widget_Paint(FILE_DIRECTORY_PAGE, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = HI_GV_List_GetSelItem(FILE_DIRECTORY_LISTBOX, &u32CurrentRow);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
        s32Ret = HI_GV_List_GetItemNum(FILE_DIRECTORY_LISTBOX, &u32ItemNum);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);
        u32CurrentRow = u32CurrentRow ? (u32CurrentRow -1) : (u32ItemNum -1);
        s32Ret = HI_GV_List_SetSelItem(FILE_DIRECTORY_LISTBOX, u32CurrentRow);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

        s32Ret = HI_GV_Widget_Paint(FILE_DIRECTORY_PAGE, NULL);
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);

    }

    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_FILE_DIRECTORY_OnClick();
        HI_APPCOMM_CHECK_RETURN(s32Ret,HIGV_PROC_GOON);
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILE_DIRECTORY_OnCellSelect(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32CurrentRow = 0;
    HI_U32 u32ItemNum = 0;

    s32Ret = HI_GV_List_GetSelItem(FILE_DIRECTORY_LISTBOX, &u32CurrentRow);
    s32Ret |= HI_GV_List_GetItemNum(FILE_DIRECTORY_LISTBOX, &u32ItemNum);
    MLOGD("ItemNum: %d, Current Row:%d\n", u32ItemNum, u32CurrentRow);

    s32Ret = PDT_UI_FILE_DIRECTORY_OnClick();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILE_DIRECTORY_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    MLOGD("widget:%d, [%d,%d]\n",WidgetHdl, pstGestureEvent->gesture.tap.pointer.x, pstGestureEvent->gesture.tap.pointer.y);

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_FILE_DIRECTORY_OnTouchAction(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
#if 0
    HIGV_TOUCH_EVENT_S* pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
#endif

    return HIGV_PROC_GOON;
}




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

