/**
 * @file      hi_product_ui.h
 * @brief     ui public interface
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#ifndef __HI_PRODUCT_UI_H__
#define __HI_PRODUCT_UI_H__


#include "hi_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define PDT_UI_CHECK_RET_WITHOUT_RETURN(RetValue,ErrString)\
do{\
    if(HI_SUCCESS != RetValue)\
    {\
        MLOGE(RED"%s failed, s32Ret(%x)\n\n"NONE, ErrString, RetValue);\
    }\
  }while(0)

typedef HI_S32 (*PDT_UI_MSGRESULTPROC_FN_PTR)(HI_EVENT_S* pstEvent);

/**State machine message context, can not send a msg successfully until last msg is finished*/
typedef struct tagPDT_UI_MESSAGE_CONTEXT
{
    HI_PDT_WORKMODE_E s32Workmode; /*<record usb mode>*/
    HI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    HI_BOOL bMsgProcessed;  /**< the message sent has been processed or not*/
    PDT_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc;   /**< used to process the response*/
    pthread_mutex_t MsgMutex;  /**< the mutex protect sent msg and bool flag*/
}PDT_UI_MESSAGE_CONTEXT;


/** \addtogroup     UI */
/** @{ */  /** <!-- [UI] */

typedef struct tagPDT_UI_WIFI_STATUS_S
{
    HI_BOOL bInit;  /** WiFi init */
    HI_BOOL bStart; /** WiFi start */
    HI_BOOL bResume; /** stop wifi when system dormant, resume wifi state when wakeup system dormant */
} PDT_UI_WIFI_STATUS_S;



/**
 * @brief      init and start UI
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 */
HI_S32 HI_PDT_UI_Init(HI_VOID);

/**
 * @brief      set SD available or not
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 */
HI_S32 HI_PDT_UI_SetSDState(HI_BOOL bAvailable);

/**
 * @brief    stop and deinit UI
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 */
HI_S32 HI_PDT_UI_Deinit(HI_VOID);


/** @}*/  /** <!-- ==== UI End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
