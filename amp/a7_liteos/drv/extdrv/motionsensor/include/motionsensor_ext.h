/******************************************************************************

Copyright (C), 2018, Hisilicon Tech. Co., Ltd.

******************************************************************************
File Name     : motionsensor_ext.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/07/30
Description   :
History       :
1.Date        : 2018/07/30
Author        : 
Modification: Created file

******************************************************************************/
#ifndef __MOTIONSENSOR_EXT_H__
#define __MOTIONSENSOR_EXT_H__


#include "hi_comm_motionsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */



//#define MAX_USRDATA_NUM 50


typedef struct hiMSENSOR_MNG_CALLBACK_S
{
    HI_S32  (*pfnMSensorGetConfigFromChip) (MSENSOR_PARAM_S* pstMSensorParam);
    HI_S32  (*pfnMSensorWriteData2Buf) (HI_VOID);
} MSENSOR_MNG_CALLBACK_S;


typedef HI_S32      FN_MSENSOR_MNG_AddUser(HI_S32* ps32Id);
typedef HI_S32      FN_MSENSOR_MNG_DeleteUser(HI_S32* ps32Id);
typedef HI_S32      FN_MSENSOR_MNG_GetData(HI_VOID* pstMSensorData);
typedef HI_S32      FN_MSENSOR_MNG_ReleaseData(HI_VOID* pstMSensorData);
typedef HI_S32      FN_MSENSOR_MNG_GetMSensorConfig(MSENSOR_PARAM_S* pstMSensorParam);

typedef HI_S32      FN_MSENSOR_MNG_WriteDataToBuff(MSENSOR_DATA_S* pstMSensorChipData);
typedef HI_S32      FN_MSENSOR_MNG_Init(MSENSOR_ATTR_S *pstMSensorAttr, MSENSOR_BUF_ATTR_S* pstMSensorBufAttr, MSENSOR_CONFIG_S * pstMSensorConfig);

typedef HI_S32      FN_MSENSOR_MNG_DeInit(HI_VOID);

typedef HI_S32      FN_MSENSOR_MNG_RegisterCallBack(MSENSOR_MNG_CALLBACK_S* pstMSensorCallback);
typedef HI_VOID     FN_MSENSOR_MNG_UnRegisterCallBack(HI_VOID);



typedef struct hiMSENSOR_MNG_EXPORT_FUNC_S
{
    /*CallBack To Dis/Avs*/
    FN_MSENSOR_MNG_AddUser*                  pfnMSensorMngAddMSensorUser;  
    FN_MSENSOR_MNG_DeleteUser*               pfnMSensorMngDeleteMSensorUser;
    FN_MSENSOR_MNG_GetData*                  pfnMSensorMngGetData;     
    FN_MSENSOR_MNG_ReleaseData*          	 pfnMSensorMngReleaseData;
    FN_MSENSOR_MNG_GetMSensorConfig*         pfnMSensorMngGetMSensorConfig;

     /*CallBack To Motionsensor_Chip*/
    FN_MSENSOR_MNG_WriteDataToBuff*          pfnChipWriteDataToMngBuff;
    FN_MSENSOR_MNG_Init*                     pfnMngInit;
    FN_MSENSOR_MNG_DeInit*                   pfnMngDeInit;

    /*Register To Motionsensor_Chip*/
    FN_MSENSOR_MNG_RegisterCallBack*         pfnMSensorMngRegisterCallBack;
    FN_MSENSOR_MNG_UnRegisterCallBack*       pfnMSensorMngUnRegisterCallBack;

} MSENSOR_MNG_EXPORT_FUNC_S;

#define CKFN_MSENSOR_MNG_RegisterCallBack()\
    (NULL != FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)->pfnMSensorMngRegisterCallBack)
#define CALL_MSENSOR_MNG_RegisterCallBack(pstMSensorMngCallback)\
    FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)->pfnMSensorMngRegisterCallBack(pstMSensorMngCallback)

#define CKFN_MSENSOR_MNG_UnRegisterCallBack()\
    (NULL != FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)->pfnMSensorMngUnRegisterCallBack)
#define CALL_MSENSOR_MNG_UnRegisterCallBack(HI_VOID)\
    FUNC_ENTRY(MSENSOR_MNG_EXPORT_FUNC_S, HI_ID_MOTIONSENSOR)->pfnMSensorMngUnRegisterCallBack(HI_VOID)


#define HI_TRACE_MSENSOR(level, fmt, ...)\
    do{ \
        HI_TRACE(level, HI_ID_MOTIONSENSOR,"[Func]:%s [Line]:%d [Info]:"fmt,__FUNCTION__, __LINE__,##__VA_ARGS__);\
    }while(0)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif


