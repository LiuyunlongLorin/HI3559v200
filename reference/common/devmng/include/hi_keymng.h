/**
* @file    hi_keymng.h
* @brief   product keymng struct and interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
* @version   1.0

*/

#ifndef _HI_KEYMNG_H
#define _HI_KEYMNG_H

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     KEYMNG */
/** @{ */  /** <!-- [KEYMNG] */

/** macro define */
#define HI_KEYMNG_KEY_NUM_EACH_GRP (2)                        /**<key number in group-key*/

#define HI_KEYMNG_EINVAL            HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_EINVAL)    /**<parm invlid*/
#define HI_KEYMNG_EINTER            HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_EINTER)          /**<intern error*/
#define HI_KEYMNG_ENOINIT           HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_ENOINIT)  /**< no initialize*/
#define HI_KEYMNG_EINITIALIZED      HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_EINITIALIZED) /**< already initialized */
#define HI_KEYMNG_EREGISTEREVENT    HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define HI_KEYMNG_ETHREAD           HI_APPCOMM_ERR_ID(HI_APP_MOD_KEYMNG,HI_ERRNO_CUSTOM_BOTTOM+1)           /**<thread creat or join error*/

typedef enum hiEVENT_KEYMNG_E
{
    HI_EVENT_KEYMNG_SHORT_CLICK = HI_APPCOMM_EVENT_ID(HI_APP_MOD_KEYMNG, 0),   /**<short key click event*/
    HI_EVENT_KEYMNG_LONG_CLICK,    /**<long key click event*/
    HI_EVENT_KEYMNG_HOLD_DOWN,     /**<key hold up event*/
    HI_EVENT_KEYMNG_HOLD_UP,       /**<key hold down event*/
    HI_EVENT_KEYMNG_GROUP,     /**<group key event*/
    HI_EVENT_KEYMNG_BUIT
} HI_EVENT_KEYMNG_E;


/** key index enum */
typedef enum hiKEYMNG_KEY_IDX_E
{
    HI_KEYMNG_KEY_IDX_0 = 0,
    HI_KEYMNG_KEY_IDX_1,
    HI_KEYMNG_KEY_IDX_2,
    HI_KEYMNG_KEY_IDX_3,
    HI_KEYMNG_KEY_IDX_BUTT,
} HI_KEYMNG_KEY_IDX_E;

/** key type enum */
typedef enum hiKEYMNG_KEY_TYPE_E
{
    HI_KEYMNG_KEY_TYPE_CLICK = 0, /**<support click and longclick event */
    HI_KEYMNG_KEY_TYPE_HOLD,      /**<support keydown and keyup event */
} HI_KEYMNG_KEY_TYPE_E;

/** click key attribute */
typedef struct hiKEYMNG_KEY_CLICK_ATTR_S
{
    HI_BOOL bLongClickEnable; /**<ture: support click and longclick event; false: only support click event */
    HI_U32  u32LongClickTime_msec; /**<long click check time, valid when longclick enabled */
} HI_KEYMNG_KEY_CLICK_ATTR_S;

/** hold key attribute */
typedef struct hiKEYMNG_KEY_HOLD_ATTR_S
{
} HI_KEYMNG_KEY_HOLD_ATTR_S;

/** key attribute */
typedef struct hiKEYMNG_KEY_ATTR_S
{
    HI_KEYMNG_KEY_TYPE_E enType;   /**<click type or hold type*/
    HI_S32 s32Id; /**<key id */
    union tagKEYMNG_KEY_ATTR_U
    {
        HI_KEYMNG_KEY_CLICK_ATTR_S stClickKeyAttr;   /**<click attr type */
        HI_KEYMNG_KEY_HOLD_ATTR_S stHoldKeyAttr;     /**<hold attr type */
    } unAttr;
} HI_KEYMNG_KEY_ATTR_S;

/** key configure */
typedef struct hiKEYMNG_KEY_CFG_S
{
    HI_U32 u32KeyCnt;     /**<key count*/
    HI_KEYMNG_KEY_ATTR_S astKeyAttr[HI_KEYMNG_KEY_IDX_BUTT];
} HI_KEYMNG_KEY_CFG_S;


/** group-key configure */
typedef struct hiKEYMNG_GRP_KEY_CFG_S
{
    HI_BOOL bEnable;    /**<ture: support group key event; false: not support */
    HI_U32 au32GrpKeyIdx[HI_KEYMNG_KEY_NUM_EACH_GRP]; /**<only support two keys group at present*/
} HI_KEYMNG_GRP_KEY_CFG_S;

/** keymng configure */
typedef struct hiKEYMNG_CFG_S
{
    HI_KEYMNG_KEY_CFG_S stKeyCfg;
    HI_KEYMNG_GRP_KEY_CFG_S stGrpKeyCfg;
} HI_KEYMNG_CFG_S;

/**
* @brief    register keymng event
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/29
*/
HI_S32 HI_KEYMNG_RegisterEvent(HI_VOID);

/**
* @brief    keymng initialization, create key event check task
* @param[in] pstCfg: keymng configure, including key/grpkey configure
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_KEYMNG_Init(const HI_KEYMNG_CFG_S* pstCfg);



/**
* @brief    keymng deinitialization, destroy key event check task
* @return 0 success,non-zero error code.
* @exception    None
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/12
*/
HI_S32 HI_KEYMNG_Deinit(HI_VOID);

/** @}*/  /** <!-- ==== KEYMNG End ====*/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _HI_KEYMNG_H */

