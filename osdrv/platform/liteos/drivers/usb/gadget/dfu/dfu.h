/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#ifndef __USB_DFU_H_
#define __USB_DFU_H_

#include <linux/list.h>

#define USB_DFU_RAM_ADDR    1
enum usb_dfu_op {
    DFU_READ = 1,
    DFU_WRITE,
    DFU_SIZE,
};

struct ram_data {
    void        *start;
    unsigned int    size;
};

#define DFU_NAME_LEN            32
#ifndef CONFIG_USB_DFU_DATA_BUF_SIZE
#define CONFIG_USB_DFU_DATA_BUF_SIZE        (1024*1024*8)    /* 8 MiB */
#endif
#ifndef USB_DFU_DEFAULT_POLL_TIMEOUT
#define USB_DFU_DEFAULT_POLL_TIMEOUT 0
#endif
#ifndef USB_DFU_MANIFEST_POLL_TIMEOUT
#define USB_DFU_MANIFEST_POLL_TIMEOUT    USB_DFU_DEFAULT_POLL_TIMEOUT
#endif

struct usb_dfu_entity {
    char            dfu_name[DFU_NAME_LEN];
    int                     alter;
    unsigned long           max_buf_size;
    int layout;
    struct ram_data ram;

    long (*get_medium_size)(struct usb_dfu_entity *dfu);

    int (*read_medium)(struct usb_dfu_entity *dfu,
            unsigned long long offset, void *buf, long *len);

    int (*write_medium)(struct usb_dfu_entity *dfu,
            unsigned long long offset, void *buf, long *len);

    int (*flush_medium)(struct usb_dfu_entity *dfu);
    unsigned int (*poll_timeout)(struct usb_dfu_entity *dfu);

    void (*free_entity)(struct usb_dfu_entity *dfu);

    struct list_head list;

    unsigned long long offset;
    unsigned int blk_seq_num;
    unsigned char *buf;
    unsigned char *buf_start;
    unsigned char *buf_end;
    long real_left;
    long buf_left;

    unsigned int inited;
};

int usb_dfu_config_entities(char *str, char *type, char *devstr);
void usb_dfu_free_entities(void);
int usb_dfu_get_alt_number(void);
struct usb_dfu_entity *usb_dfu_get_entity(int alter);
int usb_dfu_init_env_entities(char *type, char *envstr,char *devstr);
unsigned char *usb_dfu_get_buf(struct usb_dfu_entity *dfu);
void usb_dfu_free_buf(void);
int usb_dfu_get_buf_size(void);
bool usb_dfu_usb_get_reset(void);

int usb_dfu_read(struct usb_dfu_entity *de, void *buf, int size, unsigned int blk_seq_num);
int usb_dfu_write(struct usb_dfu_entity *de, void *buf, int size, unsigned int blk_seq_num);
int usb_dfu_flush(struct usb_dfu_entity *de, void *buf, int size, unsigned int blk_seq_num);

extern int usb_dfu_fill_entity_ram(struct usb_dfu_entity *dfu, char *devstr, char *str);
extern unsigned int g_dfu_update_flag;
extern unsigned int usb_dfu_update_status(void);
extern unsigned long long g_dfu_update_size;
extern unsigned long long *usb_dfu_update_size_get(void);

#define list_for_each_entry_safe_reverse(pos, n, head, member)        \
    for (pos = list_entry((head)->prev, typeof(*pos), member),    \
        n = list_entry(pos->member.prev, typeof(*pos), member);    \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.prev, typeof(*n), member))

#endif /* __USB_DFU_H_ */
