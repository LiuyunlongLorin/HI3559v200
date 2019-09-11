/* mmc_os_adapt.h begin*/
#ifndef __MMC_OS_ADAPT_H__
#define __MMC_OS_ADAPT_H__

#define _DEF_LITEOS_ADAPT_

#ifdef _DEF_LITEOS_ADAPT_
#include "stdio.h"
#include "los_mux.h"
#include "los_event.h"
#include "asm/io.h"
#include "asm/errno.h"
#include "linux/scatterlist.h"
#include "linux/interrupt.h"
#include "disk.h"

#endif /*_DEF_LITEOS_ADAPT_*/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#ifdef _DEF_LITEOS_ADAPT_

/* define atomic ops */
#ifdef __LP64__
#define mmc_atomic_t long
#else
#define mmc_atomic_t int
#endif
#define mmc_atomic_inc(atom)    LOS_AtomicInc(atom)
#define mmc_atomic_dec(atom)    LOS_AtomicDec(atom)
#define mmc_atomic_read(atom)   (*((volatile typeof(atom))(atom)))
#define MMC_ATOMIC_INIT(x)      (x)

/* define mutex lock */
typedef UINT32 mmc_mutex;
#define MMC_MUTEX_WAIT_FOREVER LOS_WAIT_FOREVER
#define MMC_MUTEX_WAIT_DEF_TIME LOSCFG_BASE_CORE_TICK_PER_SECOND*10
#define MMC_MUTEX_RETURN_TIMEOUT LOS_ERRNO_MUX_TIMEOUT
#define mmc_mutex_init(mutex)   LOS_MuxCreate(mutex)
#define mmc_mutex_lock(mutex, timeout)   LOS_MuxPend(mutex, timeout)
#define mmc_mutex_unlock(mutex) LOS_MuxPost(mutex)
#define mmc_mutex_delete(mutex) LOS_MuxDelete(mutex)

/* define event lock */
typedef EVENT_CB_S mmc_event;
#define MMC_EVENT_WAIT_FOREVER LOS_WAIT_FOREVER
#define MMC_EVENT_WAIT_DEF_TIME (LOSCFG_BASE_CORE_TICK_PER_SECOND * 30) //LOS_WAIT_FOREVER also suitable
#define mmc_event_init(event)   LOS_EventInit(event)
#define mmc_event_signal(event, bit)    LOS_EventWrite(event, bit)
#define mmc_event_wait(event, bit, timeout)  LOS_EventRead(event, bit,\
                                            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, timeout)
#define mmc_event_clear(event, bit)     LOS_EventClear(event, ~bit)
/* here's warning, LOS_EventDestroy isn't declaring in los_event.h */
extern UINT32 LOS_EventDestroy(EVENT_CB_S * pstEventCB);
#define mmc_event_delete(event)     LOS_EventDestroy(event)

/* define task/irq lock */
#define mmc_task_lock(lock)    do { LOS_TaskLock( );} while(0)
#define mmc_task_unlock(lock)    do { LOS_TaskUnlock( );} while(0)

#define mmc_irq_lock(flags)  do { (*(flags)) = LOS_IntLock();} while(0)
#define mmc_irq_unlock(flags)  do { LOS_IntRestore(flags);} while(0)

/* define timer */
typedef UINT32 mmc_timer;
#define mmc_timer_create(Interval,mode, handler, pSwtmrID, arg) LOS_SwtmrCreate(Interval, mode, \
                                                    handler, pSwtmrID, arg)
#define mmc_timer_delete(SwtmrID) LOS_SwtmrDelete(SwtmrID)
#define mmc_timer_start(SwtmrID) LOS_SwtmrStart(SwtmrID)
#define mmc_timer_stop(SwtmrID) LOS_SwtmrStop(SwtmrID)
/* define Sem */
typedef UINT32 mmc_sem;
#define MMC_SEM_WAIT_FOREVER LOS_WAIT_FOREVER
#define mmc_sem_create(sem_id)  LOS_SemCreate(1, sem_id)
#define mmc_sem_post(sem_id)  LOS_SemPost(sem_id)
#define mmc_sem_pend(sem_id, timeout)  LOS_SemPend(sem_id, timeout)
#define mmc_sem_delete(sem_id)   LOS_SemDelete(sem_id)

