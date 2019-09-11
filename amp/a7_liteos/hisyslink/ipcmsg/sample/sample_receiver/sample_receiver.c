#include <sys/prctl.h>
#include <stdio.h>
#include <pthread.h>
#include "hi_ipcmsg.h"

#ifdef __LITEOS__

#include "sys_config.h"
#include "board.h"
#include "los_typedef.h"

#ifdef LOSCFG_DRIVERS_UART
#include "console.h"
#include "hisoc/uart.h"
#include "uart.h"
#endif

#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif

#endif

void* rcv_thread(void* arg)
{
    prctl(PR_SET_NAME, "rcv_thread", 0, 0, 0);
    HI_S32* s32Id = (HI_S32*)arg;
    printf("receive from %d\n", *s32Id);
    HI_IPCMSG_Run(*s32Id);
    return NULL;
}

void handle_message(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* msg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    char content[32];

    printf("==============\n");
    printf("receive msg: %s, len: %d\n", (char*)msg->pBody, msg->u32BodyLen);

    memset(content, 0, 32);
    switch(msg->u32Module)
    {
        case 1:
            snprintf(content, 32, "modle:%d, cmd:%d, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        case 2:
            snprintf(content, 32, "modle:%d, cmd:%d, have done.", msg->u32Module, msg->u32CMD);
            s32Ret = 0;
            break;
        default:
            snprintf(content, 32, "modle:%d, cmd:%d, is not found.", msg->u32Module, msg->u32CMD);
            s32Ret = -1;
    }

    HI_IPCMSG_MESSAGE_S *respMsg = HI_IPCMSG_CreateRespMessage(msg, s32Ret, content, 32);
    HI_IPCMSG_SendAsync(s32Id, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);
    printf("==============\n\n");
}

void receive_msg(void)
{
    HI_S32 s32Id;
    int ret = 0;
    HI_IPCMSG_CONNECT_S stConnectAttr;

    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = 201;
    stConnectAttr.u32Priority = 0;
    ret = HI_IPCMSG_AddService("Test", &stConnectAttr);
    if(ret != 0)
    {
        printf("HI_IPCMSG_AddService return err:%x\n", ret);
    }

    if (HI_SUCCESS != HI_IPCMSG_Connect(&s32Id, "Test", handle_message))
    {
        printf("Connect fail\n");
        return;
    }

    pthread_t threadid;

    if (0 != pthread_create(&threadid, NULL, rcv_thread, &s32Id))
    {
        printf("pthread_create rcv_thread fail\n");
        return;
    }

    HI_CHAR cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Press q to quit\n");
    }

    HI_IPCMSG_Disconnect(s32Id);

    pthread_join(threadid, NULL);

    HI_IPCMSG_DelService("Test");

    printf("quit\n");
}

#ifdef __LITEOS__
extern int _ipcm_vdd_init(void);
extern int virt_tty_dev_init(void);

void receive_cmd_register(void)
{
    osCmdReg(CMD_TYPE_EX, "sample_receiver", 0, (CMD_CBK_FUNC)receive_msg);
}

void app_init(void)
{

    dprintf("ipcm init ...\n");
    _ipcm_vdd_init();
    dprintf("virt tty init ...\n");
    virt_tty_dev_init();

#ifdef LOSCFG_DRIVERS_UART
    dprintf("uart init ...\n");
    if (uart_dev_init() != 0)
    {
        PRINT_ERR("uart_dev_init failed");
    }
    if (virtual_serial_init(TTY_DEVICE) != 0)
    {
        PRINT_ERR("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0)
    {
        PRINT_ERR("system_console_init failed\n");
    }
#endif

    receive_cmd_register();

}

#else
int main(int argc, char** argv)
{

    receive_msg();

    return 0;
}
#endif



