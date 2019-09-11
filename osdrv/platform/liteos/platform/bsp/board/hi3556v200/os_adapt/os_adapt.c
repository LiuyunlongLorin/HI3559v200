/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
#include "los_base.h"
#include "los_event.h"
#include "los_config.h"
#ifdef LOSCFG_KERNEL_RUNSTOP
#include "los_runstop.h"
#endif
#ifdef LOSCFG_KERNEL_SCATTER
#include "los_scatter.h"
#endif
#ifdef LOSCFG_KERNEL_CPPSUPPORT
#include "los_cppsupport.h"
#endif
#ifdef LOSCFG_LIB_LIBC
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/time.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "time.h"
#include "securec.h"
#endif
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#endif
#if defined(LOSCFG_FS_YAFFS) || defined(LOSCFG_FS_JFFS)
#include "mtd_partition.h"
#endif
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
#include "linux/mtd/mtd.h"
#endif
#ifdef   LOSCFG_FS_PROC
#include "proc_fs.h"
#endif
#ifdef LOSCFG_DRIVERS_UART
#include "console.h"
#include "hisoc/uart.h"
#include "uart.h"
#endif
#ifdef LOSCFG_NET_LWIP_SACK
#include "lwip/tcpip.h"
#include "arch/perf.h"
#include "los_mac.h"
#ifdef LOSCFG_DRIVERS_HIGMAC
#include "eth_drv.h"
#endif
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND
#include "nand.h"
#endif
#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif
#include "hisoc/mmu_config.h"

#ifdef LOSCFG_KERNEL_CPUP
#include "los_cpup.h"
#endif

#ifdef LOSCFG_COMPAT_LINUX
#include "linux/module.h"
#include "linux/mtd/mtd_list.h"
#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
struct mtd_info *mtd;
extern int spinor_init(void);
#endif

extern int hifmc100_wakeup_lockresume(void);
void idle_wakeup_callback(void)
{
    if (hifmc100_wakeup_lockresume() != 0) {
        PRINT_ERR("spi resume failed!\n");
    }
}

void wakeup_callback(void)
{
#ifdef LOSCFG_KERNEL_CPUP
    LOS_CpupReset();
#endif

    hal_interrupt_unmask(83);
}

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
void spiflash_erase(size_t start, size_t size) {
    struct erase_info erase_test;
    memset(&erase_test, 0, sizeof(struct erase_info));
    erase_test.mtd = mtd;
    erase_test.callback = NULL; /*lint !e64*/
    erase_test.fail_addr = (uint64_t)MTD_FAIL_ADDR_UNKNOWN;
    erase_test.addr = start;
    erase_test.len = size;
    erase_test.time = 1;
    erase_test.retries = 1;
    erase_test.dev = 0;
    erase_test.cell = 0;
    erase_test.priv = 0;
    erase_test.state = 0;
    erase_test.next = (struct erase_info *)NULL;
    erase_test.scrub = 0;
    (void)mtd->erase(mtd, &erase_test);
}

int spiflash_write(void *memaddr, size_t start, size_t size) {
    size_t retlen;
    return mtd->write(mtd, start, size, &retlen, (const char *)memaddr);
}

int spiflash_read(void *memaddr, size_t start, size_t size) {
    size_t retlen;
    return mtd->read(mtd, start, size, &retlen, (const char *)memaddr);
}
#endif

#define NAND_ERASE_ALIGN_SIZE   (128 * 1024)
#define NAND_READ_ALIGN_SIZE    (2 * 1024)
#define NAND_WRITE_ALIGN_SIZE   (2 * 1024)

#define NOR_ERASE_ALIGN_SIZE    (64 * 1024)
#define NOR_READ_ALIGN_SIZE     (1)
#define NOR_WRITE_ALIGN_SIZE    (1)

#define EMMC_ERASE_ALIGN_SIZE   (512)
#define EMMC_READ_ALIGN_SIZE    (512)
#define EMMC_WRITE_ALIGN_SIZE   (512)


#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
int flash_read(void *memaddr, size_t start, size_t size)
{
    return spiflash_read(memaddr, start, size);
}

int flash_write(void *memaddr, size_t start, size_t size)
{
    spiflash_erase(start, size);
    return spiflash_write(memaddr, start, size);
}
#endif

