#ifndef __MOTIONSENSORDEV_H__
#define __MOTIONSENSORDEV_H__
#include "motionsensor.h"
#include "hi_osal.h"
#include "linux/workqueue.h"
#include "linux/hrtimer.h"

//#include "pthread.h"

#ifdef __HuaweiLite__
#include "gpio.h"
#endif

#if 0
#define ICM20690_DEV_ADDR 0x68
#define I2C_DEV_NUM 0x9
#else
#define ICM20690_DEV_ADDR 0x68
#define I2C_DEV_NUM 0x1
#endif

#define ICM20690_VALID_DATA_BIT   16

#define DIV_0_TO_1(a)   ( (0 == (a)) ? 1 : (a) )




#define ACCEL_XOUT_H (0x3B)
#define ACCEL_XOUT_L (0x3C)
#define ACCEL_YOUT_H (0x3D)
#define ACCEL_YOUT_L (0x3E)
#define ACCEL_ZOUT_H (0x3F)
#define ACCEL_ZOUT_L (0x40)

#define GYRO_XOUT_H (0x43)
#define GYRO_XOUT_L (0x44)
#define GYRO_YOUT_H (0x45)
#define GYRO_YOUT_L (0x46)
#define GYRO_ZOUT_H (0x47)
#define GYRO_ZOUT_L (0x48)

#define ICM20690_SLEEP_MODE (0x00)
#define ICM20690_STANDBY_MODE (0x01)
#define ICM20690_ACCEL_LOWPOWER_MODE (0x02)
#define ICM20690_ACCEL_LOWNOISE_MODE (0x03)
#define ICM20690_GYRO_LOWPOWER_MODE (0x04)
#define ICM20690_GYRO_LOWNOISE_MODE (0x05)
#define ICM20690_6_AXIS_LOWPOWER_MODE (0x06)
#define ICM20690_6_AXIS_LOWNOISE_MODE (0x07)

#define ODR_1KHZ (0x0)
//#define ICM20690_SLEEP_MODE (0x0)

#define TRUE_REGISTER_VALUE   (0x01)
#define FALSE_REGISTER_VALUE   (0x00)

#define USER_CONTROL_REGISTER_ADDR         (0x6A)
#define POWER_MANAGEMENT_REGISTER_1_ADDR   (0x6B)
#define POWER_MANAGEMENT_REGISTER_2_ADDR   (0x6C)
#define CONFIGURATION_REGISTER_ADDR   	   (0x1A)
#define GYRO_CONFIG_REGISTER_ADDR   	   (0x1B)
#define ACCEL_CONFIG_REGISTER_1_ADDR   	   (0x1C)
#define ACCEL_CONFIG_REGISTER_2_ADDR   	   (0x1D)
#define LP_MODE_CONFIG_REGISTER_ADDR	   (0x1E)

#define DEV_TEMPERATURE_LSB_ADDR   (0x41)

#define SMPLRT_DIV (0x19)
#define FIFO_ENABLE_REGISTER_ADDR          (0x23)

#define RESET_OFFSET  		  (0x07)
#define CLKSET_OFFSET  		  (0x02)
#define CLKSET_VALUE  		  (0x01)

#define FIFO_COUNTH           (0x72)
#define FIFO_COUNTL           (0x73)
#define FIFO_R_W              (0x74)

#define INT_PIN_CONFIGURATION               (0x37)
#define INTERRUPT_ENABLE                    (0x38)
#define FIFO_WM_INT_STATUS                  (0x39)
#define INT_STATUS                          (0x3A)

#define ICM20690_ACCEL_UI_FULL_SCALE_SET_2G  0x00
#define ICM20690_ACCEL_UI_FULL_SCALE_SET_4G  0x01
#define ICM20690_ACCEL_UI_FULL_SCALE_SET_8G  0x02
#define ICM20690_ACCEL_UI_FULL_SCALE_SET_16G 0x03

#define ICM20690_GYRO_FULL_SCALE_SET_250DPS  0x00
#define ICM20690_GYRO_FULL_SCALE_SET_500DPS  0x01
#define ICM20690_GYRO_FULL_SCALE_SET_1000DPS 0x02
#define ICM20690_GYRO_FULL_SCALE_SET_2000DPS 0x03
#define ICM20690_GYRO_FULL_SCALE_SET_31DPS   0x05
#define ICM20690_GYRO_FULL_SCALE_SET_62DPS   0x06
#define ICM20690_GYRO_FULL_SCALE_SET_125DPS  0x07


#define ICM20690_SELFID    (0x20)
#define ICM20690_FIFO_MAX_RECORD    (140)
#define ICM20690_FIFO_R_MAX_SIZE    (512)
#define FIFO_FRAME_CNT 				(85)


#define ROOMTEMP_OFFSET             (25*(0x1<<10))
//#define SCALE_TIMES                 (1000)
#define GRADIENT_TEMP               (10*(0x1<<10))
#define TEMP_SENSITIVITY            (3268)

/*icm20690 interrupt pin reg*/
#define INT_GPIO_CHIP (7)
#define INT_GPIO_OFFSET (3)

#define TIME_RECORD_CNT   10
#define DATA_RECORD_CNT   (TIME_RECORD_CNT)





