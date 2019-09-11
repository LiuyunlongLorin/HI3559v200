#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* sample.bin should be loaded at SYS_MEM_BASE+TEXT_OFFSET */

#define DDR_MEM_ADDR            0x80000000

#define DDR_MEM_SIZE            0x20000000ull

#define SYS_MEM_BASE            0x80200000

#define TEXT_OFFSET              0x00000000

#define SYS_MEM_SIZE_DEFAULT    0x01e00000

#define MMZ_MEM_BASE            0x88000000

#define MMZ_MEM_LEN             0x15000000

#define EXC_INTERACT_MEM_SIZE   0x100000

//#define LOSCFG_REGION_PROTECT_BASE_0
//#define LOSCFG_REGION_PROTECT_SIZE_0

//#define LOSCFG_REGION_PROTECT_BASE_1
//#define LOSCFG_REGION_PROTECT_SIZE_1

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
