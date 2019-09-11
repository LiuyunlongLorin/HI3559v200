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

#include "los_tick.ph"
#include "los_hwi.h"
#include "hisoc/timer.h"
#include "hisoc/cpu.h"
#include "hisoc/sys_ctrl.h"
#include "hisoc/spi.h"
#ifdef LOSCFG_DRIVERS_USB
#include "hisoc/usb3.h"
#endif
#ifdef LOSCFG_DRIVERS_SPI
#include "spi.h"
#endif
#include "los_memory.h"
#include "los_hw_tick_minor.h"
#include <linux/device.h>
#include "linux/platform_device.h"
#include "linux/module.h"
#ifdef LOSCFG_DRIVERS_MMC
#include "hisoc/mmc.h"
#endif
#include "hisoc/uart.h"
#include "uart.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif /* LOSCFG_LIB_LIBC */
#ifdef LOSCFG_FS_VFS
#include "vfs_config.h"
#endif
#ifdef  LOSCFG_DRIVERS_HIETH_SF
#include  "hieth.h"
#endif
#ifdef LOSCFG_DRIVERS_I2C
#include "i2c-hibvt.h"
#include "linux/i2c.h"
#endif
#include "hisoc/mmu_config.h"

#ifdef LOSCFG_PLATFORM_HISI_AMP
#include "irq_map.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern void v7_dma_clean_range(AARCHPTR start, AARCHPTR end);
extern void v7_dma_inv_range(AARCHPTR start, AARCHPTR end);
void dma_cache_clean(AARCHPTR start, AARCHPTR end)
{
    v7_dma_clean_range(start, end);

}
void dma_cache_inv(AARCHPTR start, AARCHPTR end)
{
    v7_dma_inv_range(start, end);
}

unsigned long g_usb_mem_addr_start __attribute__ ((section(".data"))) = 0;
unsigned long g_usb_mem_size __attribute__ ((section(".data")))= 0x80000; //recommend 256K nonCache for usb
extern unsigned int g_uwFatSectorsPerBlock;
extern unsigned int g_uwFatBlockNums;
#ifdef LOSCFG_KERNEL_RUNSTOP
    extern INT32 g_swResumeFromImg;
#endif
void hal_mmu_init(void) {
    UINT32 ttb_base = FIRST_PAGE_DESCRIPTOR_ADDR + 0x0;
    UINT32 uwReg = 0;

    // Set the TTB register
    __asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    // Set the Domain Access Control Register
    uwReg = ACCESS_TYPE_MANAGER(0)    |
        ACCESS_TYPE_CLIENT(1)  |
        ACCESS_TYPE_NO_ACCESS(2)  |
        ACCESS_TYPE_NO_ACCESS(3)  |
        ACCESS_TYPE_NO_ACCESS(4)  |
        ACCESS_TYPE_NO_ACCESS(5)  |
        ACCESS_TYPE_NO_ACCESS(6)  |
        ACCESS_TYPE_NO_ACCESS(7)  |
        ACCESS_TYPE_NO_ACCESS(8)  |
        ACCESS_TYPE_NO_ACCESS(9)  |
        ACCESS_TYPE_NO_ACCESS(10) |
        ACCESS_TYPE_NO_ACCESS(11) |
        ACCESS_TYPE_NO_ACCESS(12) |
        ACCESS_TYPE_NO_ACCESS(13) |
        ACCESS_TYPE_NO_ACCESS(14) |
        ACCESS_TYPE_NO_ACCESS(15);
    __asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(uwReg));

#ifdef LOSCFG_KERNEL_RUNSTOP
    if (g_swResumeFromImg == 1) return;
