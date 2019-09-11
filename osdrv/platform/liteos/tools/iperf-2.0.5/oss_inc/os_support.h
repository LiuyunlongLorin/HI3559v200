/******************************************************************************
 ** embWiSe Technologies                                                     **
 **--------------------------------------------------------------------------**
 ** COPYRIGHT INFORMATION                                                    **
 **   This program contains proprietary information which is a trade         **
 **   secret of embWiSe Technologies and also is protected as an unpublished **
 **   work under applicable Copyright laws. Recipient is to retain           **
 **   this program in confidence and is not permitted to use or make copies  **
 **   thereof other than as permitted in a written agreement with            **
 **   embWiSe Technologies                                                   **
 **--------------------------------------------------------------------------**
 ** Project       : WLAN                                                     **
 ** Module        : OS Abstraction Layer                                     **
 ** File name     : os_support.h                                             **
 ** Authors       : Gangadharan, Alagu Sankar                                **
 ** Created On    : Nov 30 2014                                              **
 ** Reference(s)  : MQX Reference and User Guide                             **
 ** Description   : Operating system abstraction for MQX                     **
 **--------------------------------------------------------------------------**
 ** History                                                                  **
 ** Date          Revision  by               Comment                 **
 **--------------------------------------------------------------------------**
 ** Aug 07 2014    1.0      Alagu Sankar     File Created                    **
 ******************************************************************************/

#ifndef _OS_SUPPORT_H_
#define _OS_SUPPORT_H_

#include <oss_config.h>

//#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
//#include <cyg/hal/hal_if.h>
//#include <cyg/hal/hal_cache.h>
//#include <cyg/kernel/kapi.h>
#include "stdio.h"
//#include <cyg/infra/diag.h>
//#include <pkgconf/memalloc.h>
//#include <pkgconf/kernel.h>
#include <oss_string.h>
#include <wlan_oss_config.h>
#include <linux/kernel.h>
#include "los_mux.h"
#include "los_sem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OSS_MEMPOOL_SIZE    (0x200000)

#ifndef OSS_TICKS_PER_SEC
//#define OSS_TICKS_PER_SEC  ((1000 * 1000 * 1000) /
//        (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR))
#define OSS_TICKS_PER_SEC LOSCFG_BASE_CORE_TICK_PER_SECOND
#endif

#define TICKS_TO_MSEC(ticks)            (((ticks) * 1000) / OSS_TICKS_PER_SEC)
#define MSEC_TO_TICKS(ms)               (((ms) * OSS_TICKS_PER_SEC) / 1000)

/************************************************************************
 *                       constants                                      *
 ************************************************************************/
#define OSS_WAIT_FOREVER            (0xffffffff)
#define OSS_NO_WAIT                 (0)

#define OSS_NAME_SIZE               (64)
#define FILE_NAME_SIZE              (64)

#define OSS_SUCCESS                 (0)
#define OSS_FAILURE                 (-1)
#define OSS_INVALID_ARG             (-2)
#define OSS_EVENT_TIMEOUT           (-3)
#define OSS_NULL_PTR                (-4)

#define OSS_SEM_COUNT_DEFAULT           (1)
#define OSS_STACK_SIZE_DEFAULT          (LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE / 2)    /* 8 Kbytes */
#define MAIN_TIMER_TASK_STK_SIZE        (OSS_STACK_SIZE_DEFAULT)
#define OSS_TIMER_TASK_STK_SIZE         (OSS_STACK_SIZE_DEFAULT)
#define HIF_IRQ_TASK_STACK_SIZE         (OSS_STACK_SIZE_DEFAULT)
#define HIF_ASYNC_TASK_STACK_SIZE       (OSS_STACK_SIZE_DEFAULT)
#define BUNDLING_TASK_STACK_SIZE        (OSS_STACK_SIZE_DEFAULT)
#define WPA_TASK_STK_SIZE               (OSS_STACK_SIZE_DEFAULT * 2)
#define WPA_CLI_TASK_STK_SIZE           (OSS_STACK_SIZE_DEFAULT * 2)

/* Lower the number, higher the priority */
#define OSS_THREAD_PRIORITY_DEFAULT     (5)
#define MAIN_TIMER_TASK_PRIORITY        (OSS_THREAD_PRIORITY_DEFAULT - 1)
#define OSS_TIMER_TASK_PRIORITY         (OSS_THREAD_PRIORITY_DEFAULT - 2)
#define OSS_INT_THREAD_PRIORITY         (OSS_THREAD_PRIORITY_DEFAULT - 3)
#define HIF_IRQ_TASK_PRIORITY           (OSS_THREAD_PRIORITY_DEFAULT - 3)
#define HIF_ASYNC_TASK_PRIORITY         (OSS_THREAD_PRIORITY_DEFAULT - 2)
#define TX_BUNDLING_TASK_PRIORITY       (OSS_THREAD_PRIORITY_DEFAULT - 2)
#define WPA_SUPPLICANT_PRIORITY         (OSS_THREAD_PRIORITY_DEFAULT - 1)
#define WPA_CLI_TASK_PRIORITY           (OSS_THREAD_PRIORITY_DEFAULT - 1)

