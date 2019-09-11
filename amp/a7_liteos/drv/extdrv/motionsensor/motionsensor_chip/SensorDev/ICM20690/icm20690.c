#include "motionsensordev.h"
#include <linux/fs.h>
#include <linux/slab.h>
#ifndef __HuaweiLite__
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <asm/dma.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#else

#include "linux/kernel.h"
#include "asm/dma.h"
#include "linux/delay.h"
#include "linux/interrupt.h"
#include <linux/module.h>
#include <linux/kernel.h>
#endif

#ifdef TRANSFER_I2C
#include "i2c_dev.h"
#elif defined TRANSFER_SPI
#include "spi_dev.h"
#endif

#include "hi_comm_motionsensor.h"
#include "motionsensor_ext.h"
#include "sys_ext.h"
//#include "vi_ext.h"
#include "motionsensorgpio.h"



//#define TEST_DEBUG (1)
HI_U32 u32MotionSensorSpiNum = 2;
//static HI_U64 timerecord[51000];
HI_U64 time_num = 0;


static ICM20690_DEV_INFO* ICM20690_dev = NULL;
static MSENSOR_DATA_S stIMUdata;

//static struct workqueue_struct*  my_wq;
//static osal_mutex_t mutexGetData;
static osal_spinlock_t lockGetData;

static HI_U8  su8GyroAccDataLen;
static MSENSOR_ATTR_S stMSensorMode;
static HI_U64 saTimeBackup[TIME_RECORD_CNT] = {0};
static HI_U32 saEveryDataCntBackup[DATA_RECORD_CNT] = {0};
static HI_U8  su8Timecnt;
static HI_U8  su8datacount;

#define SCALE_TIMES                 (100)

#define THREAD_EXIT 0
#define THREAD_CTRL 1

volatile static unsigned int hithread_state;
#ifdef __HuaweiLite__
static gpio_groupbit_info     group_bit_info;
#endif


typedef struct hiMOTION_SENSOR_OUT_DATA_S
{
    MSENSOR_SAMPLE_DATA_S        stGyroData;
    MSENSOR_SAMPLE_DATA_S        stAccData;
    //MSENSOR_SAMPLE_DATA_S        stMagnData;
} MOTION_SENSOR_OUT_DATA_S;

#ifdef TEST_DEBUG
static MOTION_SENSOR_OUT_DATA_S test_fifo[11000] = {0};
#endif
HI_U32 su32DataCount = 0, i_thread = 0;
HI_U64 u64PTSNow;

extern HI_S32 MotionSensorDev_IntCallBack(MSENSOR_DATA_S* pstMSensorData);

extern unsigned short hi_motionsensor_ssp_read_alt(unsigned int ssp_no, HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt, HI_BOOL bFifoMode);
extern int hi_motionsensor_ssp_write_alt(unsigned int ssp_no, HI_U8 u8reg_addr, HI_U8* data);


HI_S32 HI_ICM20690_Transfer_read(HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt, HI_BOOL bFifoMode)
{
    HI_S32 s32Ret;
#ifndef __HuaweiLite__
#ifdef TRANSFER_I2C
    s32Ret = MotionSersor_I2C_read(ICM20690_dev->client, u8reg_addr, u8reg_data, u32cnt);
#elif defined TRANSFER_SPI
    //s32Ret = MotionSersor_SPI_read(ICM20690_dev->hi_spi, u8reg_addr, u8reg_data, u32cnt);
    s32Ret = hi_motionsensor_ssp_read_alt(u32MotionSensorSpiNum, u8reg_addr, u8reg_data, u32cnt, bFifoMode);
#endif
#else
    s32Ret = MotionSersor_SPI_read(u8reg_addr, u8reg_data, u32cnt, u32MotionSensorSpiNum);
    //s32Ret = hi_motionsensor_ssp_read_alt(BUS_NUM, u8reg_addr, u8reg_data, u32cnt, bFifoMode);
#endif
    return s32Ret;
}

HI_S32 HI_ICM20690_Transfer_write(HI_U8 u8reg_addr, HI_U8* u8reg_data, HI_U32 u32cnt)
{
    HI_S32 s32Ret;
#ifndef __HuaweiLite__
#ifdef TRANSFER_I2C
    s32Ret = MotionSersor_I2C_write(ICM20690_dev->client, u8reg_addr, u8reg_data, u32cnt);
#elif defined TRANSFER_SPI
    //s32Ret = MotionSersor_SPI_write(ICM20690_dev->hi_spi, u8reg_addr, u8reg_data, u32cnt);
    s32Ret = hi_motionsensor_ssp_write_alt(u32MotionSensorSpiNum, u8reg_addr, u8reg_data);
#endif
#else
    s32Ret = MotionSersor_SPI_write(u8reg_addr, u8reg_data, u32cnt, u32MotionSensorSpiNum);
     //s32Ret = hi_motionsensor_ssp_write_alt(BUS_NUM, u8reg_addr, u8reg_data);
#endif
    return s32Ret;
}


static HI_U64 inline HI_ICM20690_GetCurPts(void)
{
    HI_U64 u64TimeNow;

    extern HI_U64 sched_clock(void);
    u64TimeNow = sched_clock();
    do_div(u64TimeNow, 1000);
    return u64TimeNow;
}

#ifdef TRANSFER_SPI
static HI_S32 HI_I2C_Disable(void)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;
    u8RegisterValue = 0x10;
    u8Ret = HI_ICM20690_Transfer_write(USER_CONTROL_REGISTER_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("disable i2c failed(%d)\n", u8Ret);
        return -EAGAIN;
    }

    return HI_SUCCESS;
}
#endif

