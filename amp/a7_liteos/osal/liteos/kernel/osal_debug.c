#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>

int osal_printk(const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = vprintf(fmt, args);
    va_end(args);

    return r;
}

void osal_panic(const char *fmt, const char *fun, int line, const char *cond)
{
    panic(fmt, fun, line, cond);
}



