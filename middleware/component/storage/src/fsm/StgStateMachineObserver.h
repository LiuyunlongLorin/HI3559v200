/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgStateMachine.h
* @brief     finite state  machine header file
* @author    HiMobileCam middleware develop team
* @date      2017.03.14
*/

#ifndef __HI_STATEMACHINE_OBSERVER_H__
#define __HI_STATEMACHINE_OBSERVER_H__

#include "StgState.h"

/** \addtogroup     STATEMACHINE */
/** @{ */  /** <!-- [STATEMACHINE] */

using namespace std;

class StgStateMachine;

class StgStateMachineObserver
{
    public:
        StgStateMachineObserver();
        virtual ~StgStateMachineObserver();

        virtual void onFinalState(const StgState *pState);
        virtual void onEventHandled(StgStateMachine* pStateMachine, int event, int result) = 0;
};

#endif//__HI_STATEMACHINE_OBSERVER_H__