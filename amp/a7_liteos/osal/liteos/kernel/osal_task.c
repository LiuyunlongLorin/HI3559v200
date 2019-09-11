#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "los_task.h"

osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const char *name)
{
    unsigned int taskid;
    unsigned int ret;
    TSK_INIT_PARAM_S stTaskParam;

    osal_task_t *p = (osal_task_t *)kmalloc(sizeof(osal_task_t), GFP_KERNEL);

    if (p == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return NULL;
    }
    memset(p, 0, sizeof(osal_task_t));

    memset(&stTaskParam, 0, sizeof(TSK_INIT_PARAM_S));
    stTaskParam.pfnTaskEntry = (TSK_ENTRY_FUNC)thread;
    stTaskParam.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    stTaskParam.auwArgs[0] = (AARCHPTR)data;
    stTaskParam.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stTaskParam.pcName = name;
    stTaskParam.uwResved = LOS_TASK_STATUS_DETACHED;
    ret = LOS_TaskCreate(&taskid, &stTaskParam);

    if (ret != LOS_OK) {
        osal_printk("%s - kthread create error!\n", __FUNCTION__);
        kfree(p);
        return NULL;
    }

    p->task_struct = (void *)taskid;

    return p;
}

void osal_kthread_destory(osal_task_t *task, unsigned int stop_flag)
{
    if (task == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

    if (stop_flag != 0) {
        LOS_TaskDelete ((unsigned int)(task->task_struct));
    }

    task->task_struct = NULL;
    kfree(task);
}


