/**
 * @file      hi_product_scene_ini2bin.c
 * @brief     ini2bin implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_product_scene_iniparam.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/** param config file path */
#define PARAM_PATH         "./sceneparam.bin"

/** product param */
static PDT_SCENE_PARAM_S s_stSCENEPARAMCfg;

HI_S32 main()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    /* Load Scene Ini Configure */
    s32Ret = HI_CONFACCESS_Init(PDT_SCENE_INIPARAM, PDT_SCENE_INIPARAM_PATH);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, PDT_SCENE_INIPARAM_PATH);

    memset(&s_stSCENEPARAMCfg, 0, sizeof(s_stSCENEPARAMCfg));


    /* Load SceneParam Configure */
    s32Ret = HI_PDT_SCENE_LoadSceneParam(&s_stSCENEPARAMCfg);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SceneParam");

    s32Ret = HI_CONFACCESS_Deinit(PDT_SCENE_INIPARAM);
    PDT_SCENE_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "ConfAccess Deinit");

    /* Save System Param */
    FILE *fp = NULL;
    fp = fopen(PARAM_PATH, "w+b");
    if(!fp)
    {
        printf("open [%s] failed\n", PARAM_PATH);
        return HI_FAILURE;
    }
    u32Count = fwrite(&s_stSCENEPARAMCfg, sizeof(s_stSCENEPARAMCfg), 1, fp);
    if(u32Count != 1)
    {
        printf("fwrite : total %zd , write %d\n", sizeof(s_stSCENEPARAMCfg), u32Count);
    }
    fclose(fp);
    fp = NULL;
    printf("parambin update end\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

