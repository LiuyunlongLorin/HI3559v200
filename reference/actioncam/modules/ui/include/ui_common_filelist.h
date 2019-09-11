/**
 * @file      ui_common_filelist.h
 * @brief     ui common filelist interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/12
 * @version   1.0

 */

#ifndef __UI_COMMON_FILELIST_H__
#define __UI_COMMON_FILELIST_H__

#include "ui_common.h"


#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "UI"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef struct tagPDT_UI_COMM_FILELISTROW_S
{
    HI_CHAR szThumbnail[HI_APPCOMM_MAX_PATH_LEN]; /**< thumbnail file absolute path */
    HI_CHAR szGrpType[12];                        /**< "normmp4"  eg. */
    HI_CHAR szGrpFileCount[16];                   /**< file count in group */
    HI_U32  u32Selected;                          /**< selected : 1,  not selected : 0 */
}PDT_UI_COMM_FILELISTROW_S;

typedef struct tagPDT_UI_COMM_FILELIST_GROUP_NODE_S
{
    HI_U32 u32GroupIndex;
    struct tagPDT_UI_COMM_FILELIST_GROUP_NODE_S* pstNext;
}PDT_UI_COMM_FILELIST_GROUP_NODE_S;

HI_VOID PDT_UI_COMM_FILELIST_GetCurGrpIdx(HI_U32* pu32CurGroupIdx);
HI_VOID PDT_UI_COMM_FILELIST_SetCurGrpIdx(HI_U32 u32CurGroupIdx);
HI_VOID PDT_UI_COMM_FILELIST_GetCurGrpCnt(HI_U32* pu32CurGroupCnt);
HI_VOID PDT_UI_COMM_FILELIST_SetCurGrpCnt(HI_U32 u32CurGroupCnt);

HI_BOOL PDT_UI_COMM_FILELIST_ListIsNull(HI_VOID);
HI_S32 PDT_UI_COMM_FILELIST_ListAddGrpIdx(HI_U32 u32GroupIndex);
HI_S32 PDT_UI_COMM_FILELIST_ListFindGrpIdx(HI_U32 u32GroupIndex);
HI_S32 PDT_UI_COMM_FILELIST_ListDelGrpIdx(HI_U32 u32GroupIndex);
HI_S32 PDT_UI_COMM_FILELIST_ListDelNodeAndGrp(HI_VOID);
HI_VOID PDT_UI_COMM_FILELIST_ListClear(HI_VOID);


HI_S32 PDT_UI_COMM_FILELIST_Exit(HI_PDT_WORKMODE_E enWorkMode, PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);

HI_S32 PDT_UI_COMM_FILELIST_GRID_GetRowCount(HI_U32 u32DBSource, HI_U32* pu32RowCnt);
HI_S32 PDT_UI_COMM_FILELIST_GRID_GetRowValue(HI_U32 u32DBSource, HI_U32 u32StartIndex, HI_U32 u32ReqNumber, HI_VOID* pData, HI_U32* pu32ResCnt);
HI_S32 PDT_UI_COMM_FILELIST_GRID_Register(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl);
HI_S32 PDT_UI_COMM_FILELIST_GRID_Unregister(HI_U32 u32DBSource, HIGV_HANDLE ADMHdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

