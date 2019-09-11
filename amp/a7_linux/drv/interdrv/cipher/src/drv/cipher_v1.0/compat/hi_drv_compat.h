#ifndef _HI_DRV_CPMPAT_H_
#define _HI_DRV_CPMPAT_H_

s32 hi_drv_compat_init(void);
s32 hi_drv_compat_deinit(void);
s32 klad_load_hard_key(u32 handle, u32 catype, u8 *key, u32 u32KeyLen);
s32 klad_encrypt_key(u32 keysel, u32 target, u32 clear[4], u32 encrypt[4]);

#endif

