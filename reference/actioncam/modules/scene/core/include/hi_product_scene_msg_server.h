/**
* @file    hi_scene_slave.h
* @brief   scene module struct and interface declaration
*
* Copyright (c) 2012 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date       2017/12/13
* @version   1.0

*/

#ifndef _MSG_SCENE_SERVER_H_
#define _MSG_SCENE_SERVER_H_

#include "hi_product_scene.h"
#include "hi_product_scene_msg_server.h"
#include "hi_appcomm_msg.h"
#include "hi_appcomm_msg_server.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE */
/** @{ */  /** <!-- [SCENE] */


/**
 * @brief         Init SceneMsg Service
 * @return       0 success, non-zero error code.
 * @exception  None
 * @author      HiMobileCam Reference Develop Team
 * @date         2017/12/11
 */
HI_S32 HI_PDT_SCENE_MSG_SERVER_Init(HI_VOID);

/** @}*/  /** <!-- ==== SCENE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* End of #ifndef _MSG_SCENE_SERVER_H_ */
