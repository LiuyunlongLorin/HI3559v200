/**
* @file    hi_hal_screen.h
* @brief   product hal screen struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/
#ifndef __HI_HAL_SCREEN_H__
#define __HI_HAL_SCREEN_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_SCREEN */
/** @{ */  /** <!-- [HAL_SCREEN] */
#define HI_HAL_SCREEN_LANE_MAX_NUM 4

/** Screen Index, Currently can support two screen used in same time*/
typedef enum hiHAL_SCREEN_IDX_E
{
    HI_HAL_SCREEN_IDX_0 = 0,
    HI_HAL_SCREEN_IDX_1,
    HI_HAL_SCREEN_IDX_BUTT
} HI_HAL_SCREEN_IDX_E;

typedef enum hiHAL_SCREEN_TYPE_E
{
    HI_HAL_SCREEN_INTF_TYPE_LCD = 0, /**<RGB interface type, such as lcd_6bit lcd_8bit, parallel communication protocal*/
    HI_HAL_SCREEN_INTF_TYPE_MIPI, /**<MIPI interface type, serial communication protocal*/
    HI_HAL_SCREEN_INTF_TYPE_BUIT
} HI_HAL_SCREEN_TYPE_E;

/** brief general interface screen type enum*/
typedef enum hiHAL_SCREEN_LCD_INTF_TYPE_E
{
    HI_HAL_SCREEN_LCD_INTF_6BIT = 0, /**<6bit intf type*/
    HI_HAL_SCREEN_LCD_INTF_8BIT,
    HI_HAL_SCREEN_LCD_INTF_16BIT,
    HI_HAL_SCREEN_LCD_INTF_24BIT,
    HI_HAL_SCREEN_LCD_INTF_BUIT
} HI_HAL_SCREEN_LCD_INTF_TYPE_E;

/* @brief mipi screen packet transport type*/
typedef enum hiHAL_SCREEN_MIPI_OUTPUT_TYPE_E
{
    HI_HAL_SCREEN_MIPI_OUTPUT_TYPE_CMD = 0x0,
    HI_HAL_SCREEN_MIPI_OUTPUT_TYPE_VIDEO,
    HI_HAL_SCREEN_MIPI_OUTPUT_TYPE_BUIT
} HI_HAL_SCREEN_MIPI_OUTPUT_TYPE_E;

/* @brief mipi screen video mode type enum*/
typedef enum hiHAL_SCREEN_MIPI_VIDEO_MODE_E
{
    HI_HAL_SCREEN_MIPI_VIDEO_MODE_BURST = 0x0,/**<Burst Mode*/
    HI_HAL_SCREEN_MIPI_VIDEO_MODE_PULSES,/**<Non-Burst Mode with Sync Pulses*/
    HI_HAL_SCREEN_MIPI_VIDEO_MODE_EVENTS,/**<Non-Burst Mode with Sync Events*/
    HI_HAL_SCREEN_MIPI_VIDEO_MODE_BUIT
} HI_HAL_SCREEN_MIPI_VIDEO_MODE_E;

/* @brief mipi screen video format type enum*/
typedef enum hiHAL_SCREEN_MIPI_VIDEO_FORMAT_E
{
    HI_HAL_SCREEN_MIPI_VIDEO_RGB_16BIT = 0x0,
    HI_HAL_SCREEN_MIPI_VIDEO_RGB_18BIT,
    HI_HAL_SCREEN_MIPI_VIDEO_RGB_24BIT,
    HI_HAL_SCREEN_MIPI_VIDEO_BUIT
} HI_HAL_SCREEN_MIPI_VIDEO_FORMAT_E;

/** screen sync info*/
typedef struct hiHAL_SCREEN_SYNC_ATTR_S
{
    HI_U16   u16Vact ;  /* vertical active area */
    HI_U16   u16Vbb;    /* vertical back blank porch */
    HI_U16   u16Vfb;    /* vertical front blank porch */
    HI_U16   u16Hact;   /* herizontal active area */
    HI_U16   u16Hbb;    /* herizontal back blank porch */
    HI_U16   u16Hfb;    /* herizontal front blank porch */

    HI_U16   u16Hpw;    /* horizontal pulse width */
    HI_U16   u16Vpw;    /* vertical pulse width */

    HI_BOOL bIdv;/**< data Level polarity,0 mean high level valid,default 0,can not config*/
    HI_BOOL bIhs;/**< horizon Level polarity,0 mean high level valid*/
    HI_BOOL bIvs;/**< vertical Level polarity,0 mean high level valid*/
} HI_HAL_SCREEN_SYNC_ATTR_S;

