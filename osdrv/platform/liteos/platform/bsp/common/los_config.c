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

#include "los_sys.h"
#include "los_tick.h"
#include "los_task.ph"
#include "los_config.h"
#include "los_printf.h"
#include "los_swtmr.h"
#include "los_swtmr.ph"
#include "los_timeslice.ph"
#include "los_memory.ph"
#include "hisoc/uart.h"
#include "hisoc/cpu.h"
#include "los_sem.ph"
#include "los_mux.ph"
#include "los_queue.ph"
#include "los_memstat.ph"
#include "los_hwi.ph"
#include "pprivate.h"

#ifdef LOSCFG_SHELL_DMESG
#include "dmesg.ph"
#endif

#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#include "stdio.h"
#endif
#ifdef LOSCFG_COMPAT_LINUX
#include "linux/workqueue.h"
#include "linux/module.h"
#endif
#ifdef LOSCFG_DRIVERS_UART
#include "console.h"
#endif
#ifdef LOSCFG_KERNEL_TICKLESS
#include "los_tickless.h"
#endif
#ifdef LOSCFG_ARCH_CORTEX_M7
#include "los_exc.ph"
#endif
#ifdef LOSCFG_MEM_RECORDINFO
#include "los_memrecord.ph"
#endif
#if defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) || defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
#include "mmu.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern CHAR __bss_end;
extern VOID update_sched_clock(VOID);
extern VOID osHwiInit(VOID);
extern UINT32 osTickInit(UINT32 uwSystemClock, UINT32 uwTickPerSecond);
extern struct workqueue_struct *__create_workqueue_key(const char *, int, int, int, struct lock_class_key *, const char *);
extern unsigned int hrtimers_init(void);

#ifdef LOSCFG_TEST
#if defined LOSCFG_PLATFORM_HI3911
extern void Watchdog_Disable(void);
#endif
extern UINT32 los_TestInit(VOID);
#else
extern UINT32 osAppInit(VOID);
#endif


LITE_OS_SEC_DATA_MINOR UINT16 g_swtmr_id;
LITE_OS_SEC_TEXT_INIT VOID sched_clock_swtmr(VOID)
{
    UINT32 uwRet;
    uwRet = LOS_SwtmrCreate(100, LOS_SWTMR_MODE_PERIOD, (SWTMR_PROC_FUNC)update_sched_clock, &g_swtmr_id, 0);
    if(uwRet != LOS_OK){
        PRINT_ERR("LOS_SwtmrCreate error %d\n", uwRet);
        return;
    }
    uwRet = LOS_SwtmrStart(g_swtmr_id);
    if(uwRet != LOS_OK){
        PRINT_ERR("LOS_SwtmrStart error %d\n", uwRet);
        return;
    }
}

#ifdef LOSCFG_COMPAT_POSIX
extern _pthread_data *PTHREADS_DATA;
extern CHAR LOS_BLOCK_START(pthdata);
extern CHAR LOS_BLOCK_END(pthdata);

extern struct mqarray *g_squeuetable;
extern CHAR LOS_BLOCK_START(mqcb);
extern CHAR LOS_BLOCK_END(mqcb);
#endif

#ifdef LOSCFG_LIB_LIBC
extern int *errno_array;
extern CHAR LOS_BLOCK_START(errno);
extern CHAR LOS_BLOCK_END(errno);
#endif
extern TSK_MEM_USED_INFO *g_TskMemUsedInfo;
extern CHAR LOS_BLOCK_START(sysmemused);
extern CHAR LOS_BLOCK_END(sysmemused);

#ifdef LOS_MEM_SLAB
extern TSK_MEM_USED_INFO *g_TskSlabUsedInfo;
extern CHAR LOS_BLOCK_START(sysslabused);
extern CHAR LOS_BLOCK_END(sysslabused);
#endif
#ifdef LOSCFG_DRIVERS_BASE
extern int platform_bus_init(void);
extern int bus_init(void);
extern int do_initCalls(int swLevel);
#endif

