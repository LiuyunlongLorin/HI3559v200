
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "crc.h"
#include "hi_appcomm.h"
#include "hi_upgrade_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define COMMAND_PARAM_CNT           (4)
#define PARTITION_LIST_STR_MAX_LEN  (256)

#define UPGRADE_MAX_BUF_SIZE        (0x8000000) /**<128M */
#define UPGRADE_BLOCK_SIZE          (0x10000) /**<64K */
/** upgrade config file maximum length */
#define UPGRADE_CONFIG_MAX_LEN      (2048)

#define UPGRADE_PKT_SUFFIX    "sw"

/** mtd partition information */
typedef struct tagUPGRADE_MTDPART_INFO_S
{
    HI_S32  s32PartitionCnt;
    HI_CHAR aszPartitionName[HI_UPGRADE_MAX_PART_CNT][HI_UPGRADE_PARTITION_NAME_MAX_LEN];
} UPGRADE_MTDPART_INFO_S;

typedef struct tagUPGRADE_CONFIG_INFO_S
{
    HI_U32  u32FileLen;
    HI_CHAR szFileName[HI_UPGRADE_PARTITION_NAME_MAX_LEN];
    HI_CHAR szFileText[UPGRADE_CONFIG_MAX_LEN];
    HI_CHAR szBootArgs[HI_UPGRADE_MAX_ENV_LEN];
    HI_CHAR szBootCmd[HI_UPGRADE_MAX_ENV_LEN];
    UPGRADE_MTDPART_INFO_S stMtdPartInfo;
} UPGRADE_CONFIG_INFO_S;


static inline HI_VOID usage()
{
    printf("usage: ./release model     version configfile\n");
    printf("       ./release actioncam 1.0.0.1 config\n");
    exit(1);
}

static int GetEnvValue(const HI_CHAR* pszBuf, const HI_CHAR* pszEnvName, HI_CHAR* pszEnvValue)
{
    HI_CHAR* pszStr = NULL;
    HI_CHAR* pszEnvStart = NULL;
    HI_CHAR* pszEnvEnd = NULL;
    pszStr = strstr(pszBuf, pszEnvName);
    if (!pszStr)
    {
        MLOGE("EnvName[%s] not found!\n", pszEnvName);
        return HI_FAILURE;
    }
    pszEnvStart = strchr(pszStr, '\'');
    if (!pszEnvStart)
    {
        MLOGE("EnvName[%s] not found!\n", pszEnvName);
        return HI_FAILURE;
    }
    pszEnvEnd = strchr(++pszEnvStart, '\'');
    if (!pszEnvEnd)
    {
        MLOGE("EnvName[%s] not found!\n", pszEnvName);
        return HI_FAILURE;
    }
    if ((HI_UL)(pszEnvEnd - pszEnvStart) > HI_UPGRADE_MAX_ENV_LEN)
    {
        MLOGE("%s too long!\n", pszEnvName);
        return HI_FAILURE;
    }
    snprintf(pszEnvValue, ((pszEnvEnd - pszEnvStart) + 1), "%s", pszEnvStart);
    MLOGD(YELLOW"EnvName[%s] Value[%s]"NONE"\n", pszEnvName, pszEnvValue);

    return HI_SUCCESS;
}

