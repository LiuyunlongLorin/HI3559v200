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

#include "linux/device.h"
#include "string.h"
#include "stdio.h"
#include "fs/fs.h"
#include "hisoc/spi.h"
#include "spi.h"

struct spi_platform_driver_data
{
    struct spi_driver_data driver_data;
    struct spi_platform_data platform_data;
    int enable;

};


/*
 * spi_set_cs: confit spi cs
 */
int spi_set_cs(struct spi_driver_data *sdd,
        unsigned char csn, unsigned char flag)
{
    struct spi_platform_data *spd = sdd->spd;
    if (spd->cfg_cs(sdd->bus_num, csn))
        return -1;

    if (flag == SPI_CS_ACTIVE)
        sdd->enable(sdd);
    else
        sdd->disable(sdd);

    return 0;
}

/*
 * spi_txrx: send and receive data interface
 */
static int spi_txrx(struct spi_driver_data *sdd,
        struct spi_ioc_transfer *transfer)
{
    int ret = 0;
    if(transfer->speed)
        sdd->cur_speed = transfer->speed;
    else
        /* default speed is 2MHz */
        sdd->cur_speed = 2000000;

    sdd->config(sdd);
    spi_set_cs(sdd, sdd->cur_cs, SPI_CS_ACTIVE);

    ret = sdd->flush_fifo(sdd);
    if (ret)
        return ret;

    if (sdd->cur_bits_per_word <= 8)
        ret = sdd->txrx8(sdd, transfer);
    else
        ret = sdd->txrx16(sdd, transfer);

    if (ret || transfer->cs_change) {
        spi_set_cs(sdd, sdd->cur_cs, SPI_CS_INACTIVE);
    }

    return ret;
}

/*
 * spi_init_cfg: spi init configuration
 */
int spi_init_cfg(struct spi_driver_data *sdd)
{
    struct spi_platform_data *spd = sdd->spd;

    if (spd->hw_init_cfg(sdd->bus_num))
        return -1;

    if (sdd->config(sdd))
        return -1;

    return 0;
}

/*
 * spi_set_platdata: set platform data for driver
 */
int spi_set_platdata(struct spi_platform_data *spd, int id)
{

    if (spd == NULL) {
        spi_err("%s spd == NULL\n", __func__);
        return -1;
    }
    spd->clk_rate = SPI_CLK_RATE;
    spd->cfg_cs = spi_cfg_cs;
    spd->hw_init_cfg = spi_hw_init_cfg;
    spd->hw_exit_cfg = spi_hw_exit_cfg;

    return 0;
}
/*
 * spi_transfer interface
 */
size_t spi_transfer(struct spi_driver_data *sdd, struct spi_ioc_transfer *wt)
{
    if(spi_txrx(sdd, wt) == 0x0)
        return wt->len;
    else
        return 0;
}

/*
 * spi_set: set spi device interface
 */
int spi_dev_set(int host_no, int cs_index, struct spi_ioc_transfer * transfer)
{
    int ret;
    struct spi_driver_data *sdd = NULL;
    struct spi_platform_data *spd = NULL;
    struct device *dev = NULL;
    extern struct bus_type platform_bus_type;

    LOS_DL_LIST_FOR_EACH_ENTRY(dev, &(platform_bus_type.device_list), struct device, bus_node)
    {
        if (!strcmp(dev->name,"spidev"))
        {
            sdd =  (struct spi_driver_data *)dev_get_drvdata(dev);
            spd = (struct spi_platform_data *)dev_get_platdata(dev);
            if((sdd->bus_num == host_no)&&(spd->num_cs > 0))
                break;
            else
                sdd = NULL;

        }
    }

    if(sdd != NULL)
    {
        sdd->cur_cs = cs_index;
        ret = spi_transfer(sdd, transfer);
        return ret;
    }
    else
    {
        return -1;
    }
}
