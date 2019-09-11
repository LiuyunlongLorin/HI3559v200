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
#include "los_memory.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include "shcmd.h"
#include "shell.h"

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdDumpMemByte(size_t uwlen, AARCHPTR uwAddr)
{
    size_t uwDatalen;
    AARCHPTR *pAlignAddr;
    size_t uwCnt = 0;

    PRINTK("%p\n",uwAddr);

    if (uwlen == 0)
    {
        pAlignAddr = (AARCHPTR *)TRUNCATE(uwAddr, sizeof(AARCHPTR));
        PRINTK("\r\nThe align-address:%p  value:0x%lx\n", pAlignAddr, *(AARCHPTR *)pAlignAddr);
        return 0;
    }

    uwDatalen = ALIGN(uwlen, sizeof(AARCHPTR));
    pAlignAddr = (AARCHPTR *)TRUNCATE(uwAddr, sizeof(AARCHPTR));

    if ((pAlignAddr != (AARCHPTR *)uwAddr) &&
         (((AARCHPTR)pAlignAddr + uwDatalen - 1) < (uwAddr + uwlen - 1)))
    {
        uwDatalen = uwDatalen + sizeof(AARCHPTR);
    }

    PRINTK("\r\nThe address begin %p,length:0x%lx\n", pAlignAddr, uwDatalen);
    while (1)
    {
        if (uwDatalen)
        {
            if (0 == uwCnt % sizeof(AARCHPTR))
            {
                PRINTK("\n  0x%lx  :", pAlignAddr);
            }
#ifndef __LP64__
            PRINTK("%0+8lx ",*pAlignAddr);
#else
            PRINTK("%0+16lx ",*pAlignAddr);
#endif
            pAlignAddr ++;
            uwDatalen -= sizeof(AARCHPTR);
            uwCnt++;
            continue;
        }
        break;
    }
    PRINTK("\nThe address end 0x%lx\n", pAlignAddr);

    return 0;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemRead(INT32 argc, CHAR *argv[])
{
    AARCHPTR uwtempAddr;
    size_t uwlen;
    CHAR *ptrlen = (CHAR *)NULL;
    CHAR *ptrAddr = (CHAR *)NULL;

    if (argc == 0 || argc > 2)
    {
        PRINTK("\nUsage: readreg [ADDRESS] [LENGTH]\n");
        return 0;
    }

    if (argc == 1)
    {
        uwlen = 0;
    }
    else
    {
        uwlen = strtoul(argv[1], &ptrlen, 0);
        if(ptrlen == NULL || *ptrlen != 0)
        {
            PRINTK("readreg invalid length %s\n",argv[1]);
            return OS_ERROR;
        }
    }
    uwtempAddr = strtoul(argv[0], &ptrAddr, 0);
    if(ptrAddr == (CHAR *)NULL || *ptrAddr != 0)
    {
        PRINTK("readreg invalid address %s\n",argv[0]);
        return OS_ERROR;
    }

    (VOID)osShellCmdDumpMemByte(uwlen, uwtempAddr);

    return 0;

}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemWrite(INT32 argc, CHAR *argv[])
{
    AARCHPTR *pAlignAddr;
    AARCHPTR uwtmpAddr;
    CHAR *ptrVal = (CHAR *)NULL;
    CHAR *ptrAddr = (CHAR *)NULL;
    AARCHPTR uwvalue;

    if (argc == 2)
    {
        uwtmpAddr = strtoul(argv[0], &ptrAddr, 0);
        if(ptrAddr == NULL || *ptrAddr != 0)
        {
            PRINTK("writereg invalid address %s\n",argv[0]);
            return OS_ERROR;
        }

        uwvalue = strtoul(argv[1], &ptrVal, 0);
        if(ptrVal == (CHAR *)NULL || *ptrVal != 0)
        {
            PRINTK("writereg invalid value %s\n",argv[1]);
            return OS_ERROR;
        }

        pAlignAddr = (AARCHPTR *)TRUNCATE(uwtmpAddr, sizeof(AARCHPTR));

        *pAlignAddr = uwvalue;
        PRINTK("\nThe align-address:%p,write value:0x%lx", pAlignAddr, uwvalue);
    }
    else
    {
        PRINTK("Usage: writereg [ADDRESS] [VALUE]\n");
    }

    return 0;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemcheck(INT32 argc, CHAR *argv[])
{
    if (argc > 0)
    {
        PRINTK("\nUsage: memcheck\n");
        return OS_ERROR;
    }

    if (LOS_OK == LOS_MemIntegrityCheck(m_aucSysMem1))
    {
        PRINTK("system memcheck over, all passed!\n");
    }
#ifdef LOSCFG_EXC_INTERACTION
    if (LOS_OK == LOS_MemIntegrityCheck(m_aucSysMem0))
    {
        PRINTK("exc interaction memcheck over, all passed!\n");
    }
#endif
    return 0;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdFree(INT32 argc, CHAR *argv[])
{
    UINT32 uwMemUsed = 0;
    UINT32 uwTotalMem = 0;
    UINT32 uwFreeMem = 0;
#ifdef LOSCFG_EXC_INTERACTION
    UINT32 uwMemUsed0 = 0;
    UINT32 uwTotalMem0 = 0;
    UINT32 uwFreeMem0 = 0;
#endif
    size_t uwTextLen = 0;
    size_t uwDataLen = 0;
    size_t uwRodataLen = 0;
    size_t uwBssLen;
    extern UINT32 LOS_MemTotalUsedGet(VOID *pPool);
    extern UINT32 LOS_MemPoolSizeGet(VOID *pPool);
#ifdef LOSCFG_KERNEL_SCATTER
    extern CHAR __fast_rodata_start;
    extern CHAR __fast_rodata_end;
    extern CHAR __fast_data_start;
    extern CHAR __fast_data_end;
    extern CHAR __fast_text_start;
    extern CHAR __fast_text_end;
#endif
    extern CHAR __rodata_start;
    extern CHAR __rodata_end;
    extern CHAR __ram_data_start;
    extern CHAR __ram_data_end;
    extern CHAR __text_start;
    extern CHAR __text_end;
    extern CHAR __bss_start;
    extern CHAR __bss_end;
#ifdef LOSCFG_EXC_INTERACTION
    uwMemUsed0 = LOS_MemTotalUsedGet(m_aucSysMem0);
    uwTotalMem0 = LOS_MemPoolSizeGet(m_aucSysMem0);
    uwFreeMem0 = uwTotalMem0 - uwMemUsed0;
#endif
    uwMemUsed = LOS_MemTotalUsedGet(m_aucSysMem1);
    uwTotalMem = LOS_MemPoolSizeGet(m_aucSysMem1);
    uwFreeMem = uwTotalMem - uwMemUsed;

    if (argc > 1)
    {
        PRINTK("\nUsage: free or free [-k/-m]\n");
        return OS_ERROR;
    }
#ifdef LOSCFG_EXC_INTERACTION
    PRINTK("\r\n***** Mem:system mem      Mem1:exception interaction mem *****\n");
#endif
    if ((0 == argc) ||
        (1 == argc && 0 == strncmp(argv[0], "-k", 2)) ||
        (1 == argc && 0 == strncmp(argv[0], "-m", 2)))
    {
        PRINTK("\r\n        total        used          free\n");
    }

    if (1 == argc && 0 == strncmp(argv[0], "-k", 2))
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d\n", uwTotalMem/1024, uwMemUsed/1024, uwFreeMem/1024);
#ifdef LOSCFG_EXC_INTERACTION
        PRINTK("Mem1:   %-9d    %-10d    %-10d\n", uwTotalMem0/1024, uwMemUsed0/1024, uwFreeMem0/1024);
#endif
    }
    else if (1 == argc && 0 == strncmp(argv[0], "-m", 2))
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d\n", uwTotalMem/(1024*1024), uwMemUsed/(1024*1024), uwFreeMem/(1024*1024));
#ifdef LOSCFG_EXC_INTERACTION
        PRINTK("Mem1:   %-9d    %-10d    %-10d\n", uwTotalMem0/(1024*1024), uwMemUsed0/(1024*1024), uwFreeMem0/(1024*1024));
#endif
    }
    else if (0 == argc)
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d\n", uwTotalMem, uwMemUsed, uwFreeMem);
#ifdef LOSCFG_EXC_INTERACTION
        PRINTK("Mem1:   %-9d    %-10d    %-10d\n", uwTotalMem0, uwMemUsed0, uwFreeMem0);
#endif
    }
    else
    {
        PRINTK("\nUsage: free or free [-k/-m]\n");
        return OS_ERROR;
    }
#ifdef LOSCFG_KERNEL_SCATTER
    uwTextLen = &__fast_text_end - &__fast_text_start;
    uwDataLen = &__fast_data_end - &__fast_data_start;
    uwRodataLen = &__fast_rodata_end - &__fast_rodata_start;
#endif
    uwTextLen += &__text_end - &__text_start;
    uwDataLen += &__ram_data_end - &__ram_data_start;
    uwRodataLen += &__rodata_end - &__rodata_start;
    uwBssLen = &__bss_end - &__bss_start;
    PRINTK("\r\n        text         data          rodata        bss\n");
    if (1 == argc && 0 == strncmp(argv[0], "-k", 2))
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d    %-10d\n", uwTextLen/1024, uwDataLen/1024,
            uwRodataLen/1024, uwBssLen/1024);
    }
    else if (1 == argc && 0 == strncmp(argv[0], "-m", 2))
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d    %-10d\n", uwTextLen/(1024*1024), uwDataLen/(1024*1024),
            uwRodataLen/(1024*1024), uwBssLen/(1024*1024));
    }
    else
    {
        PRINTK("Mem:    %-9d    %-10d    %-10d    %-10d\n", uwTextLen, uwDataLen, uwRodataLen, uwBssLen);
    }

    return 0;
}

