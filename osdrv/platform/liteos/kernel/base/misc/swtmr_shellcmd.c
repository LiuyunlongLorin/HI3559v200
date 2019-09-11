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
#include "string.h"
#include "stdlib.h"
#include "los_swtmr.ph"

#include "shcmd.h"
#include "shell.h"

LITE_OS_SEC_DATA_MINOR CHAR g_hshellSwtmrMode[][8] =
{
    "Once",
    "Period",
    "NSD",
    "OPP",
};

LITE_OS_SEC_DATA_MINOR CHAR g_hshellSwtmrStatus[][12] =
{
    "UnUsed",
    "Created",
    "Ticking",
};

static VOID osPrintSwtmrMsg(SWTMR_CTRL_S *pstSwtmr)
{
    PRINTK("%-6d    "
                           "%-7s  "
                           "%-6s   "
                           "%-6d     "
                           "%-6d  "
                           "0x%-8x   "
                           "0x%-8x\n",
                           pstSwtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT,
                           g_hshellSwtmrStatus[pstSwtmr->ucState],
                           g_hshellSwtmrMode[pstSwtmr->ucMode],
                           pstSwtmr->uwInterval,
                           pstSwtmr->uwCount,
                           pstSwtmr->uwArg,
                           pstSwtmr->pfnHandler);
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdSwtmrInfoGet(INT32 argc, UINT8 **argv)
{
    SWTMR_CTRL_S *pstSwtmr = m_pstSwtmrCBArray;
    SWTMR_CTRL_S *pstSwtmr1 = m_pstSwtmrCBArray;
    UINT16 usIndex = 0;
    UINT32 uwRet = LOS_OK;
    UINT32 uwTimerID;
    UINT16 uwNum = 0;
    CHAR   *pscPtr = NULL;

    if (argc > 1)
    {
        PRINTK("\nUsage: swtmr [ID]\n");
        return OS_ERROR;
    }

    if (argc == 0)
    {
        uwTimerID = 0xffffffff;
    }
    else
    {
        uwTimerID = (UINT32)strtoul((char *)argv[0], &pscPtr, 0);
        if (pscPtr == (CHAR *)NULL || *pscPtr != 0  || (INT32)uwTimerID < 0)
        {
            PRINTK("\nswtmr ID can't access %s.\n", argv[0]);
            return 0;
        }
    }

    for (usIndex = 0; usIndex < LOSCFG_BASE_CORE_SWTMR_LIMIT; usIndex++, pstSwtmr1++)
    {
        if (pstSwtmr1->ucState == 0)
        {
        uwNum = uwNum + 1;
        }
    }

    if (uwNum == LOSCFG_BASE_CORE_SWTMR_LIMIT)
    {
        PRINTK("\r\nThere is no swtmr was created!\n");
        return uwRet;
    }


    if (0xffffffff == uwTimerID)
    {
        PRINTK("\r\nSwTmrID  State    Mode   Interval   Count    Arg       pfnHandlerAddr\n");
        PRINTK("-------  -------  ------- --------- ------- --------  -------- \n");

        for (usIndex = 0; usIndex < LOSCFG_BASE_CORE_SWTMR_LIMIT; usIndex++, pstSwtmr++)
        {
            if (pstSwtmr->ucState != 0)
            {
                osPrintSwtmrMsg(pstSwtmr);
            }
        }
        return uwRet;
    }
    else
    {
        for (usIndex = 0; usIndex < LOSCFG_BASE_CORE_SWTMR_LIMIT; usIndex++, pstSwtmr++)
        {
            if ((uwTimerID == (UINT32)(pstSwtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT)) &&
                                                (pstSwtmr->ucState != 0))
            {
                PRINTK("\r\nSwTmrID  State    Mode   Interval   Count    Arg       pfnHandlerAddr\n");
                PRINTK("-------  -------  ------- --------- ------- --------  -------- \n");


                osPrintSwtmrMsg(pstSwtmr);

                return uwRet;
            }
        }
    }
    PRINTK("\r\nThe SwTimerID is not exist.\n");
    return uwRet;
}

SHELLCMD_ENTRY(swtmr_shellcmd, CMD_TYPE_EX, "swtmr", 1, (CMD_CBK_FUNC)osShellCmdSwtmrInfoGet); /*lint !e19*/
#endif
