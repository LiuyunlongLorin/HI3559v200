/******************************************************************************
  Copyright (C), 2004-2014, Hisilicon Tech. Co., Ltd.
******************************************************************************
  File Name     : hi_flash.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Last Modified :
  Description   : header file for hiflash component
  Function List :
  History       :
  1.Date        :
  Author        :
  Modification  : Created file
******************************************************************************/

/**
 * \file
 * \brief describle the information about hiflash component. CNcomment:  Provide interface and data structs of hiflash tools
 * \attention \n
 * DO NOT write/erase flash too heavily using this hiflash interface because hiflash do not support "load balancing", use filesystems or implement "load balancing" instead if you have to. read is limitless.\n
 *       for example: if max write/erase times of MLC Nand chip is 3000, write/erase any fixed area must not exceed 3000 times, or read data may be wrong. \n\n
 * CNcomment:   DO NOT use hiflash function write/erase/flash data too frequently, cause hiflash DO NOT support read-write equalization. If need to do so, please use FileSystem or use  read-write equalization algorithm. Read action don't have the limit.
 */

#ifndef __HI_FLASH_H__
#define __HI_FLASH_H__
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      FLASH*/
/** @{ */  /** <!-- [FLASH] */

#define FLASH_NAME_LEN 32       /** Flash Name max length */

/** flash type */
typedef enum hiHI_FLASH_TYPE_E
{
    HI_FLASH_TYPE_SPI_0,    /**< SPI flash type */
    HI_FLASH_TYPE_NAND_0,   /**< NAND flash type */
    HI_FLASH_TYPE_EMMC_0,   /**< eMMC flash type */
    HI_FLASH_TYPE_BUTT      /**< Invalid flash type */
} HI_FLASH_TYPE_E;

#define  INVALID_FD -1

#define  HI_FLASH_RW_FLAG_RAW           0x0   /** read and write without OOB,for example: kernel/uboot/ubi/cramfs.. */
#define  HI_FLASH_RW_FLAG_WITH_OOB      0x1   /** read and write with OOB, example: yaffs2 filesystem image */
#define  HI_FLASH_RW_FLAG_ERASE_FIRST   0x2   /** erase before write */

/** this macro for return value when nand flash have bad block or valid length less partition length */
/** CNcomment:When nand have bad block. Do write/read/erase action may let effective length smaller than partition size or open length. So after read/write/erase effective length, return the value*/
#define  HI_FLASH_END_DUETO_BADBLOCK    -10

/** Flash partition access permission type */
typedef enum ACCESS_PERM
{
    ACCESS_NONE = 0,
    ACCESS_RD   = (1 << 1),
    ACCESS_WR   = (1 << 2),
    ACCESS_RDWR = (ACCESS_RD | ACCESS_WR),
    ACCESS_BUTT
} HI_FLASH_ACCESS_PERM_E;

/** Flash partition descriptions */
typedef struct hiFlash_PartInfo_S
{
    HI_U64  StartAddr;                  /**< Partiton start address */
    HI_U64  PartSize;                   /**< Partition size */
    HI_U32  BlockSize;                  /**< The Block size of the flash where this partition at */
    HI_FLASH_TYPE_E FlashType;          /**< The flash type where this partition at */
    HI_CHAR DevName[FLASH_NAME_LEN];    /**< The device node name where this partition relate to */
    HI_CHAR PartName[FLASH_NAME_LEN];   /**< The partition name of this partition */
    HI_FLASH_ACCESS_PERM_E perm;        /**< The partition access permission type */
} HI_Flash_PartInfo_S;

/** Flash operation descriptions */
typedef struct tagFLASH_OPT_S
{
    HI_S32 (*raw_read)(HI_S32 fd, HI_U64* startaddr, HI_U8* buffer, HI_UL length,
                       HI_U64 openaddr, HI_U64 limit_leng, HI_S32 read_oob, HI_S32 skip_badblock);
    HI_S32 (*raw_write)(HI_S32 fd, HI_U64* startaddr, HI_U8* buffer, HI_UL length,
                        HI_U64 openaddr, HI_U64 limit_leng, HI_S32 write_oob);
    HI_S64 (*raw_erase)(HI_S32 fd, HI_U64 startaddr, HI_U64 length, HI_U64 openaddr, HI_U64 limit_leng);
} FLASH_OPT_S;

/** Flash Infomation */
typedef struct hiFlash_InterInfo_S
{
    HI_U64  TotalSize;                  /**< flash total size */
    HI_U64  PartSize;                   /**< flash partition size */
    HI_U32  BlockSize;                  /**< flash block size */
    HI_U32  PageSize;                   /**< flash page size */
    HI_U32  OobSize;                    /**< flash OOB size */
    HI_S32  fd;                         /**< file handle */ /**<CNcomment:file handle, open by addr could not get true handle*/
    HI_U64  OpenAddr;                   /**< flash open address */
    HI_U64  OpenLeng;                   /**< flash open length */
    HI_FLASH_TYPE_E FlashType;          /**< flash type */
    FLASH_OPT_S* pFlashopt;             /**< operation callbacks on this flash */
    HI_Flash_PartInfo_S* pPartInfo;     /**< parition descriptions on this flash */
} HI_Flash_InterInfo_S;

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      FLASH */
/** @{ */  /** <!-- [FLASH] */


