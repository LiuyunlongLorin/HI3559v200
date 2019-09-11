#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include "hi_osal.h"

#ifndef _IOC_NONE
#define _IOC_NONE   0U
#endif

#ifndef _IOC_WRITE
#define _IOC_WRITE  1U
#endif

#ifndef _IOC_READ
#define _IOC_READ   2U
#endif

#define DRVAL_DEBUG 0

#define GET_FILE(file)            \
    do {                          \
        if (__get_file(file) < 0) \
            return -1;            \
    } while (0)

#define PUT_FILE(file)            \
    do {                          \
        if (__put_file(file) < 0) \
            return -1;            \
    } while (0)

typedef struct osal_coat_dev {
    struct osal_dev osal_dev;
    // struct himedia_device himedia_dev;
} osal_coat_dev_t;

struct osal_private_data {
    struct osal_dev *dev;
    void *data;
    struct osal_poll table;
    int f_ref_cnt;
};

spinlock_t f_lock;

void osal_device_init(void)
{
    spin_lock_init(&f_lock);
}

static int __get_file(struct file *file)
{
    struct osal_private_data *pdata = NULL;

    spin_lock(&f_lock);
    pdata = file->f_priv;
    if (pdata == NULL) {
        spin_unlock(&f_lock);
        return -1;
    }

    pdata->f_ref_cnt++;
    spin_unlock(&f_lock);

    return 0;
}

static int __put_file(struct file *file)
{
    struct osal_private_data *pdata = NULL;

    spin_lock(&f_lock);
    pdata = file->f_priv;
    if (pdata == NULL) {
        spin_unlock(&f_lock);
        return -1;
    }

    pdata->f_ref_cnt--;
    spin_unlock(&f_lock);

    return 0;
}