/** vo clk type*/
typedef enum hiHAL_SCREEN_CLK_TYPE_E
{
    HI_HAL_SCREEN_CLK_TYPE_PLL = 0x0,
    HI_HAL_SCREEN_CLK_TYPE_LCDMCLK,
    HI_HAL_SCREEN_CLK_TYPE_BUTT
} HI_HAL_SCREEN_CLK_TYPE_E;

typedef struct hiHAL_SCREEN_CLK_PLL_S
{
    HI_U32  u32Fbdiv;
    HI_U32  u32Frac;
    HI_U32  u32Refdiv;
    HI_U32  u32Postdiv1;
    HI_U32  u32Postdiv2;
}HI_HAL_SCREEN_CLK_PLL_S;

typedef struct hiHAL_SCREEN_CLK_ATTR_S
{
    HI_BOOL bClkReverse; /**< vo clock reverse or not, if screen datasheet not mentioned, the value is true */
    HI_U32 u32DevDiv;    /**< vo clock division factor, RGB6&8 is 3,RGB16&18 is 1, MIPI DSI is 1 */
    HI_HAL_SCREEN_CLK_TYPE_E enClkType;  /**< vo clk type, pll or lcdmlck*/
    union
    {
        HI_HAL_SCREEN_CLK_PLL_S stClkPll;
        HI_U32 u32OutClk;    /**< for serial:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps*total_clk_per_pixel,
                                    for parallel:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps */
    };
} HI_HAL_SCREEN_CLK_ATTR_S;

/** screen common attr*/
typedef struct hiHAL_SCREEN_COMMON_ATTR_S
{
    HI_HAL_SCREEN_SYNC_ATTR_S stSynAttr;/**<screen sync attr*/
    HI_HAL_SCREEN_CLK_ATTR_S stClkAttr; /**<vo clk attr*/
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32Framerate;
} HI_HAL_SCREEN_COMMON_ATTR_S;

/** screen mipi attr*/
typedef struct hiHAL_SCREEN_MIPI_ATTR_S
{
    HI_HAL_SCREEN_MIPI_OUTPUT_TYPE_E enType;
    HI_HAL_SCREEN_MIPI_VIDEO_MODE_E enMode;
    HI_HAL_SCREEN_MIPI_VIDEO_FORMAT_E enVideoFormat;
    HI_S8 as8LaneId[HI_HAL_SCREEN_LANE_MAX_NUM];/**<lane use: value is index from zero start,lane not use:value is -1 */
    HI_U32 u32PhyDataRate;  /**<mbps* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*total_bit_per_pixel/lane_num/100000 */
    HI_U32 u32PixelClk;  /**<KHz* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps/1000/*/

} HI_HAL_SCREEN_MIPI_ATTR_S;

/** screen lcd attr*/
typedef struct hiHAL_SCREEN_LCD_ATTR_S
{
    HI_HAL_SCREEN_LCD_INTF_TYPE_E enType;
} HI_HAL_SCREEN_LCD_ATTR_S;

/** screen attr*/
typedef struct hiHAL_SCREEN_ATTR_S
{
    HI_HAL_SCREEN_TYPE_E enType;
    union tagHAL_SCREEN_ATTR_U
    {
        HI_HAL_SCREEN_LCD_ATTR_S stLcdAttr;
        HI_HAL_SCREEN_MIPI_ATTR_S stMipiAttr;
    }unScreenAttr;
    HI_HAL_SCREEN_COMMON_ATTR_S stAttr;
} HI_HAL_SCREEN_ATTR_S;

/* @brief screen status enum*/
typedef enum hiHAL_SCREEN_STATE_E
{
    HI_HAL_SCREEN_STATE_OFF = 0,/**<screen off*/
    HI_HAL_SCREEN_STATE_ON,     /**<screen on*/
    HI_HAL_SCREEN_STATE_BUIT
} HI_HAL_SCREEN_STATE_E;

