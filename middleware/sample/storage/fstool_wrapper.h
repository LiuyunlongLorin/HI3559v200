#ifndef __FSTOOL_WRAPPER__
#define __FSTOOL_WRAPPER__
#include "hi_type.h"

#define  KB_2_BYTE_FACTOR   (1024)
#define  SD_SECTOR_SIZE     (512)

HI_S32 FSTOOL_WRAPPER_RegBufOps();
HI_STORAGE_FSTOOL_ERR_E FSTOOL_WRAPPER_Check(const HI_CHAR *pszPartitionPath);
HI_STORAGE_FSTOOL_ERR_E FSTOOL_WRAPPER_Format(const HI_CHAR *pszPartitionPath, HI_U64 u64ClusterSize);
#endif
