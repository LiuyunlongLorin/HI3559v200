#ifndef __AVPLAY_MESSAGE_LOOPER_H__
#define __AVPLAY_MESSAGE_LOOPER_H__

#include "hi_type.h"
#include "StgMsg.h"
#include "StgMsgHandler.h"
#include <list>
#include <string>
#include "StgMsgPool.h"

using namespace std;

#define MAX_MSG_WAIT_TIME_US (500*1000)

typedef struct tagMsgEvent
{
    STG_MSG_INFO_S* msg;
    HI_U64 u64WhenUs;
}MsgEvent;

class StgMsgLooper
{
public:
    StgMsgLooper(HI_U32 u32MaxQueueSize, HI_U32 u32MaxMsgPayloadLen, string name);
    virtual ~StgMsgLooper();
    int registerHandler(StgMsgHandler* handler);
    int start();
    int stop();
    int send(const STG_MSG_INFO_S* pstInMsg);
    int post(const STG_MSG_INFO_S* pstInMsg, HI_U64 u64delayUs);
    bool hasMessage(const STG_MSG_INFO_S* pstMsg);
    void removeMessage(const STG_MSG_INFO_S* pstMsg);
    void dump();

private:
    static HI_VOID* Looper(HI_VOID* args);
    HI_VOID QueueHandlr();
    HI_VOID insertQueue(STG_MSG_INFO_S* msg, HI_U64 u64delayUs);
    void lockHandler();
    void unlockHandler();

private:
    StgMsgHandler* mMsgHandlr;
    HI_BOOL bThreadRunning;
    list<MsgEvent> mEventQueue;
    pthread_cond_t  mQueueChangedCondition;
    pthread_mutex_t mQueueLock;
    pthread_t mLoopThrd;
    string mName;
    pthread_mutex_t mHandlerMutex;

    StgMsgPool* mMsgPool;

    StgMsgLooper(const StgMsgLooper &);
    StgMsgLooper &operator=(const StgMsgLooper &);
};
#endif //__AVPLAY_MESSAGE_LOOPER_H__
