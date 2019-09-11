#include "linux/kernel.h"
#include "linux/module.h"
#include "linux/device.h"
#include "linux/i2c.h"
#include "stdio.h"
#include "limits.h"

#include "i2c.h"
#include "errno.h"
#include "los_mux.h"

#ifdef LOSCFG_FS_VFS
#include "linux/jiffies.h"
#include "linux/fs.h"

#ifndef MAX_JIFFY_OFFSET
#define MAX_JIFFY_OFFSET    ((LONG_MAX >> 1)-1)
#endif
#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC        1000L
#endif
#ifndef HZ
#define HZ          LOSCFG_BASE_CORE_TICK_PER_SECOND
#endif

static unsigned long msecs_to_jiffies(const unsigned int m)
{
    if ((int)m < 0)
        return MAX_JIFFY_OFFSET;

    return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
}

ssize_t i2cdev_read(struct file * filep, char __user *buf, size_t count)
{
    int retval;
    struct inode * inode = filep -> f_inode ;
    struct i2c_client * client = filep->f_priv;

    retval = i2c_master_recv(client, buf, count);

    return retval;
}

ssize_t i2cdev_write(struct file * filep , const char __user *buf,
        size_t count)
{
    int retval;
    struct inode * inode = filep -> f_inode ;
    struct i2c_client * client = filep->f_priv;

    retval = i2c_master_send(client, buf, count);

    return retval;
}

static int i2cdev_ioctl_rdwr(struct i2c_client *client,
        unsigned long data)
{
    struct i2c_rdwr_ioctl_data *rdwr_arg = NULL;
    struct i2c_msg *rdwr_pa = NULL;
    int res = -EINVAL;
    int i;
    rdwr_arg = (struct i2c_rdwr_ioctl_data *)data;
    if(!rdwr_arg) {
        PRINT_ERR("Invalid argument!\n");
        return res;
    }
    if (rdwr_arg->nmsgs > I2C_RDWR_IOCTL_MAX_MSGS)
        return res;
    rdwr_pa = rdwr_arg->msgs;
    if(!rdwr_pa) {
        PRINT_ERR("Invalid argument!\n");
        return res;
    }
    for (i = 0; i < rdwr_arg->nmsgs; i++) {
        if (rdwr_pa[i].len > 8192)
        return res;
    }
    res = i2c_transfer(client->adapter, rdwr_pa, rdwr_arg->nmsgs);
    return res;
}

static int i2cdev_ioctl(struct file * filep, int cmd, unsigned long arg)
{
    struct inode * inode = filep -> f_inode;
    struct i2c_client * cur_client = filep->f_priv;
    int retval = ENOERR;

    switch (cmd) {
        case IOCTL_SLAVE_FORCE:
        case IOCTL_SLAVE:
            if ((((cur_client->flags & I2C_M_TEN) == 0) && arg > 0xfe) || (arg > 0x3ff)) {
                PRINT_ERR("Not support arg(%0d)!!!", arg);
                retval = -EINVAL;
                break;
            }
            cur_client->addr = arg;
            break;
        case IOCTL_RDWR:
            retval = i2cdev_ioctl_rdwr(cur_client, arg);
            break;
        case IOCTL_16BIT_REG:
            if (arg == 1)
                cur_client->flags |= I2C_M_16BIT_REG;
            else if (arg == 0)
                cur_client->flags &= ~I2C_M_16BIT_REG;
            else {
                PRINT_ERR("error reg width! arg = %d\n", arg);
                retval = -EINVAL;
            }
            break;
        case IOCTL_16BIT_DATA:
            if (arg == 1)
                cur_client->flags |= I2C_M_16BIT_DATA;
            else if (arg == 0)
                cur_client->flags &= ~I2C_M_16BIT_DATA;
            else {
                PRINT_ERR("error data width! arg = %d\n", arg);
                retval = -EINVAL;
            }
            break;
        case IOCTL_TENBIT:
            if (arg == 1)
                cur_client->flags |= I2C_M_TEN;
            else if (arg == 0)
                cur_client->flags &= ~I2C_M_TEN;
            else {
                PRINT_ERR("error device width! arg = %d\n", arg);
                retval = -EINVAL;
            }
            break;
        case IOCTL_PEC:
            if (arg == 0)
                cur_client->flags |= I2C_CLIENT_PEC;
            else if (arg == 1)
                cur_client->flags &= ~I2C_CLIENT_PEC;
            break;
        case IOCTL_RETRIES:
            cur_client->adapter->retries = arg;
            break;
        case IOCTL_TIMEOUT:
            cur_client->adapter->timeout = msecs_to_jiffies(arg * 10);
            break;
        default:
            PRINT_ERR("Not support cmd(%0d)!!!", cmd);
            retval = -EINVAL;
    }
    return retval;
}

