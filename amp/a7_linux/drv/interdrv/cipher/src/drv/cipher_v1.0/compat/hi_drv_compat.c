/******************************************************************************

    Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name     : ext_aead.c
  Version       : Initial Draft
  Created       : 2017
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include "drv_osal_lib.h"
#include "drv_symc.h"
#include "drv_klad.h"

s32 klad_load_hard_key(u32 handle, u32 catype, u8 *key, u32 u32KeyLen)
{
    s32 ret = HI_FAILURE;

    ret = DRV_Cipher_KladLoadKey(handle, catype, HI_UNF_CIPHER_KLAD_TARGET_AES, key, u32KeyLen);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(DRV_Cipher_KladLoadKey, ret);
        return ret;
    }

    return HI_SUCCESS;
}

s32 klad_encrypt_key(u32 keysel, u32 target, u32 clear[4], u32 encrypt[4])
{
    s32 ret = HI_FAILURE;

    ret = DRV_Cipher_KladEncryptKey(keysel, target, clear, encrypt);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_PrintFuncErr(DRV_Cipher_KladEncryptKey, ret);
        return ret;
    }

    return HI_SUCCESS;
}

s32 hi_drv_compat_init(void)
{
    s32 ret = HI_FAILURE;

    ret = DRV_KLAD_Init();
    if (HI_SUCCESS != ret)
    {
         HI_LOG_PrintErrCode(ret);
         return ret;
    }

    return HI_SUCCESS;
}

s32 hi_drv_compat_deinit(void)
{
    DRV_KLAD_DeInit();

    return HI_SUCCESS;
}

