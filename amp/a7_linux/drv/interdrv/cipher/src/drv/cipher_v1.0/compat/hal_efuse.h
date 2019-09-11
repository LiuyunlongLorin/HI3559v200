#ifndef _HAL_EFUSE_H_
#define _HAL_EFUSE_H_

#include "hi_types.h"

#define HAL_CIPHER_ReadReg(addr, result)    (*(result) = *(volatile unsigned int *)(addr))
#define HAL_CIPHER_WriteReg(addr,result)    (*(volatile unsigned int *)(addr) = (result))

#define HAL_SET_BIT(src, bit)               ((src) |= (1<<bit))
#define HAL_CLEAR_BIT(src,bit)              ((src) &= ~(1<<bit))

HI_S32 HAL_Efuse_WriteKey(HI_U32 * p_key, HI_U32 opt_id);
HI_S32 HAL_Efuse_OTP_LoadCipherKey(HI_U32 chn_id, HI_U32 opt_id);
HI_S32 HAL_Efuse_LoadHashKey(HI_U32 opt_id);
HI_S32 HAL_Efuse_OTP_Init(HI_VOID);

#endif

