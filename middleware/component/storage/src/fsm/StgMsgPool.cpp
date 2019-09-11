#include "securec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stg_fsm_common.h"
#include "StgMsgPool.h"
//#include <iostream>


#define DEFAULT_MSG_ID (-1)
#define DEFAULT_MSG_ARG (0)

StgMsgPool::StgMsgPool(string name) : mName(name)
{
    mMsgPayloadList = HI_NULL;
    mMsgMemList = HI_NULL;
    mMsgPrivList = HI_NULL;
    mMaxMsgNum = 0;
    mMaxMsgPayloadLen = 0;
    mMsgFreeList.clear();
    mMsgBusyList.clear();

    (HI_VOID)pthread_mutex_init(&mListLock, HI_NULL);
}

StgMsgPool::~StgMsgPool()
{
    (HI_VOID)pthread_mutex_destroy(&mListLock);
}

HI_S32 StgMsgPool::init(HI_U32 u32MaxMsgNum, HI_U32 u32MaxMsgPayloadLen)
{
    HI_U32 i = 0;

    if (!u32MaxMsgNum)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "msgpool init msgNum 0 err\n");
        return HI_FAILURE;
    }

    if (u32MaxMsgPayloadLen > 0)
    {
        mMsgPayloadList = (HI_U8*)malloc(u32MaxMsgPayloadLen * u32MaxMsgNum);
        if (!mMsgPayloadList)
        {
            STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR,
                "alloc msg payload list size: %u failed\n",
                (u32MaxMsgPayloadLen * u32MaxMsgNum));
            return HI_FAILURE;
        }
        memset_s(mMsgPayloadList, u32MaxMsgPayloadLen * u32MaxMsgNum, 0x00, (u32MaxMsgPayloadLen * u32MaxMsgNum));
    }

    mMsgMemList = (STG_MSG_INFO_S*)malloc(sizeof(STG_MSG_INFO_S) * u32MaxMsgNum);
    if (!mMsgMemList)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "alloc msg list num: %u failed\n", u32MaxMsgNum);
        free(mMsgPayloadList);
        mMsgPayloadList = HI_NULL;
        return HI_FAILURE;
    }
    memset_s(mMsgMemList, sizeof(STG_MSG_INFO_S) * u32MaxMsgNum, 0x00, (sizeof(STG_MSG_INFO_S)*u32MaxMsgNum));

    mMsgPrivList = (MSG_PRIVATE_INFO_S*)malloc(sizeof(MSG_PRIVATE_INFO_S) * u32MaxMsgNum);
    if (!mMsgPrivList)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "alloc msg private list num: %u failed\n", u32MaxMsgNum);
        free(mMsgPayloadList);
        mMsgPayloadList = HI_NULL;
        free(mMsgMemList);
        mMsgMemList = HI_NULL;
        return HI_FAILURE;
    }
    memset_s(mMsgPrivList, sizeof(MSG_PRIVATE_INFO_S) * u32MaxMsgNum, 0x00, (sizeof(MSG_PRIVATE_INFO_S)*u32MaxMsgNum));

    for (i = 0; i < u32MaxMsgNum; i++)
    {
        mMsgMemList[i].pMsgData = (u32MaxMsgPayloadLen > 0) ? (HI_VOID*)&mMsgPayloadList[i * u32MaxMsgPayloadLen] : HI_NULL;
        mMsgMemList[i].pPrivate = (HI_VOID*)&mMsgPrivList[i];
        mMsgMemList[i].s32What = DEFAULT_MSG_ID;
        mMsgFreeList.push_back(&mMsgMemList[i]);
    }

    mMaxMsgNum = u32MaxMsgNum;
    mMaxMsgPayloadLen = u32MaxMsgPayloadLen;

    return HI_SUCCESS;
}

HI_S32 StgMsgPool::deinit()
{
    if (!mMsgBusyList.empty())
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR,
            "pool: %s: %d msgs not released\n", mName.c_str(), (HI_U32)mMsgBusyList.size());
        return HI_FAILURE;
    }

    if (mMsgFreeList.size() != mMaxMsgNum)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "free msg size not same with inited\n");
        return HI_FAILURE;
    }

    list<STG_MSG_INFO_S*>::iterator msgIter = mMsgFreeList.begin();
    while (msgIter != mMsgFreeList.end())
    {
        msgIter = mMsgFreeList.erase(msgIter);
    }
    mMsgFreeList.clear();
    mMsgBusyList.clear();

    if (mMsgPrivList)
    {
        free(mMsgPrivList);
        mMsgPrivList = HI_NULL;
    }

    if (mMsgPayloadList)
    {
        free(mMsgPayloadList);
        mMsgPayloadList = HI_NULL;
    }

    if (mMsgMemList)
    {
        free(mMsgMemList);
        mMsgMemList = HI_NULL;
    }

    return HI_FAILURE;
}

