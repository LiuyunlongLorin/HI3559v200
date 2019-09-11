#ifndef __COMPOSITE_H__
#define __COMPOSITE_H__

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#endif

#include <controller/usb_device/usb_gadget.h>
#include "implementation/_bsd_kernel_ref.h"

#define    HSETW(ptr, val) ptr = { (uint8_t)(val), (uint8_t)((val) >> 8) }
#define    HSETDW(ptr, val) ptr = { (uint8_t)(val), (uint8_t)((val) >> 8), \
                    (uint8_t)((val) >> 16), (uint8_t)((val) >> 24)}
/* This is a flag that device strings struct is ended. */
#define USBD_DEVICE_STRINGS_END {0,NULL}

#define NO_DEV ENODEV
#define INVAL_ARGU EINVAL
#define MAX_CONFIG_INTERFACES_NUM 16    /* arbitrary; max 255 */

struct usbd_composite_softc;

/* predefined index for usb_composite_driver */
enum {
    USBD_GADGET_MANUFACTURER_IDX = 0,
    USBD_GADGET_PRODUCT_IDX,
    USBD_GADGET_SERIAL_IDX,
    USBD_GADGET_FIRST_AVAIL_IDX,
};


struct usbd_configuration {
#define MAC_LABEL_LENGTH 30 /* Must be greater than or eque to 10. */
    const char *label;      /* The label length must be small than the MAC_LABEL_LENGTH. */
    void    *usbd_config_descriptor;
    u32      config_desc_length;

    u8      bConfigurationValue;
    u8      iConfiguration;
    u8      bmAttributes;
    u16     MaxPower;

    struct usbd_composite_softc *cdev;

    struct list_head    list;
    struct list_head    functions;
    u8                  next_interface_id;
    struct usb_function *interface[MAX_CONFIG_INTERFACES_NUM];
};

struct usbd_string {
    u8          id;
    const char  *s;
};

struct usbd_gadget_strings {
    u16                 language;
    struct usbd_string  *strings;
};

struct usbd_composite_driver {
    const char                          *name;
    const struct usb_device_descriptor  *dev;
    struct usbd_gadget_strings          *strings;
    u8                                  max_speed;
    unsigned                            needs_serial:1;

    int     (*bind)(struct usbd_composite_softc *);
    void     (*unbind)(struct usbd_composite_softc *);

    struct usbd_gadget_driver       gadget_driver;
};

#define USBD_DRIVER_DATA_INDEX 2
struct usbd_driver_data {
    char *name;
    void *data;
};

typedef struct usbd_composite_softc{
    struct usbd_driver_data drv_data[USBD_DRIVER_DATA_INDEX];

    struct usbd_gadget_device* gadget;  /* gadget information supports by udc */
    struct usbd_request ctrlreq;

    struct usbd_configuration *config_dec;  /* save other's descriptors */

    struct usb_device_descriptor    desc;
    struct list_head    configs;    /* save all configuration information */
    struct list_head    gstrings;
    struct usbd_composite_driver    driver;
    u8      next_string_id;
    char    *def_manufacturer;

    struct cv           task_cv;
    struct mtx          task_mtx;
    spinlock_t  lock;
}composite_t;

/*-----------------------------------------------------------------------*/
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_PER_BYTE   8
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

static inline void set_bit(unsigned int nr, volatile unsigned long * addr)
{
    unsigned long mask = 1UL << (nr&0x1f);
    addr += (nr >> 5);
    *addr |= mask;
}

static inline int test_bit(unsigned int nr, volatile unsigned long * addr)
{
    return (1UL & (addr[nr>>5] >> (nr & 0x1f)));
}

struct usb_function{
    const char *name;
    struct usbd_gadget_strings  **strings;
    struct usb_descriptor   **descriptors;
    struct usb_descriptor   **hs_descriptors;

    struct usbd_configuration   *config;

    /* configuration manegement */
    int     (*bind)(struct usbd_configuration *, struct usb_function *);
    void    (*unbind)(struct usbd_composite_softc *, struct usb_function *);
    void    (*free_func)(struct usbd_composite_softc *);

    /* runtime state management */
    int     (*set_alt)(struct usbd_composite_softc*, unsigned intf, unsigned alt);
    int     (*get_alt)(struct usb_function *, unsigned interface);
    void    (*disable)(struct usbd_composite_softc *);
    int     (*setup)(struct usbd_composite_softc *, const struct usb_device_request *);
    void    (*suspend)(struct usb_function *);
    void    (*resume)(struct usb_function *);
    void    (*reset)(struct usbd_composite_softc *);

    /* USB 3.0 additions */
    int     (*get_status)(struct usb_function *);
    int     (*func_suspend)(struct usb_function *,
            unsigned char suspend_opt);

    /* private: */
    /* internals */
    struct list_head list;
    unsigned long endpoints[BITS_TO_LONGS(32)];
};

struct usbd_function_driver {
    const char *name;
    struct list_head list;
    int (*driver_init)(struct usbd_composite_softc *cdev);
    struct usb_function *(*alloc_func)(struct usbd_composite_softc *cdev);
};

#define USBD_FUNCTION(_name, _driver_init, _func_alloc) \
    static struct usbd_function_driver _name ## usbd_func = { \
        .name           = #_name,          \
        .driver_init    = _driver_init,    \
        .alloc_func     = _func_alloc,     \
    };

int usbd_function_register(struct usbd_function_driver *);
void usbd_function_unregister(struct usbd_function_driver *);

#define USBD_FUNCTION_INIT(_name, _driver_init, _func_alloc) \
    USBD_FUNCTION(_name, _driver_init, _func_alloc) \
    int _name ## mod_init(void)  \
    {   \
        return usbd_function_register(&_name ## usbd_func); \
    }   \
    void _name ## mod_exit(void)  \
    {   \
        usbd_function_unregister(&_name ## usbd_func);   \
    }


/*-----------------------------------------------------------------------*/
#define to_composite_softc(d)   container_of(d, struct usbd_composite_softc, driver)
#define to_composite_driver(gd) container_of(gd, struct usbd_composite_driver, gadget_driver)

extern char *devname_get(void);
extern struct module_data *bus_get_device(const char *modname);
int usbd_set_driver_data(struct usbd_composite_softc *cdev, void *name, void *data);
void *usbd_get_driver_data(struct usbd_composite_softc *cdev, char *name);
void usbd_clear_driver_data(struct usbd_composite_softc *cdev, char *name);

int composite_device_init(struct usbd_composite_softc *);
void composite_request_complete(struct usbd_endpoint*,
                                               struct usbd_request*);
int usbd_add_config_only(struct usbd_composite_softc *cdev,
                struct usbd_configuration *configs);
int usbd_driver_init(struct usbd_composite_softc *cdev, const char *name);
struct usb_function *usbd_get_function(struct usbd_composite_softc *cdev, const char *name);
int usbd_add_function(struct usbd_configuration *config, struct usb_function *func);
int usbd_interface_id(struct usbd_configuration *c, struct usb_function *f);
int usbd_string_id(struct usbd_composite_softc *cdev);
void usbd_remove_function(struct usbd_configuration *config, struct usb_function *func);
void usbd_put_function(struct usbd_configuration *config, struct usb_function *func);


#endif


