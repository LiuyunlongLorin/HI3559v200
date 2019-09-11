#ifndef __GPIO_H__
#define __GPIO_H__

#include "linux/interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef void (*irq_func)(unsigned int irq, void *data);

typedef struct {
    unsigned int  groupnumber;
    unsigned int  bitnumber;

    unsigned char value;
#define GPIO_VALUE_HIGH 1
#define GPIO_VALUE_LOW  0
    unsigned char direction;
#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

    unsigned char irq_status;
    unsigned char irq_enable;
#define GPIO_IRQ_ENABLE         1
#define GPIO_IRQ_DISABLE        0
    irq_func  irq_handler;
    unsigned int irq_type;
#define IRQ_TYPE_NONE           0x00000000
#define IRQ_TYPE_EDGE_RISING    0x00000001
#define IRQ_TYPE_EDGE_FALLING   0x00000002
#define IRQ_TYPE_EDGE_BOTH      (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH     0x00000004
#define IRQ_TYPE_LEVEL_LOW      0x00000008
#define IRQ_TYPE_LEVEL_MASK     (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH)
    void * data;
} gpio_groupbit_info;

#define GPIO_SET_DIR      _IOWR('w', 4, gpio_groupbit_info)
#define GPIO_GET_DIR    _IOWR('r', 5, gpio_groupbit_info)
#define GPIO_READ_BIT      _IOWR('r', 6, gpio_groupbit_info)
#define GPIO_WRITE_BIT      _IOWR('w', 7, gpio_groupbit_info)

struct gpio_descriptor {
    unsigned int group_num;
    unsigned int bit_num;
    struct gpio_ops *ops;
    void *private;
};

void gpio_dir_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
void gpio_write(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
unsigned int gpio_read(unsigned char gpio_group, unsigned char gpio_offset);
void gpio_is_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
void gpio_ibe_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
void gpio_iev_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
void gpio_ie_config(unsigned char gpio_group, unsigned char gpio_offset, unsigned char flag);
unsigned int gpio_mis_read(unsigned char gpio_group, unsigned char gpio_offset);
void gpio_ic_clear(unsigned char gpio_group, unsigned char gpio_offset);

int gpio_chip_init(struct gpio_descriptor *gd);
int gpio_chip_deinit(struct gpio_descriptor *gd);

int gpio_get_direction(gpio_groupbit_info * gpio_info);
int gpio_direction_input(gpio_groupbit_info * gpio_info);
int gpio_direction_output(gpio_groupbit_info * gpio_info);
int gpio_get_value(gpio_groupbit_info * gpio_info);
int gpio_set_value(gpio_groupbit_info * gpio_info);

int gpio_irq_register(gpio_groupbit_info * gpio_info);

int gpio_set_irq_type(gpio_groupbit_info * gpio_info);
int gpio_irq_enable(gpio_groupbit_info * gpio_info);
int gpio_get_irq_status(gpio_groupbit_info * gpio_info);

int gpio_clear_irq(gpio_groupbit_info * gpio_info);

//void gpio_resource_lock(void);
//void gpio_unresource_lock(void);

//void gpio_free_irq(gpio_groupbit_info * gpio_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
