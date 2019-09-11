#ifndef __MTD_MTD_H__
#define __MTD_MTD_H__

#include "mtd/mtd-abi.h"
#include "sys/types.h"
#include "stddef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MAX_MTD_DEVICES 32
#define MTD_BLOCK_MAJOR 31
#define MTD_CHAR_MAJOR 90

#define MTD_FAIL_ADDR_UNKNOWN    -1LL

#define MTD_ERASE_FAILED        0x10
#define MTD_ERASE_DONE          0x08
#define MTD_ERASE_SUSPEND       0x04
#define MTD_ERASING             0x02
#define MTD_ERASE_PENDING       0x01

struct erase_info {
    int scrub;
    struct erase_info *next;
    u_char state;
    u_long priv;
    void (*callback) (struct erase_info *self);
    u_int cell;
    u_int dev;
    u_long retries;
    u_long time;
    uint64_t fail_addr;
    uint64_t len;
    uint64_t addr;
    struct mtd_info *mtd;
};

struct mtd_oob_ops {
    size_t          len;
    size_t          ooblen;
    const char      *datbuf;
    const char      *oobbuf;
};

struct mtd_info {
    void *priv;

    uint8_t type;
    const char *name;
    uint32_t flags;

    uint64_t size;     /* Total size of the MTD */
    uint32_t erasesize;
    uint32_t writesize;

    uint32_t oobsize;

    int (*erase)(struct mtd_info *mtd, struct erase_info *instr);
    int (*read)(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, const char *buf);
    int (*write)(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const char *buf);
    int (*read_oob)(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, const char *buf);
    int (*write_oob)(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const char *buf);

    int (*block_isbad)(struct mtd_info *mtd, loff_t ofs);
    int (*block_markbad)(struct mtd_info *mtd, loff_t ofs);
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __MTD_MTD_H__ */

