/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_errno.h
 * @brief   errno module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_MAPI_ERRNO_H__
#define __HI_MAPI_ERRNO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     COMM */
/** @{ */ /** <!-- [COMM] */

#define HI_MAPI_ERR_APPID (0x80000000L + 0x23000000L)

typedef enum hiMAPI_ERR_LEVEL_E {
    MAPI_EN_ERR_LEVEL_ERROR = 4, /* error conditions                             */
    MAPI_EN_ERR_LEVEL_BUTT
} MAPI_ERR_LEVEL_E;

/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define HI_MAPI_DEF_ERR(module, level, errid) \
    ((HI_S32)((HI_MAPI_ERR_APPID) | ((module) << 16) | ((level) << 13) | (errid)))

/* NOTE! the following defined all common error code,
** all module must reserved 0~63 for their common error code
*/
typedef enum hiMAPI_EN_ERR_CODE_E {
    MAPI_EN_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    MAPI_EN_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    MAPI_EN_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                                                        * eg, an illegal enumeration value             */
    MAPI_EN_ERR_EXIST = 4,         /* resource exists                              */
    MAPI_EN_ERR_UNEXIST = 5,       /* resource unexists                            */

    MAPI_EN_ERR_NULL_PTR = 6, /* using a NULL point                           */

    MAPI_EN_ERR_NOT_CONFIG = 7, /* try to enable or initialize system, device
                                                  ** or channel, before configing attribute       */

    MAPI_EN_ERR_NOT_SUPPORT = 8, /* operation or type is not supported by NOW    */
    MAPI_EN_ERR_NOT_PERM = 9,    /* operation is not permitted
                                                    ** eg, try to change static attribute           */

    MAPI_EN_ERR_NOMEM = 12, /* failure caused by malloc memory              */
    MAPI_EN_ERR_NOBUF = 13, /* failure caused by malloc buffer              */

    MAPI_EN_ERR_BUF_EMPTY = 14, /* no data in buffer                            */
    MAPI_EN_ERR_BUF_FULL = 15, /* no buffer for new data                       */

    MAPI_EN_ERR_SYS_NOTREADY = 16, /* System is not ready,maybe not initialed or
                                                       ** loaded. Returning the error code when opening
                                                       ** a device file failed.                        */

    MAPI_EN_ERR_BADADDR = 17, /* bad address,
                                               ** eg. used for copy_from_user & copy_to_user   */

    MAPI_EN_ERR_BUSY = 18, /* resource is busy,
                                           ** eg. destroy a venc chn without unregister it */
    MAPI_EN_ERR_INVALID_FD = 19, /* fd is invlalid
                                                    ** eg. open a file or dev fail */
    MAPI_EN_ERR_ILLEGAL_HANDLE = 20, /* HANDLE id is invlalid
                                                          ** eg. pipe id over range */
    MAPI_EN_ERR_NOT_INITED = 21, /* sys or Module not init yet operation failed
                                                    ** eg. vcap not init */
    MAPI_EN_ERR_OPERATE_FAIL = 22, /* sys or Module operate fail
                                                       ** eg. start vcap dev fail */
    MAPI_EN_ERR_TIME_OUT = 23,    /* sys or Module operate timeout
                                                     ** eg. vcap dump raw time out */

    MAPI_EN_ERR_BUTT = 63, /* maxium code, private error code of all modules
                                          ** must be greater than it                      */
} MAPI_EN_ERR_CODE_E;

/** @} */ /** <!-- ==== COMM End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MAPI_ERRNO_H__ */
