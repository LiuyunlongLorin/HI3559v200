/**
 * @file      product_media_msg.h
 * @brief     product media osd msg inferface declaration
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/4/09
 * @version   1.0

 */

#ifndef __PRODUCT_MEDIA_MSG_H__
#define __PRODUCT_MEDIA_MSG_H__

#include "hi_appcomm_msg.h"
#include "hi_appcomm_msg_server.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#if defined(AMP_LINUX_HUAWEILITE)

typedef enum hiMSG_MEDIA_E
{
    HI_MSG_MEDIA_SETCROP = HI_APPCOMM_MSG_ID(HI_APP_MOD_MEDIA, 0),   /**<set crop msg*/
    HI_MSG_MEDIA_INFO_OSD_START,
    HI_MSG_MEDIA_INFO_OSD_STOP,
    HI_MSG_MEDIA_BUIT
} HI_MSG_MEDIA_E;

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __PRODUCT_MEDIA_MSG_H__ */