LITE_OS_SEC_TEXT_MINOR VOID osDumpMemory(CHAR *pcBuffer, UINT32 uwLen)
{
    CHAR stream[60];
    CHAR byteOffsetStr[10];
    UINT32 i;
    UINT32 offset, count, byteOffset;

    PRINTK("<------------Dump buffer %d bytes at %p------------------->\n", uwLen, pcBuffer);
    count = 0;
    offset = 0;
    byteOffset = 0;
    for (i = 0; i < uwLen; i ++)
    {
        (VOID)snprintf(stream + offset, sizeof(stream) - offset, "%2.2X ", pcBuffer[i]);
        count ++;
        offset += 3;

        if (count == 16)
        {
            count = 0;
            offset = 0;
            (VOID)snprintf(byteOffsetStr, sizeof(byteOffsetStr), "%4.4X", pcBuffer[byteOffset]);
            PRINTK("[%s]:%s\n", byteOffsetStr, stream);
            memset(stream, 0, sizeof(stream));
            byteOffset += 16;
        }
    }

    if (offset != 0)
    {
        (VOID)snprintf(byteOffsetStr, sizeof(byteOffsetStr), "%4.4X", pcBuffer[byteOffset]);
        PRINTK("[%s]:%s\n", byteOffsetStr, stream);
    }
}

