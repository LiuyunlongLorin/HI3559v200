/**
 * @file    filemng_repairer.c
 * @brief   file manager repairer function.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/19
 * @version   1.0

 */
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "hi_mp4_format.h"
#include "hi_eventhub.h"

#include "hi_appcomm_util.h"
#include "filemng_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define FILEMNG_REPAIRER_MAX_BACKUP_NUM (4)
#define FILEMNG_REPAIRER_LAST_FILE_NAME ".LastFileName"/**<This file will save the last video name recorded */

static HI_BOOL s_bParseExtraMdat; /**<Parse extra Mdat or not */
static HI_CHAR s_szLastFilePrefix[HI_APPCOMM_MAX_FILENAME_LEN];
static HI_CHAR s_szLastFileName[HI_APPCOMM_MAX_PATH_LEN];

static HI_S32 REPAIRER_CheckMP4(const HI_CHAR *pszFilePath)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MW_PTR Handle;
    HI_U64 u64Duration;
    HI_MP4_CONFIG_S stMP4Cfg;
    snprintf(stMP4Cfg.aszFileName, HI_MP4_MAX_FILE_NAME, pszFilePath);
    stMP4Cfg.enConfigType = HI_MP4_CONFIG_DEMUXER;
    stMP4Cfg.stDemuxerConfig.u32VBufSize = 1 << 20;
    s32Ret = HI_MP4_Create(&Handle, &stMP4Cfg);
    if (HI_SUCCESS != s32Ret) {
        return s32Ret;
    }
    HI_MP4_FILE_INFO_S stMP4Info;
    s32Ret = HI_MP4_GetFileInfo(Handle, &stMP4Info);
    HI_MP4_Destroy(Handle, &u64Duration);
    return s32Ret;
}

HI_S32 FILEMNG_REPAIRER_Init(const HI_FILEMNG_REPAIR_CFG_S *pstRepairCfg)
{
    HI_S32 s32Ret = 0;
    HI_CHAR szTopPath[HI_APPCOMM_MAX_PATH_LEN];
    snprintf(szTopPath, HI_APPCOMM_MAX_PATH_LEN, "%s%s", pstRepairCfg->szRootPath, pstRepairCfg->szTopDirName);
    s32Ret = HI_mkdir(szTopPath, 0777);
    if (HI_SUCCESS != s32Ret) {
        return HI_FILEMNG_EINTER;
    }

    snprintf(s_szLastFileName, sizeof(s_szLastFileName), "%s/%s", szTopPath, FILEMNG_REPAIRER_LAST_FILE_NAME);
    memset(s_szLastFilePrefix, 0, HI_APPCOMM_MAX_PATH_LEN);
    s_bParseExtraMdat = pstRepairCfg->bParseExtraMdat;
    return HI_SUCCESS;
}

