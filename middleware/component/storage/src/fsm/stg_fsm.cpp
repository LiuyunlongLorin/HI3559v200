#include "securec.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "hi_type.h"
#include "stg_fsm.h"
#include "stg_fsm_common.h"
#include "stg_fsm_state.h"
#include "StgStateMachine.h"

class FSMOberver;

typedef struct tagFSM_INSTANCE_S
{
    StgStateMachine* pstStateMachine;
    pthread_mutex_t instanceLock;
    list<FsmState*> fsmStateList;
    FSMOberver* pstFsmOberver;
}FSM_INSTANCE_S;

typedef struct tagFSM_CONTEXT_S
{
    pthread_mutex_t listLock;
    list<FSM_INSTANCE_S*> fsmInstanceList;
}FSM_CONTEXT_S;

static FSM_CONTEXT_S s_gFsmContext = {PTHREAD_MUTEX_INITIALIZER, };

static FSM_INSTANCE_S* FSM_FindInstance(const StgStateMachine* pstStateMachine)
{
    FSM_INSTANCE_S* pstInstance = HI_NULL;

    FSM_LOCK(s_gFsmContext.listLock);
    list<FSM_INSTANCE_S*>::iterator msgIter = s_gFsmContext.fsmInstanceList.begin();

    for(;msgIter != s_gFsmContext.fsmInstanceList.end(); msgIter++)
    {
        FSM_INSTANCE_S* pstInstance = *msgIter;
        if(pstInstance->pstStateMachine  == pstStateMachine)
        {
            FSM_UNLOCK(s_gFsmContext.listLock);
            return pstInstance;
        }
    }
    FSM_UNLOCK(s_gFsmContext.listLock);
    return pstInstance;
}

class FSMOberver: public StgStateMachineObserver
{
public:
    FSMOberver(STG_FSM_EVENT_CALLBACK pfnCallback, HI_VOID* pPriv) : mEvtCallback(pfnCallback),mPriv(pPriv)
    {

    }

    ~FSMOberver()
    {

    }

    void onEventHandled(StgStateMachine* pstStateMachine, int event, int result)
    {
        FSM_INSTANCE_S* pstInstance = FSM_FindInstance(pstStateMachine);
        if(!pstInstance)
        {
            STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "could not find respond fsm instance by statemachine err\n");
            return;
        }

        if(mEvtCallback)
        {
            mEvtCallback(pstInstance, mPriv, event, result);
        }
    }

private:
    STG_FSM_EVENT_CALLBACK mEvtCallback;
    HI_VOID* mPriv;
};

static HI_BOOL FSM_CheckInstanceExist(FSM_INSTANCE_S* pstFsmInstance)
{
    FSM_LOCK(s_gFsmContext.listLock);
    list<FSM_INSTANCE_S*>::iterator msgIter = s_gFsmContext.fsmInstanceList.begin();

    for(;msgIter != s_gFsmContext.fsmInstanceList.end();msgIter++)
    {
        if(pstFsmInstance == *msgIter)
        {
            FSM_UNLOCK(s_gFsmContext.listLock);
            return HI_TRUE;
        }
    }
    FSM_UNLOCK(s_gFsmContext.listLock);
    return HI_FALSE;
}

static HI_VOID FSM_AddInstance(FSM_INSTANCE_S* pstFsmInstance)
{
    FSM_LOCK(s_gFsmContext.listLock);

    s_gFsmContext.fsmInstanceList.push_back(pstFsmInstance);

    FSM_UNLOCK(s_gFsmContext.listLock);
}

static HI_VOID FSM_DelInstance(FSM_INSTANCE_S* pstFsmInstance)
{
    FSM_LOCK(s_gFsmContext.listLock);

    list<FSM_INSTANCE_S*>::iterator msgIter = s_gFsmContext.fsmInstanceList.begin();

    for(;msgIter != s_gFsmContext.fsmInstanceList.end();msgIter++)
    {
        if(pstFsmInstance == *msgIter)
        {
            s_gFsmContext.fsmInstanceList.erase(msgIter);
            break;
        }
    }
    FSM_UNLOCK(s_gFsmContext.listLock);
}

