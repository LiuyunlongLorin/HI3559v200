/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_vcap_motionsensor.c
 * @brief   server vcap motionsensor module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "hi_mapi_comm_define.h"
#include "hi_mapi_errno.h"
#include "mapi_vcap_inner.h"
#include "hi_mapi_vcap_define.h"
#include "mapi_comm_inner.h"
#ifdef SUPPORT_GYRO
#include "hi_comm_motionfusion.h"
#include "hi_comm_motionsensor.h"
#include "mpi_motionfusion.h"
#include "motionsensor_chip_cmd.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef SUPPORT_GYRO
static VCAP_MOTIONSENSOR_ATTR_S g_stGlobMotionAttr = {
    .s32MotionFd = -1,
    .bInitMotionSensor = HI_FALSE,
    .bSetDrift = HI_FALSE,
    .pVirAddr = 0L,
    .u64PhyAddr = 0L,
};

#define X_BUF_LEN             20000
#define GYRO_BUF_LEN          ((4 * 3 * X_BUF_LEN) + 8 * X_BUF_LEN)
#define GYRO_DEVICE           0
#define MOTIONSENSOR_DEV_NODE "/dev/motionsensor_chip"

HI_BOOL MAPI_VCAP_GetMotionSensorStatus(HI_VOID)
{
    return g_stGlobMotionAttr.bInitMotionSensor;
}

static HI_VOID MAPI_VCAP_ResetMotionSensorStatus(HI_VOID)
{
    g_stGlobMotionAttr.s32MotionFd = -1;
    g_stGlobMotionAttr.bInitMotionSensor = HI_FALSE;
    g_stGlobMotionAttr.bSetDrift = HI_FALSE;
    g_stGlobMotionAttr.pVirAddr = 0L;
    g_stGlobMotionAttr.u64PhyAddr = 0L;
}

HI_S32 MAPI_VCAP_SetMotionSensor(const HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
    HI_S32 s32Ret;
    HI_BOOL bEnDrift = HI_TRUE;
    HI_BOOL bEnSixSideCal = HI_TRUE;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    s32Ret = HI_MPI_MOTIONFUSION_SetAttr(GYRO_DEVICE, &pstMotionAttr->stMFusionAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_MOTIONFUSION_SetAttr fail.\n");

    s32Ret = HI_MPI_MOTIONFUSION_SetGyroSixSideCal(GYRO_DEVICE, bEnSixSideCal, pstMotionAttr->aRotationMatrix);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_MOTIONFUSION_SetGyroSixSideCal fail.\n");

    /* todo: the offonline drift may be needed in the future version */
    if (!g_stGlobMotionAttr.bSetDrift) {
        s32Ret = HI_MPI_MOTIONFUSION_SetGyroOnLineTempDrift(GYRO_DEVICE, bEnDrift, &pstMotionAttr->stTempDrift);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_MOTIONFUSION_SetGyroOnLineTempDrift fail.\n");
    }

    return s32Ret;
}

static HI_S32 MAPI_VCAP_GetMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
    HI_S32 s32Ret;
    HI_BOOL bEnDrift = HI_TRUE;
    HI_BOOL bEnSixSideCal = HI_TRUE;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);
    pstMotionAttr->stTempDrift.enMode = IMU_TEMP_DRIFT_LUT;

    /* todo: the offonline drift may be needed in the future version */
    s32Ret = HI_MPI_MOTIONFUSION_GetGyroOnLineTempDrift(GYRO_DEVICE, &bEnDrift, &pstMotionAttr->stTempDrift);
    CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_MOTIONFUSION_GetGyroOnlineTempDrift fail.\n");

    return s32Ret;
}

static HI_S32 MAPI_VCAP_InitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32BufSize = GYRO_BUF_LEN;
    HI_S32 fd;
    MSENSOR_BUF_ATTR_S stMotionBufAttr;
    MSENSOR_PARAM_S stMotionSet;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    memset(&stMotionBufAttr, 0x00, sizeof(MSENSOR_BUF_ATTR_S));

    fd = open(MOTIONSENSOR_DEV_NODE, O_RDWR);

    if (fd < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Error: cannot open MotionSensor device.may not load motionsensor driver !\n");
        return HI_MAPI_VCAP_EINVALID_FD;
    }

    s32Ret = HI_MPI_SYS_MmzAlloc(&stMotionBufAttr.u64PhyAddr, (HI_VOID **)&stMotionBufAttr.pVirAddr,
                                 "MotionsensorData", NULL, u32BufSize);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "set Gyro Param failed\n");
        s32Ret = HI_MAPI_VCAP_ENOMEM;
        goto GYRO_INIT_FAIL;
    }

    memset((HI_VOID *)stMotionBufAttr.pVirAddr, 0, u32BufSize);
    memset(&stMotionSet, 0, sizeof(MSENSOR_PARAM_S));
    stMotionBufAttr.u32Buflen = u32BufSize;

    // set device work mode
    stMotionSet.stMSensorAttr.u32DeviceMask = pstMotionAttr->stMFusionAttr.u32DeviceMask;
    stMotionSet.stMSensorAttr.u32TemperatureMask = pstMotionAttr->stMFusionAttr.u32TemperatureMask;

    // set gyro samplerate and full scale range
    stMotionSet.stMSensorConfig.stGyroConfig.u64ODR = 1000 * GRADIENT;
    stMotionSet.stMSensorConfig.stGyroConfig.u64FSR = pstMotionAttr->u32GyroFSR;

    // set accel samplerate and full scale range
    stMotionSet.stMSensorConfig.stAccConfig.u64ODR = 1000 * GRADIENT;
    stMotionSet.stMSensorConfig.stAccConfig.u64FSR = pstMotionAttr->u32AccFSR;

    memcpy(&stMotionSet.stMSensorBufAttr, &stMotionBufAttr, sizeof(MSENSOR_BUF_ATTR_S));

    s32Ret = ioctl(fd, MSENSOR_CMD_INIT, &stMotionSet);  //会读取SPI的设备号
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Gyro init fail\n");
        s32Ret = HI_MAPI_VCAP_EOPERATE_FAIL;
        goto GYRO_INIT_FAIL;
    }

    g_stGlobMotionAttr.s32MotionFd = fd;
    g_stGlobMotionAttr.u64PhyAddr = stMotionBufAttr.u64PhyAddr;
    g_stGlobMotionAttr.pVirAddr = (HI_U64)(HI_UL)stMotionBufAttr.pVirAddr;

    return s32Ret;

