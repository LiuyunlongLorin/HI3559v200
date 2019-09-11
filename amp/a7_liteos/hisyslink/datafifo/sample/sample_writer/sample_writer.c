#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "hi_datafifo.h"

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
#include "osal_mmz.h"

#endif

static HI_S32 g_s32Index = 0;
static HI_DATAFIFO_HANDLE hDataFifo = HI_DATAFIFO_INVALID_HANDLE;
static const HI_S32 BLOCK_LEN = 1024;

static void release(void* pStream)
{
    printf("release %p\n", pStream);
}

static int datafifo_init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_DATAFIFO_PARAMS_S params = {10, BLOCK_LEN, HI_TRUE, DATAFIFO_WRITER};

    s32Ret = HI_DATAFIFO_Open(&hDataFifo, &params);

    if (HI_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    HI_U64 phyAddr = 0;
    s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }

    printf("PhyAddr: %llx\n", phyAddr);

    s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, release);

    if (HI_SUCCESS != s32Ret)
    {
        printf("set release func callback error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init finish\n");

    return 0;
}


static HI_BOOL s_bStop = HI_FALSE;

void* send_more(void* arg)
{
    HI_CHAR buf[BLOCK_LEN];
    HI_S32 s32Ret = HI_SUCCESS;

    while (HI_FALSE == s_bStop)
    {
        HI_U32 availWriteLen = 0;

        // call write NULL to flush
        s32Ret = HI_DATAFIFO_Write(hDataFifo, NULL);
        if (HI_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
        }

        s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get available write len error:%x\n", s32Ret);
            break;
        }

        if (availWriteLen >= BLOCK_LEN)
        {
            memset(buf, 0, BLOCK_LEN);
            snprintf(buf, BLOCK_LEN, "========%d========", g_s32Index);
            s32Ret = HI_DATAFIFO_Write(hDataFifo, buf);
            if (HI_SUCCESS != s32Ret)
            {
                printf("write error:%x\n", s32Ret);
                break;
            }

            printf("send: %s\n", buf);

            s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_WRITE_DONE, NULL);
            if (HI_SUCCESS != s32Ret)
            {
                printf("write done error:%x\n", s32Ret);
                break;
            }

            g_s32Index++;
        }
        else
        {
            //printf("no free space: %d\n", availWriteLen);
        }

        usleep(500000);
    }

    return NULL;
}

void datafifo_deinit(void)
{
    HI_S32 s32Ret = HI_SUCCESS;

    // call write NULL to flush and release stream buffer.
    s32Ret = HI_DATAFIFO_Write(hDataFifo, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }

    HI_DATAFIFO_Close(hDataFifo);
    printf("datafifo_deinit finish\n");
}

void write_msg(void)
{
    HI_CHAR cmd[64];
    HI_S32 s32Ret = HI_SUCCESS;
    pthread_t sendThread = -1;

    s32Ret = datafifo_init();
    if (0 != s32Ret)
    {
        return;
    }

    printf("press any key to start. \n");
    getchar();

    s_bStop = HI_FALSE;
    pthread_create(&sendThread, NULL, send_more, NULL);

    do
    {
        printf("Input q to exit: \n");
    }
    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1));

    s_bStop = HI_TRUE;

    if (sendThread != -1)
    {
        pthread_join(sendThread, NULL);
    }

    printf("press any key to stop. \n");
    getchar();

    datafifo_deinit();

    return;
}

#ifdef __LITEOS__

extern int _ipcm_vdd_init(void);
extern int virt_tty_dev_init(void);
extern void osal_proc_init(void);

static HI_S32 MMZ_init(void)
{
    extern int media_mem_init(void * pArgs);
    MMZ_MODULE_PARAMS_S stMMZ_Param;
#if (defined hi3559av100 || defined hi3559av100)
    const HI_U64 u64MMZStartAddr = 0x90000000ULL;
    const HI_U64 u64MMZShareAddr = 0x80000000ULL;
#elif defined hi3519av100
    const HI_U64 u64MMZStartAddr = 0x52000000ULL;
    const HI_U64 u64MMZShareAddr = 0x32000000ULL;
#elif defined hi3516cv500
    const HI_U64 u64MMZStartAddr = 0x88000000ULL;
    const HI_U64 u64MMZShareAddr = 0x9d000000ULL;
#else
    #error please choose chip type
#endif
    snprintf(stMMZ_Param.mmz, MMZ_SETUP_CMDLINE_LEN, "anonymous,0,0x%llx,%dM", u64MMZStartAddr, 256);
    snprintf(stMMZ_Param.map_mmz, MMZ_SETUP_CMDLINE_LEN, "0x%llx,%dM", u64MMZShareAddr, 48);

    stMMZ_Param.anony = 1;

    printf("mmz param= %s\n", stMMZ_Param.mmz);

    return media_mem_init(&stMMZ_Param);
}


static void writer_cmd_register(void)
{
    osCmdReg(CMD_TYPE_EX, "sample_writer", 0, (CMD_CBK_FUNC)write_msg);
}

void app_init(void)
{
    int ret = 0;

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

    osal_proc_init();

    ret = MMZ_init();
    if (ret != 0)
    {
        printf("MMZ init error.\n");
    }

    writer_cmd_register();

}

#else
int main(int argc, char** argv)
{
    write_msg();

    return 0;
}
#endif