#define OSS_ALIGN_ADDR(addr, size)      (((unsigned long)(addr) +   \
            (size) - 1) & ~(size - 1))
#define HAL_DCACHE_LINE_SIZE            32
#define OSS_CACHE_LINE_SIZE             HAL_DCACHE_LINE_SIZE

#define oss_min(a, b)                   (((a) < (b)) ? (a) : (b))
#define oss_max(a, b)                   (((a) > (b)) ? (a) : (b))

#define BIT(n)                          (1U << (n))
#define oss_set_bit(addr, bit)          oss_set_bit_long(addr, bit)
#define oss_test_bit(addr, bit)         oss_test_bit_long(addr, bit)
#define oss_clear_bit(addr, bit)        oss_clear_bit_long(addr, bit)

#define oss_swab16(x)                                                       \
    ((T_U16)(                                                               \
            (((T_U16)(x) & (T_U16)0x00ffU) << 8) |                          \
            (((T_U16)(x) & (T_U16)0xff00U) >> 8) ))

#define oss_swab32(x)                                                       \
    ((T_U32)(                                                               \
            (((T_U32)(x) & (T_U32)0x000000ffUL) << 24) |                    \
            (((T_U32)(x) & (T_U32)0x0000ff00UL) <<  8) |                    \
            (((T_U32)(x) & (T_U32)0x00ff0000UL) >>  8) |                    \
            (((T_U32)(x) & (T_U32)0xff000000UL) >> 24) ))

#define oss_put_le32(buf, val)                                              \
    do {                                                                    \
        (buf)[3] = (T_U8)((((T_U32)(val)) >> 24) & 0xff);                   \
        (buf)[2] = (T_U8)((((T_U32)(val)) >> 16) & 0xff);                   \
        (buf)[1] = (T_U8)((((T_U32)(val)) >> 8) & 0xff);                    \
        (buf)[0] = (T_U8)(((T_U32)(val)) & 0xff);                           \
    }while(0)

#define oss_put_le16(buf, val)                                              \
    do {                                                                    \
        (buf)[1] = (T_U8)((((T_U32)(val)) >> 8) & 0xff);                    \
        (buf)[0] = (T_U8)(((T_U32)(val)) & 0xff);                           \
    }while(0)


#define oss_put_be24(buf, val)                                              \
    do {                                                                    \
        (buf)[0] = ((T_U32)(val) >> 16) & 0xff;                             \
        (buf)[1] = ((T_U32)(val) >> 8) & 0xff;                              \
        (buf)[2] = (T_U32)(val) & 0xff;                                     \
    }while(0)

#define oss_put_be32(buf, val)                                              \
    do {                                                                    \
        (buf)[0] = (T_U8)((((T_U32)(val)) >> 24) & 0xff);                   \
        (buf)[1] = (T_U8)((((T_U32)(val)) >> 16) & 0xff);                   \
        (buf)[2] = (T_U8)((((T_U32)(val)) >> 8) & 0xff);                    \
        (buf)[3] = (T_U8)(((T_U32)(val)) & 0xff);                           \
    }while(0)

#define oss_put_be16(buf, val)                                              \
    do {                                                                    \
        (buf)[0] = (T_U8)((((T_U32)(val)) >> 8) & 0xff);                    \
        (buf)[1] = (T_U8)(((T_U32)(val)) & 0xff);                           \
    }while(0)

#define oss_put_be64(buf, val)                                                \
    do {                                                                    \
        (buf)[0] = (T_U8) (((T_U64) (val)) >> 56);                            \
        (buf)[1] = (T_U8) (((T_U64) (val)) >> 48);                            \
        (buf)[2] = (T_U8) (((T_U64) (val)) >> 40);                            \
        (buf)[3] = (T_U8) (((T_U64) (val)) >> 32);                            \
        (buf)[4] = (T_U8) (((T_U64) (val)) >> 24);                            \
        (buf)[5] = (T_U8) (((T_U64) (val)) >> 16);                            \
        (buf)[6] = (T_U8) (((T_U64) (val)) >> 8);                            \
        (buf)[7] = (T_U8) (((T_U64) (val)) & 0xff);                            \
    } while (0)

