#include "linux/kernel.h"
#include "linux/module.h"
#include "linux/device.h"
#include "linux/errno.h"
#include "linux/i2c.h"
#include "i2c.h"
#include "los_base.h"
#include "los_mux.h"
#include "string.h"
#include "stdio.h"


struct i2c_adapter *adapter_array[LOSCFG_I2C_ADAPTER_COUNT];

int i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count)
{
    int retval;

    if (LOS_MuxPend(adapter->adapter_lock, LOSCFG_BASE_CORE_TICK_PER_SECOND * 10)  != LOS_OK) {
        PRINT_ERR("Lock adapter fail!\n");
        retval = -ETIMEDOUT;
        return retval;
    }
    retval = adapter->algo->master_xfer(adapter, msgs, count);
    LOS_MuxPost(adapter->adapter_lock);

    return retval;
}

int i2c_master_send(struct i2c_client *client, const char *buffer, int count)
{
    int retval = 0;
    struct i2c_msg send_msg;
    struct i2c_adapter *cur_adapter = client->adapter;

    send_msg.addr = client->addr;
    send_msg.flags = client->flags & (~I2C_M_RD);
    send_msg.len = count;
    send_msg.buf = (char *)buffer;

    retval = i2c_transfer(cur_adapter, &send_msg, 1);

    return (retval == 1) ? count : retval;
}

int i2c_master_recv(const struct i2c_client *client, char *buffer, int count)
{
    int retval = 0;
    struct i2c_msg recv_msg;
    struct i2c_adapter *adapter = client->adapter;

    recv_msg.addr = client->addr;
    recv_msg.flags = client->flags;

    recv_msg.flags |= I2C_M_RD;
    recv_msg.len = count;
    recv_msg.buf = buffer;

    retval = i2c_transfer(adapter, &recv_msg, 1);

    /*
     * If everything went ok (i.e. 1 msg received), return #bytes received,
     * else error code.
     */
    return (retval == 1) ? count : retval;
}

struct i2c_adapter * get_adapter_by_index(unsigned int index)
{
    struct i2c_adapter *adapter = NULL;

    if (index >= LOSCFG_I2C_ADAPTER_COUNT) {
        PRINT_ERR("argument out of range!\n");
        return adapter;
    }
    adapter = adapter_array[index];
    if (!adapter) {
        PRINT_WARN("driver:host %d is disabled!\n",index);
        return adapter;
    }
    return adapter;
}

const struct i2c_device_id *i2c_match_id(const struct i2c_device_id *devid,
        const struct i2c_client *client)
{
    if (!(devid && client))
        return NULL;

    while (devid->name[0]) {
        if (strcmp(client->name, devid->name) == 0)
            return devid;
        devid++;
    }
    return NULL;
}

static int i2c_dev_match(struct device *dev, struct device_driver *drv)
{
    struct i2c_client   *cur_client = to_i2c_client(dev);
    struct i2c_driver   *cur_driver;

    cur_driver = to_i2c_driver(drv);

    if (i2c_match_id(cur_driver->id_table, cur_client))
        return 1;

    return 0;
}

static int i2c_dev_probe(struct device *dev)
{
    struct i2c_client   *cur_client = to_i2c_client(dev);
    struct i2c_driver   *cur_driver;
    int retval = 0;

    cur_driver = to_i2c_driver(dev->driver);

    retval = cur_driver->probe(cur_client,
            i2c_match_id(cur_driver->id_table, cur_client));

    return retval;
}

struct bus_type i2c_bus_type = {
    .name       = "i2c",
    .match      = i2c_dev_match,
    .probe      = i2c_dev_probe,
#if 0
    .remove     = i2c_device_remove,
    .shutdown   = i2c_device_shutdown,
#endif
};

static UINT32 __i2c_board_lock;

static LOS_DL_LIST __i2c_board_list;
/* refactor the code later to avoid the GPL code match problem */
static void i2c_scan_board_info(struct i2c_adapter *adapter)
{
    struct i2c_devinfo  *dev_info;

    (void)LOS_MuxPend(__i2c_board_lock, LOS_WAIT_FOREVER);
    LOS_DL_LIST_FOR_EACH_ENTRY(dev_info, &__i2c_board_list, struct i2c_devinfo, list) {
        if (dev_info->busnum == adapter->nr && !i2c_new_device(adapter,
                    &dev_info->board_info))
            dprintf("Can't create device at 0x%02x\n",
                    dev_info->board_info.addr);
    }
    (void)LOS_MuxPost(__i2c_board_lock);
}

