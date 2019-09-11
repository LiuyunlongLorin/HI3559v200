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
        "*******************************************************************************************************\n"

#ifndef __HuaweiLite__
        "Usage: ./mipitx_write [devno] [datatpye] [paramnum][param0][param1]...[paramn] \n"
        "datatype : DCS Data Type ,such as 0x05--DSC write,0x23--generic short write,0x29--generic long write\n"
        "           to use 0x05 or others to write,please refer to screen spec\n"
        "paramnum : the total num of param.(unit:byte)\n"
#else
        "Usage:   mipitx_write [devno] [datatpye] [paramnum][param0][param1]...[paramn] \n"
        "datatype : DCS Data Type ,such as 0x05--DSC write,0x23--generic short write,0x29--generic long write\n"
        "           to use 0x05 or others to write,please refer to screen spec\n"
        "paramnum : the total num of param.(unit:byte)\n"
#endif
#ifndef __HuaweiLite__
        "   e.g : ./mipitx_write 0  0x05 0x1 0x11\n"
        "   e.g : ./mipitx_write 0  0x23 0x2 0x12 0x34\n"
        "   e.g : ./mipitx_write 0  0x29 0x3 0x12 0x34 0x56\n"
#else
        "   e.g :   mipitx_write 0  0x05 0x1 0x11\n"
        "   e.g :   mipitx_write 0  0x23 0x2 0x12 0x34\n"
        "   e.g :   mipitx_write 0  0x29 0x3 0x12 0x34 0x56\n"
#endif
        "*******************************************************************************************************\n"
        "\n");

}
#define INPUT_PARAM_NUM_BEFORE_SEND_DATA 4
#define MAX_SEND_DATA_NUM  28
static HI_S32 CheckParamNum(int argc, char* argv[])
{
    if (argc == 1) {
#ifndef __HuaweiLite__
        printf("\tTo see more usage, please enter: ./mipitx_write -h\n\n");
#else
        printf("\tTo see more usage, please enter: mipitx_write -h\n\n");
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
        printf("\t input not enough,use mipitx_write -h to get help\n\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

#ifdef __HuaweiLite__
HI_S32 MipitxWrite(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    HI_S32  ret;
    HI_S32 i;
    HI_S32 fd;
    cmd_info_t cmdInfo;
    unsigned char sendData[MAX_SEND_DATA_NUM];
    HI_S32 sendParamNum;
    sendParamNum = -1;
      ret = CheckParamNum(argc,argv);
    if(ret != HI_SUCCESS) {
        return HI_SUCCESS;
    }

    memset(&cmdInfo, 0x0, sizeof(cmd_info_t));
    memset(sendData, 0x0, sizeof(MAX_SEND_DATA_NUM));


    for (i = 0; i < argc; i++) {
        switch (i) {
            case 1:
                cmdInfo.devno = strtol(argv[i], NULL, 0);
                break;

            case 2:
                cmdInfo.data_type = strtol(argv[i], NULL, 0);
                break;

            case 3:
                sendParamNum = strtol(argv[i], NULL, 0);
                break;
        }
    }

    if (cmdInfo.data_type <= 0) {
        printf("data_type input err ");
        return HI_SUCCESS;
    }

    if (sendParamNum > MAX_SEND_DATA_NUM) {
        printf("sorry this tool only support less than %d param\n", MAX_SEND_DATA_NUM);
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

    if (sendParamNum <= 2) {
        if (sendParamNum == 0) {
            cmdInfo.cmd_size = 0;
        }

        if (sendParamNum == 1) {
            cmdInfo.cmd_size = strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
        }

        if (sendParamNum == 2) {
            cmdInfo.cmd_size = (strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0) |
                               (strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA + 1], NULL, 0) << 8));
        }

        cmdInfo.cmd = NULL;
    } else {
        cmdInfo.cmd = sendData;
        cmdInfo.cmd_size = sendParamNum;

        for (i = 0; i < sendParamNum; i++) {
            sendData[i] = strtol((argv[i + INPUT_PARAM_NUM_BEFORE_SEND_DATA]), NULL, 0);
        }
    }


    fd = open("/dev/hi_mipi_tx", O_RDWR);

    if (fd < 0) {
        printf("open hi_mipi_tx dev failed\n");
        return HI_FAILURE;
    }

    ret = ioctl(fd, HI_MIPI_TX_SET_CMD, &cmdInfo);

    if (ret != HI_SUCCESS) {
        printf("MIPI_TX SET CMD failed\n");
        close(fd);
        return HI_FAILURE;
    }

    close(fd);

    printf("write end\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

