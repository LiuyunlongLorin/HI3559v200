#ifndef __STG_DEVFSM_H__
#define __STG_DEVFSM_H__

#include <semaphore.h>
#include "hi_mw_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef HI_S32 (*STG_DEVFSM_EVENT_CALLBACK)(HI_VOID* pStateMachine, HI_VOID* pPriv, HI_S32 s32Event, HI_S32 s32RetVal);

typedef struct tagSTG_DEVFSM_CFG_S
{
    HI_MW_PTR pFSTool;
    HI_MW_PTR pDev;
    HI_MW_PTR pStorage;
    HI_STORAGE_ON_STATE_CHANGE_PFN pfnOnStateChange;
    STG_DEVFSM_EVENT_CALLBACK pfnFSMEvntCB;
}STG_DEVFSM_CFG_S;


HI_S32 STG_DEVFSM_Create(const STG_DEVFSM_CFG_S* pstCfg, HI_MW_PTR* ppDevFsm);
HI_VOID STG_DEVFSM_Destroy(HI_MW_PTR pDevFsm);
HI_S32 STG_DEVFSM_SendEvent(HI_MW_PTR pDevFsm, STG_DEV_EVENT_INFO_S *pInfo);
HI_S32 STG_DEVFSM_GetCurState(HI_MW_PTR pDevFsm, STG_DEV_STATE_E* penState);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__STG_DEVFSM_H__*/

