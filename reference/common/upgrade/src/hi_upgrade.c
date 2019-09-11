/**
 * @file      hi_upgrade.c
 * @brief     product upgrade interface implementatio
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/5
 * @version   1.0

 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include "hi_appcomm_util.h"
#include "hi_upgrade.h"
#include "hi_eventhub.h"
#include "crc.h"

#ifdef __cplusplus
#if __cplusplus
 extern "C" {
#endif
#endif /* __cplusplus */


#define UPGRADE_IMGDATA_BUFF_MAX_SIZE   (0x40000)
#define UPGRADE_PKT_BUFF_SIZE           (sizeof(HI_UPGRADE_PKT_HEAD_S) \
    + sizeof(HI_UPGRADE_PARTITION_HEAD_S) + UPGRADE_IMGDATA_BUFF_MAX_SIZE)
#define UPGRADE_BLOCK_SIZE              (0x10000) /**<64K */

#if defined(PWR_USER_REG0)

#define UPGRADE_STATUS_REG              PWR_USER_REG0
#define UPGRADE_STATUS_BITSTART         0      /**< upgrade status start bit*/
#define UPGRADE_STATUS_BITMASK          0xff   /**< Bit[0-7]*/

#elif defined(SOFTINT_REG)

#define UPGRADE_STATUS_REG              SOFTINT_REG
#define UPGRADE_STATUS_BITSTART         8      /**< upgrade status start bit*/
#define UPGRADE_STATUS_BITMASK          0xff00 /**< Bit[8-15]*/

#endif

#define UPGRADE_DEVBLK_BITSTART         16
#define UPGRADE_DEVBLK_BITMASK          0x030000 /**<Bit[16-17] */
#define UPGRADE_PARTITION_BITSTART      18
#define UPGRADE_PARTITION_BITMASK       0x7c0000 /**<Bit[18-22] */

/** upgrade status macro */
#if defined(PWR_USER_REG0) || defined(SOFTINT_REG)

#define UPGRADE_GET_STATUS(status)      \
    { \
        HI_U32 __u32RegValue = 0; \
        himd(UPGRADE_STATUS_REG, &__u32RegValue); \
        status = (__u32RegValue & UPGRADE_STATUS_BITMASK) >> UPGRADE_STATUS_BITSTART; \
    }

#define UPGRADE_CLR_STATUS()            himm(UPGRADE_STATUS_REG, 0)

#define UPGRADE_SET_STATUS(status, devno, partno)      \
    { \
        HI_U32 __u32RegValue = 0; \
        __u32RegValue = ((status << UPGRADE_STATUS_BITSTART) & UPGRADE_STATUS_BITMASK) \
            | ((devno << UPGRADE_DEVBLK_BITSTART) & UPGRADE_DEVBLK_BITMASK) \
            | ((partno << UPGRADE_PARTITION_BITSTART) & UPGRADE_PARTITION_BITMASK); \
        himm(UPGRADE_STATUS_REG, __u32RegValue);\
    }

#else

#define UPGRADE_GET_STATUS(status)      status = 0
#define UPGRADE_CLR_STATUS()
#define UPGRADE_SET_STATUS(status, devno, partno)

#endif

/** upgrade version information register */
#if defined(PWR_USER_REG1) && defined(PWR_USER_REG2)
#define UPGRADE_VERSION_REG_ADDR        PWR_USER_REG1
#define UPGRADE_BUILDDATE_REG_ADDR      PWR_USER_REG2
#endif

/** check init status */
#define UPGRADE_CHECK_INIT(bInit)\
    do{\
        if (!bInit)\
        {\
            MLOGE("module not init yet\n");\
            return HI_UPGRADE_ENOTINIT;\
        }\
    }while(0);

/** upgrade version type: register read/write */
typedef struct tagUPGRADE_VERSION_TYPE1_S
{
    HI_U32 u32Version;
    HI_U32 u32BuildDate;
} UPGRADE_VERSION_TYPE1_S;

/** upgrade version type: section parser */
typedef struct tagUPGRADE_VERSION_TYPE2_S
{
    HI_U8  u8MainVer;
    HI_U8  u8SubVer;
    HI_U8  u8BuildVer;
    HI_U8  u8TmpVer;
    HI_U32 u32BuildDate;
} UPGRADE_VERSION_TYPE2_S;

/** upgrade version union */
typedef union tagUPGRADE_VERSION_U
{
    UPGRADE_VERSION_TYPE1_S stVerType1;
    UPGRADE_VERSION_TYPE2_S stVerType2;
} UPGRADE_VERSION_U;

/** upgrade packet information */
typedef struct tagUPGRADE_PKT_INFO_S
{
    HI_U32  u32PktLen;
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN]; /**<packet version */
    HI_CHAR szModel[HI_APPCOMM_COMM_STR_LEN];   /**<packet model */
    HI_CHAR szPktFilePath[HI_UPGRADE_PKT_PATH_MAX_LEN];
} UPGRADE_PKT_INFO_S;

