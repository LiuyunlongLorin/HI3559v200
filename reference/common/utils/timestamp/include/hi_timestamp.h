/**
* @file    hi_timestamp.h
* @brief   timestamp log header file.
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
* @version   1.0

*/
#ifndef __HI_TIMESTAMP_H__
#define __HI_TIMESTAMP_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** performance tiem stamp */
//#define CFG_PERFORMANCE_TIME_STAMP

/** \addtogroup     APPCOMM */
/** @{ */  /** <!-- [APPCOMM] */

/** timestamp log funtions */
HI_VOID HI_TimeStamp(const HI_CHAR* pszFunc, HI_U32 u32Line, HI_S32 s32Type);
HI_VOID HI_TimeStampClean(HI_VOID);
HI_VOID HI_TimeStampPrint(HI_VOID);


/** timestamp log macro define */
#ifdef CFG_TIME_STAMP_ON
#define HI_TIME_STAMP HI_TimeStamp(__func__, __LINE__, 0);
#define HI_TIME_CLEAN HI_TimeStampClean();
#define HI_TIME_PRINT HI_TimeStampPrint();
#else
#define HI_TIME_STAMP
#define HI_TIME_CLEAN
#define HI_TIME_PRINT
#endif

#ifdef CFG_PERFORMANCE_TIME_STAMP
#define HI_PERFORMANCE_TIME_STAMP HI_TIME_STAMP
#else
#define HI_PERFORMANCE_TIME_STAMP
#endif

HI_VOID HI_PrintBootTime(const HI_CHAR* pcKeyword);


/** @}*/  /** <!-- ==== APPCOMM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __HI_TIMESTAMP_H__ */

