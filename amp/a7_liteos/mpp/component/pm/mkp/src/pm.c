/******************************************************************************
 Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : pm.c
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/09/17
Last Modified :
Description   :
Function List :
******************************************************************************/

#include "hi_osal.h"
#include "hi_common.h"
#include "hi_type.h"
#include "mod_ext.h"
#include "vb_ext.h"
#include "hi_debug.h"
#include "proc_ext.h"
#include "sys_ext.h"


#include "dev_ext.h"
#include "pm.h"
#include "mkp_pm.h"

#include "pm_device.h"
#include "pm_dev_api.h"
#include "pm_ext.h"

#include "autoconf.h"

osal_spinlock_t g_stPmLock;
osal_semaphore_t g_stPmSem;

/*
#if (HICHIP==HI3516C_V500) || (HICHIP==HI3519A_V100)

#else

#endif
*/


PM_CTX_S*                  g_pstPmCtx = HI_NULL;
static osal_dev_t*          astPmDevice;
PM_MOD_STATE_E         g_enPmModState = PM_MOD_STATE_STOPPED;  /*module state*/
static osal_atomic_t        s_stPmUserRef = OSAL_ATOMIC_INIT(0);
HI_MPI_PM_MEDIA_CFG_S  g_stPmParam;

extern struct pm_device pm_dev;

int g_bAvspOn = HI_FALSE;
int g_bSvpAcceleratorOn = HI_FALSE;
/***********************************************************************************
 Prototype      : PMProcShow
 Description  : PM proc information
 Input          : struct seq_file* s, HI_VOID* pData
 Output       : none
 Return Value :
 Calls          :
 Called By      :

 History      :
 Date          : 2018/09/17
 Author       :
 Modification : Created function
***********************************************************************************/

#define _1M_ 1000000
HI_S32 PM_ProcShow(osal_proc_entry_t* sfile)
{
    osal_seq_printf(sfile, "\n-------------------------------------------------------------------\n");
    osal_seq_printf(sfile, "PM status:\n");
    osal_seq_printf(sfile, "  domain_num    = %0d;\n", pm_dev.domain_num);
    osal_seq_printf(sfile, "  pm_enable     = %0d;\n", pm_dev.pm_enable);
    osal_seq_printf(sfile, "-------------CPU domain-------------\n");
#ifdef PM_DOMAIN_CPU_ON
    struct pm_domain_device* cpu = pm_dev.cpu_domain_device;
    struct freq_device* cpu_freq = cpu->freq_dev;
//    struct avs_device* cpu_avs = cpu->avs_dev;
    if(cpu_freq != NULL) {
        osal_seq_printf(sfile, "  dvfs_enable   = %0d\n", cpu_freq->dvfs_enable);
        osal_seq_printf(sfile, "  cur_volt    = %0duV \n", cpu->cur_volt);
        /*
        osal_seq_printf(sfile, "  cur_freq      = %0dMHz [%0d~%0d];\n" \
                        , cpu->cur_freq / _1M_, cpu_freq->min_freq / _1M_, cpu_freq->max_freq / _1M_);
        osal_seq_printf(sfile, "  cur_volt      = %0duV [%0d~%0d];\n", \
                        cpu->cur_volt, cpu_avs->cur_volt_min, cpu_avs->cur_volt_max);
        */
    }
#else
    osal_seq_printf(sfile, "    not support  \n");
#endif
    osal_seq_printf(sfile, "------------- MEDIA domain -------------\n");
#ifdef PM_DOMAIN_MEDIA_ON
    struct pm_domain_device* media = pm_dev.media_domain_device;
    struct avs_device* media_avs = media->avs_dev;
    if(media_avs != NULL) {
        extern int hal_media_get_avs_policy(void);
        HI_MEDIA_AVS_POLICY_E mediaAvsPolicy = (HI_MEDIA_AVS_POLICY_E)hal_media_get_avs_policy();
        if(( mediaAvsPolicy != HI_MEDIA_AVS_POLICY_CLOSE ) && (mediaAvsPolicy != HI_MEDIA_AVS_POLICY_BUTT )) {
            osal_seq_printf(sfile, "  cur_volt    = %0duV \n",  media->cur_volt);
        }
        osal_seq_printf(sfile, "  profile_num   = %0d;\n", media_avs->profile_num);
        osal_seq_printf(sfile, "  cur_profile   = %0d;\n", 0);
        /*
        osal_seq_printf(sfile, "  cur_profile   = %0d;\n",(int) (get_media_profile()));
        osal_seq_printf(sfile, "  cur_volt      = %0duV [%0d~%0d];\n", \
               media->cur_volt, media_avs->cur_volt_min, media_avs->cur_volt_max);
        */
    }
#else
    osal_seq_printf(sfile, "    not support    \n");
#endif
    osal_seq_printf(sfile, "-------------CORE domain -------------\n");
#ifdef PM_DOMAIN_CORE_ON
    struct pm_domain_device* core = pm_dev.core_domain_device;
    struct avs_device* core_avs = core->avs_dev;
    if(core_avs != NULL) {
        osal_seq_printf(sfile, "  cur_volt    = %0d;\n", core->cur_volt);
    }
#else
    osal_seq_printf(sfile, "    not support  \n");
#endif
    osal_seq_printf(sfile, "\n-------------------------------------------------------------------\n");
    return 0;
}



