/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_client_motionfusion.c
 * @brief   vcap client motionfusion function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <stdio.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mpi_sys.h"
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "hi_mapi_vcap.h"
#include "mapi_vcap_inner.h"
#include "hi_type.h"
#include "hi_mapi_vcap_define.h"
#include "hi_ipcmsg.h"
#include "hi_comm_ipcmsg.h"
#include "hi_mapi_sys.h"
#include "msg_vcap.h"
#include "msg_define.h"
#include "msg_wrapper.h"
#include <errno.h>
#include "mapi_comm_inner.h"

HI_S32 HI_MAPI_VCAP_InitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, 0, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_INIT_MOTIONSENSOR, pstMotionAttr,
                           sizeof(HI_MAPI_MOTIONSENSOR_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_INIT_MOTIONSENSOR fail\n");

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_VCAP_DeInitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    u32ModFd = MODFD(HI_MAPI_MOD_VCAP, 0, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_VCAP_DEINIT_MOTIONSENSOR, pstMotionAttr,
                           sizeof(HI_MAPI_MOTIONSENSOR_INFO_S), HI_NULL);

    CHECK_MAPI_VCAP_RET(s32Ret, "MSG_CMD_VCAP_DEINIT_MOTIONSENSOR fail\n");

    return HI_SUCCESS;
}


