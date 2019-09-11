#ifndef __STORAGE_FSTOOL_H__
#define __STORAGE_FSTOOL_H__

#include "hi_mw_type.h"
#include "hi_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct tagSTG_FSTOOL_CFG_S
{
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX];
    HI_CHAR aszMountPath[HI_STORAGE_PATH_LEN_MAX];
}STG_FSTOOL_CFG_S;

typedef enum tagSTG_FS_STATE_E
{
    STG_FS_STATE_CHECKING = 0x0,
    STG_FS_STATE_CHECK_FAILED,
    STG_FS_STATE_EXCEPTION,
    STG_FS_STATE_MOUNTED,
    STG_FS_STATE_MOUNT_FAILED,
    STG_FS_STATE_IDEL
} STG_FS_STATE_E;

HI_S32 STG_FSTOOL_Register(const HI_STORAGE_FS_TOOL_S* pstFSTool);
HI_S32 STG_FSTOOL_Unregister(const HI_CHAR* pszFSType);

HI_S32 STG_FSTOOL_Create(const STG_FSTOOL_CFG_S* pstCfg, HI_MW_PTR* ppFSTool);
HI_VOID STG_FSTOOL_Destroy(HI_MW_PTR pFSTool);
HI_VOID STG_FSTOOL_SetState(HI_MW_PTR pFSTool, STG_FS_STATE_E enState);
HI_VOID STG_FSTOOL_GetState(HI_MW_PTR pFSTool, STG_FS_STATE_E* penState);
HI_S32 STG_FSTOOL_GetPartitionPath(HI_MW_PTR pFSTool, HI_CHAR* pszPartitionPath);
HI_S32 STG_FSTOOL_GetInfo(HI_MW_PTR pFSTool, HI_STORAGE_FS_INFO_S* pstInfo);
HI_STORAGE_FSTOOL_ERR_E STG_FSTOOL_Check(HI_MW_PTR pFSTool, HI_MW_PTR pStorage);
HI_S32 STG_FSTOOL_Format(HI_MW_PTR pFSTool, const HI_CHAR* pszFSType, HI_U64 u64ClusterSize);
HI_S32 STG_FSTOOL_Mount(HI_MW_PTR pFSTool,HI_MW_PTR pStorage);
HI_S32 STG_FSTOOL_Umount(HI_MW_PTR pFSTool, HI_BOOL bEnableLog);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__STORAGE_FSTOOL_H__*/

