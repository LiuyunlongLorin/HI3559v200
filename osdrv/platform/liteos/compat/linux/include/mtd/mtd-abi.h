#ifndef __MTD_ABI_H__
#define __MTD_ABI_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MTD_UBIVOLUME   7
#define MTD_DATAFLASH   6
#define MTD_NANDFLASH   4
#define MTD_NORFLASH    3
#define MTD_ROM         2
#define MTD_RAM         1
#define MTD_ABSENT      0

#define MTD_STUPID_LOCK     0x2000  /* Always locked after reset */
#define MTD_NO_ERASE        0x1000  /* No erase necessary */
#define MTD_BIT_WRITEABLE   0x800   /* Single bits can be flipped */
#define MTD_WRITEABLE       0x400   /* Device is writeable */


/* Some common devices / combinations of capabilities */
#define MTD_CAP_NANDFLASH   (MTD_WRITEABLE)
#define MTD_CAP_NORFLASH    (MTD_WRITEABLE | MTD_BIT_WRITEABLE)
#define MTD_CAP_RAM         (MTD_WRITEABLE | MTD_BIT_WRITEABLE | MTD_NO_ERASE)
#define MTD_CAP_ROM         0
        
#define    MEMGETINFO       0x00000001

#define    MEMERASE         0x00000002
#define    MEMERASE64       0x00000004

#define    MEMGETBADBLOCK   0x00000008
#define    MEMSETBADBLOCK   0x00000010

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __MTD_ABI_H__ */