static int osal_open(struct file *file)
{
    osal_coat_dev_t *coat_dev = NULL;
    struct osal_private_data *pdata = NULL;

    coat_dev = (struct osal_coat_dev *)file->f_inode->i_private;
    if (coat_dev == NULL) {
        osal_printk("%s - get coat_dev device error!\n", __FUNCTION__);
        return -1;
    }

    pdata = (struct osal_private_data *)kmalloc(sizeof(struct osal_private_data), GFP_KERNEL);
    if (pdata == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    if (DRVAL_DEBUG) {
        osal_printk("%s - file->f_priv=%p!\n", __FUNCTION__, pdata);
    }

    memset(pdata, 0, sizeof(struct osal_private_data));

    file->f_priv = pdata;
    pdata->dev = &(coat_dev->osal_dev);
    if (coat_dev->osal_dev.fops->open != NULL) {
        return coat_dev->osal_dev.fops->open((void *)&(pdata->data));
    }
    return 0;
}

static int osal_read(struct file *file, char *buf, size_t size)
{
    struct osal_private_data *pdata = file->f_priv;
    int ret = 0;

    GET_FILE(file);

    if (pdata->dev->fops->read != NULL) {
        ret = pdata->dev->fops->read(buf, (int)size, 0, (void *)&(pdata->data));
    }

    PUT_FILE(file);

    return ret;
}

static int osal_write(struct file *file, const char *buf, size_t size)
{
    struct osal_private_data *pdata = file->f_priv;
    int ret = 0;

    GET_FILE(file);

    if (pdata->dev->fops->write != NULL) {
        ret = pdata->dev->fops->write(buf, (int)size, 0, (void *)&(pdata->data));
    }

    PUT_FILE(file);

    return ret;
}

static off_t osal_llseek(struct file *file, off_t offset, int whence)
{
    struct osal_private_data *pdata = file->f_priv;
    int ret = 0;

    GET_FILE(file);
    if (DRVAL_DEBUG) {
        osal_printk("%s - file->f_priv=%p!\n", __FUNCTION__, pdata);
    }

    if (whence == SEEK_SET) {
        if (pdata->dev->fops->llseek != NULL) {
            ret = pdata->dev->fops->llseek((long)offset, OSAL_SEEK_SET, (void *)&(pdata->data));
        }
    } else if (whence == SEEK_CUR) {
        if (pdata->dev->fops->llseek != NULL) {
            ret = pdata->dev->fops->llseek((long)offset, OSAL_SEEK_CUR, (void *)&(pdata->data));
        }
    } else if (whence == SEEK_END) {
        if (pdata->dev->fops->llseek != NULL) {
            ret = pdata->dev->fops->llseek((long)offset, OSAL_SEEK_END, (void *)&(pdata->data));
        }
    }

    PUT_FILE(file);

    return (loff_t)ret;
}

static int osal_release(struct file *file)
{
    int ret = 0;
    struct osal_private_data *pdata = file->f_priv;

    GET_FILE(file);

    if (DRVAL_DEBUG) {
        osal_printk("%s - file->f_priv=%p!\n", __FUNCTION__, pdata);
    }

    if (pdata->dev->fops->release != NULL) {
        ret = pdata->dev->fops->release((void *)&(pdata->data));
    }
    if (ret != 0) {
        PUT_FILE(file);
        osal_printk("%s - release failed!\n", __FUNCTION__);
        return ret;
    }

    PUT_FILE(file);
    spin_lock(&f_lock);
    if (pdata->f_ref_cnt != 0) {
        osal_printk("%s - release failed!\n", __FUNCTION__);
        spin_unlock(&f_lock);
        return -1;
    }
    kfree(file->f_priv);
    file->f_priv = NULL;
    spin_unlock(&f_lock);

    return 0;
}

static int __osal_unlocked_ioctl(struct file *file, int cmd, unsigned long arg)
{
    int ret = -1;
    struct osal_private_data *pdata = file->f_priv;
    char *kbuf = NULL;

    if (DRVAL_DEBUG) {
        osal_printk("%s - file->f_priv=%p!\n", __FUNCTION__, pdata);
    }

    if ((_IOC_SIZE(cmd) > 8192) || ((_IOC_SIZE(cmd) == 0) && (_IOC_NONE != _IOC_DIR(cmd)))) {
        return -1;
    }

    if ((_IOC_DIR(cmd) != _IOC_NONE) && (((char *)(uintptr_t)arg) == NULL)) {
        osal_printk("%s - Input param err,it is null!\n", __FUNCTION__);
        return -1;
    }

    if (_IOC_DIR(cmd) == _IOC_NONE) {
        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            return -1;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl(cmd, arg, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_WRITE) {
        kbuf = (char *)vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            osal_printk("%s - vmalloc failed!\n", __FUNCTION__);
            return -1;
        }
        if ((char *)(uintptr_t)arg == NULL) {
            vfree(kbuf);
            return -1;
        }
        if (copy_from_user(kbuf, (char *)(uintptr_t)arg, _IOC_SIZE(cmd))) {
            vfree(kbuf);
            return -1;
        }

        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl(cmd, (unsigned long)(uintptr_t)kbuf, (void *)&(pdata->data));
        }
    } else if (_IOC_DIR(cmd) == _IOC_READ) {
        kbuf = vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            osal_printk("%s - vmalloc failed!\n", __FUNCTION__);
            return -1;
        }
        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl(cmd, (unsigned long)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((char *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd))) {
                    vfree(kbuf);
                    return -1;
                }
            }
        }
    } else if (_IOC_DIR(cmd) == (_IOC_READ + _IOC_WRITE)) {
        kbuf = vmalloc(_IOC_SIZE(cmd));
        if (kbuf == NULL) {
            osal_printk("%s - vmalloc failed!\n", __FUNCTION__);
            return -1;
        }

        if (copy_from_user(kbuf, (char *)(uintptr_t)arg, _IOC_SIZE(cmd))) {
            vfree(kbuf);
            return -1;
        }

        if (pdata->dev->fops->unlocked_ioctl == NULL) {
            vfree(kbuf);
            return -1;
        } else {
            ret = pdata->dev->fops->unlocked_ioctl(cmd, (unsigned long)(uintptr_t)kbuf, (void *)&(pdata->data));
            if (ret == 0) {
                if (copy_to_user((char *)(uintptr_t)arg, kbuf, _IOC_SIZE(cmd))) {
                    vfree(kbuf);
                    return -1;
                }
            }
        }
    }

    if (kbuf != NULL) {
        vfree(kbuf);
    }

    return ret;
}