HI_S32 FILEMNG_REPAIRER_Backup(const HI_CHAR *pszSrcFilePath)
{
    HI_CHAR szFilePath[FILEMNG_REPAIRER_MAX_BACKUP_NUM][HI_APPCOMM_MAX_PATH_LEN];
    HI_U32 u32BackupCnt = 0;
    HI_S32 i = 0;
    HI_BOOL bCover = HI_FALSE;
    FILE *fp = fopen(s_szLastFileName, "ab+");
    if (NULL == fp) {
        MLOGE("fopen[%s] Error:%s\n", s_szLastFileName, strerror(errno));
        return HI_FILEMNG_EINTER;
    }
    fseek(fp, 0L, SEEK_SET);

    while (NULL != fgets(szFilePath[u32BackupCnt], HI_APPCOMM_MAX_PATH_LEN, fp)) {
        u32BackupCnt++;

        if (u32BackupCnt >= FILEMNG_REPAIRER_MAX_BACKUP_NUM) {
            bCover = HI_TRUE;
            fclose(fp);
            fp = NULL;

            fp = fopen(s_szLastFileName, "wb");
            if (NULL == fp) {
                MLOGE("fopen[%s] Error:%s\n", s_szLastFileName, strerror(errno));
                return HI_FILEMNG_EINTER;
            }
            break;
        }
    }

    if (bCover) {
        fseek(fp, 0L, SEEK_SET);

        for (i = 1; i < FILEMNG_REPAIRER_MAX_BACKUP_NUM; i++) {
            if (0 > fputs(szFilePath[i], fp)) {
                MLOGE("save LastFileName fail!%s \n", strerror(errno));
                fclose(fp);
                return HI_FAILURE;
            }
        }

        if (0 > fputs(pszSrcFilePath, fp)) {
            MLOGE("save LastFileName fail!%s \n", strerror(errno));
            fclose(fp);
            return HI_FAILURE;
        }

        fwrite("\n", 1, 1, fp);
    } else {
        fseek(fp, 0L, SEEK_END);

        if (0 > fputs(pszSrcFilePath, fp)) {
            MLOGE("save LastFileName fail!%s \n", strerror(errno));
            fclose(fp);
            return HI_FAILURE;
        }

        fwrite("\n", 1, 1, fp);
    }

    fflush(fp);
#ifndef __LITEOS__
    HI_S32 fd = fileno(fp);
    if (-1 != fd) {
        fsync(fd);
    }
#endif
    MLOGD("save LastFileName in %s, len %lu, filename:%s\n", s_szLastFileName, (HI_UL)strnlen(pszSrcFilePath,
            HI_APPCOMM_MAX_PATH_LEN), pszSrcFilePath);
    fclose(fp);
    return HI_SUCCESS;
}

HI_S32 FILEMNG_REPAIRER_Repair(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szFilePath[HI_APPCOMM_MAX_PATH_LEN];
    HI_BOOL bRepairFlg = HI_FALSE;
    HI_EVENT_S stEvent;

    /**get last file name */
    FILE *fp = fopen(s_szLastFileName, "rb");
    if (NULL == fp) {
        MLOGD("fopen[%s] Error:%s, won't repair any file.\n", s_szLastFileName, strerror(errno));
        return HI_SUCCESS;
    }

    while (NULL != fgets(szFilePath, HI_APPCOMM_MAX_PATH_LEN, fp)) {
        szFilePath[strnlen(szFilePath, HI_APPCOMM_MAX_PATH_LEN) - 1] = '\0';

        /**repair */
        if (0 != access(szFilePath, F_OK)) {
            MLOGW("Won't repair. No this file:%s.\n", szFilePath);
            continue;
        }

        s32Ret = REPAIRER_CheckMP4(szFilePath);
        if (HI_SUCCESS == s32Ret) {
            MLOGD("%s does not need to be repaired or it has already been repaired\n", szFilePath);
        } else {
            MLOGD("%s needs to be repaired.\n", szFilePath);
            if (HI_FALSE == bRepairFlg) {
                stEvent.EventID = HI_EVENT_FILEMNG_REPAIR_BEGIN;
                HI_EVTHUB_Publish(&stEvent);
                bRepairFlg = HI_TRUE;
            }

            s32Ret = HI_MP4_RepairFile(szFilePath, s_bParseExtraMdat);
            if (s32Ret != HI_SUCCESS) {
                stEvent.EventID = HI_EVENT_FILEMNG_REPAIR_FAILED;
                snprintf(stEvent.aszPayload, EVENT_PAYLOAD_LEN, szFilePath);
                HI_EVTHUB_Publish(&stEvent);
                HI_LOG_PrintFuncErr(HI_MP4_RepairFile, s32Ret);
                continue;
            }
            MLOGI("%s has been repaired successfully.\n", szFilePath);
        }
    }
    fclose(fp);

    if (bRepairFlg) {
        sync();
        stEvent.EventID = HI_EVENT_FILEMNG_REPAIR_END;
        HI_EVTHUB_Publish(&stEvent);
    }
    return HI_SUCCESS;
}

HI_S32 FILEMNG_REPAIRER_Deinit(HI_VOID)
{
    memset(s_szLastFilePrefix, 0, HI_APPCOMM_MAX_PATH_LEN);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