LITE_OS_SEC_TEXT_INIT VOID osRegister(VOID)
{
#ifdef LOSCFG_COMPAT_POSIX
    PTHREADS_DATA = (_pthread_data *)&LOS_BLOCK_START(pthdata);
    g_squeuetable = (struct mqarray *)&LOS_BLOCK_START(mqcb);
    memset(PTHREADS_DATA, 0, (&LOS_BLOCK_END(pthdata) - &LOS_BLOCK_START(pthdata)));
    memset(g_squeuetable, 0, (&LOS_BLOCK_END(mqcb) - &LOS_BLOCK_START(mqcb)));
#endif
#ifdef LOSCFG_LIB_LIBC
    errno_array = (int *)&LOS_BLOCK_START(errno);
    memset(errno_array, 0, (&LOS_BLOCK_END(errno) - &LOS_BLOCK_START(errno)));
#endif
    g_TskMemUsedInfo = (TSK_MEM_USED_INFO *)&LOS_BLOCK_START(sysmemused);
    memset(g_TskMemUsedInfo, 0, (&LOS_BLOCK_END(sysmemused) - &LOS_BLOCK_START(sysmemused)));
#ifdef LOS_MEM_SLAB
    g_TskSlabUsedInfo = (TSK_MEM_USED_INFO *)&LOS_BLOCK_START(sysslabused);
    memset(g_TskSlabUsedInfo, 0, (&LOS_BLOCK_END(sysslabused) - &LOS_BLOCK_START(sysslabused)));
#endif
    g_uwSysClock = OS_SYS_CLOCK;
    g_uwTickPerSecond =  LOSCFG_BASE_CORE_TICK_PER_SECOND;

    return;
}

LITE_OS_SEC_TEXT_INIT VOID osStart(void)
{
#if (LOSCFG_BASE_CORE_TICK_HW_TIME == YES)
    extern VOID osTickStart(VOID);
    osTickStart();
    sched_clock_swtmr();
#endif
    osStartToRun();
}

LITE_OS_SEC_TEXT_INIT int osMain(void)
{
    UINT32 uwRet;


#if !defined LOSCFG_PLATFORM_HIM5V100 && !defined LOSCFG_ARCH_CORTEX_M7
    extern VOID osSecPageInit(VOID);
    osSecPageInit();
#endif

    osRegister();

#if OS_SYS_NOCACHEMEM_SIZE
    uwRet = osNocacheMemSystemInit();
    if (uwRet != LOS_OK) {
        PRINT_ERR("osNocacheMemSystemInit error %d\n", uwRet);
        return uwRet;
    }
#endif



#if (LOSCFG_PLATFORM_HWI == YES)
    osHwiInit();
#endif

#ifdef LOSCFG_ARCH_CORTEX_M7
    osExcInit();
#endif

    uwRet = osTickInit(g_uwSysClock, LOSCFG_BASE_CORE_TICK_PER_SECOND);
    if (uwRet != LOS_OK) {
        return uwRet;
    }

#ifdef LOSCFG_PLATFORM_UART_WITHOUT_VFS

    uart_init();
#ifdef LOSCFG_SHELL
    uart_hwiCreate();
#endif //LOSCFG_SHELL
#endif //LOSCFG_PLATFORM_UART_WITHOUT_VFS
    uwRet =osTaskInit();
    if (uwRet != LOS_OK) {
        PRINT_ERR("osTaskInit error\n");
        return uwRet;
    }

#if (LOSCFG_BASE_CORE_TSK_MONITOR == YES)
    osTaskMonInit();
#endif

#ifdef LOSCFG_KERNEL_CPUP
    {
        extern UINT32 osCpupInit(VOID);
        uwRet = osCpupInit();
        if (uwRet != LOS_OK) {
            PRINT_ERR("osCpupInit error\n");
            return uwRet;
        }
#ifdef LOSCFG_ARCH_CORTEX_M7
    extern LITE_OS_SEC_TEXT_MINOR VOID osTskCycleEndStart(VOID);
    g_pfnUsrTskSwitchHook = (TSKSWITCHHOOK)osTskCycleEndStart;
#endif
    }
#endif

#if (LOSCFG_BASE_IPC_SEM == YES)
    {
        uwRet = osSemInit();
        if (uwRet != LOS_OK) {
            return uwRet;
        }
    }
#endif

#if (LOSCFG_BASE_IPC_MUX == YES)
    {
        uwRet = osMuxInit();
        if (uwRet != LOS_OK) {
            return uwRet;
        }
    }
#endif


#if (LOSCFG_BASE_IPC_QUEUE == YES)
    {
        uwRet = osQueueInit();
        if (uwRet != LOS_OK) {
            return uwRet;
        }
    }
#endif


#if (LOSCFG_BASE_CORE_SWTMR == YES)
    {
        uwRet = osSwTmrInit();
        if (uwRet != LOS_OK)
        {
            return uwRet;
        }
    }
#endif

#if (LOSCFG_BASE_CORE_TIMESLICE == YES)
    osTimesliceInit();
#endif

    uwRet = osIdleTaskCreate();
    if (uwRet != LOS_OK) {
        return uwRet;
    }

#if (LOSCFG_DRIVERS_BASE == YES)/*lint !e553*/
    {
        (VOID)bus_init();
        (VOID)platform_bus_init();
        (VOID)do_initCalls(LEVEL_ARCH);
    }
#endif

#ifdef LOSCFG_TEST
#if defined LOSCFG_PLATFORM_HI3911
    Watchdog_Disable();
#endif
    uwRet = los_TestInit();
#else /* LOSCFG_PLATFORM_OSAPPINIT */
    uwRet = osAppInit();
#endif
    if (uwRet != LOS_OK) {
        return uwRet;
    }
    return LOS_OK;

}