static int osal_unlocked_ioctl(struct file *file, int cmd, unsigned long arg)
{
    int ret = 0;

    GET_FILE(file);

    ret = __osal_unlocked_ioctl(file, cmd, arg);
    PUT_FILE(file);

    return ret;
}

static int osal_fpoll(struct file *file, poll_table *table)
{
    struct osal_private_data *pdata = file->f_priv;
    struct osal_poll *t = &(pdata->table);
    int ret = 0;

    GET_FILE(file);

    if (DRVAL_DEBUG) {
        osal_printk("%s - table=%p, file=%p!\n", __FUNCTION__, table, file);
    }
    t->poll_table = table;
    t->data = file;
    if (pdata->dev->fops->poll != NULL) {
        ret = pdata->dev->fops->poll(t, (void *)&(pdata->data));
    }

    PUT_FILE(file);
    return ret;
}

static struct file_operations_vfs s_osal_fops = {
    .open = osal_open,
    .close = osal_release,
    .read = osal_read,
    .write = osal_write,
    .seek = osal_llseek,
    .ioctl = osal_unlocked_ioctl,
    .poll = osal_fpoll,
};

osal_dev_t *osal_createdev(const char *name)
{
    osal_coat_dev_t *pdev = NULL;
    if (name == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return NULL;
    }
    pdev = (osal_coat_dev_t *)kmalloc(sizeof(osal_coat_dev_t), GFP_KERNEL);
    if (pdev == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return NULL;
    }
    memset(pdev, 0, sizeof(osal_coat_dev_t));
    osal_snprintf(pdev->osal_dev.name, 48, "/dev/%s", name);
    pdev->osal_dev.dev = pdev;
    return &(pdev->osal_dev);
}

int osal_destroydev(osal_dev_t *osal_dev)
{
    osal_coat_dev_t *pdev = NULL;
    if (osal_dev == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    pdev = osal_dev->dev;
    if (pdev == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    kfree(pdev);
    pdev = NULL;

    return 0;
}

int osal_registerdevice(osal_dev_t *osal_dev)
{
    if ((osal_dev == NULL) || (osal_dev->fops == NULL)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    return register_driver(osal_dev->name, &s_osal_fops, 0x0666, osal_dev->dev);
}

void osal_deregisterdevice(osal_dev_t *pdev)
{
    if (pdev == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

    unregister_driver(pdev->name);
}

void osal_poll_wait(osal_poll_t *table, osal_wait_t *wait)
{
    if (DRVAL_DEBUG) {
        osal_printk("%s - call poll_wait +!, table=%p, file=%p\n", __FUNCTION__, table->poll_table, table->data);
    }

    poll_wait ((struct file *)table->data, (wait_queue_head_t *)(wait->wait), table->poll_table);

    if (DRVAL_DEBUG) {
        osal_printk("%s - call poll_wait -!\n", __FUNCTION__);
    }
}

void osal_pgprot_noncached(osal_vm_t *vm)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return;
}

void osal_pgprot_cached(osal_vm_t *vm)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return;
}

void osal_pgprot_writecombine(osal_vm_t *vm)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return;
}

void osal_pgprot_stronglyordered(osal_vm_t *vm)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return;
}

int osal_remap_pfn_range(osal_vm_t *vm, unsigned long addr, unsigned long pfn, unsigned long size)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return 0;
}

int osal_io_remap_pfn_range(osal_vm_t *vm, unsigned long addr, unsigned long pfn, unsigned long size)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return 0;
}

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
int osal_call_usermodehelper_force(char *path, char **argv, char **envp, int wait)
{
    /* not support yet in liteos */
    dprintf("Do not support in liteos\n");

    return 0;
}
#endif

