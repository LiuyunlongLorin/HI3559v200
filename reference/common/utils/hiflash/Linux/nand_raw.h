
#ifndef NAND_RAWH
#define NAND_RAWH

#include "nand.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define  HI_FLASH_END_DUETO_BADBLOCK -10
#define  HINFC610_OOBSIZE_FOR_YAFFS  32

/**
 * MTD operation modes
 *
 * @MTD_OPS_PLACE_OOB:  OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:   OOB data are automatically placed at the free areas
 *                      which are defined by the internal ecclayout
 * @MTD_OPS_RAW:        data are transferred as-is, with no error correction;
 *                      this mode implies %MTD_OPS_PLACE_OOB
 *
 * These modes can be passed to ioctl(MEMWRITE) and are also used internally.
 * See notes on "MTD file modes" for discussion on %MTD_OPS_RAW vs.
 * %MTD_FILE_MODE_RAW.
 */
enum
{
    MTD_OPS_PLACE_OOB = 0,
    MTD_OPS_AUTO_OOB = 1,
    MTD_OPS_RAW = 2,
};

HI_S32 nand_raw_init(HI_VOID);

HI_S32 nand_raw_read(HI_S32 fd, HI_U64* startaddr, HI_U8* buffer, HI_UL length,
                     HI_U64 openaddr, HI_U64 limit_leng, HI_S32 read_oob, HI_S32 skip_badblock);

HI_S32 nand_raw_write(HI_S32 fd, HI_U64* startaddr, HI_U8* buffer, HI_UL length,
                      HI_U64 openaddr, HI_U64 limit_leng, HI_S32 write_oob);

HI_S64 nand_raw_erase(HI_S32 fd, HI_U64 startaddr, HI_U64 length, HI_U64 openaddr, HI_U64 limit_leng);

HI_S32 nand_raw_force_erase(HI_U64 offset);

HI_S32 nand_mark_badblock(HI_U64 offset, HI_U64 length);

HI_S32 nand_show_badblock(HI_U64 offset, HI_U64 length);

HI_S32 nand_raw_info(struct mtd_info_user* mtdinfo);

HI_S32 nand_raw_dump_partition(HI_VOID);

HI_S32 nand_raw_destroy(HI_VOID);

HI_U64 nand_raw_get_start_addr(const HI_CHAR* dev_name, HI_UL blocksize, HI_S32* value_valid);

void nand_raw_get_info(HI_U64* totalsize, HI_U32* pagesize, HI_U32* blocksize,
                       HI_U32* oobsize, HI_U32* blockshift);

int  nand_raw_get_physical_index(HI_U64 startaddr, HI_S32* blockindex, HI_S32 blocksize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* NAND_RAWH */

