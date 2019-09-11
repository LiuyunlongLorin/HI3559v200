#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "motionsensor_exe.h"
#include "motionsensor_ext.h"
#include "motionsensor_proc.h"
#include "motionsensor_buf.h"
#include "motionsensor_mng_cmd.h"
#include "sys_ext.h"
#include "hi_osal.h"

static osal_dev_t* s_pstMotionsensorDev = NULL;
static MSENSOR_MNG_CALLBACK_S g_stMotionsensorMngCallback = {HI_NULL};

#define USER_SEND_DATA 1

#define MOTIONSENSOR_DEV_NAME "motionsensor_mng"
extern  MSENSOR_BUF_USER_MNG_S g_stUserMng;

HI_S32 MOTIONSENSOR_WriteDataToMngBuff(MSENSOR_DATA_S* pstMSensorData);

HI_S32 HI_MOTIONSENSOR_GetProcInfo(HI_VOID)
{
    osal_memcpy(g_stMotionsensorProcInfo.aszGyroName, "ICM20690", sizeof("ICM20690"));
    osal_memcpy(g_stMotionsensorProcInfo.aszAccelName, "ICM20690", sizeof("ICM20690"));

    return HI_SUCCESS;
}

static long motionsensor_exe_ioctl(unsigned int cmd, unsigned long arg, void* private_data)
{
    HI_S32 s32Ret = HI_SUCCESS;
    #ifdef USER_SEND_DATA
    MSENSOR_DATA_S *pstMSensorData;
    #endif

    switch (cmd)
    {
        case MSENSOR_CMD_RELEASE_BUF:
        {
            s32Ret= MOTIONSENSOR_BUF_ReleaseData((MSENSOR_DATA_INFO_S*)arg);

            if (HI_SUCCESS != s32Ret)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"HI_MOTIONSENSOR_BufRelease failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            break;
        }

        case MSENSOR_CMD_GET_DATA:
        {

            s32Ret = MOTIONSENSOR_BUF_GetData((MSENSOR_DATA_INFO_S*)arg);

            if (HI_SUCCESS != s32Ret)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"HI_MOTIONSENSOR_BufReadData failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            break;
        }

        #ifdef USER_SEND_DATA
        case MSENSOR_CMD_SEND_DATA:
        {
            pstMSensorData = (MSENSOR_DATA_S*)arg;

            s32Ret = MOTIONSENSOR_WriteDataToMngBuff(pstMSensorData);

            if (HI_SUCCESS != s32Ret)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"HI_MOTIONSENSOR_BufReadData failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            break;
        }
        #endif

        case MSENSOR_CMD_ADD_USER:
        {
            s32Ret = MOTIONSENSOR_BUF_AddUser((HI_S32 *)arg);

            if (HI_SUCCESS != s32Ret)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"MOTIONSENSOR_BUF_AddUser failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            break;
        }

        case MSENSOR_CMD_DELETE_USER:
        {
            s32Ret = MOTIONSENSOR_BUF_DeleteUser((HI_S32 *)arg);

            if (HI_SUCCESS != s32Ret)
            {
                HI_TRACE_MSENSOR(HI_DBG_ERR,"MOTIONSENSOR_BUF_DeleteUser failed! ret=%x\n", s32Ret);
                return HI_FAILURE;
            }

            break;
        }

        default :
        {
            HI_TRACE_MSENSOR(HI_DBG_ERR,"ioctl cmd 0x%x does not exist!\n", cmd);
            break;
        }
    }

    return s32Ret;
}

static HI_S32 motionsensor_open(HI_VOID* private_data)
{
    return HI_SUCCESS;
}