#define mmc_acquire_card(card)\
({ \
 int ret = 0; \
 ret = mmc_mutex_lock(card->lock, MMC_MUTEX_WAIT_FOREVER);\
 if (ret != 0) {\
     mmc_err("acquire card fail! ret = %d\n", ret); \
     } \
  card->claimed = 1; \
  card->claimer = g_stLosTask.pstRunTask->uwTaskID; \
  card->claim_cnt += 1; \
 })

#define mmc_release_card(card)\
({ \
  int ret = 0;\
  card->claimed = 0; \
  card->claimer = 0xFFFFFFFF; \
  card->claim_cnt--;\
  ret = mmc_mutex_unlock(card->lock);\
  if (ret != 0) {\
     mmc_err("release card fail!,ret = %d\n", ret);\
  }\
  })

/* define create thread */
typedef unsigned int mmc_thread;
#define mmc_thread_create(prio, func, stack, arg, name, id) \
    ({  \
     TSK_INIT_PARAM_S stSdTask;  \
     UINT32 uwRet;   \
     memset(&stSdTask, 0, sizeof(TSK_INIT_PARAM_S)); \
     stSdTask.pfnTaskEntry = (TSK_ENTRY_FUNC)func;   \
     stSdTask.auwArgs[0] = (AARCHPTR)arg;  \
     stSdTask.uwStackSize  = stack; \
     stSdTask.pcName = name; \
     stSdTask.usTaskPrio = prio;    \
     stSdTask.uwResved   = LOS_TASK_STATUS_DETACHED; \
     uwRet = LOS_TaskCreate(id, &stSdTask); \
     uwRet; \
     })

#define mmc_thread_delete(id) ({ \
    LOS_TaskDelete(id); \
})

#define mmc_thread_suspend(id) ({ \
    LOS_TaskSuspend(id); \
})

#define mmc_thread_resume(id) ({ \
    LOS_TaskResume(id); \
})

/* define delay */
extern VOID hal_delay_us(UINT32 usecs);
extern void msleep(unsigned int msecs);
#define mmc_sleep_ms(ms)     msleep(ms)
#define mmc_get_sys_ticks    LOS_TickCountGet
#define  mmc_delay_us(us)   hal_delay_us(us)
#define  mmc_delay_ms(ms)   do { \
    uint32_t tick_delay = ((ms)/(1000/LOSCFG_BASE_CORE_TICK_PER_SECOND)); \
    uint32_t rest_delay = (ms)%(1000/LOSCFG_BASE_CORE_TICK_PER_SECOND); \
    if (tick_delay) \
        mmc_sleep_ms(tick_delay *(1000/LOSCFG_BASE_CORE_TICK_PER_SECOND)); \
    if (rest_delay) { \
        mmc_delay_us(rest_delay*1000); \
    } \
}while(0)

#define mmc_thread_sched() do { \
    UINTPTR uvIntSave; \
    uvIntSave = LOS_IntLock(); \
    LOS_Schedule(); \
    (VOID)LOS_IntRestore(uvIntSave); \
}while(0)

/* messages print definitions, for debug ,err e.g.*/

#ifdef LOSCFG_DEBUG_VERSION
    #define MMC_ASSERT
    #define MMC_TRACE
    #define MMC_ERR
    #define MMC_PRINTF
#else
    #undef MMC_ASSERT
    #undef MMC_TRACE
    #undef MMC_ERR
    #undef MMC_PRINTF
#endif

/* mmc assert */
#ifdef MMC_ASSERT
    #define mmc_assert(cond) do { \
        if (!(cond)) {\
            dprintf("Assert,mmc:%s:%d\n", \
                    __func__, \
                    __LINE__); \
            while(1); \
        } \
    } while (0)
#else
    #define mmc_assert(cond)  do { } while(0)
#endif /* MMC_ASSERT */

/* BUG ON must print */
#define MMC_BUG_ON()    do { \
    dprintf("%s:%d: ", __func__, __LINE__); \
    osBackTrace(); \
    while(1);\
} while(0)

