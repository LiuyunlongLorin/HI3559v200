#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "higv_cextfile.h"
#include "sample_utils.h"
#include "hi_gv_graphiccontext.h"
#include "hi_gv_input.h"
#include "higv_language.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


HIGV_HANDLE qhMenuShow = INVALID_HANDLE;
HIGV_HANDLE qhMenuHide = INVALID_HANDLE;

HI_BOOL gMenuIsHide = HI_TRUE;

HI_S32 StartAnim2(HIGV_HANDLE hAnim, HIGV_HANDLE hWidget, HIGV_HANDLE& hAnimObj)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (INVALID_HANDLE == hAnimObj)
    {
        s32Ret = HI_GV_Anim_CreateInstance(hAnim, hWidget, &hAnimObj);
        printf("[%s][%d] OK HI_GV_Anim_CreateInstance s32Ret:%d,qBtnOk1=%d \n", __FILE__, __LINE__, s32Ret, hAnimObj);
    }

    s32Ret = HI_GV_Anim_Start(SCROLLGRID_WIN, hAnimObj);
    printf("[%s][%d] ok HI_GV_Anim_Start s32Ret:%x \n", __FUNCTION__, __LINE__, s32Ret);
    return HI_SUCCESS;
}

//////////////////////////////////////////////////////////
#define HIGV_CONTROL_PAGENUM   15

static HI_VOID MenuWinList_AddData()
{
    HI_S32 s32Ret;
    HI_U32 u32Index = 0;
    HIGV_HANDLE hDDB = 0;
    HIGV_DBROW_S stDbRow;

    HI_U32 StrSet[HIGV_CONTROL_PAGENUM] = {STR_BUTTON_SCENE, STR_FLING_SCENE, STR_SCROLLBAR_SKIN_SCENE, STR_ASY_MSG_SCENE,
                                           STR_MULTI_LANGUAGE_SCENE, STR_HISTOGRAM_SCENE, STR_SCROLLVIEW_SCENE1, STR_SCROLLVIEW_SCENE2,
                                           STR_ANIM_INOUT, STR_ANIM_WIN_MOVE, STR_ANIM_TAB_INOUT, STR_WIN_CLOCK, STR_WIN_MENU_ROLL,
                                           STR_ANIM_TEST, STR_IMAGEEX_SCENE
                                          };

    /** Get DDB (default data base) handle*/
    s32Ret = HI_GV_ADM_GetDDBHandle(ADM_SET_STRINGID, &hDDB);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Failed to add data, function:<%s>, retval %#x.\n", __FUNCTION__, s32Ret);
        return;
    }

    /** Disable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_FALSE);

    /** Clear DDB*/
    HI_GV_DDB_Clear(hDDB);

    /** Add data*/
    for (u32Index = 0; u32Index < HIGV_CONTROL_PAGENUM; u32Index++)
    {
        memset(&stDbRow, 0x00, sizeof(stDbRow));
        stDbRow.pData = &StrSet[u32Index];
        stDbRow.Size  = sizeof(HI_U32);
        HI_GV_DDB_Append(hDDB, &stDbRow);
    }

    /** Enable DDB change*/
    HI_GV_DDB_EnableDataChange(hDDB, HI_TRUE);

    HI_GV_Widget_SyncDB(MENU_WIN_LISTBOX1);

    return;
}
#define PIC_COUNT 8  //Í¼Æ¬Ô¤ÀÀµÄÍ¼Æ¬µ¥Ôª¸ñ×ÜÊý£¬Êµ¼ÊÇé¿öÓ¦µ±ÒÔÊý¾Ý¿âÎª×¼
#define FILE_LEN  128 //Í¼Æ¬ÎÄ¼þµÄÂ·¾¶Ãû³¤¶È£¬ÐëÔ¤Áô½áÊø·û£¬Óëdatamodel.xml×Ö¶ÎÖÐµÄ³¤¶È±£³ÖÒ»ÖÂ

