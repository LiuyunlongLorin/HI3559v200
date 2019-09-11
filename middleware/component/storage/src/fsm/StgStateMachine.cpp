/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgStateMachine.cpp
* @brief     state machine
* @author    HiMobileCam middleware develop team
* @date      2017.03.14  */

#include "stg_fsm_common.h"
#include "StgStateMachine.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>

#ifndef MW_VERSION
#define MW_VERSION "0.0.0.0"
#endif

static volatile const HI_CHAR HFSM_VERSIONINFO[] = "HIBERRY StgStateMachine Version: " MW_VERSION;
static volatile const HI_CHAR HFSM_BUILD_DATE[] = "HIBERRY StgStateMachine Build Date:" __DATE__" Time:" __TIME__;

StgStateMachine::StgStateMachine(HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadSize, string SMName)
{
    mCurrentState = HI_NULL;
    mFinalState = HI_NULL;
    mObserver = HI_NULL;
    mbRunning = false;

    mLooper = new StgMsgLooper(u32MaxQueueSize, u32MaxMsgPayloadSize, SMName);
    mLooper->registerHandler(this);
}

StgStateMachine::~StgStateMachine()
{
    stop();

    mAllStates.clear();
    delete mLooper;
    mLooper = NULL;
}

int StgStateMachine::addState(StgState* pState)
{
    if (mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "can't addState when running\n");
        return HI_FAILURE;
    }

    if (!stateExist(pState))
    {
        mAllStates.push_back(pState);
    }
    else
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "state already exist\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

int StgStateMachine::registerObserver(StgStateMachineObserver* pObserver)
{
    if (mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "can't registerObserver when running\n");
        return HI_FAILURE;
    }

    mObserver = pObserver;
    return HI_SUCCESS;
}

int StgStateMachine::setInitialState(StgState* pState)
{
    if (mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "can't set initstae when running\n");
        return HI_FAILURE;
    }

    if (stateExist(pState))
    {
        mCurrentState = pState;
    }
    else
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "init state not exist:%s\n", pState->mName.c_str());
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

StgState* StgStateMachine::currentState()
{
    return mCurrentState;
}

void StgStateMachine::transition(StgState* pNextState)
{
    if (stateExist(pNextState) )
    {
        mCurrentState->exit();
        pNextState->enter();
        mCurrentState = pNextState;
    }
    else
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "transition state not exist:%s\n", pNextState->mName.c_str());
    }
}

int StgStateMachine::start()
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "already running\n");
        return HI_FAILURE;
    }

    if (!mCurrentState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "initstate not set\n");
        return HI_FAILURE;
    }

    s32Ret = mLooper->start();
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    mbRunning = HI_TRUE;

    return HI_SUCCESS;
}

int StgStateMachine::stop()
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (mbRunning)
    {
        s32Ret = mLooper->stop();
        mbRunning = false;
    }

    return s32Ret;
}

HI_S32 StgStateMachine::send(STG_MSG_INFO_S* pstMsg)
{
    return mLooper->send(pstMsg);
}

HI_S32 StgStateMachine::send(HI_S32 s32What)
{
    STG_MSG_INFO_S stMsg;
    stMsg.s32What = s32What;
    stMsg.u32Arg1 = 0;
    stMsg.u32Arg2 = 0;
    stMsg.pMsgData = NULL;
    stMsg.u32MsgDataLen = 0;
    stMsg.bNeedReply = HI_FALSE;
    stMsg.pPrivate = NULL;
    stMsg.pfnCallback = NULL;

    return mLooper->send(&stMsg);
}

int StgStateMachine::post(STG_MSG_INFO_S* pstMsg, HI_U64 u64DelayUs)
{
    if (!mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "post fail: FSM stoped\n");
        return HI_FAILURE;
    }

    return mLooper->post(pstMsg, u64DelayUs);
}

int StgStateMachine::post(HI_S32 s32What, HI_U64 u64DelayUs)
{
    STG_MSG_INFO_S stMsg;
    stMsg.s32What = s32What;
    stMsg.u32Arg1 = 0;
    stMsg.u32Arg2 = 0;
    stMsg.pMsgData = NULL;
    stMsg.u32MsgDataLen = 0;
    stMsg.bNeedReply = HI_FALSE; //no requirment to reply by now, should check it in the future
    stMsg.pPrivate = NULL;
    stMsg.pfnCallback = NULL;

    if (!mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "post fail: FSM stoped\n");
        return HI_FAILURE;
    }

    return mLooper->post(&stMsg, u64DelayUs);
}

int  StgStateMachine::removeEvent(const STG_MSG_INFO_S* pstMsg)
{
    if (!mbRunning)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "removeEvent fail: FSM stoped\n");
        return HI_FAILURE;
    }

    mLooper->removeMessage(pstMsg);
    return HI_SUCCESS;
}

int StgStateMachine::onMessageReceived(const STG_MSG_INFO_S* pstMsg)
{
    int s32Ret = HI_SUCCESS;

    if (!mCurrentState)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "initstate not set\n");
        return HI_FAILURE;
    }

    s32Ret = mCurrentState->handleMessage(pstMsg);
    if (s32Ret == HI_SUCCESS)
    {
        StgState* pNextState = mCurrentState->findTransition(pstMsg->s32What);
        if (pNextState)
        {
            transition(pNextState);
        }
    }

    if (mObserver)
    {
        mObserver->onEventHandled(this, pstMsg->s32What, s32Ret);
    }

    return s32Ret;
}

bool StgStateMachine::stateExist(StgState* pState)
{
    vector<StgState*>::iterator it;

    for (it = mAllStates.begin(); it != mAllStates.end(); it++)
    {
        if (*it == pState || (*it)->name() == pState->name())
        {
            return true;
        }
    }

    return false;
}