GYRO_INIT_FAIL:
    close(fd);
    fd = -1;

    return s32Ret;
}

static HI_S32 MAPI_VCAP_DeinitMotionSensor(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 fd;
    HI_U64 u64PhyAddr;
    HI_U64 pVirAddr;

    fd = g_stGlobMotionAttr.s32MotionFd;
    u64PhyAddr = g_stGlobMotionAttr.u64PhyAddr;
    pVirAddr = g_stGlobMotionAttr.pVirAddr;

    if (fd < 0) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "motionsensor dev is close,deinit Gyro fail\n");
        return HI_MAPI_VCAP_EINVALID_FD;
    }

    s32Ret = ioctl(fd, MSENSOR_CMD_DEINIT, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "motionsensor deinit failed \n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    s32Ret = HI_MPI_SYS_MmzFree(u64PhyAddr, (HI_VOID *)(HI_UL)pVirAddr);

    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "call  HI_MPI_SYS_MmzFree failed \n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    close(fd);

    g_stGlobMotionAttr.u64PhyAddr = 0L;
    g_stGlobMotionAttr.pVirAddr = 0L;
    g_stGlobMotionAttr.s32MotionFd = -1;

    return s32Ret;
}

static HI_S32 MAPI_VCAP_StartMotionSensor(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 fd;

    fd = g_stGlobMotionAttr.s32MotionFd;

    s32Ret = ioctl(fd, MSENSOR_CMD_START, NULL);
    if (s32Ret) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Gyro start fail\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return s32Ret;
}

static HI_S32 MAPI_VCAP_StopMotionSensor(HI_VOID)
{
    HI_S32 s32Ret;
    HI_S32 fd;

    fd = g_stGlobMotionAttr.s32MotionFd;

    s32Ret = ioctl(fd, MSENSOR_CMD_STOP, NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Gyro stop fail\n");
        return HI_MAPI_VCAP_EOPERATE_FAIL;
    }

    return s32Ret;
}
#endif

HI_S32 HI_MAPI_VCAP_InitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
#ifdef SUPPORT_GYRO
    HI_S32 s32Ret;

    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    if (g_stGlobMotionAttr.bInitMotionSensor) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "already Init GYRO\n");
        return HI_SUCCESS;
    }

    s32Ret = MAPI_VCAP_InitMotionSensor(pstMotionAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_InitGyro fail.\n");

    s32Ret = MAPI_VCAP_StartMotionSensor();
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StartGyro fail.\n");

    s32Ret = MAPI_VCAP_SetMotionSensor(pstMotionAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetMotionSensor fail.\n");

    g_stGlobMotionAttr.bInitMotionSensor = HI_TRUE;
    g_stGlobMotionAttr.bSetDrift = HI_TRUE;

    return HI_SUCCESS;
#else
    MAPI_UNUSED(pstMotionAttr);

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support MotionSensor.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}

HI_S32 HI_MAPI_VCAP_DeInitMotionSensor(HI_MAPI_MOTIONSENSOR_INFO_S *pstMotionAttr)
{
#ifdef SUPPORT_GYRO
    HI_S32 s32Ret;
    HI_BOOL bEnDrift = HI_FALSE;
    CHECK_MAPI_VCAP_NULL_PTR(pstMotionAttr);

    if (!g_stGlobMotionAttr.bInitMotionSensor) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "already deinit GYRO\n");
        return HI_SUCCESS;
    }

    s32Ret = MAPI_VCAP_GetMotionSensor(pstMotionAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_SetMotionSensor fail.\n");

    /* todo: the offonline drift may be needed in the future version */
    if (g_stGlobMotionAttr.bSetDrift) {
        s32Ret = HI_MPI_MOTIONFUSION_SetGyroOnLineTempDrift(GYRO_DEVICE, bEnDrift, &pstMotionAttr->stTempDrift);
        CHECK_MAPI_VCAP_RET(s32Ret, "call HI_MPI_MOTIONFUSION_SetGyroOnLineTempDrift fail.\n");
    }

    s32Ret = MAPI_VCAP_StopMotionSensor();
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_StopGyro fail.\n");

    s32Ret = MAPI_VCAP_DeinitMotionSensor();
    CHECK_MAPI_VCAP_RET(s32Ret, "call MAPI_VCAP_DeinitGyro fail.\n");

    g_stGlobMotionAttr.bInitMotionSensor = HI_FALSE;
    g_stGlobMotionAttr.bSetDrift = HI_FALSE;
    MAPI_VCAP_ResetMotionSensorStatus();

    return HI_SUCCESS;
#else
    MAPI_UNUSED(pstMotionAttr);

    MAPI_ERR_TRACE(HI_MAPI_MOD_VCAP, "Not support MotionSensor.\n");
    return HI_MAPI_VCAP_ENOTSUPPORT;
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