int i2c_register_board_info(int busnum, struct i2c_board_info const *info, unsigned len)
{
    int retval;

    (void)LOS_MuxPend(__i2c_board_lock, LOS_WAIT_FOREVER);

    for (retval = 0; len; len--, info++) {
        struct i2c_devinfo  *dev_info;

        dev_info = kzalloc(sizeof(*dev_info), GFP_KERNEL);
        if (!dev_info) {
            dprintf("i2c-core: can't register boardinfo!\n");
            retval = -1;
            break;
        }

        dev_info->busnum = busnum;
        dev_info->board_info = *info;
        LOS_ListTailInsert(&__i2c_board_list, &dev_info->list);
    }

    (void)LOS_MuxPost(__i2c_board_lock);

    return retval;
}

struct i2c_client *i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
    struct i2c_client   *cur_client = NULL;
    int retval = 0;

    cur_client = zalloc(sizeof(struct i2c_client));
    if (!cur_client)
        return NULL;

    cur_client->adapter = adap;

    cur_client->dev.platform_data = info->platform_data;

    cur_client->flags = info->flags;
    cur_client->addr = info->addr;
    cur_client->irq = info->irq;

    strlcpy(cur_client->name, info->type, sizeof(cur_client->name));

    cur_client->dev.bus = &i2c_bus_type;
   // i2c_dev_set_name(adap, cur_client);

    cur_client->dev.name = cur_client->name;

    retval = device_register(&cur_client->dev);
    if (retval)
        goto err;

    return cur_client;

err:
    dprintf("Failed to register i2c client %s at 0x%02x (%d)\n",
            cur_client->name, cur_client->addr, retval);
    free(cur_client);

    return NULL;
}

int i2c_register_driver(struct module *owner, struct i2c_driver *driver)
{
    int retval;

    (void)owner;

    driver->driver.bus = &i2c_bus_type;

    retval = driver_register(&driver->driver);
    if (retval)
        return retval;

    return 0;
}

void i2c_del_driver(struct i2c_driver *driver)
{
    /* i2c_for_each_dev(driver, __process_removed_driver); */

    driver_unregister(&driver->driver);
}

int i2c_register_adapter(struct i2c_adapter *adap)
{
    int index;
    int retval;
    int found = 0;

    index = adap->nr;
    for (index = 0;index < LOSCFG_I2C_ADAPTER_COUNT;index++) {
        if (!adapter_array[index]) {
            found = 1;
            break;
        }
    }
    if (found) {
        adapter_array[index] = adap;
        adap->nr = index;
        (void)LOS_MuxCreate(&(adapter_array[index]->adapter_lock));
        retval = LOS_OK;
    } else {
        retval = -1; /* change it to the standard error code later */
        return retval;
    }

    i2c_scan_board_info(adap);

    return retval;

}

int i2c_add_numbered_adapter(struct i2c_adapter *adap)
{
    int index;
    int retval = LOS_OK;

    index = adap->nr;
    if (index >= LOSCFG_I2C_ADAPTER_COUNT) {
        PRINT_ERR("adapter number out of range!");
        retval = -1; /* change it to the standard error code later */
        goto error;
    }
    if (adapter_array[index]) {
        PRINT_ERR("adapter already registered!");
        retval = -1; /* change it to the standard error code later */
        goto error;
    }
    adapter_array[index] = adap;
    (void)LOS_MuxCreate(&(adapter_array[index]->adapter_lock));

    i2c_scan_board_info(adap);
error:
    return retval;
}

/* client_attach: attach cliet to adapter
 * @client: i2c client
 * @adapter_index: i2c host number
 * return:
 * 0 : success
 * -1: fail
 */
int client_attach(struct i2c_client * client, int adapter_index)
{
    struct i2c_adapter *adapter = NULL;
    int retval = 0;
    adapter = get_adapter_by_index(adapter_index);
    if (adapter == NULL) {
        return -1;
    }
    client->adapter = adapter;
    return retval;
}

int client_deinit(struct i2c_client * client)
{
    struct i2c_client * cur_client = client;

    cur_client->adapter = NULL;
    cur_client->driver = NULL;
    return 0;
}

int i2c_init(void)
{
    int retval = -1, i = 0;
    static int reinit = 0;

    if (reinit) {
        PRINT_ERR("adapter initialized!\n");
        return -1;
    }
    else
        reinit++;

    for(; i < LOSCFG_I2C_ADAPTER_COUNT; i++)
        adapter_array[i] = NULL;

    retval = bus_register(&i2c_bus_type);
    if (retval)
        return retval;

    retval = LOS_MuxCreate(&__i2c_board_lock);
    if (LOS_OK != retval)
    {
        return LOS_ERRNO_DRIVER_DEVICE_INITIALFAIL;
    }

    LOS_ListInit(&__i2c_board_list);

    return 0;
}

postcore_initcall(i2c_init);
