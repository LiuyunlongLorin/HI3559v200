/**
* @file    hi_appcomm_log.h
* @brief   product log header file.
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/13
* @version   1.0

*/
#ifndef __HI_APPCOMM_LOG_H__
#define __HI_APPCOMM_LOG_H__

#include <stdio.h>
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     APPCOMM */
/** @{ */  /** <!-- [APPCOMM] */
/** product module name, canbe redefined in module */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE ""

/** color log macro define */
#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

/** product log level define */
typedef enum hiLOG_LEVEL_E
{
    HI_LOG_LEVEL_FATAL = 0,  /**<action must be taken immediately */
    HI_LOG_LEVEL_ERROR,      /**<error conditions                 */
    HI_LOG_LEVEL_WARNING,    /**<warning conditions               */
    HI_LOG_LEVEL_INFO,       /**<informational                    */
    HI_LOG_LEVEL_DEBUG,      /**<debug-level                      */
    HI_LOG_LEVEL_BUTT
} HI_LOG_LEVEL_E;

#ifdef CFG_DEBUG_LOG_ON
#define HI_LOG_LEVEL  HI_LOG_LEVEL_DEBUG
#else
#define HI_LOG_LEVEL  HI_LOG_LEVEL_DEBUG
#endif

HI_VOID HI_LOG_Print(HI_LOG_LEVEL_E enLevel, const HI_CHAR* pszModule, const HI_CHAR* pszFunc, HI_U32 u32Line, HI_CHAR* pszFmt, ...) __attribute__((format(printf,5,6)));
HI_VOID HI_LOG_Config(HI_BOOL bLogOn,HI_BOOL bLogTime,HI_LOG_LEVEL_E enLevel);

/** general product log macro define */
#define MLOGF(fmt, args...)  HI_LOG_Print(HI_LOG_LEVEL_FATAL,  HI_MODULE, __FUNCTION__, __LINE__,fmt, ##args)
#define MLOGE(fmt, args...)  HI_LOG_Print(HI_LOG_LEVEL_ERROR,  HI_MODULE, __FUNCTION__, __LINE__,fmt, ##args)
#define MLOGW(fmt, args...)  HI_LOG_Print(HI_LOG_LEVEL_WARNING,HI_MODULE, __FUNCTION__, __LINE__,fmt, ##args)
#define MLOGI(fmt, args...)  HI_LOG_Print(HI_LOG_LEVEL_INFO,   HI_MODULE, __FUNCTION__, __LINE__,fmt, ##args)
#ifdef CFG_DEBUG_LOG_ON
#define MLOGD(fmt, args...)  HI_LOG_Print(HI_LOG_LEVEL_DEBUG,  HI_MODULE, __FUNCTION__, __LINE__,fmt, ##args)
#else
#define MLOGD(fmt, args...)
#endif

#define HI_LOG_PrintFuncWar(Func, ErrCode) MLOGW("Call %s return [0x%08X]\n",#Func,ErrCode)
#define HI_LOG_PrintFuncErr(Func, ErrCode) MLOGE("Call %s return [0x%08X]\n",#Func,ErrCode)
#define HI_LOG_PrintErrCode(ErrCode) MLOGE("Error Code: [0x%08X]\n",ErrCode)

#define HI_LOG_FuncEnter()     MLOGD(" =====>[Enter]\n")
#define HI_LOG_FuncExit()      MLOGD(" =====>[Exit]\n")

#define HI_LOG_PrintS32(val)   MLOGD("%s = %d\n",#val,val)
#define HI_LOG_PrintU32(val)   MLOGD("%s = %u\n",#val,val)
#define HI_LOG_PrintS64(val)   MLOGD("%s = %lld\n",#val,val)
#define HI_LOG_PrintU64(val)   MLOGD("%s = %llu\n",#val,val)
#define HI_LOG_PrintH32(val)   MLOGD("%s = 0x%08X\n",#val,val)
#define HI_LOG_PrintH64(val)   MLOGD("%s = 0x%016llX\n",#val,val)
#define HI_LOG_PrintStr(val)   MLOGD("%s = %s\n",#val,val)
#define HI_LOG_PrintVoid(val)  MLOGD("%s = %p\n",#val,val)
#define HI_LOG_PrintFloat(val) MLOGD("%s = %f\n",#val,val)
#define HI_LOG_PrintInfo(val)  MLOGD("<%s>\n",val)
#ifdef CFG_DEBUG_LOG_ON
#define HI_LOG_PrintBlock(pu8Datablock,u32Length) \
    do {\
        HI_U32 u32i=0;\
        HI_CHAR acOutStr[HI_APPCOMM_COMM_STR_LEN]={0};\
        HI_CHAR* pcBuffer = (HI_CHAR*)(pu8Datablock);\
        MLOGD(" %s\n", #pu8Datablock);\
        for(u32i=0;u32i<(u32Length);u32i++)\
        {\
            snprintf_s(&acOutStr[(u32i % 16)*3],4,4," %02X",pcBuffer[u32i]);\
            if((u32i+1)%16==0)\
            {\
                MLOGD("%s\n",acOutStr);\
                memset(acOutStr,0x00,sizeof(acOutStr));\
            }\
        }\
        if((u32i%16!=0)&&(u32i!=0))\
        {\
            MLOGD("%s\n",acOutStr);\
        }\
    }while(0)
#else
#define HI_LOG_PrintBlock(pu8Datablock,u32Length)
#endif

/** @}*/  /** <!-- ==== APPCOMM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __HI_APPCOMM_LOG_H__ */

