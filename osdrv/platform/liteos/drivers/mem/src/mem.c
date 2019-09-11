
#include <los_sys.h>
#include <fcntl.h>
#include "linux/kernel.h"

static int mem_open(struct file *filep)
{
    //dprintf("mem device is open!\n");

    return 0;
}

static int mem_close(struct file *filep)
{
    //dprintf("mem device is colse!\n");

    return 0;
}

static ssize_t mem_read(FAR struct file *filep, FAR char *buffer, size_t buflen)
{
    //dprintf("mem device is read!\n");

    return 0;
}

static ssize_t mem_write(FAR struct file *filep, FAR const char *buffer, size_t buflen)
{
    //dprintf("mem device is write!\n");

    return 0;
}

static const struct file_operations_vfs mem_dev_ops =
{
  mem_open,  /* open */
  mem_close, /* close */
  mem_read,  /* read */
  mem_write, /* write */
  NULL,          /* seek */
  NULL,  /* ioctl */
#ifndef CONFIG_DISABLE_POLL
  NULL, /* poll */
#endif
  NULL
};


extern int mem_dev_register(void)
{

    return register_driver("/dev/mem", &mem_dev_ops, 0666, 0);
}