#define oss_put_le64(buf, val)                                                \
    do {                                                                    \
        (buf)[7] = (T_U8) (((T_U64) (val)) >> 56);                            \
        (buf)[6] = (T_U8) (((T_U64) (val)) >> 48);                            \
        (buf)[5] = (T_U8) (((T_U64) (val)) >> 40);                            \
        (buf)[4] = (T_U8) (((T_U64) (val)) >> 32);                            \
        (buf)[3] = (T_U8) (((T_U64) (val)) >> 24);                            \
        (buf)[2] = (T_U8) (((T_U64) (val)) >> 16);                            \
        (buf)[1] = (T_U8) (((T_U64) (val)) >> 8);                            \
        (buf)[0] = (T_U8) (((T_U64) (val)) & 0xff);                            \
    } while (0)

#define oss_get_be64(buf)   ((((T_U64) (buf)[0]) << 56) |                   \
                            (((T_U64) (buf)[1]) << 48) |                    \
                            (((T_U64) (buf)[2]) << 40) |                    \
                            (((T_U64) (buf)[3]) << 32) |                    \
                            (((T_U64) (buf)[4]) << 24) |                    \
                            (((T_U64) (buf)[5]) << 16) |                    \
                            (((T_U64) (buf)[6]) << 8) |                     \
                            ((T_U64) (buf)[7]))

#define oss_get_le64(buf)   ((((T_U64) (buf)[7]) << 56) |                   \
                            (((T_U64) (buf)[6]) << 48) |                    \
                            (((T_U64) (buf)[5]) << 40) |                    \
                            (((T_U64) (buf)[4]) << 32) |                    \
                            (((T_U64) (buf)[3]) << 24) |                    \
                            (((T_U64) (buf)[2]) << 16) |                    \
                            (((T_U64) (buf)[1]) << 8) |                     \
                            ((T_U64) (buf)[0]))
#define oss_get_le32(buf)   ((((T_U32) (buf)[3]) << 24) |                   \
                            (((T_U32) (buf)[2]) << 16) |                    \
                            (((T_U32) (buf)[1]) << 8) |                     \
                            ((T_U32) (buf)[0]))

#define oss_get_be32(buf)   ((((T_U32) (buf)[0]) << 24) |                   \
                            (((T_U32) (buf)[1]) << 16) |                    \
                            (((T_U32) (buf)[2]) << 8) |                     \
                            ((T_U32) (buf)[3]))

#define oss_get_be24(buf)   ((((T_U32)(buf)[0]) << 16) |                    \
                            ((T_U32)((buf)[1]) << 8) |                      \
                            ((T_U32)(buf)[2]))

#define oss_get_le16(buf)   ((T_U16)((((T_U16) (buf)[1]) << 8) |            \
                            ((T_U16) (buf)[0])))

#define oss_get_be16(buf)   ((T_U16)((((T_U16)(buf)[0]) << 8) |          \
                            ((T_U16)(buf)[1])))

#if OSS_BIG_ENDIAN
#define oss_le2cpu_16(x)        oss_swab16(x)
#define oss_le2cpu_32(x)        oss_swab32(x)
#define oss_cpu2le_16(x)        oss_swab16(x)
#define oss_cpu2le_32(x)        oss_swab32(x)

#define oss_be2cpu_16(x)        (x)
#define oss_be2cpu_32(x)        (x)
#define oss_cpu2be_16(x)        (x)
#define oss_cpu2be_32(x)        (x)

#else

#define oss_be2cpu_16(x)        oss_swab16(x)
#define oss_be2cpu_32(x)        oss_swab32(x)
#define oss_cpu2be_16(x)        oss_swab16(x)
#define oss_cpu2be_32(x)        oss_swab32(x)

#define oss_le2cpu_16(x)        (x)
#define oss_le2cpu_32(x)        (x)
#define oss_cpu2le_16(x)        (x)
#define oss_cpu2le_32(x)        (x)

#endif


/************************************************************************
 *                       Type Definitions                               *
 ************************************************************************/
typedef signed char         T_S8,    *PT_S8;
typedef signed short        T_S16,   *PT_S16;
typedef signed int          T_S32,   *PT_S32;
typedef signed long long    T_S64,   *PT_S64;
typedef unsigned char       T_U8,    *PT_U8;
typedef unsigned short      T_U16,   *PT_U16;
typedef unsigned int        T_U32,   *PT_U32;
typedef unsigned long long  T_U64,   *PT_U64;
typedef unsigned long       T_ULONG, *PT_ULONG;

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;

typedef unsigned long oss_time_t;

typedef struct oss_list
{
    void *pcontext;
    struct oss_list *pnext;
    struct oss_list* pprev;
}OSS_LIST, *POSS_LIST;

typedef struct oss_time
{
    unsigned long sec;
    unsigned long usec;
}OSS_TIME, *POSS_TIME;

typedef struct oss_mutex
{
    UINT32 mutex_id;
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
}OSS_MUTEX, *POSS_MUTEX;

