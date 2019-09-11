#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
//#include <asm/system.h>
#include <asm/io.h>

#include "motionsensorgpio.h"
#include "motionsensordev.h"

#ifndef __HuaweiLite__
HI_S32 gpio_init(void)
{
	HI_S32 s32Ret;


	s32Ret = gpio_request(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET), "icm20690_irq");
	if (s32Ret < 0) {
		print_info("irq GPIO request failed: %d", s32Ret);
		return s32Ret;
	}

	gpio_direction_input(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));
	
    return 0;
}
HI_S32 gpio_deinit(void)
{
     gpio_free(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));
     return 0;
}
#else
HI_S32 gpio_init(struct gpio_descriptor *gd)
{

	gd->group_num = INT_GPIO_CHIP;  //GPIO7
	gd->bit_num = INT_GPIO_OFFSET;   //GPIO3
	
	gpio_chip_init(gd);

	return HI_SUCCESS;
}
HI_S32 gpio_deinit(struct gpio_descriptor *gd)
{
	return gpio_chip_deinit(gd);
}

#endif

