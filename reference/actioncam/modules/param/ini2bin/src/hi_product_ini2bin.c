/**
 * @file      hi_product_ini2bin.c
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
#include "hi_product_iniparam.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/** param config file path */
#define PARAM_PATH         "./param.bin"
#define PARAMDEF_PATH   "./paramdef.bin"

/** product param */
static PDT_PARAM_CFG_S s_stPARAMCfg;


HI_S32 main()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Count = 0;

    /* Load Product Ini Configure */
    s32Ret = HI_CONFACCESS_Init(PDT_INIPARAM, PDT_INIPARAM_PATH);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, PDT_INIPARAM_PATH);

    memset(&s_stPARAMCfg, 0, sizeof(s_stPARAMCfg));

    s_stPARAMCfg.stHead.u32MagicStart = PDT_PARAM_MAGIC_START;
    s_stPARAMCfg.stHead.u32SysLen = sizeof(s_stPARAMCfg);
    s_stPARAMCfg.u32MagicEnd = PDT_PARAM_MAGIC_END;

    /* Load DevMng Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadDevMngConf(&s_stPARAMCfg.stDevMngCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DevMng Conf");

    /* Load FileMng Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadFileMngConf(&s_stPARAMCfg.stFileMngCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "FileMng Conf");

    /* Load Media Common Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadMediaCommConf(&s_stPARAMCfg.stMediaCommCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Media Common Conf");

    /* Load WorkMode Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadWorkModeConf(&s_stPARAMCfg.stWorkModeCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "WorkMode Conf");

    /* Load ValueSet Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadValueSetConf(&s_stPARAMCfg.stItemValues);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "ValueSet Conf");

    /* Load MediaMode Param Configure */
    s32Ret = HI_PDT_INIPARAM_LoadMediaModeConf(s_stPARAMCfg.astMediaModeSpecCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "VideoMode Conf");

    s32Ret = HI_CONFACCESS_Deinit(PDT_INIPARAM);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "ConfAccess Deinit");

    MLOGD("Param Size[%lu]\n",       (HI_UL)sizeof(PDT_PARAM_CFG_S));
    MLOGD("MediaComm Size[%lu]\n",   (HI_UL)sizeof(PDT_PARAM_MEDIA_COMM_CFG_S));
    MLOGD("WorkModeMedia Size[%lu]\n", (HI_UL)sizeof(PDT_PARAM_WORKMODE_TYPE_MEDIA_CFG_S));
    MLOGD("WorkModeCfg Size[%lu]\n", (HI_UL)sizeof(PDT_PARAM_WORKMODE_CFG_S));
    MLOGD("FileMngCfg Size[%lu]\n",  (HI_UL)sizeof(HI_PDT_FILEMNG_CFG_S));
    MLOGD("DevMngCfg Size[%lu]\n",   (HI_UL)sizeof(PDT_PARAM_DEVMNG_CFG_S));
    MLOGD("ValueSetCfg Size[%lu]\n", (HI_UL)sizeof(PDT_PARAM_CONFITEM_VALUESET_S));
    MLOGD("MediaMode UnitSize[%lu], Cnt[%d]\n",
        (HI_UL)sizeof(PDT_PARAM_MEDIAMODE_SPEC_CFG_S), PDT_PARAM_MEDIAMODE_CNT);

    /* Save System Param */
    FILE *fp = NULL;
    fp = fopen(PARAM_PATH, "w+b");
    if(!fp)
    {
        MLOGE("open [%s] failed\n", PARAM_PATH);
        return HI_FAILURE;
    }
    u32Count = fwrite(&s_stPARAMCfg, sizeof(s_stPARAMCfg), 1, fp);
    if(u32Count != 1)
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)sizeof(s_stPARAMCfg), u32Count);
    }
    fclose(fp);
    fp = NULL;

    /* Save System Default Param */
    fp = fopen(PARAMDEF_PATH, "w+b");
    if(!fp)
    {
        MLOGE("open [%s] failed\n", PARAMDEF_PATH);
        return HI_FAILURE;
    }
    u32Count = fwrite(&s_stPARAMCfg, sizeof(s_stPARAMCfg), 1, fp);
    if(u32Count != 1)
    {
        MLOGE("fwrite : total %lu, write %u\n", (HI_UL)sizeof(s_stPARAMCfg), u32Count);
    }
    fflush(fp);
    fclose(fp);

    MLOGD("parambin update end\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