static HI_S32 ICM20690_Reset(void)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    u8Ret = HI_ICM20690_Transfer_read(POWER_MANAGEMENT_REGISTER_1_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("reset ICM20690 failed\n");
        return -EAGAIN;
    }

    u8RegisterValue |= TRUE_REGISTER_VALUE << RESET_OFFSET;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(POWER_MANAGEMENT_REGISTER_1_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("reset ICM20690 failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}

static HI_S32 ICM20690_SetClk(void)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue ;

    u8Ret = HI_ICM20690_Transfer_read(POWER_MANAGEMENT_REGISTER_1_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("reset ICM20690 failed  HI_S8\n");
        return -EAGAIN;
    }

    u8RegisterValue = CLKSET_VALUE;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(POWER_MANAGEMENT_REGISTER_1_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("reset ICM20690 failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}

static HI_S32 ICM20690_SetAxisMode(HI_U32 u32DevMode)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue = 0xFF;

    if (u32DevMode & MSENSOR_DEVICE_GYRO)
    {
        u8RegisterValue &= 0x38;
    }

    if (u32DevMode & MSENSOR_DEVICE_ACC)
    {
        u8RegisterValue &= 0x07;
    }

    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(POWER_MANAGEMENT_REGISTER_2_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_AXIS_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}

static HI_S32 ICM20690_SetSampleRate(HI_U64 u64SampleRate)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    if (u64SampleRate == 0 || osal_div64_u64_rem(1000 , u64SampleRate))
    {
        print_info("u8SAMPLE_RATE must be  is divisible by 1000,  %lld  \n", u64SampleRate);
        return -EAGAIN;
    }

    u8RegisterValue = osal_div64_u64(1000, u64SampleRate) - 1;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(SMPLRT_DIV, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_AXIS_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}
static HI_S32 ICM20690_SetGyroDLFP_CFG(HI_U8 dlfp_cfg)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    if (dlfp_cfg < 0 || dlfp_cfg > 7)
    {
        print_info("ICM20690_GYRO_DLFP_CFG is invalid\n");
        return -EINVAL;
    }

    u8Ret = HI_ICM20690_Transfer_read(CONFIGURATION_REGISTER_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_DLFP_CFG_SET failed\n");
        return -EAGAIN;
    }

    u8RegisterValue &= ~0x7;
    u8RegisterValue |= dlfp_cfg;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(CONFIGURATION_REGISTER_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_DLFP_CFG_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}

static HI_S32 ICM20690_SetAccelDLFP_CFG(HI_U8 dlfp_cfg)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    if (dlfp_cfg < 0 || dlfp_cfg > 7)
    {
        print_info("ICM20690_GYRO_DLFP_CFG is invalid\n");
        return -EINVAL;
    }

    u8Ret = HI_ICM20690_Transfer_read(ACCEL_CONFIG_REGISTER_2_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_DLFP_CFG_SET failed\n");
        return -EAGAIN;
    }

    u8RegisterValue &= ~0x7;
    u8RegisterValue |= dlfp_cfg;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(ACCEL_CONFIG_REGISTER_2_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_DLFP_CFG_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}



static HI_S32 ICM20690_SetGryoFchoice_B(HI_U8 fchoice_b)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    if (fchoice_b < 0 || fchoice_b > 3)
    {
        print_info("ICM20690_GYRO_FCHOICE_B is invalid\n");
        return -EINVAL;
    }

    u8Ret = HI_ICM20690_Transfer_read(GYRO_CONFIG_REGISTER_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    u8RegisterValue &= ~0x3;
    u8RegisterValue |= fchoice_b;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(GYRO_CONFIG_REGISTER_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}


static HI_S32 ICM20690_SetAccelFchoice_B(HI_U8 fchoice_b)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    if (fchoice_b < 0 || fchoice_b > 1)
    {
        print_info("ICM20690_GYRO_FCHOICE_B is invalid\n");
        return -EINVAL;
    }

    u8Ret = HI_ICM20690_Transfer_read(ACCEL_CONFIG_REGISTER_2_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    u8RegisterValue &= ~(0x1 << 3);
    u8RegisterValue |= fchoice_b;
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(ACCEL_CONFIG_REGISTER_2_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}


static HI_S32 ICM20690_Gyro_SetFullScaleRange(HI_U8 fs_sel)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    u8Ret = HI_ICM20690_Transfer_read(GYRO_CONFIG_REGISTER_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    u8RegisterValue &= ~(0x3 << 2);
    u8RegisterValue |= (fs_sel << 2);
    print_info("u8RegisterValue = %x\n", u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write( GYRO_CONFIG_REGISTER_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FULL_SCALE_RANGE_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}


static HI_S32 ICM20690_UI_SetAccelFullScaleRange(HI_U8 fs_sel)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue;

    u8Ret = HI_ICM20690_Transfer_read(ACCEL_CONFIG_REGISTER_1_ADDR, &u8RegisterValue, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FCHOICE_B_SET failed\n");
        return -EAGAIN;
    }

    //print_info("fs_sel = %x u8RegisterValue = %x\n", fs_sel << 3, u8RegisterValue);
    u8RegisterValue &= ~(0x7 << 3);
    u8RegisterValue |= (fs_sel << 3);
    //print_info("fs_sel = %x u8RegisterValue = %x\n", fs_sel << 3, u8RegisterValue);

    u8Ret = HI_ICM20690_Transfer_write(ACCEL_CONFIG_REGISTER_1_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("ICM20690_GYRO_FULL_SCALE_RANGE_SET failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}


static HI_S32 ICM20690_SetGyroAttr(GYRO_STATUS_S stGyroStatus)
{
    //HI_U8 u8Ret;
    //HI_U8 u8RegisterValue;
    /*set gyro odr and BW*/

    if (stGyroStatus.stGyroConfig.u64ODR == GYRO_OUTPUT_DATA_RATE_32KHZ)
    {
        if (stGyroStatus.u32BandWidth == GYRO_BAND_WIDTH_8800HZ)
        {
            ICM20690_SetGryoFchoice_B(0x01);
        }
        else if (stGyroStatus.u32BandWidth == GYRO_BAND_WIDTH_3600HZ)
        {
            ICM20690_SetGryoFchoice_B(0x02);
        }
        else
        {
            print_info("BandWidth and ODR is not match\n");
            return -EINVAL;
        }

    }
    else if (stGyroStatus.stGyroConfig.u64ODR == GYRO_OUTPUT_DATA_RATE_8KHZ)
    {
        if (stGyroStatus.u32BandWidth == GYRO_BAND_WIDTH_250HZ)
        {
            ICM20690_SetGyroDLFP_CFG(0x0);
            ICM20690_SetGryoFchoice_B(0x0);
        }
        else if (stGyroStatus.u32BandWidth == GYRO_BAND_WIDTH_3600HZ)
        {
            ICM20690_SetGyroDLFP_CFG(0x7);
            ICM20690_SetGryoFchoice_B(0x0);
        }
        else
        {
            print_info("BandWidth and ODR is not match\n");
            return -EINVAL;
        }
    }
    else if (stGyroStatus.stGyroConfig.u64ODR <= 1000)
    {
        switch (stGyroStatus.u32BandWidth)
        {
            case GYRO_BAND_WIDTH_5HZ:
                ICM20690_SetGyroDLFP_CFG(0x6);
                break;

            case GYRO_BAND_WIDTH_10HZ:
                ICM20690_SetGyroDLFP_CFG(0x5);
                break;

            case GYRO_BAND_WIDTH_20HZ:
                ICM20690_SetGyroDLFP_CFG(0x4);
                break;

            case GYRO_BAND_WIDTH_41HZ:
                ICM20690_SetGyroDLFP_CFG(0x3);
                break;

            case GYRO_BAND_WIDTH_92HZ:
                ICM20690_SetGyroDLFP_CFG(0x2);
                break;

            case GYRO_BAND_WIDTH_184HZ:
                ICM20690_SetGyroDLFP_CFG(0x1);
                break;

            default:
                print_info("BandWidth and ODR is not match\n");
                break;
        }
    }

    /*set gyro FSR */
    print_info("gyro_status.u32Range is %lld\n", stGyroStatus.stGyroConfig.u64FSR);

    switch (stGyroStatus.stGyroConfig.u64FSR)
    {
        case GYRO_FULL_SCALE_RANGE_250DPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_250DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_500DPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_500DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_1KDPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_1000DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_2KDPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_2000DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_31DPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_31DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_62DPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_62DPS);
            break;

        case GYRO_FULL_SCALE_RANGE_125DPS:
            ICM20690_Gyro_SetFullScaleRange(ICM20690_GYRO_FULL_SCALE_SET_125DPS);
            break;

        default:
            print_info("ICM20690_GYRO_ATTR is invalid\n");
            return -EINVAL;
    }

    return HI_SUCCESS;
}



static HI_S32 ICM20690_SetAccelAttr(ACC_STATUS_S stAccStatus)
{
    //HI_U8 u8Ret;
    //HI_U8 u8RegisterValue;
    /*set ACCEL odr and BW*/

    if (stAccStatus.stAccConfig.u64ODR == ACCEL_OUTPUT_DATA_RATE_4KHZ)
    {
        ICM20690_SetAccelFchoice_B(0x01);
    }
    else if (stAccStatus.stAccConfig.u64ODR == ACCEL_OUTPUT_DATA_RATE_1KHZ)
    {
        ICM20690_SetAccelFchoice_B(0x0);
        ICM20690_SetAccelDLFP_CFG(0x7);
    }
    else if (stAccStatus.stAccConfig.u64ODR < 1000)
    {
        switch (stAccStatus.u32BandWidth)
        {
            case ACCEL_BAND_WIDTH_5HZ:
                ICM20690_SetAccelDLFP_CFG(0x6);
                break;

            case ACCEL_BAND_WIDTH_10HZ:
                ICM20690_SetAccelDLFP_CFG(0x5);
                break;

            case ACCEL_BAND_WIDTH_21HZ:
                ICM20690_SetAccelDLFP_CFG(0x4);
                break;

            case ACCEL_BAND_WIDTH_44HZ:
                ICM20690_SetAccelDLFP_CFG(0x3);
                break;

            case ACCEL_BAND_WIDTH_99HZ:
                ICM20690_SetAccelDLFP_CFG(0x2);
                break;

            case ACCEL_BAND_WIDTH_218HZ:
                ICM20690_SetAccelDLFP_CFG(0x1);
                break;

            default:
                print_info("BandWidth and ODR is not match\n");
                break;
        }
    }


    /*set ACCEL FSR */
    print_info("accel_status.u32Range is %lld\n", stAccStatus.stAccConfig.u64FSR);

    switch (stAccStatus.stAccConfig.u64FSR)
    {
        case ACCEL_UI_FULL_SCALE_SET_2G:
            ICM20690_UI_SetAccelFullScaleRange(ICM20690_ACCEL_UI_FULL_SCALE_SET_2G);
            break;

        case ACCEL_UI_FULL_SCALE_SET_4G:
            ICM20690_UI_SetAccelFullScaleRange(ICM20690_ACCEL_UI_FULL_SCALE_SET_4G);
            break;

        case ACCEL_UI_FULL_SCALE_SET_8G:
            ICM20690_UI_SetAccelFullScaleRange(ICM20690_ACCEL_UI_FULL_SCALE_SET_8G);
            break;

        case ACCEL_UI_FULL_SCALE_SET_16G:
            ICM20690_UI_SetAccelFullScaleRange(ICM20690_ACCEL_UI_FULL_SCALE_SET_16G);
            break;

        default:
            print_info("ICM20690_ACCEL_ATTR is invalid\n");
            return -EINVAL;
    }

    return HI_SUCCESS;
}


static HI_S32 ICM20690_GyroLowPowerModeDisable(void)
{
    HI_U8 u8Ret;
    HI_U8 u8RegisterValue = 0x00;

    u8Ret = HI_ICM20690_Transfer_write(LP_MODE_CONFIG_REGISTER_ADDR, &u8RegisterValue, 1);

    if (u8Ret)
    {
        print_info("RESET_FIFO failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}



HI_S32 MotionSensor_GetTrigerConfig(TRIGER_CONFIG_S* pstTrigerConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    pstTrigerConfig->eTrigerMode =  ICM20690_dev->stTrigerConfig.eTrigerMode;
    pstTrigerConfig->uTrigerInfo.stTimerConfig.u32interval = ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval;
    return s32Ret;
}

#if 0
static HI_S32 All_Register_Read(void)
{
    HI_S32 i;
    HI_S8 ret, ch;

    for (i = 0; i < 127; i++)
    {

        ret = HI_ICM20690_Transfer_read(i, &ch, 1, 0);

        if (ret)
        {
            print_info("register_read failed\n");
            return -EAGAIN;
        }

        printk("register[0x%x] value is 0x%x\n", i, ch);
    }

    return HI_SUCCESS;
}
#endif
static HI_S32 inline Fifo_Is_Overflow(void)
{
    HI_U8 ret, ch;

    ret = HI_ICM20690_Transfer_read(0x3a, &ch, 1, HI_FALSE);

    if (ret)
    {
        print_info("register_read failed\n");
        return -EAGAIN;
    }

    return (ch & (0x1 << 4));
}

static HI_S32 inline Reset_Fifo(void)
{
    //REset FIFO , in case the FIFO is overflow //00000100 -> 0x6A 	Set 00000100 to USER_CTRL (Reset the FIFO)
    HI_U8 u8ch, u8Ret;

    u8Ret = HI_ICM20690_Transfer_read(USER_CONTROL_REGISTER_ADDR, &u8ch, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("RESET_FIFO failed\n");
        return -EAGAIN;
    }

    u8ch |= 0x04;

    u8Ret = HI_ICM20690_Transfer_write( USER_CONTROL_REGISTER_ADDR, &u8ch, 1);

    if (u8Ret)
    {
        print_info("RESET_FIFO failed\n");
        return -EAGAIN;
    }

    ICM20690_dev->b_FLAG_FIFOIncomming = 0;
    return HI_SUCCESS;
}


static HI_S32 inline ICM20690_GetFifoLengthAndCount(void)
{
    HI_U32 recordLength;
    HI_U8  ret, buff[2];
    memset(buff, 0 , sizeof(buff));
#if 1
    ret = HI_ICM20690_Transfer_read(FIFO_COUNTH, buff, 2, HI_FALSE);

    if (ret)
    {
        print_info("get_ICM20690_fifo_count_Length failed\n");
        return -EAGAIN;
    }
#else
    ret = HI_ICM20690_Transfer_read(FIFO_COUNTH, &buff[0], 1);

    if (ret)
    {
        print_info("get_ICM20690_fifo_count_Length failed\n");
        return -EAGAIN;
    }

     ret = HI_ICM20690_Transfer_read(FIFO_COUNTL, &buff[1], 1);

    if (ret)
    {
        print_info("get_ICM20690_fifo_count_Length failed\n");
        return -EAGAIN;
    }
#endif

    ///osal_printk("buff[0]:%d buff[1]:%d\n",buff[0], buff[1]);

    ICM20690_dev->recordNum = (short)(((short)buff[0]) << 8 | buff[1]);

    recordLength = ((ICM20690_dev->b_FLAG_ACC_FIFO_Enabled + ICM20690_dev->b_FLAG_GYRO_FIFO_Enabled) * 6);
    ICM20690_dev->fifoLength = ICM20690_dev->recordNum * recordLength;

    //print_info("recordNum:%d,fifoLength:%d,recordLength:%d\n",ICM20690_dev->recordNum,ICM20690_dev->fifoLength,recordLength);
    //osal_printk("++++fun:%s recordNum:%d\n",__func__,ICM20690_dev->recordNum);
    //osal_printk("+recordNum:%d\n",ICM20690_dev->recordNum);
    return HI_SUCCESS;
}

static HI_S32 inline ICM20690_UI_FifoSaveData(void)
{
    HI_S32 s32Ret, i;
    
#if 0
    ret = FIFO_IS_OVERFLOW();

    if (s32Ret)
    {
        mpu_print_info("FIFO is over flow !!\n");
        RESET_FIFO(); //?? FIFO reset , over flow
        //  	    return -EMSGSIZE;
    }

#else


    if (ICM20690_dev->fifoLength > 1024)
    {
        print_info("FIFO is over flow !!, NUM  = %d\n\n", ICM20690_dev->recordNum);
        s32Ret = Reset_Fifo(); //?? FIFO reset , over flow

        if (s32Ret)
        {
            print_info("reset fifo failed\n");
            return -EAGAIN;
        }

        return HI_SUCCESS;
    }

#endif

    //begin to read FIFO in several Sections

    for (i = 0; i < ICM20690_dev->fifoLength / ICM20690_FIFO_R_MAX_SIZE; i++)
    {

        s32Ret = HI_ICM20690_Transfer_read(FIFO_R_W, ICM20690_dev->FIFO_buf + i * ICM20690_FIFO_R_MAX_SIZE, ICM20690_FIFO_R_MAX_SIZE, HI_TRUE);

        if (s32Ret)
        {
            print_info("read FIFO in several Sections failed\n");
            return -EAGAIN;
        }
    }

    s32Ret = HI_ICM20690_Transfer_read(FIFO_R_W, ICM20690_dev->FIFO_buf + i * ICM20690_FIFO_R_MAX_SIZE, ICM20690_dev->fifoLength - \
                                       ICM20690_FIFO_R_MAX_SIZE * (ICM20690_dev->fifoLength / ICM20690_FIFO_R_MAX_SIZE), HI_TRUE);

    if (s32Ret)
    {
        print_info("read FIFO in last_data failed\n");
        return -EAGAIN;
    } //FIFO_R_W(0x74)

    ICM20690_dev->b_FLAG_FIFOIncomming = 1;

    return HI_SUCCESS;
}


HI_S32 FIFO_DATA_UPDATE(void)
{
    return ICM20690_UI_FifoSaveData();
}

HI_S32 FIFO_DATA_RESET(void)
{
    HI_S32 s32Ret;

    if (ICM20690_dev->u8FifoEn)
    {
        s32Ret = Reset_Fifo();

        if (s32Ret)
        {
            print_info("reset fifo failed\n");
            return -EAGAIN;
        }

    }

    return HI_SUCCESS;
}

static HI_S32 ICM20690_UI_FifoModeEnable(HI_U32 u32DevMode)
{
    HI_U8 ch, u8Ret;
    u8Ret = Reset_Fifo();

    if (u8Ret)
    {
        print_info("reset fifo failed\n");
        return -EAGAIN;
    }

    //i2c_write_1B(0x6A,0x40);                                     //Enable the FIFO Operation mode
    ch = 0x40;

    u8Ret = HI_ICM20690_Transfer_write(USER_CONTROL_REGISTER_ADDR, &ch, 1);

    if (u8Ret)
    {
        print_info("Enable the FIFO Operation mode failed(0x%x)\n", u8Ret);
        return -EAGAIN;
    }

    //i2c_read_1B(0x1D,&ch); ch|=0xC0; i2c_write_1B(0x1D,ch);     //11000000 -> 0x1D	Get and Set 11000000 to ACCEL _CONFIG2 (Enable the FIFO size to 1024)

    u8Ret = HI_ICM20690_Transfer_read(ACCEL_CONFIG_REGISTER_2_ADDR, &ch, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("Enable the FIFO size to 1024 failed\n");
        return -EAGAIN;
    }

    ch |= 0xC0;
    print_info("register = %x\n", ch);
    u8Ret = HI_ICM20690_Transfer_write(ACCEL_CONFIG_REGISTER_2_ADDR, &ch, 1);

    if (u8Ret)
    {
        print_info("Enable the FIFO size to 1024 failed\n");
        return -EAGAIN;
    }

    u8Ret = HI_ICM20690_Transfer_read(CONFIGURATION_REGISTER_ADDR, &ch, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("Enable FIFO mode and record mode failed\n");
        return -EAGAIN;
    }

    ch |= 0x80;
    print_info("CONFIGURATION_REGISTER_ADDR:%x\n", ch);
    u8Ret = HI_ICM20690_Transfer_write( CONFIGURATION_REGISTER_ADDR, &ch, 1);

    if (u8Ret)
    {
        print_info("Enable FIFO mode and record mode failed\n");
        return -EAGAIN;
    }

    u8Ret = HI_ICM20690_Transfer_read(FIFO_ENABLE_REGISTER_ADDR, &ch, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("Enable ACC and GYRO in FIFO failed\n");
        return -EAGAIN;
    }

    if (MSENSOR_DEVICE_ACC & u32DevMode)
    {
        ch |= 0x08;
        ICM20690_dev->b_FLAG_ACC_FIFO_Enabled = 1;
    }

    if (MSENSOR_DEVICE_GYRO & u32DevMode)
    {
        ch |= 0x70;
        ICM20690_dev->b_FLAG_GYRO_FIFO_Enabled = 1;
    }

    print_info("FIFO_ENABLE_REGISTER_ADDR:0x%x\n", ch);

    u8Ret = HI_ICM20690_Transfer_write( FIFO_ENABLE_REGISTER_ADDR, &ch, 1);

    if (u8Ret)
    {
        print_info("Enable ACC and GYRO in FIFO failed\n");
        return -EAGAIN;
    }

    //ICM20690_dev->b_FLAG_ACC_FIFO_Enabled = 1;
    //ICM20690_dev->b_FLAG_GYRO_FIFO_Enabled = 1;
    ch = ICM20690_dev->stTrigerConfig.uTrigerInfo.stExternInterruptConfig.u32Interrupt_num / (ICM20690_dev->b_FLAG_ACC_FIFO_Enabled + ICM20690_dev->b_FLAG_GYRO_FIFO_Enabled);
    u8Ret = HI_ICM20690_Transfer_write(0x61, &ch, 1);

    if (u8Ret)
    {
        print_info("Enable the FIFO Operation mode failed\n");
        return -EAGAIN;
    }

    //set fifo data len
    su8GyroAccDataLen = (ICM20690_dev->b_FLAG_ACC_FIFO_Enabled + ICM20690_dev->b_FLAG_GYRO_FIFO_Enabled) * 6;
    u8Ret = Reset_Fifo();

    if (u8Ret)
    {
        print_info("reset fifo failed\n");
        return -EAGAIN;
    }

    return HI_SUCCESS;
}

static int ICM20690_INTConfig(void)
{
    HI_U8 ch, u8Ret;

    /*set interrupt config.. bit7:INT_LEVEL. bit6:INT_OPEN.bit5:LATCH_INT_EN.bit4:INT_RD_CLEAR*/
    ch = 0xA0;
    u8Ret = HI_ICM20690_Transfer_write(INT_PIN_CONFIGURATION, &ch, 1);

    if (u8Ret)
    {
        print_info("Set INT config failed\n");
        return -EAGAIN;
    }

    /*set interrupt type  fifo watermark INT or data ready INT*/
    if (HI_TRUE == ICM20690_dev->u8FifoEn)
    {
        u8Ret = HI_ICM20690_Transfer_read(FIFO_WM_INT_STATUS, &ch, 1, HI_FALSE);
        //ch = 0x40;
        //u8Ret = HI_ICM20690_Transfer_write(FIFO_WM_INT_STATUS, &ch, 1);

        if (u8Ret)
        {
            print_info("Set INT enable failed\n");
            return -EAGAIN;
        }

    }
    else
    {
        /*data ready interrupt enable set*/
        ch = 0x01;

        u8Ret = HI_ICM20690_Transfer_write(INTERRUPT_ENABLE, &ch, 1);

        if (u8Ret)
        {
            print_info("Set INT enable failed\n");
            return -EAGAIN;
        }

        HI_ICM20690_Transfer_read(INTERRUPT_ENABLE, &ch, 1, HI_FALSE);
        print_info("INTERRUPT_ENABLE = %x\n", ch);
    }

    return u8Ret;
}


static HI_S32 ICM20690_GetTemperature(HI_S32* s32Temperature)
{
    HI_U32 u32Ret;
    HI_U8 u8RegisterValue[2];

    u32Ret = HI_ICM20690_Transfer_read(DEV_TEMPERATURE_LSB_ADDR, &u8RegisterValue[0], 2, HI_FALSE);

    if (u32Ret)
    {
        print_info("ICM20690_GetTemperature failed\n");
        return -EAGAIN;
    }

    *s32Temperature = ((HI_S32)((HI_S8) (u8RegisterValue[0]) << 8)) | u8RegisterValue[1];
    /*1024 times of Celsius temperature magnification*/
    *s32Temperature = ROOMTEMP_OFFSET + ((*s32Temperature) * GRADIENT_TEMP / TEMP_SENSITIVITY);
    return HI_SUCCESS;
}

HI_S32 ICM20690_ReadAccelData_XYZ(MSENSOR_SAMPLE_DATA_S* stAccData)
{
    HI_U8 buff[6] = {0, 0, 0, 0, 0, 0};
    HI_S32 s32Ret = HI_SUCCESS;
#ifdef Hi3559A_DMA
    /*6 registers continuously read, with a first address of accel_xout_h*/
    s32Ret = i2cdev_dma_read(ACCEL_XOUT_H, 6, buff);

    if (s32Ret < 0)
    {
        print_info("ICM20690_read_accel_xyz failed\n");
        return -EAGAIN;
    }

#else

    s32Ret = HI_ICM20690_Transfer_read(ACCEL_XOUT_H , &buff[0], 6, HI_FALSE);

    if (s32Ret)
    {
        print_info("ICM20690_read_accel_xyz failed\n");
        return -EAGAIN;
    }

    stAccData->s32XData = ((buff[0] << 8) & 0xff00) | (buff[1] & 0xff);
    stAccData->s32YData = ((buff[2] << 8) & 0xff00) | (buff[3] & 0xff);
    stAccData->s32ZData = ((buff[4] << 8) & 0xff00) | (buff[5] & 0xff);
    stAccData->s32Temp  = ICM20690_dev->s32temperature;
    stAccData->u64PTS   = u64PTSNow;

#endif
    return HI_SUCCESS;
}

HI_S32 ICM20690_ReadGyroData_XYZ(MSENSOR_SAMPLE_DATA_S* stGyroData)
{
    HI_U8 buff[6] = {0, 0, 0, 0, 0, 0};
    HI_S32 res;
#ifdef Hi3559A_DMA
    /* read out 6 registers in a row with a first address of gyro_xout_h*/
    res = i2cdev_dma_read(0x43, 6, buff);

    if (res < 0)
    {
        print_info("ICM20690_read_gyro_xyz failed\n");
        return -EAGAIN;
    }

#else

    res = HI_ICM20690_Transfer_read(GYRO_XOUT_H , &buff[0], 6, HI_FALSE);

    if (res)
    {
        print_info("ICM20690_read_accel_xH failed\n");
        return -EAGAIN;
    }

    stGyroData->s32XData = ((buff[0] << 8) & 0xff00) | (buff[1] & 0xff);
    stGyroData->s32YData = ((buff[2] << 8) & 0xff00) | (buff[3] & 0xff);
    stGyroData->s32ZData = ((buff[4] << 8) & 0xff00) | (buff[5] & 0xff);
    stGyroData->s32Temp  = ICM20690_dev->s32temperature;
    stGyroData->u64PTS   = u64PTSNow;
#endif
    return HI_SUCCESS;
}



HI_S32 ICM20690_ReadData(MSENSOR_DATA_S* pstIMUdata)
{
    HI_S32 s32Ret;
    memset(&ICM20690_dev->accel_cur_data, 0x0, sizeof(MSENSOR_SAMPLE_DATA_S));
    memset(&ICM20690_dev->gyro_cur_data, 0x0, sizeof(MSENSOR_SAMPLE_DATA_S));

    if (i_thread % 50 == 0)
    {
        s32Ret = ICM20690_GetTemperature(&ICM20690_dev->s32temperature);

        if (HI_SUCCESS != s32Ret)
        {
            print_info("bmi160_get_temp failed! ret=%x\n", s32Ret);
            return s32Ret;
        }

    }

    if (stMSensorMode.u32DeviceMask & MSENSOR_DEVICE_ACC)
    {
        s32Ret = ICM20690_ReadAccelData_XYZ(&ICM20690_dev->accel_cur_data);

        if (s32Ret)
        {
            print_info("ICM20690_read_accel_data_XYZ failed\n");
            return -ENODATA;
        }
    }

    if (stMSensorMode.u32DeviceMask & MSENSOR_DEVICE_GYRO)
    {
        s32Ret = ICM20690_ReadGyroData_XYZ(&ICM20690_dev->gyro_cur_data);

        if (s32Ret)
        {
            print_info("ICM20690_read_gyro_data_XYZ failed\n");
            return -ENODATA;
        }
    }

    memcpy(&pstIMUdata->stMSensorAttr, &stMSensorMode, sizeof(MSENSOR_ATTR_S));

    if (stMSensorMode.u32DeviceMask == (MSENSOR_DEVICE_GYRO | MSENSOR_DEVICE_ACC))
    {
        /*accel data handler, pls fix this*/
        //pstIMUdata->u32AccelCount = ICM20690_dev->recordNum;
        pstIMUdata->stMsensorAccBuffer.u32BuffDataNum = 1;
        //pst_IMUdata->u32AccelValid = ACCEL_DATA_VALID | ACCEL_TEMPERATURE_VALID;

        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32XData = ICM20690_dev->accel_cur_data.s32XData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32YData = ICM20690_dev->accel_cur_data.s32YData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32ZData = ICM20690_dev->accel_cur_data.s32ZData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32Temp  = ICM20690_dev->accel_cur_data.s32Temp;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].u64PTS   = ICM20690_dev->accel_cur_data.u64PTS;
        //ICM20690_dev->stAccStatus.u64LastPts = u64PTSNow;

        /*Gyro data handler*, pls fix this*/
        pstIMUdata->stMsensorGyroBuffer.u32BuffDataNum = 1;

        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32XData  =  ICM20690_dev->gyro_cur_data.s32XData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32YData  =  ICM20690_dev->gyro_cur_data.s32YData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32ZData  =  ICM20690_dev->gyro_cur_data.s32ZData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32Temp   =  ICM20690_dev->gyro_cur_data.s32Temp;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].u64PTS    =  ICM20690_dev->gyro_cur_data.u64PTS;

        /*PTS  handle*/
        ICM20690_dev->stGyroStatus.u64LastPts = u64PTSNow;

    }
    else if (stMSensorMode.u32DeviceMask == MSENSOR_DEVICE_GYRO)
    {
        /*Gyro data handler*, pls fix this*/
        pstIMUdata->stMsensorGyroBuffer.u32BuffDataNum = 1;
        //pst_IMUdata->u32GyroValid = GYRO_DATA_VALID  | GYRO_TEMPERATURE_VALID;

        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32XData  =  ICM20690_dev->gyro_cur_data.s32XData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32YData  =  ICM20690_dev->gyro_cur_data.s32YData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32ZData  =  ICM20690_dev->gyro_cur_data.s32ZData;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].s32Temp   =  ICM20690_dev->gyro_cur_data.s32Temp;
        pstIMUdata->stMsensorGyroBuffer.astGyroData[0].u64PTS    =  ICM20690_dev->gyro_cur_data.u64PTS;

        /*PTS  handle*/
        //ICM20690_dev->stGyroStatus.u64LastPts = u64PTSNow;
    }
    else if (stMSensorMode.u32DeviceMask == MSENSOR_DEVICE_ACC)
    {
        pstIMUdata->stMsensorAccBuffer.u32BuffDataNum = 1;
        //pst_IMUdata->u32AccelValid = ACCEL_DATA_VALID | ACCEL_TEMPERATURE_VALID;

        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32XData =  ICM20690_dev->accel_cur_data.s32XData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32YData = ICM20690_dev->accel_cur_data.s32YData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32ZData =  ICM20690_dev->accel_cur_data.s32ZData;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].s32Temp = ICM20690_dev->accel_cur_data.s32Temp;
        pstIMUdata->stMsensorAccBuffer.astAccData[0].u64PTS = ICM20690_dev->accel_cur_data.u64PTS;
        //ICM20690_dev->stAccStatus.u64LastPts = u64PTSNow;
    }


#ifdef TEST_DEBUG
    test_fifo[su32DataCount].stGyroData.s32XData	= ICM20690_dev->gyro_cur_data.s32XData;
    test_fifo[su32DataCount].stGyroData.s32YData	= ICM20690_dev->gyro_cur_data.s32YData;
    test_fifo[su32DataCount].stGyroData.s32ZData    = ICM20690_dev->gyro_cur_data.s32ZData;
    test_fifo[su32DataCount].stGyroData.s32Temp     = ICM20690_dev->s32temperature;
    test_fifo[su32DataCount].stGyroData.u64PTS 	    = u64PTSNow;

    test_fifo[su32DataCount].stAccData.s32XData	    = ICM20690_dev->accel_cur_data.s32XData;
    test_fifo[su32DataCount].stAccData.s32YData	    = ICM20690_dev->accel_cur_data.s32YData;
    test_fifo[su32DataCount].stAccData.s32ZData     = ICM20690_dev->accel_cur_data.s32ZData;
    test_fifo[su32DataCount].stAccData.s32Temp      = ICM20690_dev->s32temperature;
    test_fifo[su32DataCount].stAccData.u64PTS 	    = u64PTSNow;


    su32DataCount++;
#endif

    //	msleep(1000);
    i_thread++;
    return HI_SUCCESS;
}

static HI_U64 inline HI_MotionSensor_GetCurPts(void)
{
    HI_U64 u64TimeNow;
#if 0
    extern HI_U64 sched_clock(void);
    u64TimeNow = sched_clock();
    do_div(u64TimeNow, 1000);
#else
    u64TimeNow = CALL_SYS_GetTimeStamp();
#endif
    return u64TimeNow;
}

static HI_U64 u64Time_last;

static HI_U8  s8TimerFristFlag = 0;





static HI_S32 ICM20690_SetParam(MSENSOR_PARAM_S* stMSensorParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32SampleRate, u32value, u32FSR;

    if (MSENSOR_DEVICE_GYRO & stMSensorParam->stMSensorAttr.u32DeviceMask)
    {
        /*set GyroODRConfig*/
        u32value = stMSensorParam->stMSensorConfig.stGyroConfig.u64ODR / GRADIENT;
        u32SampleRate = u32value;

        if (u32SampleRate != GYRO_OUTPUT_DATA_RATE_32KHZ && u32SampleRate != GYRO_OUTPUT_DATA_RATE_8KHZ)
        {
            if ((1000 % u32value) != 0 || u32value > 1000)
            {
                u32value = GYRO_OUTPUT_DATA_RATE_BUTT;
            }
            else
            {
                u32value = GYRO_OUTPUT_DATA_RATE_UNDER_1KHZ;
            }
        }

        switch (u32value)
        {
            case GYRO_OUTPUT_DATA_RATE_UNDER_1KHZ:
            case GYRO_OUTPUT_DATA_RATE_32KHZ:
            case GYRO_OUTPUT_DATA_RATE_8KHZ:
                stMSensorParam->stMSensorConfig.stGyroConfig.u64ODR = u32SampleRate;
                break;

            case GYRO_OUTPUT_DATA_RATE_BUTT:
                print_info("not support Gyro ODR!\n");
                s32Ret = HI_FAILURE;
                break;

            default:
                print_info("out of gyro range!!!\n");
                s32Ret = HI_FAILURE;
                break;
        }

        /*set GyroFSRConfig*/

        u32FSR = stMSensorParam->stMSensorConfig.stGyroConfig.u64FSR / GRADIENT;

        switch (u32FSR)
        {
            case GYRO_FULL_SCALE_SET_2KDPS:

            case GYRO_FULL_SCALE_SET_1KDPS:

            case GYRO_FULL_SCALE_SET_500DPS:

            case GYRO_FULL_SCALE_SET_250DPS:

            case GYRO_FULL_SCALE_SET_125DPS:

            case GYRO_FULL_SCALE_SET_62DPS:

            case GYRO_FULL_SCALE_SET_31DPS:
                stMSensorParam->stMSensorConfig.stGyroConfig.u64FSR = u32FSR;
                break;

            default:
                print_info("not support Gyro FSR!\n");
                s32Ret = HI_FAILURE;
                break;
        }

        /*set gyro valid data bit*/
        stMSensorParam->stMSensorConfig.stGyroConfig.u8DataWidth = ICM20690_VALID_DATA_BIT;
    }

    if (MSENSOR_DEVICE_ACC & stMSensorParam->stMSensorAttr.u32DeviceMask)
    {
        print_info("set acc odr range!!\n");
        /*set AccelODRConfig*/
        u32value = stMSensorParam->stMSensorConfig.stAccConfig.u64ODR / GRADIENT;
        u32SampleRate = u32value;

        if (u32SampleRate != ACCEL_OUTPUT_DATA_RATE_4KHZ && u32SampleRate != ACCEL_OUTPUT_DATA_RATE_1KHZ)
        {
            if ((1000 % u32value) != 0 || u32value > 4000)
            {
                u32value = ACCEL_OUTPUT_DATA_RATE_BUTT;
            }
            else
            {
                u32value = ACCEL_OUTPUT_DATA_RATE_UNDER_1KHZ;
            }
        }

        switch (u32value)
        {
            case ACCEL_OUTPUT_DATA_RATE_UNDER_1KHZ:

            case ACCEL_OUTPUT_DATA_RATE_4KHZ:

            case ACCEL_OUTPUT_DATA_RATE_1KHZ:
                stMSensorParam->stMSensorConfig.stAccConfig.u64ODR = u32SampleRate;
                break;

            default:
                print_info("not support ACCEL ODR!\n");
                s32Ret = HI_FAILURE;
                break;
        }

        /*set AccelFSRConfig*/
        u32FSR = stMSensorParam->stMSensorConfig.stAccConfig.u64FSR / GRADIENT;

        switch (u32FSR)
        {
            case ACCEL_UI_FULL_SCALE_SET_2G:

            case ACCEL_UI_FULL_SCALE_SET_4G:

            case ACCEL_UI_FULL_SCALE_SET_8G:

            case ACCEL_UI_FULL_SCALE_SET_16G:
                stMSensorParam->stMSensorConfig.stAccConfig.u64FSR = u32FSR;
                break;

            default:
                print_info("not support Accel FSR!\n");
                s32Ret = HI_FAILURE;
                break;
        }

        /*set accel valid data bit*/
        stMSensorParam->stMSensorConfig.stAccConfig.u8DataWidth = ICM20690_VALID_DATA_BIT;
    }

    //print_info("gyro u32Odr:%lld,range:%lld\n",stMSensorParam->stMSensorConfig.stGyroConfig.u64ODR,
    //stMSensorParam->stMSensorConfig.stGyroConfig.u64FSR);
    //print_info("acc u32Odr:%lld,range:%lld\n",stMSensorParam->stMSensorConfig.stAccConfig.u64ODR,
    //stMSensorParam->stMSensorConfig.stAccConfig.u64FSR);
    osal_memcpy(&stMSensorMode, &stMSensorParam->stMSensorAttr, sizeof(MSENSOR_ATTR_S));
    return s32Ret;
}

static HI_S32 ICM20690_AxisFifoSensorInit(HI_U32 u32DevMode)
{
    //HI_U8 tmp_val;
    HI_S32 s32Ret;

    /*1. reset ICM20690 */
    s32Ret = ICM20690_Reset();

    if (s32Ret)
    {
        print_info("reset ICM20690 failed\n");
        return -EAGAIN;
    }

    msleep(100);
#ifdef TRANSFER_SPI
    //#ifdef TRANSFER_TYPE_SPI
    //disable i2c transfer
    s32Ret = HI_I2C_Disable();

    if (s32Ret)
    {
        print_info("disable i2c failed\n");
        return -EAGAIN;
    }

#endif

    /*2. enable  PLL, CLKSEL =  1*/
    s32Ret = ICM20690_SetClk();

    if (s32Ret)
    {
        print_info("ICM20690_SetClk failed\n");
        return -EAGAIN;
    }

    msleep(30);

    /*3. enable  gyro, accel*/
    s32Ret = ICM20690_SetAxisMode(u32DevMode);

    if (s32Ret)
    {
        print_info("ICM20690_SetAxisMode failed\n");
        return -EAGAIN;
    }

    msleep(30);

    /*4. set sample rate to 1KHz,1000/(1+0) */
    if (MSENSOR_DEVICE_GYRO & u32DevMode)
    {
        if (ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR <= 1000)
        {
            print_info("ICM20690_dev->stGyroStatus.u32Odr = %lld\n", ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR);
            s32Ret = ICM20690_SetSampleRate(ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR);

            if (s32Ret)
            {
                print_info("ICM20690_SetSampleRate failed\n");
                return -EAGAIN;
            }

            msleep(5);
        }
    }
    else
    {
        if (ICM20690_dev->stAccStatus.stAccConfig.u64ODR <= 1000)
        {
            //print_info("ICM20690_dev->stGyroStatus.u32Odr = %d\n", ICM20690_dev->stAccStatus.stAccConfig.u64ODR);
            s32Ret = ICM20690_SetSampleRate(ICM20690_dev->stAccStatus.stAccConfig.u64ODR);

            if (s32Ret)
            {
                print_info("ICM20690_SetSampleRate failed\n");
                return -EAGAIN;
            }

            msleep(5);
        }
    }

    if (ICM20690_dev->u8FifoEn)
    {
        /*5.disable FIFO_MODE, DLFP_CFG = 0*/
        s32Ret = ICM20690_UI_FifoModeEnable(u32DevMode);

        if (s32Ret)
        {
            print_info("ICM20690_UI_6FifoModeEnable failed(%d)\n", s32Ret);
            return -EAGAIN;
        }

        msleep(5);

        ICM20690_dev->FIFO_buf = osal_kmalloc(1024, osal_gfp_kernel);

        if (!ICM20690_dev->FIFO_buf) {
            print_info("kzalloc FIFO_buf failed\n");
            return -ENOMEM;
        }

        osal_memset(ICM20690_dev->FIFO_buf,0,1024);
    }

    if (TRIGER_EXTERN_INTERRUPT == ICM20690_dev->stTrigerConfig.eTrigerMode)
    {
        /*interrupt config*/
        s32Ret = ICM20690_INTConfig();

        if (s32Ret)
        {
            print_info("INT config failed\n");
            return -EAGAIN;
        }
    }

    if (MSENSOR_DEVICE_GYRO & u32DevMode)
    {
        /*6. 250dps, 8K ODR, 250Hz BW for gyro*/
        s32Ret = ICM20690_SetGyroAttr(ICM20690_dev->stGyroStatus);

        if (s32Ret)
        {
            print_info("ICM20690_SetGyroAttr failed\n");
            goto err_kzalloc;
        }

        msleep(5);
    }

    if (MSENSOR_DEVICE_ACC & u32DevMode)
    {
        /*7. accel FSR setting: UI accel to 4G, OIS accel to 2G*/
        s32Ret = ICM20690_SetAccelAttr(ICM20690_dev->stAccStatus);

        if (s32Ret)
        {
            print_info("ICM20690_SetAccelAttr failed\n");
            goto err_kzalloc;
        }

        msleep(5);
    }

    /*9. disable gyro low power mode*/
    s32Ret = ICM20690_GyroLowPowerModeDisable();

    if (s32Ret)
    {
        print_info("ICM20690_GyroLowPowerModeDisable failed\n");
        goto err_kzalloc;
    }

    return HI_SUCCESS;
err_kzalloc:

    if (ICM20690_dev->FIFO_buf)
    {
        kfree(ICM20690_dev->FIFO_buf);
        ICM20690_dev->FIFO_buf = NULL;
    }

    return -EAGAIN;
}

static HI_VOID ICM20690_AxisFifoSensorDeInit(void)
{
    if (ICM20690_dev->FIFO_buf != NULL)
    {
        kfree(ICM20690_dev->FIFO_buf);
        ICM20690_dev->FIFO_buf = NULL;
    }

}

static HI_S32 ICM20690_SensorInit(HI_U32 u32DevMode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if ((u32DevMode & MSENSOR_DEVICE_GYRO) || (u32DevMode & MSENSOR_DEVICE_ACC))
    {
        s32Ret = ICM20690_AxisFifoSensorInit(u32DevMode);

        if (HI_SUCCESS != s32Ret)
        {
            print_info("ICM20690_SensorInit failed! ret=%x\n", s32Ret);
        }
    }
    else
    {
        print_info("ICM20690_SensorInit not support this mode : %d\n", u32DevMode);
        s32Ret = HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 HI_MOTIONSENSOR_SaveData_ModeDofFifo(MSENSOR_ATTR_S stMSensorAttr)
{
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UL  time_inter = 0;
	HI_U32 u32Adddatacnt = 0;

    if (NULL == MotionSensorStatus)
    {
        print_info("MotionSensorStatus is NULL!!!! MotionSensorStatus(%p)\n", MotionSensorStatus);
        return HI_FAILURE;
    }

    //print_info("axis_mode:%d\n",axis_mode);

    HI_ASSERT(HI_NULL != MotionSensorStatus);

    //osal_printk("recordNum:%d\n", ICM20690_dev->recordNum);

	if(su8datacount < DATA_RECORD_CNT)
	{
		saEveryDataCntBackup[su8datacount] = ICM20690_dev->recordNum;
	}
	else
	{
		for(i=0; i < DATA_RECORD_CNT - 1;i++)
		{
			saEveryDataCntBackup[i]          = saEveryDataCntBackup[i+1];
		}
		saEveryDataCntBackup[DATA_RECORD_CNT - 1] = ICM20690_dev->recordNum;
	}
    if(su8Timecnt < TIME_RECORD_CNT)
	{
		saTimeBackup[su8Timecnt] = u64PTSNow;
		//saEveryDataCntBackup[su8Timecnt] = ICM20690_dev->recordNum;

		for(i=0; i <= su8datacount;i++)
		{
			u32Adddatacnt += saEveryDataCntBackup[i];
		}
		//osal_printk("time:%lld,%lld,num:%d,%d\n",saTimeBackup[su8Timecnt], saTimeBackup[0],u32Adddatacnt,su8Timecnt);
		time_inter = osal_div_u64(saTimeBackup[su8Timecnt] - saTimeBackup[0],DIV_0_TO_1(u32Adddatacnt));
		//osal_printk("inter:%ld,last:%lld\n",time_inter,ICM20690_dev->stGyroStatus.u64LastPts);
		
	}
	else
	{
		for(i=0; i < TIME_RECORD_CNT - 1;i++)
		{
			saTimeBackup[i] = saTimeBackup[i+1];

		}

		saTimeBackup[TIME_RECORD_CNT - 1] = u64PTSNow;
		for(i=1; i < DATA_RECORD_CNT;i++)
		{
			u32Adddatacnt += saEveryDataCntBackup[i];
		}
		//osal_printk("(%d)time:%lld,%lld,num:%d,now:%lld\n",__LINE__,saTimeBackup[TIME_RECORD_CNT - 1], saTimeBackup[0],u32Adddatacnt);
		time_inter = osal_div_u64(saTimeBackup[TIME_RECORD_CNT - 1] - saTimeBackup[0],DIV_0_TO_1(u32Adddatacnt));
        //osal_printk("line:%d,%lld\n",__LINE__,saTimeBackup[TIME_RECORD_CNT - 1] - saTimeBackup[0]);
	}
    if(su8datacount < DATA_RECORD_CNT)
    {
        su8datacount++;
    }
    //time_inter = osal_div_u64((u64PTSNow - ICM20690_dev->stGyroStatus.u64LastPts) * SCALE_TIMES, DIV_0_TO_1(ICM20690_dev->recordNum));

    osal_memcpy(&stIMUdata.stMSensorAttr, &stMSensorAttr, sizeof(MSENSOR_ATTR_S));

	//osal_printk("inter:%ld,last:%lld\n",time_inter,ICM20690_dev->stGyroStatus.u64LastPts);
    for (i = 0; i < ICM20690_dev->recordNum; i++)
    {
        if (stMSensorAttr.u32DeviceMask == (MSENSOR_DEVICE_GYRO | MSENSOR_DEVICE_ACC))
        {


            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32XData =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 0]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 1] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32YData = (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 2]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 3] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32ZData =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 4]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 5] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32Temp = ICM20690_dev->s32temperature;
            //stIMUdata.stMsensorAccBuffer.astAccData[i].u64PTS = u64PTSNow - (ICM20690_dev->recordNum - i - 1) * time_inter / SCALE_TIMES;
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].u64PTS = ICM20690_dev->stGyroStatus.u64LastPts + time_inter;
			ICM20690_dev->stAccStatus.u64LastPts = stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].u64PTS;
			stIMUdata.stMsensorAccBuffer.u32BuffDataNum++;

            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32XData  =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 6]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 7] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32YData  = (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 8]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 9] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32ZData  =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 10]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 11] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32Temp = ICM20690_dev->s32temperature;

            //stIMUdata.stMsensorGyroBuffer.astGyroData[i].u64PTS = u64PTSNow - (ICM20690_dev->recordNum - i - 1) * time_inter / SCALE_TIMES;
             stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].u64PTS = ICM20690_dev->stGyroStatus.u64LastPts + time_inter;

            ICM20690_dev->stGyroStatus.u64LastPts = stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].u64PTS;
             stIMUdata.stMsensorGyroBuffer.u32BuffDataNum++;

        }
        else if (stMSensorAttr.u32DeviceMask == MSENSOR_DEVICE_GYRO)
        {	
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32XData  =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 0]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 1] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32YData  = (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 2]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 3] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32ZData  =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 4]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 5] & 0xff);
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].s32Temp = ICM20690_dev->s32temperature;
            stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].u64PTS = u64PTSNow - (ICM20690_dev->recordNum - i - 1) * time_inter;
            /*PTS  handle*/
            //ICM20690_dev->stGyroStatus.u64LastPts = u64PTSNow;
            ICM20690_dev->stGyroStatus.u64LastPts = stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum].u64PTS;
            stIMUdata.stMsensorGyroBuffer.u32BuffDataNum++;
        }
        else if (stMSensorAttr.u32DeviceMask == MSENSOR_DEVICE_ACC)
        {
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32XData =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 0]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 1] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32YData = (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 2]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 3] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32ZData =  (short)(((short)(ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 4]) << 8) & 0xff00) \
                    | (ICM20690_dev->FIFO_buf[i * su8GyroAccDataLen + 5] & 0xff);
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].s32Temp = ICM20690_dev->s32temperature;
            stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].u64PTS = u64PTSNow - (ICM20690_dev->recordNum - i - 1) * time_inter;
            //ICM20690_dev->stAccStatus.u64LastPts = u64PTSNow;
            ICM20690_dev->stAccStatus.u64LastPts = stIMUdata.stMsensorAccBuffer.astAccData[stIMUdata.stMsensorAccBuffer.u32BuffDataNum].u64PTS;
            stIMUdata.stMsensorAccBuffer.u32BuffDataNum++;
        }

