/**
* @file    ui_filelist_page.c
* @brief   ui filelist page. show thumbnails.
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/7
* @version   1.0

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "hi_type.h"
#include "ui_common.h"
#include "hi_filemng_dtcf.h"
#include "hi_mw_type.h"
#include "hi_mp4_format.h"
#include "hi_gv_graphiccontext.h"
#include "hi_appcomm_util.h"
#include "libavformat/avformat.h"
#include "mpi_sys.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

/**used when filemng is busy in scanning, load only 4 files*/
#define UI_FILELIST_PRELOADCOUNT (4)
#define UI_FILELIST_MAX_THM_SIZE (1024*200)
#define MAX_THUMB_SIZE (128*1024)
#define TS_DEFAULT_THUMB_INDEX (2)
#define UI_FILELIST_CACHEROWS (12)

typedef struct tagPDT_UI_FILELISTROW_S
{
    HI_CHAR szThumbnail[HI_APPCOMM_MAX_PATH_LEN];  /**< thumbnail file absolute path */
    HI_U32  u32Selected;        /**< selected : 1,  not selected : 0 */
} PDT_UI_FILELISTROW_S;

typedef struct tagPDT_UI_FILELIST_NODE_S
{
    HI_U32 u32FileIdx;
    struct tagPDT_UI_FILELIST_NODE_S* pstNext;
} PDT_UI_FILELIST_NODE_S;

typedef struct tagPDT_UI_THMLIST_NODE_S
{
    HI_U32 u32Idx;
    HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];
    HI_U8 u8ThumbBuff[UI_FILELIST_MAX_THM_SIZE];
    HI_U32 u32DataLen;
} PDT_UI_THMLIST_NODE_S;

typedef struct tagPDT_UI_THMNAIL_SPACE_S
{
    PDT_UI_THMLIST_NODE_S thmlist[UI_FILELIST_CACHEROWS];
} PDT_UI_THMNAIL_SPACE_S;

typedef struct tagPDT_UI_FILEALLOCMMZ_S
{
    HI_U64  u64PhyAddr;                   /* Alloc MMZ PhyAddr */
    PDT_UI_THMNAIL_SPACE_S* pThmNailSpace;  /* Alloc MMZ VirAddr  */
}PDT_UI_FILEALLOCMMZ_S;


static HI_BOOL s_bSelectState = HI_FALSE;
static HI_BOOL s_bSelectAll = HI_FALSE;
static HI_U32  s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
static HI_U32  s_u32FileObjCnt = 0;
static HI_DTCF_DIR_E  s_enDir = DTCF_DIR_NORM_FRONT;
static PDT_UI_FILEALLOCMMZ_S s_uifilethmnail = {0};

/*we don't use ListHead's  u32FileIdx, just use the pointer NEXT*/
static PDT_UI_FILELIST_NODE_S s_stListHead = {.u32FileIdx = 0, .pstNext=NULL};


#ifdef CONFIG_RECORDMNG_TS
static HI_U32 getCurRelativeTimeMs()
{
    struct timespec ts = {0, 0};
    HI_U64 curT = 0;

    (HI_VOID)clock_gettime(CLOCK_MONOTONIC, &ts);
    curT = ts.tv_sec*1000 + ts.tv_nsec/1000000;

    return (HI_U32)curT;
}

