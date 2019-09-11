#include "los_swtmr.h"
#include <linux/timer.h>
#include "stdio.h"

void init_timer(struct timer_list *timer)
{
    if(NULL == timer){
        PRINTK("init_timer<error> timer is NULL\n");
        return;
    }
    timer->flag = TIMER_UNVALID;
}

void add_timer(struct timer_list *timer)
{
    int ret = 0;
    unsigned int intr_save = 0;

    if(NULL == timer){
        PRINTK("add_timer<error> timer is NULL\n");
        return;
    }

    intr_save = LOS_IntLock();

    (void)del_timer(timer);

    ret = LOS_SwtmrCreate(timer->expires, LOS_SWTMR_MODE_NO_SELFDELETE, (SWTMR_PROC_FUNC)timer->function, &timer->timerid, timer->data);
    if(LOS_OK != ret)
    {
        PRINTK("add_timer<error> timer create failed : %d \n",ret);
        LOS_IntRestore(intr_save);
        return;
    }
    ret = LOS_SwtmrStart(timer->timerid);
    if(LOS_OK != ret)
    {
        PRINTK("add_timer<error> timer start failed : %d \n",ret);
        LOS_IntRestore(intr_save);
        return;
    }

    timer->flag = TIMER_VALID;
    LOS_IntRestore(intr_save);
}

int del_timer(struct timer_list * timer)
{
    if(NULL == timer){
        PRINTK("del_timer<error> timer is NULL\n");
        return 0;
    }

    if(timer->flag == TIMER_VALID)
    {
        if(LOS_OK == LOS_SwtmrDelete(timer->timerid))
        {
            timer->flag = TIMER_UNVALID;
            return 1;
        }
    }

    return 0;
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
    int ret;

    if(NULL == timer)
        return 0;

    if(timer->flag == TIMER_VALID)
    {
        ret = del_timer(timer);
        timer->expires = expires;
        add_timer(timer);
        return ret;
    }
    else
        return 0;
}

void msleep(unsigned int msecs)
{
    LOS_Msleep(msecs);
}

/*****************************************************************************
Function   : get_jiffies_64
Description: get current jiffies
Input      : None
Output     : None
Return     : current jiffies
*****************************************************************************/
unsigned long long get_jiffies_64(void)
{
    return LOS_TickCountGet();
}
/*****************************************************************************
Function   : jiffies_to_msecs
Description: jiffies convert to milliseconds
Input      : jiffies
Output     : None
Return     : milliseconds
*****************************************************************************/
unsigned int jiffies_to_msecs(const unsigned long j)
{
    return LOS_Tick2MS(j);
}

