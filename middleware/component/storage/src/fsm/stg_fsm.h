#ifndef __HI_FINITE_STATEMACHINE_H__
#define __HI_FINITE_STATEMACHINE_H__

#include "StgMsg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_STATE_NAME_LEN (64)

typedef HI_S32 (*STG_FSM_StateHandleEvent)(HI_VOID* pPriv, const STG_MSG_INFO_S* pstMsg);

typedef HI_S32 (*STG_FSM_StateEnter)(HI_VOID* pPriv, const HI_CHAR* pstStatename);

typedef HI_S32 (*STG_FSM_StateExit)(HI_VOID* pPriv, const HI_CHAR* pstStatename);

typedef HI_S32 (*STG_FSM_EVENT_CALLBACK)(HI_VOID* pStateMachine, HI_VOID* pPriv, HI_S32 s32Event, HI_S32 s32RetVal);


typedef struct tagSTG_FSM_STATE_S
{
    HI_CHAR aszStateName[MAX_STATE_NAME_LEN];

    STG_FSM_StateHandleEvent pfnHandleEvent;

    STG_FSM_StateEnter pfnEnterState;

    STG_FSM_StateExit pfnExitState;

    HI_VOID* pPriv;
}STG_FSM_STATE_S;


typedef struct tagSTG_FSM_STATE_TRANS_S
{
    HI_S32 s32MsgType;

    STG_FSM_STATE_S* pstSrcState;

    STG_FSM_STATE_S* pstDstState;
}STG_FSM_STATE_TRANS_S;


HI_S32 STG_FSM_Create(HI_VOID** ppStateMachine, HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadLen, const HI_CHAR* pszFsmName);

HI_S32 STG_FSM_Destroy(HI_VOID* pStateMachine);

HI_S32 STG_FSM_AddState(HI_VOID* pStateMachine, const STG_FSM_STATE_S* pstState);

HI_S32 STG_FSM_AddTransition(HI_VOID* pStateMachine, const STG_FSM_STATE_TRANS_S* pstStateTrans);

HI_S32 STG_FSM_RegMsgCallback(HI_VOID* pStateMachine, STG_FSM_EVENT_CALLBACK pfnEventCallback, HI_VOID* pPriv);

HI_S32 STG_FSM_SetInitState(HI_VOID* pStateMachine, const STG_FSM_STATE_S* pstState);

HI_S32 STG_FSM_Start(HI_VOID* pStateMachine);

HI_S32 STG_FSM_Stop(HI_VOID* pStateMachine);

HI_S32 STG_FSM_GetCurrentState(HI_VOID* pStateMachine, HI_CHAR aszStateName[MAX_STATE_NAME_LEN]);

HI_S32 STG_FSM_SendAsync(HI_VOID* pStateMachine, STG_MSG_INFO_S* pstMsg, HI_U64 u64Delay);

HI_S32 STG_FSM_SendSync(HI_VOID* pStateMachine, STG_MSG_INFO_S* pstMsg, STG_MSG_INFO_S* pstReplyMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif//__HI_FINITE_STATEMACHINE_H__
