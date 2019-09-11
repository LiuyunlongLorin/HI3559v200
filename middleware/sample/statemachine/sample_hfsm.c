#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#include "hi_message.h"
#include "state_common.h"

static HI_MW_PTR s_HFSMHDL;

int msgHandleMessage(HI_MESSAGE_S* pMsg)
{
    printf("[%s][%d] pMsg->what:%d arg1:%d arg2:%d object:%p\n",__FUNCTION__,__LINE__, pMsg->what, pMsg->arg1, pMsg->arg2, pMsg->aszPayload);
    return 0;
}

HI_S32 hfsmEventCallBack(HI_MW_PTR pHfsmHandle, const HI_HFSM_EVENT_INFO_S* pstEventInfo)
{
    HI_MESSAGE_S *pMsg;
    printf("[%s][%d] pstEventInfo->enEventCode:%d\n",__FUNCTION__,__LINE__, pstEventInfo->enEventCode);
    switch(pstEventInfo->enEventCode)
    {
    case HI_HFSM_EVENT_UNHANDLE_MSG:
        pMsg = pstEventInfo->unEventInfo.stunHandlerMsgInfo.pstunHandlerMsg;
        printf("[%s][%d] pMsg->what:%d arg1:%d arg2:%d object:%p\n",__FUNCTION__,__LINE__, pMsg->what, pMsg->arg1, pMsg->arg2, pMsg->aszPayload);
        break;
    case HI_HFSM_EVENT_TRANSTION_ERROR:
        printf("[%s][%d] pOrigSate->name:%s pDestSate->name:%s pCurrentSate->name:%s s32ErrorNo:%d\n",__FUNCTION__,__LINE__,
                pstEventInfo->unEventInfo.stTranstionInfo.pstOrgState->name,
                pstEventInfo->unEventInfo.stTranstionInfo.pstCurrentState->name,
                pstEventInfo->unEventInfo.stTranstionInfo.pstCurrentState->name,
                pstEventInfo->unEventInfo.stTranstionInfo.s32ErrorNo);
        break;
    case HI_HFSM_EVENT_BUTT:
        break;
    }
    return HI_SUCCESS;
}

void stateInit()
{
}

void stateCreate()
{
    HI_HFSM_ATTR_S stHsmAttr = {hfsmEventCallBack, 32, 32};
    HI_HFSM_Create(&stHsmAttr, &s_HFSMHDL);
    HI_HFSM_AddState(s_HFSMHDL,&Idle,NULL);

    HI_HFSM_AddState(s_HFSMHDL,&Record,&Idle);
    HI_HFSM_AddState(s_HFSMHDL,&Normal,&Record);
    HI_HFSM_AddState(s_HFSMHDL,&Loop,&Record);
    HI_HFSM_AddState(s_HFSMHDL,&Lapse,&Record);
    HI_HFSM_AddState(s_HFSMHDL,&Slow,&Record);

    HI_HFSM_AddState(s_HFSMHDL,&Photo,&Idle);
    HI_HFSM_AddState(s_HFSMHDL,&Single,&Photo);
    HI_HFSM_AddState(s_HFSMHDL,&Delay,&Photo);

    HI_HFSM_AddState(s_HFSMHDL,&PlayBack,&Idle);

    HI_HFSM_SetInitialState(s_HFSMHDL,HI_STATE_REC_NORMAL);
}

void stateStart()
{
    HI_HFSM_Start(s_HFSMHDL);
}

void stateStop()
{
    HI_HFSM_Stop(s_HFSMHDL);
}

void stateDestroy()
{
    HI_HFSM_Destroy(s_HFSMHDL);
}

void stateDeInit()
{
}
static HI_BOOL bRun = HI_TRUE;
#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef __HuaweiLite__
    /*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);
    /*init cpp running context on liteos end*/
#endif
    stateInit();
    stateCreate();

    stateStart();
    HI_S32 i = 0;
    HI_MESSAGE_S msg = {100, 33, 44, 0, 0, ""};
    HI_S32 s32Ret = HI_HFSM_SendAsyncMessage(s_HFSMHDL, &msg);
    if(HI_SUCCESS != s32Ret)
    {
        printf("[%s][%d] HI_HFSM_SendMessage error:0x%x\n",__FUNCTION__,__LINE__, s32Ret);
    }
    sleep(1);
    stateStop();
    stateDestroy();
    return 0;
}