LITE_OS_SEC_TEXT_INIT int main(void)
{
    UINT32 uwRet;
#if defined(LOSCFG_ARCH_CORTEX_A7) || defined(LOSCFG_ARCH_CORTEX_A17) || defined(LOSCFG_ARCH_CORTEX_A53_AARCH32)
extern INT32 osMemNoAccessSet(AARCHPTR startaddr, size_t length);
    disable_ap_check();
    
    osMemNoAccessSet(0, 0x100000);
#if defined(LOSCFG_REGION_PROTECT_BASE_0) && defined(LOSCFG_REGION_PROTECT_SIZE_0)
    #if ((LOSCFG_REGION_PROTECT_BASE_0 & 0xfffff) || (LOSCFG_REGION_PROTECT_SIZE_0 & 0xfffff))
    #error Please check config:LOSCFG_REGION_PROTECT_BASE_0 LOSCFG_REGION_PROTECT_SIZE_0 should be aligan  1M!
    #endif
    osMemNoAccessSet(LOSCFG_REGION_PROTECT_BASE_0, LOSCFG_REGION_PROTECT_SIZE_0);
#endif
#if defined(LOSCFG_REGION_PROTECT_BASE_1) && defined(LOSCFG_REGION_PROTECT_SIZE_1)
    #if ((LOSCFG_REGION_PROTECT_BASE_1 & 0xfffff) || (LOSCFG_REGION_PROTECT_SIZE_1 & 0xfffff))
    #error Please check config:LOSCFG_REGION_PROTECT_BASE_1 LOSCFG_REGION_PROTECT_SIZE_1 should be aligan  1M!
    #endif
    osMemNoAccessSet(LOSCFG_REGION_PROTECT_BASE_1, LOSCFG_REGION_PROTECT_SIZE_1);
#endif

    clean_tlb();
    enable_ap_check();
#endif
#ifdef LOSCFG_SHELL_LK
    extern void osLkLoggerInit(const char *str);
    osLkLoggerInit(NULL);
#endif


#ifdef LOSCFG_EXC_INTERACTION
#ifdef LOSCFG_ARCH_CORTEX_M7
    // 4K space for Stack
    uwRet = osMemExcInteractionInit((UINT32)&__bss_end + 4096);
#else
    uwRet = osMemExcInteractionInit((AARCHPTR)&__bss_end);
#endif
    if (uwRet != LOS_OK) {
        return uwRet;
    }
#endif

#ifdef LOSCFG_ARCH_CORTEX_M7
    // 4K space for Stack
    uwRet = osMemSystemInit((UINT32)&__bss_end + 4096 + OS_EXC_INTERACTMEM_SIZE);
#else
    uwRet = osMemSystemInit((AARCHPTR)&__bss_end + OS_EXC_INTERACTMEM_SIZE);
#endif
    if (uwRet != LOS_OK) {
        return uwRet;
    }

#ifdef LOSCFG_SHELL_DMESG
    uwRet = osDmesgInit();
    if (uwRet != LOS_OK) {
        return uwRet;
    }
#endif

    PRINT_RELEASE("********Hello Huawei LiteOS********\n"
                "\nversion : %s\nopen-version : %s %d.%d.%d\n"
                "build data : %s %s\n\n"
                "**********************************\n",
                VER, HW_LITEOS_SYSNAME,MAJ_V,MIN_V,REL_V, __DATE__,__TIME__);

    uwRet = osMain();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }

    osStart();

    for (;;);
    /* Replace the dots (...) with your own code.  */
}