/********************** Global Variable declaration **************************/

static HI_CHAR ImgFile[PIC_COUNT][FILE_LEN] =
{
    {"./res/pic/scrollgrid/image0.JPG"},
    {"./res/pic/scrollgrid/image1.JPG"},
    {"./res/pic/scrollgrid/image2.JPG"},
    {"./res/pic/scrollgrid/image3.JPG"},
    {"./res/pic/scrollgrid/image4.JPG"},
    {"./res/pic/scrollgrid/image5.JPG"},
    {"./res/pic/scrollgrid/image6.JPG"},
    {"./res/pic/scrollgrid/image7.JPG"},
};

static HI_U32 s_TimerId = 0;

/**0: down, 1: up, 2: right, 3: left*/
static HI_S32 s_KeyDirection = 0;

/** datamodel.xml : registerdatachange */
HI_S32 ADM_IMAGE_Register(HIGV_HANDLE DBSource, HIGV_HANDLE hADM)
{
    return HI_SUCCESS;
}

/** datamodel.xml : unregisterdatachange */
HI_S32 ADM_IMAGE_unRegister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM)
{
    return HI_SUCCESS;
}

/******************************* API declaration *****************************/
static int CountInteger(int s)
{
    int i = 0;

    while (s != 0)
    {
        s /= 10;
        i++;
    }

    return i;
}

/** +++ userdb»ñÈ¡Êý¾Ý×ÜÊý»Øµ÷º¯Êý£¬¶ÔÓ¦datamodel.xmlÖÐµÄ"getrowcount" +++
    »ñÈ¡Row×ÜÊý£¬ÔÚscrollgridÖÐ±íÏÖÎªµ¥Ôª¸ñ×ÜÊý*/
HI_S32 GridGetCount(HIGV_HANDLE DBSource, HI_U32* RowCnt)
{
    *RowCnt = PIC_COUNT;
    return HI_SUCCESS;
}

/** +++ userdb»ñÈ¡Êý¾ÝÄÚÈÝ»Øµ÷º¯Êý£¬¶ÔÓ¦datamodel.xmlÖÐµÄ"getrowvalue" +++
    DBSource: »Øµ÷¸Ãº¯ÊýµÄADM¾ä±ú
    Row:      ÆðÊ¼ÐÐË÷Òý£¬ÔÚscrollgridÖÐ±íÏÖÎªµ¥Ôª¸ñË÷Òý
    Num:      ±¾´Î»ñÈ¡µÄ»º³åÐÐÊý(Cell number)£¬ÆæÊý
    pData:    Êý¾Ý´«³öÖ¸Õë
    pRowNum:  ´«³öÊý¾ÝµÄÕæÊµÐÐÊý ÔÚscrollgridÖÐ±íÏÖÎªµ¥Ôª¸ñÊý*/
HI_S32 GridGetValue(HIGV_HANDLE DBSource, HI_U32 Row, HI_U32 Num, HI_VOID* pData, HI_U32* pRowNum)
{
    HI_U32 i = Row;
    HI_U32 j = 0;
    HI_U32 RowNum = 0;

    //*pRowNum = 0;

    /** ¿½±´Êý¾ÝÔ¼Êø:
        1.¿½±´´ÎÊýÐ¡ÓÚÈë²ÎNum
        2.¿½±´Ë÷ÒýÐ¡ÓÚÊý¾Ý×ÜÊý*/
    for (j = 0; i < PIC_COUNT && j < Num ; i++, j++)
    {
        /** ½«ÊäÈë¿½±´¸ø´«³öÖ¸Õë£¬ÒòÎª»á½«×Ö¶ÎÌîÂú0£¬¿ÉÒÔ²»¿½±´½áÊø·û*/
        memcpy(pData, ImgFile[i], strlen(ImgFile[i]));
        /** ½«×Ö¶ÎÌîÂú0£¬×Ö¶Î³¤¶ÈÎªdatamodel×Ö¶ÎµÄ³¤¶È*/
        memset((char*)pData + strlen(ImgFile[i]), 0x0, FILE_LEN - strlen(ImgFile[i]));

        /** ¿½±´ÍêÒ»ÐÐÊý¾ÝÖ®ºóÆ«ÒÆ*/
        pData = (char*)pData + FILE_LEN;
        printf("file name is : %s\n", ImgFile[i]);

        /** ´«³öÊý¾ÝÐÐÊý¼Ó1*/
        //*pRowNum++;
        RowNum++;
    }

    *pRowNum = RowNum;

    return HI_SUCCESS;
}

