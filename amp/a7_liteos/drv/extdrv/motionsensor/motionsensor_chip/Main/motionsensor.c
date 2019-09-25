#include "motionsensor.h"
#include "motionsensordev.h"
//#include "bmm050.h"
#ifdef ICM20690_PARAM_PROC
#include "motionsensor_proc.h"
#endif
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>

#include "sys_ext.h"

#include "motionsensor_ext.h"
#include "motionsensor_chip_cmd.h"
#include "hi_comm_motionsensor.h"

static osal_dev_t* s_pstMotionsensorDev = NULL;

#define MNGBUFF_ENABLE

static HI_S32           g_s32MotionSensorInit;
static HI_BOOL          g_bMotionSensorStart = HI_FALSE;
MSENSOR_PARAM_S*         MotionSensorStatus = HI_NULL;
//static HI_U8          g_u8FlagConfigInit;  	  //bit[0] mode init ? bit[1] gyro odr init ?

//bit[2] gyro FSR init? bit[3] accel odr init ?
//bit[4] accel FSR init
#ifdef MNGBUFF_ENABLE
HI_S32 MotionSensorDev_IntCallBack(MSENSOR_DATA_S* pstMSensorData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MSENSOR_MNG_EXPORT_FUNC_S*  pfnMotionsensorMngExpFunc = HI_NULL;


    if (HI_NULL == pstMSensorData)
    {
        print_info( "pstMotionsensorChipData is NULL!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    pfnMotionsensorMngExpFunc = FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR);

    if (HI_NULL == pfnMotionsensorMngExpFunc)
    {
        print_info( "pfnMotionsensorMngExpFunc is NULL!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    s32Ret = pfnMotionsensorMngExpFunc->pfnChipWriteDataToMngBuff(pstMSensorData);

    if (HI_SUCCESS != s32Ret)
    {
        print_info( "pfnChipWriteDataToMngBuff Failed!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    return s32Ret;
}
HI_S32 MotionSensorDev_InitMngBuff(MSENSOR_ATTR_S pstMSensorAttr, MSENSOR_BUF_ATTR_S* pstMotionsensorBufAttr, MSENSOR_CONFIG_S* pstMSensorConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MSENSOR_MNG_EXPORT_FUNC_S*  pfnMotionsensorMngExpFunc = HI_NULL;

    if ((HI_NULL == pstMotionsensorBufAttr) || (HI_NULL == pstMSensorConfig))
    {
        print_info( "Init Mng Buff Failed!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    pfnMotionsensorMngExpFunc = FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR);

    if ((HI_NULL == pfnMotionsensorMngExpFunc) || (HI_NULL == pfnMotionsensorMngExpFunc->pfnMngInit))
    {
        print_info( "Init Mng Buff Failed!(pfnMotionsensorMngExpFunc:0x%p)\n", pfnMotionsensorMngExpFunc);
        return s32Ret;

    }

    s32Ret = pfnMotionsensorMngExpFunc->pfnMngInit(&pstMSensorAttr, pstMotionsensorBufAttr, pstMSensorConfig);

    if (HI_SUCCESS != s32Ret)
    {
        print_info( "Init Mng Buff Failed!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    return s32Ret;
}


HI_S32 HI_MOTIONSENSOR_DeInitMngBuff(HI_VOID)
{

    HI_S32 s32Ret = HI_SUCCESS;
    MSENSOR_MNG_EXPORT_FUNC_S*  pfnMotionsensorMngExpFunc = HI_NULL;

    pfnMotionsensorMngExpFunc = FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR);

    if ((HI_NULL == pfnMotionsensorMngExpFunc) || (HI_NULL == pfnMotionsensorMngExpFunc->pfnMngDeInit))
    {
        print_info( "pfnChipWriteDataToMngBuff Failed!(pfnMotionsensorMngExpFunc:0x%p)\n", pfnMotionsensorMngExpFunc);
        return s32Ret;
    }

    s32Ret = pfnMotionsensorMngExpFunc->pfnMngDeInit();

    if (HI_SUCCESS != s32Ret)
    {
        print_info( "pfnChipWriteDataToMngBuff Failed!(s32Ret:0x%x)\n", s32Ret);
        return s32Ret;

    }

    return s32Ret;
}

HI_S32 MOTIONSENSOR_GetConfigFromChip(MSENSOR_PARAM_S* pstMSensorParam)
{
    if (!MotionSensorStatus)
    {
        print_info("MotionSensorStatus is NULL\n");
        return HI_FAILURE;
    }

    osal_memcpy(pstMSensorParam, MotionSensorStatus, sizeof(MSENSOR_PARAM_S));

    return HI_SUCCESS;
}

#endif

#if 1
//HI_S32 MOTIONSENSOR_GetData(MSENSOR_ATTR_S stMSensorAttr)
//{
//    HI_S32 s32Ret = HI_SUCCESS;

//    if ((stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_GYRO) || (stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC))
//    {
//        s32Ret = HI_MOTIONSENSOR_SaveData_ModeDofFifo(stMSensorAttr);
//    }
//    else
//    {
//        print_info("[error]Not support MODE\n");
//        s32Ret = HI_FAILURE;
//    }

//#ifdef MNGBUFF_ENABLE
//    //Debug Only
//    MotionSensorDev_IntCallBack(pstIMUdata);
//#endif

//    return s32Ret;
//}
HI_S32 HI_MOTIONSENSOR_GetData(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    static MSENSOR_DATA_S stIMUdataTemp;

    s32Ret = ICM20690_Data_TransferIMU(&stIMUdataTemp);

    if (HI_SUCCESS != s32Ret)
    {
        print_info("[error]Not support MODE\n");
    }

    //HI_TRACE_MSENSOR(HI_DBG_ERR,"~~~~~u32DeviceMask:%d u32GyroCount:%d~~~\n",stIMUdata.stMSensorAttr.u32DeviceMask,stIMUdata.stMsensorGyroBuffer.u32BuffDataNum);
    //#ifdef MNGBUFF_ENABLE    
    /////MotionSensorDev_IntCallBack(&stIMUdata);
    //#endif

    return s32Ret;

}
#endif

static HI_S32 MotionSensor_open(HI_VOID* private_data)
{
    //print_info("MotionSensor_open\n");

    return HI_SUCCESS;
}

static HI_S32 MotionSensor_release(HI_VOID* private_data)
{
    print_info("MotionSensor_close\n");

    return HI_SUCCESS;
}

static HI_SL MotionSensor_ioctl(HI_U32 u32cmd, HI_UL arg, void* private_data)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //   HI_S16 s16Temperature;
    MSENSOR_BUF_ATTR_S* pstMotionsensorBufAttr;
    static TRIGER_CONFIG_S stTrigerConfig = {0};

    switch (u32cmd)
    {
        case MSENSOR_CMD_START:
        {
            if (HI_FALSE == g_s32MotionSensorInit)
            {
                print_info("MotionSensor is not init!\n");
                return HI_FAILURE;
            }

            if (HI_TRUE == g_bMotionSensorStart)
            {
                print_info("MotionSensor is already start!\n");
                return HI_SUCCESS;
            }

            FIFO_DATA_RESET();

            if (stTrigerConfig.eTrigerMode == TRIGER_TIMER)
            {
                print_info("lorin add -> eTrigerMode = TRIGER_TIMER\n");
                HI_MotionSensor_TimerRun();
            }
            else if (stTrigerConfig.eTrigerMode == TRIGER_EXTERN_INTERRUPT)
            {
                print_info("lorin add -> eTrigerMode = TRIGER_EXTERN_INTERRUPT\n");
                HI_MotionSensor_INTERRUPTRun();
            }
            else
            {
                print_info("ERROR TrigerMode!!\n");
                return HI_FAILURE;
            }

            g_bMotionSensorStart = HI_TRUE;

            break;
        }

        case MSENSOR_CMD_STOP:
        {
            if (HI_FALSE == g_s32MotionSensorInit)
            {
                print_info("MotionSensor is not init!\n");
                return HI_FAILURE;
            }

            if (HI_FALSE == g_bMotionSensorStart)
            {
                print_info("MotionSensor is already stop!\n");
                return HI_SUCCESS;
            }

            if (stTrigerConfig.eTrigerMode == TRIGER_TIMER)
            {
                HI_MotionSensor_TimerStop();
            }
            else if (stTrigerConfig.eTrigerMode == TRIGER_EXTERN_INTERRUPT)
            {
                HI_MotionSensor_INTERRUPTStop();
                //return HI_FAILURE;
            }
            else
            {
                print_info("ERROR TrigerMode!!\n");
                return HI_FAILURE;
            }

            g_bMotionSensorStart = HI_FALSE;
            //print_info("****fun:%s line:%d\n",__func__,__LINE__);

            break;
        }

        case MSENSOR_CMD_INIT:
        {
            //sprint_info("start to init!!\n");
            if (HI_TRUE == g_s32MotionSensorInit)
            {
                print_info("MotionSensor is already inited!\n");
                return HI_FAILURE;
            }

            osal_memcpy(MotionSensorStatus, (void*)arg, sizeof(MSENSOR_PARAM_S));


            MotionSensorStatus->stMSensorConfig.stGyroConfig.s32TempMax = MOTIONSENSOR_MAX_TEMP;
            MotionSensorStatus->stMSensorConfig.stGyroConfig.s32TempMin = MOTIONSENSOR_MIN_TEMP;
            MotionSensorStatus->stMSensorConfig.stAccConfig.s32TempMax = MOTIONSENSOR_MAX_TEMP;
            MotionSensorStatus->stMSensorConfig.stAccConfig.s32TempMin = MOTIONSENSOR_MIN_TEMP;
            /*init senser*/
            s32Ret = HI_MotionSensor_DevInit(MotionSensorStatus);

            if (HI_SUCCESS != s32Ret)
            {
                print_info("HI_MotionSensor_ParamInit failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            s32Ret = MotionSensor_GetTrigerConfig(&stTrigerConfig);

            if (HI_SUCCESS != s32Ret)
            {
                print_info("IMU_GetTrigerConfig failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

#ifdef ICM20690_PARAM_PROC
            /*proc info init*/
            MPU_PROC_Init();
#endif
            ///print_info("MotionSensor init finshed\n");

            /*proc info init*/

            /*buff init*/
#ifdef MNGBUFF_ENABLE
            /*fix in this*/
            pstMotionsensorBufAttr = &MotionSensorStatus->stMSensorBufAttr;
            MotionSensorDev_InitMngBuff(MotionSensorStatus->stMSensorAttr, pstMotionsensorBufAttr, &MotionSensorStatus->stMSensorConfig);
#endif
            g_s32MotionSensorInit = HI_TRUE;
            break;
        }

        case MSENSOR_CMD_DEINIT:
        {
            if (HI_FALSE == g_s32MotionSensorInit)
            {
                print_info("MotionSensor has not inited!\n");
                return HI_FAILURE;
            }

#if 0
            pfnMngDeInitBufBuff();
#endif
#ifdef ICM20690_PARAM_PROC
            MPU_PROC_Exit();
#endif
            g_s32MotionSensorInit = HI_FALSE;
            HI_MotionSensor_DevDeInit(MotionSensorStatus);
#ifdef MNGBUFF_ENABLE

            s32Ret = HI_MOTIONSENSOR_DeInitMngBuff();

            if (HI_SUCCESS != s32Ret)
            {
                print_info("HI_MOTIONSENSOR_ParamInit failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

#endif
            break;
        }

        case MSENSOR_CMD_GET_PARAM:
        {
            osal_memcpy((MSENSOR_PARAM_S*)arg, MotionSensorStatus, sizeof(MSENSOR_PARAM_S));

            break;
        }

        default :
        {
            print_info("*******MotionSensor_ioctl***IOCTL_CMD is not found******* \n ");
            break;
        }
    }

    return (HI_SL)s32Ret;
}


#ifdef MNGBUFF_ENABLE
static HI_S32 HI_CHIP_RegisterDataMngCallback(void)
{
    MSENSOR_MNG_CALLBACK_S stCallback = {0};

    if (HI_FALSE  == CKFN_SYS_ENTRY())
    {
        printk("sys is not ready, please check it\n");
        return HI_FAILURE;
    }

    stCallback.pfnMSensorGetConfigFromChip = MOTIONSENSOR_GetConfigFromChip;
    stCallback.pfnMSensorWriteData2Buf     = HI_MOTIONSENSOR_GetData;

    if ((NULL != FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)) && (CKFN_MSENSOR_MNG_RegisterCallBack()))
    {
        CALL_MSENSOR_MNG_RegisterCallBack(&stCallback);
    }
    else
    {
        printk("register motionsensor callback failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 HI_CHIP_UnRegisterDataMngCallback(void)
{
    if ((NULL != FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)) && (CKFN_MSENSOR_MNG_UnRegisterCallBack()))
    {
        CALL_MSENSOR_MNG_UnRegisterCallBack();
    }

    return HI_SUCCESS;
}

#endif

static osal_fileops_t motionsensor_fops =
{
    .open           = MotionSensor_open,
    .release        = MotionSensor_release,
    .unlocked_ioctl = MotionSensor_ioctl,
};

HI_S32 MotionSensorInit(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MotionSensorStatus = (MSENSOR_PARAM_S*)osal_kmalloc(sizeof(MSENSOR_PARAM_S), osal_gfp_kernel);

    if (!MotionSensorStatus)
    {
        print_info("Could not allocate memory\n");
        return -ENOMEM;
    }

    osal_memset(MotionSensorStatus, 0, sizeof(MSENSOR_PARAM_S));

    g_bMotionSensorStart = HI_FALSE;

    s32Ret = HI_MotionSensor_SensorInit();

    if (HI_SUCCESS != s32Ret)
    {
        print_info("init failed! ret=%x\n", s32Ret);
        osal_kfree(MotionSensorStatus);
        //osal_kfree(MSensorStatus);
        return HI_FAILURE;
    }

#ifdef MNGBUFF_ENABLE
    s32Ret = HI_CHIP_RegisterDataMngCallback();

    if (HI_SUCCESS != s32Ret)
    {
        print_info("HI_CHIP_RegisterDataMngCallback failed! ret=%x\n", s32Ret);
        osal_kfree(MotionSensorStatus);
        //osal_kfree(MSensorStatus);
        return HI_FAILURE;
    }

#endif

    s_pstMotionsensorDev = osal_createdev(MSENSOR_DEV_NAME);

    if (NULL == s_pstMotionsensorDev)
    {
        osal_printk( "motionsensor: create device failed\n");
        return HI_FAILURE;
    }

    s_pstMotionsensorDev->fops  = &motionsensor_fops;
    s32Ret = osal_registerdevice(s_pstMotionsensorDev);

    if (s32Ret)
    {
        osal_destroydev(s_pstMotionsensorDev);
        s_pstMotionsensorDev = NULL;
        osal_printk("register motionsensor device failed!\n");
        return HI_FAILURE;
    }

    osal_printk("load motionsensor_chip.ko for Hi3559AV100...OK!\n");


    return HI_SUCCESS;
}

HI_VOID MotionSensorExit(HI_VOID)
{
#ifdef MNGBUFF_ENABLE
    HI_CHIP_UnRegisterDataMngCallback();
#endif

    if (MotionSensorStatus)



    {
        HI_MotionSensor_SensorDeInit(MotionSensorStatus);
    }
    else
    {
        print_info("nothing to exit \n");
    }


    if (MotionSensorStatus == NULL)
    {
        print_info("MotionSensorStatus == NULL\n");
    }
    else
    {
        kfree(MotionSensorStatus);
        MotionSensorStatus = NULL;
    }

    osal_deregisterdevice(s_pstMotionsensorDev);
    osal_destroydev(s_pstMotionsensorDev);
    s_pstMotionsensorDev = NULL;
    osal_printk("Unload motionsensor_chip.ko for Hi3559AV100...OK!\n");
    //misc_deregister(&HI_MotionSensor_dev);
}

module_init(MotionSensorInit);
module_exit(MotionSensorExit);

MODULE_AUTHOR("hisilion");
MODULE_DESCRIPTION("MotionSensor driver");
MODULE_LICENSE("GPL");

