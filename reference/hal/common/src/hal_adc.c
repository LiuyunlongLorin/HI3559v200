/**
* @file    hal_adc.c
* @brief   product hal adc implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/18
* @version   1.0

*/
#include <fcntl.h>
#include <string.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>


#include "hi_type.h"
#include "hi_hal_adc_inner.h"

#include "hi_hal_common.h"
#include "hi_appcomm_util.h"
#include "hi_adc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#define HAL_ADC_MAXCNT                         (4)
#define HAL_ADC_SINGLE_MODE                    (0)
#define HAL_ADC_CONTINOUS_MODE                 (1)

static HI_S32 s_s32HALADCfd = HAL_FD_INITIALIZATION_VAL;
static pthread_mutex_t s_HALADCMutex = PTHREAD_MUTEX_INITIALIZER;

#define HAL_ADC_DEV    "/dev/hi_lsadc"

typedef struct tagHAL_ADC_STATE_S
{
    HI_BOOL bHALADCInitState;
} HAL_ADC_STATE_S;

typedef struct tagHAL_ADC_CTX_S
{
    HI_U32 u32InitHdlCnt;
    HAL_ADC_STATE_S astAdcState[HAL_ADC_MAXCNT];
} HAL_ADC_CTX_S;
static HAL_ADC_CTX_S s_sAdcCtx;

HI_S32 HAL_ADC_Init(HI_HANDLE AdcHdl)
{
    HI_S32 s32Model = HAL_ADC_CONTINOUS_MODE;
    HI_MUTEX_LOCK(s_HALADCMutex);

    if (HI_TRUE == s_sAdcCtx.astAdcState[AdcHdl].bHALADCInitState)
    {
        MLOGD("ADC[%d] has Already Init.\n",AdcHdl);
        HI_MUTEX_UNLOCK(s_HALADCMutex);
        return HI_SUCCESS;
    }

    /**Open ADC device once*/
    if (HAL_FD_INITIALIZATION_VAL == s_s32HALADCfd)
    {
        /** Initial ADC Device */
        s_s32HALADCfd = open(HAL_ADC_DEV, O_RDWR);

        if (HAL_FD_INITIALIZATION_VAL == s_s32HALADCfd)
        {
            MLOGE("[Error] open adcdev failed\n");
            HI_MUTEX_UNLOCK(s_HALADCMutex);
            return HI_HAL_EINVOKESYS;
        }

        if(ioctl(s_s32HALADCfd, LSADC_IOC_MODEL_SEL, &s32Model) <0)
        {
            MLOGE("[Error] Set LSADC Mode failed\n");
            close(s_s32HALADCfd);
            HI_MUTEX_UNLOCK(s_HALADCMutex);
            return HI_HAL_EINVOKESYS;
        }
    }

    /**Enable Chn*/
    if(ioctl(s_s32HALADCfd, LSADC_IOC_CHN_ENABLE, &AdcHdl) <0)
    {
        MLOGE("[Error] Enalbe LSADC Chn[%d] failed\n",AdcHdl);
        close(s_s32HALADCfd);
        HI_MUTEX_UNLOCK(s_HALADCMutex);
        return HI_HAL_EINVOKESYS;
    }

    /**Start Chn*/
    if(ioctl(s_s32HALADCfd, LSADC_IOC_START) <0)
    {
        MLOGE("[Error] Start LSADC failed\n");
        close(s_s32HALADCfd);
        HI_MUTEX_UNLOCK(s_HALADCMutex);
        return HI_HAL_EINVOKESYS;
    }

    s_sAdcCtx.u32InitHdlCnt++;
    s_sAdcCtx.astAdcState[AdcHdl].bHALADCInitState = HI_TRUE;
    HI_MUTEX_UNLOCK(s_HALADCMutex);
    return HI_SUCCESS;
}

HI_S32 HAL_ADC_GetVal(HI_HANDLE AdcHdl, HI_S32* ps32Value)
{
    if (HI_NULL == ps32Value)
    {
        MLOGE("[Error]Null pointer.\n");
        return HI_HAL_EINVAL;
    }

    if (HI_TRUE != s_sAdcCtx.astAdcState[AdcHdl].bHALADCInitState)
    {
        MLOGE("ADC[%d] has not Init.\n",AdcHdl);
        return HI_HAL_ENOINIT;
    }

    if (HAL_FD_INITIALIZATION_VAL == s_s32HALADCfd)
    {
        MLOGE("Dev has not been opened.\n");
        return HI_HAL_EINTER;
    }
    HI_MUTEX_LOCK(s_HALADCMutex);
    *ps32Value = ioctl(s_s32HALADCfd, LSADC_IOC_GET_CHNVAL, &AdcHdl);
    HI_MUTEX_UNLOCK(s_HALADCMutex);
    return HI_SUCCESS;
}

HI_S32 HAL_ADC_Deinit(HI_HANDLE AdcHdl)
{
    HI_MUTEX_LOCK(s_HALADCMutex);
    if (HI_TRUE != s_sAdcCtx.astAdcState[AdcHdl].bHALADCInitState)
    {
        MLOGD("ADC[%d] has not been Init.\n",AdcHdl);
        HI_MUTEX_UNLOCK(s_HALADCMutex);
        return HI_SUCCESS;
    }

    if (HAL_FD_INITIALIZATION_VAL != s_s32HALADCfd &&
        1 == s_sAdcCtx.u32InitHdlCnt)
    {
        /** Deinit ADC Device */
        close(s_s32HALADCfd);
    }
    s_sAdcCtx.u32InitHdlCnt--;
    HI_MUTEX_LOCK(s_HALADCMutex);
    return HI_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