#endif

    // First clear all TT entries - ie Set them to Faulting
    memset((void *)ttb_base, 0, MMU_16K); /*lint !e418*/
    // set all mem 4G as uncacheable & rw first
   X_MMU_SECTION(0, 0, (MMU_4G/MMU_1M), UNCACHEABLE, UNBUFFERABLE, ACCESS_RW, 0);/*lint !e572*/
    /**************************************************************************************************
    *    set table as your config
    *    1: LITEOS_CACHE_ADDR ~ LITEOS_CACHE_ADDR + LITEOS_CACHE_LENGTH ---- set as section(1M) and cacheable & rw
    ****************************************************************************************************/
    X_MMU_SECTION((LITEOS_CACHE_ADDR/MMU_1M), (SYS_MEM_BASE/MMU_1M), (LITEOS_CACHE_LENGTH/MMU_1M), CACHEABLE, BUFFERABLE, ACCESS_RW, 0);/*lint !e572*/
}/*lint !e438  !e550*/
__attribute__((weak)) VOID board_config(VOID)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + TEXT_OFFSET + SYS_MEM_SIZE_DEFAULT;
    g_usb_mem_addr_start = g_sys_mem_addr_end; //config start memory before startup usb_init
#ifdef LOSCFG_EXC_INTERACTION
    g_exc_interact_mem_size = EXC_INTERACT_MEM_SIZE;
#endif
#ifdef LOSCFG_FS_FAT
    g_uwFatSectorsPerBlock = CONFIG_FS_FAT_SECTOR_PER_BLOCK;
    g_uwFatBlockNums       = CONFIG_FS_FAT_BLOCK_NUMS;
#endif
}

#define READ_TIMER    (*(volatile unsigned int *) \
                (TIMER_TICK_REG_BASE + TIMER_VALUE))

VOID los_bss_init(AARCHPTR bss_start, AARCHPTR bss_end)
{
    memset((void *)bss_start, 0, bss_end - bss_start);
}