static HI_S32 PDT_UI_FILELIST_ReadTsThumbnail(const HI_CHAR* pszFilePath, HI_U8* pData, HI_U32* pu32BufLen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    AVPacket stPkt;
    HI_BOOL bGetThumb =  HI_FALSE;
    HI_U32 u32ReadTimes = 0;
    AVFormatContext* pstFmtCtx = NULL;

    HI_U32 u32BeginTime = getCurRelativeTimeMs();
    av_register_all();
    av_log_set_level(AV_LOG_QUIET);

    s32Ret =  avformat_open_input(&pstFmtCtx, pszFilePath, NULL, NULL);
    if (s32Ret < 0)
    {
        MLOGE("avformat_open_input failed \n");
        return HI_FAILURE;
    }

    if (avformat_find_stream_info(pstFmtCtx, NULL) < 0)
    {
        MLOGE("Could not find stream information \n");
        avformat_close_input(&pstFmtCtx);
        return HI_FAILURE;
    }
    HI_U32 u32UseTime = getCurRelativeTimeMs() -u32BeginTime;
    MLOGD("file proble useT: %d\n", u32UseTime);
    av_init_packet(&stPkt);

    while (!bGetThumb)
    {
        s32Ret = av_read_frame(pstFmtCtx, &stPkt);
        if (s32Ret != HI_SUCCESS)
        {
            MLOGE("av_read_frame failed! s32Ret = %d \n",  s32Ret);
            break;
        }

        if (!bGetThumb && (stPkt.stream_index == TS_DEFAULT_THUMB_INDEX))
        {
            MLOGD("readtimes %d thumbnail len: %d\n", u32ReadTimes, stPkt.size);
            if(stPkt.size > *pu32BufLen)
            {
                MLOGE("input buf size %u is small than thumbnail size: %u\n", (*pu32BufLen), stPkt.size);

            }
            else
            {
                memcpy(pData, stPkt.data, stPkt.size);
                *pu32BufLen = stPkt.size;
                bGetThumb = HI_TRUE;
            }
            av_packet_unref(&stPkt);
            break;
        }
        u32ReadTimes++;
        av_packet_unref(&stPkt);
    }
    avformat_close_input(&pstFmtCtx);

    if(!bGetThumb)
    {
        MLOGE("there no thumbnail in file\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
#endif

#ifdef CONFIG_RECORDMNG_MP4
static HI_S32 PDT_UI_FILELIST_ReadMp4Thumbnail(const HI_CHAR * pszFilePath, HI_U8** ppData, HI_U32 * pu32DataLen)
{
    HI_S32 i=0;
    for (i = 0; i < UI_FILELIST_CACHEROWS; i++)
    {
        if (0 == strncmp(pszFilePath, s_uifilethmnail.pThmNailSpace->thmlist[i].szFileName, HI_APPCOMM_MAX_PATH_LEN - 1))
        {
            *ppData = s_uifilethmnail.pThmNailSpace->thmlist[i].u8ThumbBuff;
            *pu32DataLen = s_uifilethmnail.pThmNailSpace->thmlist[i].u32DataLen;
            return HI_SUCCESS;
        }
    }

    for (i = 0; i < UI_FILELIST_CACHEROWS; i++)
    {
        if(s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx == 0)
        {
            s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx = UI_FILELIST_CACHEROWS - 1;
        }
        else
        {
            s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx--;
        }
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR pDemuxer = NULL;
    HI_MP4_CONFIG_S stMp4DemuxerCfg = {};
    snprintf(stMp4DemuxerCfg.aszFileName, HI_MP4_MAX_FILE_NAME, pszFilePath);
    stMp4DemuxerCfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
    stMp4DemuxerCfg.stDemuxerConfig.u32VBufSize = 0;
    s32Ret = HI_MP4_Create(&pDemuxer, &stMp4DemuxerCfg);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_MP4_Create(%s) fail:%#x\n", pszFilePath, s32Ret);
        return HI_FAILURE;
    }

    HI_MP4_ATOM_INFO_S stAtomInfo = {};
    stAtomInfo.u32DataLen = UI_FILELIST_MAX_THM_SIZE;
    for (i = 0; i < UI_FILELIST_CACHEROWS; i++)
    {
        if (s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx == UI_FILELIST_CACHEROWS - 1)
        {
            stAtomInfo.pu8DataBuf = s_uifilethmnail.pThmNailSpace->thmlist[i].u8ThumbBuff;
            s32Ret = HI_MP4_GetAtom(pDemuxer, "/thm ", &stAtomInfo);
            if (s32Ret != HI_SUCCESS)
            {
                HI_MP4_Destroy(pDemuxer, NULL);
                MLOGE("HI_MP4_GetAtom(%s) fail:%#x\n", pszFilePath, s32Ret);
                return HI_FAILURE;
            }

            HI_MP4_Destroy(pDemuxer, NULL);
            if (stAtomInfo.u32DataLen > UI_FILELIST_MAX_THM_SIZE)
            {
                MLOGE("%s ThumbnailLength = %u \n", pszFilePath, stAtomInfo.u32DataLen);
                return HI_FAILURE;
            }

            snprintf(s_uifilethmnail.pThmNailSpace->thmlist[i].szFileName, HI_APPCOMM_MAX_PATH_LEN, pszFilePath);
            s_uifilethmnail.pThmNailSpace->thmlist[i].u32DataLen = stAtomInfo.u32DataLen;

            *ppData = s_uifilethmnail.pThmNailSpace->thmlist[i].u8ThumbBuff;
            *pu32DataLen = s_uifilethmnail.pThmNailSpace->thmlist[i].u32DataLen;
            break;
        }

    }
    return HI_SUCCESS;
}
#endif

HI_VOID PDT_UI_FILELIST_GetCurFileIdx(HI_U32* pu32CurFileIdx)
{
    *pu32CurFileIdx = s_u32CurFileIdx;
}

HI_VOID PDT_UI_FILELIST_SetCurFileIdx(HI_U32 u32CurFileIdx)
{
    s_u32CurFileIdx = u32CurFileIdx;
}

HI_VOID PDT_UI_FILELIST_SetSearchScope(HI_DTCF_DIR_E enDir)
{
    s_enDir = enDir;
}

HI_VOID PDT_UI_FILELIST_GetSearchScope(HI_DTCF_DIR_E* penDir)
{
    *penDir = s_enDir;
}

/*static HI_VOID PDT_UI_FILELIST_ListDumpAll(HI_VOID)
{
    PDT_UI_FILELIST_NODE_S* pstGridNode = s_stListHead.pstNext;
    MLOGD("List DumpAll: ");

    while (NULL != pstGridNode)
    {
        printf("%u ", pstGridNode->u32FileIdx);
        pstGridNode = pstGridNode->pstNext;
    }

    MLOGD("\n");
    return;
}*/


/**
 * @brief  add selected grid index to list, order by descend.
 * @return 0 success
 * @return -1 failure
 */
static HI_S32 PDT_UI_FILELIST_ListAddFileIdx(HI_U32 u32FileIdx)
{
    PDT_UI_FILELIST_NODE_S* pstTemp  = NULL;
    PDT_UI_FILELIST_NODE_S* pstGridNode = (PDT_UI_FILELIST_NODE_S*)malloc(sizeof(PDT_UI_FILELIST_NODE_S));

    if (NULL == pstGridNode)
    {
        MLOGE("malloc failed\n");
        return HI_FAILURE;
    }

    pstGridNode->pstNext = NULL;
    pstGridNode->u32FileIdx = u32FileIdx;

    pstTemp = &s_stListHead;

    while (NULL != pstTemp->pstNext
        && u32FileIdx < pstTemp->pstNext->u32FileIdx)
    {
        pstTemp = pstTemp->pstNext;
    }

    pstGridNode->pstNext = pstTemp->pstNext;
    pstTemp->pstNext = pstGridNode;
    /*PDT_UI_FILELIST_ListDumpAll();*/
    return HI_SUCCESS;
}

static HI_S32 PDT_UI_FILELIST_ListFindFileIdx(HI_U32 u32FileIdx)
{
    PDT_UI_FILELIST_NODE_S* pstGridNode = s_stListHead.pstNext;

    while (NULL != pstGridNode)
    {
        if (pstGridNode->u32FileIdx == u32FileIdx)
        {
            return HI_SUCCESS;
        }

        pstGridNode = pstGridNode->pstNext;
    }

    return HI_FAILURE;
}

static HI_S32 PDT_UI_FILELIST_ListDelFileIdx(HI_U32 u32FileIdx)
{
    PDT_UI_FILELIST_NODE_S* pstTemp = NULL;
    PDT_UI_FILELIST_NODE_S* pstDel = NULL;

    pstTemp = &s_stListHead;
    while ( NULL != pstTemp->pstNext && pstTemp->pstNext->u32FileIdx != u32FileIdx)
    {
        pstTemp = pstTemp->pstNext;
    }

    if (NULL == pstTemp->pstNext)
    {
        MLOGD("not found %u\n", u32FileIdx);
        return HI_SUCCESS;
    }
    pstDel = pstTemp->pstNext;
    pstTemp->pstNext = pstDel->pstNext;

    free(pstDel);
    /*PDT_UI_FILELIST_ListDumpAll();*/
    return HI_SUCCESS;
}

/**
 * @brief delete all list node and the corresponding file.
 * @return 0 success
 * @return -1 failure
 */
static HI_S32 PDT_UI_FILELIST_ListDelNodeAndFile(HI_VOID)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN];

    while (NULL != s_stListHead.pstNext)
    {
        s32Ret = HI_FILEMNG_GetFileByIndex(s_stListHead.pstNext->u32FileIdx, szAbsPath, sizeof(szAbsPath));
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_FILEMNG_GetFileByIndex");

        s32Ret = HI_FILEMNG_RemoveFile(szAbsPath);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_FILEMNG_RemoveFile");

        PDT_UI_FILELIST_ListDelFileIdx(s_stListHead.pstNext->u32FileIdx);
    }
    HI_async();
    return HI_SUCCESS;
}

static HI_VOID PDT_UI_FILELIST_ListClear(HI_VOID)
{
    PDT_UI_FILELIST_NODE_S* pstDelNode = NULL;

    while (NULL != s_stListHead.pstNext)
    {
        pstDelNode = s_stListHead.pstNext;
        s_stListHead.pstNext = pstDelNode->pstNext;
        free(pstDelNode);
    }
    s_stListHead.pstNext = NULL;
}

static HI_S32 PDT_UI_FILELIST_Exit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_bSelectState)
    {
        s_bSelectState = HI_FALSE;
        s_bSelectAll = HI_FALSE;
        PDT_UI_FILELIST_ListClear();
        HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(ALBUM_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_StartWindow");
    s32Ret = PDT_UI_WINMNG_FinishWindow(FILELIST_PAGE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_FinishWindow");
    s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGI("\n");
    HI_S32 i=0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_FILEMNG_CFG_S stCfg = {};
    HI_S32  s32CollisionCnt = 0;
    HI_CHAR szFileListLable[HI_DIR_LEN_MAX] = {};
    HI_SYSTEM_STARTUP_SRC_E enStartupSrc = HI_SYSTEM_STARTUP_SRC_BUTT;

    /* malloc mmz  */
    if (s_uifilethmnail.pThmNailSpace != NULL)
    {
        MLOGE("thumbnail space have data !\n");
    }

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&s_uifilethmnail.u64PhyAddr,(HI_VOID*)&(s_uifilethmnail.pThmNailSpace),"thumbnail","anonymous", \
                                        sizeof(PDT_UI_THMLIST_NODE_S)*UI_FILELIST_CACHEROWS);

    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "malloc mmz of file thumbnail");
    memset(s_uifilethmnail.pThmNailSpace,0,sizeof(PDT_UI_THMLIST_NODE_S)*UI_FILELIST_CACHEROWS);

    for(i=0;i<UI_FILELIST_CACHEROWS;i++)
    {
        s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx = i;
    }

    HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
    HI_SYSTEM_GetStartupWakeupSource(&enStartupSrc);

    if ((enStartupSrc == HI_SYSTEM_STARTUP_SRC_STARTUP) && (s32CollisionCnt>0))
    {
        s_enDir = DTCF_DIR_EMR_FRONT;
        s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_PARAM_GetFileMngCfg");

        s32Ret = PDT_UI_COMM_Translate(stCfg.stDtcfCfg.aszDirNames[s_enDir], szFileListLable, HI_DIR_LEN_MAX);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "DTCF_DIR_Translate");

        s32Ret = HI_GV_Widget_SetText(FILELIST_LABEL_INFO, szFileListLable);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_SetText");

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ADM_Sync");

        if(s_u32FileObjCnt > 0)
        {
            s_u32CurFileIdx = 0;
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");
        }
        else
        {
            HI_S32  s32CollisionCnt = 0;
            s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
    }
    else
    {
        s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_PDT_PARAM_GetFileMngCfg");

        s32Ret = PDT_UI_COMM_Translate(stCfg.stDtcfCfg.aszDirNames[s_enDir], szFileListLable, HI_DIR_LEN_MAX);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "DTCF_DIR_Translate");

        s32Ret = HI_GV_Widget_SetText(FILELIST_LABEL_INFO, szFileListLable);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_SetText");

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ADM_Sync");

        if (s_u32FileObjCnt > 0)
        {
            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_MoveToOrigin");
        }

        if (UI_FILELIST_INVALID_FILE_IDX == s_u32CurFileIdx || 0 == s_u32FileObjCnt )
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
        }
        else
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_Widget_Active");
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "HI_GV_ScrollGrid_SetSelCell");
        }
    }

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_end();
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("\n");
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_SYS_MmzFree(s_uifilethmnail.u64PhyAddr, s_uifilethmnail.pThmNailSpace);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret, "free mmz of file thumbnail");

    s_uifilethmnail.pThmNailSpace = NULL;

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