typedef struct oss_semaphore
{
    UINT32 sem_id;
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
}OSS_SEMAPHORE, *POSS_SEMAPHORE;

typedef struct oss_lock
{
    spinlock_t spinlock_id;
    UINT32 istate;
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
}OSS_LOCK, *POSS_LOCK;

typedef struct oss_lock OSS_IRQ_LOCK, *POSS_IRQ_LOCK;
typedef struct oss_lock OSS_SPINLOCK, *POSS_SPINLOCK;

typedef struct oss_event
{
    EVENT_CB_S levent;
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
}OSS_EVENT, *POSS_EVENT;

typedef struct _que_list
{
    char *bufferAddress;
    unsigned long length;
}OSS_MSG_QUE_LIST, *POSS_MSG_QUE_LIST;

typedef struct oss_msg_que
{
    char name[OSS_NAME_SIZE];
    int maxQueSize;
    int readQueIndex;
    int writeQueIndex;
    OSS_EVENT pMsgQueEvent;
    POSS_MSG_QUE_LIST msgQueList;
    int inUse;
    unsigned char debug_flag;
}OSS_MSG_QUE, *POSS_MSG_QUE;

typedef struct oss_timer
{
    int periodic;
#define OSS_TIMER_ACTIVATED             (1 << 0)
    unsigned int flags;
    unsigned long timer_id;
    unsigned int timeout_ms;
    OSS_TIME expiry;
    void *pcontext;             // Argument to routine
    void (*pfunc)(void *);  // Routine
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
    OSS_LIST timer_list;
}OSS_TIMER, *POSS_TIMER;

typedef struct oss_thread
{
    OSS_EVENT task_event;
    void (*pfunc)(void *);
    void *parg;
    UINT32 thread_id;
    LOS_TASK_CB*   t_info;
    unsigned int priority;
    unsigned int stack_size;
    void *pstack;
    char name[OSS_NAME_SIZE];
    unsigned char debug_flag;
}OSS_THREAD, *POSS_THREAD;

struct oss_tm {
    int sec; /* 0..59 or 60 for leap seconds */
    int min; /* 0..59 */
    int hour; /* 0..23 */
    int day; /* 1..31 */
    int month; /* 1..12 */
    int year; /* Four digit year */
};


/*
 * File System Wrappers
 */
#define OSS_SEEK_SET                1
#define OSS_SEEK_CUR                2
#define OSS_SEEK_END                3

typedef struct _OSS_FILE_ROOT {
    struct oss_list flist;
    const unsigned char *pfdata;
    unsigned int fsize;
    unsigned int fflags;
    unsigned int use_count;
    char fname[FILE_NAME_SIZE];
}OSS_FILE_ROOT;

typedef struct _OSS_FILE {
    unsigned int offset;
    unsigned int fflags;
    const unsigned char *pfdata;
    char *fname;
    void *proot;
}OSS_FILE;

/************************************************************************
 *                       Exported Functions                             *
 ************************************************************************/
void setup_ossupport_layer(void);
void delete_ossupport_layer(void);
int oss_setup_timer(void);
void oss_cleanup_timer(void);

/*****************************************
 *      Mutex related Apis               *
 *****************************************/
#define oss_is_mutex_valid(plock)       1
int oss_mutex_init(POSS_MUTEX plock, const char *name);
int oss_mutex_lock(POSS_MUTEX plock);
int oss_mutex_unlock(POSS_MUTEX plock);
int oss_mutex_delete(POSS_MUTEX plock);
void oss_mutex_set_debug(POSS_MUTEX plock, unsigned char flag);

/*****************************************
 *      InterruptLock related Apis       *
 *****************************************/
void oss_irq_lock_init(POSS_IRQ_LOCK, const char*);
void oss_irq_lock_acquire(POSS_IRQ_LOCK);
void oss_irq_lock_release(POSS_IRQ_LOCK);
void oss_irq_lock_delete(POSS_IRQ_LOCK);
void oss_enter_critical(void);
void oss_exit_critical(void);
unsigned int oss_atomic_read(void *addr);

/*****************************************
 *      SpinLock related Apis               *
 *****************************************/
int oss_spin_lock_init(POSS_SPINLOCK plock, const char *name);
int oss_spin_lock(POSS_SPINLOCK lock);
int oss_spin_unlock(POSS_SPINLOCK);
int oss_spin_lock_delete(POSS_SPINLOCK);
void oss_spin_lock_set_debug(POSS_SPINLOCK, unsigned char);

/*****************************************
 *      Event related Apis               *
 *****************************************/
int oss_event_init(POSS_EVENT pevent, const char *name);
int oss_event_wait_flag_get(POSS_EVENT pevent, unsigned long flags_in,
        unsigned long time_ms, unsigned long *flags_out);