static FsmState* FSM_FindState(FSM_INSTANCE_S* pstFsmInstance, const STG_FSM_STATE_S* pstInState)
{
    list<FsmState*>::iterator msgIter = pstFsmInstance->fsmStateList.begin();
    FsmState* pstFsmState = HI_NULL;

    for(;msgIter != pstFsmInstance->fsmStateList.end();msgIter++)
    {
        pstFsmState = *msgIter;
        string stateName(pstInState->aszStateName);
        if(stateName == pstFsmState->name())
        {
            return pstFsmState;
        }
    }

    //STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "could not find state: %s \n", pstInState->aszStateName);
    return HI_NULL;
}

HI_S32 STG_FSM_Create(HI_VOID** ppStateMachine, HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadLen, const HI_CHAR* pszFsmName)
{
    FSM_INSTANCE_S* pstFsmInstance = new FSM_INSTANCE_S();
    if(!pstFsmInstance)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "malloc FSM_INSTANCE_S failed\n");
        return HI_FAILURE;
    }
    pstFsmInstance->pstStateMachine = HI_NULL;
    pstFsmInstance->pstStateMachine = new StgStateMachine(u32MaxQueueSize, u32MaxMsgPayloadLen, pszFsmName);

    pstFsmInstance->fsmStateList.clear();

    pstFsmInstance->pstFsmOberver = HI_NULL;

    (HI_VOID)pthread_mutex_init(&pstFsmInstance->instanceLock, HI_NULL);

    FSM_AddInstance(pstFsmInstance);

    *ppStateMachine = (HI_VOID*)pstFsmInstance;
    return HI_SUCCESS;
}

HI_S32 STG_FSM_Destroy(HI_VOID* pStateMachine)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    HI_S32 s32Ret = HI_SUCCESS;

    if(pstFsmInstance->pstStateMachine)
    {
        s32Ret = pstFsmInstance->pstStateMachine->stop();
        if(s32Ret != HI_SUCCESS)
        {
            STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "pstStateMachine stop failed\n");
        }
        delete pstFsmInstance->pstStateMachine;
        pstFsmInstance->pstStateMachine = HI_NULL;
    }

    if(pstFsmInstance->pstFsmOberver)
    {
        delete pstFsmInstance->pstFsmOberver;
        pstFsmInstance->pstFsmOberver = HI_NULL;
    }

    list<FsmState*>::iterator msgIter = pstFsmInstance->fsmStateList.begin();
    while(msgIter != pstFsmInstance->fsmStateList.end())
    {
        FsmState* pstFsmState = *msgIter;
        delete pstFsmState;
        msgIter = pstFsmInstance->fsmStateList.erase(msgIter);
    }

    (HI_VOID)pthread_mutex_destroy(&pstFsmInstance->instanceLock);

    FSM_DelInstance(pstFsmInstance);

    delete pstFsmInstance ;

    return HI_SUCCESS;
}

HI_S32 STG_FSM_AddState(HI_VOID* pStateMachine, const STG_FSM_STATE_S* pstState)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    FSM_CHECK_NULL(pstState);
    HI_S32 s32Ret = HI_SUCCESS;
    string stateName(pstState->aszStateName);

    FSM_LOCK(pstFsmInstance->instanceLock);
    FsmState* pstFsmState = FSM_FindState(pstFsmInstance, pstState);
    if(pstFsmState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "state: %s already exist\n", pstState->aszStateName);
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    pstFsmState = new FsmState(pstState->pfnHandleEvent, pstState->pfnEnterState, pstState->pfnExitState, pstState->pPriv, stateName);

    pstFsmInstance->fsmStateList.push_back(pstFsmState);

    s32Ret = pstFsmInstance->pstStateMachine->addState(pstFsmState);
    FSM_UNLOCK(pstFsmInstance->instanceLock);
    return s32Ret;
}

HI_S32 STG_FSM_RegMsgCallback(HI_VOID* pStateMachine, STG_FSM_EVENT_CALLBACK pfnEventCallback, HI_VOID* pPriv)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    FSM_CHECK_NULL(pfnEventCallback);
    HI_S32 s32Ret = HI_SUCCESS;
    FSM_LOCK(pstFsmInstance->instanceLock);

    FSMOberver* pstObserver = new FSMOberver(pfnEventCallback, pPriv);

    s32Ret = pstFsmInstance->pstStateMachine->registerObserver(pstObserver);
    if(s32Ret != HI_SUCCESS)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "pstStateMachine registerObserver failed\n");
        delete pstObserver;
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    if(pstFsmInstance->pstFsmOberver)
    {
        delete pstFsmInstance->pstFsmOberver;
    }

    pstFsmInstance->pstFsmOberver = pstObserver;

    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return HI_SUCCESS;
}