HI_S32 PM_CheckModuleParam(void)
{
    if ( g_bAvspOn > 1 ) {
        osal_printk("ModuleParam:bAvspOn =%d Range[0,1]\n", g_bAvspOn  );
        return HI_FAILURE;
    }
    if ( g_bSvpAcceleratorOn > 1 ) {
        osal_printk("ModuleParam:bSvpAcceleratorOn =%d Range[0,1]\n", g_bSvpAcceleratorOn );
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32     PM_Init(HI_VOID* pArgs)
{
    HI_S32     s32Ret = -1;
    /*if mod has inserted , just return ok*/
    if (PM_MOD_STATE_STARTED == g_enPmModState) {
        return HI_SUCCESS;
    }
    if (HI_FAILURE == PM_CheckModuleParam()) {
        HI_PRINT("PM module param is illegal!\n");
        return HI_FAILURE;
    }
#if NEED_MEM_INFO
    osal_printk("sizeof(PM_CTX_S):%d \n", sizeof(PM_CTX_S));
#endif
    g_pstPmCtx = (PM_CTX_S*)osal_vmalloc(sizeof(PM_CTX_S));
    if (HI_NULL == g_pstPmCtx) {
        HI_TRACE_PM(HI_DBG_ERR, "vmalloc for g_pstPmCtx(size = %d)failed!!  \n", sizeof(PM_CTX_S));
        return s32Ret;
    }
    /*init proc info*/
    osal_memset(&g_pstPmCtx->s_stPmDbgInfo, 0, sizeof(PM_DBG_INFO_S));
    /*change mod state as started*/
    g_enPmModState = PM_MOD_STATE_STARTED;
    return HI_SUCCESS;
}

HI_VOID  PM_Exit(HI_VOID)
{
    if (PM_MOD_STATE_STOPPED == g_enPmModState) {
        return;
    }
    osal_vfree(g_pstPmCtx);
    g_pstPmCtx = HI_NULL;
    g_enPmModState = PM_MOD_STATE_STOPPED;
    HI_TRACE_PM(HI_DBG_INFO, "PM exit successfully...\n");
    return;
}

static HI_VOID PM_Notify(MOD_NOTICE_ID_E enNotice)
{
    if (MOD_NOTICE_STOP == enNotice) {
        HI_TRACE_PM(HI_DBG_INFO, "PM was notified to exit ...\n");
        if (PM_MOD_STATE_STOPPED == g_enPmModState) {
            return;
        }
        g_enPmModState = PM_MOD_STATE_STOPPING;
    }
    return;
}

static HI_VOID PM_QueryState(MOD_STATE_E* pstState)
{
    if ( PM_MOD_STATE_STARTED == g_enPmModState  ) {
        *pstState =  MOD_STATE_BUSY ;
    } else {
        *pstState = MOD_STATE_FREE ;
    }
    return;
}

static HI_U32 PM_GetVerMagic(HI_VOID)
{
    return VERSION_MAGIC;
}

static long PmIoctl(unsigned int cmd, unsigned long arg, void* private_data)
{
    HI_S32 s32Ret = HI_SUCCESS;
    if (PM_MOD_STATE_STARTED != g_enPmModState) {
        return HI_ERR_PM_SYS_NOTREADY;
    }
    osal_atomic_inc_return(&s_stPmUserRef);
    switch (cmd) {
        case PM_SET_MEDIA_PARAM: {
            HI_MPI_PM_MEDIA_CFG_S* pstUsrParam = (HI_MPI_PM_MEDIA_CFG_S*) arg;
            SetMediaProfileByUsrCfg(pstUsrParam);
            break;
        }
        case PM_GET_MEDIA_PARAM: {
            get_media_usrcfg((HI_MPI_PM_MEDIA_CFG_S*) arg);
            break;
        }
        case PM_SET_SINGLE_MEDIA_PARAM: {
            HI_MPI_PM_SIGLE_MEDIA_CFG_S* pstUsrParam = (HI_MPI_PM_SIGLE_MEDIA_CFG_S*) arg;
            SetSingleFreqByUsrCfg(pstUsrParam);
            break;
        }
        case PM_SVP_ENABLE_CTRL: {
            svp_ctrl(1);
            break;
        }
        case PM_DVFS_ENABLE_CTRL: {
            dvfs_ctrl(1);
            break;
        }
        case PM_DVFS_DISABLE_CTRL: {
            dvfs_ctrl(0);
            break;
        }
        case PM_GET_TEMPERATURE: {
            HI_S32 temperature = pm_hal_get_temperature();
            *(HI_S32*)arg = temperature;
            break;
        }
        default:
            s32Ret = HI_FAILURE;
            break;
    }
    osal_atomic_dec_return(&s_stPmUserRef);
    return s32Ret;
}

#ifdef CONFIG_COMPAT
static long PmCompatIoctl(unsigned int cmd, unsigned long arg, void* private_data)
{
    switch (cmd) {
        default: {
            break;
        }
    }
    return PmIoctl(cmd, arg, private_data);
}
#endif

static int PmOpen(void* private_data)
{
    return HI_SUCCESS;
}

static int PmClose(void* private_data)
{
    return HI_SUCCESS;
}

static int PmMmap(osal_vm_t* vm, unsigned long start, unsigned long end, unsigned long vm_pgoff, void* private_data)
{
    return HI_SUCCESS;
}


static UMAP_MODULE_S s_stModule = {
    .enModId        = HI_ID_PM,
    .aModName       = "pm",

    .pfnInit        = PM_Init,
    .pfnExit        = PM_Exit,
    .pfnQueryState     = PM_QueryState,
    .pfnNotify         = PM_Notify,
    .pfnVerChecker    = PM_GetVerMagic,
    .pstExportFuncs = NULL,//&s_stExportFuncs,
    .pData = HI_NULL,
};

static struct osal_fileops pm_fops = {
    .open            = PmOpen,
    .unlocked_ioctl = PmIoctl,
    .release        = PmClose,
    .mmap           = PmMmap,
#ifdef CONFIG_COMPAT
    .compat_ioctl    = PmCompatIoctl
#endif

};

static int PmFreeze(osal_dev_t* pdev)
{
    return HI_SUCCESS;
}

static int PmRestore(osal_dev_t* pdev)
{
    return HI_SUCCESS;
}

struct osal_pmops stPmDrvOps = {
    .pm_freeze  = PmFreeze,
    .pm_restore = PmRestore,
};


void PM_ModInit(void)
{
    char buf[20];
    osal_proc_entry_t* proc;
    osal_snprintf(buf, 20, "%s", UMAP_DEVNAME_PM_BASE);
    astPmDevice = osal_createdev(buf);
    astPmDevice->fops  = &pm_fops;
    astPmDevice->minor = UMAP_PM_MINOR_BASE;
    astPmDevice->osal_pmops = &stPmDrvOps;
    if (osal_registerdevice(astPmDevice) < 0) {
        HI_TRACE_PM(HI_DBG_ERR, "Regist device err.\n");
        return ;
    }
    if (CMPI_RegisterModule(&s_stModule)) {
        osal_printk("load pm.ko for %s...FAILURE!\n", CHIP_NAME);
        goto fail;
    }
    pm_dev_init(false, false);
    proc = osal_create_proc_entry(PROC_ENTRY_PM, NULL);
    if (NULL == proc) {
        HI_TRACE_PM(HI_DBG_ERR, "PM create proc failed. \n");
        goto fail;
    }
    proc->read = PM_ProcShow;
    if (osal_atomic_init(&s_stPmUserRef)) {
        HI_TRACE_PM(HI_DBG_ERR, "PM atomic init failed. \n");
        goto fail1;
    }
    osal_atomic_set(&s_stPmUserRef, 0);
    if (osal_sema_init(&g_stPmSem, 1)) {
        HI_TRACE_PM(HI_DBG_ERR, "pm sem init failed. \n");
        goto fail1;
    }
    osal_printk("load pm.ko for %s...OK!\n", CHIP_NAME);
    return;
fail1:
    osal_remove_proc_entry(PROC_ENTRY_PM, NULL);
fail:
    osal_deregisterdevice(astPmDevice);
    osal_destroydev(astPmDevice);
    astPmDevice = HI_NULL;
    return ;
}

void PM_ModExit(void)
{
    if (HI_NULL != astPmDevice) {
        /* stop the pm dev */
        pm_dev_deinit();
        osal_sema_destory(&g_stPmSem);
        CMPI_UnRegisterModule(HI_ID_PM);
        osal_remove_proc_entry(PROC_ENTRY_PM, NULL);
        osal_deregisterdevice(astPmDevice);
        osal_destroydev(astPmDevice);
        osal_atomic_destory(&s_stPmUserRef);
    }
    osal_printk("Unload pm.ko for %s...OK!\n", CHIP_NAME);
    return ;
}

