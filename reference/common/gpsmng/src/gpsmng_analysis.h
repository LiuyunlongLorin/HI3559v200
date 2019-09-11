#ifndef __GPSMNG_ANALYSIS_H__
#define __GPSMNG_ANALYSIS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_type.h"
#include"hi_gpsmng.h"

#define HI_GPSMNG_MESSAGE_MAX_LEN (120)

/* GPS RAW DATA*/
typedef struct tagGPSMNG_RAW_DATA
{
    HI_CHAR ggaStr[HI_GPSMNG_MESSAGE_MAX_LEN];
    HI_CHAR gllStr[HI_GPSMNG_MESSAGE_MAX_LEN];
    HI_CHAR gsaStr[HI_GPSMNG_MESSAGE_MAX_LEN];
    HI_CHAR rmcStr[HI_GPSMNG_MESSAGE_MAX_LEN];
    HI_CHAR vtgStr[HI_GPSMNG_MESSAGE_MAX_LEN];
    HI_CHAR gsvStr[HI_GPSMNG_GSV_MAX_MSG_NUM][HI_GPSMNG_MESSAGE_MAX_LEN];
}GPSMNG_RAW_DATA;

HI_S32 GPSMNG_ParserRawData(GPSMNG_RAW_DATA* gpsRawData, HI_GPSMNG_MSG_PACKET* gpsMsgPack);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_H__*/
