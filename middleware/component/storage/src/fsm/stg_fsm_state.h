#ifndef __FSM_STATE_H__
#define __FSM_STATE_H__

#include <string>
#include "hi_type.h"
#include "stg_fsm.h"
#include "StgState.h"

using namespace std;

class FsmState: public StgState
{
public:
    FsmState(STG_FSM_StateHandleEvent eventHandle, STG_FSM_StateEnter enterState, STG_FSM_StateExit exitState, HI_VOID* priv, string name)
        : StgState(name) , mEventHandler(eventHandle), mStateEnter(enterState), mStateExit(exitState), mPriv(priv){}

    virtual ~FsmState(){}

    int handleMessage(const STG_MSG_INFO_S* pstMsg)
    {
        return mEventHandler(mPriv, pstMsg);
    }

    int enter()
    {
        return mStateEnter(mPriv, name().c_str());
    }

    int exit()
   {
        return mStateExit(mPriv, name().c_str());
    }

private:
    STG_FSM_StateHandleEvent mEventHandler;
    STG_FSM_StateEnter mStateEnter;
    STG_FSM_StateExit mStateExit;
    HI_VOID* mPriv;
};

#endif