static HI_S32 motionsensor_release(HI_VOID* private_data)
{
    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_MNG_Init(HI_VOID* pArgs)
{
    return HI_SUCCESS;
}
static HI_VOID MOTIONSENSOR_MNG_Exit(HI_VOID)
{
    return ;
}

static HI_VOID MOTIONSENSOR_MNG_QueryState(MOD_STATE_E* pstState)
{
    *pstState = MOD_STATE_FREE;
    return ;
}


static HI_VOID MOTIONSENSOR_MNG_Notify(MOD_NOTICE_ID_E enNotice)
{
    return ;
}

static HI_U32 MOTIONSENSOR_MNG_GetVerMagic(HI_VOID)
{
    return VERSION_MAGIC;
}


HI_S32 MOTIONSENSOR_WriteDataToMngBuff(MSENSOR_DATA_S* pstMSensorData)
{
    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    //unsigned long mngflags;

    osal_spin_lock(&g_stUserMng.msensormng_lock);
    #if 1
    if (MSENSOR_DEVICE_GYRO == (pstMSensorData->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_GYRO ))
    {
        //HI_TRACE_MSENSOR(HI_DBG_ERR,"### u32DeviceMask:%d u32GyroCount:%d++\n",pstMSensorData->stMSensorAttr.u32DeviceMask,pstMSensorData->stMsensorGyroBuffer.u32BuffDataNum);
        ////osal_printk("### u32DeviceMask:%d u32GyroCount:%d++\n",pstMSensorData->stMSensorAttr.u32DeviceMask,pstMSensorData->stMsensorGyroBuffer.u32BuffDataNum);
        for (i = 0; i < pstMSensorData->stMsensorGyroBuffer.u32BuffDataNum; i++)
        {
            if(MSENSOR_TEMP_GYRO != (pstMSensorData->stMSensorAttr.u32TemperatureMask & MSENSOR_TEMP_GYRO))
            {
                pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32Temp= 0xffffffff;
            }

            //Debug Set
            #if 0
            pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32XData = 100;
            pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32YData = 200;
            pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32ZData = 300;
            pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32Temp  = 400;
            #endif

            #if 0
            if(i%300 == 0)
            {
                osal_printk("Gyro:u32X:%d u32Y:%d u32Z:%d u32Temprature:%d u64PTS:%lld  !\n",pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32XData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32YData,
                                       pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32ZData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32Temp, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].u64PTS);
            }
            #endif

            HI_TRACE_MSENSOR(HI_DBG_DEBUG,"Gyro:u32X:%8d u32Y:%8d u32Z:%8d u32Temprature:%8d u64PTS:%10lld!\n",pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32XData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32YData,
                                   pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32ZData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32Temp, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].u64PTS);

            s32Ret = MOTIONSENSOR_BUF_WriteData(MSENSOR_DATA_GYRO, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32XData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32YData,
                                       pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32ZData, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].s32Temp, pstMSensorData->stMsensorGyroBuffer.astGyroData[i].u64PTS);
        }

    }

    if (MSENSOR_DEVICE_ACC == (pstMSensorData->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_ACC))
    {
        ////osal_printk("+++ fun:%s line:%d u32AccelValid:%d u32AccelCount:%d++\n",__func__,__LINE__,pstMotionsensorChipData->u32AccelValid,pstMotionsensorChipData->u32AccelCount);

        for (i = 0; i < pstMSensorData->stMsensorAccBuffer.u32BuffDataNum; i++)
        {
            if(MSENSOR_TEMP_ACC != (pstMSensorData->stMSensorAttr.u32TemperatureMask & MSENSOR_TEMP_ACC))
            {
                pstMSensorData->stMsensorAccBuffer.astAccData[i].s32Temp = 0xffffffff;
            }

            //Debug Set
            #if 0
            pstMotionsensorChipData->stMotionsensorAccelData[i].u32X = 1000;
            pstMotionsensorChipData->stMotionsensorAccelData[i].u32Y = 2000;
            pstMotionsensorChipData->stMotionsensorAccelData[i].u32Z = 3000;
            pstMotionsensorChipData->stMotionsensorAccelData[i].u32Temprature = 4000;
            #endif

            #if 0
            if(intwrite_exe_cnt%300 == 1)
            {
                osal_printk("Accel:u32X:%d u32Y:%d u32Z:%d u32Temprature:%d u64PTS:%lld!\n",pstMSensorData->stMsensorAccBuffer.astAccData[i].s32XData, pstMSensorData->stMsensorAccBuffer.astAccData[i].s32YData,
                                       pstMSensorData->stMsensorAccBuffer.astAccData[i].s32ZData, pstMSensorData->stMsensorAccBuffer.astAccData[i].s32Temp, pstMSensorData->stMsensorAccBuffer.astAccData[i].u64PTS);

            }
            #endif

            s32Ret = MOTIONSENSOR_BUF_WriteData(MSENSOR_DATA_ACC, pstMSensorData->stMsensorAccBuffer.astAccData[i].s32XData, pstMSensorData->stMsensorAccBuffer.astAccData[i].s32YData,
                                       pstMSensorData->stMsensorAccBuffer.astAccData[i].s32ZData, pstMSensorData->stMsensorAccBuffer.astAccData[i].s32Temp, pstMSensorData->stMsensorAccBuffer.astAccData[i].u64PTS);
        }
    }

    if (MSENSOR_DEVICE_MAGN == (pstMSensorData->stMSensorAttr.u32DeviceMask & MSENSOR_DEVICE_MAGN))
    {
        for (i = 0; i < pstMSensorData->stMsensorMagnBuffer.u32BuffDataNum; i++)
        {
            if(MSENSOR_TEMP_MAGN != (pstMSensorData->stMSensorAttr.u32TemperatureMask & MSENSOR_TEMP_MAGN))
            {
                 pstMSensorData->stMsensorMagnBuffer.astMagnData[i].s32Temp = 0xffffffff;
            }

            s32Ret = MOTIONSENSOR_BUF_WriteData(MSENSOR_DATA_MAGN, pstMSensorData->stMsensorMagnBuffer.astMagnData[i].s32XData, pstMSensorData->stMsensorMagnBuffer.astMagnData[i].s32YData,
                                       pstMSensorData->stMsensorMagnBuffer.astMagnData[i].s32ZData, pstMSensorData->stMsensorMagnBuffer.astMagnData[i].s32Temp, pstMSensorData->stMsensorMagnBuffer.astMagnData[i].u64PTS);
        }
    }
    #endif
    osal_spin_unlock(&g_stUserMng.msensormng_lock);

    return s32Ret;
}