/** upgrade status enum */
typedef enum tagUPGRADE_STATUS_E
{
    UPGRADE_STATUS_IDLE = 0,
    UPGRADE_STATUS_PROCESSING,
    UPGRADE_STATUS_FINISH,
    UPGRADE_STATUS_BUTT
} UPGRADE_STATUS_E;

/** upgrade information, between app and uboot */
typedef struct tagUPGRADE_INFO_S
{
    HI_U32 u32Magic;
    UPGRADE_STATUS_E enStatus;
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN];
} UPGRADE_INFO_S;

/** upgrade module context */
typedef struct tagUPGRADE_CONTEXT_S
{
    HI_BOOL bInit;
    UPGRADE_STATUS_E  enStatus;
    UPGRADE_VERSION_U unVersion;
} UPGRADE_CONTEXT_S;
static UPGRADE_CONTEXT_S s_stUPGRADECtx;


HI_S32 HI_UPGRADE_RegisterEvent(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret  = HI_EVTHUB_Register(HI_EVENT_UPGRADE_NEWPKT);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_UPGRADE_SUCCESS);
    s32Ret |= HI_EVTHUB_Register(HI_EVENT_UPGRADE_FAILURE);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    return HI_SUCCESS;
}

HI_S32 HI_UPGRADE_Init(HI_VOID)
{
    if (s_stUPGRADECtx.bInit)
    {
        MLOGD("already Init\n");
        return HI_UPGRADE_EINITIALIZED;
    }

    /* get upgrade status/softversion */
    UPGRADE_GET_STATUS(s_stUPGRADECtx.enStatus);
#if defined(PWR_USER_REG1) && defined(PWR_USER_REG2)
    himd(UPGRADE_VERSION_REG_ADDR, &s_stUPGRADECtx.unVersion.stVerType1.u32Version);
    himd(UPGRADE_BUILDDATE_REG_ADDR, &s_stUPGRADECtx.unVersion.stVerType1.u32BuildDate);
#endif
    MLOGI("Status[%d]\n", s_stUPGRADECtx.enStatus);
    MLOGD("Version[%u.%u.%u.%u.%u]\n", s_stUPGRADECtx.unVersion.stVerType2.u8MainVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8SubVer, s_stUPGRADECtx.unVersion.stVerType2.u8BuildVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8TmpVer, s_stUPGRADECtx.unVersion.stVerType2.u32BuildDate);

    /* check upgrade result */
    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));
    HI_UPGRADE_EVENT_INFO_S* pstEventInfo = (HI_UPGRADE_EVENT_INFO_S*)stEvent.aszPayload;
    snprintf(pstEventInfo->szSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%u.%u.%u.%u.%u",
        s_stUPGRADECtx.unVersion.stVerType2.u8MainVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8SubVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8BuildVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8TmpVer,
        s_stUPGRADECtx.unVersion.stVerType2.u32BuildDate);

    if (UPGRADE_STATUS_PROCESSING == s_stUPGRADECtx.enStatus)
    {
        stEvent.EventID = HI_EVENT_UPGRADE_FAILURE;
        MLOGD("Failure\n");
        HI_EVTHUB_Publish(&stEvent);
    }
    else if (UPGRADE_STATUS_FINISH == s_stUPGRADECtx.enStatus)
    {
        stEvent.EventID = HI_EVENT_UPGRADE_SUCCESS;
        MLOGD("Success\n");
        HI_EVTHUB_Publish(&stEvent);
    }

    /* Clear Upgrade Register */
    UPGRADE_CLR_STATUS();
#if defined(PWR_USER_REG1) && defined(PWR_USER_REG2)
    himm(UPGRADE_VERSION_REG_ADDR, 0);
    himm(UPGRADE_BUILDDATE_REG_ADDR, 0);
#endif
    /* Reset upgrade information */
    s_stUPGRADECtx.bInit = HI_TRUE;
    s_stUPGRADECtx.enStatus = UPGRADE_STATUS_IDLE;

    return HI_SUCCESS;
}

HI_S32 HI_UPGRADE_Deinit(HI_VOID)
{
    UPGRADE_CHECK_INIT(s_stUPGRADECtx.bInit);
    s_stUPGRADECtx.bInit = HI_FALSE;
    return HI_SUCCESS;
}

static HI_BOOL UPGRADE_CheckPktNameValid(const HI_CHAR* pszPktName)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* prefix */
    HI_S32 s32PrefixStrLen = strnlen(HI_UPGRADE_PKT_PREFIX, HI_APPCOMM_MAX_FILENAME_LEN);
    s32Ret = strncmp(pszPktName, HI_UPGRADE_PKT_PREFIX, s32PrefixStrLen);
    if (0 != s32Ret)
    {
        return HI_FALSE;
    }
    MLOGD("PktName[%s]\n", pszPktName);
    return HI_TRUE;
}

static HI_S32 UPGRADE_CompareVersion(const HI_CHAR* pszVer1, const HI_CHAR* pszVer2)
{
    HI_S32 s32Ret = strncmp(pszVer1, pszVer2, HI_APPCOMM_COMM_STR_LEN);
    if (0 == s32Ret)
    {
        MLOGD("Ver1[%s] is the same with Ver2[%s]\n", pszVer1, pszVer2);
        return 0;
    }
    else if (0 > s32Ret)
    {
        MLOGD("Ver1[%s] is older than Ver2[%s]\n", pszVer1, pszVer2);
        return -1;
    }
    else
    {
        MLOGD("Ver1[%s] is newer than Ver2[%s]\n", pszVer1, pszVer2);
        return 1;
    }
}

