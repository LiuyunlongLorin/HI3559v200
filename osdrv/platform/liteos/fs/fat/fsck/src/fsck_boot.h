#ifndef __FSCK_BOOT_H__
#define __FSCK_BOOT_H__
#include "fstool_common.h"

/******BPB BIOS Parameter block struct just for FAT32****/
#define FSCK_BOOT_CLUSTER_MASK      0xfffffff
#define FSCK_BOOT_MAX_BLOCK_SIZE    512
#define FSCK_BOOT_MAX_FSI_SIZE      1024
#define FSCK_BOOT_BS_JUMP_SIZE      3
#define FSCK_BOOT_BS_OEM_SIZE       8
#define FSCK_BOOT_FSI_STR_OFFSET    484
#define FSCK_BOOT_FSI_FREEC_OFFSET  488
#define FSCK_BOOT_FSI_NXT_OFFSET    492
#define FSCK_BOOT_FSI_TRA_OFFSET    508

typedef struct  tagFSCK_BOOT_SECTOR_S
{
    u_int   BPB_BytesPerSec;    /* size of sector by bytes */
    u_int   BPB_SecPerClus;     /* the number sectors per cluster */
    u_int   BPB_RsvdSecCnt;     /* the reserved sectors numbers */
    u_int   BPB_NumFATs;        /* the FAT numbers of disk */
    u_int   BPB_RootEntCnt;     /* number of root directory entries */
    u_int   BPB_TotSec16;       /* the totle number of sectors */
    u_int   BPB_Media;          /* media descriptor */
    u_int   BPB_FATSz16;        /* the number sectors of FAT */
    u_int   BPB_SecPerTrk;      /* the number sectors of track */
    u_int   BPB_NumHeads;       /* the number of heads */
    u_int   BPB_HiddSe;         /* the number of hidden sectors */
    u_int   BPB_TotSec32;       /* the numberof sectors if BPB_TotSec16 == 0 */

    //FAT 32
    u_int   BPB_FATflags;       /* FAT 32 flags:1 others flags:0 */
    u_int   BPB_FATSz32;        /* the numbre of sectors  in FAT */
    int     BPB_ExtFlags;       /* only used for FAT32 mirrored */
    u_int   BPB_RootClus;       /* the Start Cluster of Root Directory */
    u_int   BPB_FSInfo;         /* the FSInfo sector */
    u_int   BPB_BkBootSec;      /* Backup of Boot sectors */
    u_int   BPB_Reserved1;      /* FAT 32 dirty flag*/
    // Fsinfo
    u_int   FSI_FreeClCnt;      /* the Number of free clusters from FSInfo */
    u_int   FSI_LastCl;         /* Next free cluster from FSInfo */
    // FAT entry info
    u_int   FATClnShutBit;      /* umount :0  mount:1 */
    u_int   FATClustMask;       /* mask for entries in FAT */
    u_int   NumClusters;        /* the totle vale  entries in a FAT */
    u_int   NumSectors;         /* the totle value sectors are there */
    u_int   NumFatEntries;      /* the entries really are there */
    u_int   StartClusSecNo;     /* the sector No of first cluster*/
    u_int   ClusterSize;        /* Cluster size in bytes */
    // Now some statistics
    u_int   NumFiles;           /* # of plain files */
    u_int   NumFree;            /* # of free clusters */
    u_int   NumBad;             /* # of bad clusters */
}FSCK_BOOT_SECTOR_S;


/**********************functions ********************/

u_int FSCK_BOOT_Read(FSCK_BOOT_SECTOR_S* pstBoot, int fd, u_char** pstBootSector, u_char** pstFsinfoSector);
int FSCK_BOOT_IsDirty(FSCK_BOOT_SECTOR_S* pstBoot);
u_int FSCK_BOOT_CleanDirtyFlag(int fd);
u_int FSCK_BOOT_UpdateFSInfo(FSCK_BOOT_SECTOR_S* pstBoot, int fd, cl_t lastAllocatedClu, u_char* fsinfo_sector);
#endif
