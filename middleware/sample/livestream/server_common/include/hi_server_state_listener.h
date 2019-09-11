/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_server_state_listener.h
* @brief     livestream common  module state listener header file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_SERVER_STATE_LISTENER_H__
#define __HI_SERVER_STATE_LISTENER_H__

#include "hi_mw_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


/** \addtogroup     LIVESTREAM */
/** @{ */  /** <!-- [LIVESTREAM] */
#define MODULE_NAME_LIVESTREAM           "LIVESTREAM"

/**
 * listener for RTSPServer , reporting server running
 * error event, client connect and disconnect event.
 */
typedef struct  hi_SERVER_STATE_LISTENER_S
{
    /**
     * @brief report of server internal error.
     * @param[in] pstListener :listener obj
     * @param[in] s32ErrId : server error id
     * @param[in] pchMsg : string server error message info
     */
    HI_VOID (*onServerError)(struct hi_SERVER_STATE_LISTENER_S* pstListener, HI_S32 s32ErrId, HI_CHAR* pchMsg);

    /**
     * @brief reporting client connected.
     * @param[in] pstListener : listener obj
     * @param[in] pchIp : string connected client ipaddress
     */
    HI_VOID (*onClientConnect)(struct hi_SERVER_STATE_LISTENER_S* pstListener, HI_CHAR* pchIp);

    /**
     * @brief reporting client disconnected.
     * @param[in] pstListener : listener obj
     * @param[in] pchIp : string disconnected client ipaddress
     */
    HI_VOID (*onClientDisconnect)(struct hi_SERVER_STATE_LISTENER_S* pstListener, HI_CHAR* pchIp);

    /** handle for statelistener, do not change, used internal*/
    HI_MW_PTR handle;
} HI_SERVER_STATE_LISTENER_S;


/** @}*/  /** <!-- ==== LIVESTREAM End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__HI_SERVER_STATE_LISTENER_H__*/
