/**
 * @file      hi_product_param.c
 * @brief     product parameter module interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <sys/mount.h>

#include "hi_eventhub.h"
#include "hi_product_param_inner.h"
#include "hi_product_param.h"
#include "hi_flash.h"
#include "hi_cycle.h"
#include "hi_appcomm_util.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#ifdef CFG_RAW_PARAM_ON

/**------------------- Raw Param Configure  ------------------------- */

#define PDT_PARAM_SIZE  (0x10000)

/** param partition name */
#define PDT_PARAM_MTD_DEVNAME     "rawparam"
#define PDT_PARAM_BAK_MTD_DEVNAME "rawparambak"

/** param partition start addr */
#define PDT_PARAM_FLASH_ADDR     (0x100000)
#define PDT_PARAM_BAK_FLASH_ADDR (0x110000)

#define PDT_DEFPARAM_PATH "/app/param/paramdef.bin"

/** param partition information */
typedef struct hiPDT_PARAM_PARTITION_S {
    HI_FLASH_TYPE_E enFlashType;
    HI_U32   u32ParamSize;
    HI_CHAR *pcPartition;
    HI_CHAR *pcPartitionBackup;
    HI_U32   u32Compress;
} HI_PDT_PARAM_PARTITION_S;
static HI_PDT_PARAM_PARTITION_S s_stPARAMPartInfo;

/** param flash handle */
static HI_VOID *s_pvPARAMFlashHdl = NULL;

#else

/**------------------- Rootfs Param Configure  ------------------------- */

/** parameter data rootfs path */
#define PDT_PARAM_PATH    "/app/param/param.bin"
#define PDT_DEFPARAM_PATH "/app/param/paramdef.bin"

#endif

/**--------------------- Common  Configure  ------------------------- */

/** parameter context */
static PDT_PARAM_CONTEXT_S s_stPARAMCtx = {
    .bInit = HI_FALSE,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .pstCfg = NULL
};

/** param save task context */
typedef struct tagPDT_PARAM_SAVETSK_CTX_S {
    HI_BOOL bRun;              /**<save task running flag */
    pthread_t tskId;           /**<save task threadId */
    pthread_mutex_t mutexLock; /**<save task mutex, protect save process */
    HI_BOOL bSaveFlg;          /**<param save flag */
} PDT_PARAM_SAVETSK_CTX_S;
static PDT_PARAM_SAVETSK_CTX_S s_stPARAMSaveTskCtx = {
    .bRun = HI_FALSE,
    .tskId = -1,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .bSaveFlg = HI_FALSE
};

/**------------------- Common Configure End  ------------------------- */

#ifdef CFG_RAW_PARAM_ON

/**------------------- Raw Param Interface  ------------------------- */

static HI_S32 PDT_PARAM_PartInfoInit(HI_PDT_PARAM_PARTITION_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);
    memset(pstParam, 0, sizeof(HI_PDT_PARAM_PARTITION_S));
#if defined (CFG_SPI_NAND_FLASH) || defined (CFG_NAND_FLASH)
    pstParam->enFlashType = HI_FLASH_TYPE_NAND_0;
#elif defined (CFG_EMMC_FLASH)
    pstParam->enFlashType = HI_FLASH_TYPE_EMMC_0;
#else
    pstParam->enFlashType = HI_FLASH_TYPE_SPI_0;
