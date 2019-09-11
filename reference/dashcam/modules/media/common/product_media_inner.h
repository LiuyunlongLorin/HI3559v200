/**
 * @file      product_media_inner.h
 * @brief     product media osd inferface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 * @version   1.0

 */

#ifndef __PRODUCT_MEDIA_INNER_H__
#define __PRODUCT_MEDIA_INNER_H__

#include "hi_product_media.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_PDT_MEDIA_CFG_S* PDT_MEDIA_GetMediaCfg(HI_VOID);


/* ++++++++++++++++++++++++ OSD ++++++++++++++++++++++++ */
/**
 * @brief     init osd module
 * @param[in] pstOsdCfg : osd configure
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 */
HI_S32 PDT_MEDIA_InitOsd(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg);/* osd init and Set osd bitmap size */

/**
* @brief    start osd,include time and logo
* @param[in] pstOsdCfg : osd configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_MEDIA_StartOsd(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg);

/**
* @brief    stop osd,include time and logo
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_MEDIA_StopOsd(HI_VOID);

/**
* @brief    start ispinfo osd for pq debug
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_MEDIA_StartInfoOsd(HI_VOID);

/**
* @brief    stop ispinfo osd
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_MEDIA_StopInfoOsd(HI_VOID);

/**
* @brief    osd deinit
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2018/3/24
*/
HI_S32 PDT_MEDIA_DeinitOsd(HI_VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __PRODUCT_MEDIA_INNER_H__ */


