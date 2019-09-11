#ifndef __STORAGE_DEVINFO_H__
#define __STORAGE_DEVINFO_H__

#include "hi_mw_type.h"
#include "hi_storage.h"
#include "stg_dev.h"
#include "stg_devinfo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 STG_DEVINFO_Create(HI_U8 u8DevPortNo, HI_U8 u8DevErrCountThr, const HI_CHAR* pszPartitionPath, HI_MW_PTR* ppDevInfo);
HI_VOID STG_DEVINFO_Destroy(HI_MW_PTR pDevInfo);
HI_S32 STG_DEVINFO_GetEventInfo(HI_MW_PTR pDevInfo, STG_DEV_EVENT_INFO_S* pInfo);
HI_S32 STG_DEVINFO_GetInfo(HI_MW_PTR pDevInfo, HI_STORAGE_DEV_INFO_S* pInfo);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__STORAGE_DEVINFO_H__*/