HI_S32 MOTIONSENSOR_MngInitBuf(MSENSOR_ATTR_S* pstMotionAttr, MSENSOR_BUF_ATTR_S* pstMSensorBufAttr, MSENSOR_CONFIG_S * pstMSensorConfig)
{
    HI_U32 u32GyroOdr = 0;
    HI_U32 u32AccOdr = 0;
    HI_U32 u32MagnOdr = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    switch (pstMotionAttr->u32DeviceMask)
    {
        case MSENSOR_DEVICE_GYRO|MSENSOR_DEVICE_ACC:
        {

            /*Only For American Present*/
            u32GyroOdr = pstMSensorConfig->stGyroConfig.u64ODR;
            u32AccOdr  = pstMSensorConfig->stAccConfig.u64ODR;
            u32MagnOdr = 0;

            HI_TRACE_MSENSOR(HI_DBG_DEBUG,"ODR:u32GyroOdr:%d u32AccOdr:%d u32MagnOdr:%d\n",u32GyroOdr,u32AccOdr,u32MagnOdr);

            s32Ret = MOTIONSENSOR_BUF_Init(pstMSensorBufAttr,  u32GyroOdr,  u32AccOdr,  u32MagnOdr);

            break;
        }

        case MSENSOR_DEVICE_GYRO:
        {

            /*Only For American Present*/
            u32GyroOdr = pstMSensorConfig->stGyroConfig.u64ODR;
            u32AccOdr  = 0;
            u32MagnOdr = 0;

           HI_TRACE_MSENSOR(HI_DBG_DEBUG,"u32GyroOdr:%d u32AccOdr:%d u32MagnOdr:%d\n",u32GyroOdr,u32AccOdr,u32MagnOdr);

            s32Ret = MOTIONSENSOR_BUF_Init(pstMSensorBufAttr,  u32GyroOdr,  u32AccOdr,  u32MagnOdr);
            break;
        }
        case MSENSOR_DEVICE_ALL:
        {
            /*Only For American Present*/
            u32GyroOdr = pstMSensorConfig->stGyroConfig.u64ODR;
            u32AccOdr  = pstMSensorConfig->stAccConfig.u64ODR;
            u32MagnOdr = pstMSensorConfig->stAccConfig.u64ODR;

            HI_TRACE_MSENSOR(HI_DBG_DEBUG,"u32GyroOdr:%d u32AccOdr:%d u32MagnOdr:%d\n",u32GyroOdr,u32AccOdr,u32MagnOdr);

            s32Ret = MOTIONSENSOR_BUF_Init(pstMSensorBufAttr,  u32GyroOdr,  u32AccOdr,  u32MagnOdr);

            break;
        }
        default:
        {
            HI_TRACE_MSENSOR(HI_DBG_ERR,"MOTIONSENSOR_MngInitBuf(u32GyroOdr:%d u32AccOdr:%d u32MagnOdr:%d) err!\n",u32GyroOdr,u32AccOdr,u32MagnOdr);
            break;
        }
    }

    return s32Ret;
}

HI_S32 MOTIONSENSOR_MngDeInitBuf(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    MOTIONSENSOR_BUF_Deinit();

    return s32Ret;
}