LITE_OS_SEC_TEXT_MINOR INT32 osShellCmdUname(INT32 argc, CHAR *argv[])
{
    if(argc == 0)
    {
        PRINTK("Huawei LiteOS\n");
        return 0;
    }

    if(argc == 1)
    {
        if(strcmp(argv[0], "-a") == 0)
        {
            PRINTK("%s %s %d.%d.%d %s %s\n", VER, HW_LITEOS_SYSNAME, MAJ_V, MIN_V, REL_V, __DATE__, __TIME__);
            return 0;
        }
        else if(strcmp(argv[0], "-s") == 0)
        {
            PRINTK("Huawei LiteOS\n");
            return 0;
        }
        else if(strcmp(argv[0], "-t") == 0)
        {
            PRINTK("build date : %s %s", __DATE__, __TIME__);
            return 0;
        }
        else if(strcmp(argv[0], "-v") == 0)
        {
            PRINTK("%s %d.%d.%d %s %s\n", HW_LITEOS_SYSNAME, MAJ_V, MIN_V, REL_V, __DATE__, __TIME__);
            return 0;
        }
        else if(strcmp(argv[0], "--help") == 0)
        {
            PRINTK("-a,            print all information\n"
                        "-s,            print the kernel name\n"
                        "-t,            print the build date\n"
                        "-v,            print the kernel version\n");
            return 0;
        };
    }

    PRINTK("uname: invalid option %s\n"
                "Try 'uname --help' for more information.\n", argv[0]);
    return 0;
}
#ifdef LOS_MEM_LEAK_CHECK
LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemLeak(INT32 argc, CHAR *argv[])
{
    if (argc > 0)
    {
        PRINTK("\nUsage: memleak\n");
        return OS_ERROR;
    }

    LOS_MemLeakCheckShow();
    return 0;
}
LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemCount(INT32 argc, CHAR *argv[])
{
    if (argc > 0)
    {
        PRINTK("\nUsage: memcount\n");
        return OS_ERROR;
    }

    LOS_CheckMaxcount();
    return 0;
}
#endif

