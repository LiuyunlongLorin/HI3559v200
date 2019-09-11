#include <linux/module.h>
#include <linux/kernel.h>
#include "hi_osal.h"

/* the result of u64/u32. */
unsigned long long osal_div_u64(unsigned long long dividend, unsigned int divisor)
{
    return div_u64(dividend, divisor);
}

/* the result of s64/s32. */
long long osal_div_s64(long long dividend, int divisor)
{
    return div_s64(dividend, divisor);
}

/* the result of u64/u64. */
unsigned long long osal_div64_u64(unsigned long long dividend, unsigned long long divisor)
{
    return div64_u64(dividend, divisor);
}

/* the result of s64/s64. */
long long osal_div64_s64(long long dividend, long long divisor)
{
    return div64_s64(dividend, divisor);
}

/* the remainder of u64/u32. */
unsigned long long osal_div_u64_rem(unsigned long long dividend, unsigned int divisor)
{
    unsigned int remainder = 0;

    div_u64_rem(dividend, divisor, &remainder);

    return remainder;
}

/* the remainder of s64/s32. */
long long osal_div_s64_rem(long long dividend, int divisor)
{
    int remainder = 0;

    div_s64_rem(dividend, divisor, &remainder);

    return remainder;
}

/* the remainder of u64/u64. */
unsigned long long osal_div64_u64_rem(unsigned long long dividend, unsigned long long divisor)
{
    unsigned long long remainder = 0;

    div64_u64_rem(dividend, divisor, &remainder);
    return remainder;
}

unsigned int osal_random(void)
{
    return (unsigned int)random();
}