typedef struct hiHAL_SCREEN_OBJ_S
{
    HI_S32 (*pfnInit)(HI_VOID);
    HI_S32 (*pfnGetAttr)(HI_HAL_SCREEN_ATTR_S* pstAttr);
    HI_S32 (*pfnGetDisplayState)(HI_HAL_SCREEN_STATE_E* penDisplayState);
    HI_S32 (*pfnSetDisplayState)(HI_HAL_SCREEN_STATE_E enDisplayState);
    HI_S32 (*pfnGetBackLightState)(HI_HAL_SCREEN_STATE_E* penBackLightState);
    HI_S32 (*pfnSetBackLightState)(HI_HAL_SCREEN_STATE_E enBackLightState);
    HI_S32 (*pfnGetLuma)(HI_U32* pu32Luma);
    HI_S32 (*pfnSetLuma)(HI_U32 u32Luma);
    HI_S32 (*pfnGetSaturature)(HI_U32* pu32Satuature);
    HI_S32 (*pfnSetSaturature)(HI_U32 u32Satuature);
    HI_S32 (*pfnGetContrast)(HI_U32* pu32Contrast);
    HI_S32 (*pfnSetContrast)(HI_U32 u32Contrast);
    HI_S32 (*pfnDeinit)(HI_VOID);
} HI_HAL_SCREEN_OBJ_S;

/**
* @brief       hal screen hook, selected by menuconfig,carried out by init module
* @param[in]   enScreenIndex: Screen Index
* @param[in]   pstScreenObj: Operations
* @return 0    success,non-zero error code.
* @exception   None
* @author      HiMobileCam Reference Develop Team
* @date        2018/03/16
*/
HI_S32 HI_HAL_SCREEN_Register(HI_HAL_SCREEN_IDX_E enScreenIndex, const HI_HAL_SCREEN_OBJ_S* pstScreenObj);

/**
* @brief          hal screen initialization, insmod driver
* @param[in]   enScreenIndex: Screen Index
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_Init(HI_HAL_SCREEN_IDX_E enScreenIndex);

/**
* @brief          get screen attr,inlcude resolution/syn info/Rotate
* @param[in]   enScreenIndex: Screen Index
* @param[out] pstAttr: screen fixed attr
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetAttr(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_ATTR_S* pstAttr);

/**
* @brief          get screen display State
* @param[in]   enScreenIndex: Screen Index
* @param[out] penDisplayState: screen display State
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetDisplayState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penDisplayState);

/**
* @brief          set screen display State
* @param[in]   enScreenIndex: Screen Index
* @param[in]   enDisplayState: screen display State
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E enDisplayState);

/**
* @brief          get screen backlight state
* @param[in]   enScreenIndex: Screen Index
* @param[out] penDisplayState: screen backlight state
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetBackLightState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E* penBackLightState);

/**
* @brief          set screen backlight state
* @param[in]   enScreenIndex: Screen Index
* @param[in]   enBackLightState: screen backlight state
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_HAL_SCREEN_STATE_E enBackLightState);

/**
* @brief          get screen luma
* @param[in]   enScreenIndex: Screen Index
* @param[out] pu32Luma: screen luma,value range is [0,100]
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetLuma(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Luma);

/**
* @brief          set screen screen luma
* @param[in]   enScreenIndex: Screen Index
* @param[in]   u32Luma: screen luma,value range is [0,100]
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_SetLuma(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Luma);

/**
* @brief          get screen satuature
* @param[in]   enScreenIndex: Screen Index
* @param[out] pu32Satuature: screen satuature,value range is [0,100]
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetSaturature(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Satuature);

/**
* @brief          set screen screen satuature
* @param[in]   enScreenIndex: Screen Index
* @param[in]   u32Satuature: screen satuature
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_SetSaturature(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Saturature);

/**
* @brief          get screen contrast
* @param[in]   enScreenIndex: Screen Index
* @param[out] pu32Contrast: screen contrast,value range is [0,100]
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_GetContrast(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32* pu32Contrast);

/**
* @brief          set screen screen contrast
* @param[in]   enScreenIndex: Screen Index
* @param[in]   u32Contrast: screen contrast,value range is [0,100]
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_SetContrast(HI_HAL_SCREEN_IDX_E enScreenIndex, HI_U32 u32Contrast);

/**
* @brief          HAL screen deinitialization, rmmod driver
* @param[in]   enScreenIndex: Screen Index
* @return 0     success,non-zero error code.
* @exception   None
* @author       HiMobileCam Reference Develop Team
* @date          2017/12/12
*/
HI_S32 HI_HAL_SCREEN_Deinit(HI_HAL_SCREEN_IDX_E enScreenIndex);

/** @}*/  /** <!-- ==== HAL_SCREEN End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_SCREEN_H__*/

