#ifndef U_SERIAL_H
#define U_SERIAL_H

#ifdef USB_GLOBAL_INCLUDE_FILE
#include USB_GLOBAL_INCLUDE_FILE
#endif
//#include "gadget/tty_serial.h"
#include "gadget/composite.h"

/* be used in struct portmaster */
#define MAX_U_SERIAL_PORTS  1

#define USB_SERIAL_EVENT_READ   0x11
volatile UINT32 g_usbSerialMask;

struct gserial;

struct gs_packet{
    unsigned    packet_size;
    char        *packet_buf;
    char        *packet_get;
    char        *packet_put;
};

struct gs_port{
    struct tty_struct *ttys;
    spinlock_t port_lock;

    struct gserial  *port_usb;

    bool            openclose;  /* open/close in progress */
    struct mtx      lock;       /* portect open/close */
    unsigned char   port_num;

    struct list_head    read_pool;
    int read_started;
    int read_allocated;
    struct list_head    read_queue;
    unsigned            n_read;

    struct list_head    write_pool;
    int write_started;
    int write_allocated;
    struct gs_packet    port_write_buf;

    struct f_acm *acm;

    struct usbd_composite_softc *cdev;
    struct usbd_gadget_device *gadget;

    struct usb_cdc_line_state port_line_state;
};

struct gserial{
    /* be used in f_serila.c to transfer parameter */
    UINT8 data_id;
    UINT8 port_num;

    struct usb_function func;
    /* port is managed in u_serial.c */
    struct gs_port *ioport;

    struct usbd_endpoint *in_ep;
    struct usbd_endpoint *out_ep;

    /* REVISIT avoid this CDC-ACM support harder ...*/
    struct usb_cdc_line_state port_line_state; /* 9600-8-N-1 etc */

    /* notification callbacks */
    void    (*connect)(struct gserial *);
    void    (*disconnect)(struct gserial *);
    int     (*send_break)(struct gserial *, int duration);
 };

struct gs_port *gserial_setup_name_default(const char *sername);
int gserial_connect(struct gserial *, UINT8);
void gserial_disconnect(struct gserial *gser);
static inline struct gs_port *gserial_setup_default(void)
{
    return gserial_setup_name_default("usb");
}

void userial_init(struct gs_port *);
int gserial_alloc_line(struct gs_port *);
int gs_rx_push(void*, char *, size_t);

 #endif

