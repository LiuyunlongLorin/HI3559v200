/** @defgroup linux_kernel Linux Kernel Adapt
 *  @ingroup linux
*/

#ifndef __LINUX_KERNEL_H__
#define __LINUX_KERNEL_H__

#include <linux/types.h>
#include <linux/slab.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/io.h>
#include <sys/time.h>
#include "asm/io.h"
#include "mutex.h"
#include "errno.h"
#include "string.h"
#include "unistd.h"
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#endif
#include "los_exc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define printk    dprintf  //do not modify for code check

#define jiffies     0
#define HZ          LOSCFG_BASE_CORE_TICK_PER_SECOND

#define SZ_1K       (0x00000400)
#define __init
#define __exit
#define __user

#define ERR_PTR(err) ((void*)(unsigned long)(err))
#define PTR_ERR(err) ((unsigned long)(err))
#define IS_ERR(err) ((unsigned long)err > (unsigned long)-1000L)

#define    KERN_EMERG              "<0>" // system is unusable
#define    KERN_ALERT              "<1>" // action must be taken immediately
#define    KERN_CRIT               "<2>" // critical conditions
#define    KERN_ERR                "<3>" // error conditions
#define    KERN_WARNING            "<4>" // warning conditions
#define    KERN_NOTICE             "<5>" // normal but significant condition
#define    KERN_INFO               "<6>" // informational
#define    KERN_DEBUG              "<7>" // debug-level messages
#define    KERN_CONT                "<c>"

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_emerg(fmt, ...) \
    dprintf(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
    dprintf(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
    dprintf(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
    dprintf(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warning(fmt, ...) \
    dprintf(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn pr_warning
#define pr_notice(fmt, ...) \
    dprintf(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
    dprintf(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define pr_cont(fmt, ...) \
    dprintf(KERN_CONT fmt, ##__VA_ARGS__)

/* pr_devel() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pr_devel(fmt, ...) \
    dprintf(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_devel(fmt, ...) do {} while(0)
#endif
/* If you are writing a driver, please use dev_dbg instead */
#if defined(DEBUG)
#define pr_debug(fmt, ...) \
    dprintf(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) do {} while(0)
#endif

#define WARN_ON(condition) do {} while(0)

#ifndef min
#define min(x, y) (x < y ? x : y)
#endif

#ifndef max
#define max(x, y) (x < y ? y : x)
#endif

#ifndef min_t
#define min_t(t, x, y) ((t)x < (t)y ? (t)x : (t)y)
#endif

#include <asm/bug.h>
#define BUG_ON(condition) do { if (condition) BUG(); } while(0)

/*it can not be commented ,because it used for business*/
#define panic  LOS_Panic

#define __setup(str, fn)

/**
 * @ingroup  linux_kernel
 * @brief change jiffies time to tick (not supported).
 *
 * @par Description:
 * This API is used to change jiffies time to tick time.
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 *@param  j   [IN] the jiffies time value.
 *
 * @retval Tick time value          The value of tick time.
 * @par Dependency:
 * <ul><li> kernel.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V100R001C00
 */
extern UINT64 jiffies_to_tick(unsigned long j);
//void msleep(unsigned int msecs);

 /**
 * @ingroup  linux_kernel
 * @brief Delay a task.
 *
 * @par Description:
 * This API is used to delay the execution of the current task. The task is able to be scheduled after it is delayed for a specified number of Ticks.
 *
 * @attention
 * <ul>
 * <li>The task fails to be delayed if it is being delayed during interrupt processing or it is locked.</li>
 * <li>If 0 is passed in and the task scheduling is not locked, execute the next task in the queue of tasks with the priority of the current task.
 * If no ready task with the priority of the current task is available, the task scheduling will not occur, and the current task continues to be executed.</li>
 * </ul>
 *
 * @param  timeout [IN] Type #signed long Number of Ticks for which the task is delayed(unit: Tick).
 *
 * @retval #LOS_ERRNO_SWTMR_HWI_ACTIVE                   The software timer is being used during an interrupt.
 * @retval #LOS_OK                      The task is successfully delayed.
 * @retval #LOS_ERRNO_SWTMR_NOT_STARTED                 The software timer is not started.
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
signed long schedule_timeout(signed long timeout);
#define schedule_timeout_uninterruptible(t) schedule_timeout(t)
#define schedule_timeout_interruptible(t) schedule_timeout(t)

#define in_interrupt()        (0)

extern unsigned long long hi_sched_clock(void);

 /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the remainder
 *
 * @attention
 * <ul>
 * <li>the param n should point to a valid address.</li>
 * <li>the param base should not be 0.</li>
 * </ul>
 *
 * @param  n [IN/OUT]     the dividend as IN,the  quotient as OUT
 * @param  base [IN]                           the divisor
 * @retval remainder
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
extern UINT32 do_div_imp(UINT64 *n, UINT32 base);

  /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the remainder
 *
 * @attention
 * <ul>
 * <li>the param n should point to a valid address.</li>
 * <li>the param base should not be 0.</li>
 * </ul>
 *
 * @param  n [IN/OUT]     the dividend as IN,the  quotient as OUT
 * @param  base [IN]                           the divisor  >0
 * @retval remainder
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
extern INT32 do_div_s64_imp(INT64 *n, INT32 base);
  /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient
 *
 * @attention
 * <ul>
 * <li>the param divisor should not be 0.</li>
 * </ul>
 *
 * @param  dividend [IN]     the dividend as IN
 * @param  divisor [IN]      the divisor  >0
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V200R001C00
 */
static inline UINT64 div64_u64(UINT64 dividend, UINT64 divisor)
{
    return dividend / divisor;
}
  /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient
 *
 * @attention
 * <ul>
 * <li>the param divisor should not be 0.</li>
 * </ul>
 *
 * @param  dividend [IN]     the dividend as IN
 * @param  divisor [IN]      the divisor not is 0
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V200R001C00
 */
static inline INT64 div64_s64(INT64 dividend, INT64 divisor)
{
    return dividend / divisor;
}

#define do_div(n,base) ({                    \
UINT32 __base = (base);                \
UINT32 __rem;                        \
__rem = ((UINT64)(n)) % __base;            \
(n) = ((UINT64)(n)) / __base;                \
__rem;                            \
 })

  /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient ,and remainder as OUT
 *
 * @attention
 * <ul>
 * <li>the param divisor should not be 0.</li>
 * <li>the param remainder should point to a valid address.</li>
 * </ul>
 *
 * @param dividend [IN]     the dividend as IN
 * @param  divisor [IN]       the divisor is not 0 ,and as IN
 * @param  remainder [OUT]   the remainder should point to a valid address. remainder as OUT
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration and implimentation.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
static inline INT64 div_s64_rem(INT64 dividend, INT32 divisor, INT32 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

 /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient, and remainder as OUT
 *
 * @attention
 * <ul>
 * <li>the param divisor should be greater than 0.</li>
 * <li>the param remainder should point to a valid address.</li>
 * </ul>
 *
 * @param dividend [IN]     the dividend as IN
 * @param  divisor [IN]       the divisor is greater than 0, and as IN
 * @param  remainder [OUT]   the remainder should point to a valid address. remainder as OUT
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration and implimentation.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
static inline UINT64 div64_u64_rem(UINT64 dividend, UINT64 divisor, UINT64 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

 /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient,and  remainder as OUT
 *
 * @attention
 * <ul>
 * <li>the param divisor should be greater than 0.</li>
 * <li>the param remainder should point to a valid address.</li>
 * </ul>
 *
 * @param dividend [IN]     the dividend as IN
 * @param  divisor [IN]       the divisor is greater than 0, and as IN
 * @param  remainder [OUT]   the remainder should point to a valid address. remainder as OUT
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration and implimentation.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
static inline UINT64 div_u64_rem(UINT64 dividend, UINT32 divisor, UINT32 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

 /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient
 *
 * @attention
 * <ul>
 * <li>the param divisor should not be 0.</li>
 * </ul>
 *
 * @param dividend [IN]     the dividend as IN
 * @param  divisor [IN]       the divisor is not 0, and as IN
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration and implimentation.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
static inline INT64 div_s64(INT64 dividend, INT32 divisor)
{
    INT32 remainder;
    return div_s64_rem(dividend, divisor, &remainder);
}

 /**
 * @ingroup  linux_kernel
 * @brief do division implimentation.
 *
 * @par Description:
 * This API is used to do a division implimentation,and return the quotient
 *
 * @attention
 * <ul>
 * <li>the param divisor should be greater than 0.</li>
 * </ul>
 *
 * @param dividend [IN]     the dividend as IN
 * @param  divisor [IN]       the divisor is greater than 0, and as IN
 * @retval quotient
 * @par Dependency:
 * <ul><li>kernel.h: the header file that contains the API declaration and implimentation.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */
static inline UINT64 div_u64(UINT64 dividend, UINT32 divisor)
{
    UINT32 remainder;
    return div_u64_rem(dividend, divisor, &remainder);
}
static inline unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
    if(to == NULL || from == NULL)
        return (unsigned long)-1;
    memcpy(to, from, n);
    return 0;
}

static inline unsigned long copy_to_user(void *to, const void *from, unsigned long n)
{
    if(to == NULL || from == NULL)
        return (unsigned long)-1;
    memcpy(to, from, n);
    return 0;
}

static inline void * ioremap_cached(unsigned long phys_addr, unsigned int size)
{
    extern VOID osRemapCached(unsigned long phys_addr, unsigned long size);
    osRemapCached(phys_addr, size);

    return (void *)phys_addr;
}

static inline void * ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
    extern VOID osRemapNoCached(unsigned long phys_addr, unsigned long size);
    osRemapNoCached(phys_addr, size);

    return (void *)phys_addr;
}

static inline void iounmap(void *addr)
{
}

#define likely(x)       (x)
#define unlikely(x)     (x)
#define EXPORT_SYMBOL(x)

typedef void (*unused_func_t)(void);

struct file_operations {
    struct module *owner;
    unused_func_t   llseek;
    unused_func_t   read;
    unused_func_t   write;
    unused_func_t   aio_read;
    unused_func_t   aio_write;
    unused_func_t   readdir;
    unused_func_t   poll;
    unused_func_t   unlocked_ioctl;
    unused_func_t   compat_ioctl;
    unused_func_t   mmap;
    unused_func_t   open;
    unused_func_t   flush;
    unused_func_t   release;
    unused_func_t   fsync;
    unused_func_t   aio_fsync;
    unused_func_t   fasync;
    unused_func_t   lock;
    unused_func_t   sendpage;
    unused_func_t   get_unmapped_area;
    unused_func_t   check_flags;
    unused_func_t   flock;
    unused_func_t   splice_write;
    unused_func_t   splice_read;
    unused_func_t   setlease;
    unused_func_t   fallocate;
};



#define MAP_FAILED      ((void *)-1)
#define simple_strtol   strtol
#define do_gettimeofday(a)  gettimeofday(a, NULL)
#define DEFINE_MUTEX(m)     pthread_mutex_t m;
#define mutex_lock          (void)pthread_mutex_lock
#define mutex_unlock        (void)pthread_mutex_unlock
#define mutex_init(m)          pthread_mutex_init(m, NULL)

static inline void printtime(void)
{
    struct timeval time;

    gettimeofday(&time, (struct timezone *)NULL);
    PRINTK("[time:%d.%03d]", time.tv_sec, time.tv_usec/1000);
    return;
}

#define TRACETIME()    do { printtime(); printk("func:%s, line %d\r\n", __FUNCTION__, __LINE__); } while(0)

extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern int munmap(void *addr, size_t length);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_KERNEL_H__ */