HI_S32 STG_FSM_AddTransition(HI_VOID* pStateMachine, const STG_FSM_STATE_TRANS_S* pstStateTrans)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    FSM_CHECK_NULL(pstStateTrans);
    FSM_CHECK_NULL(pstStateTrans->pstSrcState);
    FSM_CHECK_NULL(pstStateTrans->pstDstState);

    FSM_LOCK(pstFsmInstance->instanceLock);

    FsmState* pstFsmSrcState = FSM_FindState(pstFsmInstance, pstStateTrans->pstSrcState);
    if(!pstFsmSrcState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "could not find state: %s \n", pstStateTrans->pstSrcState->aszStateName);
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    FsmState* pstFsmDstState = FSM_FindState(pstFsmInstance, pstStateTrans->pstDstState);
    if(!pstFsmDstState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "could not find state: %s \n", pstStateTrans->pstDstState->aszStateName);
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    pstFsmSrcState->addTransition(pstStateTrans->s32MsgType, pstFsmDstState);
    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return HI_SUCCESS;
}

HI_S32 STG_FSM_SetInitState(HI_VOID* pStateMachine, const STG_FSM_STATE_S* pstState)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    FSM_CHECK_NULL(pstState);
    HI_S32 s32Ret = HI_SUCCESS;

    FSM_LOCK(pstFsmInstance->instanceLock);

    FsmState* pstFsmState = FSM_FindState(pstFsmInstance, pstState);
    if(!pstFsmState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "could not find state: %s \n", pstState->aszStateName);
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    s32Ret = pstFsmInstance->pstStateMachine->setInitialState(pstFsmState);
    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return s32Ret;
}

HI_S32 STG_FSM_Start(HI_VOID* pStateMachine)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    HI_S32 s32Ret = HI_SUCCESS;

    FSM_LOCK(pstFsmInstance->instanceLock);

    s32Ret = pstFsmInstance->pstStateMachine->start();

    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return s32Ret;
}

HI_S32 STG_FSM_Stop(HI_VOID* pStateMachine)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);

    HI_S32 s32Ret = HI_SUCCESS;

    FSM_LOCK(pstFsmInstance->instanceLock);

    s32Ret = pstFsmInstance->pstStateMachine->stop();

    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return s32Ret;
}

HI_S32 STG_FSM_GetCurrentState(HI_VOID* pStateMachine, HI_CHAR aszStateName[MAX_STATE_NAME_LEN])
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);

    FSM_LOCK(pstFsmInstance->instanceLock);

    StgState* pState = pstFsmInstance->pstStateMachine->currentState();
    if(!pState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "current state is NULL\n");
        FSM_UNLOCK(pstFsmInstance->instanceLock);
        return HI_FAILURE;
    }

    strncpy_s(aszStateName, MAX_STATE_NAME_LEN, pState->name().c_str(), MAX_STATE_NAME_LEN-1);

    FSM_UNLOCK(pstFsmInstance->instanceLock);

    return HI_SUCCESS;
}

HI_S32 STG_FSM_SendAsync(HI_VOID* pStateMachine, STG_MSG_INFO_S* pstMsg, HI_U64 u64Delay)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);

    return pstFsmInstance->pstStateMachine->post(pstMsg, u64Delay);
}

HI_S32 STG_FSM_SendSync(HI_VOID* pStateMachine, STG_MSG_INFO_S* pstMsg, STG_MSG_INFO_S* pstReplyMsg)
{
    FSM_INSTANCE_S* pstFsmInstance = (FSM_INSTANCE_S*)pStateMachine;
    CHECK_FSM_HANDLE_EXIST(FSM_CheckInstanceExist, pstFsmInstance);
    (HI_VOID)(pstReplyMsg);
    return pstFsmInstance->pstStateMachine->send(pstMsg);
}
