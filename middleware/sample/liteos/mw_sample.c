
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "los_cppsupport.h"
#include "hi_type.h"
#include "hi_avplay_server_ipcmsg.h"
#include "hi_mw_media_intf.h"




/******************************************************************************
* function    : main()
* Description : video vdec sample
******************************************************************************/
int mw_init()
{
    HI_S32 s32Ret = HI_SUCCESS;

/*init cpp running context on liteos begin*/
    extern char __init_array_start__, __init_array_end__;

    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__,NO_SCATTER);
/*init cpp running context on liteos end*/

    printf("HI_AVPLAY_IPCMSG_SVR_Init \n");
    s32Ret = HI_AVPLAY_IPCMSG_SVR_Init(NULL);
    if (HI_SUCCESS != s32Ret)
    {
       printf("HI_AVPLAY_IPCMSG_SVR_Init  failed Ret: %x\n", s32Ret);
       return HI_FAILURE;
    }

    printf("HI_MW_Sys_Init \n");
    s32Ret = HI_MW_Sys_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MW_Sys_Init failed Ret: %x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
