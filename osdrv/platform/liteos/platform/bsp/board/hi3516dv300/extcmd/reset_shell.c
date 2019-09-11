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
#include "reset_shell.h"
#include "signal.h"
#include "asm/io.h"
#include "stdlib.h"
#include "hisoc/sys_ctrl.h"
#include "unistd.h"
#include "shcmd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

Hook_Func_Node *g_hook_func_node = (Hook_Func_Node*)NULL;

void cmd_reset(void)
{
    osReHookFuncHandle();
    sleep(1);/*lint !e534*/
    /* Any value to this reg will reset the cpu */
    writel(0xffffffff, (SYS_CTRL_REG_BASE + REG_SC_SYSRES));
}

UINT32 osReHookFuncAdd(STORAGE_HOOK_FUNC handler, VOID *param)
{
    Hook_Func_Node *pstFuncNode;

    pstFuncNode = (Hook_Func_Node*)malloc(sizeof(Hook_Func_Node));

    if(NULL == pstFuncNode)
    {
        return OS_ERROR;
    }

    pstFuncNode->pHandler = handler;
    pstFuncNode->pParam = param;

    pstFuncNode->pNext = g_hook_func_node;
    g_hook_func_node = pstFuncNode;

    return  LOS_OK;
}

UINT32 osReHookFuncDel(STORAGE_HOOK_FUNC handler)
{
    Hook_Func_Node *pstFuncNode;
    Hook_Func_Node *pstCurFuncNode;
    while(g_hook_func_node)
    {
        pstCurFuncNode = g_hook_func_node;
        if (g_hook_func_node->pHandler == handler)
        {
            g_hook_func_node = g_hook_func_node->pNext;
            free(pstCurFuncNode);
            continue;
        }
        break;
    }

    if (g_hook_func_node)
    {
        pstCurFuncNode = g_hook_func_node;
        while(pstCurFuncNode->pNext)
        {
            pstFuncNode = pstCurFuncNode->pNext;
            if (pstFuncNode->pHandler == handler)
            {
                pstCurFuncNode->pNext = pstFuncNode->pNext;
                free(pstFuncNode);
                continue;
            }
            pstCurFuncNode = pstCurFuncNode->pNext;
        }
    }
    return LOS_OK;
}

VOID osReHookFuncHandle(VOID)
{
    Hook_Func_Node *pstFuncNode;

    pstFuncNode = g_hook_func_node;
    while(pstFuncNode)
    {
        (void)pstFuncNode->pHandler(pstFuncNode->pParam);
        pstFuncNode = pstFuncNode->pNext;
    }
}

SHELLCMD_ENTRY(reset_shellcmd, CMD_TYPE_EX,"reset",0,(CMD_CBK_FUNC)cmd_reset); /*lint !e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* LOSCFG_SHELL */