int oss_event_wakeup_flag(POSS_EVENT pevent, unsigned long flags);
int oss_event_wakeup_isr_flag(POSS_EVENT pevent, unsigned long flags);
int oss_event_delete(POSS_EVENT pevent);
int oss_event_reset_flag(POSS_EVENT pevent, unsigned long flags);
int oss_event_reset_isr(POSS_EVENT pevent);
void oss_event_set_debug(POSS_EVENT pevent, unsigned char debug_flag);

#define oss_event_wait(pevent, timeout)                                     \
                oss_event_wait_flag_get(pevent, BIT(0), timeout, NULL)
#define oss_event_wait_flag(pevent, flag, timeout)                          \
                oss_event_wait_flag_get(pevent, flag, timeout, NULL)
#define oss_event_wakeup(pevent)                                            \
                oss_event_wakeup_flag(pevent, BIT(0))
#define oss_event_wakeup_isr(pevent)                                        \
                oss_event_wakeup_isr_flag(pevent, BIT(0))
#define oss_event_reset(pevent)                                             \
                oss_event_reset_flag(pevent, BIT(0))

/*****************************************
 *      Message Queues related Apis      *
 *****************************************/
int oss_msg_que_init(POSS_MSG_QUE, unsigned long , char *);
int oss_msg_que_delete(POSS_MSG_QUE pMsgQue);
int oss_msg_que_check(POSS_MSG_QUE );
int oss_msg_que_write(POSS_MSG_QUE pMsgQue, void *bufPtr, unsigned long length);
POSS_MSG_QUE_LIST oss_msg_que_pend(POSS_MSG_QUE);
POSS_MSG_QUE_LIST oss_msg_que_pendTimeOut(POSS_MSG_QUE, long);
POSS_MSG_QUE_LIST oss_msg_que_read(POSS_MSG_QUE);
int oss_msg_que_post(POSS_MSG_QUE, void *, unsigned long);
int oss_msg_que_status(POSS_MSG_QUE pMsgQue);
void oss_msg_que_set_debug(POSS_MSG_QUE pMsgQue, unsigned char flag);

/*****************************************
 *      Semaphore related Apis               *
 *****************************************/
int oss_sem_init(POSS_SEMAPHORE psem, int count, const char *name);
int oss_sem_acquire(POSS_SEMAPHORE psem);
int oss_sem_release(POSS_SEMAPHORE psem);
int oss_sem_delete(POSS_SEMAPHORE psem);
void oss_sem_set_debug(POSS_SEMAPHORE psem, unsigned char debug_flag);

/*****************************************
 *        Thread related Apis            *
 *****************************************/
typedef void (*os_thread_func)(void *);
int oss_thread_init(POSS_THREAD pthread, char *name, int priority,
        os_thread_func pfunc, void *parg, int stack_size);
int oss_thread_start(POSS_THREAD pthread);
int oss_thread_delete(POSS_THREAD pthread);
void oss_thread_set_debug(POSS_THREAD pthread, unsigned char debug_flag);
void show_threads(int argc, char* argv[]);

/*****************************************
 *       Timer related Apis              *
 *****************************************/
typedef void (*os_timer_callback)(void *);
void oss_timer_thread(void*);
void oss_main_timer_thread(void*);
int oss_timer_init(POSS_TIMER, char*, void*, void*);
int oss_timer_start(POSS_TIMER, unsigned long, int periodic);
int oss_timer_cancel(POSS_TIMER);
int oss_timer_delete(POSS_TIMER);
unsigned long oss_timeout_timer(os_timer_callback, void*, unsigned long);
unsigned long oss_repeat_timer(os_timer_callback, void*, unsigned long);
void oss_timer_set_debug(POSS_TIMER, unsigned char);
unsigned long oss_timer_add(os_timer_callback pFunc, void *pArg,
        unsigned long time, unsigned long periodic);

/*****************************************
 *          Bit Operations                *
 *****************************************/
void oss_set_bit_long(unsigned long *, unsigned char);
void oss_clear_bit_long(unsigned long *, unsigned char);
unsigned long oss_test_bit_long(unsigned long *, unsigned char);
unsigned long oss_test_and_clear_bit_long(unsigned long *, unsigned char);

/*****************************************
 *       Time related Apis              *
 *****************************************/
int oss_get_time(POSS_TIME);
unsigned long oss_get_ticks(void);
unsigned long oss_get_time_ms(void);
int oss_get_time_difference(POSS_TIME, POSS_TIME);
unsigned long oss_convert_ticks(unsigned int secs, unsigned int usecs);
void oss_convert_timeval(unsigned int ticks, unsigned long *secs,
        unsigned long *usecs);
void oss_time_add(OSS_TIME *dtime, OSS_TIME *stime, unsigned int timeout_ms);
#define oss_get_time_diff_ms(x, y)  oss_get_time_difference(x, y)

