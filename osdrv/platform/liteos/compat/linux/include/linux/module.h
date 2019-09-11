#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/list.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct symbol {
    int (*initcall)(void);
};
extern struct symbol _initcall0_start, _initcall0_end;
extern struct symbol _initcall1_start, _initcall1_end;
extern struct symbol _initcall2_start, _initcall2_end;
extern struct symbol _initcall3_start, _initcall3_end;
extern struct symbol _initcall4_start, _initcall4_end;
extern struct symbol _initcall5_start, _initcall5_end;
extern struct symbol _initcall6_start, _initcall6_end;
extern struct symbol _initcall7_start, _initcall7_end;

#define LEVEL_PURE      0
#define LEVEL_CORE      1
#define LEVEL_POSTCORE  2
#define LEVEL_ARCH      3
#define LEVEL_SUBSYS    4
#define LEVEL_FS        5
#define LEVEL_DEVICE    6
#define LEVEL_LATE      7


#define __used          __attribute__((__used__))
/*lint -esym(528,sfn_*)*/
#define define_initcall(f, level) static struct symbol sfn_##f##level __used  __attribute__((section(".initcall" #level ".init" )))={f}

#define pure_initcall(f)       define_initcall(f, 0)
#define core_initcall(f)       define_initcall(f, 1)
#define postcore_initcall(f)   define_initcall(f, 2)
#define arch_initcall(f)       define_initcall(f, 3)
#define subsys_initcall(f)     define_initcall(f, 4)
#define fs_initcall(f)         define_initcall(f, 5)
#define device_initcall(f)     define_initcall(f, 6)
#define late_initcall(f)       define_initcall(f, 7)

#define _initcall(f) device_initcall(f)

#define module_init(f) _initcall(f)
#define module_exit(f)

static inline int do_initcall(struct symbol *start, struct symbol *end)
{
    struct symbol *symbol;

    for (symbol = start; symbol != end; symbol++) {
        if (symbol->initcall()) {
            dprintf("failed to initialize the module:%08x\n", symbol->initcall);
            return -1; /* use the standard error code later */
        }
    }

    return 0;
}

#define do_initcall_level(level) do_initcall(&_initcall##level##_start, &_initcall##level##_end)
static inline int do_initCalls(int swLevel)
{
    int ret;

    if(swLevel > LEVEL_LATE)
    {
        ret = LOS_NOK;
        goto error;
    }

    if(swLevel >= LEVEL_PURE)
    {
        ret = do_initcall_level(0);
        if (ret) goto error;
    }

    if(swLevel >= LEVEL_CORE)
    {
        ret = do_initcall_level(1);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_POSTCORE)
    {
        ret = do_initcall_level(2);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_ARCH)
    {
        ret = do_initcall_level(3);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_SUBSYS)
    {
        ret = do_initcall_level(4);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_FS)
    {
        ret = do_initcall_level(5);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_DEVICE)
    {
        ret = do_initcall_level(6);
        if (ret) goto error;
    }
    if(swLevel >= LEVEL_LATE)
    {
        ret = do_initcall_level(7);
        if (ret) goto error;
    }

    return LOS_OK;
error:
    return ret;
}

#define module_param_named(u, v , t, f)
#define MODULE_PARM_DESC(_parm, desc)

#define MODULE_AUTHOR(a)
#define MODULE_DESCRIPTION(d)
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(s)
#endif
#define MODULE_VERSION(v)
#define module_param_array(name, type, nump, perm)

struct module
{
};

static inline int try_module_get(struct module *module)
{
    return 1;
}

static inline void module_put(struct module *module)
{
}

#define THIS_MODULE ((struct module *)0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LINUX_MODULE_H */
