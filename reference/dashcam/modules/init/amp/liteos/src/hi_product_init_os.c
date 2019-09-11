/**
 * @file      hi_product_init_os.c
 * @brief     os init source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#include "sys_config.h"
#include "board.h"
#include "los_typedef.h"

#include "stdlib.h"
#include "stdio.h"

#include "console.h"
#include "hisoc/uart.h"
#include "uart.h"

#include "los_cppsupport.h"
#include "los_ld_elflib.h"
#include "hi_appcomm.h"
#include "hi_system.h"
#include "hi_product_init_os.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern AARCHPTR g_sys_mem_addr_end;
VOID board_config(VOID)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
#ifdef LOSCFG_DRIVERS_USB
    extern unsigned long g_usb_mem_addr_start;
    extern unsigned long g_usb_mem_size;
    g_usb_mem_addr_start = g_sys_mem_addr_end;
    g_usb_mem_size = 0x20000; /**<recommend 128K nonCache for usb */
#endif
#ifdef LOSCFG_DRIVERS_EMMC
    size_t part0_start_sector = 16   * (0x100000/512);
    size_t part0_count_sector = 1024 * (0x100000/512);
    size_t part1_start_sector = 16   * (0x100000/512) + part0_count_sector;
    size_t part1_count_sector = 1024 * (0x100000/512);
    extern struct disk_divide_info emmc;
    add_mmc_partition(&emmc, part0_start_sector, part0_count_sector);
    add_mmc_partition(&emmc, part1_start_sector, part1_count_sector);
#endif
}

HI_S32 HI_PDT_INIT_OS_PreInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    extern int _ipcm_vdd_init(void);
    MLOGD("-->ipcm init ...\n");
    _ipcm_vdd_init();

    extern int sharefs_client_init(const char *path);
    MLOGD("-->sharefs init ...\n");
    sharefs_client_init(HI_SHAREFS_ROOT_PATH);

    extern int virt_tty_dev_init(void);
    MLOGD("-->virt tty init ...\n");
    virt_tty_dev_init();

    /* uart init */
    if (uart_dev_init() != 0)
    {
        MLOGE("uart_dev_init failed");
    }
    if (virtual_serial_init(TTY_DEVICE) != 0)
    {
        MLOGE("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0)
    {
        MLOGE("system_console_init failed\n");
    }

    MLOGD("-->random dev init ...\n");
    extern int ran_dev_register(void);
    ran_dev_register();
    MLOGD("-->mem dev init ...\n");
    extern int mem_dev_register(void);
    mem_dev_register();

    MLOGD("-->porc fs init ...\n");
    extern void proc_fs_init(void);
    proc_fs_init();

#ifdef LOSCFG_DRIVERS_GPIO
    MLOGD("-->gpio init ...\n");
    extern int gpio_dev_init(void);
    gpio_dev_init();
#endif

    return s32Ret;
}

static HI_VOID PDT_GetCompileTime(HI_SYSTEM_TM_S* pstDateTime)
{
    HI_CHAR mon[4] = "Jan";
    sscanf(__DATE__, "%3s %d %d", mon, &pstDateTime->s32mday, &pstDateTime->s32year);
    HI_CHAR const mon_name[12][3] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    HI_S32 i=0;
    for (i=0;i<12;i++)
    {
        if(0==strncmp(mon,mon_name[i],3))
        {
            pstDateTime->s32mon = i+1;
        }
    }
    sscanf(__TIME__, "%d:%d:%d", &pstDateTime->s32hour,&pstDateTime->s32min, &pstDateTime->s32sec);
}

static HI_VOID PDT_SetSysInitTime(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_SYSTEM_TM_S stRTCTime;
    memset(&stRTCTime,0,sizeof(HI_SYSTEM_TM_S));
    HI_SYSTEM_GetRTCDateTime(&stRTCTime);
    MLOGI("RTC   DateTime: %04d/%02d/%02d-%02d:%02d:%02d\n",stRTCTime.s32year,stRTCTime.s32mon,stRTCTime.s32mday,
        stRTCTime.s32hour,stRTCTime.s32min,stRTCTime.s32sec);

    HI_SYSTEM_TM_S stCompileTime;
    memset(&stCompileTime,0,sizeof(HI_SYSTEM_TM_S));
    PDT_GetCompileTime(&stCompileTime);
    MLOGI("Build DateTime: %04d/%02d/%02d-%02d:%02d:%02d\n",stCompileTime.s32year,stCompileTime.s32mon,stCompileTime.s32mday,
        stCompileTime.s32hour,stCompileTime.s32min,stCompileTime.s32sec);

    if(stRTCTime.s32year < stCompileTime.s32year)
    {
        s32Ret = HI_SYSTEM_SetDateTime(&stCompileTime);
    }
    else
    {
        s32Ret = HI_SYSTEM_SetDateTime(&stRTCTime);
    }
    if(s32Ret)
    {
        MLOGE("HI_SYSTEM_SetDateTime failed: %x\n", s32Ret);
    }
}

HI_S32 HI_PDT_INIT_OS_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    extern char __init_array_start__, __init_array_end__;
    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__,NO_SCATTER);
    LOS_PathAdd(HI_SHAREFS_ROOT_PATH);
    PDT_SetSysInitTime();
    return s32Ret;
}

HI_S32 HI_PDT_INIT_OS_PostInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

