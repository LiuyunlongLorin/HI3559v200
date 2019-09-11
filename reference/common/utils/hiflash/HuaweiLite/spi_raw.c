#include "spinor.h"
#include "hi_appcomm.h"
#include "../HuaweiLite/spi_raw.h"

extern struct mtd_info* spinor_mtd;
extern int hispinor_read(void* memaddr, unsigned long start, unsigned long size);
extern int hispinor_erase(unsigned long start, unsigned long size);
extern int hispinor_write(void* memaddr, unsigned long start, unsigned long size);

/*****************************************************************************/

HI_S32 spi_raw_init(HI_VOID)
{
    return 0;
}

HI_VOID spi_raw_get_info
(
    HI_U64* totalsize,
    HI_U32* pagesize,
    HI_U32* blocksize,
    HI_U32* oobsize,
    HI_U32* blockshift)
{
    *totalsize  = spinor_mtd->size;
    *pagesize   = spinor_mtd->writesize;
    *blocksize  = spinor_mtd->erasesize;
    *oobsize    = spinor_mtd->oobsize;
}

/*****************************************************************************/
/*
 * warning:
 *    1. startaddr should be alignment with pagesize
 */
HI_S32 spi_raw_read
(
    HI_S32 fd,
    HI_U64* startaddr, /* this address maybe change when meet bad block */
    HI_U8*  buffer,
    HI_UL  length,    /* if HI_FLASH_RW_FLAG_WITH_OOB, include oob*/
    HI_U64 openaddr,
    HI_U64 limit_leng,
    HI_S32 read_oob,
    HI_S32 skip_badblock)
{
    if (0 > hispinor_read(buffer, *startaddr, (unsigned long)length))
    {
        MLOGE("\n error read spinor flash Addr:0x%08lx len:0x%lx\n",(HI_UL)*startaddr,length);
        return HI_FAILURE;
    }
    return length;
}

/*****************************************************************************/
/*
 * warning:
 *    1. offset and length should be alignment with blocksize
 */
HI_S64 spi_raw_erase
(
    HI_S32 fd,
    HI_U64 startaddr,
    HI_U64 length,
    HI_U64 openaddr,
    HI_U64 limit_leng)
{
    if (0 > hispinor_erase(startaddr, (unsigned long)length))
    {
        MLOGE("\n error erase spinor flash Addr:0x%08lx len:0x%lx\n",(HI_UL)startaddr,(HI_UL)length);
        return HI_FAILURE;
    }
    return length;
}

/*****************************************************************************/
/*
 * warning:
 *    1. startaddr should be alignment with pagesize
 */
HI_S32 spi_raw_write
(
    HI_S32 fd,
    HI_U64* startaddr,
    HI_U8* buffer,
    HI_UL length,
    HI_U64 openaddr,
    HI_U64 limit_leng,
    HI_S32 write_oob)
{
    if (0 > hispinor_write(buffer, *startaddr, (unsigned long)length))
    {
        MLOGE("\n error write spinor flash Addr:0x%08lx len:0x%lx\n",(HI_UL)*startaddr,length);
        return HI_FAILURE;
    }
    return length;
}


/*****************************************************************************/

/*****************************************************************************/

HI_S32 spi_raw_destroy(HI_VOID)
{
    return 0;
}

