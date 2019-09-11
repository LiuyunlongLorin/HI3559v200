#ifndef __HI_GPIO_H__
#define __HI_GPIO_H__

#define GPIO_DIR_IN 0
#define GPIO_DIR_OUT 1

typedef struct hi_gpio_grp_info_s
{
    unsigned int  group_num;
    unsigned long base_addr;
    unsigned long base_addr_virtual;
} hi_gpio_grp_info_s;

typedef struct hi_gpio_groupbit_info
{
    unsigned int  group_num;
    unsigned int  bit_num;
    unsigned int  value;
} hi_gpio_groupbit_info;

typedef struct hiGPIO_DATA_S
{
    unsigned char pin;  /**<the pin's No */
    unsigned char dir;  /**<0:input 1:output*/
    unsigned int  value;
} GPIO_DATA_S;

#define GPIO_SET_DIR      _IOWR('w', 4, GPIO_DATA_S)
#define GPIO_GET_DIR      _IOWR('r', 5, GPIO_DATA_S)
#define GPIO_READ_BIT     _IOWR('r', 6, GPIO_DATA_S)
#define GPIO_WRITE_BIT    _IOWR('w', 7, GPIO_DATA_S)

int hi_gpio_set_dir(hi_gpio_groupbit_info* pstGrpBitInfo);

int hi_gpio_get_dir(hi_gpio_groupbit_info* pstGrpBitInfo);

int hi_gpio_write_bit(hi_gpio_groupbit_info* pstGrpBitInfo);

int hi_gpio_read_bit(hi_gpio_groupbit_info* pstGrpBitInfo);

#endif

