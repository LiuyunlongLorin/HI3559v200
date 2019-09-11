/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      StgState.h
* @brief     state machine header file
* @author    HiMobileCam middleware develop team
* @date      2017.03.14
*/

#ifndef __STG_STATE_H__
#define __STG_STATE_H__

#include <vector>
#include <string>
#include <map>
#include "StgMsg.h"

/** \addtogroup     STATE_MACHINE */
/** @{ */  /** <!-- [STATE_MACHINE] */

using namespace std;

class StgState
{
    public:
        StgState(string name);
        virtual ~StgState();

        virtual void addTransition(int event, StgState *pState);

        virtual StgState* findTransition(int event);

        virtual int handleMessage(const STG_MSG_INFO_S* pstMsg) = 0;

        virtual int enter();

        virtual int exit();

        string name() const;

        bool operator ==  (const StgState& state);

        friend class StgStateMachine;

    private:
        map<int, StgState*> mTransitionMap;
        string mName;
};

/** @}*/  /** <!-- ==== STATE_MACHINE End ====*/

#endif //__STG_STATE_H__
