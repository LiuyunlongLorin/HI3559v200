/**
 * @file      hi_upgrade.h
 * @brief     upgrade interface defination
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 * @version   1.0

 */


#ifndef __HI_UPGRADE_H__
#define __HI_UPGRADE_H__

#include "hi_appcomm.h"
#include "hi_upgrade_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     UPGRADE */
/** @{ */  /** <!-- [UPGRADE] */

/** error code define */
#define HI_UPGRADE_EINVAL         HI_APPCOMM_ERR_ID(HI_APP_MOD_UPGRADE, HI_EINVAL)       /**<Invalid argument */
#define HI_UPGRADE_ENOTINIT       HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_ENOINIT)        /**<Not inited */
#define HI_UPGRADE_EINITIALIZED   HI_APPCOMM_ERR_ID(HI_APP_MOD_MEDIA, HI_EINITIALIZED)   /**<re-initialized */
#define HI_UPGRADE_ELOST          HI_APPCOMM_ERR_ID(HI_APP_MOD_UPGRADE, HI_ELOST)        /**<Resource not exist*/
#define HI_UPGRADE_EINTR          HI_APPCOMM_ERR_ID(HI_APP_MOD_UPGRADE, HI_EINTR)
#define HI_UPGRADE_EPKT_INVALID   HI_APPCOMM_ERR_ID(HI_APP_MOD_UPGRADE, HI_ERRNO_CUSTOM_BOTTOM)

/** return value define */
#define HI_UPGRADE_PKT_AVAILABLE HI_APPCOMM_ERR_ID(HI_APP_MOD_UPGRADE, HI_ERRNO_CUSTOM_BOTTOM+1)

/** event id define */
#define HI_EVENT_UPGRADE_NEWPKT   HI_APPCOMM_EVENT_ID(HI_APP_MOD_UPGRADE, 1)
#define HI_EVENT_UPGRADE_SUCCESS  HI_APPCOMM_EVENT_ID(HI_APP_MOD_UPGRADE, 2)
#define HI_EVENT_UPGRADE_FAILURE  HI_APPCOMM_EVENT_ID(HI_APP_MOD_UPGRADE, 3)


/**
 * @brief     register event
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 */
HI_S32 HI_UPGRADE_RegisterEvent(HI_VOID);

/**
 * @brief     module init and check upgrade status
 *            if upgrade status finish, publish event HI_EVENT_UPGRADE_SUCCESS
 *            if upgrade status processing, publish event HI_EVENT_UPGRADE_FAILURE
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/8/13
 */
HI_S32 HI_UPGRADE_Init(HI_VOID);

/**
 * @brief     module deinit
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/8/13
 */
HI_S32 HI_UPGRADE_Deinit(HI_VOID);

/**
 * @brief     check specified packet
 *            if packet valid, publish event HI_EVENT_UPGRADE_NEWPKT
 * @param[in] pszPktUrl : check packet url
 * @param[in] pstDevInfo : device information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/8/13
 */
HI_S32 HI_UPGRADE_CheckPkt(const HI_CHAR* pszPktUrl, const HI_UPGRADE_DEV_INFO_S* pstDevInfo);

/**
 * @brief     check new packet exist or not in specified pktpath
 *            if new valid packet detected, publish event HI_EVENT_UPGRADE_NEWPKT
 * @param[in] pszPktPath : check packet path
 * @param[in] pstDevInfo : device information
 * @return    0 success, 1 new packet, <0 error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 */
HI_S32 HI_UPGRADE_SrchNewPkt(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo);

/**
 * @brief     exec upgrade action
 * @param[in] pszPktPath : output pkt, eg. sd root directory
 * @param[in] pszPktUrl : upgrade packet url
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 */
HI_S32 HI_UPGRADE_DoUpgrade(const HI_CHAR* pszPktPath, const HI_CHAR* pszPktUrl);

/**
 * @brief     delete upgrade file
 * @param[in] pszPktPath : pkt path, eg. sd root directory
 * @param[in] pstDevInfo : device information
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 */
HI_S32 HI_UPGRADE_DelUpgradeFiles(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo);

/** @}*/  /** <!-- ==== UPGRADE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_UPGRADE_DEFINE_H__ */