static HI_S32 PDT_UI_FILELIST_OnGridSelect(HI_U32 u32GridIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("GridIndex = %u\n", u32GridIndex);
    s_u32CurFileIdx = u32GridIndex;

    if (0 == s_u32FileObjCnt || s_u32CurFileIdx > s_u32FileObjCnt)
    {
        return HI_SUCCESS;
    }

    if (s_bSelectState)
    {
        s32Ret = PDT_UI_FILELIST_ListFindFileIdx(s_u32CurFileIdx);

        if (HI_SUCCESS == s32Ret)
        {
            s32Ret = PDT_UI_FILELIST_ListDelFileIdx(s_u32CurFileIdx);  /** unselect it */
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "ListDelGrpIdx");
            s_bSelectAll = HI_FALSE;
        }
        else
        {
            s32Ret = PDT_UI_FILELIST_ListAddFileIdx(s_u32CurFileIdx);  /** select it */
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "ListAddGrpIdx");
        }
        s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, u32GridIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");
        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);  /** update filelist page */
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ADM_Sync");
        return HI_SUCCESS;
    }

    s32Ret = PDT_UI_WINMNG_StartWindow(PLAYBACK_PAGE, HI_TRUE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_StartWindow");
    return HI_SUCCESS;
}

static HI_VOID PDT_UI_FILELIST_ListAddAllGrpIdx(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_u32FileObjCnt <= 0)
    {
        return;
    }

    HI_S32 i;
    HI_U32 u32FileIdx = 0;
    for (i=0; i<s_u32FileObjCnt; i++)
    {
        s32Ret = PDT_UI_FILELIST_ListFindFileIdx(u32FileIdx);
        if (HI_SUCCESS != s32Ret)
        {
            s32Ret = PDT_UI_FILELIST_ListAddFileIdx(u32FileIdx);  /** select it */
            PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "ListAddGrpIdx");
        }
        u32FileIdx++;
    }

}