/*****************************************
 *        Memory related Apis            *
 *****************************************/
void *oss_malloc(int size);
void *oss_zalloc(int size);
void *oss_calloc(int size);
void *oss_realloc(void *pmem, int size);
void oss_free(void *pmem);
void display_resource_usage(void);
void oss_print_mem_table(void);
void reset_os_counters(void);
unsigned long oss_get_total_mem_allocated(void);
unsigned long oss_get_total_mem_count(void);
int oss_hex2byte(char *hex);
unsigned long oss_copy_from_user(void *to, void *from, unsigned long n);

#define oss_memfree(ptr)                oss_free(ptr)

/*****************************************
 *         List related Apis            *
 *****************************************/
void oss_list_init(POSS_LIST);
int oss_list_empty(POSS_LIST);
void oss_list_add(POSS_LIST phead, POSS_LIST pnode);
void oss_list_add_tail(POSS_LIST phead, POSS_LIST pnode);
OSS_LIST *oss_list_del(POSS_LIST phead);
OSS_LIST *oss_list_del_tail(POSS_LIST phead);

#define oss_get_container(plist_addr, base_struct, plist_name)              \
    ((base_struct *)((unsigned long)(plist_addr) -                          \
        (unsigned long)(&((base_struct *)0)->plist_name)))

#define oss_list_first_entry(head, type, field)                             \
    oss_list_entry((head)->pnext, type, field)

#define oss_list_for_each(node, head)                                       \
    for (node = (head)->pnext; node != (head); node = node->pnext)

#define oss_list_for_each_safe(node, tmp, head)                             \
    for (node = (head)->pnext, tmp = node->pnext;                           \
            node != (head); node = tmp, tmp = node->pnext)

#define oss_list_for_each_reverse(node, head)                               \
    for (node = (head)->pprev; node != (head); node = node->pprev)

/*
 * oss_list_entry - get the struct for this entry
 * head:          the &struct list_head pointer.
 * type:           the type of the struct this is embedded in.
 * member:         the name of the list_struct within the struct.
 */
#define oss_list_entry(head, type, member)                                 \
    ((type *)((char *)(head) - (unsigned long)(&((type *)0)->member)))

/**
 * oss_list_for_each_entry  -   iterate over list of given type
 * @entry:                      the type * to use as a loop counter.
 * @head:                       the head for your list.
 * @field:                      the name of the list_struct within the struct.
 */
#define oss_list_for_each_entry(entry, head, type, field)                 \
    for (entry = oss_list_entry((head)->pnext, type, field);              \
            &entry->field != (head);                                      \
            entry = oss_list_entry(entry->field.pnext, type, field))

#define oss_list_for_each_entry_safe(node, tmp, head, type, field)         \
    for (node = oss_list_entry((head)->pnext, type, field),                \
            tmp = oss_list_entry(node->field.pnext, type, field);          \
            &node->field != (head);                                        \
            node = tmp, tmp = oss_list_entry(tmp->field.pnext, type, field))

#define oss_list_for_each_entry_reverse(node, head, type, field)           \
    for (node = oss_list_entry((head)->pprev, type, field);                \
            &node->field != (head);                                        \
            node = oss_list_entry(node->field.pprev, type, field))

/*****************************************
 *         File System Apis              *
 *****************************************/
void oss_file_system_init(void);
int oss_file_delete(OSS_FILE_ROOT *proot);
int oss_file_remove(OSS_FILE_ROOT *proot);
int oss_file_add(OSS_FILE_ROOT *proot);
int oss_file_exists(const char *fname);
OSS_FILE_ROOT *oss_file_create(char *fname, void *pfdata, int fsize);

OSS_FILE *oss_fopen(const char *path, const char *mode);
int oss_fclose(OSS_FILE *fptr);
int oss_fwrite(const void *fbuf, unsigned int size, unsigned int nmemb,
        OSS_FILE *fptr);
int oss_fread(void *fbuf, unsigned int size, unsigned int nmemb,
        OSS_FILE *fptr);
int oss_fseek(OSS_FILE *fptr, long offset, int whence);
int oss_ftell(OSS_FILE *fptr);

/*****************************************
 *         Delay related Apis             *
 *****************************************/
void oss_ssleep(unsigned long secs);
void oss_msleep(unsigned long msecs);
void oss_usleep(unsigned long usecs);
void oss_udelay(unsigned long usecs);
void oss_nsleep(unsigned long nsecs);
void oss_sleep(unsigned long secs, unsigned long usecs);

unsigned long oss_random(void);
int oss_get_random(unsigned char *buf, int length);
int oss_time_before(POSS_TIME a, POSS_TIME b);
void oss_time_sub(POSS_TIME a, POSS_TIME b, POSS_TIME res);
void oss_time_age(struct oss_time *start, struct oss_time *age);
int oss_time_expired(struct oss_time *now, struct oss_time *ts,
        long timeout_secs);
