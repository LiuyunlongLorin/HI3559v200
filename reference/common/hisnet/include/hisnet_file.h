/****************************************************************************
*              Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hisnet_file.h
* Description: Describe API of file.
*
* History:
* Version   Date         Author     DefectNum    Description
* B010      2012-05-05   skf71347   NULL         Create this file.
* B020      2012-09-11   l00228447  NULL         Modify this file.
* B030
******************************************************************************/

#ifndef HISNET_FILE_H
#define HISNET_FILE_H

#include "hisnet_type_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef HI_S32 (*Hisnet_FILE_GetFileNum_FN_PTR)(HI_U32* pu32FileNum);
typedef HI_S32 (*Hisnet_FILE_GetFileList_FN_PTR)(HI_U32 u32Start, HI_U32 u32End, HISNET_REMOTEFILENAME_S stFileList[], HI_U32* pu32Cnt);
typedef HI_S32 (*Hisnet_FILE_GetFileInfo_FN_PTR)(HISNET_REMOTEFILENAME_S* pstFileName, FILEMNG_REMOTEFILEINFO_S* pstFileInfo);
typedef HI_S32 (*Hisnet_FILE_DeleteFile_FN_PTR)(HISNET_REMOTEFILENAME_S* pstFileName);
typedef HI_S32 (*Hisnet_FILE_DeleteAllFiles_FN_PTR)(HI_VOID);

typedef struct hisnet_FILE_SVR_CB_S
{
    Hisnet_FILE_GetFileNum_FN_PTR           pfnGetFileNum;
    Hisnet_FILE_GetFileList_FN_PTR          pfnGetFileList;
    Hisnet_FILE_GetFileInfo_FN_PTR          pfnGetFileInfo;
    Hisnet_FILE_DeleteFile_FN_PTR           pfnDeleteFile;
    Hisnet_FILE_DeleteAllFiles_FN_PTR       pfnDeleteAllFiles;
} HISNET_FILE_SVR_CB_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif/* HISNET_FILE_H */
