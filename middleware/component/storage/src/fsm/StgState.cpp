/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgState.cpp
* @brief     used for finite state machine
* @author    HiMobileCam middleware develop team
* @date      2017.03.14  */

//#include <iostream>
#include <stdio.h>
#include "StgState.h"
#include "stg_fsm_common.h"

StgState::StgState(string name)
{
    mName = name;
}

StgState::~StgState()
{
    mTransitionMap.clear();
}

void StgState::addTransition(int event, StgState* pState)
{
    mTransitionMap.insert(pair<int, StgState*>(event, pState));
}

StgState* StgState::findTransition(int event)
{
    map<int, StgState*>::iterator it;

    it = mTransitionMap.find(event);
    if ( it == mTransitionMap.end() )
    {
        return NULL;
    }

    return it->second;
}

string StgState::name() const
{
    return mName;
}

bool StgState::operator==(const StgState& state)
{
    if (mName == state.name() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

int StgState::enter()
{
    return 0;
}

int StgState::exit()
{
    return 0;
}
