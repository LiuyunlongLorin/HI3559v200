#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#ifdef __HuaweiLite__
#include "los_cppsupport.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#include "hi_message.h"
#include "hi_eventhub.h"

HI_EVENT_ID SD_OUT      = 0x00100200;
HI_EVENT_ID SD_IN       = 0x00100201;
HI_EVENT_ID WIFI_OUT    = 0x00100202;

static HI_BOOL s_bRun = HI_TRUE;
static HI_MW_PTR s_pSubscriber;
static HI_MW_PTR s_pSubscriber_sync;

HI_S32 EVTHUB_EventProc(HI_EVENT_S* pEvent, HI_VOID* argv)
{
    printf("[%s][%d] EventID:%d aszPayload:%s u64CreateTime:%lld argv:%p\n", __FUNCTION__, __LINE__,
            pEvent->EventID, &pEvent->aszPayload[0], pEvent->u64CreateTime, argv);
    return HI_SUCCESS;
}

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
    HI_S32 s32Ret = HI_SUCCESS;
    HI_EVTHUB_Init();
    sleep(1);
    HI_EVTHUB_Register(SD_OUT);
    HI_EVTHUB_Register(SD_IN);
    HI_EVTHUB_Register(WIFI_OUT);
    HI_SUBSCRIBER_S stSubscriber = {"event_text", EVTHUB_EventProc, &s_pSubscriber, HI_TRUE};
    HI_EVTHUB_CreateSubscriber(&stSubscriber, &s_pSubscriber);
    HI_SUBSCRIBER_S stSubscriber_Sync = {"event_text_sync", EVTHUB_EventProc, &s_pSubscriber_sync, HI_FALSE};
    HI_EVTHUB_CreateSubscriber(&stSubscriber_Sync, &s_pSubscriber_sync);
    HI_EVTHUB_Subscribe(s_pSubscriber_sync, SD_OUT);
    HI_EVTHUB_Subscribe(s_pSubscriber_sync, SD_IN);
    HI_EVTHUB_Subscribe(s_pSubscriber, SD_IN);
    HI_EVTHUB_Subscribe(s_pSubscriber, SD_OUT);
    HI_EVTHUB_Subscribe(s_pSubscriber, WIFI_OUT);

    HI_EVENT_S stEvent1 = {SD_IN, 0, 0, 0, time(NULL), "SD_IN"};
    s32Ret = HI_EVTHUB_Publish(&stEvent1);
    sleep(1);
    HI_EVENT_S stEvent2 = {SD_OUT, 0, 0, 0, time(NULL), "SD_OUT"};
    s32Ret = HI_EVTHUB_Publish(&stEvent2);
    sleep(1);
    HI_EVENT_S stEvent3 = {WIFI_OUT, 0, 0, 0, time(NULL), "WIFI_OUT"};
    s32Ret = HI_EVTHUB_Publish(&stEvent3);
    sleep(2);
    HI_EVTHUB_Deinit();
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