static HI_S32 UPGRADE_GetMtdPartInfo(const HI_CHAR* pszBootargs, UPGRADE_MTDPART_INFO_S* pstMtdPartInfo)
{
    HI_CHAR szMtdPartInfo[HI_UPGRADE_MAX_ENV_LEN] = {0,};
    HI_CHAR* pszStr = NULL;
    pszStr = strstr(pszBootargs, "parts=");
    if (!pszStr)
    {
        MLOGE("Invalid Bootargs[%s]\n", pszBootargs);
        return HI_FAILURE;
    }

    snprintf(szMtdPartInfo, HI_UPGRADE_MAX_ENV_LEN, "%s", pszStr);
    MLOGD(YELLOW"MtdInfo[%s]"NONE"\n", szMtdPartInfo);

    HI_CHAR* pSave = NULL;
    HI_CHAR* pToken = NULL;
    HI_CHAR* pTokenStart = NULL;
    HI_CHAR* pTokenEnd = NULL;
    memset(pstMtdPartInfo, 0, sizeof(UPGRADE_MTDPART_INFO_S));
    for (pszStr = szMtdPartInfo; ; pszStr = NULL)
    {
        pToken = strtok_r(pszStr, ",", &pSave);
        if (!pToken)
        {
            break;
        }
        pTokenStart = strchr(pToken, '(');
        if (!pTokenStart)
        {
            continue;
        }
        pTokenEnd = strchr(pToken, ')');
        if (!pTokenEnd)
        {
            continue;
        }

        if (HI_UPGRADE_MAX_PART_CNT <= pstMtdPartInfo->s32PartitionCnt)
        {
            MLOGE("too many partition\n");
            return HI_FAILURE;
        }
        snprintf(pstMtdPartInfo->aszPartitionName[pstMtdPartInfo->s32PartitionCnt],
                 (pTokenEnd - pTokenStart), "%s", pTokenStart + 1);
        MLOGD(GREEN"Part[%d] Name[%s]"NONE"\n", pstMtdPartInfo->s32PartitionCnt,
              pstMtdPartInfo->aszPartitionName[pstMtdPartInfo->s32PartitionCnt]);
        pstMtdPartInfo->s32PartitionCnt++;
    }

    return HI_SUCCESS;
}