VOID platform_hal_clock_initialize(UINT32 period)
{
    UINT32 temp;
    /*
     * enable time0, timer1 here,
     * but only time0 is used for system clock.
     */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= TIMER_TICK_ENABLE | TIMER_TIME_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /* disable first */
    WRITE_UINT32(0x0, TIMER_TICK_REG_BASE + TIMER_CONTROL);

    /* set init value as period */
    WRITE_UINT32(period, TIMER_TICK_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     */
    temp = (1 << 6) | (1 << 5) | (1 << 1);
    WRITE_UINT32(temp, TIMER_TICK_REG_BASE + TIMER_CONTROL);

    /* init the timestamp and lastdec value */
    reset_timer_masked();

    /* disable timer1 */
    WRITE_UINT32(0x0, TIMER_TIME_REG_BASE + TIMER_CONTROL);
    /* set init value as period */
    WRITE_UINT32(0xffffffff, TIMER_TIME_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */
    temp = (1 << 7) | (1 << 6) | (1 << 1);
    WRITE_UINT32(temp, TIMER_TIME_REG_BASE + TIMER_CONTROL);

    (VOID)LOS_HwiCreate(NUM_HAL_INTERRUPT_TIMER, 0xa0, 0, osTickHandler, 0);

}

VOID platform_gic_dist_init(unsigned int gic_nr, VOID *base, unsigned int irq_start)
{
    unsigned int i;
    UINT32 cpumask =   0x01010101;   //cpu0
    unsigned int max_irq;
    AARCHPTR addr_base = (AARCHPTR)base;
#ifdef LOSCFG_PLATFORM_HISI_AMP
	unsigned int *irq_dist = (unsigned int *)irq_map;
#endif
    WRITE_UINT32(0x0, addr_base + ARM_GIC_DIST_CTRL);

    /*
     * find out how many interrupts are supported.
     */
    max_irq = OS_HWI_MAX_NUM; //256 interrupts at most

    /*
     * Set all global interrupts to be level triggered, active low.
     */
    for (i = 32; i < max_irq; i += 16) {
        WRITE_UINT32(0x0, addr_base + ARM_GIC_DIST_CONFIG + i * 4 / 16);
    }

    for (i = 32; i < max_irq; i += 4) {
#ifdef LOSCFG_PLATFORM_HISI_AMP
        /*
         * distribute all global interrupts to A53UP or A53MP-0
         */
        WRITE_UINT32(*irq_dist, addr_base + ARM_GIC_DIST_TARGET + i);
        irq_dist++;
#else
        /*
         * Set all global interrupts to this CPU only.
         */
        WRITE_UINT32(cpumask, addr_base + ARM_GIC_DIST_TARGET + i);
#endif
    }
    /*
     * Set priority on all interrupts.
     */
    for (i = 0; i < max_irq; i += 4) {
        WRITE_UINT32(0xa0a0a0a0, addr_base + ARM_GIC_DIST_PRI + i * 4 / 4);
    }

    /*
     * Disable all interrupts.
     */
    for (i = 0; i < max_irq; i += 32) {
        WRITE_UINT32(0xffffffff, addr_base + ARM_GIC_DIST_ENABLE_CLEAR + i * 4 / 32);
    }

    WRITE_UINT32(1, addr_base + ARM_GIC_DIST_CTRL);
#ifdef LOSCFG_PLATFORM_HISI_AMP
#define GIC_DIST_INIT_FLAG_BASE SYS_CTRL_REG_BASE
#define GIC_DIST_INIT_FLAG_OFFSET 0x0130
#define GIC_DIST_INIT_FLAG 0x47444946
	WRITE_UINT32(GIC_DIST_INIT_FLAG, GIC_DIST_INIT_FLAG_BASE + GIC_DIST_INIT_FLAG_OFFSET);
#endif
}
/**************************************************/
/*         platform devices                       */
/**************************************************/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef ARRAY_AND_SIZE
#define ARRAY_AND_SIZE(x)   (x), ARRAY_SIZE(x)
#endif
/**************************************************/
/*        MTD_Flash Device                     */
/**************************************************/
#if defined (LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100) || defined(LOSCFG_DRIVERS_MTD_NAND_HIFMC100) || defined(LOSCFG_DRIVERS_MTD_NAND_HIFMC100_PARALLEL)
static struct resource hifmc100_resources[] = {
    {
        .start  = FMC_REG_BASE,
        .end    = FMC_REG_BASE + 0xdc,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = FMC_MEM_BASE,
        .end    = FMC_MEM_BASE + 8,
        .flags  = IORESOURCE_MEM,
    },
};
#endif
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100
struct platform_device device_hifmc100 = {
    .name       = "hifmc100",
    .id     = -1,
    .resource   = hifmc100_resources,
    .num_resources  = ARRAY_SIZE(hifmc100_resources),
};
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND_HIFMC100
struct platform_device device_hifmc100_nand = {
    .name       = "hifmc100_nand",
    .id     = -1,
    .resource   = hifmc100_resources,
    .num_resources  = ARRAY_SIZE(hifmc100_resources),
};
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND_HIFMC100_PARALLEL
struct platform_device device_hifmc100_parallel = {
    .name       = "hifmc100_parallel",
    .id     = -1,
    .resource   = hifmc100_resources,
    .num_resources  = ARRAY_SIZE(hifmc100_resources),
};
#endif
/**************************************************/
/*        EMMC Device                      */
/**************************************************/
#if USE_MMC0
static struct resource sdmmc0_resources[] = {
    {
        .start  = SDIO0_REG_BASE,
        .end    = SDIO0_REG_BASE + 0x118,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_SDIO,
        .end    = NUM_HAL_INTERRUPT_SDIO,
        .flags  = IORESOURCE_IRQ,
    },
};
struct platform_device device_sdmmc0 = {
    .name       = "sdmci",
    .id     = 0,
    .resource   = sdmmc0_resources,
    .num_resources  = ARRAY_SIZE(sdmmc0_resources),
};
#endif
#if USE_MMC1
static struct resource sdmmc1_resources[] = {
    {
        .start  = SDIO1_REG_BASE,
        .end    = SDIO1_REG_BASE + 0x118,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_SDIO1,
        .end    = NUM_HAL_INTERRUPT_SDIO1,
        .flags  = IORESOURCE_IRQ,
    },
};
struct platform_device device_sdmmc1 = {
    .name       = "sdmci",
    .id     = 1,
    .resource   = sdmmc1_resources,
    .num_resources  = ARRAY_SIZE(sdmmc1_resources),
};
#endif
#if USE_MMC2
static struct resource sdmmc2_resources[] = {
    {
        .start  = EMMC_REG_BASE,
        .end    = EMMC_REG_BASE + 0x118,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_EMMC,
        .end    = NUM_HAL_INTERRUPT_EMMC,
        .flags  = IORESOURCE_IRQ,
    },
};
struct platform_device device_sdmmc2 = {
    .name       = "sdmci",
    .id     = 2,
    .resource   = sdmmc2_resources,
    .num_resources  = ARRAY_SIZE(sdmmc2_resources),
};
#endif
/**************************************************************************************************
*    spi device
****************************************************************************************************/
#ifdef LOSCFG_DRIVERS_SPI
#if SPI0_ENABLE
static struct spi_driver_data hi_spi0_driver = {/*lint !e121 -esym(528, hi_spi0_driver)*/
    .regbase = (void *)SPI0_REG_BASE,
    .cur_speed = 2000000,
    .cur_mode = SPI_MODE_3 | SPI_LSB_FIRST,
    .cur_bits_per_word = 8,
    .bus_num = 0,
};
static struct spi_platform_data hi_spi0_platform = {/*lint !e10 !e129*/
    .num_cs = 1,
    .fifo_size = 256,
};
static struct platform_device hi_spi0_device = {/*lint !e10 !e129*/
    .name       = "spidev",
    .id = 0,
    .dev = {
        .platform_data  = &hi_spi0_platform,
        .driver_data = &hi_spi0_driver,
    },
};
#endif
#if SPI1_ENABLE
static struct spi_driver_data hi_spi1_driver = {/*lint !e10 !e129*/
    .regbase = (void *)SPI1_REG_BASE,
    .cur_speed = 2000000,
    .cur_mode = SPI_MODE_3 | SPI_LSB_FIRST,
    .cur_bits_per_word = 8,
    .bus_num = 1,
};
static struct spi_platform_data hi_spi1_platform = {/*lint !e10 !e129*/
    .num_cs = 1,
    .fifo_size = 256,
};
static struct platform_device hi_spi1_device = {/*lint !e10 !e129*/
    .name       = "spidev",
    .id = 1,
    .dev = {
        .platform_data  = &hi_spi1_platform,
        .driver_data = &hi_spi1_driver,
    },
};
#endif
#if SPI2_ENABLE
static struct spi_driver_data hi_spi2_driver = {/*lint !e10 !e129*/
    .regbase = (void *)SPI2_REG_BASE,
    .cur_speed = 2000000,
    .cur_mode = SPI_MODE_3 | SPI_LSB_FIRST,
    .cur_bits_per_word = 8,
    .bus_num = 2,
};
static struct spi_platform_data hi_spi2_platform = {/*lint !e10 !e129*/
    .num_cs = 2,
    .fifo_size = 256,
};
static struct platform_device hi_spi2_device = {/*lint !e10 !e129*/
    .name       = "spidev",
    .id = 2,
    .dev = {
        .platform_data  = &hi_spi2_platform,
        .driver_data = &hi_spi2_driver,
    },
};
#endif
#if SPI3_ENABLE
static struct spi_driver_data hi_spi3_driver = {/*lint !e10 !e129*/
    .regbase = (void *)SPI3_REG_BASE,
    .cur_speed = 2000000,
    .cur_mode = SPI_MODE_3 | SPI_LSB_FIRST,
    .cur_bits_per_word = 8,
    .bus_num = 3,
};
static struct spi_platform_data hi_spi3_platform = {/*lint !e10 !e129*/
    .num_cs = 2,
    .fifo_size = 256,
};
static struct platform_device hi_spi3_device = {/*lint !e10 !e129*/
    .name       = "spidev",
    .id = 3,
    .dev = {
        .platform_data  = &hi_spi3_platform,
        .driver_data = &hi_spi3_driver,
    },
};
#endif
#if SPI4_ENABLE
static struct spi_driver_data hi_spi4_driver = {/*lint !e10 !e129*/
    .regbase = (void *)SPI4_REG_BASE,
    .cur_speed = 2000000,
    .cur_mode = SPI_MODE_3 | SPI_LSB_FIRST,
    .cur_bits_per_word = 8,
    .bus_num = 4,
};
static struct spi_platform_data hi_spi4_platform = {/*lint !e10 !e129*/
    .num_cs = 4,
    .fifo_size = 256,
};
static struct platform_device hi_spi4_device = {/*lint !e10 !e129*/
    .name       = "spidev",
    .id = 4,
    .dev = {
        .platform_data  = &hi_spi4_platform,
        .driver_data = &hi_spi4_driver,
    },
};
#endif

#endif
/**************************************************/
/*        i2c Device                      */
/**************************************************/
#ifdef LOSCFG_DRIVERS_I2C
#define I2C_NUM 5
#define ENABLE_I2C0
#define ENABLE_I2C1
#define ENABLE_I2C2
#define ENABLE_I2C3
#define ENABLE_I2C4
#define CLK_LIMIT_DEFAULT 400000

void /*__init */ i2c_register_platform_device(struct platform_device *dev, void *data)
{
    int ret;

    dev->dev.platform_data = data;

    ret = platform_device_register(dev);
    if (ret)
        dprintf("unable to register hisilicon device %s: %d\n", dev->name, ret);
}

void /*__init*/ hibvt_set_i2c_info(struct platform_device *device, struct i2c_hibvt_platform_data *info)
{
    i2c_register_platform_device(device, info);
}

/* declare the i2c resource */
#ifdef ENABLE_I2C0
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C0 NUM_HAL_INTERRUPT_I2C0
#else
#define IRQ_I2C0 0
#endif
static struct resource hibvt_i2c0_resources[] = {
    {
        .start  = I2C0_REG_BASE,
        .end    = I2C0_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C0,
        .end    = IRQ_I2C0,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c0 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 0,
    .resource   = hibvt_i2c0_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c0_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c0 = {
    .freq = CLK_LIMIT_DEFAULT,
};

#endif

#ifdef ENABLE_I2C1
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C1 NUM_HAL_INTERRUPT_I2C1
#else
#define IRQ_I2C1 0
#endif
static struct resource hibvt_i2c1_resources[] = {
    {
        .start  = I2C1_REG_BASE,
        .end    = I2C1_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C1,
        .end    = IRQ_I2C1,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c1 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 1,
    .resource   = hibvt_i2c1_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c1_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c1 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C2
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C2 NUM_HAL_INTERRUPT_I2C2
#else
#define IRQ_I2C2 0
#endif
static struct resource hibvt_i2c2_resources[] = {
    {
        .start  = I2C2_REG_BASE,
        .end    = I2C2_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C2,
        .end    = IRQ_I2C2,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c2 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 2,
    .resource   = hibvt_i2c2_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c2_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c2 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C3
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C3 NUM_HAL_INTERRUPT_I2C3
#else
#define IRQ_I2C3 0
#endif
static struct resource hibvt_i2c3_resources[] = {
    {
        .start  = I2C3_REG_BASE,
        .end    = I2C3_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C3,
        .end    = IRQ_I2C3,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c3 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 3,
    .resource   = hibvt_i2c3_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c3_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c3 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C4
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C4 NUM_HAL_INTERRUPT_I2C4
#else
#define IRQ_I2C4 0
#endif
static struct resource hibvt_i2c4_resources[] = {
    {
        .start  = I2C4_REG_BASE,
        .end    = I2C4_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C4,
        .end    = IRQ_I2C4,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c4 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 4,
    .resource   = hibvt_i2c4_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c4_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c4 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C5
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C5 NUM_HAL_INTERRUPT_I2C5
#else
#define IRQ_I2C5 0
#endif
static struct resource hibvt_i2c5_resources[] = {
    {
        .start  = I2C5_REG_BASE,
        .end    = I2C5_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C5,
        .end    = IRQ_I2C5,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c5 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 5,
    .resource   = hibvt_i2c5_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c5_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c5 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C6
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C6 NUM_HAL_INTERRUPT_I2C6
#else
#define IRQ_I2C6 0
#endif
static struct resource hibvt_i2c6_resources[] = {
    {
        .start  = I2C6_REG_BASE,
        .end    = I2C6_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C6,
        .end    = IRQ_I2C6,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c6 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 6,
    .resource   = hibvt_i2c6_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c6_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c6 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#ifdef ENABLE_I2C7
#ifdef HIBVT_I2C_INTERRUPT_MODE
#define IRQ_I2C7 NUM_HAL_INTERRUPT_I2C7
#else
#define IRQ_I2C7 0
#endif
static struct resource hibvt_i2c7_resources[] = {
    {
        .start  = I2C7_REG_BASE,
        .end    = I2C7_REG_BASE + 0xd0,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = IRQ_I2C7,
        .end    = IRQ_I2C7,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device hibvt_device_i2c7 = {
    .name       = I2C_DRIVER_NAME,
    .id         = 7,
    .resource   = hibvt_i2c7_resources,
    .num_resources  = ARRAY_SIZE(hibvt_i2c7_resources),
};

struct i2c_hibvt_platform_data hibvt_pdata_i2c7 = {
    .freq = CLK_LIMIT_DEFAULT,
};
#endif

#endif

/**************************************************/
/*        UART Device                      */
/**************************************************/
#if LOSCFG_DRIVERS_UART
#if UART0_ENABLE
static struct uart_driver_data uart0_pl011_driver = {/*lint !e121 !e129 !e10 -esym(528, uart0_pl011_driver)*/
    .num             = 0,
    .baudrate        = 115200,
    .attr.fifo_rx_en = 1,
    .attr.fifo_tx_en = 1,
    .flags           = UART_FLG_RD_BLOCK,
};

static struct resource uart0_pl011_resources[] = {/*lint !e10 !e129*/
    {
        .start  = UART0_REG_BASE,
        .end    = UART0_REG_BASE + 0x48,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_UART0,
        .end    = NUM_HAL_INTERRUPT_UART0,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device uart0_pl011_device = {/*lint !e10 !e129*/
    .name       = "uart-pl011",
    .id = -1,
    .dev = {
        .driver_data = &uart0_pl011_driver,
    },
    .resource   = uart0_pl011_resources,
    .num_resources  = ARRAY_SIZE(uart0_pl011_resources),
};
#endif
#if UART1_ENABLE
static struct uart_driver_data uart1_pl011_driver = {/*lint !e10 !e129*/
    .num             = 1,
    .baudrate        = 9600,
    .attr.fifo_rx_en = 1,
    .attr.fifo_tx_en = 1,
    .flags           = UART_FLG_RD_BLOCK,
};
static struct resource uart1_pl011_resources[] = {/*lint !e10 !e129*/
    {
        .start  = UART1_REG_BASE,
        .end    = UART1_REG_BASE + 0x48,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_UART1,
        .end    = NUM_HAL_INTERRUPT_UART1,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device uart1_pl011_device = {/*lint !e10 !e129*/
    .name       = "uart-pl011",
    .id = -1,
    .dev = {
        .driver_data = &uart1_pl011_driver,
    },
    .resource   = uart1_pl011_resources,
    .num_resources  = ARRAY_SIZE(uart1_pl011_resources),
};
#endif
#if UART2_ENABLE
static struct uart_driver_data uart2_pl011_driver = {/*lint !e10 !e129*/
    .num             = 2,
    .baudrate        = 115200,
    .attr.fifo_rx_en = 1,
    .attr.fifo_tx_en = 1,
    .flags           = UART_FLG_RD_BLOCK,
};
static struct resource uart2_pl011_resources[] = {/*lint !e10 !e129*/
    {
        .start  = UART2_REG_BASE,
        .end    = UART2_REG_BASE + 0x48,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_UART2,
        .end    = NUM_HAL_INTERRUPT_UART2,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device uart2_pl011_device = {/*lint !e10 !e129*/
    .name       = "uart-pl011",
    .id = -1,
    .dev = {
        .driver_data = &uart2_pl011_driver,
    },
    .resource   = uart2_pl011_resources,
    .num_resources  = ARRAY_SIZE(uart2_pl011_resources),
};
#endif
#if UART3_ENABLE
static struct uart_driver_data uart3_pl011_driver = {/*lint !e10 !e129*/
    .num             = 3,
    .baudrate        = 9600,
    .attr.fifo_rx_en = 1,
    .attr.fifo_tx_en = 1,
    .flags           = UART_FLG_RD_BLOCK,
};
static struct resource uart3_pl011_resources[] = {/*lint !e10 !e129*/
    {
        .start  = UART3_REG_BASE,
        .end    = UART3_REG_BASE + 0x48,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_UART3,
        .end    = NUM_HAL_INTERRUPT_UART3,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device uart3_pl011_device = {/*lint !e10 !e129*/
    .name       = "uart-pl011",
    .id = -1,
    .dev = {
        .driver_data = &uart3_pl011_driver,
    },
    .resource   = uart3_pl011_resources,
    .num_resources  = ARRAY_SIZE(uart3_pl011_resources),
};
#endif
#endif

/**************************************************/
/*        USB Device                      */
/**************************************************/
#ifdef LOSCFG_DRIVERS_USB_HOST_XHCI
static struct resource hi_xhci_resources[] = {/*lint !e10 !e129*/
    {
        .start  = CONFIG_HIUSB_XHCI_IOBASE,
        .end    = CONFIG_HIUSB_XHCI_IOBASE + CONFIG_HIUSB_XHCI_IOSIZE,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_USB_XHCI,
        .end    = NUM_HAL_INTERRUPT_USB_XHCI,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device hixhci_device = {/*lint !e10 !e129*/
    .name       = "hi_xhci",
    .id = -1,
    .resource   = hi_xhci_resources,
    .num_resources  = ARRAY_SIZE(hi_xhci_resources),
};
#endif
#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
static struct resource hi_udc3_resources[] = {/*lint !e10 !e129*/
    {
        .start  = NUM_HAL_INTERRUPT_USB_DEV,
        .end    = NUM_HAL_INTERRUPT_USB_DEV,
        .flags  = IORESOURCE_IRQ,
    },
};
static struct platform_device hiudc3_device = {/*lint !e10 !e129*/
    .name       = "hi_udc3",
    .id = -1,
    .resource   = hi_udc3_resources,
    .num_resources  = ARRAY_SIZE(hi_udc3_resources),
};
#endif
int machine_init(void)
{
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100
    (void)platform_device_register(&device_hifmc100);
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND_HIFMC100
    (void)platform_device_register(&device_hifmc100_nand);
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND_HIFMC100_PARALLEL
    (void)platform_device_register(&device_hifmc100_parallel);
#endif
#ifdef LOSCFG_DRIVERS_MMC
#if USE_MMC0
    (void)platform_device_register(&device_sdmmc0);
#endif
#if USE_MMC1
    (void)platform_device_register(&device_sdmmc1);
#endif
#if USE_MMC2
    (void)platform_device_register(&device_sdmmc2);
#endif
#endif
#ifdef LOSCFG_DRIVERS_SPI
#if SPI0_ENABLE
 (void)platform_device_register(&hi_spi0_device);
#endif
#if SPI1_ENABLE
 (void)platform_device_register(&hi_spi1_device);
#endif
#if SPI2_ENABLE
 (void)platform_device_register(&hi_spi2_device);
#endif
#if SPI3_ENABLE
 (void)platform_device_register(&hi_spi3_device);
#endif
#if SPI4_ENABLE
 (void)platform_device_register(&hi_spi4_device);
#endif
#endif
#ifdef LOSCFG_DRIVERS_I2C
#ifdef ENABLE_I2C0
 hibvt_set_i2c_info(&hibvt_device_i2c0, &hibvt_pdata_i2c0);
#endif
#ifdef ENABLE_I2C1
 hibvt_set_i2c_info(&hibvt_device_i2c1, &hibvt_pdata_i2c1);
#endif
#ifdef ENABLE_I2C2
 hibvt_set_i2c_info(&hibvt_device_i2c2, &hibvt_pdata_i2c2);
#endif
#ifdef ENABLE_I2C3
 hibvt_set_i2c_info(&hibvt_device_i2c3, &hibvt_pdata_i2c3);
#endif
#ifdef ENABLE_I2C4
 hibvt_set_i2c_info(&hibvt_device_i2c4, &hibvt_pdata_i2c4);
#endif
#ifdef ENABLE_I2C5
 hibvt_set_i2c_info(&hibvt_device_i2c5, &hibvt_pdata_i2c5);
#endif
#ifdef ENABLE_I2C6
 hibvt_set_i2c_info(&hibvt_device_i2c6, &hibvt_pdata_i2c6);
#endif
#ifdef ENABLE_I2C7
 hibvt_set_i2c_info(&hibvt_device_i2c7, &hibvt_pdata_i2c7);
#endif
#endif

#if LOSCFG_DRIVERS_UART
#if UART0_ENABLE
    (void)platform_device_register(&uart0_pl011_device);
#endif
#if UART1_ENABLE
    (void)platform_device_register(&uart1_pl011_device);
#endif
#if UART2_ENABLE
    (void)platform_device_register(&uart2_pl011_device);
#endif
#if UART3_ENABLE
    (void)platform_device_register(&uart3_pl011_device);
#endif
#endif
#ifdef LOSCFG_DRIVERS_USB_HOST_XHCI
    (void)platform_device_register(&hixhci_device);
#endif
#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
    (void)platform_device_register(&hiudc3_device);
#endif
    return 0;
}
arch_initcall(machine_init);

/**************************************************/
/*        hieth_sf                       */
/**************************************************/
#ifdef LOSCFG_DRIVERS_HIETH_SF

struct resource hieth_resources[] = {
    {
        .start  = ETH_REG_BASE,
        .end    = ETH_REG_BASE + 0x2000 - 1,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = NUM_HAL_INTERRUPT_ETH,
        .end    = NUM_HAL_INTERRUPT_ETH,
        .flags  = IORESOURCE_IRQ,
    }
};

struct hieth_platform_data stHiethPlatformData[] = {
    {
        .stNetdevLocal = {
            .port = UP_PORT,

            .depth = { .hw_xmitq = CONFIG_HIETH_HWQ_XMIT_DEPTH },
            .q_size = HIETH_HWQ_TXQ_SIZE,

            .mdio_frqdiv = HIETH_MDIO_FRQDIV,
            .tx_busy = 0,

            .phy_mode = PHY_INTERFACE_MODE_RMII,
        }
    },
};

struct platform_device hieth_platform_device = {
    .name = HIETH_DRIVER_NAME,
    .dev = {
        .platform_data  = stHiethPlatformData,
    },
    .id   = 0,
    .num_resources = ARRAY_SIZE(hieth_resources),
    .resource = hieth_resources,
};

int hieth_machine_init(void)
{
    return platform_device_register(&hieth_platform_device);
}

arch_initcall(hieth_machine_init);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

