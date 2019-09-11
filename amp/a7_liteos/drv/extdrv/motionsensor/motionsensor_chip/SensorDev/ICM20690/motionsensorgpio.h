#ifndef GPIO_H
#define GPIO_H
#include "hi_type.h"
#ifndef __HuaweiLite__
#include<linux/gpio.h>
#else
#include "gpio.h"
#endif

#define gpio_num(gpio_chip_num, gpio_offset_num)  ((gpio_chip_num) * 8 + (gpio_offset_num))

#ifndef __HuaweiLite__
HI_S32 gpio_init(void);
HI_S32 gpio_deinit(void);
#else
HI_S32 gpio_init(struct gpio_descriptor *gd);
HI_S32 gpio_deinit(struct gpio_descriptor *gd);
#endif
#endif
