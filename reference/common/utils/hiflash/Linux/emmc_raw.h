
#ifndef __EMMC_RAW_H__
#define __EMMC_RAW_H__

#include "hi_type.h"
#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EMMC_EXT_PART_ID    (5)
#define EMMC_SECTOR_TAIL    (0xAA55)
#define EMMC_SECTOR_SIZE    (512)
#define MMC_BLOCK_MAJOR     (179)
#define MMC_ERASE_CMD       _IOW(MMC_BLOCK_MAJOR, 1, struct mmc_erase_cmd)

typedef enum tagEMMC_PART_TYPE_E
{
    EMMC_PART_TYPE_RAW,
    EMMC_PART_TYPE_LOGIC,
    EMMC_PART_TYPE_BUTT
} EMMC_PART_TYPE_E;

typedef struct tagEMMC_FLASH_S
{
    HI_U64 u64RawAreaStart; /**< absolutely offset from emmc flash start
                                 none ext area start address */
    HI_U64 u64RawAreaSize;  /**< none ext area size, in Byte */
    HI_U32 u32EraseSize;    /**< Block size. Default is 512B */
} EMMC_FLASH_S;

typedef struct tagEMMC_CB_S
{
    HI_S32 fd;
    HI_U64 u64Address;
    HI_U64 u64PartSize;
    HI_U32 u32EraseSize;
    EMMC_PART_TYPE_E enPartType;
} EMMC_CB_S;

struct mmc_erase_cmd
{
    HI_U32 from; /**< first sector to erase */
    HI_U32 nr;   /**< number of sectors to erase */
    HI_U32 arg;  /**< erase command argument (SD supports only %MMC_ERASE_ARG) */
};


HI_S32 emmc_raw_init(char* bootargs);
EMMC_CB_S* emmc_raw_open(HI_U64 u64Addr, HI_U64 u64Length);
EMMC_CB_S* emmc_node_open(const HI_U8* pu8Node);

HI_S32 emmc_block_read(HI_S32 fd, HI_U64 u32Start, HI_U32 u32Len, HI_VOID* buff);

HI_S32 emmc_block_write(HI_S32 fd, HI_U64 u32Start, HI_U32 u32Len, const HI_VOID* buff);

HI_S32 emmc_raw_read(const EMMC_CB_S* pstEmmcCB, HI_U64 u64Offset, HI_U32 u32Length, HI_U8* buf);

HI_S32 emmc_raw_write(const EMMC_CB_S* pstEmmcCB, HI_U64 u64Offset, HI_U32 u32Length, const HI_U8* buf);

HI_S32 emmc_raw_close(EMMC_CB_S* pstEmmcCB);

#if defined (HI_EMMC_ERASE_SUPPORT)
HI_S32 emmc_raw_erase(EMMC_CB_S* pstEmmcCB, HI_U64 u64Offset, HI_U64 u64Length);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __EMMC_RAW_H__ */

