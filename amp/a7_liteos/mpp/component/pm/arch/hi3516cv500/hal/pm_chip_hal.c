
#include "hi_osal.h"
#include "pm_chip_hal.h"
#include "pm_drv_comm.h"
#ifdef __LITEOS__
#include <unistd.h>
#else
#define IO_ADDRESS(offset) (offset)
#endif


//#define HISI_IO_ADDRESS(addr, offset) (addr + offset)

//============================hi3559v200===============================================


/* -----tsensor regs address ---- */
#define MISC_BASE_ADDR 0x12030000
#define TSENSOR_CTRL 0xb4
#define TSENSOR_STATUS0 0xbc
#define CYCLE_NUM 10


/* ------------core voltage reg---------*/
#define HI_PMC_CTL_REG         0x120300b0

#define reg_read(addr) (*(volatile unsigned int *)(addr))
#define reg_write(addr, val) (*(volatile unsigned int *)(addr) = (val))




//******************************************************************************
//  Function    :
//  Description : init the tsensor
//  Input       : void
//  Return      : void
//******************************************************************************
void pm_hal_tsensor_init(void)
{
    reg_write(IO_ADDRESS(MISC_BASE_ADDR + TSENSOR_CTRL), 0xc3200000); //Tsensor enable and begin to loop detect
}


//******************************************************************************
//  Function    :
//  Description : read temperature from tsensor regs
//  Input       : void
//  Return      : void
//******************************************************************************

int pm_hal_get_temperature(void)
{
    int i, j, tmp;
    int value = 0;
    int temperature;
    for (i = 0; i < CYCLE_NUM; i++) {
        osal_msleep(150);
        for (j = 0; j < 4; j++) {
            tmp = reg_read(IO_ADDRESS(MISC_BASE_ADDR + (TSENSOR_STATUS0 + 0x4 * j)));
            value += tmp & 0x3ff;
            value += (tmp >> 16) & 0x3ff;
        }
    }
    value = value / CYCLE_NUM / 8;
    temperature = (((value - 136) * 165) / 793 ) - 40;
    PM_DEBUG("Tsensor:%d 'C.\n", temperature);
    return temperature;
}


//******************************************************************************
//  Function    :
//  Description : do nothing
//  Input       : void
//  Return      : void
//******************************************************************************
void pm_media_pwm_set_voltage( int set_volt_uv)
{
    return ;
}


//******************************************************************************
//  Function    :
//  Description : do nothing
//  Input       : void
//  Return      : void
//******************************************************************************
int  pm_media_pwm_get_voltage(void)
{
    return 0;
}


//******************************************************************************
//  Function    :dc-dc write
//  Description : set  core domain voltage
//  Input       :  set_volt_uv
//  Return      : void
//******************************************************************************

void pm_core_pwm_set_voltage( int set_volt_uv)
{
    int  svb_value;
    //DC-DC write
    svb_value = (((1050000 - set_volt_uv) * 31) & 0xffff0000) + 0x0c75;
    reg_write(IO_ADDRESS(HI_PMC_CTL_REG), svb_value );
    PM_DEBUG("==============Core reg 0x%08x  = 0x%08x  in setvolt intf============\n", HI_PMC_CTL_REG, svb_value);
}

//******************************************************************************
//  Function    :dc-dc read
//  Description : get  core domain voltage
//  Input       : void
//  Return      : void
//******************************************************************************
int  pm_core_pwm_get_voltage(void)
{
    int set_volt_uv, svb_value;
    svb_value = reg_read(IO_ADDRESS(HI_PMC_CTL_REG ));
    PM_DEBUG("=======Core reg 0x%08x = 0x%08x=======\n", HI_PMC_CTL_REG, svb_value);
    set_volt_uv = 1050000 - (svb_value & 0xffff0000) / 31;
    return set_volt_uv;
}

//******************************************************************************
//  Function    :
//  Description :
//  Input       : void
//  Return      : void
//******************************************************************************
void pm_cpu_pwm_set_voltage( int set_volt_uv)
{
    return ;
}

//******************************************************************************
//  Function    :
//  Description :
//  Input       : void
//  Return      : void
//******************************************************************************
int  pm_cpu_pwm_get_voltage(void)
{
    return 0;
}


