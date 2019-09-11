#ifndef __FSCK_FAT_H__
#define __FSCK_FAT_H__

/**********************functions ********************/
int FAT_IsValidCluster(FSCK_BOOT_SECTOR_S* pstBoot, cl_t cl);
int FAT_IsAllocatedCluster(cl_t cl);
int FAT_IsEOFCluster(cl_t cl);
int FAT_IsBadCluster(cl_t cl);
int FAT_IsRsrvCluster(cl_t cl);
int FAT_IsFreeCluster(cl_t cl);
int FAT_IsCircularChain(cl_t curCl, cl_t nextCl);

u_int FSCK_FAT_Read(int fs, FSCK_BOOT_SECTOR_S *pstBoot, int fatNo, u_char **buffer);

int FSCK_FAT_IsDirty(FSCK_BOOT_SECTOR_S *pstBoot, u_char* fat);

u_int FSCK_FAT_CleanDirtyFlag(FSCK_BOOT_SECTOR_S* pstBoot, int fd);

u_int FSCK_FAT_Cmp(FSCK_BOOT_SECTOR_S *pstBoot, u_char* first, u_char  *second, int fatNo);

u_int FSCK_FAT_Check(FSCK_BOOT_SECTOR_S *pstBoot, int fatNo, u_char *buffer);

u_int FSCK_FAT_Write(int fd, FSCK_BOOT_SECTOR_S *pstBoot);

void FSCK_FAT_ClearChain( FSCK_BOOT_SECTOR_S *pstBoot, FSCK_CACHE_CLUSTER_CHAIN_S* fat);

u_int FSCK_FAT_CheckLost(FSCK_BOOT_SECTOR_S *pstBoot);

u_int FSCK_FAT_GetLastAlloCl(FSCK_BOOT_SECTOR_S *pstBoot, cl_t* pLastAllocCl);

u_int FSCK_FAT_CheckFragment(unsigned int u32FragmentThr);

#endif
