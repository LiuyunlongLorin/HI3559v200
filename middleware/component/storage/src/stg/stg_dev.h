#ifndef __STORAGE_DEV_H__
#define __STORAGE_DEV_H__
#include "hi_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum tagSTG_DEV_STATE_E
{
    STG_DEV_STATE_UNPLUGGED = 0x00,
    STG_DEV_STATE_CONNECTING,
    STG_DEV_STATE_CONNECTED,
    STG_DEV_STATE_IDEL
} STG_DEV_STATE_E;

typedef struct tagSTG_DEV_EVENT_INFO_S
{
    STG_DEV_STATE_E enState;
    HI_U32          u32ErrorCnt;
}STG_DEV_EVENT_INFO_S;

typedef struct tagSTG_DEV_CFG_S
{
    HI_U8 u8DevPortNo;
    HI_U8 u8DevErrCountThr;
    HI_CHAR aszPartitionPath[HI_STORAGE_PATH_LEN_MAX];
    HI_MW_PTR pStorage;
    HI_MW_PTR pFSTool;
    HI_STORAGE_ON_STATE_CHANGE_PFN pfnOnStateChange;
}STG_DEV_CFG_S;

HI_S32 STG_DEV_Create(const STG_DEV_CFG_S* pstCfg, HI_MW_PTR* ppDev);
HI_VOID STG_DEV_Destroy(HI_MW_PTR pDev);
HI_S32 STG_DEV_GetState(HI_MW_PTR pDev, STG_DEV_STATE_E* penState);
HI_S32 STG_DEV_GetInfo(HI_MW_PTR pDev, HI_STORAGE_DEV_INFO_S* pInfo);
HI_S32 STG_DEV_PauseMonitor(HI_MW_PTR pDev);
HI_S32 STG_DEV_ResumeMonitor(HI_MW_PTR pDev);
HI_BOOL STG_DEV_IsPaused(HI_MW_PTR pDev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__STORAGE_DEV_H__*/
