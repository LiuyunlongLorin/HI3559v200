#include <stdio.h>
#include <unistd.h>

#include "hi_hfsm.h"
#include "state_common.h"

HI_S32 IdleModeEnter(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 IdleModeExit(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 Idle_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1, pMsg->arg2);
    if(100 == pMsg->what)
    {
        *pStateID = SAMPLE_STATE_REC;
        sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    return HI_PROCESS_MSG_UNHANDLER;
}

HI_STATE_S Idle = {SAMPLE_STATE_IDLE, HI_STATE_IDLE,IdleModeEnter,IdleModeExit,Idle_processMessage,&Idle};

HI_S32 NormalModeEnter(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 NormalModeExit(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 Normal_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1, pMsg->arg2);
    if(100 == pMsg->what)
    {
        *pStateID = SAMPLE_STATE_REC_LOOP;
        sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    return HI_PROCESS_MSG_UNHANDLER;
}

HI_STATE_S Normal = {SAMPLE_STATE_REC_NORMAL, HI_STATE_REC_NORMAL,NormalModeEnter,NormalModeExit,Normal_processMessage,&Normal};


HI_S32 RecordModeEnter(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 RecordModeExit(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_FAILURE;
}

HI_S32 Record_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1, pMsg->arg2);
    if(100 == pMsg->what)
    {
        *pStateID = SAMPLE_STATE_REC_NORMAL;
        sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
    }
    return HI_PROCESS_MSG_UNHANDLER;
}

HI_STATE_S Record = {SAMPLE_STATE_REC, HI_STATE_REC,RecordModeEnter,RecordModeExit,Record_processMessage,&Idle};

HI_S32 LoopModeEnter(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 LoopModeExit(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 Loop_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_REC_LAPSE;
       sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}
HI_STATE_S Loop = {SAMPLE_STATE_REC_LOOP, HI_STATE_REC_LOOP, LoopModeEnter, LoopModeExit, Loop_processMessage, &Loop };

HI_S32 LapseModeEnter(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 LapseModeExit(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 Lapse_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_REC_SLOW;
       sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}

HI_STATE_S Lapse =
        {SAMPLE_STATE_REC_LAPSE,  HI_STATE_REC_LAPSE, LapseModeEnter, LapseModeExit, Lapse_processMessage, &Lapse };

#define ENTER(mode) mode##StateEnter
#define EXIT(mode)  mode##StateEXIT

HI_S32 ENTER(Slow)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}

HI_S32 EXIT(Slow)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 Slow_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_PHOTO;
       sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}

HI_STATE_S Slow = {SAMPLE_STATE_REC_SLOW,   HI_STATE_REC_SLOW, ENTER(Slow), EXIT(Slow), Slow_processMessage, &Slow };


HI_S32 ENTER(Photo)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 EXIT(Photo)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 Photo_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_PHOTO_SINGLE;
       sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}
HI_STATE_S Photo = {SAMPLE_STATE_PHOTO,  HI_STATE_PHOTO,ENTER(Photo),EXIT(Photo),Photo_processMessage,&Photo};


HI_S32 ENTER(Single)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 EXIT(Single)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 Single_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_PHOTO_DELAY;
       sleep(3);
        return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}
HI_STATE_S Single = {SAMPLE_STATE_PHOTO_SINGLE,  HI_STATE_PHOTO_SINGLE,ENTER(Single),EXIT(Single),Single_processMessage,&Single};


HI_S32 ENTER(Delay)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 EXIT(Delay)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 Delay_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_PLAYBACK;
       sleep(3);
       return HI_PROCESS_MSG_RESULTE_OK;

   }
    return HI_PROCESS_MSG_UNHANDLER;
}
HI_STATE_S Delay    = {SAMPLE_STATE_PHOTO_DELAY,  HI_STATE_PHOTO_DELAY,ENTER(Delay),EXIT(Delay),Delay_processMessage,&Delay};

HI_S32 ENTER(PlayBack)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 EXIT(PlayBack)(HI_MW_PTR argv)
{
    printf("[%s] \n", __FUNCTION__);
    return HI_SUCCESS;
}
HI_S32 PlayBack_processMessage(HI_MESSAGE_S *pMsg, void *argv, HI_STATE_ID *pStateID)
{
    printf("[%s] pMsg->what:%d arg1:%d arg2:%d\n", __FUNCTION__, pMsg->what, pMsg->arg1,
            pMsg->arg2);
    if(100 == pMsg->what)
   {
        *pStateID = SAMPLE_STATE_IDLE;
       sleep(3);
    return HI_PROCESS_MSG_RESULTE_OK;
   }
    return HI_PROCESS_MSG_UNHANDLER;
}
HI_STATE_S PlayBack = {SAMPLE_STATE_PLAYBACK,  HI_STATE_PLAYBACK,ENTER(PlayBack),EXIT(PlayBack),PlayBack_processMessage,&PlayBack};
