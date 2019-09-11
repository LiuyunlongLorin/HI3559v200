#ifndef __STORAGE_DEVMONITOR_H__
#define __STORAGE_DEVMONITOR_H__

#include "hi_mw_type.h"
#include "hi_storage.h"
#include "stg_dev.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_S32 STG_DEVMONITOR_Create(HI_MW_PTR pDevInfo, HI_MW_PTR pDevFSM, HI_MW_PTR* ppDevMonitor);
HI_VOID STG_DEVMONITOR_Destroy(HI_MW_PTR pDevMonitor);
HI_S32 STG_DEVMONITOR_StopNotify(HI_MW_PTR pDevMonitor);
HI_S32 STG_DEVMONITOR_RestartNotify(HI_MW_PTR pDevMonitor);
HI_BOOL STG_DEVMONITOR_IsStopNotify(HI_MW_PTR pDevMonitor);
HI_VOID STG_DEVMONITOR_PauseNotify(HI_MW_PTR pDevMonitor);
HI_VOID STG_DEVMONITOR_ResumeNotify(HI_MW_PTR pDevMonitor);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__STORAGE_DEVMONITOR_H__*/
