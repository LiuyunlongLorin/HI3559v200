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
#ifdef LOSCFG_SHELL
#include "shcmd.h"
#include "stdlib.h"

#define KEEP_OUT_AREA 0xc0000000

static UINT32 cmd_himd(UINT32 argc, CHAR ** argv)
{
    UINT32 uwDatalen;
    UINT32 *pAlignAddr = (UINT32 *)NULL;
    unsigned long uwtempAddr;
    unsigned long uwlen;
    UINT32 uwCnt = 0;

    if (argc == 0) {
        dprintf("himd <ADDRESS> [LENGTH]\n" );
        return OS_FAIL;
    }

    uwtempAddr = strtoul(argv[0], 0, 0);
    if(!uwtempAddr)
        dprintf("[info]input argv[0] value = %s, set it as 0x0\n",argv[0]);
    if (argc == 1) {
        uwDatalen = 128;
        if(uwtempAddr + 128 >= KEEP_OUT_AREA) {
            dprintf("error address! \n");
            return OS_FAIL;
        }
        pAlignAddr = (UINT32 *)TRUNCATE(uwtempAddr, 4);
        if ((pAlignAddr != (UINT32 *)uwtempAddr) || (pAlignAddr == NULL)) {
            dprintf("error address! \n");
            return OS_FAIL;
        }
        dprintf("\r\nThe address begin 0x%x,length:0x%x\n",
                pAlignAddr,uwDatalen);
        while (1) {
            if (uwDatalen) {
                if (0 == uwCnt % 4)
                    dprintf("\n%08x:", pAlignAddr);

                dprintf("%08x ", *pAlignAddr);
                pAlignAddr++;
                uwDatalen -= 4;
                uwCnt++;
                continue;
            }
            break;
        }
        dprintf("\nThe address end 0x%x\n", pAlignAddr);
        return LOS_OK;
    }

    uwlen = strtoul(argv[1],0,0);
    if(!uwtempAddr)
        dprintf("[info]input argv[1] value = %s, set it as 0x0\n",argv[1]);

    uwDatalen = ALIGN(uwlen, 4);

    pAlignAddr = (UINT32 *)TRUNCATE(uwtempAddr, 4);

    if ((pAlignAddr != (VOID*)argv[0])
            && (((AARCHPTR)pAlignAddr + uwDatalen - 1)
                < (uwtempAddr + uwlen - 1))) {
        uwDatalen = uwDatalen + 4;
    }

    if ((uwtempAddr + uwDatalen >= KEEP_OUT_AREA) || (pAlignAddr == NULL)) {
            dprintf("error address! \n");
            return OS_FAIL;
    }

    dprintf("\r\nThe address begin 0x%x,length:0x%x\n",
            pAlignAddr,uwDatalen);
    while (1) {
        if (uwDatalen) {
            if (0 == uwCnt % 4)
                dprintf("\n%08x:", pAlignAddr);

            dprintf("%08x ", *pAlignAddr);
            pAlignAddr++;
            uwDatalen -= 4;
            uwCnt++;
            continue;
        }
        break;
    }
    dprintf("\nThe address end 0x%x\n", pAlignAddr);
    return LOS_OK;
}

static UINT32 cmd_himm(UINT32 argc, CHAR ** argv)
{
    UINT32 wvalue = 0;
    unsigned long uwtempAddr = 0;
    UINT32 * pAlignAddr;
    UINT32 oldvalue = 0;

    if (argc == 2) {
        uwtempAddr = strtoul(argv[0],0,0);
        if(!uwtempAddr)
            dprintf("[info]input argv[0] value = %s, set it as 0x0\n",argv[0]);
        wvalue = strtoul(argv[1],0,0);
        if(!uwtempAddr)
            dprintf("[info]input argv[1] value = %s, set it as 0x0\n",argv[1]);

        pAlignAddr = (UINT32 *)TRUNCATE(uwtempAddr, 4);
        if ((pAlignAddr != (VOID *)uwtempAddr) || (pAlignAddr == NULL)) {
            dprintf("error address ! \n");
            return OS_FAIL;
        }
        oldvalue = *pAlignAddr;
        *pAlignAddr = wvalue;
        dprintf("\n%s: 0x%08lX -->  0x%08lx \n",
                argv[0], oldvalue , wvalue);
        dprintf("[END]\n");
        return LOS_OK;
    } else {
        dprintf("himm <ADDRESS> <VALUE>\n");
        return OS_FAIL;
    }

}

void shell_cmd_register(void)
{}

SHELLCMD_ENTRY(himd_shellcmd, CMD_TYPE_EX, "himd",0,(CMD_CBK_FUNC)cmd_himd);/*lint !e19 */
SHELLCMD_ENTRY(himm_shellcmd, CMD_TYPE_EX, "himm",0,(CMD_CBK_FUNC)cmd_himm);/*lint !e19 */
#endif /* LOSCFG_SHELL */
