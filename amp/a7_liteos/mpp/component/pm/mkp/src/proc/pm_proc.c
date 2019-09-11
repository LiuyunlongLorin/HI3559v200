#ifdef LOSCFG_FS_PROC
#include "proc_fs.h"
#include "linux/seq_file.h"
#include "pm_device.h"

extern struct pm_device pm_dev;
extern int get_media_profile(void);

#define _1M_ 1000000
static int pm_proc_show(struct seq_file* sfile, void* v)
{
    struct pm_domain_device* cpu = pm_dev.cpu_domain_device;
    struct freq_device* cpu_freq = cpu->freq_dev;
    struct avs_device* cpu_avs = cpu->avs_dev;
    struct pm_domain_device* media = pm_dev.media_domain_device;
    struct avs_device* media_avs = media->avs_dev;
    seq_printf(sfile, "\n==================================================\n");
    seq_printf(sfile, "PM status:\n");
    seq_printf(sfile, "  domain_num    = %0d;\n", pm_dev.domain_num);
    seq_printf(sfile, "  pm_enable     = %0d;\n", pm_dev.pm_enable);
    seq_printf(sfile, "  dvfs_enable   = %0d;\n", cpu_freq->dvfs_enable);
    seq_printf(sfile, "CPU domain:\n");
    seq_printf(sfile, "  cur_freq      = %0dMHz [%0d~%0d];\n" \
               , cpu->cur_freq / _1M_, cpu_freq->min_freq / _1M_, cpu_freq->max_freq / _1M_);
    seq_printf(sfile, "  cur_volt      = %0duV [%0d~%0d];\n", \
               cpu->cur_volt, cpu_avs->cur_volt_min, cpu_avs->cur_volt_max);
    seq_printf(sfile, "MEDIA domain:\n");
    seq_printf(sfile, "  profile_num   = %0d;\n", media_avs->profile_num);
    seq_printf(sfile, "  cur_profile   = %0d;\n", get_media_profile());
    seq_printf(sfile, "  cur_volt      = %0duV [%0d~%0d];\n", \
               media->cur_volt, media_avs->cur_volt_min, media_avs->cur_volt_max);
    seq_printf(sfile, "==================================================\n");
    return 0;
}

int pm_proc_open(struct inode* inode, struct proc_file* pf)
{
    return single_open(pf, pm_proc_show, NULL);
}

static const struct proc_file_operations pm_proc_fops = {
    .open     = pm_proc_open,
    .read     = seq_read,
    .llseek   = seq_lseek,
    .release  = single_release
};

#define PM_PROC_NAME "pm"
int pm_proc_init(void)
{
    struct proc_dir_entry* p;
    p = create_proc_entry(PM_PROC_NAME, 0, NULL);
    if (!p) {
        dprintf("Create pm proc fail!\n");
        return -1;
    }
    p->proc_fops = &pm_proc_fops;
    return 0;
}

#endif /* LOSCFG_FS_PROC */

