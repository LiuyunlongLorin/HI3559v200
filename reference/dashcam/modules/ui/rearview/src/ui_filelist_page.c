
/**
 * @file      ui_filelist_page.c
 * @brief     File list and playback
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
#include "hi_mapi_disp.h"
#include "ui_common.h"
#include "hi_mw_type.h"
#include "hi_mp4_format.h"
#include "hi_gv_graphiccontext.h"
#include "hi_appcomm_util.h"
#include "mpi_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */


/**used when filemng is busy in scanning, load only 4 files*/
#define UI_FILELIST_PRELOADCOUNT (4)
#define UI_FILELIST_MAX_THM_SIZE (1024*200)
#define UI_FILELIST_CACHEROWS (12)

typedef struct tagPDT_UI_FILELISTROW_S
{
    HI_CHAR szThumbnail[HI_APPCOMM_MAX_PATH_LEN];  /**< thumbnail file absolute path */
    HI_U32  u32Selected;         /**< selected : 1,  not selected : 0 */
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

typedef enum tagEVENT_UI_PLAYBACK_E
{
    PLAYBACK_EVENT_PLAY_END,
    PLAYBACK_EVENT_PLAY_ERROR,
    PLAYBACK_EVENT_REFRESH_UI
} EVENT_UI_PLAYBACK_E;



static HI_BOOL s_bSelectState = HI_FALSE;
static HI_BOOL s_bSelectAll = HI_FALSE;
static HI_U32  s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
static HI_U32  s_u32FileObjCnt = 0;
static HI_DTCF_DIR_E  s_enDir = DTCF_DIR_NORM_FRONT;
static HI_CHAR s_szCurFileAbsPath[HI_APPCOMM_MAX_PATH_LEN];
static PDT_UI_FILEALLOCMMZ_S s_uifilethmnail = {0};

/*we don't use ListHead's  u32FileIdx, just use the pointer NEXT*/
static PDT_UI_FILELIST_NODE_S s_stListHead = {.u32FileIdx = 0, .pstNext=NULL};


static HI_BOOL s_bIsTracking = HI_FALSE;
static HI_U32  s_u32PlayTime_ms = 0;
static HI_U32  s_u32FileTotalTime_ms = 0;




HI_U32 PDT_UI_PLAYBACK_MillisecondToSecond(HI_U32 u32Time_ms)
{
    return u32Time_ms/1000 + (u32Time_ms%1000>500 ? 1 : 0);
}

HI_S32 PDT_UI_PlAYBACK_Refresh(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szPlayTime[16] = {};

    if (DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        snprintf(szPlayTime, 16, "%02d:%02d/%02d:%02d",
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms)/60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32PlayTime_ms)%60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms)/60,
            PDT_UI_PLAYBACK_MillisecondToSecond(s_u32FileTotalTime_ms)%60);
    }
    else /*PHOTO*/
    {

    }
    s32Ret = HI_GV_Widget_SetText(PLAYBACK_BUTTON_INFO, szPlayTime);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


    if (DTCF_DIR_PHOTO_FRONT == s_enDir || DTCF_DIR_PHOTO_REAR == s_enDir)
    {
        s32Ret = HI_GV_Widget_Hide(PLAYBACK_BUTTON_PLAY);
        s32Ret |= HI_GV_Widget_Hide(PLAYBACK_TRACKBAR_PROGRESS);
    }
    else
    {
        s32Ret = HI_GV_Widget_Show(PLAYBACK_BUTTON_PLAY);
        s32Ret |= HI_GV_Widget_Show(PLAYBACK_TRACKBAR_PROGRESS);
    }
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if ( !s_bIsTracking && DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        if (s_u32FileTotalTime_ms != 0)
        {
            s32Ret = HI_GV_Track_SetRange(PLAYBACK_TRACKBAR_PROGRESS, 0, s_u32FileTotalTime_ms);
        }

        s32Ret |= HI_GV_Track_SetCurVal(PLAYBACK_TRACKBAR_PROGRESS, s_u32PlayTime_ms);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    if (DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);

        if((s32Ret == HI_SUCCESS) && (HI_LITEPLAYER_STATE_PLAY == enPlayState
            || HI_LITEPLAYER_STATE_TPLAY == enPlayState
            || HI_LITEPLAYER_STATE_PREPARED == enPlayState))
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL,SKIN_PLAY_ACTIVE);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE,SKIN_PLAY_ACTIVE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            s32Ret = HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_NORMAL,SKIN_PLAY_NORMAL);
            s32Ret |= HI_GV_Widget_SetSkin(PLAYBACK_BUTTON_PLAY, HIGV_SKIN_ACITVE,SKIN_PLAY_NORMAL);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }

    s32Ret = HI_GV_Widget_Paint(FILELIST_PAGE, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}