/**
\brief: open flash partiton
\attention \n
\param[in] enFlashType      Flash type
\param[in] pPartitionName   CNcomment:    None EMMC device (such as SPI/NAND) can only use /dev/mtdx to be partition name. EMMC device can only use /dev/mmcblkopx to be partition name.
\param[in] u64Address       open address
\param[in] u64Len             open length
\retval    fd                      Flash handle
\retval    INVALID_FD         invaild fd
\see \n
*/
HI_HANDLE HI_Flash_Open(HI_FLASH_TYPE_E enFlashType, HI_CHAR* pPartitionName, HI_U64 u64Address, HI_U64 u64Len);
/**
\brief:  open flash patition by name
\attention \n
\param[in] pPartitionName   patition name CNcomment:None EMMC device (such as SPI/NAND) can only use /dev/mtdx to be partition name. Emmc device can only use the name set by blkdevparts=mmcblk0 in bootargs, could not use /dev/mmcblkopx to be partition name
\retval    fd               Flash hande
\retval    INVALID_FD       invaild fd
\see \n
*/
HI_HANDLE HI_Flash_OpenByName(HI_CHAR* pPartitionName);

/**
\brief: open flash patition by  type and name
\attention \n
\param[in] enFlashType      flash type
\param[in] pPartitionName   patition name    None EMMC device (such as SPI/NAND) can only use /dev/mtdx to be partition name. EMMC device can only use /dev/mmcblkopx to be partition name.
\retval    fd               Flash hande
\retval    INVALID_FD       invaild fd
\see \n
*/
HI_HANDLE HI_Flash_OpenByTypeAndName(HI_FLASH_TYPE_E enFlashType, HI_CHAR* pPartitionName);

/**
\brief: open flash patition by  address
\attention \n
\param[in] enFlashType       flash type
\param[in] u64Address        open address
\param[in] u64Len            open length
\retval    fd                Flash hande
\retval    INVALID_FD        invaild fd
\see \n
*/
HI_HANDLE HI_Flash_OpenByTypeAndAddr(HI_FLASH_TYPE_E enFlashType, HI_U64 u64Address, HI_U64 u64Len);


/**
\brief: close flash partition
\attention \n
\param[in] hFlash    flash handle
\retval ::HI_SUCCESS
\retval ::HI_FAILURE
\see \n
*/
HI_S32 HI_Flash_Close(HI_HANDLE hFlash);

/**
\brief: erase Flash partiton
\attention \n
\param[in] hFlash      flash handle
\param[in] u64Address  erase start address, must align with blocksize
\param[in] u32Len      data length, must align with blocksize
\retval ::TotalErase   erase total length
\retval ::HI_FLASH_END_DUETO_BADBLOCK     return value when nand flash have bad block or valid length less partition length
\retval ::HI_FAILURE   failure
\see \n
*/
HI_S32 HI_Flash_Erase(HI_HANDLE hFlash, HI_U64 u64Address, HI_U64 u64Len);

/**
\brief:  read data from flash
\attention \n
\param[in] hFlash       flash handle
\param[in] u64Address   read start address, for nand, must align with pagesize.
\param[in] pBuf         destination buffer pointer
\param[in] u32Len       destination data length
\param[in] u32Flags     OOB flag  CNcomment:Can be valued to HI_FLASH_RW_FLAG_WITH_OOB, means if data has OOB CNend
\retval ::TotalRead     read flash vaild length
\retval ::HI_FLASH_END_DUETO_BADBLOCK    CNcomment: Read Flash till meet bad block CNend
\retval ::HI_FAILURE    failure
\see \n
*/
HI_S32 HI_Flash_Read(HI_HANDLE hFlash, HI_U64 u64Address, HI_U8* pBuf,
                     HI_U32 u32Len, HI_U32 u32Flags);

/**
\brief: write data to flash
\attention \n
1) forbidden used the function when yaffs filesystem is using
2) can use HI_FLASH_RW_FLAG_ERASE_FIRST manner write flah , can write over all partition one time or write with block

CNcomment:1) Could not call this interface to undate current using yaffs filesystem CNend
CNcomment:2) When call this interface to write flash, can use HI_FLASH_RW_FLAG_ERASE_FIRS. And support write down once or block by block.
But when write yaffs, must call HI_Flash_Erase to erase partition to be write CNend

\param[in] hFlash       flash handle
\param[in] u64Address   data start address, for nand, must align with pagesize
\param[in] pBuf         destination buffer pointer
\param[in] u32Len       destination data length, for nand, if write with oob, must align with (pagesize + oobsize)
\param[in] u32Flags     OOB flag CNcomment: Can be valued to HI_FLASH_RW_FLAG_WITH_OOB, means if data has OOB CNend
\retval ::TotalWrite    write flash vaild length
\retval ::HI_FLASH_END_DUETO_BADBLOCK   have bad block CNcomment: Read Flash till meet bad block CNend
\retval ::HI_FAILURE
\see \n
*/
HI_S32 HI_Flash_Write(HI_HANDLE hFlash, HI_U64 u64Address,
                      HI_U8* pBuf, HI_U32 u32Len, HI_U32 u32Flags);

/**
\brief: get flash partition info
\attention \n
info content: TotalSize,PartSize,BlockSize,PageSize,OobSize,fd
\param[in] hFlash        flash handle
\param[in] pInterInfo    info struct pointer
\retval ::HI_SUCCESS
\retval ::HI_FAILURE
\see \n
*/
HI_S32 HI_Flash_GetInfo(HI_HANDLE hFlash, HI_Flash_InterInfo_S* pFlashInfo);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __HI_FLASH_H__

