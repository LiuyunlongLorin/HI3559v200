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

#include "los_hwi.h"
#include "los_tick.ph"
#include "time.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#ifdef LOSCFG_SHELL_EXCINFO
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
#include "linux/mtd/mtd.h"
#include "linux/module.h"
#include "linux/mtd/mtd_list.h"
#endif

log_read_write_fn  g_pfnExcInfoRW = NULL;                                           // the hook of   read-writing exception information
CHAR *g_pExcInfoBuf  = NULL;                                                              //pointer  to the buffer for storing the exception information
UINT32 g_uwExcInfoIndex = 0xffff;                                                     //the index of the buffer  for storing the exception information
UINT32 g_uwRecordAddr = 0;                                                                 //the address  of  storing the exception information
UINT32 g_uwRecordSpace = 0;                                                               //the size of  storing the exception information


#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
static struct mtd_info *mtdSpinor;
extern int spinor_init(void);

static void osSpiflashErase(size_t start, size_t size) {
    struct erase_info stEraseInfo;
    memset(&stEraseInfo, 0, sizeof(struct erase_info));
    stEraseInfo.mtd        = mtdSpinor;
    stEraseInfo.callback  = NULL; /*lint !e64*/
    stEraseInfo.fail_addr = (uint64_t)MTD_FAIL_ADDR_UNKNOWN;
    stEraseInfo.addr       = start;
    stEraseInfo.len         = size;
    stEraseInfo.time       = 1;
    stEraseInfo.retries    = 1;
    stEraseInfo.dev        = 0;
    stEraseInfo.cell        = 0;
    stEraseInfo.priv       = 0;
    stEraseInfo.state      = 0;
    stEraseInfo.next       = (struct erase_info *)NULL;
    stEraseInfo.scrub     = 0;
    (void)mtdSpinor->erase(mtdSpinor, &stEraseInfo);
}

static int osWriteExcInfoToSpiFlush(UINT32 uwStartAddr, UINT32 uwSpace, VOID *pBuf)
{
    UINT32  uwOutLen;
    UINT32  uwLen = (uwSpace < g_uwRecordSpace) ? uwSpace:g_uwRecordSpace;
    int          wRet  = -1;
    osSpiflashErase(g_uwRecordAddr, LOS_Align(g_uwRecordSpace,mtdSpinor->erasesize));
    wRet = mtdSpinor->write(mtdSpinor, g_uwRecordAddr,uwLen,&uwOutLen,(const char*)pBuf);
    return wRet;
}

static int  osReadExcInfoForSpiFlush(UINT32 uwStartAddr, UINT32 uwSpace, VOID *pBuf)
{
     UINT32  uwOutLen;
     UINT32  uwLen = (uwSpace < g_uwRecordSpace) ? uwSpace:g_uwRecordSpace;
     int           wRet = -1;
     wRet = mtdSpinor->read(mtdSpinor, g_uwRecordAddr, uwLen, &uwOutLen, (const char *)pBuf);
     return wRet;
}
#endif
#endif


extern UINT32 time_clk_read(void);

static volatile unsigned long long sched_clock_ns = 0;
static volatile unsigned long long sched_clock_cycle = 0;
static volatile unsigned long time_clk_last = 0;

static unsigned long update_time_clk(unsigned long *time_clk32)
{
    unsigned long time_clk_now;

    time_clk_now = time_clk_read();
    if(time_clk_now >= time_clk_last)
    {
        *time_clk32 = (unsigned long)(time_clk_now - time_clk_last);
    }
    else
    {
        *time_clk32 = (unsigned long)(time_clk_now + (TIMER_MAXLOAD - time_clk_last));
    }
    return time_clk_now;
}

extern void os_adjtime(void);
void update_sched_clock(void)
{
    unsigned long time_clk32;
    unsigned int uwIntSave;

    uwIntSave = LOS_IntLock();
    time_clk_last = update_time_clk(&time_clk32);
    sched_clock_cycle += (unsigned long long)time_clk32;
    sched_clock_ns += (unsigned long long)CYCLE_TO_NS(time_clk32);
    (void)os_adjtime();
    (void)LOS_IntRestore(uwIntSave);
}

