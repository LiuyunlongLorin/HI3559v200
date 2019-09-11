#ifndef __AVPLAY_MESSAGE_HANDLER_H__
#define __AVPLAY_MESSAGE_HANDLER_H__

#include "hi_type.h"
#include "StgMsg.h"

using namespace std;

class StgMsgHandler
{
public:
    StgMsgHandler(){};
    virtual HI_S32 onMessageReceived(const STG_MSG_INFO_S* pstMsg) = 0;
    virtual ~StgMsgHandler(){};

private:
    StgMsgHandler(const StgMsgHandler &);
    StgMsgHandler &operator=(const StgMsgHandler &);
};

#endif//__AVPLAY_MESSAGE_HANDLER_H__