#ifdef TEST_DEBUG

        test_fifo[su32DataCount].stGyroData.s32XData	= stIMUdata.stMsensorGyroBuffer.astGyroData[i].s32XData;
        test_fifo[su32DataCount].stGyroData.s32YData	= stIMUdata.stMsensorGyroBuffer.astGyroData[i].s32YData;
        test_fifo[su32DataCount].stGyroData.s32ZData   = stIMUdata.stMsensorGyroBuffer.astGyroData[i].s32ZData;
        test_fifo[su32DataCount].stGyroData.s32Temp = stIMUdata.stMsensorGyroBuffer.astGyroData[i].s32Temp;
        test_fifo[su32DataCount].stGyroData.u64PTS 	= stIMUdata.stMsensorGyroBuffer.astGyroData[i].u64PTS;

        if (stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC)
        {
            test_fifo[su32DataCount].stAccData.s32XData = stIMUdata.stMsensorAccBuffer.astAccData[i].s32XData;
            test_fifo[su32DataCount].stAccData.s32YData = stIMUdata.stMsensorAccBuffer.astAccData[i].s32YData;
            test_fifo[su32DataCount].stAccData.s32ZData = stIMUdata.stMsensorAccBuffer.astAccData[i].s32ZData;
            test_fifo[su32DataCount].stAccData.s32Temp = stIMUdata.stMsensorAccBuffer.astAccData[i].s32Temp;
            test_fifo[su32DataCount].stAccData.u64PTS = stIMUdata.stMsensorAccBuffer.astAccData[i].u64PTS;
        }

        su32DataCount++;