static unsigned long long get_sched_clock(void)
{
    unsigned long time_clk32;
    update_time_clk(&time_clk32);/*lint !e534*/

    return  (sched_clock_ns + (unsigned long long)CYCLE_TO_NS(time_clk32));
}

unsigned long long sched_clock(void)
{
    unsigned int uwIntSave;
    unsigned long long time_clk64;

    uwIntSave = LOS_IntLock();
    time_clk64 = get_sched_clock();
    (void)LOS_IntRestore(uwIntSave);

    return time_clk64;
}

unsigned long long hi_sched_clock(void)
{
    unsigned long long c;
    c = sched_clock();
    return c;
}

unsigned long long hi_sched_cycle(void)
{
    unsigned long time_clk32;
    unsigned int uwIntSave;
    unsigned long long cycle = 0;

    uwIntSave = LOS_IntLock();
    update_time_clk(&time_clk32);/*lint !e534*/
    cycle = sched_clock_cycle + (unsigned long long)time_clk32;
    (void)LOS_IntRestore(uwIntSave);

    return cycle;
}

#ifdef LOSCFG_SHELL_EXCINFO
VOID LOS_ExcInfoRegHook(UINT32 uwStartAddr, UINT32 uwSpace, CHAR *pBuf,  log_read_write_fn pfnHook)
{
    if(pfnHook == NULL || pBuf == NULL)
    {
        PRINT_ERR("pBuf or pfnHook is null\n");
        return;
    }
    g_uwRecordAddr     = uwStartAddr;
    g_uwRecordSpace    = uwSpace;
    g_pExcInfoBuf         = pBuf;
    g_pfnExcInfoRW     = pfnHook;

#ifdef LOSCFG_FS_VFS
    extern void los_vfs_init(void);
    los_vfs_init();
#endif
}

VOID osReadWriteExceptionInfo(UINT32 uwStartAddr, UINT32 uwSpace, UINT32 uwRWFlag, CHAR *pBuf)
{
    if(pBuf == NULL || uwSpace == 0)
    {
        PRINT_ERR("buffer is null or uwSpace is zero\n");
        return;
    }

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    mtdSpinor = get_mtd("spinor");
    if(mtdSpinor == NULL)
    {
        (void)spinor_init();
        mtdSpinor = get_mtd("spinor");
        if(mtdSpinor == NULL)
        {
            PRINT_ERR("Init spinor is failed\n");
            return;
        }
    }
#endif

    if(uwRWFlag == 0)
    {
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
        if(osWriteExcInfoToSpiFlush(uwStartAddr,uwSpace,pBuf) != LOS_OK)
        {
            PRINT_ERR("Exception information written to flush failed\n");
        }
        free(pBuf);                                                        //Consider whether or not  the "pBuf" is released according to actual use
        pBuf = NULL;
#endif
    }
    else if(uwRWFlag == 1)
    {
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
        if(osReadExcInfoForSpiFlush(uwStartAddr,uwSpace,pBuf) != LOS_OK)
        {
            PRINT_ERR("Exception information read from flush failed\n");
        }
#endif
    }
    else
    {
        PRINT_ERR("RWFlag is error\n");
    }
}

void osRecordExcInfoTime(void)
{
#ifdef LOSCFG_FS_VFS
    time_t t;
    struct tm *stTM;
    char nowtime[24];

    (void)time(&t);
    stTM = localtime(&t);
    memset(nowtime, 0, sizeof(nowtime));
    (void)strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", stTM);/*lint !e586*/

    if(g_uwRecordSpace > g_uwExcInfoIndex)
    {
        g_uwExcInfoIndex += snprintf(g_pExcInfoBuf + g_uwExcInfoIndex, g_uwRecordSpace - g_uwExcInfoIndex,
               "%s \n",nowtime);
    }
#endif
}
#endif
