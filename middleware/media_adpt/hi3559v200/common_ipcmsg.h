#ifndef __COMMON_PLAYER_SVR_IPCMSG_H__
#define __COMMON_PLAYER_SVR_IPCMSG_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IPCMSG_COMMON_MODID (0x8002)
#define IPCMSG_COMMON "PrivateIPCMsg"
#define IPMSG_COMMON_PORT (167)

typedef struct tagCOMMON_VO_OPEN_REQ_MSG_S
{
    HI_HANDLE hVpssGrpHdl;
    HI_HANDLE hVpssChnHdl;

    HI_HANDLE hDispHdl;
    HI_HANDLE hWndHdl;
    HI_U32 u32Width;
    HI_U32 u32Height;
}COMMON_VO_OPEN_REQ_MSG_S;

typedef struct tagCOMMON_VO_CLOSE_REQ_MSG_S
{
    HI_HANDLE hVpssGrpHdl;
    HI_HANDLE hVpssChnHdl;

    HI_HANDLE hDispHdl;
    HI_HANDLE hWndHdl;
}COMMON_VO_CLOSE_REQ_MSG_S;

typedef struct tagCOMMON_AO_OPEN_REQ_MSG_S
{
    HI_HANDLE hAoHdl;
    HI_S32 s32SampleRate;
    HI_U32 u32ChnCnt;
}COMMON_AO_OPEN_REQ_MSG_S;

typedef struct tagCOMMON_AO_CLOSE_REQ_MSG_S
{
    HI_HANDLE hAoHdl;
}COMMON_AO_CLOSE_REQ_MSG_S;

typedef struct tagCOMMON_VB_ALLOC_REQ_MSG_S
{
    HI_U32 u32FrameBufSize;
    HI_U32 u32FrameBufCnt;
}COMMON_VB_ALLOC_REQ_MSG_S;


typedef struct tagCOMMON_VB_ALLOC_REPLY_MSG_S
{
    HI_U32 u32VbPoolId;
}COMMON_VB_ALLOC_REPLY_MSG_S;


typedef struct tagCOMMON_VB_FREE_REQ_MSG_S
{
    HI_U32 u32VbPoolId;
}COMMON_VB_FREE_REQ_MSG_S;

#define COMMON_CLIENT_MSG_BASE (3000)

typedef enum tagCOMMON_CLIENT_MSG_TYPE_E
{
    COMMON_MSG_CLIENT_VO_OPEN_REQ = COMMON_CLIENT_MSG_BASE,
    COMMON_MSG_CLIENT_VO_CLOSE_REQ,
    COMMON_MSG_CLIENT_AO_OPEN_REQ,
    COMMON_MSG_CLIENT_AO_CLOSE_REQ,
    COMMON_MSG_CLIENT_VB_ALLOC_REQ,
    COMMON_MSG_CLIENT_VB_FREE_REQ
}COMMON_CLIENT_MSG_TYPE_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif/*__HI_MW_PLAYER_SVR_IPCMSG_H__*/
