#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "hi_osal.h"
#include "proc_fs.h"

#define OSAL_PROC_DEBUG 0

static struct osal_list_head list;
static osal_proc_entry_t *proc_entry = NULL;

static int osal_seq_show(struct seq_file *s, void *p)
{
    osal_proc_entry_t *oldsentry = s->private;
    osal_proc_entry_t sentry;
    if (oldsentry == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    memset(&sentry, 0, sizeof(osal_proc_entry_t));
    /* only these two parameters are used */
    sentry.seqfile = s;
    sentry.private = oldsentry->private;
    oldsentry->read(&sentry);
    return 0;
}

static ssize_t osal_procwrite(struct proc_file *file, const char *buf, size_t count, loff_t *ppos)
{
    osal_proc_entry_t *sentry = ((struct seq_file *)(file->private_data))->private;
    if ((sentry != NULL) && (sentry->write != NULL)) {
        return sentry->write(sentry, (char *)buf, count, (long long *)ppos);
    }

    return -ENOSYS;
}

static int osal_procopen(struct inode *inode, struct proc_file *file)
{
    osal_proc_entry_t *sentry = file->pPDE->data;
    if (sentry->open != NULL) {
        sentry->open(sentry);
    }
    return single_open(file, osal_seq_show, sentry);
}

static struct proc_file_operations osal_proc_ops = {
    .open = osal_procopen,
    .read = seq_read,
    .write = osal_procwrite,
    .llseek = seq_lseek,
    .release = single_release
};

osal_proc_entry_t *osal_create_proc(const char *name, osal_proc_entry_t *parent)
{
    struct proc_dir_entry *entry = NULL;
    osal_proc_entry_t *sentry = NULL;
    if (parent == NULL) {
        entry = create_proc_entry(name, 0, NULL);
    } else {
        entry = create_proc_entry(name, 0, parent->proc_dir_entry);
    }
    if (entry == NULL) {
        osal_printk("%s - create_proc_entry failed!\n", __FUNCTION__);
        return NULL;
    }
    sentry = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sentry == NULL) {
        if (parent != NULL) {
            remove_proc_entry(name, parent->proc_dir_entry);
        } else {
            remove_proc_entry(name, NULL);
        }
        osal_printk("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }
    memset(sentry, 0, sizeof(struct osal_proc_dir_entry));
    osal_strncpy(sentry->name, name, sizeof(sentry->name) - 1);
    sentry->proc_dir_entry = entry;
    entry->proc_fops = &osal_proc_ops;
    entry->data = sentry;
    osal_list_add_tail(&(sentry->node), &list);
    return sentry;
}

void osal_remove_proc(const char *name, osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;
    if (name == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, &list, node) {
        if (osal_strcmp(sproc->name, name) == 0) {
            osal_list_del(&(sproc->node));
            kfree(sproc);
            break;
        }
    }
}

osal_proc_entry_t *osal_create_proc_entry(const char *name, osal_proc_entry_t *parent)
{
    parent = proc_entry;

    return osal_create_proc(name, parent);
}

void osal_remove_proc_entry(const char *name, osal_proc_entry_t *parent)
{
    parent = proc_entry;
    osal_remove_proc(name, parent);
    return;
}

osal_proc_entry_t *osal_proc_mkdir(const char *name, osal_proc_entry_t *parent)
{
    struct proc_dir_entry *proc = NULL;
    struct osal_proc_dir_entry *sproc = NULL;
    if (parent != NULL) {
        proc = proc_mkdir(name, parent->proc_dir_entry);
        osal_printk("%s - parent is not NULL!\n", __FUNCTION__);
        // proc = create_proc_entry(name, S_IFDIR | S_IRUGO | S_IXUGO, parent->proc_dir_entry);
    } else {
        proc = proc_mkdir(name, NULL);
        osal_printk("%s - parent is NULL! proc=%p \n", __FUNCTION__, proc);
        // proc = create_proc_entry(name, S_IFDIR | S_IRUGO | S_IXUGO, NULL);
    }
    if (proc == NULL) {
        osal_printk("%s - proc_mkdir failed!\n", __FUNCTION__);
        return NULL;
    }
    // osal_memset(proc, 0x00, sizeof(proc));
    sproc = kmalloc(sizeof(struct osal_proc_dir_entry), GFP_KERNEL);
    if (sproc == NULL) {
        if (parent != NULL) {
            remove_proc_entry(name, parent->proc_dir_entry);
        } else {
            remove_proc_entry(name, NULL);
        }
        osal_printk("%s - kmalloc failed!\n", __FUNCTION__);
        return NULL;
    }
    memset(sproc, 0, sizeof(struct osal_proc_dir_entry));

    osal_strncpy(sproc->name, name, sizeof(sproc->name) - 1);
    sproc->proc_dir_entry = proc;
    // sproc->read = NULL;
    // sproc->write = NULL;
    // proc->proc_fops = NULL;
    proc->data = sproc;
    osal_list_add_tail(&(sproc->node), &list);
    return sproc;
}
void osal_remove_proc_root(const char *name, osal_proc_entry_t *parent)
{
    struct osal_proc_dir_entry *sproc = NULL;
    if (name == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
    if (parent != NULL) {
        remove_proc_entry(name, parent->proc_dir_entry);
    } else {
        remove_proc_entry(name, NULL);
    }
    osal_list_for_each_entry(sproc, &list, node) {
        if (osal_strcmp(sproc->name, name) == 0) {
            osal_list_del(&(sproc->node));
            kfree(sproc);
            break;
        }
    }
}

static int reallocate_larger_buf(struct seq_file *seqfilep, size_t oldcount)
{
    char *largerbuf = (char *)NULL;

    /* check error! */
    if ((seqfilep == NULL) || (seqfilep->buf == NULL)) {
        return -1;
    }

    /* limitted the size of the seq file, small than 1M. */
    if (seqfilep->size >= 256 * PAGE_SIZE) {
        /* error! */
        free(seqfilep->buf);
        seqfilep->buf = (char *)NULL;
        return -1;
    }

    largerbuf = (char *)malloc(seqfilep->size <<= 1);
    if (largerbuf == NULL) {
        /* error! */
        free(seqfilep->buf);
        seqfilep->buf = (char *)NULL;
        return -1;
    }
    memset(largerbuf, 0, seqfilep->size);

    /* get the data before the latest vsprintf. */
    memcpy(largerbuf, seqfilep->buf, oldcount);
    seqfilep->count = oldcount;

    /* free the old small buf. */
    free(seqfilep->buf);
    /* point to the new larger buf. */
    seqfilep->buf = largerbuf;

    return 0;
}

int osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...)
{
    int buflen = 0;
    size_t oldcount = 0;
    va_list arglist;
    bool needreprintf = FALSE;  // if need re-printf.
    struct seq_file *seqfilep = entry->seqfile;

    /* check error! */
    if (seqfilep == NULL) {
        return -1;
    }

    /* memory alloc the buf. */
    if (seqfilep->buf == NULL) {
        seqfilep->buf = (char *)malloc(seqfilep->size = PAGE_SIZE);
        if (seqfilep->buf == NULL) {
            /* no memory, error! */
            return -1;
        }
        memset(seqfilep->buf, 0, seqfilep->size);
        seqfilep->count = 0;
    }

    do {
        oldcount = seqfilep->count;

        /* write the new data. */
        va_start(arglist, fmt);
        buflen = vsnprintf(seqfilep->buf + seqfilep->count, seqfilep->size - seqfilep->count, fmt, arglist);
        va_end(arglist);

        if (buflen < 0) {
            free(seqfilep->buf);
            seqfilep->buf = (char *)NULL;
            return -1;
        }

        if (seqfilep->count + buflen < seqfilep->size) {
            /* succeed write. */
            seqfilep->count += buflen;
            return 0;
        }

        /* need re-printf the data after we get a larger buf. */
        needreprintf = TRUE;

        /* means the buf is too small, we need a larger one. */
        if (reallocate_larger_buf(seqfilep, oldcount) != 0) {
            /* error! */
            return -1;
        }
    } while (needreprintf);

    return -1;
}

void osal_proc_init(void)
{
    OSAL_INIT_LIST_HEAD(&list);
    proc_entry = osal_proc_mkdir("umap", OSAL_NULL);
    if (proc_entry == OSAL_NULL) {
        osal_printk("test init, proc mkdir error!\n");
    }
}
void osal_proc_exit(void)
{
    osal_remove_proc_root("umap", OSAL_NULL);
}