STG_MSG_INFO_S* StgMsgPool::getEmptyMsg()
{
    STG_MSG_INFO_S* msg = HI_NULL;

    FSM_LOCK(mListLock);

    list<STG_MSG_INFO_S*>::iterator msgIter = mMsgFreeList.begin();

    if (msgIter != mMsgFreeList.end())
    {
        msg = *msgIter;
        mMsgFreeList.erase(msgIter);
    }

    if (msg == HI_NULL)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "getMsg failed, msg pool is empty\n");
        FSM_UNLOCK(mListLock);
        return HI_NULL;
    }

    memset_s(msg->pPrivate, sizeof(MSG_PRIVATE_INFO_S), 0x00, sizeof(MSG_PRIVATE_INFO_S));
    msg->s32What = DEFAULT_MSG_ID;
    msg->u32MsgDataLen = mMaxMsgPayloadLen;

    mMsgBusyList.push_back(msg);

    FSM_UNLOCK(mListLock);

    return msg;
}

STG_MSG_INFO_S* StgMsgPool::getMsg(HI_S32 s32What, HI_U32 u32Arg1,
                                 HI_U32 u32Arg2, HI_VOID* pMsgData, HI_U32 u32MsgDataLen)
{
    STG_MSG_INFO_S* msg = HI_NULL;

    if (u32MsgDataLen > mMaxMsgPayloadLen)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "getMsg input msg len: %d large than maxlen: %d\n",
                   u32MsgDataLen, mMaxMsgPayloadLen);
        return HI_NULL;
    }

    FSM_LOCK(mListLock);

    list<STG_MSG_INFO_S*>::iterator msgIter = mMsgFreeList.begin();
    if (msgIter != mMsgFreeList.end())
    {
        msg = *msgIter;

        memset_s(msg->pPrivate, sizeof(MSG_PRIVATE_INFO_S), 0x00, sizeof(MSG_PRIVATE_INFO_S));

        msg->s32What = s32What;
        msg->u32Arg1 = u32Arg1;
        msg->u32Arg2 = u32Arg2;

        if (u32MsgDataLen > 0 && pMsgData
            && msg->pMsgData)
        {
            memcpy_s(msg->pMsgData, u32MsgDataLen, pMsgData, u32MsgDataLen);
            msg->u32MsgDataLen = u32MsgDataLen;
        }

        mMsgFreeList.erase(msgIter);
    }

    if (msg == HI_NULL)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR,
            "pool: %s is empty, left size: %zd busy size: %zd\n",
            mName.c_str(), mMsgFreeList.size(), mMsgBusyList.size());

        FSM_UNLOCK(mListLock);
        return HI_NULL;
    }

    mMsgBusyList.push_back(msg);

    FSM_UNLOCK(mListLock);

    return msg;
}

HI_S32 StgMsgPool::putMsg(STG_MSG_INFO_S* msg)
{
    HI_BOOL bFound = HI_FALSE;

    FSM_LOCK(mListLock);

    list<STG_MSG_INFO_S*>::iterator msgIter = mMsgBusyList.begin();
    for (; msgIter != mMsgBusyList.end(); msgIter++)
    {
        STG_MSG_INFO_S* pMsg = *msgIter;
        if (pMsg == msg)
        {
            bFound = HI_TRUE;
            break;
        }
    }

    if (!bFound)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_ERR, "pool: %s putMsg invalid msg\n", mName.c_str());
        FSM_UNLOCK(mListLock);
        return HI_FAILURE;
    }

    mMsgBusyList.erase(msgIter);

    msg->s32What = DEFAULT_MSG_ID;

    mMsgFreeList.push_back(msg);

    FSM_UNLOCK(mListLock);
    return HI_SUCCESS;
}

HI_VOID StgMsgPool::dump()
{
    FSM_LOCK(mListLock);
    STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_INFO,
        "pool: %s, free msg: %zd, busy msg: %zd\n",
        mName.c_str(), mMsgFreeList.size(), mMsgBusyList.size());

    list<STG_MSG_INFO_S*>::iterator msgIter = mMsgBusyList.begin();
    for (; msgIter != mMsgBusyList.end(); msgIter++)
    {
        STG_LOG_Printf(MOD_NAME_FSM, STG_LOG_LEVEL_INFO, "msg id: %d\n", (*msgIter)->s32What);
    }
    FSM_UNLOCK(mListLock);
}