static int i2cdev_open(struct file *filep)
{
    struct i2c_client * client = NULL;
    struct inode * inode = filep -> f_inode;
    int adapter_index = (unsigned long)inode->i_private;
    int retval = 0;

    client = malloc(sizeof(struct i2c_client));
    if (NULL == client) {
        PRINT_ERR("Fail to malloc i2c_client-%d!\n",adapter_index);
        return -1;
    }
    memset(client,0,sizeof(struct i2c_client));
    snprintf(client->name, 12, "/dev/i2c-%d", adapter_index);

    /* attach client to adapter */
    retval = client_attach(client, adapter_index);
    if (retval) {
        PRINT_WARN("Fail to attach client-%d!\n",adapter_index);
        free(client);
        return -1; /* change it to the standard error code later */
    }

    /* record the client as file private data */
    filep->f_priv = client;

    return retval;
}

static int i2cdev_release(struct file *filep)
{
    struct i2c_client * client = filep->f_priv;
    int retval = 0;

    if(NULL == client) {
        PRINT_WARN("i2c has not been opened!\n");
        return retval;
    }

    (void)client_deinit(client);

    free(client);

    filep->f_priv = NULL;
    return retval;
}

static const struct file_operations_vfs i2cdev_fops =
{
    i2cdev_open,
    i2cdev_release,
    i2cdev_read,
    i2cdev_write,
    NULL,
    i2cdev_ioctl,
#ifndef CONFIG_DISABLE_POLL
    NULL,
#endif
    NULL
};
#endif /* LOSCFG_FS_VFS */

int /*__init*/ i2cdev_init(void)
{
    int retval = 0, res = 0;
    int adapter_index = 0;
    unsigned long index = 0;
    char name[I2C_NAME_SIZE];

#ifdef LOSCFG_FS_VFS
    /* create /dev/i2c-x device files for the i2c adatpers */
    for (; index < LOSCFG_I2C_ADAPTER_COUNT; index++) {
        snprintf(name, 12, "/dev/i2c-%d", index);
        res = register_driver(name, &i2cdev_fops,
                0666, (void *)index);
        if (res) {
            PRINT_ERR("Fail to register /dev/i2c-%d driver!!!\n",
                    index);
            retval = -1;
            goto init_out;
        }
    }
#endif /* LOSCFG_FS_VFS */
    return 0;
init_out:
    return retval;
}

void i2cdev_exit(void)
{
    char name[I2C_NAME_SIZE];
    int index = 0;
    int res = 0;
    /* unregister drivers later */
#ifdef LOSCFG_FS_VFS
    for (; index < LOSCFG_I2C_ADAPTER_COUNT; index++) {
        snprintf(name, 12, "/dev/i2c-%d", index);
        res = unregister_driver(name);
        if (res) {
            PRINT_ERR("Fail to unregister /dev/i2c-%d driver!!!\n",
                    index);
        }
    }
#endif /* LOSCFG_FS_VFS */
}
module_init(i2cdev_init);
module_exit(i2cdev_exit);

//////////////////////////////
///just used for debug
//////////////////////////////

int i2c_dev_init(void)
{
#ifdef LOSCFG_HOST_TYPE_HIBVT
    extern int i2c_adap_hibvt_init(void);
    (void)i2c_adap_hibvt_init();
#else
    extern int i2c_adap_hisilicon_init(void);
    (void)i2c_adap_hisilicon_init();
#endif
    (void)i2cdev_init();
#ifdef I2C_BUS_DRVIER_DEMO
    extern int cw201x_init(void);
    cw201x_init();
#endif
    return 0;
}
