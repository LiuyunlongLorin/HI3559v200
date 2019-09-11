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
#include "shcmd.h"

extern CMD_ITEM g_shellcmd[];
extern CMD_ITEM g_shellcmd_end;

extern CMD_MOD_INFO_S m_stCmdInfo;

UINT32 osShellCmdHelp(UINT32 argc, CHAR **argv)
{
    UINT32      uwloop = 0;


    CMD_ITEM_S *pstCurCmdItem_s;

    if (argc > 0)
    {
        PRINTK("\nUsage: help\n");
        return OS_ERROR;
    }

    PRINTK("*******************shell commands:*************************\n");
    LOS_DL_LIST_FOR_EACH_ENTRY(pstCurCmdItem_s, &(m_stCmdInfo.stCmdList.list), CMD_ITEM_S, list)
    {
        if (uwloop % 8 == 0)
        {
            PRINTK("\n");
        }
        PRINTK("%-12s  ",pstCurCmdItem_s->pstCmd->pscCmdKey);

        uwloop++;
    }


    PRINTK("\n");

    return 0;
}

SHELLCMD_ENTRY(help_shellcmd, CMD_TYPE_EX, "help", 0, (CMD_CBK_FUNC)osShellCmdHelp); /*lint !e19*/
