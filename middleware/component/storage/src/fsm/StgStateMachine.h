/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgStateMachine.h
* @brief     finite state  machine header file
* @author    HiMobileCam middleware develop team
* @date      2017.03.14
*/

#ifndef __HI_STATEMACHINE_H__
#define __HI_STATEMACHINE_H__

#include <vector>
#include <queue>
#include <pthread.h>

#include "StgState.h"
#include "StgStateMachineObserver.h"
#include "StgMsgLooper.h"

/** \addtogroup     STATEMACHINE */
/** @{ */  /** <!-- [STATEMACHINE] */

using namespace std;

class StgStateMachine : public StgMsgHandler
{
public:
    StgStateMachine(HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadSize, string SMName);
    virtual ~StgStateMachine();

    int addState(StgState *pState);

    int setInitialState(StgState *pState);

    StgState* currentState();

    int registerObserver(StgStateMachineObserver *pObserver);

    int start(void);

    int stop(void);

    /* handled immediately */
    int send(STG_MSG_INFO_S* pstMsg);

    /* handled immediately */
    int send(HI_S32 s32What);

    /* handled async */
    int post(STG_MSG_INFO_S* pstMsg, HI_U64 u64DelayUs);

    /* handled async */
    int post(HI_S32 s32What, HI_U64 u64DelayUs);

    int removeEvent(const STG_MSG_INFO_S* pstMsg);

private:
    int onMessageReceived(const STG_MSG_INFO_S* pstMsg);
    bool stateExist(StgState *pState);
    void transition(StgState *pNextState);

private:
    StgMsgLooper* mLooper;
    StgState *mCurrentState;
    StgState *mFinalState;
    vector<StgState*> mAllStates;
    StgStateMachineObserver* mObserver;
    bool mbRunning;
};

#endif//__HI_STATEMACHINE_H__
