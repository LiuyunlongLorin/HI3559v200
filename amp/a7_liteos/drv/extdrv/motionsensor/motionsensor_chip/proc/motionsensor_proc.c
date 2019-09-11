#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "motionsensor.h"
#include "motionsensor_proc.h"

#define  ICN20690_INFO "mpu_info"

#define  MPU_VERSION_INFO "MPU debug 0.0.0.1"
//MPU_PROC_INFO_S g_stMpuProcInfo;

static HI_S32 MPU_PROC_Open(struct inode *inode, struct file *filp);
static HI_S32 MPU_PROC_Show(struct seq_file *s, void *v);
static HI_S32 MPU_PROC_Release(struct inode *inode, struct file *filp);

static const struct file_operations mpu_proc_fops = {
    .owner   = THIS_MODULE,
    .open    = MPU_PROC_Open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    //.release = seq_release,
    .release = MPU_PROC_Release,
};

static HI_S32 MPU_PROC_Open(struct inode *inode, struct file *filp)
{
    HI_S32 s32Ret = 0;

    s32Ret = single_open(filp, MPU_PROC_Show, NULL);
	//seq_open
    return s32Ret;
}

static HI_S32 MPU_PROC_Release(struct inode *inode, struct file *filp)
{
	return single_release(inode, filp);
}



static HI_S32 MPU_PROC_Show(struct seq_file *s, void *v)
{

    HI_S32 s32Ret;

	if(MotionSensorStatus->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_GYRO)
	{
		seq_printf(s,"------gyro parameter------\n");
		seq_printf(s,"%24s\n","##ICM20690##");
		seq_printf(s,"%24s %24s %24s %24s %24s\n","SampleRate","Full-scale-range", "Datawidth", "Max-Chip-Temperature", "Min-Chip-Temperature");
		seq_printf(s,"%24d %24d %24d %24d %24d\n", MotionSensorStatus->stMSensorConfig.stGyroConfig.u64ODR,
			MotionSensorStatus->stMSensorConfig.stGyroConfig.u64FSR, MotionSensorStatus->stMSensorConfig.stGyroConfig.u8DataWidth,
			MotionSensorStatus->stMSensorConfig.stGyroConfig.s32TempMax, MotionSensorStatus->stMSensorConfig.stGyroConfig.s32TempMin);
	}
	if(MotionSensorStatus->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC)
	{
		seq_printf(s,"------accelerometer parameter------\n");
		seq_printf(s,"%24s\n","##ICM20690##");
		seq_printf(s,"%24s %24s %24s %24s %24s\n","SampleRate","Full-scale-range", "Datawidth", "Max-Chip-Temperature", "Min-Chip-Temperature");
		seq_printf(s,"%24d %24d %24d %24d %24d\n", MotionSensorStatus->stMSensorConfig.stAccConfig.u64ODR,
			MotionSensorStatus->stMSensorConfig.stAccConfig.u64FSR, MotionSensorStatus->stMSensorConfig.stAccConfig.u8DataWidth,
			MotionSensorStatus->stMSensorConfig.stAccConfig.s32TempMax, MotionSensorStatus->stMSensorConfig.stAccConfig.s32TempMin);
	}
    return 0;
}


HI_S32 MPU_PROC_Init(void)
{
    struct proc_dir_entry *pentry;
    //memset(&g_stMpuProcInfo, 0, sizeof(g_stMpuProcInfo));

    pentry = proc_create(ICN20690_INFO, 0444, NULL, &mpu_proc_fops);
    if(pentry == NULL)
    {
        return -ENOMEM;
    }

    return 0;
}

void  MPU_PROC_Exit(void)
{
     remove_proc_entry(ICN20690_INFO, 0);
}