#endif
    }

    if(su8Timecnt < TIME_RECORD_CNT)
    {
        saTimeBackup[su8Timecnt] = ICM20690_dev->stGyroStatus.u64LastPts;
        su8Timecnt++;
    }
    else
    {
    	saTimeBackup[TIME_RECORD_CNT - 1] = ICM20690_dev->stGyroStatus.u64LastPts;
    }

    return s32Ret;
}

HI_S32 MOTIONSENSOR_GetData(MSENSOR_ATTR_S stMSensorAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
//
    if ((stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_GYRO) || (stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC))
    {
        s32Ret = HI_MOTIONSENSOR_SaveData_ModeDofFifo(stMSensorAttr);
    }
    else
    {
        print_info("[error]Not support MODE\n");
        s32Ret = HI_FAILURE;
    }
    return s32Ret;
}


static HI_VOID ICM20690_SensorDeInit(HI_S32 axis_mode)
{
    if ((axis_mode & MSENSOR_DEVICE_GYRO) || (axis_mode & MSENSOR_DEVICE_ACC))
    {
        ICM20690_AxisFifoSensorDeInit();
    }
    else
    {
        print_info("ICM20690_SensorInit not support this mode : %d\n", axis_mode);
    }

    //mutex_destroy(&ICM20690_dev->mutex);
    return;
}


