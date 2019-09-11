/*----------------------------------------------------------------------------
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
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
#ifdef LOSCFG_DRIVERS_HIETH_SF
#include "ctrl.h"

void hieth_hw_mac_core_reset()
{
    unsigned int v = 0;

    v = GET_UINT32(HIETH_CRG_IOBASE);

    v |= ETH_CORE_CLK_SELECT_54M;
    v |= (0x1 << 1); /* enable clk */
    WRITE_UINT32(v, HIETH_CRG_IOBASE);

    /* set reset bit */
    v = GET_UINT32(HIETH_CRG_IOBASE);
    v |= 0x1;
    WRITE_UINT32(v, HIETH_CRG_IOBASE);

    LOS_Udelay(50);

    /* clear reset bit */
    v = GET_UINT32(HIETH_CRG_IOBASE);
    v &= ~(0x1);
    WRITE_UINT32(v, HIETH_CRG_IOBASE);
}

void hieth_hw_external_phy_reset(void)
{
    unsigned int val;

    val = GET_UINT32(HIETH_CRG_IOBASE);
    val |= ETH_PHY_RESET;
    WRITE_UINT32(val, HIETH_CRG_IOBASE);

    LOS_Msleep(20);

    val = GET_UINT32(HIETH_CRG_IOBASE);
    val &= ~ETH_PHY_RESET;
    WRITE_UINT32(val, HIETH_CRG_IOBASE);

    LOS_Msleep(30);
}
#endif