HI_S32 MOTIONSENSOR_BUF_GetMotionsensorConfig(MSENSOR_PARAM_S* pstMSensorParam)
{
    g_stMotionsensorMngCallback.pfnMSensorGetConfigFromChip(pstMSensorParam);

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_BUF_WriteData2Buf(HI_VOID)
{
    g_stMotionsensorMngCallback.pfnMSensorWriteData2Buf();

    return HI_SUCCESS;
}

HI_S32 MOTIONSENSOR_MNG_RegisterMotionsensorCallBack (MSENSOR_MNG_CALLBACK_S* pstCallback)
{
    CHECK_NULL_PTR(pstCallback);

    g_stMotionsensorMngCallback.pfnMSensorGetConfigFromChip = pstCallback->pfnMSensorGetConfigFromChip;
    g_stMotionsensorMngCallback.pfnMSensorWriteData2Buf     = pstCallback->pfnMSensorWriteData2Buf;
    return HI_SUCCESS;
}

HI_VOID MOTIONSENSOR_MNG_UnRegisterMotionsensorCallBack (HI_VOID)
{
    g_stMotionsensorMngCallback.pfnMSensorGetConfigFromChip = HI_NULL;

}

static MSENSOR_MNG_EXPORT_FUNC_S s_stExportFuncs =
{
    .pfnMSensorMngAddMSensorUser        = MOTIONSENSOR_BUF_AddUser,
    .pfnMSensorMngDeleteMSensorUser     = MOTIONSENSOR_BUF_DeleteUser,
    .pfnMSensorMngGetData               = MOTIONSENSOR_BUF_GetData,
    .pfnMSensorMngReleaseData           = MOTIONSENSOR_BUF_ReleaseData,
    .pfnMSensorMngGetMSensorConfig      = MOTIONSENSOR_BUF_GetMotionsensorConfig,

    //CallBack2Chip
    .pfnChipWriteDataToMngBuff          = MOTIONSENSOR_WriteDataToMngBuff,
    .pfnMngInit                         = MOTIONSENSOR_MngInitBuf,
    .pfnMngDeInit                       = MOTIONSENSOR_MngDeInitBuf,

    //CallBackToChip
    .pfnMSensorMngRegisterCallBack      = MOTIONSENSOR_MNG_RegisterMotionsensorCallBack,
    .pfnMSensorMngUnRegisterCallBack    = MOTIONSENSOR_MNG_UnRegisterMotionsensorCallBack,
};

static UMAP_MODULE_S s_stModule =
{
    .enModId           = HI_ID_MOTIONSENSOR,
    .aModName          = "motionsensor",

    .pfnInit           = MOTIONSENSOR_MNG_Init,
    .pfnExit           = MOTIONSENSOR_MNG_Exit,
    .pfnQueryState     = MOTIONSENSOR_MNG_QueryState,
    .pfnNotify         = MOTIONSENSOR_MNG_Notify,
    .pfnVerChecker     = MOTIONSENSOR_MNG_GetVerMagic,

    .pstExportFuncs    = &s_stExportFuncs,
    .pData = HI_NULL,
};



static osal_fileops_t motionsensor_fops =
{
    .open           = motionsensor_open,
    .release        = motionsensor_release,
    .unlocked_ioctl = motionsensor_exe_ioctl,
};


HI_S32 motionsensor_init(HI_VOID)
{
    HI_S32 s32Ret;

    if (CMPI_RegisterModule(&s_stModule))
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"RegisterModule FAILURE(Chip:%s)!\n", CHIP_NAME);
        return HI_FAILURE;
    }

    MOTIONSENSOR_PROC_Init();

    s_pstMotionsensorDev = osal_createdev(MOTIONSENSOR_DEV_NAME);

    if (NULL == s_pstMotionsensorDev)
    {
        HI_TRACE_MSENSOR(HI_DBG_ERR,"motionsensor: create device failed\n");
        return HI_FAILURE;
    }

    s_pstMotionsensorDev->fops  = &motionsensor_fops;
    s32Ret = osal_registerdevice(s_pstMotionsensorDev);

    if (s32Ret)
    {
        osal_destroydev(s_pstMotionsensorDev);
        s_pstMotionsensorDev = NULL;
        HI_TRACE_MSENSOR(HI_DBG_ERR,"register motionsensor device failed!\n");
        return HI_FAILURE;
    }

    s32Ret = MOTIONSENSOR_BUF_LockInit();
    if (s32Ret)
    {
        osal_deregisterdevice(s_pstMotionsensorDev);
        osal_destroydev(s_pstMotionsensorDev);
        s_pstMotionsensorDev = NULL;
        HI_TRACE_MSENSOR(HI_DBG_ERR,"register MOTIONSENSOR_BUF_LockInit failed!\n");
        return HI_FAILURE;
    }

    osal_printk("load motionsensor_mng.ko for Hi3559AV100...OK!\n");

    return HI_SUCCESS;
}

HI_VOID motionsensor_exit(HI_VOID)
{
    CMPI_UnRegisterModule(HI_ID_MOTIONSENSOR);
    MOTIONSENSOR_PROC_Exit();
    MOTIONSENSOR_BUF_LockDeInit();
    osal_deregisterdevice(s_pstMotionsensorDev);
    osal_destroydev(s_pstMotionsensorDev);
    s_pstMotionsensorDev = NULL;
    osal_printk("Unload motionsensor_mng.ko for Hi3559AV100...OK!\n");
}

module_init(motionsensor_init);
module_exit(motionsensor_exit);

MODULE_DESCRIPTION("motionsensor driver");
MODULE_LICENSE("GPL");

