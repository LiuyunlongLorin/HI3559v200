#include "los_cpup.h"
#include "los_swtmr.h"
#include "los_base.h"
#include "los_mux.h"
#include "los_task.ph"
#include "asm/delay.h"
#include "string.h"

#include "pm_device.h"


void start_timer(timer_handler_func_t pfntimer_handler)
{
    unsigned int uwRet = 0;
    TSK_INIT_PARAM_S pm_task;
    unsigned int pm_taskid;
    memset(&pm_task, 0, sizeof(TSK_INIT_PARAM_S));
    pm_task.pfnTaskEntry = (TSK_ENTRY_FUNC)pfntimer_handler;
    pm_task.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    pm_task.pcName = "pm_task";
    pm_task.usTaskPrio = 2;
    pm_task.uwResved = LOS_TASK_STATUS_DETACHED;
    uwRet = LOS_TaskCreate(&pm_taskid, &pm_task);
    if (LOS_OK != uwRet) {
        dprintf("Error:create pm_task failed!\n");
        return;
    }
}