#ifdef LOSCFG_NET_LWIP_SACK
int secure_func_register(void)
{
    int ret;
    STlwIPSecFuncSsp stlwIPSspCbk= {0};
    stlwIPSspCbk.pfMemset_s = memset_s;
    stlwIPSspCbk.pfMemcpy_s = memcpy_s;
    stlwIPSspCbk.pfStrNCpy_s = strncpy_s;
    stlwIPSspCbk.pfStrNCat_s = strncat_s;
    stlwIPSspCbk.pfStrCat_s = strcat_s;
    stlwIPSspCbk.pfMemMove_s = memmove_s;
    stlwIPSspCbk.pfSnprintf_s = snprintf_s;
    stlwIPSspCbk.pfRand = rand;
    ret = lwIPRegSecSspCbk(&stlwIPSspCbk);
    if (ret != 0)
    {
        PRINT_ERR("\n***lwIPRegSecSspCbk Failed***\n");
        return -1;
    }

    PRINTK("\nCalling lwIPRegSecSspCbk\n");
    return ret;
}
#endif

#ifdef LOSCFG_NET_LWIP_SACK
#ifdef LOSCFG_DRIVERS_HIGMAC
struct netif *pnetif;
extern void get_defaultNetif(struct netif **pnetif);

extern int ethnet_higmac_init(void);

void ipc_gmac_init(void)
{
    static unsigned int overtime = 0;
    PRINTK("Ethernet start.");

    (void)ethnet_higmac_init();
    get_defaultNetif(&pnetif);

    (void)netifapi_netif_set_up(pnetif);
    do {
        LOS_Msleep(60);
        overtime++;
        if (overtime > 100){
            PRINTK("netif_is_link_up overtime!\n");
            break;
        }
    } while(netif_is_link_up(pnetif) == 0);
}
#endif /* LOSCFG_DRIVERS_HIGMAC */
#endif /* LOSCFG_NET_LWIP_SACK */
extern void code_protect(void);
__attribute__((weak))  void app_init(void)
{
#ifdef LOSCFG_DRIVERS_UART
    if (uart_dev_init() != 0)
    {
        PRINT_ERR("uart_dev_init failed");
    }
#endif

#ifdef LOSCFG_KERNEL_RUNSTOP
    extern UINT32 g_uwWowImgSize;
    RUNSTOP_PARAM_S stRunstopParam;
#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    (void)spinor_init();
    mtd = get_mtd("spinor");
#endif

#if defined (LOSCFG_KERNEL_RUNSTOP) && defined (LOSCFG_DRIVERS_MTD_SPI_NOR)
    memset(&stRunstopParam, 0, sizeof(RUNSTOP_PARAM_S));

    stRunstopParam.pfFlashReadFunc          = flash_read;
    stRunstopParam.pfFlashWriteFunc         = flash_write;
    stRunstopParam.pfIdleWakeupCallback     = idle_wakeup_callback;
    stRunstopParam.pfImageDoneCallback      = NULL;
    stRunstopParam.pfWakeupCallback         = wakeup_callback;
    stRunstopParam.uwFlashEraseAlignSize    = NOR_ERASE_ALIGN_SIZE;
    stRunstopParam.uwFlashWriteAlignSize    = NOR_WRITE_ALIGN_SIZE;
    stRunstopParam.uwFlashReadAlignSize     = NOR_READ_ALIGN_SIZE;
    stRunstopParam.uwImageFlashAddr         = 0x100000;
    stRunstopParam.uwWowFlashAddr           = 0x800000;

    LOS_MakeImage(&stRunstopParam);
#endif

#ifndef MAKE_WOW_IMAGE

#ifdef LOSCFG_KERNEL_RUNSTOP
    PRINTK("Image length 0x%x\n", g_uwWowImgSize);
#endif

#if defined (LOSCFG_KERNEL_SCATTER) && defined (LOSCFG_DRIVERS_MTD_SPI_NOR)
    PRINTK("LOS_ScatterLoad...\n");
    LOS_ScatterLoad(0x100000, flash_read, NOR_READ_ALIGN_SIZE);
#endif

#ifndef MAKE_SCATTER_IMAGE
#ifdef LOSCFG_DRIVERS_UART
    if (virtual_serial_init(TTY_DEVICE) != 0)
    {
        PRINT_ERR("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0)
    {
        PRINT_ERR("system_console_init failed\n");
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    (VOID)secure_func_register();
    tcpip_init(NULL, NULL);
    PRINTK("###tcpip_init done#####\n");
#endif

#ifdef LOSCFG_DRIVERS_HIGMAC
    ipc_gmac_init();
#endif

#endif /* MAKE_WOW_IMAGE */
#endif /* MAKE_SCATTER_IMAGE */
}
