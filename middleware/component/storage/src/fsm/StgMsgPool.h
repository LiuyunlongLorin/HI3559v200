#include <list>
#include "hi_type.h"
#include "StgMsg.h"
#include <string>

using namespace std;

typedef struct tagMSG_PRIVATE_INFO_S
{
    HI_BOOL bReplied;
    STG_MSG_INFO_S* pstReplyMsg;
    HI_BOOL bCondErr;
}MSG_PRIVATE_INFO_S;

class StgMsgPool
{
public:
    StgMsgPool(string name);
    ~StgMsgPool();
    HI_S32 init(HI_U32 u32MaxMsgNum, HI_U32 u32MaxMsgPayloadLen);
    HI_S32 deinit(HI_VOID);
    STG_MSG_INFO_S* getMsg(HI_S32 s32What, HI_U32 u32Arg1, HI_U32 u32Arg2, HI_VOID* pMsgData, HI_U32 u32MsgDataLen);
    HI_S32 putMsg(STG_MSG_INFO_S* msg);
    STG_MSG_INFO_S* getEmptyMsg();
    HI_VOID dump();

private:
    list<STG_MSG_INFO_S*> mMsgFreeList;
    list<STG_MSG_INFO_S*> mMsgBusyList;
    HI_U32 mMaxMsgNum;
    HI_U32 mMaxMsgPayloadLen;
    string mName;
    STG_MSG_INFO_S* mMsgMemList;
    MSG_PRIVATE_INFO_S* mMsgPrivList;
    HI_U8* mMsgPayloadList;
    pthread_mutex_t mListLock;
};
