/******************************************************************************

  Copyright (C), 2011-2014, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : spacc_intf.c
  Version       : Initial Draft
  Author        : Hisilicon hisecurity team
  Created       :
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/
#include "hi_types.h"
#include "drv_osal_lib.h"
#include "hal_efuse.h"
#include "hal_otp.h"
#include "drv_cipher_kapi.h"

#define KLAD_REG_BASE_ADDR                      g_u32KladBase
#define KLAD_REG_KLAD_CTRL                      (KLAD_REG_BASE_ADDR + 0x00)
#define KLAD_REG_DAT_IN                         (KLAD_REG_BASE_ADDR + 0x10)
#define KLAD_REG_ENC_OUT                        (KLAD_REG_BASE_ADDR + 0x20)

#define CIPHER_WAIT_IDEL_TIMES         1000

static u8 *g_u32KladBase = HI_NULL;
extern u8 *g_u32EfuseOtpRegBase;

HI_S32 HAL_Cipher_KladConfig(HI_U32 chnId, HI_U32 u32OptId, HI_UNF_CIPHER_KLAD_TARGET_E enTarget, HI_BOOL bIsDecrypt)
{
    HI_S32 ret = HI_SUCCESS;
    HI_U32 u32Ctrl;

    /* Load efuse or OTP key to KLAD */
    ret = HAL_Efuse_OTP_LoadCipherKey(chnId, u32OptId);
    if(ret != HI_SUCCESS)
    {
        return ret;
    }

    u32Ctrl  = chnId << 16;
    u32Ctrl |= enTarget << 2;   /* cipher klad */
    u32Ctrl |= bIsDecrypt << 1; /* decrypt     */
    u32Ctrl |= 0x00;            /* start       */

    (HI_VOID)HAL_CIPHER_WriteReg(KLAD_REG_KLAD_CTRL, u32Ctrl);

    return HI_SUCCESS;
}

HI_VOID HAL_Cipher_StartKlad(HI_U32 BlockNum)
{
    HI_U32 u32Ctrl = 0;
    HI_U32 u32High = 0;

    u32High = (BlockNum == 1 ? 1 : 0);

    /* start */
    (HI_VOID)HAL_CIPHER_ReadReg(KLAD_REG_KLAD_CTRL, &u32Ctrl);
    u32Ctrl &= ~(0x01 << 4);
    u32Ctrl |= u32High << 4;
    u32Ctrl |= 0x01;  /* start */
    (HI_VOID)HAL_CIPHER_WriteReg(KLAD_REG_KLAD_CTRL, u32Ctrl);
}

HI_VOID HAL_Cipher_SetKladData(HI_U32 *pu32DataIn)
{
    HI_U32 i = 0;

    for(i=0; i<4; i++)
    {
        (HI_VOID)HAL_CIPHER_WriteReg(KLAD_REG_DAT_IN + i * 4, pu32DataIn[i]);
    }
}

HI_VOID HAL_Cipher_GetKladData(HI_U32 *pu32DataOut)
{
    HI_U32 i = 0;

    for(i=0; i<4; i++)
    {
        (HI_VOID)HAL_CIPHER_ReadReg(KLAD_REG_ENC_OUT+ i * 4, &pu32DataOut[i]);
    }
}

HI_S32 HAL_Cipher_WaitKladDone(HI_VOID)
{
    HI_U32 u32TryCount = 0;
    HI_U32 u32Ctrl;

    do {

        HAL_CIPHER_ReadReg(KLAD_REG_KLAD_CTRL, &u32Ctrl);
        if ((u32Ctrl & 0x01) == 0x00)
        {
            return HI_SUCCESS;
        }
        u32TryCount++;
    } while (u32TryCount < CIPHER_WAIT_IDEL_TIMES);

    HI_LOG_ERROR("Klad time out!\n");

    return HI_FAILURE;

}

HI_VOID HAL_KLAD_Init(HI_VOID)
{
    HI_U32 CrgValue;
    HI_U32 *u32SysAddr;

    u32SysAddr = crypto_ioremap_nocache(KLAD_CRG_ADDR_PHY, 0x100);
    if (u32SysAddr == HI_NULL)
    {
        HI_LOG_ERROR("ERROR: u32SysAddr ioremap with nocache failed!!\n");
        return ;
    }

    HAL_CIPHER_ReadReg(u32SysAddr, &CrgValue);
    CrgValue |= KLAD_CRG_RESET_BIT;   /* reset */
    CrgValue |= KLAD_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    HAL_CIPHER_WriteReg(u32SysAddr, CrgValue);

    /* clock select and cancel reset 0x30100*/
    CrgValue &= (~KLAD_CRG_RESET_BIT); /* cancel reset */
    CrgValue |= KLAD_CRG_CLOCK_BIT;    /* set the bit 0, clock opened */
    HAL_CIPHER_WriteReg(u32SysAddr, CrgValue);

    crypto_iounmap(u32SysAddr, 0x100);
}