static HI_S32 UPGRADE_ReadPktHead(HI_S32 s32Fd, HI_UPGRADE_PKT_HEAD_S* pstPktHead)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = lseek(s32Fd, 0, SEEK_SET);
    if (-1 == s32Ret)
    {
        MLOGE("lseek file failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_S32 s32ReadLen = 0;
    s32ReadLen = read(s32Fd, pstPktHead, sizeof(HI_UPGRADE_PKT_HEAD_S));
    if (s32ReadLen < 0)
    {
        MLOGE("read file failed\n");
        return HI_UPGRADE_EINTR;
    }
    MLOGD("Pkt HeadInfo:\n");
    MLOGD("  Magic[%#08x] Crc[%#08x] PktLen[%u] bCompress[%d]\n",
        pstPktHead->u32Magic, pstPktHead->u32Crc, pstPktHead->u32PktLen, pstPktHead->bCompress);
    MLOGD("  Model[%s] Version[%s]\n", pstPktHead->szPktModel, pstPktHead->szPktSoftVersion);
    MLOGD("  BootArgs[%s]\n", pstPktHead->szBootArgs);
    MLOGD("  BootCmd[%s]\n", pstPktHead->szBootCmd);
    MLOGD("  ConfigOffset: %u\n", pstPktHead->u32ConfigFileOffSet);
    MLOGD("  PartitionCnt[%d]\n", pstPktHead->s32PartitionCnt);
    return HI_SUCCESS;
}

static HI_S32 UPGRADE_ReadPktTail(HI_S32 s32Fd, HI_U32 u32PktLen, HI_UPGRADE_PKT_TAIL_S* pstPktTail)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = lseek(s32Fd, u32PktLen - sizeof(HI_UPGRADE_PKT_TAIL_S), SEEK_SET);
    if (-1 == s32Ret)
    {
        MLOGE("lseek file failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_S32 s32ReadLen = 0;
    s32ReadLen = read(s32Fd, pstPktTail, sizeof(HI_UPGRADE_PKT_TAIL_S));
    if (s32ReadLen < 0)
    {
        MLOGE("read file failed\n");
        return HI_UPGRADE_EINTR;
    }
    MLOGD("Pkt TailInfo:\n");
    MLOGD("  Magic[%#08x]\n", pstPktTail->u32Magic);
    return HI_SUCCESS;
}

static HI_S32 UPGRADE_CheckPktCrc(const HI_UPGRADE_PKT_HEAD_S* pstPktHead,
                        HI_S32 s32Fd, HI_U8* pu8Buf, HI_U32 u32BufSize)
{
    HI_U32 u32PktCrc = 0;
    u32PktCrc = __crc32(u32PktCrc, (HI_U8*)&pstPktHead->u32HeadVer,
        sizeof(HI_UPGRADE_PKT_HEAD_S) - 8);

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = lseek(s32Fd, sizeof(HI_UPGRADE_PKT_HEAD_S), SEEK_SET);
    if (-1 == s32Ret)
    {
        MLOGE("lseek file failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_S32 s32RemainReadLen =
        pstPktHead->u32PktLen - sizeof(HI_UPGRADE_PKT_HEAD_S) - sizeof(HI_UPGRADE_PKT_TAIL_S);
    HI_S32 s32BytesToRead = 0;
    HI_S32 s32ReadLen = 0;
    while(s32RemainReadLen > 0)
    {
        /* read image data */
        s32BytesToRead = (s32RemainReadLen > UPGRADE_BLOCK_SIZE) ?
                                    UPGRADE_BLOCK_SIZE : s32RemainReadLen;
        s32ReadLen = read(s32Fd, pu8Buf, s32BytesToRead);
        if (0 > s32ReadLen)
        {
            if (EINTR == errno)
            {
                MLOGW("read file interuptted, retry\n");
                continue;
            }

            MLOGE("read file failed\n");
            return HI_UPGRADE_EINTR;
        }
        else if (s32ReadLen == 0)
        {
            MLOGE("read end of file\n");
            return HI_UPGRADE_EINTR;
        }
        s32RemainReadLen -= s32ReadLen;

        u32PktCrc = __crc32(u32PktCrc, pu8Buf, s32ReadLen);
    }
    MLOGD("PktCrc[%#08x]\n", u32PktCrc);

    if (u32PktCrc != pstPktHead->u32Crc)
    {
        MLOGE("CrcDismatch[%#08x, %#08x]\n", u32PktCrc, pstPktHead->u32Crc);
        return HI_UPGRADE_EPKT_INVALID;
    }
    return HI_SUCCESS;
}

static HI_S32 UPGRADE_CheckPktValid(UPGRADE_PKT_INFO_S* pstPktInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8PktBuf = (HI_U8*)malloc(UPGRADE_PKT_BUFF_SIZE);
    HI_APPCOMM_CHECK_POINTER(pu8PktBuf, HI_UPGRADE_EINTR);
    memset(pu8PktBuf, 0, UPGRADE_PKT_BUFF_SIZE);

    HI_S32 s32PktFd = -1;
    s32PktFd = open(pstPktInfo->szPktFilePath, O_RDONLY);
    if (s32PktFd < 0)
    {
        MLOGE("open file %s failed\n", pstPktInfo->szPktFilePath);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    /* Read Pkt Head Info */
    HI_UPGRADE_PKT_HEAD_S* pstPktHead = (HI_UPGRADE_PKT_HEAD_S*)pu8PktBuf;
    s32Ret = UPGRADE_ReadPktHead(s32PktFd, pstPktHead);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_ReadPktHead failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    /* Check Pkt Head Info */
    if (HI_UPGRADE_PACKET_HEAD_MAGIC != pstPktHead->u32Magic)
    {
        MLOGD("Invalid HeadMagic[%#08x] expect[%#08x]\n",
            pstPktHead->u32Magic, HI_UPGRADE_PACKET_HEAD_MAGIC);
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EPKT_INVALID;
    }

    if (strncmp(pstPktHead->szPktModel, pstPktInfo->szModel, HI_APPCOMM_COMM_STR_LEN))
    {
        MLOGE("Invalid Model[%s] expect[%s]\n", pstPktHead->szPktModel, pstPktInfo->szModel);
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EPKT_INVALID;
    }
    snprintf(pstPktInfo->szSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%s", pstPktHead->szPktSoftVersion);
    pstPktInfo->u32PktLen = pstPktHead->u32PktLen;

    /* Crc Check */
    s32Ret = UPGRADE_CheckPktCrc(pstPktHead, s32PktFd,
        (HI_U8*)(pu8PktBuf + sizeof(HI_UPGRADE_PKT_HEAD_S)), UPGRADE_IMGDATA_BUFF_MAX_SIZE);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_CheckPktCrc failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EPKT_INVALID;
    }

    /* Read Pkt Tail Info */
    HI_UPGRADE_PKT_TAIL_S stPktTail;
    s32Ret = UPGRADE_ReadPktTail(s32PktFd, pstPktInfo->u32PktLen, &stPktTail);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_ReadPktTail failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    /* Check Pkt Tail Info */
    if (HI_UPGRADE_PACKET_TAIL_MAGIC != stPktTail.u32Magic)
    {
        MLOGD("Invalid TailMagic[%#08x] expect[%#08x]\n",
            stPktTail.u32Magic, HI_UPGRADE_PACKET_TAIL_MAGIC);
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EPKT_INVALID;
    }

    close(s32PktFd);
    HI_APPCOMM_SAFE_FREE(pu8PktBuf);

    return HI_SUCCESS;
}

HI_S32 HI_UPGRADE_CheckPkt(const HI_CHAR* pszPktUrl, const HI_UPGRADE_DEV_INFO_S* pstDevInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszPktUrl, HI_UPGRADE_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDevInfo, HI_UPGRADE_EINVAL);
    UPGRADE_CHECK_INIT(s_stUPGRADECtx.bInit);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));
    HI_UPGRADE_EVENT_INFO_S* pstEventInfo = (HI_UPGRADE_EVENT_INFO_S*)stEvent.aszPayload;
    UPGRADE_PKT_INFO_S stPktInfo;

    memset(&stPktInfo, 0, sizeof(UPGRADE_PKT_INFO_S));
    snprintf(stPktInfo.szModel, HI_APPCOMM_COMM_STR_LEN, "%s", pstDevInfo->szModel);
    snprintf(stPktInfo.szPktFilePath, HI_UPGRADE_PKT_PATH_MAX_LEN, "%s", pszPktUrl);

    if ((HI_SUCCESS == UPGRADE_CheckPktValid(&stPktInfo))
        && (1 == UPGRADE_CompareVersion(stPktInfo.szSoftVersion, pstDevInfo->szSoftVersion)))
    {
        memcpy(pstEventInfo->szSoftVersion, stPktInfo.szSoftVersion, HI_APPCOMM_COMM_STR_LEN);
        memcpy(pstEventInfo->szPktFilePath, stPktInfo.szPktFilePath, HI_UPGRADE_PKT_PATH_MAX_LEN);
        pstEventInfo->u32PktLen = stPktInfo.u32PktLen;
        MLOGD(YELLOW"ValidPkt[%s] Version[%s] Len[%#x]"NONE"\n",
            stPktInfo.szPktFilePath, stPktInfo.szSoftVersion, pstEventInfo->u32PktLen);

        stEvent.EventID = HI_EVENT_UPGRADE_NEWPKT;
        MLOGI(GREEN"NewPkt[%s]"NONE"\n", pstEventInfo->szSoftVersion);
        HI_EVTHUB_Publish(&stEvent);
        return HI_UPGRADE_PKT_AVAILABLE;
    }

    return HI_SUCCESS;
}

HI_S32 HI_UPGRADE_SrchNewPkt(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszPktPath, HI_UPGRADE_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstDevInfo, HI_UPGRADE_EINVAL);
    UPGRADE_CHECK_INIT(s_stUPGRADECtx.bInit);
    MLOGD("PktPath[%s] Model[%s] SoftVersion[%s]\n", pszPktPath,
        pstDevInfo->szModel, pstDevInfo->szSoftVersion);

    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));
    HI_UPGRADE_EVENT_INFO_S* pstEventInfo = (HI_UPGRADE_EVENT_INFO_S*)stEvent.aszPayload;
    UPGRADE_PKT_INFO_S stPktInfo;
    HI_BOOL bFound = HI_FALSE;

    snprintf(pstEventInfo->szSoftVersion, HI_APPCOMM_COMM_STR_LEN, "%s", pstDevInfo->szSoftVersion);

    memset(&stPktInfo, 0, sizeof(UPGRADE_PKT_INFO_S));
    snprintf(stPktInfo.szModel, HI_APPCOMM_COMM_STR_LEN, "%s", pstDevInfo->szModel);

    /* Traverse Upgrade Path to find available packet */
    DIR* pstDir = NULL;
    struct dirent* pstDirItem = NULL;
    pstDir = opendir(pszPktPath);
    if (NULL == pstDir)
    {
        MLOGD("%s is not a directory(%s)\n", pszPktPath, strerror(errno));
        return HI_UPGRADE_ELOST;
    }

    pstDirItem = readdir(pstDir);
    while(NULL != pstDirItem)
    {
        if (UPGRADE_CheckPktNameValid(pstDirItem->d_name))
        {
            snprintf(stPktInfo.szPktFilePath, HI_UPGRADE_PKT_PATH_MAX_LEN,
                "%s/%s", pszPktPath, pstDirItem->d_name);
            if ((HI_SUCCESS == UPGRADE_CheckPktValid(&stPktInfo))
                && (1 == UPGRADE_CompareVersion(stPktInfo.szSoftVersion, pstEventInfo->szSoftVersion)))
            {
                memcpy(pstEventInfo->szSoftVersion, stPktInfo.szSoftVersion, HI_APPCOMM_COMM_STR_LEN);
                snprintf(pstEventInfo->szPktFilePath, HI_UPGRADE_PKT_PATH_MAX_LEN,
                    "%s/%s", pszPktPath, pstDirItem->d_name);
                pstEventInfo->u32PktLen = stPktInfo.u32PktLen;
                MLOGD(YELLOW"Find NewerVersion[%s], Url[%s] Len[%#x]"NONE"\n",
                    pstEventInfo->szSoftVersion, pstEventInfo->szPktFilePath, pstEventInfo->u32PktLen);
                bFound = HI_TRUE;
            }
        }

        pstDirItem = readdir(pstDir);
    }
    closedir(pstDir);

    /* publish newpkt event if found */
    if (bFound)
    {
        stEvent.EventID = HI_EVENT_UPGRADE_NEWPKT;
        MLOGI(GREEN"NewPkt[%s]"NONE"\n", pstEventInfo->szSoftVersion);
        HI_EVTHUB_Publish(&stEvent);
        return HI_UPGRADE_PKT_AVAILABLE;
    }

    return HI_SUCCESS;
}