#ifdef LOSCFG_KERNEL_MEM_SPEC
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdSpecInfo(INT32 argc, CHAR *argv[])
{
    if (argc > 0)
    {
        PRINTK("\nUsage: specinfo\n");
        return OS_ERROR;
    }

    LOS_SpecInfoShow();
    return 0;
}
#endif
#endif
#ifdef LOSCFG_MEM_RECORDINFO
extern UINT32 g_uwMemRecordShowEnable;
LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemRecordEnable(INT32 argc, CHAR *argv[])
{
    g_uwMemRecordShowEnable = 1;
    return 0;
}

LITE_OS_SEC_TEXT_MINOR UINT32 osShellCmdMemRecordDisable(INT32 argc, CHAR *argv[])
{
    g_uwMemRecordShowEnable = 0;
    return 0;
}

SHELLCMD_ENTRY(memshowenable_shellcmd, CMD_TYPE_EX, "memshowenable", 0, (CMD_CBK_FUNC)osShellCmdMemRecordEnable); /*lint !e19*/
SHELLCMD_ENTRY(memshowdisable_shellcmd, CMD_TYPE_EX, "memshowdisable", 0, (CMD_CBK_FUNC)osShellCmdMemRecordDisable); /*lint !e19*/
#endif
#ifdef LOS_MEM_LEAK_CHECK
SHELLCMD_ENTRY(memleak_shellcmd, CMD_TYPE_EX, "memleak", 0, (CMD_CBK_FUNC)osShellCmdMemLeak); /*lint !e19*/
SHELLCMD_ENTRY(memcount_shellcmd, CMD_TYPE_EX, "memcount", 0, (CMD_CBK_FUNC)osShellCmdMemCount); /*lint !e19*/
#endif

#ifdef LOSCFG_KERNEL_MEM_SPEC
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
SHELLCMD_ENTRY(specinfoshow_shellcmd, CMD_TYPE_EX, "specinfo", 0, (CMD_CBK_FUNC)osShellCmdSpecInfo); /*lint !e19*/
#endif
#endif
SHELLCMD_ENTRY(memcheck_shellcmd, CMD_TYPE_EX, "memcheck", 1, (CMD_CBK_FUNC)osShellCmdMemcheck); /*lint !e19*/
SHELLCMD_ENTRY(uname_shellcmd, CMD_TYPE_EX, "uname", 1, (CMD_CBK_FUNC)osShellCmdUname); /*lint !e19*/
SHELLCMD_ENTRY(free_shellcmd, CMD_TYPE_EX, "free", XARGS, (CMD_CBK_FUNC)osShellCmdFree); /*lint !e19*/
//SHELLCMD_ENTRY(writereg_shellcmd, CMD_TYPE_EX, "writereg", 2, (CMD_CBK_FUNC)osShellCmdMemWrite); /*lint !e19*/
//SHELLCMD_ENTRY(readreg_shellcmd, CMD_TYPE_EX, "readreg", 2, (CMD_CBK_FUNC)osShellCmdMemRead); /*lint !e19*/
#endif
