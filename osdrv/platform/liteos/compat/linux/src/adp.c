#include "los_sys.ph"

#include "los_swtmr.h"
#include "linux/kernel.h"
#include "linux/module.h"
#include "linux/seq_file.h"
#include "time.h"
#include "limits.h"
#include "sys/statfs.h"
#include "math.h"
#ifdef LOSCFG_NET_LWIP_SACK
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#endif

extern int fstat(int fildes, struct stat *buf);
extern UINT32 ticks_read(void);
extern struct hostent* lwip_gethostbyname(const char *name);

#ifdef __LP64__
int dl_iterate_phdr(int (*callback) (void *info, size_t size, void *data),
                    void *data)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}
#endif

void fs_show(const char *path)
{
    int ret = 0;
    struct statfs fss;
    if (NULL == path)
    {
        PRINTK("path is NULL\n");
    }
    ret = statfs(path, &fss);
    PRINTK("Filesystem %s info: \n", path);
    PRINTK("----------------------------------------\n");
    if (ret == ENOERR)
    {
        PRINTK("  Total clusters: %u \n", fss.f_blocks);
        PRINTK("  Cluster size: %u \n", fss.f_bsize);
        PRINTK("  Free clusters: %u \n", fss.f_bfree);
    }
    else
    {
        ret = get_errno();
        PRINT_ERR("Get fsinfo failed: %d \n", ret);
    }
}

long int syscall(long int sysno, ...)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return ENOSUPP;
}

int fs_fssync( const char *path )
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return ENOSUPP;
}

int __tls_get_addr(void * th,int map_address, size_t offset, void * address)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return ENOSUPP; // FIXME: TODO
}

#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1)-1)

#define    MAX_SCHEDULE_TIMEOUT    LONG_MAX

unsigned long msecs_to_jiffies(const unsigned int m)
{
    /*
     * Negative value, means infinite timeout:
     */
    if ((int)m < 0)
        return (unsigned long)MAX_JIFFY_OFFSET;

#if HZ <= OS_SYS_MS_PER_SECOND && !(OS_SYS_MS_PER_SECOND % HZ)
    /*
     * HZ is equal to or smaller than 1000, and 1000 is a nice
     * round multiple of HZ, divide with the factor between them,
     * but round upwards:
     */
    return (m + (OS_SYS_MS_PER_SECOND / HZ) - 1) / (OS_SYS_MS_PER_SECOND / HZ);
#else
    PRINT_ERR("HZ: %d is not supported in %s\n", HZ, __FUNCTION__);
    return ENOSUPP;
#endif
}

UINT64 jiffies_to_tick(unsigned long j)
{
    return j;
}

signed long schedule_timeout(signed long timeout)
{
    UINT32   uwRet = 0;

    if (OS_INT_ACTIVE) {
        PRINT_ERR("ERROR: OS_ERRNO_SWTMR_HWI_ACTIVE\n");
        return LOS_ERRNO_SWTMR_HWI_ACTIVE;
    }

    uwRet = LOS_TaskDelay(timeout);

    if (uwRet == LOS_OK) {
        return uwRet;
    }
    else {
        PRINT_ERR("ERROR: OS_ERRNO_SWTMR_NOT_STARTED\n");
        return LOS_ERRNO_SWTMR_NOT_STARTED;
    }

}

void linux_module_init(void)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
}

// TODO:
UINT32 do_div_imp(UINT64 *n, UINT32 base)
{
    UINT32 r = 0;

    if ((NULL == n) || (0 == base))
    {
        PRINT_ERR("%s invalid input param, n:%p, base %d\n", __FUNCTION__, n, base);
        return 0;
    }

    r = *n % base;
    *n = *n / base;
    return r;
}

INT32 do_div_s64_imp(INT64 *n, INT32 base)
{
    INT32 r = 0;

    if ((NULL == n) || (0 == base))
    {
        PRINT_ERR("%s invalid input param, n:%p, base:%d\n", __FUNCTION__, n, base);
        return 0;
    }

    r = *n % base;
    *n = *n / base;
    return r;
}

    //mmap and munmap is
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    return (void *)offset;
}

int munmap(void *addr, size_t length)
{
    return 0;
}

int waitpid (int __pid, int *__stat_loc, int __options)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

void closelog(void)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
}

unsigned int alarm (unsigned int __seconds)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int daemon (int __nochdir, int __noclose)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

typedef void __signalfn_t(int);
typedef __signalfn_t *__sighandler_t;
__sighandler_t sigset(int __sig, __sighandler_t __disp)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int setgroups(size_t __size, const gid_t* __list)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int initgroups(const char * user, __gid_t group)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}


int chroot (const char *__path)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

void *getpwnam (const char *__name)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int fchown(int __fd, uid_t __owner, gid_t __group)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}
ssize_t readlink (const char * __path, char * __buf, size_t __len)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

typedef uint32_t in_addr_t;

int gethostname (char *name, size_t len)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int fork (void)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int nice (int inc)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int getdtablesize (void)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int pipe (int pipedes[2])
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int execve(const char* __file, char* const* __argv, char* const* __envp)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int getrlimit (int resource, void *rlimits)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int setrlimit (int resource, void *rlimits)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

uid_t getuid(void)
{
    //PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

uid_t geteuid(void)
{
    //PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

pid_t setsid(void)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int setgid(gid_t id)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

int setuid(uid_t id)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

const char *crypt(const char *key, const char *salt)
{
    return key;
}

void openlog(const char *ident, int option, int facility)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
}
char *basename(const char *path)
{
    static const char null_or_empty[] = ".";
    char *first;
    register char *last;

    first = (char *) null_or_empty;

    if (path && *path) {
        first = (char *)path;
        last = (char *)path - 1;

        do {
            if ((*path != '/') && (path > ++last)) {
                last = first = (char *)path;
            }
        } while (*++path);

        if (*first == '/') {
            last = first;
        }
        last[1] = 0;
    }

    return first;
}

char *
stpcpy(char * to, const char * from)
{
    for (; (*to = *from); ++from, ++to);
    return(to);
}

void *__dso_handle = 0;

void __cxa_atexit(void (*func)(void*), void* arg1, void* arg2)
{
    ;
}

__attribute__((weak)) void __aeabi_memclr(void *dest, size_t n)
{
    memset(dest, 0, n);
}

__attribute__((weak)) void __aeabi_memclr4(void *dest, size_t n)
{
    memset(dest, 0, n);
}

__attribute__((weak)) void __aeabi_memclr8(void *dest, size_t n)
{
    memset(dest, 0, n);
}
//--------------------------------------------------------------------------------

