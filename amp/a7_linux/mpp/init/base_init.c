#include <linux/module.h>

#include "hi_type.h"
#include "hi_common.h"
#include "mod_ext.h"

extern int HI_LOG(HI_S32 s32Level, MOD_ID_E enModId, const char *fmt, ...);
EXPORT_SYMBOL(HI_LOG);

extern HI_U32  CMPI_MmzMalloc(HI_CHAR *pMmzName, HI_CHAR *pBufName, HI_UL ulSize);
EXPORT_SYMBOL(CMPI_MmzMalloc);
extern HI_VOID CMPI_MmzFree(HI_U64 u64PhyAddr, HI_VOID* pVirtAddr);
EXPORT_SYMBOL(CMPI_MmzFree);
extern HI_S32  CMPI_MmzMallocNocache(HI_CHAR* cpMmzName, HI_CHAR* pBufName,
                             HI_U32* pu32PhyAddr, HI_VOID** ppVitAddr, HI_UL ulLen);
EXPORT_SYMBOL(CMPI_MmzMallocNocache);
extern HI_S32  CMPI_MmzMallocCached(HI_CHAR* cpMmzName, HI_CHAR* pBufName,
                            HI_U32* pu32PhyAddr, HI_VOID** ppVitAddr, HI_UL ulLen);
EXPORT_SYMBOL(CMPI_MmzMallocCached);

extern HI_VOID * CMPI_Remap_Cached(HI_U64 u64PhyAddr, HI_UL ulSize);
EXPORT_SYMBOL(CMPI_Remap_Cached);

extern HI_VOID * CMPI_Remap_Nocache(HI_U64 u64PhyAddr, HI_UL ulSize);
EXPORT_SYMBOL(CMPI_Remap_Nocache);

extern HI_VOID   CMPI_Unmap(HI_VOID *pVirtAddr);
EXPORT_SYMBOL(CMPI_Unmap);


extern HI_U32 vb_force_exit;
module_param(vb_force_exit, uint, S_IRUGO);


/*************************MOD********************/
EXPORT_SYMBOL(CMPI_GetModuleName);
EXPORT_SYMBOL(CMPI_GetModuleById);
EXPORT_SYMBOL(CMPI_GetModuleFuncById);
EXPORT_SYMBOL(CMPI_StopModules);
EXPORT_SYMBOL(CMPI_QueryModules);
EXPORT_SYMBOL(CMPI_ExitModules);
EXPORT_SYMBOL(CMPI_InitModules);
EXPORT_SYMBOL(CMPI_RegisterModule);
EXPORT_SYMBOL(CMPI_UnRegisterModule);

/******* create node  /proc/sys/dev/debug/proc_message_enable ************/
extern HI_S32 g_proc_enable;
EXPORT_SYMBOL(g_proc_enable);

static struct ctl_table comm_eproc_table[] = {
    {
        .procname       = "proc_message_enable",
        .data           = &g_proc_enable,
        .maxlen         = sizeof(g_proc_enable),
        .mode           = 0644,
        .proc_handler   = proc_dointvec
    },
    {}
};

static struct ctl_table comm_dir_table[] = {
    {
        .procname       = "debug",
        .mode           = 0555,
        .child          = comm_eproc_table
    },
    {}
};

static struct ctl_table comm_parent_tbl[] = {
    {
     .procname       = "dev",
     .mode           = 0555,
     .child          = comm_dir_table
    },
    {}
};

static struct ctl_table_header *comm_eproc_tbl_head;

int __init COMM_init_proc_ctrl(void)
{
    comm_eproc_tbl_head = register_sysctl_table(comm_parent_tbl);
    if (!comm_eproc_tbl_head)
        return -ENOMEM;
    return 0;
}

void COMM_exit_proc_ctrl(void)
{
    unregister_sysctl_table(comm_eproc_tbl_head);
}


extern int COMM_Init(void);
extern void COMM_Exit(void);

static int __init base_mod_init(void)
{
    COMM_Init();

    /* init proc switch */
    if(HI_SUCCESS != COMM_init_proc_ctrl())
    {
        COMM_Exit();
        return -1;
    }

    return 0;
}
static void __exit base_mod_exit(void)
{
    COMM_exit_proc_ctrl();
    COMM_Exit();
}

module_init(base_mod_init);
module_exit(base_mod_exit);

MODULE_LICENSE("Proprietary");