#if 0
char mode[8][MAX_LEN] = {"Sleep", "Standby", "Accel_Low-Power", "Accel_Low-Noise", "Gyro_Low-Power", "Gyro_Low-Noise", "6-Axis Low-Noise", "6-Axis Low-Power"};
char odr[5][MAX_LEN] = {"nocfg", "<=1", "04", "08", "32"};
char bw[10][MAX_LEN] = {"nocfg", "05", "10", "20", "41", "92", "184", "250", "3600", "8800"};
char fsr_gyro[8][MAX_LEN] = {"nocfg", "250", "500", "1000", "2000", "31.25", "62.5", "125"};
char fsr_accel[5][MAX_LEN] = {"nocfg", "02", "04", "08", "16"};
#endif


typedef struct hiGYRO_STATUS_S
{
    GYRO_CONFIG_S stGyroConfig;
    HI_U32        u32BandWidth;
    HI_U64        u64LastPts;
} GYRO_STATUS_S;

typedef struct hiACC_STATUS_S
{
    ACC_CONFIG_S stAccConfig;
    HI_U32       u32BandWidth;
    HI_U64       u64LastPts;
} ACC_STATUS_S;


typedef struct hiMAGN_STATUS_S
{
    MAGN_CONFIG_S stMagnConfig;
    HI_U32        u32BandWidth;
    HI_U64        u64LastPts;
} MAGN_STATUS_S;


//struct mpu_fifo_data_header_t {
//ACCEL_DRV_DATA_S accel_fifo[FIFO_FRAME_CNT];/**<
//Accel data of XYZ */
////struct mpu_mag_t mag_fifo[BMI160_FIFO_FRAME_CNT];
///**<Mag data of XYZ */
//GYRO_DRV_DATA_S gyro_fifo[FIFO_FRAME_CNT];/**<
//Gyro data of XYZ */
//unsigned int fifo_time;/**< Value of FIFO time*/
//unsigned char skip_frame;/**< The value of skip frame information */
//unsigned char fifo_input_config_info; /**< FIFO input config info*/
//unsigned char accel_frame_count; /**< The total number of Accel frame stored
//in the FIFO*/
//unsigned char gyro_frame_count; /**< The total number of Gyro  frame stored
//in the FIFO*/
////unsigned char mag_frame_count;
///**< The total number of Mag frame stored
//in the FIFO*/
//unsigned char fifo_header[FIFO_FRAME_CNT]; /**< FIFO header info*/
//};
typedef struct hiICM20690_DEV_INFO
{
    MSENSOR_SAMPLE_DATA_S  accel_cur_data;
    MSENSOR_SAMPLE_DATA_S  gyro_cur_data;
	MSENSOR_SAMPLE_DATA_S  stMagnData;
#ifndef __HuaweiLite__
    struct i2c_client*     client;
	struct spi_device*     hi_spi;
	struct task_struct*    read_data_task;
#else
	struct gpio_descriptor gd;
	osal_task_t*           read_data_task;
#endif
    struct task_struct*    get_data_kthread;
	struct osal_work_struct work;
	//struct mutex           mutex;
    HI_U8                  power_mode;
    HI_U8                  b_FLAG_ACC_FIFO_Enabled;
    HI_U8                  b_FLAG_GYRO_FIFO_Enabled;
    HI_U8                  b_FLAG_FIFOIncomming;
    HI_U8*                 FIFO_buf;
    HI_U32                 fifoLength;
    HI_U8                  recordNum;
    HI_U8                  u8EnableKthread;
	HI_U8                  u8FifoEn;
    TRIGER_CONFIG_S        stTrigerConfig;
    struct hrtimer         hrtimer;
    HI_U8                  thread_wakeup;
    osal_semaphore_t       g_sem;
    ACC_STATUS_S           stAccStatus;
    GYRO_STATUS_S          stGyroStatus;
    MAGN_STATUS_S          stMagnStatus;
    HI_S32                 s32temperature;
    HI_S32                 s32IRQNum;
    HI_S32                 s32WorkqueueCallTimes;
    osal_wait_t            stWaitCallStopWorking;
}ICM20690_DEV_INFO;



//extern struct ICM20690_dev_info *ICM20690_dev;

extern HI_S32    FIFO_DATA_UPDATE(void);
extern HI_S32    FIFO_DATA_RESET(void);
extern HI_S32    MotionSensor_GetTrigerConfig(TRIGER_CONFIG_S* pstTrigerConfig);
extern HI_S32    HI_MotionSensor_DevInit(MSENSOR_PARAM_S* pstMSensorParam);
extern HI_VOID   HI_MotionSensor_DevDeInit(MSENSOR_PARAM_S* MotionSensor_status);
extern HI_S32    HI_MotionSensor_SensorInit(HI_VOID);
extern HI_VOID   HI_MotionSensor_SensorDeInit(MSENSOR_PARAM_S* pstMSensorParam);
extern HI_S32    HI_MotionSensor_DataHandle(void* data);
extern HI_S32    HI_MotionSensor_TimerRun(void);
extern HI_S32    HI_MotionSensor_INTERRUPTRun(void);
extern HI_S32    HI_MotionSensor_TimerStop(void);
extern HI_S32    HI_MotionSensor_INTERRUPTStop(void);
//extern HI_S32    HI_MOTIONSENSOR_SaveData_ModeDofFifo(MSENSOR_ATTR_S stMSensorAttr, MSENSOR_DATA_S* pstIMUdata);
extern HI_S32    ICM20690_Data_TransferIMU(MSENSOR_DATA_S *pstMotiondata);



#endif

