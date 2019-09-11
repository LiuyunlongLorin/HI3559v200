/**
* @file    hi_dummy_gps.c
* @brief   product gps interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include<errno.h>

#include "hi_appcomm_log.h"
#include "hi_hal_gps_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define GPS_INFO_TYPE 8
#define HAL_GPS_STR_LEN 120

static HI_S32 g_deviceFd;
static HI_U32 g_readDataCnt;

static HI_CHAR g_gpsInfo[GPS_INFO_TYPE][HAL_GPS_STR_LEN] =
{
    {"$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62"},
    {"$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"},
    {"$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39"},
    {"$GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41"},
    {"$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48"},
    {"$GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74"},
    {"$GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D"},
    {"$GPGSV,4,2,11,08,51,203,30,09,45,215,28*75"},
};

static HI_S32 GPSDummyInit(HI_VOID)
{
    HI_S32 ret = HI_SUCCESS;
    HI_S32 i;
    off_t f_offset;
    MLOGI("GPSDummyInit\n");

    g_deviceFd = open("/dev/dummy.txt", O_RDWR | O_NOCTTY | O_NONBLOCK | O_CREAT);

    if (g_deviceFd == -1)
    {
        MLOGE("open failed\n");
        return HI_FAILURE;
    }

    for(i = 0; i < GPS_INFO_TYPE; i++)
    {
        ret = write(g_deviceFd, g_gpsInfo[i], strlen(g_gpsInfo[i]));
        if(ret < 0)
        {
            MLOGE("write failed:%d %s\n", errno, strerror(errno));
            return ret;
        }
        ret = write(g_deviceFd, "\n", 1);
    }

    f_offset = lseek(g_deviceFd, 0, SEEK_SET);

    if (f_offset == -1)
    {
        MLOGE("lseek error for \n");
        return f_offset;
    }


    return HI_SUCCESS;
}

static HI_S32 GPSDummyDeInit(HI_VOID)
{
    MLOGI("GPSDummyDeInit\n");
    g_deviceFd = -1;
    close(g_deviceFd);
    return HI_SUCCESS;
}

static HI_S32 GPSDummyGetRawData(HI_GPSDATA *gpsData)
{
    HI_S32 ret = HI_SUCCESS;
    off_t f_offset;

     if(NULL == gpsData)
    {
       MLOGE("gpsData is NULL pointer!\n");
       return HI_FAILURE;
    }

    ret = read(g_deviceFd, &gpsData->rawData, gpsData->wantReadLen);

    if (ret < 0)
    {
        if (EAGAIN == errno)
        {
            MLOGE("No data readable!\n");
            gpsData->actualReadLen = 0;
            return HI_SUCCESS;
        }
        else
        {
            perror("read data faild!\n");
            return HI_FAILURE;
        }
    }
    else if (ret == 0)
    {
        g_readDataCnt++;

        if (g_readDataCnt % 100 == 0)
        {
            MLOGD("g_readDataCnt:%d!\n", g_readDataCnt);
        }

        /* Reach the end of the file , Read from the beginning */
        f_offset = lseek(g_deviceFd, 0, SEEK_SET);

        if (f_offset == -1)
        {
            MLOGE("lseek error for \n");
            return f_offset;
        }

        return HI_SUCCESS;
    }

    gpsData->actualReadLen = ret;
    return HI_SUCCESS;
}

HI_GPSHAL_DEVICE g_gpsDevice =
{
    .Init = GPSDummyInit,
    .DeInit = GPSDummyDeInit,
    .GetRawData = GPSDummyGetRawData,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