static HI_S32 PDT_UI_FILELIST_OnClick(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        if (!s_bSelectAll)
        {
            s_bSelectAll = HI_TRUE;
            s_bSelectState = HI_TRUE;
            PDT_UI_FILELIST_ListAddAllGrpIdx();
        }
        else
        {
            s_bSelectAll = HI_FALSE;
            s_bSelectState = HI_FALSE;
            PDT_UI_FILELIST_ListClear();
        }
        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ADM_Sync");
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        if (s_bSelectState)
        {
            s_bSelectState = HI_FALSE;
            s_bSelectAll = HI_FALSE;
            s32Ret = PDT_UI_FILELIST_ListDelNodeAndFile();
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "ListDelNodeAndGrp");

            s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ADM_Sync");

            if (s_u32FileObjCnt <= 6)
            {
                s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_MoveToOrigin");
            }
        }
        else
        {
            s_bSelectState = HI_TRUE;
            s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ADM_Sync");
        }

    }
    else if (FILELIST_SCROLLGRID_FILES == WidgetHdl)
    {
        s32Ret = PDT_UI_FILELIST_OnGridSelect(s_u32CurFileIdx);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "OnGridSelect");
    }

    return HI_SUCCESS;

}

static HI_S32 PDT_UI_FILELLIST_FocusToNext(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DELETE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        if(s_u32FileObjCnt > 0)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
            s_u32CurFileIdx = 0;
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");
        }
        else
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
    }
    else
    {
        if(0 == s_u32FileObjCnt || s_u32CurFileIdx >= s_u32FileObjCnt -1 )
        {
            s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;

            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_MoveToOrigin");

            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
        else
        {
            s_u32CurFileIdx += 1;
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");
        }
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_FILELIST_FocusGoBack(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        if(0 == s_u32FileObjCnt)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DELETE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
        else
        {
            s_u32CurFileIdx = s_u32FileObjCnt -1;
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");

            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");

            s32Ret = HI_GV_ScrollGrid_MoveToLast(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_MoveToOrigin");

        }
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else
    {
        if(s_u32CurFileIdx == 0)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DELETE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
            s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_MoveToOrigin");
        }
        else
        {
            s_u32CurFileIdx = s_u32CurFileIdx - 1;
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_OnKeyDown(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
        s32Ret = PDT_UI_FILELIST_Exit();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_Exit");
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_FILELLIST_FocusToNext(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELLIST_FocusToNext");
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_FILELIST_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_OnClick");
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_FILELIST_FocusGoBack(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_FocusGoBack");
    }

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_GRID_OnCellColSelect(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM u32ChildIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_FILELIST_OnGridSelect(u32GridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_OnGridSelect");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_GRID_OnFocusMove(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM lParam)
{
    MLOGD("GridIndex = %lu, lParam = %lu\n", (HI_UL)u32GridIndex, (HI_UL)lParam);

    return HIGV_PROC_GOON;
}
HI_S32 PDT_UI_FILELIST_GRID_GetPhotoThumbnail(HIGV_HANDLE GridHdl, HI_U32 u32GridChild,
    HI_U32 u32Grid, const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    if(DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        return HI_SUCCESS;
    }

    if (NULL == pSrcStr || NULL == pDstStr)
    {
        MLOGE("NULL Pointer\n");
        return HI_FAILURE;
    }
    if (strstr(pSrcStr, ".JPG") == NULL)
    {
        return HI_FAILURE;
    }

    snprintf(pDstStr, u32Length, "%s", pSrcStr);
    return HI_SUCCESS;

}


HI_S32 PDT_UI_FILELIST_GRID_GetVideoThumbnail(HIGV_HANDLE GridHdl, HI_U32 u32GridChild,
    HI_U32 u32Grid, const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HIGV_HANDLE SurfaceHdl = 0;

    if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
    {
        return HI_SUCCESS;
    }

    if (NULL == pSrcStr || NULL == pDstStr)
    {
        MLOGE("NULL Pointer\n");
        return HI_FAILURE;
    }

#ifdef CONFIG_RECORDMNG_TS

    HI_U32 u32DataLen = MAX_THUMB_SIZE;
    HI_U8* pu8Data = malloc(MAX_THUMB_SIZE);
    if (!pu8Data)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        return HI_FAILURE;
    }

    s32Ret = PDT_UI_FILELIST_ReadTsThumbnail(pSrcStr, pu8Data, &u32DataLen);
    if (s32Ret != HI_SUCCESS)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        free(pu8Data);
        return HI_FAILURE;
    }

    s32Ret = HI_GV_GraphicContext_DecodeMemImg((HI_CHAR*)pu8Data, u32DataLen, 0, 0, &SurfaceHdl);
    if (s32Ret != HI_SUCCESS)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        MLOGE("HI_GV_GraphicContext_DecodeMemImg failed:%#x\n", s32Ret);
        free(pu8Data);
        return HI_FAILURE;
    }

    snprintf(pDstStr, u32Length, "%d", SurfaceHdl);
    free(pu8Data);

#endif

#ifdef CONFIG_RECORDMNG_MP4

    HI_U8* pu8Data = NULL;
    HI_U32 u32DataLen = 0;

    s32Ret = PDT_UI_FILELIST_ReadMp4Thumbnail(pSrcStr, &pu8Data, &u32DataLen);
    if (s32Ret != HI_SUCCESS)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        MLOGE("PDT_UI_FILELIST_ReadMp4Thumbnail %s failed:%#x\n", pSrcStr,s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_GV_GraphicContext_DecodeMemImg((HI_CHAR*)pu8Data, u32DataLen, 0, 0, &SurfaceHdl);
    if (s32Ret != HI_SUCCESS)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        MLOGE("HI_GV_GraphicContext_DecodeMemImg %s failed:%#x\n", pSrcStr,s32Ret);
        return HI_FAILURE;
    }
    snprintf(pDstStr, u32Length, "%d", SurfaceHdl);

#endif
    return HI_SUCCESS;
}


/**
 * @brief  given data from row value,  output  SELECT state image's path.
 * @param u32GridChild  grid's child's index.
 * @param u32Grid  grid index
 * @param pSrcStr   in XML file, coldbindex="3", so we get a row's 3rd column value.
 * @param pDstStr  output TYPE image's path
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_FILELIST_GRID_GetSelIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                       const HI_CHAR* pSrcStr, HI_CHAR* pDstStr,HI_U32 u32Length)
{

    if (HI_NULL == pSrcStr || HI_NULL == pDstStr)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }
    if (HI_FALSE == s_bSelectState)
    {
        snprintf(pDstStr, u32Length, "%s", "");
        return HI_SUCCESS;
    }

    if (0 == strcmp("0", pSrcStr))
    {
        snprintf(pDstStr, u32Length, "%s", "/app/bin/res/pic/filelist_selecting.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "/app/bin/res/pic/filelist_selected.png");
    }

    return HI_SUCCESS;
}

/**
 * @brief  given data from row value,  output  FOCUS state image's path.
 * @param u32GridChild  grid's child's index.
 * @param u32Grid  grid index
 * @param pSrcStr  we don't use it.
 * @param pDstStr  output FOCUS box image's path
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_FILELIST_GRID_GetFocusIcon(HIGV_HANDLE GridHdl, HI_U32 u32GridChild, HI_U32 u32Grid,
                                        const HI_CHAR* pSrcStr, HI_CHAR* pDstStr, HI_U32 u32Length)
{
    if (s_u32CurFileIdx == u32Grid)
    {
        snprintf(pDstStr, u32Length, "%s", "/app/bin/res/pic/filelist_focus_box.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "");
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_GRID_Register(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

/** used by datamodel, unregisterdatachange */
HI_S32 PDT_UI_FILELIST_GRID_Unregister(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl)
{
    return HI_SUCCESS;
}

/**
 * @brief used  by datamodel, get filemgr's file count. A grid's data come from a row, which corresponds a group.
 */
HI_S32 PDT_UI_FILELIST_GRID_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_APPCOMM_CHECK_POINTER(pu32RowCnt, HI_FAILURE);

    s32Ret = HI_FILEMNG_SetSearchScope(&s_enDir, 1, &s_u32FileObjCnt);
    if (HI_SUCCESS == s32Ret)
    {
        *pu32RowCnt = s_u32FileObjCnt;
    }
    else if (HI_FILEMNG_EBUSY == s32Ret)
    {
        *pu32RowCnt = UI_FILELIST_PRELOADCOUNT;
    }
    else
    {
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    MLOGD("DirType: %d, RowCount: %d\n",s_enDir, *pu32RowCnt);

    return HI_SUCCESS;
}

/**
 * @brief  get N row's value.  The struct FileListRow_S is consistent with datamodel(datamodel_file_list).
 * @param DBSource DataBase source
 * @param u32StartNow: start from u32StartNow.
 * @param u32RowNumber: hope to get u32RowNumber rows.
 * @param pu32RowCount:  the final gotten row count
 * @return HI_SUCCESS, HI_FAILURE
 */
HI_S32 PDT_UI_FILELIST_GRID_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartIndex, HI_U32 u32ReqNumber, HI_VOID* pData, HI_U32* pu32ResCnt)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_UI_FILELISTROW_S * pstRow = NULL;

    if (HI_NULL == pData || HI_NULL == pu32ResCnt)
    {
        MLOGE("Null Pointer\n");
        return HI_FAILURE;
    }

    MLOGD("u32StartIndex:%u, u32ReqNumber:%u\n", u32StartIndex, u32ReqNumber);

    *pu32ResCnt = 0;
    pstRow = (PDT_UI_FILELISTROW_S *)pData;

    PDT_UI_FILELISTROW_S stRowData;
    HI_U32 u32Start;
    HI_S32 i;

    for (i=0; i<u32ReqNumber; i++)
    {
        memset(&stRowData, 0, sizeof(stRowData));
        u32Start = u32StartIndex+i;
        s32Ret = HI_FILEMNG_GetFileByIndex(u32Start, stRowData.szThumbnail, HI_APPCOMM_MAX_PATH_LEN);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE,"HI_FILEMNG_GetFileByIndex");

        if (HI_SUCCESS == PDT_UI_FILELIST_ListFindFileIdx(u32Start))
        {
            stRowData.u32Selected = 1;
        }
        else
        {
            stRowData.u32Selected = 0;
        }

        memcpy(pstRow, &stRowData, sizeof(PDT_UI_FILELISTROW_S));
        MLOGD("Thumbnail : %s\n", pstRow->szThumbnail);

        pstRow++;
        (*pu32ResCnt)++;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

