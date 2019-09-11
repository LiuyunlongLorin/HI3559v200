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
#include "los_hwi.ph"
#include "shcmd.h"
#include "shell.h"

#ifdef LOSCFG_CPUP_INCLUDE_IRQ
#include "los_cpup.h"
#include "stdlib.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#ifdef LOSCFG_CPUP_INCLUDE_IRQ
LITE_OS_SEC_TEXT_MINOR UINT32 shCmd_hwi(int argc, char **argv)
{
    UINT32 i  = 0;
    UINTPTR uwIntSave;
    UINT32 uwMaxTaskNum = g_uwTskMaxNum + OS_HWI_MAX_NUM;
    CPUP_INFO_S *pstCpu     = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *pstCpu10s  = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *pstCpu1s   = (CPUP_INFO_S *)NULL;

    if (argc > 0)
    {
        PRINTK("\nUsage: hwi\n");
        return OS_ERROR;
    }

    pstCpu = (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
    if (pstCpu == NULL)
    {
        PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }
    memset((VOID *)pstCpu, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);

    pstCpu10s = (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
    if (pstCpu10s == NULL)
    {
        PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
        return OS_ERROR;
    }
    memset((VOID *)pstCpu10s, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);

    pstCpu1s= (CPUP_INFO_S *)LOS_MemAlloc(m_aucSysMem0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
    if (pstCpu1s == NULL)
    {
        PRINTK("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
        (VOID)LOS_MemFree(m_aucSysMem0, pstCpu10s);
        return OS_ERROR;
    }
    memset((VOID *)pstCpu1s, 0, sizeof(CPUP_INFO_S) * uwMaxTaskNum);
    uwIntSave = LOS_IntLock();
    (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu, 0xffff);
    (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu10s, 0);
    (VOID)LOS_AllTaskCpuUsage(uwMaxTaskNum, pstCpu1s, 1);
    LOS_IntRestore(uwIntSave);

    PRINTK(" InterruptNo     Count     Name           CPUUSE   CPUUSE10s   CPUUSE1s   mode\n");
    for(i = OS_HWI_FORM_EXC_NUM; i < OS_HWI_MAX_NUM + OS_HWI_FORM_EXC_NUM; i++)
    {
        /* Different cores has different hwi form implementation */
        if (HWI_IS_REGISTED && g_ucHwiFormName[i])
        {
            PRINTK(" %8d:%10d:      %-12s    "
                        "%2d.%-7d"
                        "%2d.%-9d"
                        "%2d.%-6d"
                        "%s\n", i, g_vuwHwiFormCnt[i], g_ucHwiFormName[i],
                        pstCpu[g_uwTskMaxNum + i].uwUsage / LOS_CPUP_PRECISION_MULT,
                        pstCpu[g_uwTskMaxNum + i].uwUsage % LOS_CPUP_PRECISION_MULT,
                        pstCpu10s[g_uwTskMaxNum + i].uwUsage / LOS_CPUP_PRECISION_MULT,
                        pstCpu10s[g_uwTskMaxNum + i].uwUsage % LOS_CPUP_PRECISION_MULT,
                        pstCpu1s[g_uwTskMaxNum + i].uwUsage / LOS_CPUP_PRECISION_MULT,
                        pstCpu1s[g_uwTskMaxNum + i].uwUsage % LOS_CPUP_PRECISION_MULT,
                        m_astHwiForm[i].uwParam == IRQF_SHARED ? "shared":"normal");

        }
        else if (HWI_IS_REGISTED)
        {
            PRINTK(" %8d:%10d:      %-12s    "
                        "%2d.%-7d"
                        "%2d.%-9d"
                        "%2d.%-6d"
                        "%s\n", i, g_vuwHwiFormCnt[i], "", 0, 0, 0, 0, 0, 0,
                        m_astHwiForm[i].uwParam == IRQF_SHARED ? "shared":"normal");
        }
    }
    (VOID)LOS_MemFree(m_aucSysMem0, pstCpu);
    (VOID)LOS_MemFree(m_aucSysMem0, pstCpu10s);
    (VOID)LOS_MemFree(m_aucSysMem0, pstCpu1s);
    return 0;
}
#else
LITE_OS_SEC_TEXT_MINOR UINT32 shCmd_hwi(int argc, char **argv)
{
    UINT32 i  = 0;

    if (argc > 0)
    {
        PRINTK("\nUsage: hwi\n");
        return OS_ERROR;
    }

    PRINTK(" InterruptNo     Count     Name\n");
    for(i = OS_HWI_FORM_EXC_NUM; i < OS_HWI_MAX_NUM + OS_HWI_FORM_EXC_NUM; i++)
    {
        /* Different cores has different hwi form implementation */
        if (HWI_IS_REGISTED && g_ucHwiFormName[i])
        {
            PRINTK(" %8d:%10d:      %-s\n",i,g_vuwHwiFormCnt[i],g_ucHwiFormName[i]);
        }
        else if(HWI_IS_REGISTED)
        {
            PRINTK(" %8d:%10d:\n",i,g_vuwHwiFormCnt[i]);
        }
    }
    return 0;
}
#endif

SHELLCMD_ENTRY(hwi_shellcmd, CMD_TYPE_EX, "hwi", 1, (CMD_CBK_FUNC)shCmd_hwi); /*lint !e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* LOSCFG_SHELL */

