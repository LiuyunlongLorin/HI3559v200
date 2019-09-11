#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hi_storage.h"
#include "hi_fstool.h"
#include "fstool_wrapper.h"

#if 0// user can open it when buf need be alloc from MMZ
#include "mpi_sys.h"

HI_S32 FSTOOL_WRAPPER_Alloc(HI_VOID* pPhyAddr, HI_U32 u32BufSize, HI_VOID** ppVmAddr)
{
    HI_S32 s32Ret = 0;
    HI_VOID *pVmAddr = NULL;
    const char* pszBufName = "FSTool Buf";

    s32Ret = HI_MPI_SYS_MmzAlloc(pPhyAddr, &pVmAddr, pszBufName, HI_NULL, u32BufSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Alloc buf from mmz failed, and return:%x\n",s32Ret);
        return s32Ret;
    }

    *ppVmAddr = pVmAddr;

    return HI_SUCCESS;
}

HI_S32 FSTOOL_WRAPPER_Free(HI_VOID* pPhyAddr, HI_VOID* pVmAddr)
{
    HI_S32 s32Ret = 0;
    HI_U64 u64PhyAddr = 0;
    u64PhyAddr = *(HI_U64*)pPhyAddr;

    s32Ret = HI_MPI_SYS_MmzFree(u64PhyAddr, pVmAddr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Free buf of mmz failed, and return:%x\n",s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
#endif

HI_S32 FSTOOL_WRAPPER_RegBufOps()
{
#if 0
    HI_S32 s32Ret = 0;
    HI_FSTOOL_BUF_OPS_S stBufOps = {0};

    stBufOps.pfnBufAlloc = (HI_FSTOOL_BUF_ALLOC_FN)FSTOOL_WRAPPER_Alloc;
    stBufOps.pfnBufFree = (HI_FSTOOL_BUF_FREE_FN)FSTOOL_WRAPPER_Free;
    s32Ret = HI_FSTOOL_RegisterBufOps(&stBufOps);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_FSTOOL_RegisterBufOps failed, and return:%x\n",s32Ret);
        return s32Ret;
    }
#endif

    return HI_SUCCESS;
}

HI_STORAGE_FSTOOL_ERR_E FSTOOL_WRAPPER_Check(const HI_CHAR *pszPartitionPath)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_FSTOOL_CHECK_CFG_S stCheckCfg;
    stCheckCfg.enMode = HI_FSTOOL_CHECK_MODE_ALWAYS_MAIN_FAT;
    stCheckCfg.u32FragmentThr = 1;
    s32Ret = HI_FSTOOL_Check(pszPartitionPath, &stCheckCfg);

    if (HI_FSTOOL_SUCCESS == s32Ret)
    {
        return HI_STORAGE_FSTOOL_SUCCESS;
    }
    else if (HI_ERR_FSTOOL_ILLEGAL_PARAM == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM;
    }
    else if (HI_ERR_FSTOOL_OPEN_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_OPEN_FAIL;
    }
    else if (HI_ERR_FSTOOL_READ_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_READ_FAIL;
    }
    else if (HI_ERR_FSTOOL_WRITE_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_WRITE_FAIL;
    }
    else if (HI_ERR_FSTOOL_NOT_ENOUGH_MEMORY == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_NOT_ENOUGH_MEMORY;
    }
    else if (HI_ERR_FSTOOL_FSTYPE_UNSUPPORT == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_FSTYPE_UNSUPPORT;
    }
    else if (HI_ERR_FSTOOL_FS_EXCEPTION == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_FS_EXCEPTION;
    }
    else if (HI_ERR_FSTOOL_FRAGMENT_SEVERELY == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY;
    }
    else if (HI_ERR_FSTOOL_NOT_64K_CLUSTER == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_FS_NOT_64K_CLUSTER;
    }
    else
    {
        return HI_STORAGE_FSTOOL_ERR_ERR_BUTT;
    }
}

HI_STORAGE_FSTOOL_ERR_E FSTOOL_WRAPPER_Format(const HI_CHAR *pszPartitionPath, HI_U64 u64ClusterSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_FSTOOL_FORMAT_CFG_S stFormatCfg;
    stFormatCfg.enMode = HI_FSTOOL_FORMAT_MODE_ADVANCED;
    stFormatCfg.u32ClusterSize = u64ClusterSize;
    stFormatCfg.enable4KAlignCheck = 1;
    s32Ret = HI_FSTOOL_Format(pszPartitionPath, &stFormatCfg);
    if (HI_FSTOOL_SUCCESS == s32Ret)
    {
        return HI_STORAGE_FSTOOL_SUCCESS;
    }
    else if (HI_ERR_FSTOOL_ILLEGAL_PARAM == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_ILLEGAL_PARAM;
    }
    else if (HI_ERR_FSTOOL_OPEN_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_OPEN_FAIL;
    }
    else if (HI_ERR_FSTOOL_WRITE_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_WRITE_FAIL;
    }
    else if (HI_ERR_FSTOOL_DEVICE_EXCEPTION == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_DEVICE_EXCEPTION;
    }
    else if (HI_ERR_FSTOOL_SYSTEM_CALL_FAILURE == s32Ret)
    {
        return HI_STORAGE_FSTOOL_ERR_SYSTEM_CALL_FAILURE;
    }
    else
    {
        return HI_STORAGE_FSTOOL_ERR_ERR_BUTT;
    }
}

