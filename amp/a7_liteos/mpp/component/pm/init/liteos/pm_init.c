
#include "hi_module_param.h"


extern void  PM_ModInit(void);
extern void PM_ModExit(void);
extern int g_bAvspOn;
extern int g_bSvpAcceleratorOn;


int pm_mod_init(void* pArgs)
{
    PM_MODULE_PARAMS_S* pstPmModuleParam = (PM_MODULE_PARAMS_S*)pArgs;

    if (NULL != pstPmModuleParam)
    {
        g_bAvspOn  = pstPmModuleParam->bAvspOn;
        g_bSvpAcceleratorOn = pstPmModuleParam->bSvpAcceleratorOn;
    }

    PM_ModInit();

    return 0;
}

void pm_mod_exit(void)
{
    PM_ModExit();
}



