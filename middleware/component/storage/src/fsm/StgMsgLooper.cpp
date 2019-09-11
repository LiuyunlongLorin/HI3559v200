#include "securec.h"
#include "stg_fsm_common.h"
#include "StgMsgLooper.h"
#include "stg_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iterator>
#include <sys/prctl.h>
#include <errno.h>

using namespace std;

#ifdef __HuaweiLite__
#define MSGLOOPER_LITEOS_STACK_SIZE (0x10000)
#endif

#define MODULE_NAME_MSGLOOPER "StgMsgLooper"

StgMsgLooper::StgMsgLooper(HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadLen, string name) : mName(name)
{
    mMsgHandlr = HI_NULL;
    bThreadRunning = HI_FALSE;
    mLoopThrd = 0;
    mEventQueue.clear();
    pthread_mutex_init(&mHandlerMutex, NULL);
    pthread_mutex_init(&mQueueLock, HI_NULL);
    STG_FSM_Cond_InitRelative(&mQueueChangedCondition);

    mMsgPool = new StgMsgPool(name);
    if (mMsgPool->init(u32MaxQueueSize, u32MaxMsgPayloadLen) != HI_SUCCESS)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "mMsgPool init failed\n");
    }
}

StgMsgLooper::~StgMsgLooper()
{
    STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s before destruct\n", mName.c_str());
    mEventQueue.clear();

    FSM_LOCK(mQueueLock);
    bThreadRunning = HI_FALSE;
    (HI_VOID)pthread_cond_broadcast(&mQueueChangedCondition);
    FSM_UNLOCK(mQueueLock);

    (HI_VOID)pthread_mutex_destroy(&mQueueLock);
    (HI_VOID)pthread_cond_destroy(&mQueueChangedCondition);

    mMsgPool->deinit();
    delete mMsgPool;
    mMsgPool = NULL;
    STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s after destruct\n", mName.c_str());
}

HI_S32 StgMsgLooper::registerHandler(StgMsgHandler* handler)
{
    if (bThreadRunning)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "looper be in running, do not support reg handlr\n");
        return HI_FAILURE;
    }

    mMsgHandlr = handler;
    return HI_SUCCESS;
}

HI_VOID* StgMsgLooper::Looper(HI_VOID* args)
{
    StgMsgLooper* pLooper = (StgMsgLooper*)args;
    pLooper->QueueHandlr();

    return HI_NULL;
}

HI_S32 StgMsgLooper::start()
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (bThreadRunning)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "looper already be running\n");
        return HI_SUCCESS;
    }

    if (!mMsgHandlr)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "no message handlr be registered\n");
        return HI_FAILURE;
    }

    bThreadRunning = HI_TRUE;
#ifdef __HuaweiLite__
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, MSGLOOPER_LITEOS_STACK_SIZE);
    s32Ret = pthread_create(&mLoopThrd, &attr, Looper, this);
#else
    s32Ret = pthread_create(&mLoopThrd, HI_NULL, Looper, this);
#endif
    if (0 != s32Ret)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR,
                    "pthread_create failed %d\n", s32Ret);
#ifdef __HuaweiLite__
        pthread_attr_destroy(&attr);
#endif
        bThreadRunning = HI_FALSE;
        return HI_FAILURE;
    }

#ifdef __HuaweiLite__
    pthread_attr_destroy(&attr);
#endif

    return HI_SUCCESS;
}

HI_S32 StgMsgLooper::stop()
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (bThreadRunning)
    {
        FSM_LOCK(mQueueLock);
        bThreadRunning = HI_FALSE;
        (HI_VOID)pthread_cond_broadcast(&mQueueChangedCondition);
        FSM_UNLOCK(mQueueLock);

        pthread_join(mLoopThrd, HI_NULL);

        bThreadRunning = HI_FALSE;

        FSM_LOCK(mQueueLock);
        if (!mEventQueue.empty())
        {
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR,
                "%s have msg in queue, could not stop error\n", mName.c_str());
            s32Ret =  HI_FAILURE;
        }
        FSM_UNLOCK(mQueueLock);

    }

    mMsgPool->dump();

    return s32Ret;
}

HI_VOID StgMsgLooper::QueueHandlr()
{
    HI_S32 s32Ret = HI_SUCCESS;

    prctl(PR_SET_NAME, mName.c_str(), 0, 0, 0);

    while (bThreadRunning)
    {
        FSM_LOCK(mQueueLock);
        if(!bThreadRunning)// must check value again inside lock
        {
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_INFO, "%s will stop\n", mName.c_str());
            FSM_UNLOCK(mQueueLock);
            break;
        }
        if (mEventQueue.empty())
        {
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s wait condition\n", mName.c_str());
            FSM_COND_WAIT(mQueueChangedCondition, mQueueLock);
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s recived condition\n", mName.c_str());
            FSM_UNLOCK(mQueueLock);
            continue;
        }

        HI_U64 u64WhenUs = (*mEventQueue.begin()).u64WhenUs;
        HI_U64 u64NowUs = STG_FSM_GetCurTimeUs();

        if (u64WhenUs > u64NowUs)
        {
            HI_U64 u64delayUs = u64WhenUs - u64NowUs;
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s timedwait condition, delayUs=%llu\n", mName.c_str(), u64delayUs);
            s32Ret = STG_FSM_Cond_Timewait(&mQueueChangedCondition, &mQueueLock, u64delayUs);
            if (s32Ret != ETIMEDOUT && s32Ret != HI_SUCCESS)
            {
                STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_INFO, "STG_FSM_Cond_Timewait err: %d\n", s32Ret);
            }
            STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_DEBUG, "%s timedwait recived condition\n", mName.c_str());
            FSM_UNLOCK(mQueueLock);
            continue;
        }

        MsgEvent event = *mEventQueue.begin();
        mEventQueue.erase(mEventQueue.begin());

        FSM_UNLOCK(mQueueLock);

        lockHandler();
        mMsgHandlr->onMessageReceived(event.msg);
        unlockHandler();

        // statemachine.post() set bNeedReply=HI_FALSE, for there is no requirment by now.
        if (event.msg->bNeedReply && event.msg->pfnCallback)
        {
            event.msg->pfnCallback(event.msg->pPrivate);
        }

        mMsgPool->putMsg(event.msg);
    }
}