static HI_S32 UPGRADE_ReadPartitionHead(HI_S32 s32Fd, HI_U32 PartitionOffset,
                                 HI_UPGRADE_PARTITION_HEAD_S* pstPartitionHead)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = lseek(s32Fd, (HI_S32)PartitionOffset, SEEK_SET);
    if (-1 == s32Ret)
    {
        MLOGE("lseek file failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_S32 s32ReadLen = 0;
    s32ReadLen = read(s32Fd, pstPartitionHead, sizeof(HI_UPGRADE_PARTITION_HEAD_S));
    if (s32ReadLen < 0)
    {
        MLOGE("read file failed\n");
        return HI_UPGRADE_EINTR;
    }
    MLOGD("Partition Head:\n");
    MLOGD("  PartitionName[%s] OriDataLen[%u] DataLen[%u]\n",
        pstPartitionHead->szPartName, pstPartitionHead->u32OriDataLen, pstPartitionHead->u32DataLen);
    return HI_SUCCESS;
}

static HI_S32 UPGRADE_SeparatePartitionImage(const HI_CHAR* pszPktPath, HI_S32 s32PktFd, HI_U32 DataOffset,
                                      const HI_UPGRADE_PARTITION_HEAD_S* pstPartitionHead,
                                      HI_U8* pu8Buf, HI_U32 u32BufSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szImgFileName[HI_APPCOMM_MAX_FILENAME_LEN] = {0,};
    snprintf(szImgFileName, HI_APPCOMM_MAX_FILENAME_LEN, "%s/%s", pszPktPath, pstPartitionHead->szPartName);
    MLOGD(YELLOW"Image FileName[%s]"NONE"\n", szImgFileName);

    s32Ret = lseek(s32PktFd, (HI_S32)DataOffset, SEEK_SET);
    if (-1 == s32Ret)
    {
        MLOGE("lseek file failed\n");
        return HI_UPGRADE_EINTR;
    }

    /* recreate image file */
    HI_S32 s32ImageFd = -1;
    unlink(szImgFileName);
    s32ImageFd = open(szImgFileName, O_WRONLY | O_CREAT);
    if (s32ImageFd < 0)
    {
        MLOGE("create image file %s failed\n", szImgFileName);
        return HI_UPGRADE_EINTR;
    }

    HI_S32 s32RemainReadLen = pstPartitionHead->u32DataLen;
    HI_S32 s32BytesToRead = 0;
    HI_S32 s32ReadLen = 0;
    HI_S32 s32WriteLen = 0;
    while(s32RemainReadLen > 0)
    {
        /* read image data */
        s32BytesToRead = (s32RemainReadLen > UPGRADE_BLOCK_SIZE) ?
                                    UPGRADE_BLOCK_SIZE : s32RemainReadLen;
        s32ReadLen = read(s32PktFd, pu8Buf, s32BytesToRead);
        if (0 > s32ReadLen)
        {
            if (EINTR == errno)
            {
                MLOGW("read file interuptted, retry\n");
                continue;
            }

            MLOGE("read file failed\n");
            close(s32ImageFd);
            return HI_UPGRADE_EINTR;
        }
        else if (s32ReadLen == 0)
        {
            MLOGE("read end of file\n");
            close(s32ImageFd);
            return HI_UPGRADE_EINTR;
        }
        s32RemainReadLen -= s32ReadLen;

        /* write image data */
        HI_S32 s32RemainWriteLen = s32ReadLen;
        HI_S32 s32BytesWritten = 0;
        while(s32RemainWriteLen > 0)
        {
            s32WriteLen = write(s32ImageFd, pu8Buf + s32BytesWritten, s32RemainWriteLen);
            if (s32WriteLen < 0)
            {
                if (EINTR == errno)
                {
                    MLOGW("write is interuptted, retry\n");
                    continue;
                }

                MLOGE("write image data failed\n");
                close(s32ImageFd);
                unlink(szImgFileName);
                return HI_UPGRADE_EINTR;
            }
            s32RemainWriteLen -= s32WriteLen;
            s32BytesWritten += s32WriteLen;
        }
    }

    fsync(s32ImageFd);
#ifndef __HuaweiLite__
    fchmod(s32ImageFd, 0777);
    close(s32ImageFd);
#else
    close(s32ImageFd);
    chmod(szImgFileName, 0777);
#endif

    return HI_SUCCESS;
}

HI_S32 HI_UPGRADE_DoUpgrade(const HI_CHAR* pszPktPath, const HI_CHAR* pszPktUrl)
{
    HI_APPCOMM_CHECK_POINTER(pszPktPath, HI_UPGRADE_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pszPktUrl, HI_UPGRADE_EINVAL);
    UPGRADE_CHECK_INIT(s_stUPGRADECtx.bInit);
    MLOGD("PktUrl[%s]\n", pszPktUrl);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8* pu8PktBuf = (HI_U8*)malloc(UPGRADE_PKT_BUFF_SIZE);
    HI_APPCOMM_CHECK_POINTER(pu8PktBuf, HI_UPGRADE_EINTR);
    memset(pu8PktBuf, 0, UPGRADE_PKT_BUFF_SIZE);

    HI_S32 s32PktFd = -1;
    s32PktFd = open(pszPktUrl, O_RDONLY);
    if (pszPktUrl < 0)
    {
        MLOGE("open file %s failed\n", pszPktUrl);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    HI_UPGRADE_PKT_HEAD_S* pstPktHead = (HI_UPGRADE_PKT_HEAD_S*)pu8PktBuf;

    /* Read Pkt Head Info */
    s32Ret = UPGRADE_ReadPktHead(s32PktFd, pstPktHead);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_ReadPktHead failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    /* Record PktVersion */
    HI_U32 u32MainVer = 0;
    HI_U32 u32SubVer = 0;
    HI_U32 u32BuildVer = 0;
    HI_U32 u32TmpVer = 0;
    sscanf(pstPktHead->szPktSoftVersion, "%u.%u.%u.%u.%u",
        &u32MainVer, &u32SubVer, &u32BuildVer, &u32TmpVer,
        &s_stUPGRADECtx.unVersion.stVerType2.u32BuildDate);
    s_stUPGRADECtx.unVersion.stVerType2.u8MainVer = u32MainVer;
    s_stUPGRADECtx.unVersion.stVerType2.u8SubVer = u32SubVer;
    s_stUPGRADECtx.unVersion.stVerType2.u8BuildVer = u32BuildVer;
    s_stUPGRADECtx.unVersion.stVerType2.u8TmpVer = u32TmpVer;
    MLOGD("     Version[%u.%u.%u.%u.%u]\n", s_stUPGRADECtx.unVersion.stVerType2.u8MainVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8SubVer, s_stUPGRADECtx.unVersion.stVerType2.u8BuildVer,
        s_stUPGRADECtx.unVersion.stVerType2.u8TmpVer, s_stUPGRADECtx.unVersion.stVerType2.u32BuildDate);

    HI_UPGRADE_PARTITION_HEAD_S* pstPartitionHead =
            (HI_UPGRADE_PARTITION_HEAD_S*)(pu8PktBuf + sizeof(HI_UPGRADE_PKT_HEAD_S));
    HI_U8* pu8ImgDataBuf = (HI_U8*)(pu8PktBuf + sizeof(HI_UPGRADE_PKT_HEAD_S) +
                                    sizeof(HI_UPGRADE_PARTITION_HEAD_S));

    /* Config File */
    s32Ret = UPGRADE_ReadPartitionHead(s32PktFd,
            pstPktHead->u32ConfigFileOffSet,pstPartitionHead);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_ReadPartitionHead failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    s32Ret = UPGRADE_SeparatePartitionImage(pszPktPath, s32PktFd,
                pstPktHead->u32ConfigFileOffSet + sizeof(HI_UPGRADE_PARTITION_HEAD_S),
                pstPartitionHead, pu8ImgDataBuf, UPGRADE_IMGDATA_BUFF_MAX_SIZE);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_SeparatePartitionImage failed\n");
        close(s32PktFd);
        HI_APPCOMM_SAFE_FREE(pu8PktBuf);
        return HI_UPGRADE_EINTR;
    }

    /* Partition Image */
    HI_S32 s32Idx = 0;
    for (s32Idx  = 0; s32Idx < pstPktHead->s32PartitionCnt; ++s32Idx)
    {
        s32Ret = UPGRADE_ReadPartitionHead(s32PktFd,
            pstPktHead->au32PartitionOffSet[s32Idx],pstPartitionHead);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("UPGRADE_ReadPartitionHead failed\n");
            close(s32PktFd);
            HI_APPCOMM_SAFE_FREE(pu8PktBuf);
            return HI_UPGRADE_EINTR;
        }

        s32Ret = UPGRADE_SeparatePartitionImage(pszPktPath, s32PktFd,
                    pstPktHead->au32PartitionOffSet[s32Idx] + sizeof(HI_UPGRADE_PARTITION_HEAD_S),
                    pstPartitionHead, pu8ImgDataBuf, UPGRADE_IMGDATA_BUFF_MAX_SIZE);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("UPGRADE_SeparatePartitionImage failed\n");
            close(s32PktFd);
            HI_APPCOMM_SAFE_FREE(pu8PktBuf);
            return HI_UPGRADE_EINTR;
        }
    }
    close(s32PktFd);
    HI_APPCOMM_SAFE_FREE(pu8PktBuf);

    /* Update upgrade status */
    s_stUPGRADECtx.enStatus = UPGRADE_STATUS_PROCESSING;
