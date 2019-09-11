#ifndef HISNET_CMD_SYS_H
#define HISNET_CMD_SYS_H

#include "hisnet.h"
#include "hisnet_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 Hisnet_CMD_SYS_DevAttr(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_GetSDStatus(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_SDCommand(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_SysTimeCfg(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_Reset(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_GetBatteryStatus(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_CMD_SYS_WifiAttrCfg(int argc, const char* argv[], PfnCgiStringProc pfnCGIStringProc, void* pData);

HI_S32 Hisnet_SYS_GetCallBackFunction(HISNET_SYS_SVR_CB_S* pstSvrCB);

HI_S32 Hisnet_SYS_RegCallBackFunction(const HISNET_SYS_SVR_CB_S* pstSvrCB);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
