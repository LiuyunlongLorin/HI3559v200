/*****************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : kapi_trng.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include "drv_osal_lib.h"
#include "cryp_trng.h"

/** @}*/  /** <!-- ==== Structure Definition end ====*/


/******************************* API Code *****************************/
/** \addtogroup      trng */
/** @{*/  /** <!-- [kapi]*/

s32 kapi_trng_get_random(u32 *randnum, u32 timeout)
{
    s32 ret = HI_FAILURE;

    HI_LOG_FuncEnter();

    HI_LOG_CHECK_PARAM(HI_NULL == randnum);

    ret = cryp_trng_get_random(randnum, timeout);
    if (HI_SUCCESS != ret)
    {
        return ret;
    }

    HI_LOG_FuncExit();
    return HI_SUCCESS;
}

/** @}*/  /** <!-- ==== Structure Definition end ====*/
