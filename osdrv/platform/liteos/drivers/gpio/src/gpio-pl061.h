#ifndef __GPIO_PL061_H__
#define __GPIO_PL061_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define GPIO_IRQ_NOSHAREABLE 1
#define GPIO_GROUP_DISABLE (~(unsigned int )(0))

typedef void (*irq_handler_func)(unsigned int irq, void *data);
typedef unsigned int gpio_mutex;

typedef struct {
    irq_handler_func handler;
    void * data;
} irq_handler_ctrl;

typedef struct {
    unsigned int irq;
    unsigned int share;
    irq_handler_func irq_handler;
} gpio_irq_handler_tab;


typedef struct {
    unsigned int group_num;
    unsigned int gpio_group_reg_base;
    unsigned int irq;
    irq_handler_ctrl handler_ctrl[GPIO_BIT_NUM];
    gpio_mutex  group_lock;
} gpio_group_ctrl;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