static HI_S32 UPGRADE_ParseConfig(const HI_CHAR* pszConfigFile, HI_UPGRADE_PKT_HEAD_S* pstPktHead,
                                  UPGRADE_CONFIG_INFO_S* pstConfigFileInfo)
{
    HI_S32 s32Fd = -1;
    MLOGD("ConfigFile[%s]\n", pszConfigFile);

    /* Read Config */
    s32Fd = open(pszConfigFile, O_RDONLY);
    if (s32Fd < 0)
    {
        MLOGE("open file %s failed\n", pszConfigFile);
        return -1;
    }

    snprintf(pstConfigFileInfo->szFileName, HI_UPGRADE_PARTITION_NAME_MAX_LEN, "%s", pszConfigFile);
    HI_U8* pu8Buf = (HI_U8*)pstConfigFileInfo->szFileText;
    HI_U32 u32BufOffset = 0;
    HI_S32 s32ReadLen = 0;
    while (1)
    {
        s32ReadLen = read(s32Fd, pu8Buf + u32BufOffset, UPGRADE_CONFIG_MAX_LEN);
        if (s32ReadLen < 0)
        {
            MLOGE("read file failed\n");
            close(s32Fd);
            return -1;
        }
        else if (s32ReadLen == 0)
        {
            MLOGI("read file %s finished\n", pszConfigFile);
            close(s32Fd);
            break;
        }
        u32BufOffset += s32ReadLen;
    }
    pstConfigFileInfo->u32FileLen = u32BufOffset;
    MLOGD("Config Len[%u]\n", u32BufOffset);

    /* Get Env Value */
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = GetEnvValue((HI_CHAR*)pu8Buf, "bootargs", pstConfigFileInfo->szBootArgs);
    s32Ret |= GetEnvValue((HI_CHAR*)pu8Buf, "bootcmd", pstConfigFileInfo->szBootCmd);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    /* Parse Bootargs */
    s32Ret = UPGRADE_GetMtdPartInfo(pstConfigFileInfo->szBootArgs, &pstConfigFileInfo->stMtdPartInfo);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 main(HI_S32 argc, HI_CHAR** argv)
{
    HI_S32 s32Idx = 0;

    /* check command line param */
    if (COMMAND_PARAM_CNT != argc)
    {
        MLOGE("invalid input param number(%d)!\n", argc);
        for (s32Idx = 0; s32Idx < argc; ++s32Idx)
        {
            MLOGE("param[%d] %s\n", s32Idx, argv[s32Idx]);
        }
        usage();
        exit(1);
    }
    for (s32Idx = 0; s32Idx < argc; ++s32Idx)
    {
        MLOGD("param[%d] %s\n", s32Idx, argv[s32Idx]);
    }

    HI_CHAR szModel[HI_APPCOMM_COMM_STR_LEN];
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN];
    snprintf(szModel, HI_APPCOMM_COMM_STR_LEN, "%s", argv[1]);
    snprintf(szSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%s", argv[2]);
    MLOGD("szModel[%s] SoftVersion[%s]\n", szModel, szSoftVersion);

    /* allocate memory for release packet */
    const HI_U32 u32PktBufSize = UPGRADE_MAX_BUF_SIZE;
    HI_U8* pu8PktBuf = malloc(u32PktBufSize);
    if (!pu8PktBuf)
    {
        MLOGE("allocate packet buffer failed\n");
        exit(1);
    }
    memset(pu8PktBuf, 0, u32PktBufSize);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BufOffset = 0;
    HI_UPGRADE_PKT_HEAD_S* pstPktHead = (HI_UPGRADE_PKT_HEAD_S*)pu8PktBuf;
    HI_UPGRADE_PARTITION_HEAD_S* pstPartitionHead = NULL;
    UPGRADE_CONFIG_INFO_S  stConfigFileInfo;

    /* read and parse config file */
    memset(&stConfigFileInfo, 0, sizeof(stConfigFileInfo));
    s32Ret = UPGRADE_ParseConfig(argv[3], pstPktHead, &stConfigFileInfo);
    if (HI_SUCCESS != s32Ret)
    {
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        exit(1);
    }

    /* packet head */
    pstPktHead->u32Magic = HI_UPGRADE_PACKET_HEAD_MAGIC;
    pstPktHead->u32Crc = 0;
    pstPktHead->u32PktLen = 0;
    pstPktHead->bCompress = HI_FALSE;
    snprintf(pstPktHead->szPktModel, HI_APPCOMM_COMM_STR_LEN, "%s", szModel);
    snprintf(pstPktHead->szPktSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%s", szSoftVersion);
    memcpy(pstPktHead->szBootArgs, stConfigFileInfo.szBootArgs, HI_UPGRADE_MAX_ENV_LEN);
    memcpy(pstPktHead->szBootCmd, stConfigFileInfo.szBootCmd, HI_UPGRADE_MAX_ENV_LEN);

    u32BufOffset += sizeof(HI_UPGRADE_PKT_HEAD_S);

    /* config file */
    pstPktHead->u32ConfigFileOffSet = u32BufOffset;
    pstPartitionHead = (HI_UPGRADE_PARTITION_HEAD_S*)(pu8PktBuf + u32BufOffset);
    memcpy(pstPartitionHead->szPartName, stConfigFileInfo.szFileName, HI_UPGRADE_PARTITION_NAME_MAX_LEN);
    pstPartitionHead->u32OriDataLen = stConfigFileInfo.u32FileLen;
    pstPartitionHead->u32DataLen = stConfigFileInfo.u32FileLen;
    MLOGD("Config[%s] Offset[%u]\n", pstPartitionHead->szPartName, pstPktHead->u32ConfigFileOffSet);

    u32BufOffset += sizeof(HI_UPGRADE_PARTITION_HEAD_S);

    memcpy(pu8PktBuf + u32BufOffset, stConfigFileInfo.szFileText, stConfigFileInfo.u32FileLen);
    u32BufOffset += stConfigFileInfo.u32FileLen;

    /* partition image */
    for (s32Idx = 0; s32Idx < stConfigFileInfo.stMtdPartInfo.s32PartitionCnt; ++s32Idx)
    {
        /* check partition image */
        s32Ret = access(stConfigFileInfo.stMtdPartInfo.aszPartitionName[s32Idx], F_OK);
        if (0 != s32Ret)
        {
            MLOGW("image file[%s] not exist, ignore\n",
                  stConfigFileInfo.stMtdPartInfo.aszPartitionName[s32Idx]);
            continue;
        }

        pstPktHead->au32PartitionOffSet[pstPktHead->s32PartitionCnt] = u32BufOffset;
        MLOGD("Partition[%d:%s] Offset[%u]\n", pstPktHead->s32PartitionCnt,
              stConfigFileInfo.stMtdPartInfo.aszPartitionName[s32Idx],
              pstPktHead->au32PartitionOffSet[pstPktHead->s32PartitionCnt]);
        pstPartitionHead = (HI_UPGRADE_PARTITION_HEAD_S*)(pu8PktBuf + u32BufOffset);
        memcpy(pstPartitionHead->szPartName, stConfigFileInfo.stMtdPartInfo.aszPartitionName[s32Idx],
               HI_UPGRADE_PARTITION_NAME_MAX_LEN);
        MLOGD(BLUE"    Name[%s]"NONE"\n", pstPartitionHead->szPartName);

        u32BufOffset += sizeof(HI_UPGRADE_PARTITION_HEAD_S);

        HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN] = {0,};
        snprintf(szFileName, HI_APPCOMM_MAX_PATH_LEN, "./%s", pstPartitionHead->szPartName);

        /* open file */
        HI_S32 s32InputFd = -1;
        s32InputFd = open(szFileName, O_RDONLY);
        if (s32InputFd < 0)
        {
            MLOGE("open file %s failed\n", szFileName);
            HI_APPCOMM_SAFE_FREE(pu8PktBuf);
            exit(1);
        }

        HI_S32 s32ReadLen = 0;
        while (1)
        {
            s32ReadLen = read(s32InputFd, pu8PktBuf + u32BufOffset, UPGRADE_BLOCK_SIZE);
            if (s32ReadLen < 0)
            {
                MLOGE("read file failed\n");
                close(s32InputFd);
                HI_APPCOMM_SAFE_FREE(pu8PktBuf);
                exit(1);
            }
            else if (s32ReadLen == 0)
            {
                MLOGI("read file %s finished\n", szFileName);
                close(s32InputFd);
                break;
            }
            u32BufOffset += s32ReadLen;
            pstPartitionHead->u32OriDataLen += s32ReadLen;

        }
        pstPartitionHead->u32DataLen = pstPartitionHead->u32OriDataLen;
        MLOGD("    OriDataLen[%u] DataLen[%u] u32BufOffset[%u]\n",
              pstPartitionHead->u32OriDataLen, pstPartitionHead->u32DataLen, u32BufOffset);

        pstPktHead->s32PartitionCnt++;
    }

    /* packet tail */
    HI_UPGRADE_PKT_TAIL_S* pstPktTail = NULL;
    pstPktTail = (HI_UPGRADE_PKT_TAIL_S*)(pu8PktBuf + u32BufOffset);
    pstPktTail->u32Magic = HI_UPGRADE_PACKET_TAIL_MAGIC;

    /* update packet head info: length/crc */
    pstPktHead->u32PktLen = u32BufOffset + sizeof(HI_UPGRADE_PKT_TAIL_S);
    pstPktHead->u32Crc = __crc32(0, (HI_U8*)&pstPktHead->u32HeadVer, u32BufOffset - 8);
    MLOGD("PktLen[%u], Crc[%#08x]\n", pstPktHead->u32PktLen, pstPktHead->u32Crc);

    /* ------------------------------------------------------------------- */

    /* generate release packet name */
    HI_S32 s32ReleaseFd = -1;
    HI_CHAR szReleasePacketName[HI_UPGRADE_PKT_PATH_MAX_LEN] = {0,};
    snprintf(szReleasePacketName, HI_UPGRADE_PKT_PATH_MAX_LEN,
             "./"HI_UPGRADE_PKT_PREFIX"_%s_%s."UPGRADE_PKT_SUFFIX,
             szModel, szSoftVersion);
    MLOGD("release packet name %s\n", szReleasePacketName);

    /* recreate release packet file */
    unlink(szReleasePacketName);
    s32ReleaseFd = open(szReleasePacketName, O_WRONLY | O_CREAT);
    if (s32ReleaseFd < 0)
    {
        MLOGE("create release file %s failed\n", szReleasePacketName);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        exit(1);
    }

    /* write release packet */
    HI_S32 s32RemainWriteLen = pstPktHead->u32PktLen;
    HI_S32 s32BytesToWrite = 0;
    HI_S32 s32WriteLen = 0;
    HI_S32 s32PktBufIdx = 0;
    while (s32RemainWriteLen > 0)
    {
        s32BytesToWrite = (s32RemainWriteLen > UPGRADE_BLOCK_SIZE) ?
                          UPGRADE_BLOCK_SIZE : s32RemainWriteLen;
        s32WriteLen = write(s32ReleaseFd, pu8PktBuf + s32PktBufIdx, s32BytesToWrite);
        if (s32WriteLen < 0)
        {
            MLOGE("write release packet failed\n");
            close(s32ReleaseFd);
            unlink(szReleasePacketName);
            HI_APPCOMM_SAFE_FREE(pu8PktBuf);
            exit(1);
        }
        s32RemainWriteLen -= s32WriteLen;
        s32PktBufIdx += s32WriteLen;
    }

    fsync(s32ReleaseFd);
    fchmod(s32ReleaseFd, 0777);
    close(s32ReleaseFd);

    HI_APPCOMM_SAFE_FREE(pu8PktBuf);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

