/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description:
 * Author: Hisilicon multimedia software group
 * Create: 2019-03-30
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>


#include "hi_common.h"
#include "hi_mipi_tx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */



static void Usage(void)
{
    printf(
        "\n"
        "*********************************************************************************************************\n"
#ifndef __HuaweiLite__
        "Usage: ./mipitx_read [devno] [datatpye] [readparamnum] [sendparamnum][param0]...[param1]\n"
        "datatype : DCS Data Type ,such as 0x14--generic read 1 param,0x24--generic read 2 param,\n"
        "           to use 0x14 or 0x24,please refer to screen spec\n"
        "readparamnum: the num of data (byte) you want to get from the screen.\n"
        "sendparamnum: the num of data (byte),you want to send to the screen.\n"
#else
        "Usage:   mipitx_read [devno] [datatpye] [readparamnum] [sendparamnum][param0]...[param1]\n"
        "datatype : DCS Data Type ,such as 0x14--generic read 1 param,0x24--generic read 2 param,\n"
        "           to use 0x14 or 0x24,please refer to screen spec\n"
        "readparamnum: the num of data (byte) you want to get from the screen.\n"
        "sendparamnum: the num of data (byte),you want to send to the screen.\n"
#endif
#ifndef __HuaweiLite__
        "   e.g : read 1 byte from screen ,target register is 0xda\n"
        "                ./mipitx_read   0  0x14 0x1 0x1 0xda\n"
        "   e.g : read 3 bytes from screen ,target register is 0xa1,first set max return data num to 3 in step 1,\n"
        "         then use 0x14 or 0x24(refer to screen) to get data ,in step 2\n"
        "             step 1 ./mipitx_write  0  0x37 0x1 0x3      \n"
        "             step 2 ./mipitx_read   0  0x14 0x3 0x1 0xa1 \n"
#else
        "   e.g : read 1 byte from screen ,target register is 0xda\n"
        "                mipitx_read   0  0x14 0x1 0x1 0xda\n"
        "   e.g : read 3 bytes from screen ,target register is 0xa1,first set max return data num to 3 in step 1,\n"
        "         then use 0x14 or 0x24(refer to screen) to get data ,in step 2\n"
        "             step 1 mipitx_write  0  0x37 0x1 0x3      \n"
        "             step 2 mipitx_read   0  0x14 0x3 0x1 0xa1 \n"
#endif
        "*********************************************************************************************************\n"
        "\n");
}

#define INPUT_PARAM_NUM_BEFORE_SEND_DATA 5
#define MAX_SENDDATA_NUM  2
#define MAX_READ_DATA_NUM  100

static HI_S32 CheckParamNum(int argc, char* argv[])
{
    if (argc == 1) {
#ifndef __HuaweiLite__
        printf("\tTo see more usage, please enter: ./mipitx_read -h\n\n");
#else
        printf("\tTo see more usage, please enter: mipitx_read -h\n\n");
#endif
        return HI_FAILURE;
    }

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) {
            Usage();
            return HI_FAILURE;
        }
    }

    if (argc < INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("\t input not enough,use mipitx_read -h to get help\n\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


#ifdef __HuaweiLite__
HI_S32 MipitxRead(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    HI_S32  ret;
    HI_S32 i;
    HI_S32 fd;
    unsigned char readdata[MAX_READ_DATA_NUM];
    get_cmd_info_t getCmdInfo;
    HI_S32 sendParamNum;
    HI_S32 readParamNum;
    sendParamNum = -1;
    readParamNum = -1;

    ret = CheckParamNum(argc,argv);
    if(ret != HI_SUCCESS) {
        return HI_SUCCESS;
    }

    memset(readdata, 0x0, sizeof(MAX_READ_DATA_NUM));

    for (i = 0; i < argc; i++) {
        switch (i) {
            case 1:
                getCmdInfo.devno = strtol(argv[i], NULL, 0);
                break;

            case 2:
                getCmdInfo.data_type = strtol(argv[i], NULL, 0);
                break;

            case 3:
                readParamNum = strtol(argv[i], NULL, 0);
                break;

            case 4:
                sendParamNum = strtol(argv[i], NULL, 0);
                break;
        }
    }

    if (getCmdInfo.data_type <= 0) {
        printf("data_type input err\n");
        return HI_SUCCESS;
    }


    if (sendParamNum < 0) {
        printf("invalid paramnum\n");
        return HI_SUCCESS;
    }

    if (argc < sendParamNum + INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("please input enough param\n");
        return HI_SUCCESS;
    }

    if (sendParamNum > MAX_SENDDATA_NUM) {
        printf("sorry this tool only support sendparamnum less than %d param\n", MAX_SENDDATA_NUM);
        return HI_SUCCESS;
    }

    if (sendParamNum > MAX_READ_DATA_NUM) {
        printf("sorry this tool only support readparamnum less than %d param\n", MAX_READ_DATA_NUM);
        return HI_SUCCESS;
    }

    if (sendParamNum == 0) {
        getCmdInfo.data_param = 0;
    }

    if (sendParamNum == 1) {
        getCmdInfo.data_param = strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
    }

    if (sendParamNum == 2) {
        getCmdInfo.data_param = (strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0) |
                                (strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA + 1], NULL, 0) << 8));
    }

    fd = open("/dev/hi_mipi_tx", O_RDWR);

    if (fd < 0) {
        printf("open hi_mipi_tx dev failed\n");
        return HI_FAILURE;
    }

    getCmdInfo.devno = 0;
    getCmdInfo.get_data = readdata;
    getCmdInfo.get_data_size = readParamNum;
    ret = ioctl(fd, HI_MIPI_TX_GET_CMD, &getCmdInfo);

    if (ret != HI_SUCCESS) {
        printf("MIPI_TX GET CMD failed,please check ,may not use mipitx_set first\n");
        close(fd);
        return HI_SUCCESS;
    }

    for (i = 0; i < readParamNum; i++) {
        printf("readdata[%d] = 0x%x\n", i, getCmdInfo.get_data[i]);
    }

    close(fd);
    printf("read end\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

