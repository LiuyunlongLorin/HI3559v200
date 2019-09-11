#ifndef __TTY_SERIAL_H__
#define __TTY_SERIAL_H__

#include "los_event.h"
#include "linux/spinlock.h"
#include "gadget/composite.h"

#define TTY_MTX_LOCK(sc) mtx_lock(&(sc)->lock)
#define TTY_MTX_UNLOCK(sc) mtx_unlock(&(sc)->lock)

/* Define usb serial gadget path. */
#define TTY_USB_SERIAL "/dev/ttyGS0"

struct tty_operations {
    int  (*open)(struct usbd_composite_softc *);
    void (*close)(struct usbd_composite_softc *);
    int  (*write)(struct usbd_composite_softc*,
            const char *buf, int count);
};

struct tty_struct {
    const struct tty_operations *ops;

    EVENT_CB_S   usb_serial_event;  /* wait while read_pool is empty */
    bool is_open;
    spinlock_t tty_lock;
};

int tty_init(struct usbd_composite_softc *fserial);

#endif