static HI_S32 ICM20690_ParamInit(MSENSOR_PARAM_S stMSensorParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (MSENSOR_DEVICE_GYRO & stMSensorParam.stMSensorAttr.u32DeviceMask)
    {
        /*gyro attr set*/
        if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR == GYRO_OUTPUT_DATA_RATE_32KHZ)
        {
            //default :GYRO_BAND_WIDTH_3600HZ
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_3600HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR == GYRO_OUTPUT_DATA_RATE_8KHZ)
        {
            //default :GYRO_BAND_WIDTH_250HZ
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_250HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR <= 100)
        {
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_20HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR <= 300)
        {
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_41HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR <= 500)
        {
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_92HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR <= 1000)
        {
            ICM20690_dev->stGyroStatus.u32BandWidth = GYRO_BAND_WIDTH_184HZ;
            ICM20690_dev->stGyroStatus.stGyroConfig.u64ODR = stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR;
        }
        else
        {
            print_info("ICM20690_ParamInit failed! gyro_odr:%lld not found !\n", stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR);
            s32Ret = HI_FAILURE;
        }

        switch (stMSensorParam.stMSensorConfig.stGyroConfig.u64FSR)
        {
            case GYRO_FULL_SCALE_SET_2KDPS:

            case GYRO_FULL_SCALE_SET_1KDPS:

            case GYRO_FULL_SCALE_SET_500DPS:

            case GYRO_FULL_SCALE_SET_250DPS:

            case GYRO_FULL_SCALE_SET_31DPS:

            case GYRO_FULL_SCALE_SET_62DPS:

            case GYRO_FULL_SCALE_SET_125DPS:
                ICM20690_dev->stGyroStatus.stGyroConfig.u64FSR = stMSensorParam.stMSensorConfig.stGyroConfig.u64FSR;
                break;

            default:
                print_info("ICM20690_ParamInit failed! gyro_fsr:%lld not found !\n", stMSensorParam.stMSensorConfig.stGyroConfig.u64FSR);
                s32Ret = HI_FAILURE;
                break;
        }
    }

    /*accel attr set*/
    if (MSENSOR_DEVICE_ACC & stMSensorParam.stMSensorAttr.u32DeviceMask)
    {
        if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR == ACCEL_OUTPUT_DATA_RATE_4KHZ)
        {
            //default :ACCEL_BAND_WIDTH_1046HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_1046HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR == ACCEL_OUTPUT_DATA_RATE_1KHZ)
        {
            //default :ACCEL_BAND_WIDTH_420HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_420HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR < 100)
        {
            //default :ACCEL_BAND_WIDTH_218HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_21HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR < 300)
        {
            //default :ACCEL_BAND_WIDTH_218HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_44HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR < 500)
        {
            //default :ACCEL_BAND_WIDTH_218HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_99HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else if (stMSensorParam.stMSensorConfig.stAccConfig.u64ODR < 1000)
        {
            //default :ACCEL_BAND_WIDTH_218HZ
            ICM20690_dev->stAccStatus.u32BandWidth = ACCEL_BAND_WIDTH_218HZ;
            ICM20690_dev->stAccStatus.stAccConfig.u64ODR = stMSensorParam.stMSensorConfig.stAccConfig.u64ODR;
        }
        else
        {
            print_info("ICM20690_ParamInit failed! gyro_odr:%lld not found !\n", stMSensorParam.stMSensorConfig.stAccConfig.u64ODR);
            s32Ret = HI_FAILURE;
        }

        switch (stMSensorParam.stMSensorConfig.stAccConfig.u64FSR)
        {
            case ACCEL_UI_FULL_SCALE_SET_2G:

            case ACCEL_UI_FULL_SCALE_SET_4G:

            case ACCEL_UI_FULL_SCALE_SET_8G:

            case ACCEL_UI_FULL_SCALE_SET_16G:
                ICM20690_dev->stAccStatus.stAccConfig.u64FSR = stMSensorParam.stMSensorConfig.stAccConfig.u64FSR;
                break;

            default:
                print_info("ICM20690_ParamInit failed! accel_range:%lld not found !\n", stMSensorParam.stMSensorConfig.stAccConfig.u64FSR);
                s32Ret = HI_FAILURE;
                break;
        }
    }

    print_info("gyro odr:%lld,acc odr:%lld\n", stMSensorParam.stMSensorConfig.stGyroConfig.u64ODR,
               stMSensorParam.stMSensorConfig.stAccConfig.u64ODR);
    return s32Ret;

}

static HI_S32 ICM20690_ConfigToParam(MSENSOR_PARAM_S* stMSensorParam)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!(stMSensorParam))
    {
        print_info("MotionSensor_status is NULL\n");
        return HI_FAILURE;
    }

    if ((stMSensorParam->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_GYRO) || (stMSensorParam->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC))
    {
        s32Ret = ICM20690_SetParam(stMSensorParam);

        if (HI_SUCCESS != s32Ret)
        {
            print_info("ICM20690_SetParam failed! ret=%x\n", s32Ret);
        }
    }
    else
    {
        print_info("not support mode!\n");
        s32Ret = HI_FAILURE;
    }

    return s32Ret;
}


HI_S32 HI_MotionSensor_DataHandle(void* data)
{
    MSENSOR_ATTR_S stMSensorAttr = *(MSENSOR_ATTR_S*)data;
    //static MSENSOR_DATA_S stMotionSensorData;
    HI_S32 s32Ret;

    //HI_U32 time_inter;
    //MOTION_SENSOR_OUT_DATA_S tmp_data;
    //print_info("start to get data!!\n");
    while (!kthread_should_stop())
    {

        if ((stMSensorAttr.u32TemperatureMask & MSENSOR_TEMP_GYRO) ||
            (stMSensorAttr.u32TemperatureMask & MSENSOR_TEMP_ACC))
        {
            if (i_thread % 50 == 0)
            {
                s32Ret = ICM20690_GetTemperature(&ICM20690_dev->s32temperature);

                if (HI_SUCCESS != s32Ret)
                {
                    print_info("get_temp failed! ret=%x\n", s32Ret);
                    return s32Ret;
                }

            }
        }

        //osal_printk("***fun:%s line:%d***\n",__func__,__LINE__);
        osal_down_interruptible(&ICM20690_dev->g_sem);


#if 0

        if (kthread_should_stop())
        {
            break;
        }

#endif

        s32Ret = FIFO_DATA_UPDATE();

        if (HI_SUCCESS != s32Ret)
        {
            print_info("ICM20690_UI_FIFO_DATA_UPDATE failed\n");
            continue;
            //return HI_FAILURE;
        }


        s32Ret = MOTIONSENSOR_GetData(stMSensorAttr);

        if (HI_SUCCESS != s32Ret)
        {
            print_info("MOTIONSENSOR_GetData failed! ret=%x\n", s32Ret);
            Reset_Fifo();
            continue;
            //return HI_FAILURE;
        }

        i_thread ++;

    }

    return HI_SUCCESS;
}


HI_VOID HI_MotionSensor_TimerStart(HI_VOID)
{
#ifndef __HuaweiLite__
    ktime_t stime;
    stime = ktime_set(0, ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval * NSEC_PER_USEC);
#else
    union ktime stime;

    stime.tv.sec = 0;
    stime.tv.usec = ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval;
#endif
    hrtimer_start(&ICM20690_dev->hrtimer, stime, HRTIMER_MODE_REL);
}
#ifdef TEST_DEBUG
volatile static HI_BOOL print_debug = HI_FALSE;
#endif
static void HI_MotionSensor_GetData_ATTR(void)
{
    HI_S32 s32Ret;

    if (ICM20690_dev->u8EnableKthread == HI_TRUE && su32DataCount < 1000)
    {

        /* get PTS*/
        if (!s8TimerFristFlag)
        {
        	
            HI_U64 u64TimeNow = HI_MotionSensor_GetCurPts();
            ICM20690_dev->stGyroStatus.u64LastPts = u64TimeNow;
            ICM20690_dev->stAccStatus.u64LastPts = u64TimeNow;
            ICM20690_dev->stMagnStatus.u64LastPts = u64TimeNow;
            u64Time_last = u64TimeNow;
            s8TimerFristFlag = HI_TRUE;
            u64PTSNow = u64TimeNow;
			saTimeBackup[su8Timecnt] = u64PTSNow;
			su8Timecnt++;
			
            FIFO_DATA_RESET();
            return;
        }

        if (ICM20690_dev->u8FifoEn)
        {
            /*get fifo count*/
            s32Ret = ICM20690_GetFifoLengthAndCount();

            if (HI_SUCCESS != s32Ret)
            {
                print_info("get_ICM20690_fifo_length_and_count failed\n");
                return;
            }

        }

        u64PTSNow = HI_MotionSensor_GetCurPts();

        //i_hande ++;
        /*wakeup thread*/
        //if(ICM20690_dev->u8FifoEn)
        //{
        //    osal_up(&ICM20690_dev->g_sem);
        //}
    }

#ifdef TEST_DEBUG
    //test
    else if (!print_debug)
    {

        print_info("time = %llu , data_count = %d\n", u64PTSNow - u64Time_last, su32DataCount);

        for (i = 0; i < su32DataCount; i++)
        {
            /*print_info("stGyroData.temperature = %d, stGyroData.x = %d, stGyroData.y = %d, stGyroData.z = %d,pts:%lld\n",
                       test_fifo[i].stGyroData.s32Temp, test_fifo[i].stGyroData.s32XData, test_fifo[i].stGyroData.s32YData,
                       test_fifo[i].stGyroData.s32ZData,test_fifo[i].stGyroData.u64PTS);
            print_info("stAccData.temperature = %d, stAccData.x = %d, stAccData.y = %d, stAccData.z = %d, pts:%lld\n",
                       test_fifo[i].stAccData.s32Temp, test_fifo[i].stAccData.s32XData, test_fifo[i].stAccData.s32YData,
                       test_fifo[i].stAccData.s32ZData,test_fifo[i].stAccData.u64PTS);*/
            printk("%d, %d, %d, %d,%lld\n",
                   test_fifo[i].stGyroData.s32XData, test_fifo[i].stGyroData.s32YData,
                   test_fifo[i].stGyroData.s32ZData, test_fifo[i].stGyroData.s32Temp, test_fifo[i].stGyroData.u64PTS);
        }
        print_debug = HI_TRUE;

    }

#endif
}
HI_U64 su64testtime;
static HI_S32 HI_GetData_Handle(void);

static enum hrtimer_restart timer_hr_interrupt(struct hrtimer* timer)
{
    //HI_UL  flags;
    HI_S32 s32Ret = HI_SUCCESS;

#ifndef __HuaweiLite__
    ktime_t stime;
    stime = ktime_set(0, ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval * NSEC_PER_USEC);
    hrtimer_forward_now(timer, stime);
#endif
    //HI_MotionSensor_GetData_ATTR();
    s32Ret = osal_schedule_work(&ICM20690_dev->work);
    if(s32Ret == HI_FALSE) {
        return HI_FAILURE;
    }

    ICM20690_dev->s32WorkqueueCallTimes++;

    return HRTIMER_RESTART;
}


static HI_S32 HI_MotionSensor_TimerInit(void)
{
#ifdef __HuaweiLite__
    HI_S32 s32Ret;
    union ktime time;

    time.tv.sec = 0;

    time.tv.usec = ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval;

    s32Ret = hrtimer_create(&ICM20690_dev->hrtimer, time, timer_hr_interrupt);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("create tiemr failed!(%d)\n", s32Ret);
    }

#else
    hrtimer_init(&ICM20690_dev->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    ICM20690_dev->hrtimer.function = timer_hr_interrupt;
#endif
    return HI_SUCCESS;
}

static HI_S32 HI_MotionSensor_TimerDeInit(void)
{
    hrtimer_cancel(&ICM20690_dev->hrtimer);
    return HI_SUCCESS;
}

static HI_S32 HI_GetData_Handle(void)
{
    //static MSENSOR_DATA_S stMotionSensorData;
    HI_S32 s32Ret;

    //print_info("start to get data!!\n");
	if(!ICM20690_dev->recordNum)
		return HI_SUCCESS;

    if ((stMSensorMode.u32TemperatureMask & MSENSOR_TEMP_GYRO) ||
        (stMSensorMode.u32TemperatureMask & MSENSOR_TEMP_ACC))
    {
        if (i_thread % 50 == 0)
        {
            s32Ret = ICM20690_GetTemperature(&ICM20690_dev->s32temperature);

            if (HI_SUCCESS != s32Ret)
            {
                print_info("get_temp failed! ret=%x\n", s32Ret);
                return s32Ret;
            }

        }
    }

    s32Ret = FIFO_DATA_UPDATE();

    if (HI_SUCCESS != s32Ret)
    {
        print_info("ICM20690_UI_FIFO_DATA_UPDATE failed\n");

        return HI_FAILURE;
    }


    s32Ret = MOTIONSENSOR_GetData(stMSensorMode);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("MOTIONSENSOR_GetData failed! ret=%x\n", s32Ret);
        Reset_Fifo();

        return HI_FAILURE;
    }

    i_thread ++;

    return HI_SUCCESS;

}
HI_U64 lastime;

