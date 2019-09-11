/**
* @file    hal_motionsensor.c
* @brief   hal motionsensor implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include "asm/io.h"

#include <spi.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "hal_motionsensor_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

#ifdef HI3556AV100
#ifdef BOARD_DEMB
#define SPI_DEV "/dev/spidev2.0"
#endif
#ifdef BOARD_ACTIONCAM_REFB
#define SPI_DEV "/dev/spidev1.0"
#endif
#endif

#ifdef HI3559V200
#define SPI_DEV "/dev/spidev1.0"
#endif

extern HI_U32 u32MotionSensorSpiNum;

static HI_VOID motionsensor_pin_mux(HI_VOID)
{
#ifdef HI3559V200
    MLOGI("Lorin add Default Param -> HI3559v200 Defined\n");
    u32MotionSensorSpiNum = 1; // SPI 1.0
    himm(0x112F0020, 0x000014f1);   //寄存器跟手册不一致啊
    himm(0x112f0024, 0x000014f1);
    himm(0x112f0028, 0x000014f1);
    himm(0x112f002C, 0x000014f1);
    //  himm(0x120c1020, 0x000014f1);   //寄存器跟手册不一致啊
    // himm(0x120c1024, 0x000014f1);
    // himm(0x120c1028, 0x000014f1);
    // himm(0x120c102C, 0x000014f1);

    //   himm(0x112F0020, 0x4f1);
    // himm(0x112F0024, 0x4f1);
    // himm(0x112F002c, 0x14f1);
    // himm(0x112F0028, 0x4f1)
#endif

#ifdef HI3556AV100
#ifdef BOARD_DEMB
    u32MotionSensorSpiNum = 2; // SPI 2.0
    himm(0x040580C4, 0x000014f1);
    himm(0x040580C8, 0x000014f1);
    himm(0x040580CC, 0x000014f1);
    himm(0x040580D0, 0x000014f1);
#endif
#ifdef BOARD_ACTIONCAM_REFB
    u32MotionSensorSpiNum = 1; // SPI 1.0
    himm(0x04058064, 0x000014f1);
    himm(0x04058068, 0x000014f1);
    himm(0x0405806C, 0x000014f1);
    himm(0x04058070, 0x000014f1);
#endif
#endif
}

static HI_S32 HAL_MOTIONSENSOR_SetSpiAttr(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 fd;
    HI_S32 s32mode = SPI_MODE_3; // SPI_LSB_FIRST
    HI_S32 s32bits = 8;
    HI_U64 u64speed = 10000000;

    fd = open(SPI_DEV, O_RDWR);
    if(fd < 0) {
        MLOGE("open");
        return HI_FAILURE;
    }
    MLOGI("Loirn add -> SPI_DEV = %s\n", SPI_DEV);
    /*set spi mode */
    s32Ret = ioctl(fd, SPI_IOC_WR_MODE, &s32mode);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't set spi mode");
        close(fd);
        return HI_FAILURE;
    }

    s32Ret = ioctl(fd, SPI_IOC_RD_MODE, &s32mode);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't get spi mode");
        close(fd);
        return HI_FAILURE;
    }

    /* bits per word*/
    s32Ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &s32bits);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't set bits per word");
        close(fd);
        return HI_FAILURE;
    }

    s32Ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &s32bits);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't get bits per word");
        close(fd);
        return HI_FAILURE;
    }

    /*set spi max speed*/
    s32Ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &u64speed);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't set bits max speed HZ");
        close(fd);
        return HI_FAILURE;
    }

    s32Ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &u64speed);
    if (s32Ret == HI_FAILURE) {
        MLOGE("can't set bits max speed HZ");
        close(fd);
        return HI_FAILURE;
    }

    close(fd);
    return s32Ret;
}

HI_S32 HAL_MOTIONSENSOR_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    motionsensor_pin_mux();
#ifdef HI3556AV100
#ifdef BOARD_DEMB
    himm(0x04510198, 0x6cff0000);
#endif
#ifdef BOARD_ACTIONCAM_REFB
    himm(0x04510198, 0x6aff0000);
#endif
#endif

    s32Ret = HAL_MOTIONSENSOR_SetSpiAttr();
    if (HI_SUCCESS != s32Ret) {
        MLOGE("Set_SPI_ATTR failed!\n");
        return HI_FAILURE;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