HI_S32 PDT_UI_PLAYBACK_HandlePlayEndEvent(HI_VOID)
{

    if(DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
    {
        HI_S32 s32Ret = HI_SUCCESS;
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        s_u32PlayTime_ms = 0;
    }
    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_HandlePlayErrorEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_PLAYBACK_Stop();
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_u32PlayTime_ms = s_u32FileTotalTime_ms;

    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_PlayerEventProc(HI_LITEPLAYER_EVENT_E enEvent, const HI_VOID* pvData, HI_VOID* pvUserData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(enEvent)
    {
        case HI_LITEPLAYER_EVENT_STATE_CHANGED:
            MLOGD("HI_LITEPLAYER_EVENT_STATE_CHANGED:%d\n", *(HI_LITEPLAYER_STATE_E*)pvData);
            s32Ret = HI_GV_Msg_SendAsync(FILELIST_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_REFRESH_UI, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_EOF:
            s32Ret = HI_GV_Msg_SendAsync(FILELIST_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_END, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_SEEK_END:
            break;

        case HI_LITEPLAYER_EVENT_ERROR:
            s32Ret = HI_GV_Msg_SendAsync(FILELIST_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_PLAY_ERROR, HI_TRUE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;

        case HI_LITEPLAYER_EVENT_PROGRESS:
            if(DTCF_DIR_PHOTO_FRONT != s_enDir && DTCF_DIR_PHOTO_REAR != s_enDir)
            {
                s_u32PlayTime_ms = *((HI_U32 *)pvData);
                MLOGD("s_u32PlayTime_ms = %u\n", s_u32PlayTime_ms);
                s32Ret = HI_GV_Msg_SendAsync(FILELIST_PAGE, HIGV_MSG_EVENT, PLAYBACK_EVENT_REFRESH_UI, HI_TRUE);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            }
            break;

        default:
            MLOGD("Event ID %u not need to be processed here\n", enEvent);
            break;
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_UI_PLAYBACK_UpdateFileInfo(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_SetSearchScope(&s_enDir, 1, &s_u32FileObjCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if (s_u32CurFileIdx >= s_u32FileObjCnt)
    {
        s_u32CurFileIdx = 0;
    }

    MLOGD("s_u32CurFileIdx:%u, s_u32FileObjCnt:%u \n", s_u32CurFileIdx, s_u32FileObjCnt);

    if (0 == s_u32FileObjCnt)
    {
        MLOGE("file count:%d \n", s_u32FileObjCnt);
        return HI_FAILURE;
    }


    memset(s_szCurFileAbsPath, 0, sizeof(s_szCurFileAbsPath));

    s32Ret = HI_FILEMNG_GetFileByIndex(s_u32CurFileIdx, s_szCurFileAbsPath, sizeof(s_szCurFileAbsPath));
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);


    MLOGD("s_u32CurFileIdx:%u, s_u32FileObjCnt:%u\n", s_u32CurFileIdx, s_u32FileObjCnt);

    return HI_SUCCESS;
}

HI_VOID PDT_UI_PLAYBACK_VideoPlayerInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_WORKMODE_CFG_S stCfg;
    HI_PLAYBACK_CFG_S stPlayBackCfg;

    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_PLAYBACK, &stCfg);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_PDT_PARAM_GetWorkModeCfg failed: %#x\n", s32Ret);
    }

    stPlayBackCfg.pfnEventProc = PDT_UI_PLAYBACK_PlayerEventProc;
    stPlayBackCfg.pvUserData = NULL;
    memcpy(&stPlayBackCfg.stPlayerParam, &stCfg.unModeCfg.stPlayBackCfg.stPlayerParam, sizeof(HI_LITEPLAYER_PARAM_S));
    memcpy(&stPlayBackCfg.stVoutOpt, &stCfg.unModeCfg.stPlayBackCfg.stVoutOpt, sizeof(HI_LITEPLAYER_VOUT_OPT_S));
    memcpy(&stPlayBackCfg.stAoutOpt, &stCfg.unModeCfg.stPlayBackCfg.stAoutOpt, sizeof(HI_LITEPLAYER_AOUT_OPT_S));

    s32Ret = HI_PLAYBACK_Init(&stPlayBackCfg);
    if(s32Ret && HI_EINITIALIZED != s32Ret)
    {
        MLOGE("HI_PLAYBACK_Init: %#x\n", s32Ret);
    }

}

static HI_S32 PDT_UI_PLAYBACK_ClearVoBuf(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_WORKMODE_CFG_S stCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_PLAYBACK, &stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    HI_HANDLE DispHdl = stCfg.unModeCfg.stPlayBackCfg.stVoutOpt.hModHdl;
    HI_HANDLE WndHdl = stCfg.unModeCfg.stPlayBackCfg.stVoutOpt.hChnHdl;
    s32Ret = HI_MAPI_DISP_ClearWindow(DispHdl, WndHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

HI_S32 PDT_UI_PLAYBACK_PlayFile(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_LITEPLAYER_STATE_E enPlayState = HI_LITEPLAYER_STATE_BUTT;


    s32Ret = HI_PLAYBACK_GetPlayState( &enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if( HI_LITEPLAYER_STATE_PLAY == enPlayState
        || HI_LITEPLAYER_STATE_PAUSE == enPlayState
        || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    HI_FORMAT_FILE_INFO_S stMediaInfo = {};
    s32Ret = HI_PLAYBACK_GetMediaInfo(s_szCurFileAbsPath, &stMediaInfo);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_PLAYBACK_GetMediaInfo");
    s_u32FileTotalTime_ms = (HI_U32)stMediaInfo.s64Duration;
    s_u32PlayTime_ms = 0;

    PDT_UI_PLAYBACK_ClearVoBuf();

    MLOGI("pszFilePath:%s\n", s_szCurFileAbsPath);
    s32Ret = HI_PLAYBACK_Play(stMediaInfo.s32UsedVideoStreamIndex, stMediaInfo.s32UsedAudioStreamIndex, NULL);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    return HI_SUCCESS;
}

#ifdef CONFIG_RECORDMNG_MP4
static HI_S32 PDT_UI_FILELIST_ReadMp4Thumbnail(const HI_CHAR * pszFilePath, HI_U8** ppData, HI_U32 * pu32DataLen)
{
    HI_S32 i=0;
    for(i=0;i<UI_FILELIST_CACHEROWS;i++)
    {
        if(0 == strncmp(pszFilePath,s_uifilethmnail.pThmNailSpace->thmlist[i].szFileName,HI_APPCOMM_MAX_PATH_LEN-1))
        {
            *ppData = s_uifilethmnail.pThmNailSpace->thmlist[i].u8ThumbBuff;
            *pu32DataLen = s_uifilethmnail.pThmNailSpace->thmlist[i].u32DataLen;
            return HI_SUCCESS;
        }
    }

    for(i=0;i<UI_FILELIST_CACHEROWS;i++)
    {
        if(s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx == 0)
        {
            s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx = UI_FILELIST_CACHEROWS-1;
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

    for(i=0;i<UI_FILELIST_CACHEROWS;i++)
    {
        if(s_uifilethmnail.pThmNailSpace->thmlist[i].u32Idx == UI_FILELIST_CACHEROWS-1)
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
            snprintf(s_uifilethmnail.pThmNailSpace->thmlist[i].szFileName,HI_APPCOMM_MAX_PATH_LEN,pszFilePath);
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

HI_VOID PDT_UI_FILELIST_SetDirectory(HI_DTCF_DIR_E enDir)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szFileListLable[HI_DIR_LEN_MAX] = {};
    s_enDir = enDir;
    HI_CHAR szBuffer[HI_DIR_LEN_MAX] = {};

    s32Ret = PDT_UI_COMM_GetDirName(s_enDir, szBuffer, HI_DIR_LEN_MAX);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"GetDirName");

    s32Ret = PDT_UI_COMM_Translate(szBuffer, szFileListLable, HI_DIR_LEN_MAX);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "PDT_UI_COMM_Translate");

    s32Ret = HI_GV_Widget_SetText(FILELIST_BUTTON_DIRECTORY, szFileListLable);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"");

    s32Ret = HI_GV_Widget_Paint(FILELIST_BUTTON_DIRECTORY, NULL);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"");

    s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DIRECTORY);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret,"");

    s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
    PDT_UI_CHECK_RET_WITHOUT_RETURN_VALUE(s32Ret, "");
}

HI_VOID PDT_UI_FILELIST_GetDirectory(HI_DTCF_DIR_E* penDir)
{
    *penDir = s_enDir;
}


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
    HI_CHAR szAbsPath[HI_APPCOMM_MAX_PATH_LEN] = {0};
    HI_LITEPLAYER_STATE_E enPlayState;

    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    while (NULL != s_stListHead.pstNext)
    {
        s32Ret = HI_FILEMNG_GetFileByIndex(s_stListHead.pstNext->u32FileIdx, szAbsPath, HI_APPCOMM_MAX_PATH_LEN);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_FILEMNG_GetFileByIndex");

        s32Ret = strncmp(s_szCurFileAbsPath, szAbsPath, HI_APPCOMM_MAX_PATH_LEN);
        if(!s32Ret)
        {
            if(HI_LITEPLAYER_STATE_PLAY == enPlayState
                || HI_LITEPLAYER_STATE_PAUSE == enPlayState
                || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
            {
                s32Ret = HI_PLAYBACK_Stop();
                HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
                s_u32PlayTime_ms = 0;
            }

        }

        s32Ret = HI_FILEMNG_RemoveFile(szAbsPath);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "HI_FILEMNG_RemoveFile");

        PDT_UI_FILELIST_ListDelFileIdx(s_stListHead.pstNext->u32FileIdx);
    }

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


static HI_S32 PDT_UI_FILELIST_OnReceiveMsgResult(HI_EVENT_S * pstEvent)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    switch(pstEvent->EventID)
    {
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
            if(HI_SUCCESS != pstEvent->s32Result)
            {
                MLOGE("Switch workmode failed:%x\n", s32Ret);
                break;
            }

            s32Ret = PDT_UI_WINMNG_FinishWindow(FILELIST_PAGE);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_WINMNG_FinishWindow");
            s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
            break;

        default:
            MLOGD("Event ID %u not need to be processed here\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_UI_FILELIST_EventProc(HI_EVENT_S * pstEvent)
{
    HI_APPCOMM_CHECK_POINTER(pstEvent, HI_FAILURE);

    switch (pstEvent->EventID)
    {
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;

        case HI_EVENT_FILEMNG_SCAN_COMPLETED:
        case HI_EVENT_STORAGEMNG_MOUNTED:
            HI_GV_ADM_Sync(DATAMODEL_FILELIST);
            break;

        default:
            MLOGD("Event ID %u not need to be processed here\n", pstEvent->EventID);
            break;
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_FILELIST_Back(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MESSAGE_S stMessage = {};

#ifdef CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_begin();
#endif

    if(s_bSelectState)
    {
        s_bSelectState = HI_FALSE;
        s_bSelectAll = HI_FALSE;
        PDT_UI_FILELIST_ListClear();
        HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        return HI_SUCCESS;
    }

    HI_LITEPLAYER_STATE_E enPlayState=HI_LITEPLAYER_STATE_IDLE;
    s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    if(HI_LITEPLAYER_STATE_PLAY == enPlayState
    || HI_LITEPLAYER_STATE_PAUSE == enPlayState
    || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
    {
        s32Ret = HI_PLAYBACK_Stop();
        HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
        s_u32PlayTime_ms = 0;
    }

    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "GetPoweronWorkmode");
    MLOGD("enPoweronWorkmode(%d)\n", enPoweronWorkmode);

    HI_U32 u32Index = 0;
    HI_U32 u32DirCount = 0;
    HI_PDT_FILEMNG_CFG_S stCfg = {};
    HI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++)
    {
        if ( 0 < strnlen(stCfg.stDtcfCfg.aszDirNames[u32Index], HI_DIR_LEN_MAX))
        {
            aenDirs[u32DirCount++] = u32Index;
        }
    }

    HI_U32 u32Temp = 0;
    s32Ret = HI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32Temp);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    stMessage.what = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stMessage.arg2 = enPoweronWorkmode;

    s32Ret = PDT_UI_WINMNG_StartWindow(BUSY_PAGE, HI_FALSE);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "StartWindow");

    s32Ret = PDT_UI_COMM_SendAsyncMsg(&stMessage, PDT_UI_FILELIST_OnReceiveMsgResult);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SendAsyncMsg failed. %x\n", s32Ret);
        s32Ret = PDT_UI_WINMNG_FinishWindow(BUSY_PAGE);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 PDT_UI_FILELIST_OnShow(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32  s32CollisionCnt;
    MLOGD("\n");
    HI_S32 i=0;

    /* malloc mmz  */
    if(s_uifilethmnail.pThmNailSpace != NULL)
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

    PDT_UI_PLAYBACK_VideoPlayerInit();
    s_u32PlayTime_ms = 0;
    s_u32FileTotalTime_ms = 0;
    s32Ret = PDT_UI_PlAYBACK_Refresh();
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT,&s32CollisionCnt);
    HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    if (s32CollisionCnt > 0 )
    {
        s_enDir = DTCF_DIR_EMR_FRONT;
        PDT_UI_FILELIST_SetDirectory(s_enDir);
        if (s_u32FileObjCnt > 0)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
            s_u32CurFileIdx = 0;
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            s32Ret = PDT_UI_PLAYBACK_UpdateFileInfo();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            s32CollisionCnt = 0;
            s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_PARKING_COLLISION_CNT, &s32CollisionCnt);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
    }
    else
    {
        PDT_UI_FILELIST_SetDirectory(s_enDir);
    }

#ifdef  CFG_TIME_STAMP_ON
    PDT_UI_COMM_SwitchPageTime_end();
#endif
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnHide(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if(s_bSelectState)
    {
        s_bSelectState = HI_FALSE;
        s_bSelectAll = HI_FALSE;
        PDT_UI_FILELIST_ListClear();
    }

    s32Ret = HI_PLAYBACK_Stop();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGW("HI_PLAYBACK_Stop %x\n", s32Ret);
    }

    s_u32PlayTime_ms = 0 ;
    s_u32FileTotalTime_ms = 0;

    s32Ret = HI_PLAYBACK_Deinit();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGW("HI_PLAYBACK_Deinit %x\n", s32Ret);
    }

    /*clean info of file selected currently */
    if (s_u32FileObjCnt > 0)
    {
        s_u32CurFileIdx = 0;
        s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGW("HI_GV_ScrollGrid_SetSelCell failed! %x\n", s32Ret);
        }
        s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
    }

    s32Ret = HI_MPI_SYS_MmzFree(s_uifilethmnail.u64PhyAddr,s_uifilethmnail.pThmNailSpace);
    PDT_UI_CHECK_RET_WITHOUT_RETURN(s32Ret,"free mmz of file thumbnail");

    s_uifilethmnail.pThmNailSpace = NULL;

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnTimer(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnEvent(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("wParam: %#lx, lParam = %#lx\n", (HI_UL)wParam, (HI_UL)lParam);

    switch(wParam)
    {
        case PLAYBACK_EVENT_PLAY_END:
            s32Ret = PDT_UI_PLAYBACK_HandlePlayEndEvent();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        case PLAYBACK_EVENT_REFRESH_UI:
            MLOGD("----PLAYBACK_EVENT_REFRESH_UI\n");
            s32Ret = PDT_UI_PlAYBACK_Refresh();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        case PLAYBACK_EVENT_PLAY_ERROR:
            s32Ret = PDT_UI_PLAYBACK_HandlePlayErrorEvent();
            HI_APPCOMM_CHECK_RETURN(s32Ret, HIGV_PROC_GOON);
            break;
        default:
            MLOGE("unhandled event: %#lx !\n", (HI_UL)wParam);
            break;
    }
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
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            s_bSelectAll = HI_FALSE;
        }
        else
        {
            s32Ret = PDT_UI_FILELIST_ListAddFileIdx(s_u32CurFileIdx);  /** select it */
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, u32GridIndex);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "SetSelCell");

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);  /** update filelist page */
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        return HI_SUCCESS;
    }

    /*TODO: Play file */
    PDT_UI_PLAYBACK_UpdateFileInfo();
    PDT_UI_PLAYBACK_PlayFile();

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

    if (FILELIST_BUTTON_BACK == WidgetHdl)
    {
        s32Ret = PDT_UI_FILELIST_Back();
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_FILELIST_Exit");
    }
    if (FILELIST_BUTTON_DIRECTORY == WidgetHdl)
    {
        if(s_bSelectState)/* clean  select_state when click dir_button by touch */
        {
            s_bSelectAll = HI_FALSE;
            s_bSelectState = HI_FALSE;
            PDT_UI_FILELIST_ListClear();
        }

        HI_LITEPLAYER_STATE_E enPlayState;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if(HI_LITEPLAYER_STATE_PLAY == enPlayState
            || HI_LITEPLAYER_STATE_PAUSE == enPlayState
            || HI_LITEPLAYER_STATE_TPLAY == enPlayState)
        {
            s32Ret = HI_PLAYBACK_Stop();
            HI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            s_u32PlayTime_ms = 0;
        }

        s32Ret = PDT_UI_WINMNG_StartWindow(FILE_DIRECTORY_PAGE, HI_FALSE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "PDT_UI_FILELIST_Exit");
    }
    else if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        if (0 == s_u32FileObjCnt)
        {
            return HI_SUCCESS;
        }
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
        }
        else
        {
            s_bSelectState = HI_TRUE;
        }
        HI_async();

        s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ADM_Sync");

        if (s_u32FileObjCnt <= 6)
        {
            HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
        }
    }
    else if (FILELIST_SCROLLGRID_FILES == WidgetHdl)
    {
        s32Ret = PDT_UI_FILELIST_OnGridSelect(s_u32CurFileIdx);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "OnGridSelect");
    }
    else if (PLAYBACK_BUTTON_PLAY == WidgetHdl)
    {
        HI_LITEPLAYER_STATE_E enPlayState;
        s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        if (HI_LITEPLAYER_STATE_PLAY == enPlayState || HI_LITEPLAYER_STATE_TPLAY == enPlayState \
            || HI_LITEPLAYER_STATE_PREPARED == enPlayState)
        {
            MLOGD("HI_PLAYBACK_Pause\n");
            s32Ret = HI_PLAYBACK_Pause();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else if (HI_LITEPLAYER_STATE_PAUSE == enPlayState)
        {
            MLOGD("HI_PLAYBACK_Resume\n");
            s32Ret = HI_PLAYBACK_Resume();
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        }
        else
        {
            MLOGD("PDT_UI_PLAYBACK_PlayFile\n");
            PDT_UI_PLAYBACK_UpdateFileInfo();
            PDT_UI_PLAYBACK_PlayFile();
        }
    }

    return HI_SUCCESS;

}

