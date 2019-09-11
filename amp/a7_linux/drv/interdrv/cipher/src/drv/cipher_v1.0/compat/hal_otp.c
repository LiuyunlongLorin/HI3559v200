#include "hi_types.h"
#include "drv_osal_lib.h"
#include "hal_otp.h"

#ifdef OTP_SUPPORT

HI_U8 *g_u32EfuseOtpRegBase = HI_NULL;

/* OTP init */
HI_S32 HAL_Efuse_OTP_Init(HI_VOID)
{
    HI_U32 CrgValue = 0;
    HI_U32 *u32SysAddr = HI_NULL;

    u32SysAddr = crypto_ioremap_nocache(REG_SYS_OTP_CLK_ADDR_PHY, 0x100);
    if (u32SysAddr == HI_NULL)
    {
        HI_LOG_ERROR("ERROR: u32SysAddr ioremap with nocache failed!!\n");
        return HI_FAILURE;
    }

    HAL_CIPHER_ReadReg(u32SysAddr, &CrgValue);
#if defined(CHIP_TYPE_hi3559av100)
    CrgValue |= OTP_CRG_RESET_BIT;   /* reset */
    CrgValue |= OTP_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    HAL_CIPHER_WriteReg(u32SysAddr, CrgValue);

    /* clock select and cancel reset 0x30100*/
    CrgValue &= (~OTP_CRG_RESET_BIT); /* cancel reset */
#endif
    CrgValue |= OTP_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    HAL_CIPHER_WriteReg(u32SysAddr, CrgValue);

    crypto_iounmap(u32SysAddr, 0x100);

    g_u32EfuseOtpRegBase = crypto_ioremap_nocache(OTP_REG_BASE_ADDR_PHY, 0x100);
    if (g_u32EfuseOtpRegBase == HI_NULL)
    {
        HI_LOG_ERROR("ERROR: osal_ioremap_nocache for OTP failed!!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HAL_OTP_WaitFree(HI_VOID)
{
    HI_U32 u32TimeOutCnt = 0;
    HI_U32 u32RegValue = 0;

    while(1)
    {
        HAL_CIPHER_ReadReg(OTP_USER_CTRL_STA, &u32RegValue);
        if((u32RegValue&0x1)==0)//bit0:otp_op_busy 0:idle, 1:busy
        {
            return HI_SUCCESS;
        }

        u32TimeOutCnt++;
        if(u32TimeOutCnt >= 10000)
        {
            HI_LOG_ERROR("OTP_WaitFree TimeOut!\n");
            break;
        }
    }
    return HI_FAILURE;
}

HI_S32 HAL_OTP_SetMode(OTP_USER_WORK_MODE_E enOtpMode)
{
    HI_U32 u32RegValue = enOtpMode;

    if(enOtpMode >= OTP_UNKOOWN_MODE)
    {
        HI_LOG_ERROR("Mode Unknown!\n");
        return  HI_FAILURE;
    }

    (HI_VOID)HAL_CIPHER_WriteReg(OTP_USER_WORK_MODE, u32RegValue);
    return HI_SUCCESS;
}

HI_VOID HAL_OTP_OP_Start(HI_VOID)
{
    HI_U32 u32RegValue = 0x1acce551;
    (HI_VOID)HAL_CIPHER_WriteReg(OTP_USER_OP_START, u32RegValue);
}

HI_S32 HAL_OTP_Wait_OP_done(HI_VOID)
{
    HI_U32 u32TimeOutCnt = 0;
    HI_U32 u32RegValue = 0;

    while(1)
    {
        HAL_CIPHER_ReadReg(OTP_USER_CTRL_STA, &u32RegValue);
        if(u32RegValue & 0x2)
        {
            return HI_SUCCESS;
        }

        u32TimeOutCnt++;
        if(u32TimeOutCnt >= 10000)
        {
            HI_LOG_ERROR("OTP_Wait_OP_done TimeOut!\n");
            break;
        }
    }
    return HI_FAILURE;
}

HI_VOID HAL_CHOOSE_OTP_key(OTP_USER_KEY_INDEX_E enWhichKey)
{
    HI_U32 RegValue = 0;

    RegValue = enWhichKey;
    (HI_VOID)HAL_CIPHER_WriteReg(OTP_USER_KEY_INDEX, RegValue);
}

HI_S32 Is_Locked(OTP_USER_KEY_INDEX_E enWhichKey,HI_U32 lock_sta)
{
    HI_U32 status = lock_sta;

    if(enWhichKey>OTP_USER_KEY3)
    {
        HI_LOG_ERROR("Unsupport Key!\n");
        return HI_FAILURE;
    }

    if((status>>(enWhichKey+5))&0x1)
    {
        HI_LOG_ERROR("Key%d was locked!\n",enWhichKey);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/* set otp key to klad */
HI_S32 HAL_Efuse_OTP_LoadCipherKey(HI_U32 chn_id, HI_U32 opt_id)
{
    if(opt_id > OTP_USER_KEY3)
    {
        opt_id = OTP_USER_KEY0;
    }

    if(HI_FAILURE == HAL_OTP_WaitFree())
    {
        return HI_FAILURE;
    }
    HAL_CHOOSE_OTP_key(opt_id);

    if(HAL_OTP_SetMode(OTP_LOCK_CIPHER_KEY_MODE))
    {
        return HI_FAILURE;
    }

    HAL_OTP_OP_Start();

    if(HI_FAILURE == HAL_OTP_Wait_OP_done())
    {
        return HI_FAILURE;
    }

    return  HI_SUCCESS;
}
#endif

