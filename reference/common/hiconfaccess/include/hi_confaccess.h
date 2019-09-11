/**
 * @file      hi_confaccess.h
 * @brief     The external header of libconfaccess, support manipulation to ini config file
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 * @version   1.0

 */

#ifndef __HI_CONFACCESS_H__
#define __HI_CONFACCESS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/** \addtogroup     CONFACCESS */
/** @{ */  /** <!-- [CONFACCESS] */

/** Length Define */
#define HI_CONFACCESS_PATH_MAX_LEN     HI_APPCOMM_MAX_PATH_LEN
#define HI_CONFACCESS_NAME_MAX_LEN     HI_APPCOMM_COMM_STR_LEN
#define HI_CONFACCESS_KEY_MAX_LEN      (128)

/** Error Define */
#define HI_CONFACCESS_EINVAL          HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 0)/**<Invalid argument */
#define HI_CONFACCESS_ENOTINIT        HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 1)/**<Not inited */
#define HI_CONFACCESS_EREINIT         HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 2)/**<re init */
#define HI_CONFACCESS_EMALLOC         HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 3)/**<malloc failed */
#define HI_CONFACCESS_ECFG_NOTEXIST   HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 4)/**<cfg not exist */
#define HI_CONFACCESS_EMOD_NOTEXIST   HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 5)/**<module not exist */
#define HI_CONFACCESS_EITEM_NOTEXIST  HI_APPCOMM_ERR_ID(HI_APP_MOD_CONFACCESS, 6)/**<confitem not exist */


/**
 * @brief     Load a config, includes of the common config and all the submode config
 * @param[in] pszCfgName : The config name to be load
 * @param[in] pszCfgPath : The path of config
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_Init(const HI_CHAR* pszCfgName,const HI_CHAR*  pszCfgPath);

/**
 * @brief     Deinit all source of a config
 * @param[in] pszCfgName : The config name
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_Deinit(const HI_CHAR* pszCfgName);

/**
 * @brief     get string conf item of certain module, if not found the default willbe given
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[in] pszDefault : default value if item not found
 * @param[out]ppszValue : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_GetString(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR* pszConfItem, HI_CHAR* pszDefault, HI_CHAR** const ppszValue);

/**
 * @brief     get string conf item of certain module
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[out]ppszValue : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_GetStr(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR*  pszConfItem, HI_CHAR** ppszValue);

/**
 * @brief     get int conf item of certain module
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[in] s32Notfound : default value if item not found
 * @param[out]ps32Value : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_GetInt(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR* pszConfItem, HI_S32 s32Notfound, HI_S32* const ps32Value);

/**
 * @brief     get double conf item of certain module
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[in] dNotfound : default value if item not found
 * @param[out]pdValue : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_GetDouble(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR* pszConfItem, HI_DOUBLE dNotfound, HI_DOUBLE* const pdValue);

/**
 * @brief     get bool conf item of certain module
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[in] bNotfound : default value if item not found
 * @param[out]pbValue : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_GetBool(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR* pszConfItem, HI_BOOL bNotfound, HI_BOOL* pbValue);

/**
 * @brief     set string conf item of certain module
 * @param[in] pszCfgName : The config name
 * @param[in] pszModule : The module name
 * @param[in] pszConfItem : The confitem name
 * @param[in] pszValue : value
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/15
 */
HI_S32 HI_CONFACCESS_SetString(const HI_CHAR* pszCfgName, const HI_CHAR* pszModule,
            const HI_CHAR* pszConfItem, const HI_CHAR* pszValue);

/** @}*/  /** <!-- ==== CONFACCESS End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_CONFACCESS_H__ */