static HI_S32 PDT_UI_FILELIST_FocusToNext(HIGV_HANDLE WidgetHdl)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (FILELIST_BUTTON_BACK == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DIRECTORY);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        if(s_bSelectState)
        {
            /*skip  DIRECTORY  while selecting files*/
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
    }
    else if (FILELIST_BUTTON_DIRECTORY == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DELETE);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        if (s_u32FileObjCnt > 0)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
            s_u32CurFileIdx = 0;
            s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");
        }
        else
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
    }
    else if (FILELIST_SCROLLGRID_FILES == WidgetHdl)
    {
        if(0 == s_u32FileObjCnt || s_u32CurFileIdx >= s_u32FileObjCnt -1 )
        {
            s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");

            s32Ret = HI_GV_ScrollGrid_MoveToOrigin(FILELIST_SCROLLGRID_FILES);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_MoveToOrigin");

            s32Ret = HI_GV_ADM_Sync(DATAMODEL_FILELIST);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
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

    if (FILELIST_BUTTON_BACK == WidgetHdl)
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
    else if (FILELIST_BUTTON_DIRECTORY == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else if (FILELIST_BUTTON_SELECT_ALL == WidgetHdl)
    {
        if(s_bSelectState)
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
        else
        {
            s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_DIRECTORY);
            HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
        }
    }
    else if (FILELIST_BUTTON_DELETE == WidgetHdl)
    {
        s32Ret = HI_GV_Widget_Active(FILELIST_BUTTON_SELECT_ALL);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");
    }
    else if (FILELIST_SCROLLGRID_FILES == WidgetHdl)
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
    MLOGD("Handle:%x, wParam: %#lx, lParam = %#lx\n",WidgetHdl,  (HI_UL)wParam, (HI_UL)lParam);
    HI_S32 s32Ret = HI_SUCCESS;

    if (PDT_UI_KEY_BACK == wParam)
    {
        PDT_UI_FILELIST_Back();
    }
    else if (PDT_UI_KEY_MENU == wParam)
    {
        s32Ret = PDT_UI_FILELIST_FocusToNext(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_FocusToNext");
    }
    else if (PDT_UI_KEY_OK == wParam)
    {
        s32Ret = PDT_UI_FILELIST_OnClick(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_OnClick");
    }
    else if (PDT_UI_KEY_EXPAND == wParam)
    {
        s32Ret = PDT_UI_FILELIST_FocusGoBack(WidgetHdl);
        HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_FocusToNext");
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnGestureTap(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("Handle:%x, wParam: %#lx, lParam = %#lx\n", WidgetHdl,  (HI_UL)wParam, (HI_UL)lParam);

    s32Ret = HI_GV_Widget_Active(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");

    s32Ret = PDT_UI_FILELIST_OnClick(WidgetHdl);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "PDT_UI_FILELIST_OnClick");

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_OnGestureFling(HIGV_HANDLE WidgetHdl, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_GESTURE_EVENT_S* pstGestureEvent = (HIGV_GESTURE_EVENT_S*)lParam;
    HI_S32 s32StartX, s32StartY, s32EndX, s32EndY;

    s32StartX = pstGestureEvent->gesture.fling.start.x;
    s32StartY = pstGestureEvent->gesture.fling.start.y;
    s32EndX   = pstGestureEvent->gesture.fling.end.x;
    s32EndY   = pstGestureEvent->gesture.fling.end.y;
    MLOGD("Start: [%d,%d] --> End:[%d,%d]\n", s32StartX, s32StartY, s32EndX, s32EndY);

    if (abs(s32StartX - s32EndX) > abs(s32StartY - s32EndY))
    {
        if (s32StartX > s32EndX)
        {
            MLOGD("fling left\n");

        }
        else
        {
            MLOGD("fling right\n");

        }
    }
    else
    {
        if (s32StartY > s32EndY)
        {
            MLOGD("fling up\n");
        }
        else
        {
            MLOGD("fling down\n");

        }
    }

    return HIGV_PROC_GOON;
}


HI_S32 PDT_UI_FILELIST_GRID_OnCellColSelect(HIGV_HANDLE WidgetHdl, HI_PARAM u32GridIndex, HI_PARAM u32ChildIndex)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MLOGD("GridIndex = %lu, ChildIndex = %lu\n", (HI_UL)u32GridIndex, (HI_UL)u32ChildIndex);
    if(s_u32FileObjCnt == 0)
    {
        HI_GV_Widget_Active(FILELIST_BUTTON_BACK);
        s_u32CurFileIdx = UI_FILELIST_INVALID_FILE_IDX;
        return HIGV_PROC_GOON;
    }
    s_u32CurFileIdx = u32GridIndex;

    s32Ret = HI_GV_Widget_Active(FILELIST_SCROLLGRID_FILES);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_Widget_Active");

    s32Ret = HI_GV_ScrollGrid_SetSelCell(FILELIST_SCROLLGRID_FILES, s_u32CurFileIdx);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "HI_GV_ScrollGrid_SetSelCell");

    s32Ret = PDT_UI_FILELIST_OnGridSelect(u32GridIndex);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HIGV_PROC_GOON, "OnGridSelect");

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
 * @param pSrcStr   in XML file, coldbindex="1", so we get a row's 1st column value.
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

    if (0 == strncmp("0", pSrcStr, sizeof("0")))
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_selecting.png");
    }
    else
    {
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_selected.png");
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
        snprintf(pDstStr, u32Length, "%s", "./res/pic/filelist_focus_box.png");
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
 * @brief used  by datamodel, get filemgr's group count. A grid's data come from a row, which corresponds a group.
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
    MLOGD("DirType: %d, Got RowCount: %u\n", s_enDir, *pu32RowCnt);


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

    MLOGD("u32StartIndex:%u, u32ReqNumber:%u, s_u32CurFileIdx:%u\n", u32StartIndex, u32ReqNumber, s_u32CurFileIdx);

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
        MLOGD("thumbnail: %s\n", pstRow->szThumbnail);
        pstRow++;
        (*pu32ResCnt)++;
    }

    return HI_SUCCESS;
}



HI_S32 PDT_UI_FILELIST_TRACKBAR_OnTouchAction(HI_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S * pstTouchEvent = NULL;
    HI_S32 s32Ret = 0;
    HI_U32 u32CurValue = 0;
    HI_LITEPLAYER_STATE_E   enPlayState;
    pstTouchEvent = (HIGV_TOUCH_EVENT_S*)lParam;
    if(NULL == pstTouchEvent)
    {
        MLOGE("Null Pointer\n");
        return HIGV_PROC_GOON;
    }

    switch (pstTouchEvent->last.type)
    {
        case HIGV_TOUCH_END:
            if (s_bIsTracking)
            {
                s_bIsTracking = HI_FALSE;
                HI_GV_Track_GetCurVal(PLAYBACK_TRACKBAR_PROGRESS, &u32CurValue);
                s32Ret = HI_PLAYBACK_GetPlayState(&enPlayState);
                HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                if (enPlayState != HI_LITEPLAYER_STATE_IDLE)
                {
                    s_u32PlayTime_ms = u32CurValue;
                    s32Ret = HI_PLAYBACK_Seek((HI_S64)u32CurValue);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
                }
            }
            break;
        case HIGV_TOUCH_START:
            s_bIsTracking = HI_TRUE;
            break;
        default:
            break;
    }
    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_TRACKBAR_OnValueChange(HI_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    MLOGD("%#lx\n", (HI_UL)wParam);

    return HIGV_PROC_GOON;
}

HI_S32 PDT_UI_FILELIST_TRACKBAR_OnMouseOut(HI_HANDLE hWidget, HI_PARAM wParam, HI_PARAM lParam)
{
    HIGV_TOUCH_EVENT_S touchEvent;
    memset(&touchEvent, 0x0, sizeof(touchEvent));
    touchEvent.last.id = 0;
    touchEvent.last.type = HIGV_TOUCH_END;

    HI_GV_Msg_SendAsyncWithData(PLAYBACK_TRACKBAR_PROGRESS, HIGV_MSG_TOUCH,
            &touchEvent, sizeof(HIGV_TOUCH_EVENT_S));

    return HIGV_PROC_GOON;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

