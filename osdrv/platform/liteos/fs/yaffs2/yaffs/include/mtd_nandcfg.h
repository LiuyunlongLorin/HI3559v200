#ifndef __MTD_NANDCFG_H__
#define __MTD_NANDCFG_H__

#include "linux/mtd/mtd.h"
#include "mtd_partition.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern struct mtd_info *nand_mtd;

#define NANDBLK_NAME "/dev/nandblk"
#define NANDCHR_NAME  "/dev/nandchr"
#define OOB_SIZE                (nand_mtd->oobsize)
#define PAGE_DATA_SIZE          (nand_mtd->writesize)
#define NAND_PAGE_SIZE          (PAGE_DATA_SIZE + OOB_SIZE)
#define SUPPORT_MAX_PAGE_SIZE   (8192 + 256)
#define PAGE_PER_BLOCK          (nand_mtd->erasesize/nand_mtd->writesize)
#define NAND_BLOCK_DATA_SIZE    (PAGE_DATA_SIZE * PAGE_PER_BLOCK)
#define NAND_BLOCK_ERASE_SIZE   (nand_mtd->erasesize)
#define NAND_BLOCK_SIZE         (NAND_PAGE_SIZE * PAGE_PER_BLOCK)

#define ECC_SIZE        24 //eccbytes in nand driver

#define NAND_CHIP_SIZE (nand_mtd->size)
#define NAND_FLASH_END_ADDR (NAND_CHIP_SIZE - 1)
#define CONFIG_YAFFS_ECC_MODE     1 //1 use ecc, 0 no ecc
#define CONFIG_YAFFS_INBAND_TAGS  0 //1 use in band tags, 0-no in band tags
#define CONFIG_YAFFS_USE_YAFFS2   1 //1 yaffs2, 0-yaffs1
#define CONFIG_YAFFS_NAND_DEFAULT_START_BLOCK 0
#define CONFIG_YAFFS_NAND_DEFAULT_END_BLOCK  (nand_mtd->size/nand_mtd->erasesize - 1)
#define CONFIG_YAFFS_NAND_PAGE_SIZE PAGE_DATA_SIZE
#define CONFIG_YAFFS_NAND_OOB_SIZE OOB_SIZE
#define CONFIG_YAFFS_NAND_PAGES_PER_BLOCK PAGE_PER_BLOCK
#define YAFFS_NAND_BLOCK_SIZE (CONFIG_YAFFS_NAND_PAGE_SIZE * CONFIG_YAFFS_NAND_PAGES_PER_BLOCK)
int nf_read_page (long page,
                          unsigned char* data, unsigned long data_len,
                          unsigned char* spare, unsigned long spare_len);
int nf_write_page (long page,
                           const unsigned char * data, unsigned long data_len,
                           const unsigned char * spare, unsigned long spare_len);
int nf_move_page (long src_page, long dst_page);
int nf_erase_block (unsigned long block);
int nf_check_block (unsigned long block);
int nf_mark_badblock(unsigned long block);
int hinand_yaffs_read(void* memaddr, unsigned long start, unsigned long size);
int hinand_yaffs_write(void* memaddr, unsigned long start, unsigned long size);
int hinand_erase(unsigned long start, unsigned long size);
int hinand_yaffs_nand_block_isbad(loff_t ofs);
int hinand_yaffs_nand_block_markbad(loff_t ofs);
partition_param *init_yaffspar_param(partition_param *yaffspar_param);
void deinit_yaffspar_param(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
