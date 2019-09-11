#ifndef _RNDIS_H
#define _RNDIS_H

#include "implementation/_macro_ref.h"
#include "gadget/u_ether.h"
#include "gadget/skbuff.h"

#define NO_MEM ENOMEM
#define NO_SUPPORT ENOTSUPP

#define get_unaligned(p)    \
({  \
    struct packed_dummy_struct {    \
            typeof(*(p)) __val; \
    } __attribute__((packed)) *__ptr = (void *) (p);    \
    \
    __ptr->__val;   \
})

#define get_unaligned_le16(p)   le16_to_cpu(get_unaligned((uint16_t *)(p)))
#define get_unaligned_le32(p)   le32_to_cpu(get_unaligned((uint32_t *)(p)))

/*  Remote NDIS medium types. */
#define RNDIS_MEDIUM_UNSPECIFIED    0x00000000
#define RNDIS_MEDIUM_802_3  0x00000000

#define USB_RNDIS_MAXIMUM_FRAME_SIZE    1518
#define USB_RNDIS_MAX_TOTAL_SIZE    1558

typedef struct usb_rndis_init_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int MajVersion;
    unsigned int MinVersion;
    unsigned int MaxSize;
} usb_rndis_init_msg_type;

typedef struct usb_rndis_init_cmplt_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int Status;
    unsigned int MajVersion;
    unsigned int MinVersion;
    unsigned int DeviceFlags;
    unsigned int Medium;
    unsigned int MaxPacketsPerTransfer;
    unsigned int MaxSize;
    unsigned int PacketAlignmentFactor;
    unsigned int AFListOffset;
    unsigned int AFListSize;
} usb_rndis_init_cmplt_type;

typedef struct usb_rndis_query_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int OID;
    unsigned int BufferLength;
    unsigned int BufferOffset;
    unsigned int Handle;
} usb_rndis_query_msg_type;

typedef struct usb_rndis_query_cmplt_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int Status;
    unsigned int BufferLength;
    unsigned int BufferOffset;
} usb_rndis_query_cmplt_type;

typedef struct usb_rndis_set_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int OID;
    unsigned int BufferLength;
    unsigned int BufferOffset;
    unsigned int Handle;
} usb_rndis_set_msg_type;

typedef struct usb_rndis_set_cmplt_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int Status;
} usb_rndis_set_cmplt_type;

typedef struct usb_rndis_reset_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int Reserved;
} usb_rndis_reset_msg_type;

typedef struct usb_rndis_reset_cmplt_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int Status;
    unsigned int AddressingReset;
} usb_rndis_reset_cmplt_type;

typedef struct usb_rndis_indicate_status_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int Status;
    unsigned int BufferLength;
    unsigned int BufferOffset;
} usb_rndis_indicate_status_msg_type;

typedef struct usb_rndis_keepalive_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
} usb_rndis_keepalive_msg_type;

typedef struct usb_rndis_keepalive_cmplt_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int ReqID;
    unsigned int Status;
} usb_rndis_keepalive_cmplt_type;

struct usb_rndis_packet_msg_type
{
    unsigned int Type;
    unsigned int Length;
    unsigned int DataOffset;
    unsigned int DataLength;
    unsigned int OOBDataOffset;
    unsigned int OOBDataLength;
    unsigned int NumOOBDataElements;
    unsigned int InfoOffset;
    unsigned int InfoLength;
    unsigned int Handle;
    unsigned int Reserved;
};

enum usb_rndis_state
{
    USB_RNDIS_UNINITIALIZED,
    USB_RNDIS_INITIALIZED,
    USB_RNDIS_DATA_INITIALIZED,
};

typedef struct usb_rndis_resp_t
{
    struct list_head list;
    unsigned char *buf;
    unsigned int length;
    int send;
} usb_rndis_resp_t;

typedef struct usb_rndis_params
{
    unsigned char confignr;
    unsigned char used;
    enum usb_rndis_state state;
    unsigned int medium;
    unsigned int speed;
    unsigned int media_state;

    const unsigned char *host_mac;
    unsigned short *filter;
    struct eth_dev *dev;

    unsigned int vendorID;
    unsigned char max_pkt_per_xfer;
    const char *vendorDescr;
    void (*resp_avail)(void *v);
    void *v;
    struct list_head resp_queue;
} usb_rndis_params;

/* RNDIS Message parser and other useless functions */
int  usb_rndis_register(void (*resp_avail)(void *v), void *v);
void usb_rndis_deregister (int configNr);
int  usb_rndis_msg_parser (u8 configNr, u8 *buf);
int  usb_rndis_set_param_dev (u8 configNr, struct eth_dev *dev, u16 *cdc_filter);
void usb_rndis_add_hdr (struct sk_buff *skb);
int usb_rndis_rm_hdr(struct gether *port, struct sk_buff *skb, struct sk_buff_head *list);
int  usb_rndis_set_param_medium (u8 configNr, u32 medium, u32 speed);
void usb_rndis_set_max_pkt_xfer(u8 configNr, u8 max_pkt_per_xfer);
u8 *usb_rndis_get_next_response (int configNr, u32 *length);
void usb_rndis_free_response (int configNr, u8 *buf);
int  usb_rndis_state (int configNr);
void usb_rndis_set_host_mac (int configNr, const u8 *addr);
int  usb_rndis_signal_connect (int configNr);
int  usb_rndis_signal_disconnect (int configNr);
int usb_rndis_init(void);
void usb_rndis_uninit (int configNr);

#endif
