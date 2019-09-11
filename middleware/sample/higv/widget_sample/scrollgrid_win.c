#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "higv_cextfile.h"
#include "sample_utils.h"
#include "hi_gv_graphiccontext.h"
#include "hi_gv_input.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

//#define KEY_TEST

#define PIC_COUNT 50  //图片预览的图片单元格总数，实际情况应当以数据库为准
#define FILE_LEN  128 //图片文件的路径名长度，须预留结束符，与datamodel.xml字段中的长度保持一致

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
    {"./res/pic/scrollgrid/image8.JPG"},
    {"./res/pic/scrollgrid/image9.JPG"},
    {"./res/pic/scrollgrid/image10.JPG"},
    {"./res/pic/scrollgrid/image11.JPG"},
    {"./res/pic/scrollgrid/image12.JPG"},
    {"./res/pic/scrollgrid/image13.JPG"},
    {"./res/pic/scrollgrid/image14.JPG"},
    {"./res/pic/scrollgrid/image15.JPG"},
    {"./res/pic/scrollgrid/image16.JPG"},
    {"./res/pic/scrollgrid/image17.JPG"},
    {"./res/pic/scrollgrid/image18.JPG"},
    {"./res/pic/scrollgrid/image19.JPG"},
    {"./res/pic/scrollgrid/image20.JPG"},
    {"./res/pic/scrollgrid/image21.JPG"},
    {"./res/pic/scrollgrid/image22.JPG"},
    {"./res/pic/scrollgrid/image23.JPG"},
    {"./res/pic/scrollgrid/image24.JPG"},
    {"./res/pic/scrollgrid/image25.JPG"},
    {"./res/pic/scrollgrid/image26.JPG"},
    {"./res/pic/scrollgrid/image27.JPG"},
    {"./res/pic/scrollgrid/image28.JPG"},
    {"./res/pic/scrollgrid/image29.JPG"},
    {"./res/pic/scrollgrid/image30.JPG"},
    {"./res/pic/scrollgrid/image31.JPG"},
    {"./res/pic/scrollgrid/image32.JPG"},
    {"./res/pic/scrollgrid/image33.JPG"},
    {"./res/pic/scrollgrid/image34.JPG"},
    {"./res/pic/scrollgrid/image35.JPG"},
    {"./res/pic/scrollgrid/image36.JPG"},
    {"./res/pic/scrollgrid/image37.JPG"},
    {"./res/pic/scrollgrid/image38.JPG"},
    {"./res/pic/scrollgrid/image39.JPG"},
    {"./res/pic/scrollgrid/image40.JPG"},
    {"./res/pic/scrollgrid/image41.JPG"},
    {"./res/pic/scrollgrid/image42.JPG"},
    {"./res/pic/scrollgrid/image43.JPG"},
    {"./res/pic/scrollgrid/image44.JPG"},
    {"./res/pic/scrollgrid/image45.JPG"},
    {"./res/pic/scrollgrid/image46.JPG"},
    {"./res/pic/scrollgrid/image47.JPG"},
    {"./res/pic/scrollgrid/image48.JPG"},
    {"./res/pic/scrollgrid/image49.JPG"},
};

static HI_U32 s_TimerId = 0;

/**0: down, 1: up, 2: right, 3: left*/
static HI_S32 s_KeyDirection = 0;

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

/** datamodel.xml : registerdatachange */
HI_S32 GridRegister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM)
{
    return HI_SUCCESS;
}

/** datamodel.xml : unregisterdatachange */
HI_S32 GridUnregister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM)
{
    return HI_SUCCESS;
}

/** +++ userdb获取数据总数回调函数，对应datamodel.xml中的"getrowcount" +++
    获取Row总数，在scrollgrid中表现为单元格总数*/
HI_S32 GridGetCount(HIGV_HANDLE DBSource, HI_U32* RowCnt)
{
    *RowCnt = PIC_COUNT;
    return HI_SUCCESS;
}

/** +++ userdb获取数据内容回调函数，对应datamodel.xml中的"getrowvalue" +++
    DBSource: 回调该函数的ADM句柄
    Row:      起始行索引，在scrollgrid中表现为单元格索引
    Num:      本次获取的缓冲行数(Cell number)，奇数
    pData:    数据传出指针
    pRowNum:  传出数据的真实行数 在scrollgrid中表现为单元格数*/