extern  void app_init(void);
UINT32 osAppInit(VOID)
{
#ifdef LOSCFG_PLATFORM_OSAPPINIT
    UINT32 ipctaskid;
    TSK_INIT_PARAM_S stappTask;
    UINT32 uwRet;
#ifdef LOSCFG_FS_VFS
    extern void los_vfs_init(void);
    los_vfs_init();
#endif
#ifdef LOSCFG_COMPAT_LINUX
#if (defined LOSCFG_PLATFORM_HI3519V101 && defined LOSCFG_ARCH_CORTEX_A17) || \
    (defined LOSCFG_PLATFORM_HI3519 && defined LOSCFG_ARCH_CORTEX_A17) || \
    (defined LOSCFG_PLATFORM_HI3559 && defined LOSCFG_ARCH_CORTEX_A17) || \
    (defined LOSCFG_PLATFORM_HI3731) || \
    (defined LOSCFG_PLATFORM_HI3911) || \
    (defined LOSCFG_PLATFORM_HIM5V100) || \
    (defined LOSCFG_ARCH_CORTEX_M7) || \
    (defined LOSCFG_ARCH_CORTEX_A53_AARCH64)
#else
    uwRet = hrtimers_init();
    if (uwRet != LOS_OK)
    {
        PRINT_ERR("Hrtimers_init error\n");
        return uwRet;
    }
#endif
    g_pstSystemWq = create_workqueue("system_wq");
#endif
    memset(&stappTask, 0, sizeof(TSK_INIT_PARAM_S));
    stappTask.pfnTaskEntry = (TSK_ENTRY_FUNC)app_init;
    stappTask.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stappTask.pcName = "app_Task";
    stappTask.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    stappTask.uwResved = LOS_TASK_STATUS_DETACHED;
    uwRet = LOS_TaskCreate(&ipctaskid, &stappTask);
    PRINTK("osAppInit\n");
    if (LOS_OK != uwRet) {
        return uwRet;
    }
#ifdef LOSCFG_MEM_RECORDINFO
    extern void osMemRecordShowTask(void);
    memset(&stappTask, 0, sizeof(TSK_INIT_PARAM_S));
    stappTask.pfnTaskEntry = (TSK_ENTRY_FUNC)osMemRecordShowTask;
    stappTask.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stappTask.pcName = "memshow_Task";
    stappTask.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    stappTask.uwResved = LOS_TASK_STATUS_DETACHED;
    uwRet = LOS_TaskCreate(&ipctaskid, &stappTask);
    if (LOS_OK != uwRet) {
        PRINTK("create memshow_Task error %d\n",uwRet);
        return uwRet;
    }
    PRINTK("create memshow_Task ok\n");
#endif
#ifdef LOSCFG_KERNEL_TICKLESS
    LOS_TicklessEnable();
#endif

#endif /* LOSCFG_PLATFORM_OSAPPINIT */

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