/** ÁÐ×ª»»º¯Êý£¬ÓÃ×÷Íø¸ñÖÐÐ¡Í¼±ê¿ØÖÆ*/
HI_S32 GridIconConv(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 Length)
{
    HIGV_HANDLE SurfaceHandle;
    FILE* pFile = HI_NULL;

    if (0 == (Item % 2))
    {
        pFile = fopen("./res/pic/scrollgrid/jpg_icon.png", "rb");
    }
    else
    {
        pFile = fopen("./res/pic/scrollgrid/mp4_icon.png", "rb");
    }

    char* pBuf;
    fseek(pFile, 0, SEEK_END);
    int len = ftell(pFile);
    pBuf = (char*)malloc(len + 1);
    rewind(pFile);
    fread(pBuf, 1, len, pFile);
    pBuf[len] = 0;

    HI_GV_GraphicContext_DecodeMemImg(pBuf, len, 0, 0, &SurfaceHandle);

    snprintf(pDstStr, CountInteger(SurfaceHandle) + 1, "%d", SurfaceHandle);

    fclose(pFile);
    free(pBuf);
    pBuf = HI_NULL;

    return HI_SUCCESS;
}


HI_S32 SCROLLGRID_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Widget_Show(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Active(MAIN_WIN);
    s32Ret |= HI_GV_Widget_Hide(SCROLLGRID_WIN);

    if (s32Ret != HI_SUCCESS)
    {
        printf("SCROLLGRID_WIN_IMAGE1_ontouchaction failed! s32Ret: %x\n", s32Ret);
    }

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_LABEL1_OnAnimStop(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ((MENU_WIN_LISTBOX1 == hWidget) && (qhMenuHide == lParam))
    {
        (HI_VOID)HI_GV_Widget_Hide(MENU_WIN_LISTBOX1);
    }

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_LABEL1_ontouchaction(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_TOUCH_EVENT_S* touchEvent = (HIGV_TOUCH_EVENT_S*)lParam;

    if (HIGV_TOUCH_END != touchEvent->last.type)
    {
        return s32Ret;
    }

    if (INVALID_HANDLE != qhMenuShow)
    {
        s32Ret = HI_GV_Anim_Stop(SCROLLGRID_WIN, qhMenuShow);
        printf("sucess to stop show animation s32Ret=%d\n", s32Ret);
    }

    if (INVALID_HANDLE != qhMenuHide)
    {
        s32Ret = HI_GV_Anim_Stop(SCROLLGRID_WIN, qhMenuHide);
        printf("sucess to stop hide animation s32Ret=%d\n", s32Ret);
    }

    if (HI_TRUE == gMenuIsHide)
    {
        (HI_VOID)HI_GV_Widget_Show(MENU_WIN_LISTBOX1);
        StartAnim2(animroll_listbox_up_down, MENU_WIN_LISTBOX1, qhMenuShow);
        gMenuIsHide = HI_FALSE;
    }
    else
    {
        StartAnim2(animroll_listbox_up, MENU_WIN_LISTBOX1, qhMenuHide);
        gMenuIsHide = HI_TRUE;
    }

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    MenuWinList_AddData();
    (HI_VOID)HI_GV_Widget_Hide(MENU_WIN_LISTBOX1);
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
