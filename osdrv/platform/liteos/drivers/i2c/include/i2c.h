#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include "linux/rwsem.h"
#include "linux/i2c.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOSCFG_I2C_ADAPTER_COUNT
#define LOSCFG_I2C_ADAPTER_COUNT 16
#endif

#ifndef LOSCFG_DEBUG_VERSION
#define LOSCFG_DEBUG_VERSION
#endif
#ifdef LOSCFG_DEBUG_VERSION
    #define i2c_err(x...) \
        do { \
                    dprintf("%s->%d: ", __func__, __LINE__); \
                    dprintf(x); \
                    dprintf("\n"); \
            } while (0)
    #undef I2C_DEBUG
#else
    #define i2c_err(x...) do { } while (0)
    #undef I2C_DEBUG
#endif

#ifdef I2C_DEBUG
    #define i2c_msg(x...) \
        do { \
                    dprintf("%s (line:%d) ", __func__, __LINE__); \
                    dprintf(x); \
            } while (0)
#else
    #define i2c_msg(x...) do { } while (0)
#endif

#ifdef LOSCFG_FS_VFS
#define IOCTL_RETRIES     0x0701
#define IOCTL_TIMEOUT     0x0702
#define IOCTL_SLAVE       0x0703
#define IOCTL_SLAVE_FORCE 0x0706
#define IOCTL_TENBIT      0x0704
#define IOCTL_FUNCS       0x0705
#define IOCTL_RDWR        0x0707
#define IOCTL_PEC         0x0708
#define IOCTL_SMBUS       0x0720
#define IOCTL_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define IOCTL_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */
/* /dev/i2c-x ioctl commands */
#define I2C_RETRIES     IOCTL_RETRIES
#define I2C_TIMEOUT     IOCTL_TIMEOUT
#define I2C_SLAVE       IOCTL_SLAVE
#define I2C_SLAVE_FORCE IOCTL_SLAVE_FORCE
#define I2C_TENBIT      IOCTL_TENBIT
#define I2C_FUNCS       IOCTL_FUNCS
#define I2C_RDWR        IOCTL_RDWR
#define I2C_PEC         IOCTL_PEC
#define I2C_SMBUS       IOCTL_SMBUS
#define I2C_16BIT_REG   IOCTL_16BIT_REG/* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  IOCTL_16BIT_DATA/* 16BIT DATA WIDTH */
#endif /* LOSCFG_FS_VFS */

struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;
    unsigned int nmsgs;
};

#define  I2C_RDWR_IOCTL_MAX_MSGS    42

struct i2c_driver_data {
    struct i2c_adapter* adapter;
    volatile unsigned char  *regbase;
    unsigned int      clk;
    unsigned int        freq;
    unsigned int irq;
    struct i2c_msg *msgs;
    unsigned int        msg_num;
    unsigned int        msg_idx;
    UINT32 lock;
    void    *private;
};

/* open for User */
extern int client_attach(struct i2c_client * client, int adapter_index);
extern int client_deinit(struct i2c_client * client);

extern int i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count);
extern int i2c_master_send(struct i2c_client *client, const char *buffer, int count);
extern int i2c_master_recv(const struct i2c_client *client, char *buffer, int count);

extern int i2c_add_numbered_adapter(struct i2c_adapter *adap);
extern int i2c_register_adapter(struct i2c_adapter *adap);
extern int hi_i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int count);
extern int hi_i2c_master_send(struct i2c_client *client, const char *buf, int count);
extern int hi_i2c_master_recv(const struct i2c_client *client, char *buf, int count);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
