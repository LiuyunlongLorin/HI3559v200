/**
* @file    hi_hal_key.h
* @brief   product hal key interface
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "hi_gpsmng.h"
#include "gpsmng_analysis.h"
#include "hi_appcomm_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

//#define GPSMNG_DEBUG

#define next_field() \
    do { \
        /* Progress to the next field. */ \
        while (GPSMNG_IsField(*message)) \
            message++; \
        /* Make sure there is a field there. */ \
        if (*message == ',') { \
            message++; \
            field = message; \
        } else { \
            field = HI_NULL; \
        } \
    } while (0)

static inline HI_BOOL GPSMNG_IsField(HI_CHAR character)
{
    return isprint((HI_U8) character) && character != ',' && character != '*';
}

static HI_BOOL GPSMNG_Scan(const HI_CHAR* message, const HI_CHAR* format, ...)
{
    HI_BOOL result = HI_FALSE;
    HI_BOOL optional = HI_FALSE;
    va_list ap;
    va_start(ap, format);

    const HI_CHAR* field = message;

    while (*format)
    {
        HI_CHAR type = *format++;

        if (type == ';')
        {
            // All further fields are optional.
            optional = HI_TRUE;
            continue;
        }

        if (!field && !optional)
        {
            // Field requested but we ran out if input. Bail out.
            goto parse_error;
        }

        switch (type)
        {
            case 'c':   // Single character field (HI_CHAR).
            {
                HI_CHAR value = '\0';

                if (field && GPSMNG_IsField(*field))
                { value = *field; }

                *va_arg(ap, HI_CHAR*) = value;
            }
            break;

            case 'd':   // Single character direction field (HI_S32).
            {
                HI_S32 value = 0;

                if (field && GPSMNG_IsField(*field))
                {
                    switch (*field)
                    {
                        case 'N':
                        case 'E':
                            value = 1;
                            break;

                        case 'S':
                        case 'W':
                            value = -1;
                            break;

                        default:
                            goto parse_error;
                    }
                }

                *va_arg(ap, HI_S32*) = value;
            }
            break;

            case 'f':   // Fractional value with scale (struct F).
            {
                HI_S32 sign = 0;
                HI_S32 value = -1;
                HI_S32 scale = 0;

                if (field)
                {
                    while (GPSMNG_IsField(*field))
                    {
                        if (*field == '+' && !sign && value == -1)
                        {
                            sign = 1;
                        }
                        else if (*field == '-' && !sign && value == -1)
                        {
                            sign = -1;
                        }
                        else if (isdigit((HI_U8) *field))
                        {
                            HI_S32 digit = *field - '0';

                            if (value == -1)
                            { value = 0; }

                            if (value > (INT_LEAST32_MAX - digit) / 10)
                            {
                                /* we ran out of bits, what do we do? */
                                if (scale)
                                {
                                    /* truncate extra precision */
                                    break;
                                }
                                else
                                {
                                    /* integer overflow. bail out. */
                                    goto parse_error;
                                }
                            }

                            value = (10 * value) + digit;

                            if (scale)
                            { scale *= 10; }
                        }
                        else if (*field == '.' && scale == 0)
                        {
                            scale = 1;
                        }
                        else if (*field == ' ')
                        {
                            /* Allow spaces at the start of the field. Not NMEA
                             * conformant, but some modules do this. */
                            if (sign != 0 || value != -1 || scale != 0)
                            { goto parse_error; }
                        }
                        else
                        {
                            goto parse_error;
                        }

                        field++;
                    }
                }

                if ((sign || scale) && value == -1)
                { goto parse_error; }

                if (value == -1)
                {
                    /* No digits were scanned. */
                    value = 0;
                    scale = 0;
                }
                else if (scale == 0)
                {
                    /* No decimal point. */
                    scale = 1;
                }

                if (sign)
                { value *= sign; }

                *va_arg(ap, HI_GPSMNG_FLOAT*) = (HI_GPSMNG_FLOAT) {value, scale};
            }
            break;

            case 'i':   // Integer value, default 0 (HI_S32).
            {
                HI_S32 value = 0;

                if (field)
                {
                    HI_CHAR* endptr;
                    value = strtol(field, &endptr, 10);

                    if (GPSMNG_IsField(*endptr))
                    { goto parse_error; }
                }

                *va_arg(ap, HI_S32*) = value;
            }
            break;

            case 's':   // String value (HI_CHAR *).
            {
                HI_CHAR* buff = va_arg(ap, HI_CHAR*);

                if (field)
                {
                    while (GPSMNG_IsField(*field))
                    { *buff++ = *field++; }
                }

                *buff = '\0';
            }
            break;

            case 't':   // NMEA talker+message identifier (HI_CHAR *).
            {
                // This field is always mandatory.
                if (!field)
                { goto parse_error; }

                if (field[0] != '$')
                { goto parse_error; }

                for (HI_S32 f = 0; f < 5; f++)
                    if (!GPSMNG_IsField(field[1 + f]))
                    { goto parse_error; }

                HI_CHAR* buff = va_arg(ap, HI_CHAR*);
                memcpy(buff, field + 1, 5);
                buff[5] = '\0';
            }
            break;

            case 'D':   // Date (HI_S32, HI_S32, HI_S32), -1 if empty.
            {
                HI_GPSMNG_DATE* gpsDate = va_arg(ap, HI_GPSMNG_DATE*);

                HI_S32 d = -1, m = -1, y = -1;

                if (field && GPSMNG_IsField(*field))
                {
                    // Always six digits.
                    for (HI_S32 f = 0; f < 6; f++)
                        if (!isdigit((HI_U8) field[f]))
                        { goto parse_error; }

                    HI_CHAR dArr[] = {field[0], field[1], '\0'};
                    HI_CHAR mArr[] = {field[2], field[3], '\0'};
                    HI_CHAR yArr[] = {field[4], field[5], '\0'};
                    d = strtol(dArr, HI_NULL, 10);
                    m = strtol(mArr, HI_NULL, 10);
                    y = strtol(yArr, HI_NULL, 10);
                }

                gpsDate->day= d;
                gpsDate->month= m;
                gpsDate->year= y;
            }
            break;

            case 'T':   // Time (HI_S32, HI_S32, HI_S32, HI_S32), -1 if empty.
            {
                HI_GPSMNG_TIME* gpsTime = va_arg(ap, HI_GPSMNG_TIME*);

                HI_S32 h = -1, i = -1, s = -1, u = -1;

                if (field && GPSMNG_IsField(*field))
                {
                    // Minimum required: integer time.
                    for (HI_S32 f = 0; f < 6; f++)
                        if (!isdigit((HI_U8) field[f]))
                        { goto parse_error; }

                    HI_CHAR hArr[] = {field[0], field[1], '\0'};
                    HI_CHAR iArr[] = {field[2], field[3], '\0'};
                    HI_CHAR sArr[] = {field[4], field[5], '\0'};
                    h = strtol(hArr, HI_NULL, 10);
                    i = strtol(iArr, HI_NULL, 10);
                    s = strtol(sArr, HI_NULL, 10);
                    field += 6;

                    // Extra: fractional time. Saved as microseconds.
                    if (*field++ == '.')
                    {
                        HI_U32 value = 0;
                        HI_U32 scale = 1000000LU;

                        while (isdigit((HI_U8) *field) && scale > 1)
                        {
                            value = (value * 10) + (*field++ - '0');
                            scale /= 10;
                        }

                        u = value * scale;
                    }
                    else
                    {
                        u = 0;
                    }
                }

                gpsTime->hours= h;
                gpsTime->microseconds= i;
                gpsTime->seconds= s;
                gpsTime->microseconds= u;
            }
            break;

            case '_':   // Ignore the field.
            {
            } break;

            default:   // Unknown.
            {
                goto parse_error;
            }
        }

        next_field();
    }

    result = HI_TRUE;

parse_error:
    va_end(ap);
    return result;
}

