/**
 * @file      hi_product_scene_iniparam.h
 * @brief     scene iniparam struct and interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/4/3
 * @version   1.0

 */

#ifndef __HI_PRODUCT_SCENE_INIPARAM_H__
#define __HI_PRODUCT_SCENE_INIPARAM_H__

#include "hi_appcomm.h"
#include "hi_confaccess.h"
#include "hi_product_scene.h"
#include "product_scene_inner.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCENE_INIPARAM */
/** @{ */  /** <!-- [SCENE_INIPARAM] */

/** product ini identification */
#define PDT_SCENE_INIPARAM  "sceneini"

/** ini configure access totality information file */
#define PDT_SCENE_INIPARAM_PATH "./config_cfgaccess_entry.ini"

/** ini configure module: scenemode */
#define PDT_SCENE_INI_SCENEMODE "scene_param_"
#define PDT_SCENE_INI_SCENESNAP "scene_snap"
#define PDT_SCENE_INI_SCENEEXPARAM "scene_exparam"


/** ini module name maximum length */
#define PDT_SCENE_INIPARAM_MODULE_NAME_LEN (64)

/** ini node name maximum length */
#define PDT_SCENE_INIPARAM_NODE_NAME_LEN   (128)

/** Load IniNode Result Check */
#define PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(ret, name) \
    do{ \
        if (HI_SUCCESS != ret){   \
            MLOGE(" Load [%s] failed\n", name); \
            return HI_FAILURE;  \
        }   \
    }while(0)


/**
 * @brief         Load Scene Param
 * @param[in]  pstSceneParam: Scene Param
 * @return       0 success, non-zero error code.
 * @exception  None
 * @author      HiMobileCam Reference Develop Team
 * @date         2017/12/11
 */
HI_S32 HI_PDT_SCENE_LoadSceneParam(PDT_SCENE_PARAM_S* pstSceneParam);


/** @}*/  /** <!-- ==== SCENE_INIPARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __HI_PRODUCT_SCENE_INIPARAM_H__ */
