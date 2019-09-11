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

#ifndef __SPI_H__
#define __SPI_H__
#include "stdint.h"
#include "los_typedef.h"

#define SPI_IOC_RD_MODE                 0x0801
#define SPI_IOC_RD_LSB_FIRST            0x0802
#define SPI_IOC_RD_BITS_PER_WORD        0x0803
#define SPI_IOC_RD_MAX_SPEED_HZ         0x0804
#define SPI_IOC_WR_MODE                 0x0805
#define SPI_IOC_WR_LSB_FIRST            0x0806
#define SPI_IOC_WR_BITS_PER_WORD        0x0807
#define SPI_IOC_WR_MAX_SPEED_HZ         0x0808
#define SPI_IOC_MESSAGE(N)              0x0809

#define SPI_CS_ACTIVE           0
#define SPI_CS_INACTIVE         1

#define SCR_MAX                 255
#define SCR_MIN                 0
#define CPSDVSR_MAX             254
#define CPSDVSR_MIN             2

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define SPI_CPHA        0x01                    /* clock phase */
#define SPI_CPOL        0x02                    /* clock polarity */
#define SPI_MODE_0      (0|0)                   /* (original MicroWire) */
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)
#define SPI_CS_HIGH     0x04                    /* chipselect active high? */
#define SPI_LSB_FIRST   0x08                    /* per-word bits-on-wire */
#define SPI_3WIRE       0x10                    /* SI/SO signals shared */
#define SPI_LOOP        0x20                    /* loopback mode */
#define SPI_NO_CS       0x40                    /* 1 dev/bus, no chipselect */
#define SPI_READY       0x80                    /* slave pulls low to pause */

struct spi_ioc_transfer {
    const char      *tx_buf;
    char            *rx_buf;
    unsigned        len;
    unsigned        cs_change;
    unsigned int    speed;
};

struct spi_platform_data {
    unsigned long   clk_rate;
    unsigned char   num_cs;
    unsigned int    fifo_size;
    int             (*cfg_cs)(uint16_t bus_num, uint8_t csn);
    int             (*hw_init_cfg)(uint16_t bus_num);
    int             (*hw_exit_cfg)(uint16_t bus_num);
};

struct spi_driver_data {
    struct spi_platform_data    *spd;
    char                        *regbase;
    unsigned long               max_speed;
    unsigned long               min_speed;
    unsigned int                cur_speed;
    unsigned int                cur_mode;
    unsigned int                cur_bits_per_word;
    unsigned int                bus_num;
    unsigned int                cur_cs;
    UINT32                      lock;
    
    int (*config)(struct spi_driver_data *sdd);
    void (*enable)(struct spi_driver_data *sdd);
    void (*disable)(struct spi_driver_data *sdd);
    int (*flush_fifo)(struct spi_driver_data *sdd);
    int (*txrx8)(struct spi_driver_data *sdd, struct spi_ioc_transfer *st);
    int (*txrx16)(struct spi_driver_data *sdd, struct spi_ioc_transfer *st);
};

struct spi_dev {
    unsigned char cs_index;
    struct spi_driver_data *cur_sdd;
};

/*****************************************************************************/
#define SPI_TRACE_LEVEL 2
#define spi_trace(level, msg...) do { \
    if ((level) >= SPI_TRACE_LEVEL) { \
        dprintf("%s:%d: ", __func__, __LINE__); \
        dprintf(msg); \
        dprintf("\n"); \
    } \
} while (0)

#define spi_assert(cond) do { \
    if (!(cond)) {\
        dprintf("Assert:spi:%s:%d\n", \
                __func__, \
                __LINE__); \
        BUG(); \
    } \
} while (0)

#define spi_error(s...) do { \
    dprintf("spi:%s:%d: ", __func__, __LINE__); \
    dprintf(s); \
    dprintf("\n"); \
} while (0)

#define spi_readl(addr) ({uint32_t reg = readl((void*)(addr)); \
        spi_trace(1, "readl(0x%p) = 0x%08X", (void*)(addr), reg); \
        reg; })

#define spi_writel(v, addr) do { \
    writel(v, (void*)(addr)); \
    spi_trace(1, "writel(0x%p) = 0x%08X",\
            (void*)(addr), (uint32_t)(v)); \
} while (0)

/*****************************************************************************/
#ifndef LOSCFG_DEBUG_VERSION
#define LOSCFG_DEBUG_VERSION
#endif
#ifdef LOSCFG_DEBUG_VERSION
    #define spi_err(x...) \
        do { \
                    dprintf("%s->%d: ", __func__, __LINE__); \
                    dprintf(x); \
                    dprintf("\n"); \
            } while (0)
    #undef SPI_DEBUG
#else
    #define spi_err(x...) do { } while (0)
    #undef SPI_DEBUG
#endif

#ifdef SPI_DEBUG
    #define spi_msg(x...) \
        do { \
                    dprintf("%s (line:%d) ", __func__, __LINE__); \
                    dprintf(x); \
            } while (0)
#else
    #define spi_msg(x...) do { } while (0)
#endif

int spi_dev_init(void);
int spi_dev_set(int host_no, int cs_no, struct spi_ioc_transfer * transfer);

#endif /* __SPI_H_ */