int oss_time_initialized(struct oss_time *t);

int oss_snprintf(char *str, unsigned long size, const char *format, ...);
int oss_program_init(void);
void oss_program_deinit(void);
int oss_setenv(char *name, char *value, int overwrite);
int oss_unsetenv(char *name);
int oss_daemonize(char *pid_file);
void oss_daemonize_terminate(char *pid_file);
char *oss_rel2abs_path(const char *rel_path);
char *oss_readfile(const char *name, unsigned long *len);

int oss_get_random(unsigned char *buf, int len);
char *oss_rel2abs_path(const char *rel_path);
unsigned long oss_random(void);

/*
 * Debug Message Support
 *
 * Print information that can be enabled/disabled at compile time and/or
 * at run time. By defining the exact OSS_DEBUG_LEVEL at run-time, lower
 * level debug messages can be compiled out to reduce code size.
 *
 * oss_printf will ALWAYS print the message
 */

/******************************************************************************
 * Name        : oss_printf                                                   *
 * Description : This function used to print the oss_support success messages.*
 * Parameters  : fmt -pointer to the string,format specifiers.                *
 * Return      : none                                                         *
 * Precondition: DEBUG_OSSUPPORT,EnableTrace should be enable to get the print*
 * Warning     :                                                              *
 * See also    :                                                              *
 ******************************************************************************/
#define oss_printf(...)                 diag_printf(__VA_ARGS__)
#define oss_gets(str)                   gets(str)
#define oss_printstr(str)               diag_printf(str)

#if DEBUG_OSSUPPORT

#define OSS_DEBUG_TIMER                     1
#define OSS_DEBUG_SPINLOCK                  1
#define OSS_DEBUG_MUTEX                     1
#define OSS_DEBUG_SEM                       1
#define OSS_DEBUG_EVENT                     1
#define OSS_DEBUG_MSGQUE                    1
#define OSS_DEBUG_THREAD                    1
#define OSS_MEMORY_DEBUG                    1
#define OSS_DEBUG_RESOURCES                 1
#define OSS_ENABLE_SANITY_CHECK             1

#else

/* Disable debug messages for all objects */
#define OSS_DEBUG_TIMER                     0
#define OSS_DEBUG_SPINLOCK                  0
#define OSS_DEBUG_MUTEX                     0
#define OSS_DEBUG_SEM                       0
#define OSS_DEBUG_EVENT                     0
#define OSS_DEBUG_MSGQUE                    0
#define OSS_DEBUG_THREAD                    0
#define OSS_MEMORY_DEBUG                    0
#define OSS_DEBUG_RESOURCES                 0
#define OSS_ENABLE_SANITY_CHECK             0

#endif

#if OSS_DEBUG_RESOURCES
struct rtos_resource
{
    unsigned long mutexCount;
    unsigned long spinLockCount;
    unsigned long eventCount;
    unsigned long irqLockCount;
    unsigned long semCount;
    unsigned long threadCount;
    unsigned long queueCount;
    unsigned long timerCount;
};

extern struct rtos_resource rrtos;

#define oss_inc_mutex_count()       rrtos.mutexCount++
#define oss_dec_mutex_count()       rrtos.mutexCount--
#define oss_inc_event_count()       rrtos.eventCount++
#define oss_dec_event_count()       rrtos.eventCount--
#define oss_inc_spinlock_count()    rrtos.spinLockCount++
#define oss_dec_spinlock_count()    rrtos.spinLockCount--
#define oss_inc_irqlock_count()     rrtos.irqLockCount++
#define oss_dec_irqlock_count()     rrtos.irqLockCount--
#define oss_inc_sem_count()         rrtos.semCount++
#define oss_dec_sem_count()         rrtos.semCount--
#define oss_inc_thread_count()      rrtos.threadCount++
#define oss_dec_thread_count()      rrtos.threadCount--
#define oss_inc_queue_count()       rrtos.queueCount++
#define oss_dec_queue_count()       rrtos.queueCount--
#define oss_inc_timer_count()       rrtos.timerCount++
#define oss_dec_timer_count()       rrtos.timerCount--

#else

#define oss_inc_mutex_count()
#define oss_dec_mutex_count()
#define oss_inc_event_count()
#define oss_dec_event_count()
#define oss_inc_spinlock_count()
#define oss_dec_spinlock_count()
#define oss_inc_irqlock_count()
#define oss_dec_irqlock_count()
#define oss_inc_sem_count()
#define oss_dec_sem_count()
#define oss_inc_thread_count()
#define oss_dec_thread_count()
#define oss_inc_queue_count()
#define oss_dec_queue_count()
#define oss_inc_timer_count()
#define oss_dec_timer_count()
#define reset_os_counters()
#define display_resource_usage()

