/**
* @file    hi_gpsmng.h
* @brief   product gps interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#ifndef __HI_GPSMNG_H__
#define __HI_GPSMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_appcomm.h"

/** error code define */
#define HI_GPSMNG_EINVAL                   HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_EINVAL)/**<param error */
#define HI_GPSMNG_ENOTINIT                 HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ENOINIT)/**<Not inited */
#define HI_GPSMNG_EINITIALIZED             HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_EINITIALIZED)/**<Already Initialized */
#define HI_GPSMNG_EINTER                   HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_EINTER)/**<Internal error */
#define HI_GPSMNG_EREGISTEREVENT           HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM)/**<thread creat or join error*/
#define HI_GPSMNG_EREGISTER                HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 1)/**<register error*/
#define HI_GPSMNG_EUREGISTER               HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 2)/**<uregister error*/
#define HI_GPSMNG_ESTART                   HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 3)/**<start error*/
#define HI_GPSMNG_EALREADYSTART            HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 4)/**<start error*/
#define HI_GPSMNG_EDEINIT                  HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 5)/**<deinit error*/
#define HI_GPSMNG_EGETDATA                 HI_APPCOMM_ERR_ID(HI_APP_MOD_GPSMNG, HI_ERRNO_CUSTOM_BOTTOM + 6)/**<get data error*/

#define HI_GPSMNG_CALLBACK_MAX_NUM (2)
#define HI_GPSMNG_GSV_MAX_MSG_NUM (3)

typedef struct hiGPSMNG_FLOAT
{
    HI_S32 value;
    HI_S32 scale;
} HI_GPSMNG_FLOAT;

typedef struct hiGPSMNG_DATE
{
    HI_S32 day;
    HI_S32 month;
    HI_S32 year;
} HI_GPSMNG_DATE;

typedef struct hiGPSMNG_TIME
{
    HI_S32 hours;
    HI_S32 minutes;
    HI_S32 seconds;
    HI_S32 microseconds;
} HI_GPSMNG_TIME;

typedef struct hiGPSMNG_MSG_RMC
{
    HI_GPSMNG_TIME time;
    HI_BOOL valid;
    HI_GPSMNG_FLOAT latitude;
    HI_GPSMNG_FLOAT longitude;
    HI_GPSMNG_FLOAT speed;
    HI_GPSMNG_FLOAT course;
    HI_GPSMNG_FLOAT date;
    HI_GPSMNG_FLOAT variation;
} HI_GPSMNG_MSG_RMC;

typedef struct hiGPSMNG_MSG_GGA
{
    HI_GPSMNG_TIME time;
    HI_GPSMNG_FLOAT latitude;
    HI_GPSMNG_FLOAT longitude;
    HI_S32 fixQuality;
    HI_S32 satellitesTracked;
    HI_GPSMNG_FLOAT hdop;
    HI_GPSMNG_FLOAT altitude;
    HI_CHAR altitudeUnits;
    HI_GPSMNG_FLOAT height;
    HI_CHAR heightUnits;
    HI_GPSMNG_FLOAT dgpsAge;
} HI_GPSMNG_MSG_GGA;

/* FAA mode added to some fields in NMEA 2.3. */
typedef enum hiGPSMNG_FAA_MODE
{
    HI_GPS_FAA_MODE_AUTONOMOUS = 'A',
    HI_GPS_FAA_MODE_DIFFERENTIAL = 'D',
    HI_GPS_FAA_MODE_ESTIMATED = 'E',
    HI_GPS_FAA_MODE_MANUAL = 'M',
    HI_GPS_FAA_MODE_SIMULATED = 'S',
    HI_GPS_FAA_MODE_NOT_VALID = 'N',
    HI_GPS_FAA_MODE_PRECISE = 'P',
} HI_GPSMNG_FAA_MODE;

typedef struct hiGPSMNG_MSG_GLL
{
    HI_GPSMNG_FLOAT latitude;
    HI_GPSMNG_FLOAT longitude;
    HI_GPSMNG_TIME  time;
    HI_CHAR status;
    HI_CHAR mode;
} HI_GPSMNG_MSG_GLL;

