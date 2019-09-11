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

#include "los_sem.ph"
#include "los_task.ph"

#include "shcmd.h"

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdSemInfoGet(UINT32 argc, CHAR **argv)
{
    UINT32    uwLoop,uwSemCnt = 0;
    UINTPTR   uvIntSave;
    UINT32    uwFirstNode = 1;
    LOS_TASK_CB *pstTskCB = (LOS_TASK_CB *)NULL;
    SEM_CB_S    *pstSemNode = (SEM_CB_S *)NULL;
    UINT32    uwRet = LOS_OK;
    UINT16    usSemID;
    CHAR      *pscPtr = (CHAR *)NULL;

    if (argc > 1)
    {
        PRINTK("\nUsage: sem [ID]\n");
        return OS_ERROR;
    }

    if (argc == 0)
    {
        usSemID = 0xffff;
    }
    else
    {
        usSemID = (UINT16)strtoul(argv[0], &pscPtr, 0);
        if (pscPtr == (CHAR *)NULL || *pscPtr != 0 || (INT16)usSemID < 0)
        {
            PRINTK("\nsem ID can't access %s.\n", argv[0]);
            return 0;
        }
    }

    if (0xffff == usSemID)
    {
        for (uwLoop = 0; uwLoop < LOSCFG_BASE_IPC_SEM_LIMIT; uwLoop++)
        {
            pstSemNode = GET_SEM(uwLoop);
            if (pstSemNode->usSemStat == OS_SEM_USED)
            {
                uwSemCnt++;
                PRINTK("\r\n   SemID     Count\n");
                PRINTK("   -----     -----\n");
                PRINTK("   %d          0x%u\n",pstSemNode->usSemID,pstSemNode->uwSemCount);
            }
        }
        PRINTK("   SemUsingNum    :  %d\n\n",uwSemCnt);
        return uwRet;
    }
    else
    {
        if (usSemID >= LOSCFG_BASE_IPC_SEM_LIMIT)
        {
            PRINTK("\nInvalid semphore id!\n");
            return uwRet;
        }

        pstSemNode = GET_SEM(usSemID);

        if (pstSemNode->usSemStat != OS_SEM_USED)
        {
            PRINTK("\nThe semphore is not in use!\n");
            return uwRet;
        }

        PRINTK("\r\n   SemID     Count\n");
        PRINTK("   -----     -----\n");
        PRINTK("   %d          0x%u\n",pstSemNode->usSemID,pstSemNode->uwSemCount);

        if (LOS_ListEmpty(&pstSemNode->stSemList))
        {
            PRINTK("No task is pended on this semphore!\n");
            return uwRet;
        }
        else
        {
            PRINTK("Pended task list : ");

            uvIntSave = LOS_IntLock();

            LOS_DL_LIST_FOR_EACH_ENTRY(pstTskCB, &pstSemNode->stSemList, LOS_TASK_CB, stPendList)
            {
                if (uwFirstNode)
                {
                    PRINTK("%s\n",pstTskCB->pcTaskName);
                    uwFirstNode = 0;
                }
                else
                {
                    PRINTK(", %s\n",pstTskCB->pcTaskName);
                }
            }

            LOS_IntRestore(uvIntSave);
        }
    }

    return LOS_OK;
}

SHELLCMD_ENTRY(sem_shellcmd, CMD_TYPE_EX, "sem", 1, (CMD_CBK_FUNC)osShellCmdSemInfoGet); /*lint !e19*/
#endif