#endif
    pstParam->u32ParamSize = PDT_PARAM_SIZE;
    pstParam->pcPartition = PDT_PARAM_MTD_DEVNAME;
    pstParam->pcPartitionBackup = PDT_PARAM_BAK_MTD_DEVNAME;
    pstParam->u32Compress = PDT_PARAM_COMPRESS;
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_OpenFlash(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (!s_pvPARAMFlashHdl) {
        /* Cycle Init */
        HI_CYCLE_INIT_S stCycleInit;
        stCycleInit.enFlashType = s_stPARAMPartInfo.enFlashType;
        s32Ret = HI_CYCLE_Init(&stCycleInit);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        /* Cycle Open */
        HI_CYCLE_OPEN_S stCycleOpen;
#ifndef __HuaweiLite__
        stCycleOpen.pcPartition = s_stPARAMPartInfo.pcPartition;
        stCycleOpen.pcPartitionBackup = s_stPARAMPartInfo.pcPartitionBackup;
#else
        stCycleOpen.u64Addr = PDT_PARAM_FLASH_ADDR;
        stCycleOpen.u64BackupAddr = PDT_PARAM_BAK_FLASH_ADDR;
#endif
        stCycleOpen.u32CycleFlashSize = s_stPARAMPartInfo.u32ParamSize;
        stCycleOpen.u32Compress = s_stPARAMPartInfo.u32Compress;
        s32Ret = HI_CYCLE_Open(&s_pvPARAMFlashHdl, &stCycleOpen);
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SaveParam(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = PDT_PARAM_OpenFlash();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_CYCLE_Write(s_pvPARAMFlashHdl, (HI_U8 *)s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_LoadDefault(HI_VOID)
{
    /* Clear Current Param Data */
    HI_S32 s32Ret = PDT_PARAM_OpenFlash();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_CYCLE_Clear(s_pvPARAMFlashHdl);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    /* Load Default Param Data */
    HI_S32 s32Fd = open(PDT_DEFPARAM_PATH, O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "PDT_DEFPARAM_PATH);
    ssize_t readCount = read(s32Fd, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    close(s32Fd);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(0 < readCount, HI_FAILURE, "Read "PDT_DEFPARAM_PATH);
    /* Check Default Param Data */
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stPARAMCtx.pstCfg->stHead.u32MagicStart == PDT_PARAM_MAGIC_START
                                       && s_stPARAMCtx.pstCfg->u32MagicEnd == PDT_PARAM_MAGIC_END, HI_FAILURE, "LoadDef");
    return HI_SUCCESS;
}

#else

/**------------------- Rootfs Param Specified Interface  ------------------------- */

static HI_S32 PDT_PARAM_LoadParam(HI_VOID)
{
    /* Load Current Param Data */
    HI_S32 s32Fd = open(PDT_PARAM_PATH, O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "PDT_PARAM_PATH);
    ssize_t readCount = read(s32Fd, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    close(s32Fd);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(0 < readCount, HI_FAILURE, "Read "PDT_PARAM_PATH);
    /* Check Current Param Data */
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stPARAMCtx.pstCfg->stHead.u32MagicStart == PDT_PARAM_MAGIC_START
                                       && s_stPARAMCtx.pstCfg->u32MagicEnd == PDT_PARAM_MAGIC_END, HI_FAILURE, "LoadParam");
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_SaveParam(HI_VOID)
{
    HI_S32 s32Fd = open(PDT_PARAM_PATH, O_CREAT | O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "PDT_PARAM_PATH);
    HI_S32 s32Ret = write(s32Fd, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    if (s32Ret < 0) {
        MLOGE("Failed\n");
        close(s32Fd);
        return HI_FAILURE;
    }
    fsync(s32Fd);
    close(s32Fd);
    return HI_SUCCESS;
}

static HI_S32 PDT_PARAM_LoadDefault(HI_VOID)
{
    /* Load Default Param Data */
    HI_S32 s32Fd = open(PDT_DEFPARAM_PATH, O_RDWR, 777);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s32Fd >= 0, HI_FAILURE, "Open "PDT_DEFPARAM_PATH);
    ssize_t readCount = read(s32Fd, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    close(s32Fd);
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(0 < readCount, HI_FAILURE, "Read "PDT_DEFPARAM_PATH);
    /* Check Default Param Data */
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stPARAMCtx.pstCfg->stHead.u32MagicStart == PDT_PARAM_MAGIC_START
                                       && s_stPARAMCtx.pstCfg->u32MagicEnd == PDT_PARAM_MAGIC_END, HI_FAILURE, "LoadDef");
    return HI_SUCCESS;
}

#endif

/**------------------- Common Interface  ------------------------- */

static HI_S32 PDT_PARAM_Save2Flash(HI_VOID)
{
    static PDT_PARAM_CFG_S s_stParam;
    if (memcmp(&s_stParam, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S)) == 0) {
        return HI_SUCCESS;
    } else {
        memcpy(&s_stParam, s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    }
    return PDT_PARAM_SaveParam();
}

static HI_S32 PDT_PARAM_SetDefault(HI_VOID)
{
    HI_S32 s32Ret = PDT_PARAM_LoadDefault();
    HI_APPCOMM_CHECK_EXPR(HI_SUCCESS == s32Ret, HI_FAILURE);
    MLOGI("Load Default Success\n");
    return PDT_PARAM_SaveParam();
}

static HI_VOID *PDT_PARAM_SaveTsk(HI_VOID *pParam)
{
    prctl(PR_SET_NAME, (unsigned long)"HiParamSave", 0, 0, 0);
    PDT_PARAM_SAVETSK_CTX_S *pstTskCtx = (PDT_PARAM_SAVETSK_CTX_S *)pParam;
    while (pstTskCtx->bRun) {
        HI_usleep(1 * 1000 * 1000);
        if (HI_TRUE == pstTskCtx->bSaveFlg) {
            pstTskCtx->bSaveFlg = HI_FALSE;
            HI_MUTEX_LOCK(pstTskCtx->mutexLock);
            PDT_PARAM_Save2Flash();
            HI_MUTEX_UNLOCK(pstTskCtx->mutexLock);
        }
    }
    HI_MUTEX_LOCK(pstTskCtx->mutexLock);
    PDT_PARAM_Save2Flash();
    HI_MUTEX_UNLOCK(pstTskCtx->mutexLock);
    return NULL;
}

HI_VOID PDT_PARAM_SetSaveFlg(HI_VOID)
{
    s_stPARAMSaveTskCtx.bSaveFlg = HI_TRUE;
}

inline PDT_PARAM_CONTEXT_S *PDT_PARAM_GetCtx(HI_VOID)
{
    return &s_stPARAMCtx;
}

HI_S32 HI_PDT_PARAM_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_EVTHUB_Register(HI_EVENT_PDT_PARAM_CHANGE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_Init(HI_VOID)
{
    /* Check Module Init Status */
    if (s_stPARAMCtx.bInit) {
        MLOGD("has already init\n");
        return HI_SUCCESS;
    }
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef CFG_RAW_PARAM_ON
    /* Init Partition Information */
    PDT_PARAM_PartInfoInit(&s_stPARAMPartInfo);
#endif
    MLOGD("Param Phy Address[%#x] Len[%#lx\n", PDT_PARAM_PHY_ADDR, (HI_UL)sizeof(PDT_PARAM_CFG_S));
    /* MemMap Param Physical Address */
#ifdef AMP_HUAWEILITE
    s32Ret = PDT_PARAM_OpenFlash();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_stPARAMCtx.pstCfg = (PDT_PARAM_CFG_S *)PDT_PARAM_PHY_ADDR;
    s32Ret = HI_CYCLE_Read(s_pvPARAMFlashHdl, (HI_U8 *)s_stPARAMCtx.pstCfg, sizeof(PDT_PARAM_CFG_S));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#else
    s_stPARAMCtx.pstCfg = (PDT_PARAM_CFG_S *)HI_MemMap(PDT_PARAM_PHY_ADDR, sizeof(PDT_PARAM_CFG_S));
    HI_APPCOMM_CHECK_EXPR_WITH_ERRINFO(s_stPARAMCtx.pstCfg, HI_FAILURE, "MemMap");
#endif
#ifndef CFG_RAW_PARAM_ON
    /* Load Param Data */
    PDT_PARAM_LoadParam();
#endif
    /* Check Head and Tail Data */
    MLOGD("Param Size[%lu]\n", (HI_UL)sizeof(PDT_PARAM_CFG_S));
    if ((s_stPARAMCtx.pstCfg->stHead.u32MagicStart) != PDT_PARAM_MAGIC_START
            || (s_stPARAMCtx.pstCfg->u32MagicEnd != PDT_PARAM_MAGIC_END)) {
        MLOGE("\n Error!!! param can't resume!!! Please check the param data\n");
        s32Ret = HI_PDT_PARAM_SetDefault();
        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    }
    /* Create SaveParam Task */
    s_stPARAMSaveTskCtx.bRun = HI_TRUE;
    s32Ret = pthread_create(&s_stPARAMSaveTskCtx.tskId, NULL, PDT_PARAM_SaveTsk, &s_stPARAMSaveTskCtx);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "CreateSaveTsk");
    MLOGD("Param SaveTsk(%lu) create successful\n", s_stPARAMSaveTskCtx.tskId);
    s_stPARAMCtx.bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_Deinit(HI_VOID)
{
    if (HI_FALSE == s_stPARAMCtx.bInit) {
        MLOGD("has already deinit or not init yet\n");
        return HI_SUCCESS;
    }
    /* Destroy SaveParam Task */
    s_stPARAMSaveTskCtx.bRun = HI_FALSE;
    pthread_join(s_stPARAMSaveTskCtx.tskId, NULL);
    s_stPARAMSaveTskCtx.tskId = -1;
    s_stPARAMSaveTskCtx.bSaveFlg = HI_FALSE;
#ifdef CFG_RAW_PARAM_ON
    if (s_pvPARAMFlashHdl) {
        HI_CYCLE_Close(s_pvPARAMFlashHdl);
        s_pvPARAMFlashHdl = NULL;
        HI_CYCLE_Deinit();
    }
#endif
    s_stPARAMCtx.bInit = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 HI_PDT_PARAM_SetDefault(HI_VOID)
{
    HI_MUTEX_LOCK(s_stPARAMSaveTskCtx.mutexLock);
    HI_S32 s32Ret = PDT_PARAM_SetDefault();
    HI_MUTEX_UNLOCK(s_stPARAMSaveTskCtx.mutexLock);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

