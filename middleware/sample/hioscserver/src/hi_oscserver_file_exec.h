/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_oscserver_file_exec.h
* @brief     oscserver file exec head file
* @author    HiMobileCam middleware develop team
* @date      2017.10.10
*/
#ifndef __HI_OSCSERVER_FILE_EXEC_H__
#define __HI_OSCSERVER_FILE_EXEC_H__

#include <pthread.h>
#include "hi_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 OSCSVR_Process_GetImage(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_ListFiles(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_ListImages(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_GetMetadata(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_Delete(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_SwitchWifi(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_UploadFile(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);
HI_S32 OSCSVR_Process_ProcessPicture(HI_MW_PTR pOSC, HI_S32* ps32SocketFd, HI_CHAR* pClientReq, HI_U32 u32ReqLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_OSCSERVER_FILE_EXEC_H__*/
