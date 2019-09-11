#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "hi_flash.h"
#include "../HuaweiLite/spi_raw.h"
#include "hi_appcomm_log.h"
#define MAX_PARTS         2    /* Flash max partition number*/
#define MAX_HANDLE        MAX_PARTS    /* Flash max handle number */
#define MAX_BOOTARGS_LEN  1024  /* The max length of bootargs */

#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "HiFlash"

/* Expand hiflash handle fd, only HI_Flash_OpenByTypeAndName() use it */
#define  SPAN_PART_HANDLE 1000
#define HI_ERR_FLASH(fmt...) MLOGE(fmt)
#define HI_INFO_FLASH(fmt...) MLOGD(fmt)

/** Flash Infomation */
typedef struct
{
    HI_U64  TotalSize;                  /**< flash total size */
    HI_U64  PartSize;                   /**< flash partition size */
    HI_U32  BlockSize;                  /**< flash block size */
    HI_U32  PageSize;                   /**< flash page size */
    HI_U32  OobSize;                    /**< flash OOB size */
    HI_VOID* fd;                        /**< file handle */
    HI_U64  OpenAddr;                   /**< flash open address */
    HI_U64  OpenLeng;                   /**< flash open length */
    HI_FLASH_TYPE_E FlashType;          /**< flash type */
    FLASH_OPT_S* pFlashopt;             /**< operation callbacks on this flash */
    HI_Flash_PartInfo_S* pPartInfo;     /**< parition descriptions on this flash */
} HI_FLASH_INTER_INFO_S;

typedef enum hi_flash_dev_stat
{
    HI_FLASH_STAT_INSTALL,
    HI_FLASH_STAT_UNINSTALL,
    /*lint -save -e749 */
    HI_FLASH_STAT_BUTT
} HI_FLASH_DEV_STAT;

extern HI_S32 find_flash_part(HI_CHAR* cmdline_string,
                              const HI_CHAR* media_name,  /* hi_sfc, hinand */
                              HI_CHAR* ptn_name,
                              HI_U64* start,
                              HI_U64* length);
extern HI_S32 find_part_from_devname(HI_CHAR* media_name, HI_CHAR* bootargs,
                                     HI_CHAR* devname, HI_U64* start, HI_U64* size);

static HI_FLASH_INTER_INFO_S gFlashInfo[MAX_HANDLE];
static HI_Flash_PartInfo_S gPartInfo[MAX_PARTS];
static HI_BOOL gInitFlag = HI_FALSE;
static volatile HI_BOOL gInitMutex = HI_FALSE;

static HI_CHAR* pszPos[HI_FLASH_TYPE_BUTT];
static HI_FLASH_DEV_STAT g_eDevStat[HI_FLASH_TYPE_BUTT];
static pthread_mutex_t gFlashMutex;

static FLASH_OPT_S g_stFlashOpt[HI_FLASH_TYPE_BUTT];

//extern EMMC_FLASH_S g_stEmmcFlash;

#define CHECK_FLASH_INIT(hFlash) \
    do \
    { \
        if (!gInitFlag) \
        { \
            HI_ERR_FLASH("NOT init yet!\n"); \
            return HI_FAILURE; \
        } \
        if (MAX_HANDLE <= hFlash) \
        { \
            return HI_FAILURE; \
        } \
        if ((HI_VOID *)INVALID_FD == gFlashInfo[hFlash].fd) \
        { \
            return HI_FAILURE; \
        } \
    } while (0)

#define CHECK_ADDR_LEN_VALID(Address, Len, LimitLeng) \
    do \
    { \
        if ((Address >= LimitLeng) || ((Address + Len) > LimitLeng)) \
        { \
            HI_ERR_FLASH("startaddr(0x%llX) + length(0x%llx) or startaddr should be smaller than partsize(0x%llX)\n", Address, Len, LimitLeng); \
            (HI_VOID)pthread_mutex_unlock(&gFlashMutex); \
            return HI_FAILURE; \
        } \
    } while (0)




