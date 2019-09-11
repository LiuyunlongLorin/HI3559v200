#ifndef __COMMON_PLAYER_CLIENT_IPCMSG_H__
#define __COMMON_PLAYER_CLIENT_IPCMSG_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

HI_S32 COMMON_IPCMSG_CLIENT_Init();

HI_S32 COMMON_IPCMSG_CLIENT_Deinit();

HI_S32 COMMON_Client_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height);

HI_S32 COMMON_Client_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
    HI_HANDLE hDispHdl, HI_HANDLE hWndHdl);

HI_S32 COMMON_Client_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt);

HI_S32 COMMON_Client_AO_Close(HI_HANDLE hAoHdl);

HI_S32 COMMON_Client_Vb_Alloc(HI_U32* pPoolId, HI_U32 u32FrameSize, HI_U32 u32FrameCnt);

HI_S32 COMMON_Client_Vb_Free(HI_U32 poolid);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif/*__HI_MW_PLAYER_SVR_IPCMSG_H__*/