HI_BOOL GPSMNG_ParseRMC(HI_GPSMNG_MSG_RMC* gpsMsg, const HI_CHAR* message)
{
    // $GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62
    HI_CHAR type[6];
    HI_CHAR validity;
    HI_S32 latitudeDirection;
    HI_S32 longitudeDirection;
    HI_S32 variationDirection;

    if (!GPSMNG_Scan(message, "tTcfdfdffDfd",
                     type,
                     &gpsMsg->time,
                     &validity,
                     &gpsMsg->latitude, &latitudeDirection,
                     &gpsMsg->longitude, &longitudeDirection,
                     &gpsMsg->speed,
                     &gpsMsg->course,
                     &gpsMsg->date,
                     &gpsMsg->variation, &variationDirection))
    { return HI_FALSE; }

    if (strncmp(type + 2, "RMC", 3))
    { return HI_FALSE; }

    gpsMsg->valid= (validity == 'A');
    gpsMsg->latitude.value *= latitudeDirection;
    gpsMsg->longitude.value *= longitudeDirection;
    gpsMsg->variation.value *= variationDirection;

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseGGA(HI_GPSMNG_MSG_GGA* gpsMsg, const HI_CHAR* message)
{
    // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    HI_CHAR type[6];
    HI_S32 latitudeDirection;
    HI_S32 longitudeDirection;

    if (!GPSMNG_Scan(message, "tTfdfdiiffcfcf_",
                     type,
                     &gpsMsg->time,
                     &gpsMsg->latitude, &latitudeDirection,
                     &gpsMsg->longitude, &longitudeDirection,
                     &gpsMsg->fixQuality,
                     &gpsMsg->satellitesTracked,
                     &gpsMsg->hdop,
                     &gpsMsg->altitude,
                     &gpsMsg->altitudeUnits,
                     &gpsMsg->height,
                     &gpsMsg->heightUnits,
                     &gpsMsg->dgpsAge))
    { return HI_FALSE; }

    if (strncmp(type + 2, "GGA", 3))
    { return HI_FALSE; }

    gpsMsg->latitude.value *= latitudeDirection;
    gpsMsg->longitude.value *= longitudeDirection;

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseGSA(HI_GPSMNG_MSG_GSA* gpsMsg, const HI_CHAR* message)
{
    // $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
    HI_CHAR type[6];

    if (!GPSMNG_Scan(message, "tciiiiiiiiiiiiifff",
                     type,
                     &gpsMsg->mode,
                     &gpsMsg->fixType,
                     &gpsMsg->sats[0],
                     &gpsMsg->sats[1],
                     &gpsMsg->sats[2],
                     &gpsMsg->sats[3],
                     &gpsMsg->sats[4],
                     &gpsMsg->sats[5],
                     &gpsMsg->sats[6],
                     &gpsMsg->sats[7],
                     &gpsMsg->sats[8],
                     &gpsMsg->sats[9],
                     &gpsMsg->sats[10],
                     &gpsMsg->sats[11],
                     &gpsMsg->pdop,
                     &gpsMsg->hdop,
                     &gpsMsg->vdop))
    { return HI_FALSE; }

    if (strncmp(type + 2, "GSA", 3))
    { return HI_FALSE; }

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseGLL(HI_GPSMNG_MSG_GLL* gpsMsg, const HI_CHAR* message)
{
    // $GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41$;
    HI_CHAR type[6];
    HI_S32 latitudeDirection;
    HI_S32 longitudeDirection;

    if (!GPSMNG_Scan(message, "tfdfdTc;c",
                     type,
                     &gpsMsg->latitude, &latitudeDirection,
                     &gpsMsg->longitude, &longitudeDirection,
                     &gpsMsg->time,
                     &gpsMsg->status,
                     &gpsMsg->mode))
    { return HI_FALSE; }

    if (strncmp(type + 2, "GLL", 3))
    { return HI_FALSE; }

    gpsMsg->latitude.value *= latitudeDirection;
    gpsMsg->longitude.value *= longitudeDirection;

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseGST(HI_GPSMNG_MSG_GST* gpsMsg, const HI_CHAR* message)
{
    // $GPGST,024603.00,3.2,6.6,4.7,47.3,5.8,5.6,22.0*58
    HI_CHAR type[6];

    if (!GPSMNG_Scan(message, "tTfffffff",
                     type,
                     &gpsMsg->time,
                     &gpsMsg->rmsDeviation,
                     &gpsMsg->semiMajorDeviation,
                     &gpsMsg->semiMinorDeviation,
                     &gpsMsg->semiMajorOrientation,
                     &gpsMsg->latitudeErrorDeviation,
                     &gpsMsg->longitudeErrorDeviation,
                     &gpsMsg->altitudeErrorDeviation))
    { return HI_FALSE; }

    if (strncmp(type + 2, "GST", 3))
    { return HI_FALSE; }

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseGSV(HI_GPSMNG_MSG_GSV* gpsMsg, const HI_CHAR* message)
{
    // $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
    // $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D
    // $GPGSV,4,2,11,08,51,203,30,09,45,215,28*75
    // $GPGSV,4,4,13,39,31,170,27*40
    // $GPGSV,4,4,13*7B
    HI_CHAR type[6];

    if (!GPSMNG_Scan(message, "tiii;iiiiiiiiiiiiiiii",
                     type,
                     &gpsMsg->totalMsgs,
                     &gpsMsg->msgNr,
                     &gpsMsg->totalSats,
                     &gpsMsg->sats[0].nr,
                     &gpsMsg->sats[0].elevation,
                     &gpsMsg->sats[0].azimuth,
                     &gpsMsg->sats[0].snr,
                     &gpsMsg->sats[1].nr,
                     &gpsMsg->sats[1].elevation,
                     &gpsMsg->sats[1].azimuth,
                     &gpsMsg->sats[1].snr,
                     &gpsMsg->sats[2].nr,
                     &gpsMsg->sats[2].elevation,
                     &gpsMsg->sats[2].azimuth,
                     &gpsMsg->sats[2].snr,
                     &gpsMsg->sats[3].nr,
                     &gpsMsg->sats[3].elevation,
                     &gpsMsg->sats[3].azimuth,
                     &gpsMsg->sats[3].snr
                    ))
    {
        return HI_FALSE;
    }

    if (strncmp(type + 2, "GSV", 3))
    { return HI_FALSE; }

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseVTG(HI_GPSMNG_MSG_VTG* gpsMsg, const HI_CHAR* message)
{
    // $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48
    // $GPVTG,156.1,T,140.9,M,0.0,N,0.0,K*41
    // $GPVTG,096.5,T,083.5,M,0.0,N,0.0,K,D*22
    // $GPVTG,188.36,T,,M,0.820,N,1.519,K,A*3F
    HI_CHAR type[6];
    HI_CHAR cTrue, cMagnetic, cKnots, cKph, cFaaMode;

    if (!GPSMNG_Scan(message, "tfcfcfcfc;c",
                     type,
                     &gpsMsg->trueTrackDegrees,
                     &cTrue,
                     &gpsMsg->magneticTrackDegrees,
                     &cMagnetic,
                     &gpsMsg->speedKnots,
                     &cKnots,
                     &gpsMsg->speedKph,
                     &cKph,
                     &cFaaMode))
    { return HI_FALSE; }

    if (strncmp(type + 2, "VTG", 3))
    { return HI_FALSE; }

    // check chars
    if (cTrue != 'T' ||
        cMagnetic != 'M' ||
        cKnots != 'N' ||
        cKph != 'K')
    { return HI_FALSE; }

    gpsMsg->faaMode = (HI_GPSMNG_FAA_MODE)cFaaMode;

    return HI_TRUE;
}

HI_BOOL GPSMNG_ParseZDA(HI_GPSMNG_MSG_ZDA* gpsMsg, const HI_CHAR* message)
{
    // $GPZDA,201530.00,04,07,2002,00,00*60
    HI_CHAR type[6];

    if (!GPSMNG_Scan(message, "tTiiiii",
                     type,
                     &gpsMsg->time,
                     &gpsMsg->date.day,
                     &gpsMsg->date.month,
                     &gpsMsg->date.year,
                     &gpsMsg->hourOffset,
                     &gpsMsg->minuteOffset))
    { return HI_FALSE; }

    if (strncmp(type + 2, "ZDA", 3))
    { return HI_FALSE; }

    // check offsets
    if (abs(gpsMsg->hourOffset) > 13 ||
        gpsMsg->minuteOffset > 59 ||
        gpsMsg->minuteOffset < 0)
    { return HI_FALSE; }

    return HI_TRUE;
}

HI_S32 GPSMNG_ParserRawData(GPSMNG_RAW_DATA* gpsRawData, HI_GPSMNG_MSG_PACKET* gpsMsgPack)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* parser RMC rawdata */
    s32Ret = GPSMNG_ParseRMC(&(gpsMsgPack->gpsRMC), gpsRawData->rmcStr);

    if (s32Ret != HI_TRUE)
    {
        MLOGD("parser RMC str failed!\n");
    }

#ifdef GPSMNG_DEBUG
    MLOGE("RMC Parser Result:\n time: %d/%d/%d/%d \n status: %d \n latitude: %d %d\n longitude: %d %d\n speed: %d %d\n course: %d %d\n date: %d %d\n variation %d %d\n",
          gpsMsgPack->gpsRMC.time.hours, gpsMsgPack->gpsRMC.time.minutes, gpsMsgPack->gpsRMC.time.seconds, gpsMsgPack->gpsRMC.time.microseconds,
          gpsMsgPack->gpsRMC.valid,
          gpsMsgPack->gpsRMC.latitude.value, gpsMsgPack->gpsRMC.latitude.scale,
          gpsMsgPack->gpsRMC.longitude.value, gpsMsgPack->gpsRMC.longitude.scale,
          gpsMsgPack->gpsRMC.speed.value, gpsMsgPack->gpsRMC.speed.scale,
          gpsMsgPack->gpsRMC.course.value, gpsMsgPack->gpsRMC.course.scale,
          gpsMsgPack->gpsRMC.date.value, gpsMsgPack->gpsRMC.date.scale,
          gpsMsgPack->gpsRMC.variation.value, gpsMsgPack->gpsRMC.variation.scale);
#endif

    /* parser GGA rawdata*/
    s32Ret = GPSMNG_ParseGGA(&(gpsMsgPack->gpsGGA), gpsRawData->ggaStr);

    if (s32Ret != HI_TRUE)
    {
        MLOGD("parser GGA str failed!\n");
    }

#ifdef GPSMNG_DEBUG
    MLOGE("GGA Parser Result:\n time: %d/%d/%d/%d \n latitude: %d %d \n longitude : %d %d\n FixQuality: %d\n SatellitesTracked: %d\n Hdop: %d %d\n Altitude: %d %d\n AltitudeUnits: %c\n Height: %d %d\n HeightUnits: %c\n dgpsAge: %d %d\n",
          gpsMsgPack->gpsGGA.time.hours, gpsMsgPack->gpsGGA.time.minutes, gpsMsgPack->gpsGGA.time.seconds, gpsMsgPack->gpsGGA.time.microseconds,
          gpsMsgPack->gpsGGA.latitude.value, gpsMsgPack->gpsGGA.latitude.scale,
          gpsMsgPack->gpsGGA.longitude.value, gpsMsgPack->gpsGGA.longitude.scale,
          gpsMsgPack->gpsGGA.fixQuality,
          gpsMsgPack->gpsGGA.satellitesTracked,
          gpsMsgPack->gpsGGA.hdop.value, gpsMsgPack->gpsGGA.hdop.scale,
          gpsMsgPack->gpsGGA.altitude.value, gpsMsgPack->gpsGGA.altitude.scale,
          gpsMsgPack->gpsGGA.altitudeUnits,
          gpsMsgPack->gpsGGA.height.value, gpsMsgPack->gpsGGA.height.scale,
          gpsMsgPack->gpsGGA.heightUnits,
          gpsMsgPack->gpsGGA.dgpsAge.value, gpsMsgPack->gpsGGA.dgpsAge.scale);
#endif

    /* parser GLL rawdata*/
    s32Ret = GPSMNG_ParseGLL(&(gpsMsgPack->gpsGLL), gpsRawData->gllStr);

    if (s32Ret != HI_TRUE)
    {
        MLOGD("parser GLL str failed!\n");
    }

#ifdef GPSMNG_DEBUG
    MLOGE("\nGLL Parser Result:\n time: %d/%d/%d/%d \n latitude: %d %d \n longitude : %d %d\n status: %c\n mode: %c\n",
          gpsMsgPack->gpsGLL.time.hours, gpsMsgPack->gpsGLL.time.minutes, gpsMsgPack->gpsGLL.time.seconds, gpsMsgPack->gpsGLL.time.microseconds,
          gpsMsgPack->gpsGLL.latitude.value, gpsMsgPack->gpsGLL.latitude.scale,
          gpsMsgPack->gpsGLL.longitude.value, gpsMsgPack->gpsGLL.longitude.scale,
          gpsMsgPack->gpsGLL.status,
          gpsMsgPack->gpsGLL.mode);
#endif

    /* parser GSA rawdata*/
    s32Ret = GPSMNG_ParseGSA(&(gpsMsgPack->gpsGSA), gpsRawData->gsaStr);

    if (s32Ret != HI_TRUE)
    {
        MLOGD("parser GSA str failed!\n");
    }

#ifdef GPSMNG_DEBUG
    MLOGE("\nGSA Parser Result:\n mode: %c\n fixType: %d\n sats:%d %d %d %d %d %d %d %d %d %d %d %d\n Pdop: %d %d\n Hdop: %d %d\n Vdop: %d %d\n",
          gpsMsgPack->gpsGSA.mode,
          gpsMsgPack->gpsGSA.fixType,
          gpsMsgPack->gpsGSA.sats[0], gpsMsgPack->gpsGSA.sats[1], gpsMsgPack->gpsGSA.sats[2],
          gpsMsgPack->gpsGSA.sats[3], gpsMsgPack->gpsGSA.sats[4], gpsMsgPack->gpsGSA.sats[5],
          gpsMsgPack->gpsGSA.sats[6], gpsMsgPack->gpsGSA.sats[7], gpsMsgPack->gpsGSA.sats[8],
          gpsMsgPack->gpsGSA.sats[9], gpsMsgPack->gpsGSA.sats[10], gpsMsgPack->gpsGSA.sats[11],
          gpsMsgPack->gpsGSA.pdop.value, gpsMsgPack->gpsGSA.pdop.scale,
          gpsMsgPack->gpsGSA.hdop.value, gpsMsgPack->gpsGSA.hdop.scale,
          gpsMsgPack->gpsGSA.vdop.value, gpsMsgPack->gpsGSA.vdop.scale);
#endif

    /* parser VTG rawdata*/
    s32Ret = GPSMNG_ParseVTG(&(gpsMsgPack->gpsVTG), gpsRawData->vtgStr);

    if (s32Ret != HI_TRUE)
    {
        MLOGD("parser VTG str failed!\n");
    }

#ifdef GPSMNG_DEBUG
    MLOGE("\nVTG Parser Result:\n TrueTrackDegrees: %d %d\n MagneticTrackDegrees: %d %d\n SpeedKnots: %d %d\n SpeedKph: %d %d\n FaaMode:%c\n",
          gpsMsgPack->gpsVTG.trueTrackDegrees.value, gpsMsgPack->gpsVTG.magneticTrackDegrees.scale,
          gpsMsgPack->gpsVTG.magneticTrackDegrees.value, gpsMsgPack->gpsVTG.magneticTrackDegrees.scale,
          gpsMsgPack->gpsVTG.speedKnots.value, gpsMsgPack->gpsVTG.speedKnots.scale,
          gpsMsgPack->gpsVTG.speedKph.value, gpsMsgPack->gpsVTG.speedKph.scale,
          gpsMsgPack->gpsVTG.faaMode);
#endif


    /* parser GSV rawdata*/
    HI_S32 i = 0;

    for (i = 0; i < HI_GPSMNG_GSV_MAX_MSG_NUM ; i++)
    {
        s32Ret = GPSMNG_ParseGSV(&(gpsMsgPack->gpsGSV[i]), gpsRawData->gsvStr[i]);

        if (s32Ret != HI_TRUE)
        {
            MLOGD("parser GSV str [%d/%d] failed!\n", i, HI_GPSMNG_GSV_MAX_MSG_NUM);
        }
    }

#ifdef GPSMNG_DEBUG

    for (i = 0; i < HI_GPSMNG_GSV_MAX_MSG_NUM; i++)
    {
        MLOGE("\nGSV Parser Result:\n totalmsg: %d\n msgnr: %d\n totalstats: %d\n sat[0]:Nr=%d Elevation=%d Azimuth=%d Snr=%d\n sat[2]:Nr=%d Elevation=%d Azimuth=%d Snr=%d\n sat[3]:Nr=%d Elevation=%d Azimuth=%d Snr=%d\n sat[4]:Nr=%d Elevation=%d Azimuth=%d Snr=%d\n",
              gpsMsgPack->gpsGSV[i].totalMsgs,
              gpsMsgPack->gpsGSV[i].msgNr,
              gpsMsgPack->gpsGSV[i].totalSats,
              gpsMsgPack->gpsGSV[i].sats[0].nr,
              gpsMsgPack->gpsGSV[i].sats[0].elevation,
              gpsMsgPack->gpsGSV[i].sats[0].azimuth,
              gpsMsgPack->gpsGSV[i].sats[0].snr,
              gpsMsgPack->gpsGSV[i].sats[1].nr,
              gpsMsgPack->gpsGSV[i].sats[1].elevation,
              gpsMsgPack->gpsGSV[i].sats[1].azimuth,
              gpsMsgPack->gpsGSV[i].sats[1].snr,
              gpsMsgPack->gpsGSV[i].sats[2].nr,
              gpsMsgPack->gpsGSV[i].sats[2].elevation,
              gpsMsgPack->gpsGSV[i].sats[2].azimuth,
              gpsMsgPack->gpsGSV[i].sats[2].snr,
              gpsMsgPack->gpsGSV[i].sats[3].nr,
              gpsMsgPack->gpsGSV[i].sats[3].elevation,
              gpsMsgPack->gpsGSV[i].sats[3].azimuth,
              gpsMsgPack->gpsGSV[i].sats[3].snr);
    }

#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