HI_S32 permission_check(HI_FLASH_TYPE_E enFlashType,
                        HI_U64 u64StartAddress,
                        HI_U64 u64Len)
{
    HI_U64 u64EndAddress = u64StartAddress + u64Len - 1;
    HI_U32 i;

    for (i = 0; i < MAX_PARTS; i++)
    {
        if (gPartInfo[i].FlashType != enFlashType)
        { continue; }

        if (gPartInfo[i].perm != ACCESS_NONE)
        { continue; }

        if ((gPartInfo[i].StartAddr >= u64StartAddress) && (gPartInfo[i].StartAddr <= u64EndAddress))
        {
            HI_INFO_FLASH("%s(%s) is not permitted to be opened.\n", gPartInfo[i].DevName, gPartInfo[i].PartName);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 Flash_Init(HI_VOID)
{
    HI_U32 i = 0;

    for (i = 0; i < MAX_PARTS; i++)
    {
        gPartInfo[i].StartAddr = 0;
        gPartInfo[i].PartSize = 0;
        gPartInfo[i].BlockSize = 0;
        gPartInfo[i].FlashType = HI_FLASH_TYPE_BUTT;
        gPartInfo[i].perm = ACCESS_BUTT;
        memset(gPartInfo[i].DevName, '\0', FLASH_NAME_LEN);
        memset(gPartInfo[i].PartName, '\0', FLASH_NAME_LEN);
    }

    for (i = 0; i < MAX_HANDLE; i++)
    {
        (HI_VOID)pthread_mutex_lock(&gFlashMutex);
        gFlashInfo[i].fd = (HI_VOID*)INVALID_FD;
        gFlashInfo[i].OpenAddr = 0;
        gFlashInfo[i].OpenLeng = 0;
        gFlashInfo[i].pPartInfo = NULL;
        gFlashInfo[i].FlashType = HI_FLASH_TYPE_BUTT;
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
    }

    if ( (HI_FLASH_STAT_INSTALL == g_eDevStat[HI_FLASH_TYPE_SPI_0])
         || (HI_FLASH_STAT_INSTALL == g_eDevStat[HI_FLASH_TYPE_NAND_0]))
    {
        g_stFlashOpt[HI_FLASH_TYPE_SPI_0].raw_erase = spi_raw_erase;
        g_stFlashOpt[HI_FLASH_TYPE_SPI_0].raw_read  = spi_raw_read;
        g_stFlashOpt[HI_FLASH_TYPE_SPI_0].raw_write = spi_raw_write;
        //g_stFlashOpt[HI_FLASH_TYPE_NAND_0].raw_erase = nand_raw_erase;
        //g_stFlashOpt[HI_FLASH_TYPE_NAND_0].raw_read  = nand_raw_read;
        //g_stFlashOpt[HI_FLASH_TYPE_NAND_0].raw_write = nand_raw_write;
    }

    return HI_SUCCESS;
}

static HI_S32 All_FLash_Init(HI_VOID)
{
    HI_U8 u8Loop = 0;

    if (gInitMutex)
    {
        while (gInitMutex && (u8Loop < 100))
        {
            usleep(100);
            u8Loop ++;
        }
    }
    else
    {
        gInitMutex = HI_TRUE;
    }

    if (!gInitFlag)
    {
        (HI_VOID)pthread_mutex_init(&gFlashMutex, NULL);
        for ( u8Loop = 0; u8Loop < HI_FLASH_TYPE_BUTT; u8Loop++)
        {
            g_eDevStat[u8Loop] = HI_FLASH_STAT_UNINSTALL;
        }

        if ( HI_SUCCESS == spi_raw_init())
        {
            g_eDevStat[HI_FLASH_TYPE_SPI_0] = HI_FLASH_STAT_INSTALL;
        }
        else if (HI_NULL != pszPos[HI_FLASH_TYPE_SPI_0])
        {
            HI_ERR_FLASH("spi init fail! \n");
            gInitMutex = HI_FALSE;
            return HI_FAILURE;
        }

        //if ( HI_SUCCESS == nand_raw_init())
        //{
        //    g_eDevStat[HI_FLASH_TYPE_NAND_0] = HI_FLASH_STAT_INSTALL;
        //}
        //else if (HI_NULL != pszPos[HI_FLASH_TYPE_NAND_0])
        //{
        //    (HI_VOID)spi_raw_destroy();
        //    HI_ERR_FLASH("nand init fail! \n");
        //    gInitMutex = HI_FALSE;
        //    return HI_FAILURE;
        //}

        if (Flash_Init())
        {
            (HI_VOID)spi_raw_destroy();
            //(HI_VOID)nand_raw_destroy();
            HI_ERR_FLASH("Flash init fail! \n");
            gInitMutex =  HI_FALSE;
            return HI_FAILURE;
        }
    }

    gInitFlag = HI_TRUE;
    gInitMutex = HI_FALSE;

    return HI_SUCCESS;
}

HI_S32 HI_Flash_UnusedHandle(HI_VOID)
{
    HI_S8 u8Loop;

    for (u8Loop = 0; u8Loop < MAX_HANDLE; u8Loop++)
    {
        if ((HI_VOID*)INVALID_FD == gFlashInfo[u8Loop].fd)
        {
            break;
        }
    }

    return u8Loop;
}

HI_HANDLE HI_Flash_OpenByTypeAndAddr(HI_FLASH_TYPE_E enFlashType,
                                     HI_U64 u64Address,
                                     HI_U64 u64Len)
{
    HI_U64  TotalSize = 0;
    HI_U32  PageSize = 0;
    HI_U32  BlockSize = 0;
    HI_U32  OobSize = 0;
    HI_U32  BlockShift = 0;
    HI_U32  hFlash = 0;

    if (All_FLash_Init())
    {
        return (HI_HANDLE)INVALID_FD;
    }

    if (HI_FLASH_TYPE_BUTT <= enFlashType)
    {
        HI_ERR_FLASH("FlashType error! \n");
        return (HI_HANDLE)INVALID_FD;
    }

    if ( (HI_FLASH_STAT_INSTALL != g_eDevStat[enFlashType]))
    {
        HI_ERR_FLASH("No config flash[type:%d].", enFlashType);
        return (HI_HANDLE)INVALID_FD;
    }

    (HI_VOID)pthread_mutex_lock(&gFlashMutex);

    if ( HI_FLASH_TYPE_EMMC_0 == enFlashType )
    {
        HI_ERR_FLASH("unsupport emmc flash.");
    }
    else if (HI_FLASH_TYPE_SPI_0 == enFlashType)
    {
        spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
    }
    else
    {
        //nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
        HI_ERR_FLASH("unsupport nand flash.");
    }

    if (0 == BlockSize)
    {
        HI_ERR_FLASH("BlockSize shouldn't equal 0!\n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return (HI_HANDLE)INVALID_FD;
    }

    if ((HI_S32)(u64Address % BlockSize) || (HI_S32)(u64Len % BlockSize))
    {
        HI_ERR_FLASH("Open Address(%#llx) and Len(%#llx) should be align with BlockSize(0x%X)!\n",
                     u64Address, u64Len, BlockSize);
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return (HI_HANDLE)INVALID_FD;
    }

    if ((u64Address >= TotalSize) || (u64Address + u64Len) > TotalSize)
    {
        HI_ERR_FLASH("Open Address(%#llx) and Len(%#llx) should be smaller than TotalSize(0x%llX)!\n",
                     u64Address, u64Len, TotalSize);
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return (HI_HANDLE)INVALID_FD;
    }

    if (HI_SUCCESS != permission_check(enFlashType, u64Address, u64Len))
    {
        HI_INFO_FLASH("not permission to be opened.\n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return (HI_HANDLE)INVALID_FD;
    }

    if (MAX_HANDLE == (hFlash = (HI_U32)HI_Flash_UnusedHandle()))
    {
        HI_ERR_FLASH("flash array full! \n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return (HI_HANDLE)INVALID_FD;
    }

    gFlashInfo[hFlash].fd = (HI_SL*)(HI_SL)(SPAN_PART_HANDLE + hFlash);
    gFlashInfo[hFlash].OpenAddr = u64Address;
    gFlashInfo[hFlash].OpenLeng = u64Len;
    gFlashInfo[hFlash].pPartInfo = NULL;
    gFlashInfo[hFlash].FlashType = enFlashType;
    gFlashInfo[hFlash].PageSize = PageSize;
    gFlashInfo[hFlash].OobSize = OobSize;
    gFlashInfo[hFlash].BlockSize = BlockSize;

    if (HI_FLASH_TYPE_SPI_0 == enFlashType)
    {
        gFlashInfo[hFlash].pFlashopt = (FLASH_OPT_S*)&g_stFlashOpt[HI_FLASH_TYPE_SPI_0];
    }

    if (HI_FLASH_TYPE_NAND_0 == enFlashType)
    {
        gFlashInfo[hFlash].pFlashopt = (FLASH_OPT_S*)&g_stFlashOpt[HI_FLASH_TYPE_NAND_0];
    }

    HI_INFO_FLASH("fd= %ld, OpenAddress= %#llx, OpenLen= %#llx \n", (long)gFlashInfo[hFlash].fd, u64Address, u64Len);
    HI_INFO_FLASH("end.\n");
    (HI_VOID)pthread_mutex_unlock(&gFlashMutex);

    return (HI_HANDLE)hFlash;
}

HI_HANDLE HI_Flash_Open(HI_FLASH_TYPE_E enFlashType, HI_CHAR* pPartitionName, HI_U64 u64Address, HI_U64 u64Len)
{
    //HI_INFO_FLASH("FlashType=%d, PartitionName=%s, Address=0x%llx, Len=0x%llx\n", enFlashType, pPartitionName, u64Address, u64Len);
    return HI_Flash_OpenByTypeAndAddr(enFlashType, u64Address, u64Len);
}


HI_S32 HI_Flash_Close(HI_HANDLE hFlash)
{
    HI_U32 i;

    CHECK_FLASH_INIT(hFlash);


    (HI_VOID)pthread_mutex_lock(&gFlashMutex);
    HI_INFO_FLASH("fd = %p\n", gFlashInfo[hFlash].fd);

    if ((HI_VOID*)SPAN_PART_HANDLE <= gFlashInfo[hFlash].fd)
    {
        gFlashInfo[hFlash].fd = (HI_S64*)(HI_S64)INVALID_FD;
        gFlashInfo[hFlash].OpenAddr = 0;
        gFlashInfo[hFlash].OpenLeng = 0;
        gFlashInfo[hFlash].pPartInfo = NULL;
    }
    else
    {
        gFlashInfo[hFlash].fd = (HI_S64*)(HI_S64)INVALID_FD;
        gFlashInfo[hFlash].OpenAddr = 0;
        gFlashInfo[hFlash].OpenLeng = 0;
        gFlashInfo[hFlash].pPartInfo = NULL;
    }

    for (i = 0; i < MAX_HANDLE; i++)
    {
        if ((HI_VOID*)INVALID_FD != gFlashInfo[i].fd)
        {
            break;
        }
    }

    if (MAX_HANDLE == i)
    {
        (HI_VOID)spi_raw_destroy();
        //(HI_VOID)nand_raw_destroy();
        gInitFlag = HI_FALSE;
    }

    HI_INFO_FLASH("end.\n");
    (HI_VOID)pthread_mutex_unlock(&gFlashMutex);

    return HI_SUCCESS;
}



HI_S32 HI_Flash_Erase(HI_HANDLE hFlash, HI_U64 u64Address, HI_U64 u64Len)
{
    HI_U64 StartAddr = 0;
    HI_U64 LimitLeng = 0;
    HI_S32 ret;

    /* u64Len bigger than 2GB should use HI_Flash_Erase64 instead. */
    if (u64Len >= 0x80000000)
    {
        HI_ERR_FLASH("u64Len is too big(0x%llx), should use HI_Flash_Erase64() instead.\n", u64Len);
        return HI_FAILURE;
    }

    CHECK_FLASH_INIT(hFlash);

    if ( HI_FLASH_TYPE_EMMC_0 == gFlashInfo[hFlash].FlashType)
    {
#if defined (HI_EMC_ERASE_SUPPORT)
        //HI_PRINT("HI_Flash_Erase -> emc_raw_erase\nu64Address=0x%08llx, u64Len=0x%08llx\n", u64Address, u64Len);
        return emc_raw_erase((EMMC_CB_S*)gFlashInfo[hFlash].fd, u64Address, u64Len);
#else
        return HI_SUCCESS;
#endif
    }

    (HI_VOID)pthread_mutex_lock(&gFlashMutex);

    if ((HI_VOID*)SPAN_PART_HANDLE <= gFlashInfo[hFlash].fd)
    {
        StartAddr = gFlashInfo[hFlash].OpenAddr;
        LimitLeng = gFlashInfo[hFlash].OpenLeng;
    }
    else
    {
        StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
        LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
    }
    CHECK_ADDR_LEN_VALID(u64Address, u64Len, LimitLeng);

    StartAddr += u64Address;
    HI_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%llx\n", hFlash, StartAddr, u64Len);

    if (!gFlashInfo[hFlash].pFlashopt->raw_erase)
    {
        HI_ERR_FLASH("flash service function ptr(raw_erase) is NULL! \n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return HI_FAILURE;
    }

    ret = (HI_S32)gFlashInfo[hFlash].pFlashopt->raw_erase((HI_S32)(HI_SL)gFlashInfo[hFlash].fd, (unsigned long long)StartAddr, u64Len, gFlashInfo[hFlash].OpenAddr, LimitLeng);
    HI_INFO_FLASH("end.\n");
    (HI_VOID)pthread_mutex_unlock(&gFlashMutex);

    return ret;
}



HI_S32 HI_Flash_Read(HI_HANDLE hFlash, HI_U64 u64Address, HI_U8* pBuf, HI_U32 u32Len, HI_U32 u32Flags)
{
    HI_U64 StartAddr = 0;
    HI_U64 LimitLeng = 0;
    HI_S32 ret, wtihoob;

    if (NULL == pBuf)
    {
        return HI_FAILURE;
    }

    CHECK_FLASH_INIT(hFlash);

    (HI_VOID)pthread_mutex_lock(&gFlashMutex);

    if ((HI_VOID*)SPAN_PART_HANDLE <= gFlashInfo[hFlash].fd)
    {
        StartAddr = gFlashInfo[hFlash].OpenAddr;
        LimitLeng = gFlashInfo[hFlash].OpenLeng;
    }
    else
    {
        StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
        LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
    }

    if (HI_FLASH_RW_FLAG_WITH_OOB == (u32Flags & HI_FLASH_RW_FLAG_WITH_OOB))
    {
        HI_U32 u32LenWithoutOOB = (u32Len
                                   / (gFlashInfo[hFlash].OobSize
                                      + gFlashInfo[hFlash].PageSize))
                                  * gFlashInfo[hFlash].PageSize;

        if (u32Len % (gFlashInfo[hFlash].OobSize
                      + gFlashInfo[hFlash].PageSize))
        { u32LenWithoutOOB += gFlashInfo[hFlash].PageSize; }

        CHECK_ADDR_LEN_VALID(u64Address, (HI_U64)u32LenWithoutOOB, LimitLeng);
    }
    else
    {
        CHECK_ADDR_LEN_VALID(u64Address, (HI_U64)u32Len, LimitLeng);
    }

    StartAddr += u64Address;

    HI_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%x, Flag=%d\n", hFlash, StartAddr, u32Len, u32Flags);

    if (HI_FLASH_RW_FLAG_WITH_OOB == (u32Flags & HI_FLASH_RW_FLAG_WITH_OOB))
    {
        wtihoob = 1;
    }
    else
    {
        wtihoob = 0;
    }

    if (!gFlashInfo[hFlash].pFlashopt->raw_read)
    {
        HI_ERR_FLASH("flash service function ptr(raw_read) is NULL! \n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return HI_FAILURE;
    }

    ret = gFlashInfo[hFlash].pFlashopt->raw_read((HI_S32)(HI_SL)gFlashInfo[hFlash].fd, (unsigned long long*)&StartAddr, pBuf, u32Len, gFlashInfo[hFlash].OpenAddr, LimitLeng, wtihoob, 1);
    HI_INFO_FLASH("totalread =0x%x, end.\n", ret);
    (HI_VOID)pthread_mutex_unlock(&gFlashMutex);

    return ret;
}

HI_S32 HI_Flash_Write(HI_HANDLE hFlash, HI_U64 u64Address, HI_U8* pBuf, HI_U32 u32Len, HI_U32 u32Flags)
{
    HI_U64 StartAddr = 0;
    HI_U64 LimitLeng = 0;
    HI_S32 ret, wtihoob, erase;

    HI_U64 TotalSize = 0;
    HI_U32 PageSize = 0;
    HI_U32 BlockSize = 0;
    HI_U32 OobSize = 0;
    HI_U32 BlockShift = 0;

    HI_U32 u32EraseLen = 0;
    HI_U32 BlockSize_new = 0;

    if (NULL == pBuf)
    {
        return HI_FAILURE;
    }

    CHECK_FLASH_INIT(hFlash);
    (HI_VOID)pthread_mutex_lock(&gFlashMutex);



    if ((HI_VOID*)SPAN_PART_HANDLE <= gFlashInfo[hFlash].fd)
    {
        StartAddr = gFlashInfo[hFlash].OpenAddr;
        LimitLeng = gFlashInfo[hFlash].OpenLeng;
    }
    else
    {
        StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
        LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
    }
    if (HI_FLASH_RW_FLAG_WITH_OOB == (u32Flags & HI_FLASH_RW_FLAG_WITH_OOB))
    {
        HI_U32 u32LenWithoutOOB = (u32Len
                                   / (gFlashInfo[hFlash].OobSize
                                      + gFlashInfo[hFlash].PageSize))
                                  * gFlashInfo[hFlash].PageSize;

        if (u32Len % (gFlashInfo[hFlash].OobSize
                      + gFlashInfo[hFlash].PageSize))
        { u32LenWithoutOOB += gFlashInfo[hFlash].PageSize; }
        CHECK_ADDR_LEN_VALID(u64Address, (HI_U64)u32LenWithoutOOB, LimitLeng);
    }
    else
    {
        CHECK_ADDR_LEN_VALID(u64Address, (HI_U64)u32Len, LimitLeng);
    }

    StartAddr += u64Address;
    HI_INFO_FLASH("HANDLE=%d, Address=0x%llx, Len=0x%x, Flag=%d\n", hFlash, StartAddr, u32Len, u32Flags);

    if (HI_FLASH_TYPE_SPI_0 == gFlashInfo[hFlash].FlashType)
    {
        spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
    }
    else if (HI_FLASH_TYPE_NAND_0 == gFlashInfo[hFlash].FlashType)
    {
        //nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
        HI_ERR_FLASH("unsupport nand flash.");
    }

    if (HI_FLASH_RW_FLAG_ERASE_FIRST == (u32Flags & HI_FLASH_RW_FLAG_ERASE_FIRST))
    {
        erase = 1;
    }
    else
    {
        erase = 0;
    }
    /* avoid pclint div 0 warning */
    if (!PageSize)
    {
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return -1;
    }

    if (HI_FLASH_RW_FLAG_WITH_OOB == (u32Flags & HI_FLASH_RW_FLAG_WITH_OOB))
    {
        BlockSize_new = BlockSize + OobSize * (BlockSize / PageSize);
        wtihoob = 1;
    }
    else
    {
        BlockSize_new = BlockSize;
        wtihoob = 0;
    }

    if (erase)
    {
        /* avoid pclint div 0 warning */
        if (!BlockSize_new)
        {
            (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
            return -1;
        }

        u32EraseLen = u32Len / BlockSize_new;

        if (u32Len % BlockSize_new)
        {
            u32EraseLen += 1;
        }
        u32EraseLen = u32EraseLen * BlockSize;
        //HI_PRINT("> %s: [%d], u32EraseLen=%#x\n", __FUNCTION__, __LINE__, u32EraseLen);

        if (!gFlashInfo[hFlash].pFlashopt->raw_erase)
        {
            HI_ERR_FLASH("flash service function ptr(raw_erase) is NULL! \n");
            (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
            return HI_FAILURE;
        }
        ret = gFlashInfo[hFlash].pFlashopt->raw_erase((HI_SL)gFlashInfo[hFlash].fd, (unsigned long long)StartAddr, (unsigned long long)u32EraseLen, gFlashInfo[hFlash].OpenAddr, LimitLeng);
        if (0 >= ret)
        {
            if (HI_FLASH_END_DUETO_BADBLOCK != ret)
            {
                HI_ERR_FLASH("earse fail!\n");
                (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
                return ret;
            }
        }
    }

    if (!gFlashInfo[hFlash].pFlashopt->raw_write)
    {
        HI_ERR_FLASH("flash service function ptr(raw_write) is NULL! \n");
        (HI_VOID)pthread_mutex_unlock(&gFlashMutex);
        return HI_FAILURE;
    }

    ret = gFlashInfo[hFlash].pFlashopt->raw_write((HI_S32)(HI_SL)gFlashInfo[hFlash].fd, &StartAddr, pBuf, u32Len, gFlashInfo[hFlash].OpenAddr, LimitLeng, wtihoob);
    HI_INFO_FLASH("totalwrite =0x%x, end.\n", ret);
    (HI_VOID)pthread_mutex_unlock(&gFlashMutex);

    return ret;
}

HI_S32 HI_Flash_GetInfo(HI_HANDLE hFlash, HI_Flash_InterInfo_S* pFlashInfo)
{
    HI_U64 TotalSize = 0;
    HI_U32 PageSize = 0;
    HI_U32 BlockSize = 0;
    HI_U32 OobSize = 0;
    HI_U32 BlockShift = 0;

    if (NULL == pFlashInfo)
    {
        return HI_FAILURE;
    }

    CHECK_FLASH_INIT(hFlash);



    switch (gFlashInfo[hFlash].FlashType)
    {
        case HI_FLASH_TYPE_SPI_0:
        {
            spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
            break;
        }

        case HI_FLASH_TYPE_NAND_0:
        {
            //nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
            HI_ERR_FLASH("unsupport nand flash.");
            break;
        }

        default :
        {
            break;
        }
    }

    pFlashInfo->TotalSize = TotalSize;

    if ((HI_VOID*)SPAN_PART_HANDLE <= gFlashInfo[hFlash].fd)
    {
        pFlashInfo->PartSize = gFlashInfo[hFlash].OpenLeng;
        pFlashInfo->pPartInfo = NULL;
    }
    else
    {
        pFlashInfo->PartSize = gFlashInfo[hFlash].pPartInfo->PartSize;
        pFlashInfo->pPartInfo = gFlashInfo[hFlash].pPartInfo;
    }

    pFlashInfo->BlockSize   = BlockSize;
    pFlashInfo->PageSize    = PageSize;
    pFlashInfo->OobSize     = OobSize;
    pFlashInfo->fd          = -1;
    pFlashInfo->FlashType   = gFlashInfo[hFlash].FlashType;
    pFlashInfo->pFlashopt   = NULL;
    pFlashInfo->OpenAddr    = gFlashInfo[hFlash].OpenAddr;
    pFlashInfo->OpenLeng    = gFlashInfo[hFlash].OpenLeng;

    return HI_SUCCESS;
}


