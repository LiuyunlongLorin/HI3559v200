/*
 * Remote Network Driver Interface Specification (RNDIS)
 * definitions of the magic numbers used by this protocol
 */

#include "gadget/u_ether.h"

#define NO_MEM ENOMEM
#define NO_SUPPORT ENOTSUPP

/* Device descriptor */
struct rndis_hs_function_descriptor {
    struct usb_interface_assoc_descriptor ifcad;
    struct usb_interface_descriptor ifcd;
    struct usb_cdc_header_desc cdc_desc;
    struct usb_cdc_call_mgmt_descriptor cdc_call_desc;
    struct usb_cdc_acm_descriptor cdc_acm_desc;
    struct usb_cdc_union_desc cdc_union_desc;
    struct usb_endpoint_descriptor nepd;
#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
    struct usb_endpoint_ss_comp_descriptor ncompd;
#endif
    struct usb_interface_descriptor ifdd;
    struct usb_endpoint_descriptor iepd;
#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
    struct usb_endpoint_ss_comp_descriptor icompd;
#endif
    struct usb_endpoint_descriptor oepd;
#ifdef LOSCFG_DRIVERS_USB3_DEVICE_CONTROLLER
    struct usb_endpoint_ss_comp_descriptor ocompd;
#endif
} __packed;

struct rndis_hs_descriptor {
    struct usb_config_descriptor        *rndis_config;
    struct rndis_hs_function_descriptor *rndis_func;
};


/* Device Flags */
#define RNDIS_DF_CONNECTIONLESS 0x00000001U

#define RNDIS_MESSAGE_COMPLETION    0x80000000
#define RNDIS_MESSAGE_PACKET    0x00000001
#define RNDIS_MESSAGE_HALT  0x00000003
#define RNDIS_MESSAGE_INDICATE  0x00000007

#define RNDIS_MESSAGE_RESET 0x00000006
#define RNDIS_MESSAGE_RESET_C   (RNDIS_MESSAGE_RESET | RNDIS_MESSAGE_COMPLETION)
#define RNDIS_MESSAGE_SET   0x00000005
#define RNDIS_MESSAGE_SET_C (RNDIS_MESSAGE_SET | RNDIS_MESSAGE_COMPLETION)
#define RNDIS_MESSAGE_QUERY 0x00000004
#define RNDIS_MESSAGE_QUERY_C   (RNDIS_MESSAGE_QUERY | RNDIS_MESSAGE_COMPLETION)
#define RNDIS_MESSAGE_KEEPALIVE 0x00000008
#define RNDIS_MESSAGE_KEEPALIVE_C   (RNDIS_MESSAGE_KEEPALIVE | RNDIS_MESSAGE_COMPLETION)
#define RNDIS_MESSAGE_INIT  0x00000002
#define RNDIS_MESSAGE_INIT_C    (RNDIS_MESSAGE_INIT | RNDIS_MESSAGE_COMPLETION)

#define RNDIS_MEDIA_STATE_CONNECTED 0x00000000
#define RNDIS_MEDIA_STATE_DISCONNECTED  0x00000001

#define RNDIS_MAC_OPTION_RECEIVE_SERIALIZED 0x00000002
#define RNDIS_MAC_OPTION_FULL_DUPLEX 0x00000010

struct f_rndis {
    struct gether ioport;
    unsigned char ctrl_id, data_id;
    unsigned char ethaddr[6];
    unsigned int vendorID;
    const char *manufacturer;
    int config;

    struct usbd_endpoint *notify;
    unsigned int notify_ep_enable;
    struct usbd_request *notify_req;
    atomic_t notify_count;
};

/* Remote NDIS Versions */
#define RNDIS_MINOR_VERSION 0x00000000
#define RNDIS_MAJOR_VERSION 0x00000001

/* General (Required) Objects */
#define RNDIS_GEN_SUPPORTED_LIST    0x00010101
#define RNDIS_GEN_HARDWARE_STATUS   0x00010102
#define RNDIS_GEN_MEDIA_SUPPORTED   0x00010103
#define RNDIS_GEN_MAC_OPTIONS   0x00010113
#define RNDIS_GEN_PHYSICAL_MEDIUM   0x00010202
#define RNDIS_GEN_MEDIA_CONNECT_STATUS  0x00010114
#define RNDIS_GEN_VENDOR_DRIVER_VERSION 0x00010116
#define RNDIS_GEN_MEDIA_IN_USE  0x00010104
#define RNDIS_GEN_MAXIMUM_LOOKAHEAD 0x00010105
#define RNDIS_GEN_MAXIMUM_FRAME_SIZE    0x00010106
#define RNDIS_GEN_LINK_SPEED	    0x00010107

#define RNDIS_STATUS_PENDING    0x00000103
#define RNDIS_STATUS_SUCCESS    0x00000000
#define RNDIS_STATUS_NOT_SUPPORTED  0xc00000BB

#define RNDIS_GEN_VENDOR_ID 0x0001010C
#define RNDIS_GEN_VENDOR_DESCRIPTION    0x0001010D
#define RNDIS_GEN_CURRENT_PACKET_FILTER 0x0001010E
#define RNDIS_GEN_MAXIMUM_TOTAL_SIZE    0x00010111
#define RNDIS_GEN_TRANSMIT_BUFFER_SPACE 0x00010108
#define RNDIS_GEN_RECEIVE_BUFFER_SPACE  0x00010109
#define RNDIS_GEN_TRANSMIT_BLOCK_SIZE   0x0001010A
#define RNDIS_GEN_RECEIVE_BLOCK_SIZE    0x0001010B

/* 802.3 Objects (Ethernet) */
#define RNDIS_802_3_XMIT_ONE_COLLISION  0x01020102
#define RNDIS_802_3_XMIT_MORE_COLLISIONS    0x01020103
#define RNDIS_802_3_RCV_ERROR_ALIGNMENT 0x01020101

#define RNDIS_802_3_MAC_OPTIONS 0x01010105
#define RNDIS_802_3_MAXIMUM_LIST_SIZE   0x01010104
#define RNDIS_802_3_MULTICAST_LIST  0x01010103
#define RNDIS_802_3_CURRENT_ADDRESS 0x01010102
#define RNDIS_802_3_PERMANENT_ADDRESS   0x01010101

/* Required statistics OIDs */
#define RNDIS_GEN_RCV_NO_BUFFER 0x00020105
#define RNDIS_GEN_RCV_ERROR 0x00020104
#define RNDIS_GEN_XMIT_ERROR    0x00020103
#define RNDIS_GEN_RCV_OK 0x00020102
#define RNDIS_GEN_XMIT_OK 0x00020101

#define RNDIS_STATUS_MEDIA_CONNECT  0x4001000B
#define RNDIS_STATUS_MEDIA_DISCONNECT   0x4001000C
