/**
* @file    hi_storage.c
* @brief   sim storage implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <string.h>

#include "hi_math.h"
#include "hi_appcomm.h"
#include "hi_hal_gsensor.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

//SIM-STORAGE
#define SHM_PATH "/dev/shm"
#define SHM_NAME "simgsensor"



static HI_U8* gsensor = NULL;

HI_S32 HI_HAL_GSENSOR_Init(const HI_HAL_GSENSOR_CFG_S* pstCfg)
{
    HI_S32 fd;
    HI_S32 s32Ret;
    HI_S32 size = 1;

    s32Ret = mkdir(SHM_PATH, 0755);

    if (s32Ret < 0)
    {
        MLOGD("%s exit\n", SHM_PATH);
    }

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd < 0)
    {
        MLOGE("shm_open failed\n");
        return HI_FAILURE;
    }

    ftruncate(fd, size);

    gsensor = (HI_U8*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (gsensor == NULL)
    {
        MLOGE("mmap failed\n");
        return HI_FAILURE;
    }
    gsensor[0] = 0xff;
    return HI_SUCCESS;
}


HI_S32 HI_HAL_GSENSOR_GetCollisionStatus(HI_BOOL * pbOnCollison)
{
    *pbOnCollison = (HI_BOOL)gsensor[0];
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GSENSOR_SetSensitity(HI_HAL_GSENSOR_SENSITITY_E enSensitity)
{
    return HI_SUCCESS;
}

HI_S32 HI_HAL_GSENSOR_SetAttr(const HI_HAL_GSENSOR_ATTR_S* pstAttr)
{
    return HI_SUCCESS;

}


HI_S32 HI_HAL_GSENSOR_Deinit()
{
    HI_S32 s32Ret;
    HI_S32 size = 1;

    s32Ret = munmap(gsensor, size);

    if (s32Ret < 0)
    {
        MLOGE("munmap failed\n");
        return HI_FAILURE;
    }

    s32Ret = shm_unlink(SHM_NAME);

    if (s32Ret < 0)
    {
        MLOGE("shm_unlinkshm failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