HI_S32 ICM20690_Data_TransferIMU(MSENSOR_DATA_S *pstMotiondata)
{
    HI_S32 s32Ret;
    HI_UL  flags;
#ifndef __HuaweiLite__
    ktime_t stime;
    stime = ktime_set(0, ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval * NSEC_PER_USEC);
#else
    union ktime stime;

    stime.tv.sec = 0;
    stime.tv.usec = ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval;
#endif



    osal_spin_lock_irqsave(&lockGetData,&flags);
    HI_MotionSensor_GetData_ATTR();

    if (HI_FALSE == ICM20690_dev->u8FifoEn)
    {
        s32Ret = ICM20690_ReadData(&stIMUdata);

        if (s32Ret)
        {
            print_info("read Data failed: %d\n", s32Ret);
            return HI_FAILURE;
        }

    }
    else
    {
            s32Ret = HI_GetData_Handle();
            if (s32Ret)
            {
                print_info("read Data failed: %d\n", s32Ret);
                return HI_FAILURE;
            }


    }

    //osal_printk("~~~~~u32DeviceMask:%d u32GyroCount:%d~~~\n",stIMUdata.stMSensorAttr.u32DeviceMask,stIMUdata.stMsensorGyroBuffer.u32BuffDataNum);

//       osal_printk("num:%d,lastpst:%lld,pts now:%lld\n",stIMUdata.stMsensorGyroBuffer.u32BuffDataNum,
//       stIMUdata.stMsensorGyroBuffer.astGyroData[stIMUdata.stMsensorGyroBuffer.u32BuffDataNum -1].u64PTS,u64PTSNow);

//#ifdef MNGBUFF_ENABLE
    osal_memcpy(pstMotiondata,&stIMUdata,sizeof(MSENSOR_DATA_S));

    MotionSensorDev_IntCallBack(&stIMUdata);

//#endif
    osal_memset(&stIMUdata, 0, sizeof(MSENSOR_DATA_S));
    osal_spin_unlock_irqrestore(&lockGetData, &flags);
#ifndef __HuaweiLite__
    ///hrtimer_forward_now(&ICM20690_dev->hrtimer, stime);
#else
    hrtimer_forward(&ICM20690_dev->hrtimer, stime);
#endif
    

    return HI_SUCCESS;


}
static HI_S32 ICM20690_clear_irq(void)
{
    HI_U8 ch, u8Ret;
    /*read INT status*/
    u8Ret = HI_ICM20690_Transfer_read(INT_STATUS, &ch, 1, HI_FALSE);

    if (u8Ret)
    {
        print_info("read INT status failed\n");
        return HI_FAILURE;
    }

    /*read watermark status*/
    if (ICM20690_dev->u8FifoEn)
    {
        u8Ret = HI_ICM20690_Transfer_read(FIFO_WM_INT_STATUS, &ch, 1, HI_FALSE);

        if (u8Ret)
        {
            print_info("read INT status failed\n");
            return HI_FAILURE;
        }
    }

    //print_info("INT_STATUS:%d\n",ch);

    return HI_SUCCESS;
}