#endif

#if OSS_DEBUG_MSGQUE
#define oss_debug_msgque(obj, ...)                                          \
    if ((obj)->debug_flag) {                                                \
        oss_printf(__VA_ARGS__);                                            \
    }
#else
#define oss_debug_msgque(obj, ...)
#endif

#if OSS_DEBUG_MUTEX
#define oss_debug_mutex(obj, fstr)                                          \
    if ((obj)->debug_flag) {                                                \
        oss_printf("Mutex %s "fstr, (obj)->name);                           \
    }
#else
#define oss_debug_mutex(obj, fstr)
#endif

#if OSS_DEBUG_SEM
#define oss_debug_sem(obj, fstr)                                            \
    if ((obj)->debug_flag) {                                                \
        oss_printf("Semaphore %s "fstr, (obj)->name);                       \
    }
#else
#define oss_debug_sem(obj, fstr)
#endif

#if OSS_DEBUG_EVENT
#define oss_debug_event(obj, fstr)                                          \
    if ((obj)->debug_flag) {                                                \
        oss_printf("Event %s "fstr, (obj)->name);                           \
    }
#else
#define oss_debug_event(obj, fstr)
#endif

#if OSS_DEBUG_THREAD
#define oss_debug_thread(obj, fstr)                                         \
    if ((obj)->debug_flag) {                                                \
        oss_printf("Thread %s "fstr, (obj)->name);                          \
    }
#else
#define oss_debug_thread(obj, fstr)
#endif

#if OSS_DEBUG_SPINLOCK
#define oss_debug_spinlock(obj, fstr)                                       \
    if ((obj)->debug_flag) {                                                \
        oss_printf("[sLock %s "fstr, (obj)->name);                          \
    }
#else
#define oss_debug_spinlock(obj, fstr)
#endif

#if OSS_DEBUG_TIMER
#define oss_debug_timer(obj, fstr)                                          \
    if ((obj)->debug_flag) {                                                \
        oss_printf("Timer %s "fstr, (obj)->name);                           \
    }
#else
#define oss_debug_timer(obj, fstr)
#endif


#define OSS_DEBUG_LEVEL_NONE            0   // no messages
#define OSS_DEBUG_LEVEL_ERROR           1   // error messages
#define OSS_DEBUG_LEVEL_WARN            2   // error and warning messages
#define OSS_DEBUG_LEVEL_INFO            3   // information messages
#define OSS_DEBUG_LEVEL_VERBOSE         4   // all messages

/*
 * Select the level of messages to build
 */
#if DEBUG_OSSUPPORT
#define OSS_DEBUG_LEVEL                 OSS_DEBUG_LEVEL_WARN
#else
#define OSS_DEBUG_LEVEL                 OSS_DEBUG_LEVEL_WARN
#endif

#if OSS_DEBUG_LEVEL == OSS_DEBUG_LEVEL_VERBOSE
extern int oss_verbose_trace, oss_info_trace, oss_warning_trace,
       oss_error_trace;
#elif OSS_DEBUG_LEVEL == OSS_DEBUG_LEVEL_INFO
#define oss_verbose_trace       0       // compile-out verbose messages
extern int oss_info_trace, oss_warning_trace, oss_error_trace;
#elif OSS_DEBUG_LEVEL == OSS_DEBUG_LEVEL_WARN
#define oss_verbose_trace       0       // compile-out verbose messages
#define oss_info_trace          0       // compile-out info messages
extern int oss_warning_trace, oss_error_trace;
#elif OSS_DEBUG_LEVEL == OSS_DEBUG_LEVEL_ERROR
#define oss_verbose_trace       0       // compile-out verbose messages
#define oss_warning_trace       0       // compile-out error messages
#define oss_info_trace          0       // compile-out info messages
extern int oss_error_trace;
#elif OSS_DEBUG_LEVEL == OSS_DEBUG_LEVEL_NONE
#define oss_verbose_trace       0       // compile-out verbose messages
#define oss_error_trace         0       // compile-out error messages
#define oss_warning_trace       0       // compile-out warning messages
#define oss_info_trace          0       // compile-out info messages
#endif

#define oss_print_error( ...)           \
    if (oss_error_trace) {              \
        oss_printf("Error : ");         \
        oss_printf(__VA_ARGS__);        \
    }

#define oss_print_warn(...)             \
    if (oss_warning_trace) {            \
        oss_printf("Warning : ");       \
        oss_printf(__VA_ARGS__);        \
    }

#define oss_print_info(...)             \
    if (oss_info_trace) {               \
        oss_printf(__VA_ARGS__);        \
    }

#define oss_print_verbose(...)          \
    if (oss_verbose_trace) {            \
        oss_printf(__VA_ARGS__);        \
    }

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _OS_SUPPORT_H_ */

