#ifndef HISNET_CMD_FILE_H
#define HISNET_CMD_FILE_H

#include "hisnet.h"
#include "hisnet_file.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 Hisnet_CMD_FILE_GetFileNum(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData);

HI_S32 Hisnet_CMD_FILE_GetFileList(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData);

HI_S32 Hisnet_CMD_FILE_GetFileInfo(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData);

HI_S32 Hisnet_CMD_FILE_DeleteFile(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData);

HI_S32 Hisnet_CMD_FILE_DeleteAllFiles(HI_S32 argc, const HI_CHAR* argv[], PfnCgiStringProc pfnCGIStringProc, HI_VOID* pData);

HI_S32 Hisnet_FILE_GetCallBackFunction(HISNET_FILE_SVR_CB_S* pstSvrCB);

HI_S32 Hisnet_FILE_RegCallBackFunction(const HISNET_FILE_SVR_CB_S* pstSvrCB);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*HISNET_CMD_NET_H*/