#if defined(CFG_EMMC_FLASH)
    UPGRADE_SET_STATUS(s_stUPGRADECtx.enStatus, CONFIG_EMMC_STORAGE_BLK_NO, CONFIG_EMMC_STORAGE_PART_NO);
#else
    UPGRADE_SET_STATUS(s_stUPGRADECtx.enStatus, 0, 0);
#endif

#if defined(UPGRADE_VERSION_REG_ADDR) && defined(UPGRADE_BUILDDATE_REG_ADDR)
    himm(UPGRADE_VERSION_REG_ADDR, s_stUPGRADECtx.unVersion.stVerType1.u32Version);
    himm(UPGRADE_BUILDDATE_REG_ADDR, s_stUPGRADECtx.unVersion.stVerType1.u32BuildDate);
    MLOGD("[%#08x] %#08x\n", UPGRADE_VERSION_REG_ADDR, s_stUPGRADECtx.unVersion.stVerType1.u32Version);
    MLOGD("[%#08x] %#08x\n", UPGRADE_BUILDDATE_REG_ADDR, s_stUPGRADECtx.unVersion.stVerType1.u32BuildDate);
#endif
    MLOGD(GREEN"Finish!\n"NONE"");
    return HI_SUCCESS;
}

#ifndef __HuaweiLite__
static HI_S32 UPGRADE_GetBootArgsByProc(HI_CHAR* pszBootargs, HI_U32 u32Len)
{
    FILE* fp = NULL;
    fp = fopen("/proc/cmdline", "r");

    if (fp != NULL)
    {
        if (!feof(fp))
        {
            while (!fgets(pszBootargs, (u32Len - 1), fp))
            {
                MLOGD("Bootargs[%s]\n", pszBootargs);
                break;
            }
        }

        fclose(fp);
        return HI_SUCCESS;
    }
    else
    {
        MLOGE("fopen /proc/cmdline failed!\n");
        return HI_FAILURE;
    }
}
#else
static HI_S32 UPGRADE_GetBootArgsByPkt(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo, HI_CHAR* pszBootargs, HI_U32 u32Len)
{
    HI_APPCOMM_CHECK_POINTER(pstDevInfo, HI_UPGRADE_EINVAL);
    HI_S32 s32PktFd = -1;
    UPGRADE_PKT_INFO_S stPktInfo;
    DIR* pstDir = NULL;
    struct dirent* pstDirItem = NULL;
    pstDir = opendir(pszPktPath);
    if (NULL == pstDir)
    {
        MLOGD("%s is not a directory(%s)\n", pszPktPath, strerror(errno));
        return HI_UPGRADE_ELOST;
    }
    memset(&stPktInfo, 0, sizeof(UPGRADE_PKT_INFO_S));
    snprintf(stPktInfo.szModel, HI_APPCOMM_COMM_STR_LEN, "%s", pstDevInfo->szModel);

    pstDirItem = readdir(pstDir);
    while(NULL != pstDirItem)
    {
        if (UPGRADE_CheckPktNameValid(pstDirItem->d_name))
        {
            snprintf(stPktInfo.szPktFilePath, HI_UPGRADE_PKT_PATH_MAX_LEN,
                "%s/%s", pszPktPath, pstDirItem->d_name);
            if ((HI_SUCCESS == UPGRADE_CheckPktValid(&stPktInfo))
                && (0 == UPGRADE_CompareVersion(stPktInfo.szSoftVersion, pstDevInfo->szSoftVersion)))
            {
                s32PktFd = open(stPktInfo.szPktFilePath, O_RDONLY);
                break;
            }
        }

        pstDirItem = readdir(pstDir);
    }
    closedir(pstDir);

    if (s32PktFd < 0)
    {
        MLOGE("open PktFile failed\n");
        return HI_UPGRADE_EINTR;
    }

    /* Read Pkt Head Info */
    HI_UPGRADE_PKT_HEAD_S stPktHead;
    HI_S32 s32Ret = UPGRADE_ReadPktHead(s32PktFd, &stPktHead);
    close(s32PktFd);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("UPGRADE_ReadPktHead failed\n");
        return HI_UPGRADE_EINTR;
    }
    snprintf(pszBootargs,u32Len,"%s",stPktHead.szBootArgs);
    return HI_SUCCESS;
}
#endif

