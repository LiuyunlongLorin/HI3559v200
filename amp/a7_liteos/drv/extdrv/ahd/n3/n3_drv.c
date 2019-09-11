#include <linux/fcntl.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/delay.h>


#include "common.h"
#include "n3_drv.h"
#include "n3_func.h"
#include "litos_i2c.h"
static unsigned int i2cdev_addr = 0x30;

void __I2CWriteByte8(unsigned char chip_addr, unsigned char reg_addr, unsigned char value)
{
    HI_LITOS_I2C_Write(reg_addr,&value,1);
}

unsigned char __I2CReadByte8(unsigned char chip_addr, unsigned char reg_addr)
{
    unsigned char value = 0;
    HI_LITOS_I2C_Read(reg_addr, &value, 1);
    return value;
}

static struct semaphore n3_lock;

static VDEC_N3_CHIP N3_DetectChip( n3_detect_param *pParam )
{
    VDEC_N3_CHIP ret_chip = VDEC_N3_CHIP_UNKNOWN;

    int DEV_ADDR = pParam->devaddr;

    I2CWriteByte(DEV_ADDR, 0xFF, 0x00);
    pParam->chip_id = I2CReadByte(DEV_ADDR, 0xF4);

    if(pParam->chip_id==0x86)
        ret_chip =VDEC_N3_CHIP_N3;

    pParam->chip_def = ret_chip;

    printk("N3_drv : DetectChip(%x) : (%x)(%d)\n",DEV_ADDR,pParam->chip_id,ret_chip);

    return ret_chip;
}


static int n3_fops_open(struct inode * inode, struct file * file)
{
    return 0;
}

static int n3_fops_close(struct inode * inode, struct file * file)
{
    return 0;
}


static n3_init_param init_param;
static n3_detect_param  detect_param;
static n3_fmt_detect_param fmt_detect_param;
static n3_novideo_param novideo_param;
static int arg_int = 0;


int mpu_ioctl(struct file* filp, int cmd, unsigned long arg)
{
    unsigned int __user *argp = (unsigned int __user *)arg;

    switch (cmd)
    {
        case IOC_VDEC_SELECT_CHANNEL:
            down(&n3_lock);
            if (copy_from_user(&arg_int, argp, sizeof(int)))
                return -1;

            printk("N3_drv : IOC_VDEC_SELECT_CHANNEL(%d)\n",arg_int);
            N3_SetInputChannel( &init_param, arg_int );

            up(&n3_lock);
            break;

        case IOC_VDEC_INIT:
            down(&n3_lock);
            printk("N3_drv : IOC_VDEC_INIT\n");
            if (copy_from_user(&init_param, argp, sizeof(n3_init_param)))
                return -1;

            printk("N3_drv : N3_Init\n");
            N3_Init( &init_param, arg_int );

            up(&n3_lock);
            break;

        case IOC_VDEC_DETECT_CHIP:
            down(&n3_lock);
            printk("N3_drv : IOC_VDEC_DETECT_CHIP\n");
            if (copy_from_user(&detect_param, argp, sizeof(n3_detect_param)))
                            return -1;
            N3_DetectChip( &detect_param );
            if(copy_to_user(argp, &detect_param, sizeof(n3_detect_param)))
            {
                printk("IOC_VDEC_DETECT_CHIP error\n");
            }
            up(&n3_lock);
            break;

        case IOC_VDEC_READ_B1_ECtoF0:
            down(&n3_lock);
            printk("N3_drv : IOC_VDEC_READ_B1_ECtoF0\n");
            if (copy_from_user(&fmt_detect_param, argp, sizeof(n3_fmt_detect_param)))
                            return -1;

            N3_DetectFmt( &fmt_detect_param, arg_int );

            if(copy_to_user(argp, &fmt_detect_param, sizeof(n3_fmt_detect_param)))
            {
                printk("IOC_VDEC_READ_B1_ECtoF0 error\n");
            }
            up(&n3_lock);
            break;
        case IOC_VDEC_READ_B1_D8:
            down(&n3_lock);
            if (copy_from_user(&novideo_param, argp, sizeof(n3_novideo_param)))
                            return -1;

            N3_NovideoRead( &novideo_param );

            if(copy_to_user(argp, &novideo_param, sizeof(n3_novideo_param)))
            {
                printk("IOC_VDEC_READ_B1_D8 error\n");
            }
            up(&n3_lock);
            break;

        default:
            printk("N3_drv : invalid ioctl cmd[%x]\n", cmd);
            break;
    }
    return 0;

}

static int mpu_open(struct file* filp)
{
    return 0;
}

static int mpu_release(struct file* filp)
{
    return 0;
}

static const struct file_operations_vfs mpu_fops =
{
    mpu_open,       /* open */
    mpu_release,    /* close */
    0,              /* read */
    0,              /* write */
    0,              /* seek */
    mpu_ioctl       /* ioctl */
#ifndef CONFIG_DISABLE_POLL
    , 0             /* poll */
#endif
};


int n3_dev_register(void)
{
    return register_driver("/dev/n3_dev", &mpu_fops, 0666, 0);
}

int n3_dev_unregister(void)
{
    return unregister_driver("/dev/n3_dev");
}

int __init n3_module_init(unsigned int i2c_num)
{
    int ret = 0;

    HI_LITOS_I2C_Init(i2c_num,i2cdev_addr);

    detect_param.devaddr = i2cdev_addr;
    N3_DetectChip( &detect_param );

    if(n3_dev_register()<0)
    {
        printf("N3_drv Driver fail!\n");
        return -1;
    }
    else
    {
        printf("N3_drv Driver ok!\n");
    }
    printf("N3_drv Driver Loaded\n");

    return 0;
}



void __exit n3_module_exit(void)
{
    if(n3_dev_unregister()<0)
    {
        printf("N3_drv Driver unregister fail!\n");
        //return -1;
    }
    HI_LITOS_I2C_Exit();
    printf("N3_drv : Test Driver Unloaded\n");
}

//module_init(n3_module_init);
//module_exit(n3_module_exit);
