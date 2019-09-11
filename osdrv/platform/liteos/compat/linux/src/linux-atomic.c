#include "los_typedef.h"
extern BOOL LOS_AtomicCmpXchg32bits(volatile VOID *puwAddr, UINT32 uwNewVal, UINT32 uwOldVal);
extern void dmb(void);

/* Kernel helper for compare-and-exchange.  */
int __kernel_cmpxchg(int oldval,int newval,int *ptr)
{
    return LOS_AtomicCmpXchg32bits(ptr,newval,oldval);
}

/* Kernel helper for memory barrier.  */
void __kernel_dmb(void)
{
    //dmb();
}
