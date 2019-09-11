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
#include "los_config.h"

#ifdef LOSCFG_DRIVERS_HIGMAC
#include "hisoc/net.h"

#include "util.h"
#include "higmac.h"
#include "ctrl.h"
#include "eth_phy_dev.h"
#include "linux/delay.h"
#include "los_base.h"

extern VOID LOS_Udelay(UINT32 usecs);
extern VOID LOS_Mdelay(UINT32 msecs);

void higmac_hw_mac_core_reset(struct higmac_netdev_local *ld)
{
    unsigned int v = 0;
    HISI_NET_LOCK_GET(&eth_crg_lock);

    v = GET_UINT32(PERI_CRG_GSF);
	v |= CRG_GMAC_IF_CLK_EN_BIT | CRG_GMAC_CLK_EN_BIT; /* enable clk */
    WRITE_UINT32(v, PERI_CRG_GSF);

    /* set reset bit */
    v = GET_UINT32(PERI_CRG_GSF);
    v |= CRG_GMAC_SOFT_RST_BIT;
    WRITE_UINT32(v, PERI_CRG_GSF);

    HISI_NET_LOCK_PUT(&eth_crg_lock);

    LOS_Udelay(50);

    HISI_NET_LOCK_GET(&eth_crg_lock);
    /* clear reset bit */
    v = GET_UINT32(PERI_CRG_GSF);
    v &= ~CRG_GMAC_SOFT_RST_BIT;
    WRITE_UINT32(v, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);
}

void higmac_set_macif(struct higmac_netdev_local *ld, int mode, int speed)
{
    unsigned long v;

    /* enable change: port_mode */
    higmac_writel_bits(ld, 1, MODE_CHANGE_EN, BIT_MODE_CHANGE_EN); /*lint !e506*/
    if (speed == PHY_SPEED_1000)
        v = GMAC_SPEED_1000;
    else if (speed == PHY_SPEED_100)
        v = GMAC_SPEED_100;
    else
        v = GMAC_SPEED_10;
    higmac_writel_bits(ld, v, PORT_MODE, BITS_PORT_MODE);  /*lint !e506*/
    /* disable change: port_mode */
    higmac_writel_bits(ld, 0, MODE_CHANGE_EN, BIT_MODE_CHANGE_EN); /*lint !e506*/

    HISI_NET_LOCK_GET(&eth_crg_lock);
    /* soft reset mac_if */
    v = GET_UINT32(PERI_CRG_GSF);
    v |= CRG_GMAC_IF_SOFT_RST_BIT;
    WRITE_UINT32(v, PERI_CRG_GSF);

    HISI_NET_LOCK_PUT(&eth_crg_lock);

    /* config mac_if */
    WRITE_UINT32(mode, PERI_CRG_MACIF);

    HISI_NET_LOCK_GET(&eth_crg_lock);
    /* undo reset */
    v = GET_UINT32(PERI_CRG_GSF);
    v &= ~CRG_GMAC_IF_SOFT_RST_BIT;
    WRITE_UINT32(v, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);
}

void higmac_hw_internal_fephy_reset(void)
{

}

void higmac_hw_external_phy_reset(void)
{
    unsigned int val;
#if defined LOSCFG_PLATFORM_HI3519V101 || defined LOSCFG_PLATFORM_HI3519 || defined LOSCFG_PLATFORM_HI3516A || defined LOSCFG_PLATFORM_HI3559AV100ES || defined LOSCFG_PLATFORM_HI3559AV100 || defined LOSCFG_PLATFORM_HI3556AV100
    HISI_NET_LOCK_GET(&eth_crg_lock);
    /* phy clk select */
    val = GET_UINT32(PERI_CRG_GSF);
#if defined LOSCFG_PLATFORM_HI3559AV100ES || defined LOSCFG_PLATFORM_HI3559AV100 || defined LOSCFG_PLATFORM_HI3556AV100
    val &= ~CRG_GMAC_IF_SOFT_RST_BIT;
#else
    val |= CRG_GMAC_IF_SOFT_RST_BIT;
#endif
    WRITE_UINT32(val, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);
#endif
#ifdef CONFIG_HIGMAC_RESET_PHY_BY_CRG
    HISI_NET_LOCK_GET(&eth_crg_lock);
    val = GET_UINT32(PERI_CRG_GSF);
    val &= ~CRG_PHY_RST_BIT;
    WRITE_UINT32(val, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);

    LOS_Msleep(10);

    HISI_NET_LOCK_GET(&eth_crg_lock);
    val = GET_UINT32(PERI_CRG_GSF);
    val |= CRG_PHY_RST_BIT;
    WRITE_UINT32(val, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);

    LOS_Msleep(50);

    HISI_NET_LOCK_GET(&eth_crg_lock);
    val = GET_UINT32(PERI_CRG_GSF);
    val &= ~CRG_PHY_RST_BIT;
    WRITE_UINT32(val, PERI_CRG_GSF);
    HISI_NET_LOCK_PUT(&eth_crg_lock);

    LOS_Msleep(50);
#endif

#ifdef CONFIG_HIGMAC_RESET_HELPER_EN
    /* set direction */
    val = GET_UINT32(IO_ADDRESS(CONFIG_HIGMAC_RESET_HELPER_GPIO_BASE) + 0x400);
    val |= (1 << CONFIG_HIGMAC_RESET_HELPER_GPIO_BIT);
    WRITE_UINT32(val, IO_ADDRESS(CONFIG_HIGMAC_RESET_HELPER_GPIO_BASE) + 0x400);

    /* Firstly, set to 1 regardless of the value of this pin */
    WRITE_UINT32(0xFF, IO_ADDRESS(CONFIG_HIGMAC_RESET_HELPER_GPIO_BASE)
                + (4 << CONFIG_HIGMAC_RESET_HELPER_GPIO_BIT));
    LOS_Msleep(20);

    /* Set to 0 to reset, then sleep 30ms */
    WRITE_UINT32(0x0, IO_ADDRESS(CONFIG_HIGMAC_RESET_HELPER_GPIO_BASE)
                + (4 << CONFIG_HIGMAC_RESET_HELPER_GPIO_BIT));
    LOS_Msleep(30);

    /* then, cancel reset, and should sleep 50ms */
    WRITE_UINT32(0xFF, IO_ADDRESS(CONFIG_HIGMAC_RESET_HELPER_GPIO_BASE)
                + (4 << CONFIG_HIGMAC_RESET_HELPER_GPIO_BIT));
    LOS_Msleep(50);
#endif
}
#endif
