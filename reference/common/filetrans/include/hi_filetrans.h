/**
 * @file    hi_filetrans.h
 * @brief   filetrans module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/26
 * @version   1.0

 */
#ifndef __HI_FILETRANS_H__
#define __HI_FILETRANS_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     FILETRANS */
/** @{ */  /** <!-- [FILETRANS] */

/**
 * @brief    init filetrans module.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_Init(HI_VOID);

/**
 * @brief    deinit filetrans module.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_Deinit(HI_VOID);

/**
 * @brief    parse file path, and transmit the file with http protocol.
 * @param[in] s32Socket:http socket.
 * @param[in] pszRequest:http requset packet.
 * @param[in] u32ReqLen:http requset packet length.
 * @return 0 success,non-zero error code.
 * @exception    register this function to webserver and take over the socket.
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_DistribLink(HI_S32 s32Socket, HI_CHAR* pszRequest, HI_U32 u32ReqLen);

/**
 * @brief    close all linked socket.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_CloseAllLink(HI_VOID);

/**
 * @brief    get link number.
 * @return   link number.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/25
 */
HI_S32 HI_FILETRANS_GetLinkNum(HI_VOID);

/** @}*/  /** <!-- ==== FILETRANS End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif/*__HI_FILETRANS_EXT_H__*/