HI_S32 StgMsgLooper::send(const STG_MSG_INFO_S* pstInMsg)
{
    lockHandler();
    HI_S32 s32Ret = mMsgHandlr->onMessageReceived(pstInMsg);
    unlockHandler();

    return s32Ret;
}

HI_S32 StgMsgLooper::post(const STG_MSG_INFO_S* pstInMsg, HI_U64 u64delayUs)
{
    if (!bThreadRunning)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "post failed, looper is stoped\n");
        return  HI_FAILURE;
    }

    STG_MSG_INFO_S* pstMsg = mMsgPool->getMsg(pstInMsg->s32What, pstInMsg->u32Arg1,
                                          pstInMsg->u32Arg2, pstInMsg->pMsgData, pstInMsg->u32MsgDataLen);
    if (!pstMsg)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_ERR, "getMsg err\n");
        return HI_FAILURE;
    }

    pstMsg->bNeedReply = HI_FALSE;
    insertQueue(pstMsg, u64delayUs);

    return HI_SUCCESS;
}

HI_VOID StgMsgLooper::insertQueue(STG_MSG_INFO_S* msg, HI_U64 u64delayUs)
{
    HI_U64 u64whenUs;
    HI_U64 u64CurUs = STG_FSM_GetCurTimeUs();

    u64whenUs = (u64delayUs > 0) ? (u64CurUs + u64delayUs) : u64CurUs;

    FSM_LOCK(mQueueLock);

    MsgEvent event;
    event.u64WhenUs = u64whenUs;
    event.msg = msg;

    if (mEventQueue.empty())
    {
        mEventQueue.push_back(event);
        FSM_COND_SIGNAL(mQueueChangedCondition);
    }
    else
    {
        list<MsgEvent>::iterator it = mEventQueue.begin();
        while (it != mEventQueue.end() && (*it).u64WhenUs <= u64whenUs)
        {
            ++it;
        }

        mEventQueue.insert(it, event);

        if (it == mEventQueue.begin())
        {
            FSM_COND_SIGNAL(mQueueChangedCondition);
        }
    }

    FSM_UNLOCK(mQueueLock);
}

bool StgMsgLooper::hasMessage(const STG_MSG_INFO_S* pstMsg)
{
    bool bHasMsg = false;

    FSM_LOCK(mQueueLock);

    list<MsgEvent>::iterator it = mEventQueue.begin();
    while (it != mEventQueue.end())
    {
        if (it->msg->s32What == pstMsg->s32What)
        {
            bHasMsg = true;
            break;
        }
        ++it;
    }

    FSM_UNLOCK(mQueueLock);
    return bHasMsg;
}

void StgMsgLooper::removeMessage(const STG_MSG_INFO_S* pstMsg)
{
    MsgEvent event = {HI_NULL, 0};
    HI_BOOL bFound = HI_FALSE;

    FSM_LOCK(mQueueLock);

    list<MsgEvent>::iterator it = mEventQueue.begin();
    while (it != mEventQueue.end())
    {
        if (it->msg->s32What == pstMsg->s32What)
        {
            event = *it;
            mEventQueue.erase(it);
            bFound = HI_TRUE;
            break;
        }
        ++it;
    }

    FSM_UNLOCK(mQueueLock);

    if (bFound)
    {
        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_INFO,
            "%s remove msg: %d \n", mName.c_str(), event.msg->s32What);
        mMsgPool->putMsg(event.msg);
    }
}

void StgMsgLooper::dump()
{
    FSM_LOCK(mQueueLock);

    STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_INFO,
        "looper[%s], msg cnt: %zd \n", mName.c_str(), mEventQueue.size());
    list<MsgEvent>::iterator it = mEventQueue.begin();
    while (it != mEventQueue.end())
    {

        STG_LOG_Printf(MODULE_NAME_MSGLOOPER, STG_LOG_LEVEL_INFO,
            "msg id[%d], handleWhen[%llu]\n", it->msg->s32What, it->u64WhenUs);
        ++it;
    }
    FSM_UNLOCK(mQueueLock);
    mMsgPool->dump();
}

void StgMsgLooper::lockHandler()
{
    pthread_mutex_lock(&mHandlerMutex);
}

void StgMsgLooper::unlockHandler()
{
    pthread_mutex_unlock(&mHandlerMutex);
}