HI_S32 GridGetValue(HIGV_HANDLE DBSource, HI_U32 Row, HI_U32 Num, HI_VOID* pData, HI_U32* pRowNum)
{
    HI_U32 i = Row;
    HI_U32 j = 0;
    HI_U32 RowNum = 0;

    //*pRowNum = 0;

    /** 拷贝数据约束:
        1.拷贝次数小于入参Num
        2.拷贝索引小于数据总数*/
    for (j = 0; i < PIC_COUNT && j < Num ; i++, j++)
    {
        /** 将输入拷贝给传出指针，因为会将字段填满0，可以不拷贝结束符*/
        memcpy(pData, ImgFile[i], strlen(ImgFile[i]));
        /** 将字段填满0，字段长度为datamodel字段的长度*/
        memset((char*)pData + strlen(ImgFile[i]), 0x0, FILE_LEN - strlen(ImgFile[i]));

        /** 拷贝完一行数据之后偏移*/
        pData = (char*)pData + FILE_LEN;

        /** 传出数据行数加1*/
        //*pRowNum++;
        RowNum++;
    }

    *pRowNum = RowNum;

    return HI_SUCCESS;
}

/** 列转换函数，用作网格中小图标控制*/
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

HI_S32 GridTextConv(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 Length)
{
    HI_CHAR *str1 = "hello";
    HI_CHAR *str2 = "world";

    if (0 == (Item % 2))
    {
        memcpy(pDstStr, str1, strlen(str1) + 1);
    }
    else
    {
        memcpy(pDstStr, str2, strlen(str2) + 1);
    }

    return HI_SUCCESS;
}

static HI_S32 KeyTestBegin()
{
#ifdef KEY_TEST
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Create(SCROLLGRID_WIN, s_TimerId, 1000);
    HIGV_CHECK("HI_GV_Timer_Create", s32Ret);

    s32Ret = HI_GV_Timer_Start(SCROLLGRID_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Start", s32Ret);
#endif
    return HI_SUCCESS;
}

static HI_S32 KeyTestEnd()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_GV_Timer_Stop(SCROLLGRID_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Stop", s32Ret);

    s32Ret = HI_GV_Timer_Destroy(SCROLLGRID_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Destroy", s32Ret);

    return HI_SUCCESS;
}

static HI_S32 KeyTestMove(HI_U32 keyValue)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /*two ways  for send key msg,  either of them is correct*/
#if 0
    HIGV_INPUTEVENT_S inputevent;
    inputevent.msg = HIGV_MSG_KEYDOWN;
    inputevent.value = keyValue;
    inputevent.dx = 0;
    inputevent.dy = 0;
    inputevent.step = 0;
    s32Ret = HI_GV_SendInputEvent(&inputevent);
    HIGV_CHECK("HI_GV_SendInputEvent", s32Ret);
#else
    s32Ret = HI_GV_Msg_SendAsync(SCROLLGRID_WIN_SCROLLGRID, HIGV_MSG_KEYDOWN, keyValue, 0);
    HIGV_CHECK("HI_GV_SendInputEvent", s32Ret);
#endif

    s32Ret = HI_GV_Timer_Reset(SCROLLGRID_WIN, s_TimerId);
    HIGV_CHECK("HI_GV_Timer_Reset", s32Ret);

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

HI_S32 SCROLLGRID_WIN_onshow(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_onrefresh(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    /*test key up*/
    //(HI_VOID)HI_GV_ScrollGrid_MoveToLast(SCROLLGRID_WIN_SCROLLGRID);

    /*test key left*/
    //(HI_VOID)HI_GV_ScrollGrid_SetSelCell(SCROLLGRID_WIN_SCROLLGRID, 1);

    (HI_VOID)KeyTestBegin();

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_onhide(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_ontimer(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    switch (s_KeyDirection)
    {
        case 0: //down
            (HI_VOID)KeyTestMove(HIGV_KEY_DOWN);
            break;


        case 1: //up
            (HI_VOID)KeyTestMove(HIGV_KEY_UP);
            break;


        case 2: //right
            (HI_VOID)KeyTestMove(HIGV_KEY_RIGHT);
            break;


        case 3: //left
            (HI_VOID)KeyTestMove(HIGV_KEY_LEFT);
            break;

        default:
            break;
    }

    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_oncellcolselect(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
#ifndef KEY_TEST
    HI_GV_ScrollGrid_SetSelCell(hWidget, wParam);
#endif
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusmove(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachbtm(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    (HI_VOID)KeyTestEnd();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachleft(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    (HI_VOID)KeyTestEnd();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachright(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    (HI_VOID)KeyTestEnd();
    return HIGV_PROC_GOON;
}

HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachtop(HIGV_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    printf("[Func: %s, Line: %d] \n\n", __FUNCTION__, __LINE__);

    (HI_VOID)KeyTestEnd();
    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
