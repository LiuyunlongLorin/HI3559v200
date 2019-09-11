/**
* @file    hi_hal_touchpad.h
* @brief   product hal touchpad struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   v1.0

*/
#ifndef __HI_HAL_TOUCHPAD_H__
#define __HI_HAL_TOUCHPAD_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_TOUCHPAD */
/** @{ */  /** <!-- [HAL_TOUCHPAD] */

/* @brief touchpad rotate enum*/
typedef enum hiHAL_TOUCHPAD_ROTATE_E
{
    HI_HAL_TOUCHPAD_ROTATE_NONE = 0,/**<normal,no rotate*/
    HI_HAL_TOUCHPAD_ROTATE_90,/**<90 degree rotate*/
    HI_HAL_TOUCHPAD_ROTATE_180,
    HI_HAL_TOUCHPAD_ROTATE_270,
    HI_HAL_TOUCHPAD_ROTATE_BUTT
} HI_HAL_TOUCHPAD_ROTATE_E;

/** touchpad touch info*/
typedef struct hiHAL_TOUCHPAD_INPUTINFO_S
{
    HI_S32 s32ID;/**<input id info, one finger or two fingers*/
    HI_S32 s32X;/**<x coordinate absolute*/
    HI_S32 s32Y;/**<y coordinate absolute*/
    HI_U32 u32Pressure;/**<is press on screen: 0, 1*/
    HI_U32 u32TimeStamp;/**<time stamp*/
} HI_HAL_TOUCHPAD_INPUTINFO_S;

typedef struct hiHAL_TOUCHPAD_OBJ_S
{
    HI_S32 (*pfnInit)(HI_VOID);
    HI_S32 (*pfnStart)(HI_HANDLE* ps32Fd);
    HI_S32 (*pfnStop)(HI_VOID);
    HI_S32 (*pfnReadInputEvent)(HI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData);
    HI_S32 (*pfnSetRotate)(HI_HAL_TOUCHPAD_ROTATE_E enRotate);
    HI_S32 (*pfnSuspend)(HI_VOID);
    HI_S32 (*pfnResume)(HI_VOID);
    HI_S32 (*pfnDeinit)(HI_VOID);
} HI_HAL_TOUCHPAD_OBJ_S;

/**
* @brief          hal touchpad initialization, insmod driver
* @return        0 success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Init(HI_VOID);

/**
* @brief          set device rotate degree
* @param[in]   enRotate: degree nuum
* @return        0 success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/HI_S32 HI_HAL_TOUCHPAD_SetRotate(HI_HAL_TOUCHPAD_ROTATE_E enRotate);

/**
* @brief          pause touchpad device
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Suspend(HI_VOID);

/**
* @brief          resuume touchpad device
* @return        0 success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Resume(HI_VOID);

/**
* @brief          start touch info get
* @param[out] ps32Fd: FD
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Start(HI_S32* ps32Fd);

/**
* @brief          stop touch info get
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Stop(HI_VOID);

/**
* @brief          read touch info
* @param[out] pstInputData: store touch info
* @return        0 success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_ReadInputEvent(HI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData);

/**
* @brief          hal touchpad deinitialization, rmmod driver
* @return        0 success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_TOUCHPAD_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== HAL_TOUCHPAD End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_TOUCHPAD_H__*/