/*mmc trace print*/
#define MMC_TRACE_INFO      0
#define MMC_NOTICE          1
#define MMC_TRACE_WARNING   2
#define MMC_TRACE_DEBUG     3
#define MMC_TRACE_ERR       6

#define MMC_TRACE_LEVEL     6

#ifdef MMC_TRACE
    #define mmc_trace(level, msg...) do { \
        if ((level) >= MMC_TRACE_LEVEL) { \
            dprintf("%s:%d: ", __FUNCTION__, __LINE__); \
            dprintf(msg); \
            dprintf("\n"); \
        } \
    } while (0)
#else
    #define mmc_trace(level, msg...) do { }while(0)
#endif /* MMC_TRACE*/

/*mmc err print*/
#ifdef MMC_ERR
    #define mmc_err(msg...) do { \
        dprintf("%s:%d: ", __FUNCTION__, __LINE__);\
        dprintf(msg); \
        dprintf("\n"); \
    } while (0)
#else
    #define mmc_err(msg...) do { }while(0)
#endif /* MMC_ERR */

/* mmc printf */
#define MMC_PRINTF_INFO  0
#define MMC_PRINTF_WARN  1
#define MMC_PRINTF_ERR  2

#define MMC_PRINTF_LEVEL  0

#ifdef MMC_PRINTF
    #define mmc_printf(level, msg...) do { \
        if ((level) >= MMC_PRINTF_LEVEL) { \
            dprintf(msg); \
            dprintf("\n"); \
        } \
    } while (0)
#else
    #define mmc_printf(level, msg...) do { }while(0)
#endif /* MMC_PRINTF */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define sg_dma_address(sg)    ((sg)->dma_address)
#ifdef CONFIG_NEED_SG_DMA_LENGTH
#define sg_dma_len(sg)        ((sg)->dma_length)
#else
#define sg_dma_len(sg)        ((sg)->length)
#endif

static inline uint32_t  __swab32(uint32_t x)
{
    uint32_t __x = (x);
    return (uint32_t)(
            (((uint32_t)(__x) & (uint32_t)0x000000ffUL) << 24) |
            (((uint32_t)(__x) & (uint32_t)0x0000ff00UL) <<  8) |
            (((uint32_t)(__x) & (uint32_t)0x00ff0000UL) >>  8) |
            (((uint32_t)(__x) & (uint32_t)0xff000000UL) >> 24));
}
#ifndef le32_to_cpu
#define le32_to_cpu(x)  (__swab32(x))
#endif

#ifndef min
#define min(x, y) (x<y ? x : y)
#endif

#ifndef max
#define max(x, y) (x<y ? y : x)
#endif

/* for debug */
extern UINT32 cmd_himd(UINT32 argc, CHAR ** argv);
#define mmc_reg_print(addr, len) do { \
    char *str[2] = {NULL, NULL}; \
    str[0] = addr; \
    str[1] = len; \
    dprintf("\n\n");\
    dprintf("%s:%d: ", __func__, __LINE__);\
    cmd_himd(2, str);\
    dprintf("\n\n");\
}while(0)

/* #define MMC_POWERUP_TIME_CAL */

#ifdef MMC_POWERUP_TIME_CAL
#define mmc_powerup_cal() do { \
    extern unsigned long long  sd_time_s;\
    extern unsigned long long  sd_time_e;\
    extern unsigned long long hi_sched_clock(void);\
    sd_time_e = hi_sched_clock();\
    mmc_trace(5, "\npf: %d\n", (unsigned long)((sd_time_e - sd_time_s)/1000));\
}while(0)
#else
#define mmc_powerup_cal() do { }while(0)
#endif

/* extern functions declaration*/
extern void *malloc(size_t size);
extern void * memalign (size_t boundary, size_t size);
extern void *memset(void *dst, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void free(void *ptr);
extern int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
        const char *name, void *dev);
extern int register_driver(const char *path, const struct file_operations_vfs *fops,
                    mode_t mode, void *priv);

#endif /*_DEF_LITEOS_ADAPT_*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_OS_ADAPT_H */