HI_S32 DRV_KLAD_Init(HI_VOID)
{
    HI_S32 ret =HI_FAILURE;

    g_u32KladBase = crypto_ioremap_nocache(KLAD_REG_BASE_ADDR_PHY, 0x100);
    if (g_u32KladBase == HI_NULL)
    {
        HI_LOG_ERROR("ERROR: osal_ioremap_nocache for KLAD failed!!\n");
        return HI_FAILURE;
    }

    ret = HAL_Efuse_OTP_Init();
    if (HI_SUCCESS != ret)
    {
        crypto_iounmap(g_u32KladBase, 0x100);
        return ret;
    }

    HAL_KLAD_Init();

    return HI_SUCCESS;
}

HI_VOID DRV_KLAD_DeInit(HI_VOID)
{
    if (g_u32KladBase != HI_NULL)
    {
        crypto_iounmap(g_u32KladBase, 0x100);
        g_u32KladBase = HI_NULL;
    }

    if (g_u32EfuseOtpRegBase != HI_NULL)
    {
        crypto_iounmap(g_u32EfuseOtpRegBase, 0x100);
        g_u32EfuseOtpRegBase = HI_NULL;
    }

    return ;
}

HI_VOID DRV_Cipher_Invbuf(HI_U8 *buf, HI_U32 u32len)
{
    HI_U32 i;
    HI_U8 ch;

    for(i=0; i<u32len/2; i++)
    {
        ch = buf[i];
        buf[i] = buf[u32len - i - 1];
        buf[u32len - i - 1] = ch;
    }
}

HI_S32 DRV_Cipher_KladLoadKey(HI_U32 chnId,
                              HI_UNF_CIPHER_CA_TYPE_E enRootKey,
                              HI_UNF_CIPHER_KLAD_TARGET_E enTarget,
                              HI_U8 *pu8DataIn,
                              HI_U32 u32KeyLen)
{
    HI_S32 ret = HI_FAILURE;
    HI_U32 i = 0;
    HI_U32 u32Key[4] = {0};
    HI_U32 u32OptId = 0;;

    if((enRootKey < HI_UNF_CIPHER_KEY_SRC_KLAD_1) ||
        (enRootKey > HI_UNF_CIPHER_KEY_SRC_KLAD_3))
    {
        HI_LOG_ERROR("Error: Invalid Root Key src 0x%x!\n", enRootKey);
        return HI_FAILURE;
    }
    if(((u32KeyLen % 16 ) != 0) || (u32KeyLen == 0))
    {
        HI_LOG_ERROR("Error: Invalid key len 0x%x!\n", u32KeyLen);
        return HI_FAILURE;
    }
    if(HI_NULL == pu8DataIn)
    {
        HI_LOG_ERROR("Error: point for input data is null!\n");
        return HI_FAILURE;
    }

    u32OptId = enRootKey - HI_UNF_CIPHER_KEY_SRC_KLAD_1 + 1;

    ret = HAL_Cipher_KladConfig(chnId, u32OptId, enTarget, HI_TRUE);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("Error: cipher klad config failed!\n");
        return HI_FAILURE;
    }

    for(i=0; i<u32KeyLen/16; i++)
    {
        crypto_memcpy(u32Key, sizeof(u32Key), pu8DataIn+i*16, 16);
        HAL_Cipher_SetKladData(u32Key);
        HAL_Cipher_StartKlad(i);
        ret = HAL_Cipher_WaitKladDone();
        if (HI_SUCCESS != ret)
        {
            HI_LOG_ERROR("Error: cipher klad wait done failed!\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;

}

HI_S32 DRV_Cipher_KladEncryptKey(HI_UNF_CIPHER_CA_TYPE_E enRootKey,
                                 HI_UNF_CIPHER_KLAD_TARGET_E target,
                                 HI_U32 u32CleanKey[4],
                                 HI_U32 u32EncryptKey[4])
{
    HI_S32 ret;
    HI_U32 u32OptId;

    if((enRootKey < HI_UNF_CIPHER_KEY_SRC_KLAD_1) ||
        (enRootKey >= HI_UNF_CIPHER_KEY_SRC_BUTT))
    {
        HI_LOG_ERROR("Error: Invalid Root Key src 0x%x!\n", enRootKey);
        return HI_FAILURE;
    }
    if ((HI_NULL == u32CleanKey) || (HI_NULL == u32EncryptKey))
    {
        HI_LOG_ERROR("Clean key or encrypt key is null.\n");
        return HI_ERR_CIPHER_INVALID_POINT;
    }

    u32OptId = enRootKey - HI_UNF_CIPHER_KEY_SRC_KLAD_1 + 1;

    ret = HAL_Cipher_KladConfig(0, u32OptId, HI_UNF_CIPHER_KLAD_TARGET_AES, HI_FALSE);
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("Error: cipher klad config failed!\n");
        return HI_FAILURE;
    }

    if (HI_UNF_CIPHER_KLAD_TARGET_RSA == target)
    {
        DRV_Cipher_Invbuf((HI_U8*)u32CleanKey, 16);
    }

    HAL_Cipher_SetKladData(u32CleanKey);
    HAL_Cipher_StartKlad(0);
    ret = HAL_Cipher_WaitKladDone();
    if (HI_SUCCESS != ret)
    {
        HI_LOG_ERROR("Error: cipher klad wait done failed!\n");
        return HI_FAILURE;
    }
    HAL_Cipher_GetKladData(u32EncryptKey);

    return HI_SUCCESS;
}