typedef struct hiGPSMNG_MSG_GST
{
    HI_GPSMNG_TIME time;
    HI_GPSMNG_FLOAT rmsDeviation;
    HI_GPSMNG_FLOAT semiMajorDeviation;
    HI_GPSMNG_FLOAT semiMinorDeviation;
    HI_GPSMNG_FLOAT semiMajorOrientation;
    HI_GPSMNG_FLOAT latitudeErrorDeviation;
    HI_GPSMNG_FLOAT longitudeErrorDeviation;
    HI_GPSMNG_FLOAT altitudeErrorDeviation;
} HI_GPSMNG_MSG_GST;

typedef struct hiGPSMNG_MSG_GSA
{
    HI_CHAR mode;
    HI_S32 fixType;
    HI_S32 sats[12];
    HI_GPSMNG_FLOAT pdop;
    HI_GPSMNG_FLOAT hdop;
    HI_GPSMNG_FLOAT vdop;
} HI_GPSMNG_MSG_GSA;

typedef struct hiGPSMNG_STATE_INFO
{
    HI_S32 nr;
    HI_S32 elevation;
    HI_S32 azimuth;
    HI_S32 snr;
} HI_GPSMNG_STATE_INFO;

typedef struct hiGPSMNG_MSG_GSV
{
    HI_S32 totalMsgs;
    HI_S32 msgNr;
    HI_S32 totalSats;
    HI_GPSMNG_STATE_INFO sats[4];
} HI_GPSMNG_MSG_GSV;

typedef struct hiGPSMNG_MSG_VTG
{
    HI_GPSMNG_FLOAT trueTrackDegrees;
    HI_GPSMNG_FLOAT magneticTrackDegrees;
    HI_GPSMNG_FLOAT speedKnots;
    HI_GPSMNG_FLOAT speedKph;
    HI_GPSMNG_FAA_MODE faaMode;
} HI_GPSMNG_MSG_VTG;

typedef struct hiGPSMNG_MSG_ZDA
{
    HI_GPSMNG_TIME time;
    HI_GPSMNG_DATE date;
    HI_S32 hourOffset;
    HI_S32 minuteOffset;
} HI_GPSMNG_MSG_ZDA;

typedef struct hiGPSMNG_MSG_PACKET
{
    HI_GPSMNG_MSG_RMC gpsRMC;
    HI_GPSMNG_MSG_GGA gpsGGA;
    HI_GPSMNG_MSG_GLL gpsGLL;
    HI_GPSMNG_MSG_GSA gpsGSA;
    HI_GPSMNG_MSG_VTG gpsVTG;
    HI_GPSMNG_MSG_GSV gpsGSV[HI_GPSMNG_GSV_MAX_MSG_NUM];
    HI_GPSMNG_MSG_GST gpsGST;
    HI_GPSMNG_MSG_ZDA gpsZDA;
} HI_GPSMNG_MSG_PACKET;

/** get data callback */
typedef HI_S32 (*HI_GPSMNG_CALLBACK_FN_PTR)(HI_GPSMNG_MSG_PACKET *msgPacket, HI_VOID* privateData);

/** get gps data */
typedef struct hiGPSMNG_CALLBACK
{
    HI_GPSMNG_CALLBACK_FN_PTR fnGpsDataCB;
    HI_VOID* privateData;
} HI_GPSMNG_CALLBACK;

HI_S32 HI_GPSMNG_Init(HI_VOID);
HI_S32 HI_GPSMNG_Deinit(HI_VOID);

HI_S32 HI_GPSMNG_Register(HI_GPSMNG_CALLBACK* fnGpsCB);
HI_S32 HI_GPSMNG_UnRegister(HI_GPSMNG_CALLBACK* fnGpsCB);

HI_S32 HI_GPSMNG_Start(HI_VOID);
HI_S32 HI_GPSMNG_Stop(HI_VOID);

HI_S32 HI_GPSMNG_GetData(HI_GPSMNG_MSG_PACKET* msgPacket);

/* vim: set ts=4 sw=4 et: */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
