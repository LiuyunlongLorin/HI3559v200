#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "hi_datafifo.h"

static const HI_S32 BLOCK_LEN = 1024;
static HI_DATAFIFO_HANDLE hDataFifo = HI_DATAFIFO_INVALID_HANDLE;

int datafifo_init(HI_U64 phyAddr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DATAFIFO_PARAMS_S params = {10, BLOCK_LEN, HI_TRUE, DATAFIFO_READER};

    s32Ret = HI_DATAFIFO_OpenByAddr(&hDataFifo, &params, phyAddr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    printf("datafifo_init finish\n");

    return 0;
}

static HI_BOOL s_bStop = HI_FALSE;

void* read_more(void* arg)
{
    HI_U32 readLen = 0;
    HI_CHAR* pBuf;
    HI_S32 s32Ret = HI_SUCCESS;

    while (HI_FALSE == s_bStop)
    {
        readLen = 0;
        s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get available read len error:%x\n", s32Ret);
            break;
        }

        if (readLen > 0)
        {
            s32Ret = HI_DATAFIFO_Read(hDataFifo, (HI_VOID**)&pBuf);
            if (HI_SUCCESS != s32Ret)
            {
                printf("read error:%x\n", s32Ret);
                break;
            }

            printf("receive: %s\n", pBuf);

            s32Ret = HI_DATAFIFO_CMD(hDataFifo, DATAFIFO_CMD_READ_DONE, pBuf);
            if (HI_SUCCESS != s32Ret)
            {
                printf("read done error:%x\n", s32Ret);
                break;
            }

            continue;
        }

        usleep(800000);
    }

    return NULL;
}

void datafifo_deinit()
{
    HI_DATAFIFO_Close(hDataFifo);
    printf("datafifo_deinit finish\n");
}

int main(int argc, char** argv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR cmd[64];
    pthread_t readThread = -1;

    if (2 != argc)
    {
        printf("Usage: %s PhyAddr\n", argv[0]);
        return -1;
    }

    HI_U64 phyAddr;
    sscanf(argv[1], "%llx", &phyAddr);
    s32Ret = datafifo_init(phyAddr);

    if (s32Ret != 0)
    {
        return s32Ret;
    }

    printf("press any key to start. \n");
    getchar();
    
    s_bStop = HI_FALSE;
    pthread_create(&readThread, NULL, read_more, NULL);

    do
    {
        printf("Input q to exit: \n");
    }
    while ( 0 != strncmp(fgets(cmd, 64, stdin), "q", 1) );

    s_bStop = HI_TRUE;

    if (-1 != readThread)
    {
        pthread_join(readThread, NULL);
    }

    printf("press any key to stop. \n");
    getchar();
    
    datafifo_deinit();

    return 0;
}