static void ICM20690_work(struct osal_work_struct* work)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UL  flags;
    //static MSENSOR_DATA_S stIMUdata;
    //VI_PIPE ViPipe = 0;
    //HI_U64 nowtime = HI_MotionSensor_GetCurPts();
    //osal_printk("work time:%lld\n",nowtime - lastime);
    //lastime = nowtime;

    osal_spin_lock_irqsave(&lockGetData,&flags);
    //osal_printk("timer_hr_interrupt\n");
    if(ICM20690_dev->u8EnableKthread)
    {
        HI_MotionSensor_GetData_ATTR();
        if ((HI_FALSE == ICM20690_dev->u8FifoEn) && (su32DataCount < 1000))
        {

            //osal_down_interruptible(&ICM20690_dev->g_sem);
            s32Ret = ICM20690_ReadData(&stIMUdata);

            if (s32Ret)
            {
                print_info("ReadData failed: %d\n", s32Ret);
                goto EXIT;
            }

            //#ifdef MNGBUFF_ENABLE
            //Debug Only
            //MotionSensorDev_IntCallBack(&stIMUdata);
            //#endif
        }
        else
        {
            s32Ret = HI_GetData_Handle();

            if (s32Ret)
            {
                print_info("GetData_Handle failed: %d\n", s32Ret);
                goto EXIT;
            }
        }
    }

    if(stIMUdata.stMsensorAccBuffer.u32BuffDataNum> 40 || stIMUdata.stMsensorGyroBuffer.u32BuffDataNum > 40)
    {
        MotionSensorDev_IntCallBack(&stIMUdata);
        osal_memset(&stIMUdata, 0, sizeof(MSENSOR_DATA_S));
    }

EXIT:
    ICM20690_dev->s32WorkqueueCallTimes--;
    osal_wakeup(&ICM20690_dev->stWaitCallStopWorking);
    osal_spin_unlock_irqrestore(&lockGetData, &flags);

}

#ifndef __HuaweiLite__
static irqreturn_t ICM20690_IRQ(int irq, void* data)
#else
static void ICM20690_IRQ(HI_U32 irq, void* data)
#endif
{
    HI_S32 s32Ret = 0;
    //HI_U64 u64FramPTS = 0;
    HI_U64 u64CurrPTS = 0;
    static HI_U64 u64Last1s = 0;
    static HI_U64 u64LastPTS = 0;
    static HI_U32 int_cnt = 0;

    //static MSENSOR_DATA_S stIMUdata;

    int_cnt++;
    u64CurrPTS = CALL_SYS_GetTimeStamp();
    s32Ret = ICM20690_clear_irq();

    if (s32Ret)
    {
        print_info("Clear irq status failed\n");
        return HI_FAILURE;
    }

	HI_MotionSensor_GetData_ATTR();
    //schedule_work(&ICM20690_dev->work);
	osal_schedule_work(&ICM20690_dev->work);

    if (!u64Last1s)
    { u64Last1s = u64CurrPTS; }

#if 0

    if (u64FramPTS)
    {
        HI_U32 u32Diff = u64CurrPTS - u64FramPTS;
        HI_TRACE_MSENSOR(HI_DBG_ERR, "u64FramPTS:%llu gyroPTS:%llu,diff:%d\n", u64FramPTS, u64CurrPTS, u32Diff);
    }

#endif

    if (u64Last1s && (u64CurrPTS - u64Last1s > 1000000))
    {
        //HI_TRACE_MSENSOR(HI_DBG_ERR, "1s int_cnt: %d\n", int_cnt);
        u64Last1s = u64CurrPTS;
        int_cnt = 0;
    }


    //if (u64LastPTS)
    //        HI_TRACE_MSENSOR(HI_DBG_ERR,"gyro diff:%d\n", (u64CurrPTS - u64LastPTS));
    u64LastPTS = u64CurrPTS;

#ifndef __HuaweiLite__
    return IRQ_HANDLED;
#endif
}

HI_S32 HI_MotionSensor_INTERRUPTRun(void)
{
    HI_S32 s32Ret = 0;

	ICM20690_dev->u8EnableKthread = HI_TRUE;
    s32Ret = osal_init_work(&ICM20690_dev->work, ICM20690_work);

    //INIT_WORK(&ICM20690_dev->work, ICM20690_work);
#ifndef __HuaweiLite__
    ICM20690_dev->s32IRQNum = gpio_to_irq(gpio_num(INT_GPIO_CHIP, INT_GPIO_OFFSET));
    //print_info("IRQNum:%d\n",ICM20690_dev->s32IRQNum);

    s32Ret = request_threaded_irq(ICM20690_dev->s32IRQNum, NULL, ICM20690_IRQ, /*IRQF_TRIGGER_LOW*/IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "MotionSensor", ICM20690_dev->client);
#else
    group_bit_info.groupnumber   = ICM20690_dev->gd.group_num;
    group_bit_info.bitnumber     = ICM20690_dev->gd.bit_num;
    group_bit_info.direction     = GPIO_DIR_IN;
    group_bit_info.irq_type      = IRQ_TYPE_EDGE_FALLING;
    group_bit_info.irq_handler   = ICM20690_IRQ;
    group_bit_info.irq_enable    = GPIO_IRQ_ENABLE;
    group_bit_info.data          = &stMSensorMode;

    print_info("gpio:%d_%d\n", group_bit_info.groupnumber, group_bit_info.bitnumber);
    s32Ret =  gpio_direction_input(&group_bit_info);
    s32Ret |= gpio_irq_register(&group_bit_info);
    s32Ret |= gpio_set_irq_type(&group_bit_info);
    s32Ret |= gpio_irq_enable(&group_bit_info);
#endif

    if (s32Ret)
    {
        print_info("request irq failed: %d\n", s32Ret);
        return s32Ret;
    }



    return HI_SUCCESS;
}

HI_S32 HI_MotionSensor_INTERRUPTStop(void)
{
#ifndef __HuaweiLite__

    if (ICM20690_dev->s32IRQNum)
    {
        free_irq(ICM20690_dev->s32IRQNum, ICM20690_dev->client);
        ICM20690_dev->u8EnableKthread = HI_FALSE;
    }

#else

    if (group_bit_info.irq_enable)
    {
        group_bit_info.irq_enable = GPIO_IRQ_DISABLE;
        gpio_irq_enable(&group_bit_info);
        ICM20690_dev->u8EnableKthread = HI_FALSE;
    }

#endif
    else
    { print_info("irq has already free!!\n"); }

    osal_destroy_work(&ICM20690_dev->work);
    return HI_SUCCESS;
}

