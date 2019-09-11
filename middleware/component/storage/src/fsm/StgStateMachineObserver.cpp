/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgStateMachine.cpp
* @brief     state machine
* @author    HiMobileCam middleware develop team
* @date      2017.03.14  */

#include <iostream>
#include "StgStateMachineObserver.h"

StgStateMachineObserver::StgStateMachineObserver()
{
}

StgStateMachineObserver::~StgStateMachineObserver()
{
}

void StgStateMachineObserver::onFinalState(const StgState *pState)
{
    (void)(pState);
}