static HI_S32 UPGRADE_GetBootArgs(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo, HI_CHAR* pszBootargs, HI_U32 u32Len)
{
#ifndef __HuaweiLite__
    return UPGRADE_GetBootArgsByProc(pszBootargs, u32Len);
#else
    return UPGRADE_GetBootArgsByPkt(pszPktPath, pstDevInfo, pszBootargs, u32Len);
#endif
}

static HI_S32 UPGRADE_GetMtdPartInfo(const HI_CHAR* pszBootargs, HI_UPGRADE_MTDPART_INFO_S* pstMtdPartInfo)
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
    MLOGI(YELLOW"MtdInfo[%s]"NONE"\n", szMtdPartInfo);

    HI_CHAR* pSave = NULL;
    HI_CHAR* pToken = NULL;
    HI_CHAR* pTokenStart = NULL;
    HI_CHAR* pTokenEnd = NULL;
    memset(pstMtdPartInfo, 0, sizeof(HI_UPGRADE_MTDPART_INFO_S));

    for (pszStr = szMtdPartInfo; ; pszStr = NULL)
    {
        pToken = strtok_r(pszStr, ",", &pSave);

        if (!pToken)
        {
            break;
        }

        pTokenStart = strchr(pToken, '(');
        pTokenEnd = strchr(pToken, ')');

        if (!pTokenStart || !pTokenEnd)
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

/* DEL SD_upgrade FILES  */
HI_S32 HI_UPGRADE_DelUpgradeFiles(const HI_CHAR* pszPktPath, const HI_UPGRADE_DEV_INFO_S* pstDevInfo)
{
    HI_APPCOMM_CHECK_POINTER(pszPktPath, HI_UPGRADE_EINVAL);
    UPGRADE_CHECK_INIT(s_stUPGRADECtx.bInit);
    HI_S32  s32Ret = HI_SUCCESS;

    HI_CHAR szBootArgs[HI_UPGRADE_MAX_ENV_LEN] = {0,};
    s32Ret = UPGRADE_GetBootArgs(pszPktPath, pstDevInfo, szBootArgs, HI_UPGRADE_MAX_ENV_LEN);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGW("GetBootArgs failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_UPGRADE_MTDPART_INFO_S stMtdPartInfo;
    s32Ret = UPGRADE_GetMtdPartInfo(szBootArgs, &stMtdPartInfo);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGW("GetMtdInfo failed\n");
        return HI_UPGRADE_EINTR;
    }

    HI_CHAR szImgPath[HI_APPCOMM_MAX_PATH_LEN] = {0,};
    HI_S32 i;

    for (i = 0; i < stMtdPartInfo.s32PartitionCnt; ++i)
    {
        snprintf(szImgPath, HI_APPCOMM_MAX_PATH_LEN, "%s/%s", pszPktPath, stMtdPartInfo.aszPartitionName[i]);

        if (0 == access(szImgPath, F_OK))
        {
            (HI_VOID)remove(szImgPath);
            MLOGI(GREEN"Delete image file[%s]"NONE"\n", szImgPath);
        }
        else
        {
            MLOGI(YELLOW"image file[%s] not exist, ignore"NONE"\n", szImgPath);
        }
    }

    snprintf(szImgPath, HI_APPCOMM_MAX_PATH_LEN, "%s/config", pszPktPath);

    if (0 == access(szImgPath, F_OK))
    {
        (HI_VOID)remove(szImgPath);
        MLOGI(GREEN"Delete config file[%s]"NONE"\n", szImgPath);
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