HI_S32 ICM20690_WaitStopWorkingCallBack(const HI_VOID* pParam)
{
    if (ICM20690_dev->s32WorkqueueCallTimes == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



HI_S32 HI_MotionSensor_TimerRun(void)
{
    HI_S32 s32Ret;
    ICM20690_dev->u8EnableKthread = HI_TRUE;
    s32Ret = HI_MotionSensor_TimerInit();

    if (HI_SUCCESS != s32Ret)
    {
        print_info("HI_MotionSensor_TimerInit failed\n");
        return HI_FAILURE;
    }
    //INIT_WORK(&ICM20690_dev->work, ICM20690_work);
    osal_init_work(&ICM20690_dev->work, ICM20690_work);
    su8Timecnt = 0;
    su8datacount = 0;
    s8TimerFristFlag = HI_FALSE;
	ICM20690_dev->recordNum = 0;

    HI_MotionSensor_TimerStart();

    return HI_SUCCESS;
}
HI_S32 HI_MotionSensor_TimerStop(void)
{
    HI_UL  flags;
    HI_S32 s32Ret;

#ifdef TEST_DEBUG

    while (!print_debug && su32DataCount != 0)
    {
        msleep(5);
    }

#endif

    s32Ret = HI_MotionSensor_TimerDeInit();

    if (HI_SUCCESS != s32Ret)
    {
        print_info("HI_MotionSensor_TimerInit failed\n");
        return HI_FAILURE;
    }
    
    osal_wait_event_uninterruptible(&ICM20690_dev->stWaitCallStopWorking, ICM20690_WaitStopWorkingCallBack, HI_NULL);
    
    osal_spin_lock_irqsave(&lockGetData,&flags);
    ICM20690_dev->u8EnableKthread = HI_FALSE;
    osal_destroy_work(&ICM20690_dev->work);
    osal_spin_unlock_irqrestore(&lockGetData, &flags);
    //su8Timecnt = 0;
    //   su8datacount = 0;
    //   s8TimerFristFlag = HI_FALSE;
    //ICM20690_dev->recordNum = 0;
	osal_memset(saTimeBackup,0,sizeof(saTimeBackup));
	osal_memset(saEveryDataCntBackup,0,sizeof(saEveryDataCntBackup));
    return HI_SUCCESS;
}
static HI_VOID ICM20690_TrigerMode_init(MSENSOR_PARAM_S* pstMSensorParam)
{
    ICM20690_dev->stTrigerConfig.eTrigerMode = TRIGER_TIMER; //TRIGER_TIMER;TRIGER_EXTERN_INTERRUPT;
    //ICM20690_dev->u8FifoEn = HI_FALSE;
    ICM20690_dev->u8FifoEn = HI_TRUE;

    if (TRIGER_TIMER == ICM20690_dev->stTrigerConfig.eTrigerMode)
    {
        if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR <= 50)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 1400000;
        }
        else if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR <= 200)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 350000;
        }
        else if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR <= 500)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 140000;
        }
        else if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR <= 1000)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 50000;//70000;
        }
        else if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR == 8000)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 8750;
        }
        else if (pstMSensorParam->stMSensorConfig.stGyroConfig.u64ODR == 32000)
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 2188;
        }
        else
        {
            ICM20690_dev->stTrigerConfig.uTrigerInfo.stTimerConfig.u32interval = 70000;
        }
    }
    else if (TRIGER_EXTERN_INTERRUPT == ICM20690_dev->stTrigerConfig.eTrigerMode)
    {
        ICM20690_dev->stTrigerConfig.uTrigerInfo.stExternInterruptConfig.u32Interrupt_num = ICM20690_FIFO_MAX_RECORD;
    }
}

static HI_S32 ICM20690Dev_init(MSENSOR_PARAM_S* stMSensorParam)
{
    HI_S32 s32Ret;

    osal_printk("######fun:%s line:%d MotionSensor_status:%p######\n", __func__, __LINE__, stMSensorParam);

    s32Ret = ICM20690_ConfigToParam(stMSensorParam);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("ICM20690_ConfigToParam failed! ret=%x\n", s32Ret);
        goto ERR_INIT;
    }

    osal_printk("######fun:%s line:%d ######\n", __func__, __LINE__);

    s32Ret = ICM20690_ParamInit(*stMSensorParam);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("ICM20690_ParamInit failed! ret=%x\n", s32Ret);
        goto ERR_INIT;
    }

    ICM20690_TrigerMode_init(stMSensorParam);
    s32Ret = ICM20690_SensorInit(stMSensorParam->stMSensorAttr.u32DeviceMask);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("ICM20690_SensorInit failed! ret=%x\n", s32Ret);
        goto ERR_INIT;
    }

    //mutex_init(&ICM20690_dev->mutex);
    return HI_SUCCESS;

ERR_INIT:
#ifndef __HuaweiLite__
#ifdef TRANSFER_I2C
    MotionSersor_i2c_exit(&ICM20690_dev->client);
#elif defined TRANSFER_SPI
    MotionSersor_SPI_deinit(ICM20690_dev->hi_spi);
#endif
#endif

    return HI_FAILURE;

}

void ICM20690Dev_exit(MSENSOR_PARAM_S* pstMSensorParam)
{
    /*Exit may need to be forced out when Deinit is not called*/
    ICM20690_SensorDeInit(pstMSensorParam->stMSensorAttr.u32DeviceMask);
}
HI_U8 HI_MotionSensor_DevID_Read(void)
{
    HI_S32 s32Ret;
    HI_U8 u8DevID = 0;
    HI_U8 i = 0;
    for(i = 0 ; i < 5; i++)
    {
        u8DevID = 0;
       s32Ret = HI_ICM20690_Transfer_read(0x75, &u8DevID, 1, HI_FALSE);
        if (s32Ret)
        {
            print_info("read dev  failed\n");
            return -EAGAIN;
        }
            osal_printk("func:%s,dev info :0x%x\n",__func__, u8DevID);
    }
   
    return u8DevID;

}

HI_S32 HI_MotionSensor_DevInit(MSENSOR_PARAM_S* pstMSensorParam)
{
    HI_S32 s32Ret;
    //HI_S32 s32Err;
    //HI_U32 cpu = 2;
    HI_U8 u8DevID = 0;
    
    u8DevID = HI_MotionSensor_DevID_Read();
    if(ICM20690_SELFID != u8DevID)
    {
        print_info("ICM20690 Device Abnormal!!\n");
        return -ENODEV;
    }
    s32Ret = ICM20690Dev_init(pstMSensorParam);


    if (HI_SUCCESS != s32Ret)
    {
        print_info("ICM20690_init failed\n");
        return -ENODEV;
    }

#if 0

    if (ICM20690_dev->u8FifoEn)
    {
#ifndef __HuaweiLite__
        //ICM20690_dev->read_data_task = kthread_run(HI_MotionSensor_DataHandle, (void*)&pstMSensorParam->stMSensorAttr,
        //    "MotionSensor_read_data_task");
        ICM20690_dev->read_data_task = kthread_create(HI_MotionSensor_DataHandle, (void*)&pstMSensorParam->stMSensorAttr,
                                       "MotionSensor_read_data_task");
#else
        hithread_state = THREAD_CTRL;
        ICM20690_dev->read_data_task = osal_kthread_create(HI_MotionSensor_DataHandle, (void*)&pstMSensorParam->stMSensorAttr,
                                       "motionsensor_thread");
#endif

        if (IS_ERR(ICM20690_dev->read_data_task))
        {
            print_info("Unable to start kernel thread.\n");
            s32Err = PTR_ERR(ICM20690_dev->read_data_task);
            ICM20690_dev->read_data_task = NULL;
            return s32Err;
        }


        //kthread_bind(ICM20690_dev->read_data_task, cpu);
        //wake_up_process(ICM20690_dev->read_data_task);
    }

#endif
    s8TimerFristFlag = HI_FALSE;
	su8Timecnt = 0;
    su8datacount = 0;
#ifdef TEST_DEBUG
    print_debug = HI_FALSE;
    su32DataCount = 0;
    osal_memset(test_fifo, 0, sizeof(test_fifo));
#endif
    return HI_SUCCESS;
}
#ifdef TRANSFER_I2C
static struct i2c_board_info hi_icm20690_info =
{
    I2C_BOARD_INFO("ICM20690", ICM20690_DEV_ADDR),
};
#endif


//HI_S32 HI_MotionSensor_SensorInit(MOTIONSENSOR_STATUS_S MotionSensor_status)
HI_S32 HI_MotionSensor_SensorInit(HI_VOID)
{
    HI_S32 s32Ret;

    /*1.malloc a ICM20690 dev*/
    ICM20690_dev = osal_kmalloc(sizeof(ICM20690_DEV_INFO), osal_gfp_kernel);

    if (!ICM20690_dev)
    {
        print_info("Could not allocate memory\n");
        return -ENOMEM;
    }

    osal_memset(ICM20690_dev, 0, sizeof(ICM20690_DEV_INFO));
    //memset(ICM20690_dev, 0, sizeof(ICM20690_DEV_INFO));
#ifndef __HuaweiLite__
#ifdef TRANSFER_I2C
    s32Ret = MotionSersor_i2c_init(&ICM20690_dev->client, hi_icm20690_info, I2C_DEV_NUM);

    if (s32Ret)
    {
        print_info("i2cdev_init failed\n");
        goto ERR_KZALLOC;
    }

#elif defined TRANSFER_SPI
    s32Ret = MotionSersor_SPI_init(&ICM20690_dev->hi_spi);

    if (s32Ret)
    {
        print_info("spidev_init failed\n");
        goto ERR_KZALLOC;
    }

#endif
    //osal_sema_init(&ICM20690_dev->g_sem, 1);
    gpio_init();
#else
    print_info("Loirn add-> HuaweiLite Defined\n");
    s32Ret = MotionSersor_SPI_init();

    if (s32Ret)
    {
        print_info("spidev_init failed\n");
        goto ERR_KZALLOC;
    }

    gpio_init(&ICM20690_dev->gd);
#endif

    //my_wq = create_workqueue("my_wq");
    //osal_mutex_init(&mutexGetData);
    osal_spin_lock_init(&lockGetData);
    osal_wait_init(&ICM20690_dev->stWaitCallStopWorking);

    return HI_SUCCESS;

ERR_KZALLOC:
    osal_kfree(ICM20690_dev);
    ICM20690_dev = NULL;

    return HI_FAILURE;
}


void HI_MotionSensor_DevDeInit(MSENSOR_PARAM_S* MotionSensor_status)
{
    //osal_up(&ICM20690_dev->g_sem);
    //    if (ICM20690_dev->read_data_task)
    //    {
    //        //osal_up(&ICM20690_dev->g_sem);
    //        kthread_stop(ICM20690_dev->read_data_task);
    //        ICM20690_dev->read_data_task = NULL;
    //    }

    ICM20690Dev_exit(MotionSensor_status);
}


void HI_MotionSensor_SensorDeInit(MSENSOR_PARAM_S* MotionSensor_status)
{
    //osal_sema_destory(&ICM20690_dev->g_sem);
    osal_wait_destory(&ICM20690_dev->stWaitCallStopWorking);
#ifndef __HuaweiLite__
#ifdef TRANSFER_I2C
    MotionSersor_i2c_exit(&ICM20690_dev->client);
#elif defined TRANSFER_SPI
    MotionSersor_SPI_deinit(ICM20690_dev->hi_spi);
#endif
    gpio_deinit();
#else
    gpio_deinit(&ICM20690_dev->gd);
    MotionSersor_SPI_deinit();
#endif
    //osal_mutex_destory(&mutexGetData);
    osal_spin_lock_destory(&lockGetData);

    //	if(my_wq)
    //	{
    //		flush_workqueue( my_wq );
    //		destroy_workqueue(my_wq);
    //	}

    if (ICM20690_dev != NULL)
    {
        kfree(ICM20690_dev);
        ICM20690_dev = NULL;
    }
}

