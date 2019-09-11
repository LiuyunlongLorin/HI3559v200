
#ifndef _COMPAT_LINUX_I2C_H
#define _COMPAT_LINUX_I2C_H

#include <liteos/i2c.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/mutex.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define COMPAT_I2C_NAME_SIZE    20
#define I2C_NAME_SIZE    COMPAT_I2C_NAME_SIZE

typedef struct module module_t;
typedef struct i2c_adapter i2c_adapter_t;
typedef union i2c_smbus_data i2c_smbus_data_t;
typedef struct i2c_driver i2c_driver_t;
typedef struct list_head list_head_t;
typedef struct device_driver device_driver_t;
typedef struct device device_t;

typedef struct i2c_device_id {
    char name[COMPAT_I2C_NAME_SIZE];
    u_int driver_data; /* Data private to the driver */
}i2c_device_id_t;


/*
 * i2c_algorithm is the interface to hardware solutions using the same bus algorithms
 */
typedef struct i2c_algorithm {
    /* determine the adapter */
    u_long (*functionality) (i2c_adapter_t *);
    int (*smbus_xfer) (i2c_adapter_t *adap, u_short addr,
               u_short flags, char read_write,
               u_char command, int size, i2c_smbus_data_t *data);

    int (*master_xfer)(i2c_adapter_t *adap, struct i2c_msg *msgs, int num);
}i2c_algorithm_t;

/*
 *the the structure of i2c_adapter is  used to identify a physical i2c bus along
 * with the access algorithms that is necessary to access .
 */
 #define NAME_LEN   48
 typedef  int INT32;
struct i2c_adapter {
    INT32 nr;
    char name[NAME_LEN];
    u_int class;          /* classes to allow probing for */
    const i2c_algorithm_t *algo; /* the algorithm to access the bus */
    void *algo_data;
    module_t *owner;
    u_int adapter_lock;

    list_head_t userspace_clients;

    INT32 retries;
    INT32 timeout;
    device_t dev;        /* the adapter device */
};

/* struct i2c_client - represent an I2C slave device */

typedef struct i2c_client {
    u_short flags;               /* div., see below */
    u_short addr;               /* chip address - NOTE: 7bit */
    char name[COMPAT_I2C_NAME_SIZE];
    i2c_adapter_t *adapter;    /* the adapter we sit on */
    i2c_driver_t *driver;          /* and our access routines */
    device_t dev;                   /* the device structure */
    INT32 irq;                                   /* irq issued by device */
    list_head_t detected;
}i2c_client_t;

/* struct i2c_board_info - template for device creation */
typedef struct dev_archdata  dev_archdata_t;
typedef struct device_node   device_node_t;
typedef struct i2c_board_info {
    char        type[COMPAT_I2C_NAME_SIZE];
    u_short    flags;
    u_short    addr;
    void        *platform_data;
    dev_archdata_t   *archdata;
    device_node_t *of_node;
    INT32        irq;
}i2c_board_info_t;

struct i2c_driver {
    u_int class;
    const u_short *address_list;
    list_head_t clients;

    device_driver_t driver;
    const i2c_device_id_t *id_table;

    /* a ioctl like command that can be used to perform specific functionswith the device */
    int (*command)(i2c_client_t *client, unsigned int cmd, void *arg);

    /* Alert callback, for example for the SMBus alert protocol */
    void (*alert)(i2c_client_t *, unsigned int data);

    /* Device detection callback-function for automatic device creation */
    int (*detect)(i2c_client_t *, i2c_board_info_t *);

    /* Standard driver model interfaces by external registered*/
    int (*remove)(i2c_client_t *);
    int (*probe)(i2c_client_t *, const i2c_device_id_t *);
};

struct i2c_devinfo {
    LOS_DL_LIST    list;
    int         busnum;
    i2c_board_info_t   board_info;
};

/* i2c driver API */
#define i2c_add_driver(driver)    i2c_register_driver(THIS_MODULE, driver)     /* use a define to avoid include chaining to get THIS_MODULE */

extern void i2c_del_driver(i2c_driver_t *driver);
extern int i2c_register_driver(module_t *owner, i2c_driver_t *driver);

/* i2c client API */
extern i2c_client_t *i2c_new_device(i2c_adapter_t *adap, i2c_board_info_t const *info);
extern int i2c_register_board_info(int busnum, i2c_board_info_t const *info, unsigned len);
static inline void *i2c_get_clientdata(const i2c_client_t *stdev)
{
    return dev_get_drvdata(&stdev->dev);
}
static inline void i2c_set_clientdata(i2c_client_t *stdev, void *data)
{
    dev_set_drvdata(&stdev->dev, data);
}

/* i2c adapter API */
extern int i2c_register_adapter(i2c_adapter_t *adap);
extern int i2c_add_numbered_adapter(i2c_adapter_t *adap);

#define I2C_BOARD_INFO(dev_type, dev_addr)    .type = dev_type, .addr = (dev_addr)

#define I2C_ADDRS(addr, addrs...)     ((const unsigned short []){ addr, ## addrs, I2C_CLIENT_END })

#define to_i2c_driver(d)       container_of(d, i2c_driver_t, driver)
#define to_i2c_client(d)        container_of(d, i2c_client_t, dev)
#define to_i2c_adapter(d)    container_of(d,i2c_adapter_t, dev)

#define I2C_CLASS_DDC          (1<<3)
#define I2C_CLASS_SPD          (1<<7)
#define I2C_CLASS_HWMON    (1<<0)

#define I2C_CLIENT_TEN       0x10
#define I2C_CLIENT_WAKE    0x80
#define I2C_CLIENT_PEC       0x04
#define I2C_CLIENT_END       0xfffeU

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _COMPAT_LINUX_I2C_H */
