/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_listener.h
* @brief     livestream common network listener head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_NETWORK_LISTENER_H__
#define __HI_NETWORK_LISTENER_H__
#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define NETLISTEN_TIMOUT_SEC  (0)
#define NETLISTEN_TIMOUT_USEC  (10000)
#define MAX_RECV_BUFFER (1024)
#define DEFAULT_MAX_FDNUM (32)
#define INVALID_SOCKETFD (-1)

typedef HI_S32 (*onClientConnection)(HI_MW_PTR obj, HI_S32 s32LinkFd, HI_CHAR* pMsgBuff, HI_U32 u32MsgLen);

HI_S32 HI_NETWORK_Listener_Create(HI_S32 s32ListenPort,  HI_MW_PTR* pHandle);

HI_S32 HI_NETWORK_Listener_Destroy(HI_MW_PTR handle);

HI_S32 HI_NETWORK_Listener_Start(HI_MW_PTR handle);

HI_S32 HI_NETWORK_Listener_Stop(HI_MW_PTR handle);

HI_S32 HI_NETWORK_Listener_RegisterCallback(HI_MW_PTR handle, onClientConnection onConnection, HI_MW_PTR obj);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_NETWORK_LISTENER_H__*/
