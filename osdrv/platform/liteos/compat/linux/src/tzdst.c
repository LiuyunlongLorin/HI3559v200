/*----------------------------------------------------------------------------
 *      Huawei - Huawei LiteOS
 *----------------------------------------------------------------------------
 *      Name:    tzdst.c
 *      Purpose: time zone and daylight saving time functions
 *      Rev.:    V1.0.0
 *----------------------------------------------------------------------------
 *

 * Copyright (c) 2014, Huawei Technologies Co., Ltd.
 * All rights reserved.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 *THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *---------------------------------------------------------------------------*/
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "fcntl.h"
#include "tzfile.h"
#include "unistd.h"
#include "time.h"
#include "errno.h"
#include "tzdst.h"
#ifndef LOSCFG_AARCH64
#include "time64.h"
#endif

extern int time_lock(void);
extern void time_unlock(void);

static const int s_wMonLengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};


/****************************************************************************
 * Time Zone functions
 ****************************************************************************/
 #define IS_NUM(x) (((x) >= '0') && ((x) <= '9'))

long int timezone;

static void buffer_insert(char * pBuf, size_t uwBufLen, size_t uwPositions, char cData)
{
    if (uwBufLen <= uwPositions)
    {
        return;
    }
    memmove(&pBuf[uwPositions + 1], &pBuf[uwPositions], uwBufLen - uwPositions - 1);
    pBuf[uwPositions] = cData;
}

/* tzn[+/-]hh[:mm[:ss]][dzn] */
void settimezone(const char *buff)
{
    /* tzn[+/-]hh[:mm[:ss]][dzn] */
    #define STANDARD_TZ_LEN 15
    int hour = 0;
    int minute = 0;
    int second = 0;
    size_t uwBuffLen = 0;
    char aucStandardString[STANDARD_TZ_LEN] = {0};

    if ((NULL == buff) || ((uwBuffLen = strlen(buff)) < 4))
    {
        goto ERROR;
    }

    (void)memset(aucStandardString, '#', STANDARD_TZ_LEN);
    (void)memcpy(aucStandardString, buff, (uwBuffLen <= STANDARD_TZ_LEN) ? uwBuffLen : STANDARD_TZ_LEN);
    if (aucStandardString[3] == '-' || aucStandardString[3] == '+')
    {
        if (!IS_NUM(aucStandardString[4]))
        {
            goto ERROR;
        }
    }
    else if (IS_NUM(aucStandardString[3]))
    {
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 3, '+');
    }
    else
    {
        goto ERROR;
    }

    if (!IS_NUM(aucStandardString[5]))
    {
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 4, '0');
    }

    if (aucStandardString[6] == ':')
    {
        if (!IS_NUM(aucStandardString[7]))
        {
            goto ERROR;
        }
        else if (!IS_NUM(aucStandardString[8]))
        {
            buffer_insert(aucStandardString, STANDARD_TZ_LEN, 7, '0');
        }
    }
    else
    {
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 6, ':');
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 7, '0');
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 8, '0');
    }

    if (aucStandardString[9] == ':')
    {
        if (!IS_NUM(aucStandardString[10]))
        {
            goto ERROR;
        }
        else if (!IS_NUM(aucStandardString[11]))
        {
            buffer_insert(aucStandardString, STANDARD_TZ_LEN, 10, '0');
        }
    }
    else
    {
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 9, ':');
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 10, '0');
        buffer_insert(aucStandardString, STANDARD_TZ_LEN, 11, '0');
    }

    hour    = (aucStandardString[4] - '0') * 10 + aucStandardString[5] - '0';
    minute = (aucStandardString[7] - '0') * 10 + aucStandardString[8] - '0';
    second = (aucStandardString[10] - '0') * 10 + aucStandardString[11] - '0';

    /* [-12:00:00, +14:00:00] limits */
    if ((minute > 59 || second > 59) ||
        ((aucStandardString[3] == '-') && ((hour > 12) || ((hour == 12) && ((minute != 0) || (second != 0))))) ||
        ((aucStandardString[3] == '+') && ((hour > 14) || ((hour == 14) && ((minute != 0) || (second != 0))))))
    {
        goto ERROR;
    }

    if(time_lock())
    {
        goto ERROR;
    }

    timezone = hour * 3600 + minute * 60 + second;
    if (aucStandardString[3] == '-')
    {
        timezone = -timezone;
    }

    time_unlock();

    return;

ERROR:
    PRINT_ERR("TZ file data error\n");
}

/****************************************************************************
 * DST functions
 ****************************************************************************/
#define DST_STR_LEN_FORMAT_MDAY 15 // for example "Feb-03 03:00:00"
#define DST_STR_LEN_FORMAT_WDAY 20 // for example "Oct-1st-Fri 02:59:59"
#define DST_SET_LENGTH_MAX (DST_STR_LEN_FORMAT_WDAY + 1)

const char *astrMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *astrMonthWeek[] = {"1st", "2nd", "3rd", "4th", "5th"};
const char *astrWeekDay[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static BOOL bIsDstWork = FALSE;
int swDstForwardSeconds = 0;
static char strDstStart[DST_SET_LENGTH_MAX] = {0};
static char strDstEnd[DST_SET_LENGTH_MAX] = {0};

static int get_month_from_string(const char *strMonth)
{
    int index = 0;
    for (index = 0; index < sizeof(astrMonth) / sizeof(char *); index++)/*lint !e574*/
    {
        if (0 == strncmp(strMonth, astrMonth[index], 3))
        {
            /* a legal month is from 0 to 11. */
            return index;
        }
    }

    return -1;
}

static int get_weekday_from_string(const char *strWeekDay)
{
    int index = 0;
    for (index = 0; index < sizeof(astrWeekDay) / sizeof(char *); index++)/*lint !e574*/
    {
        if (0 == strncmp(strWeekDay, astrWeekDay[index], 3))
        {
            /* Day of the week (0-6, Sunday = 0) */
            return index;
        }
    }

    return -1;
}

static int get_monthweek_from_string(const char *strMonthWeek)
{
    int index = 0;
    for (index = 0; index < sizeof(astrMonthWeek) / sizeof(char *); index++)/*lint !e574*/
    {
        if (0 == strncmp(strMonthWeek, astrMonthWeek[index], 3))
        {
            /* Month of the week (1-5) */
            return index + 1;
        }
    }

    return -1;
}

static int get_monthday_from_string(int month, const char *strMonDay)
{
    int mday = 0;

    if (((strMonDay[0] < '0') && (strMonDay[0] > '9'))
        || ((strMonDay[1] < '0') && (strMonDay[1] > '9')))
    {
        return -1;
    }

    mday = (strMonDay[0] - '0') * 10 + strMonDay[1] - '0';

    if (mday > 31)
    {
        return -1;
    }

    if ((mday == 29) && ((month + 1) == 2))    //not evey year have 29 days in Feb when set DST.
    {
        return -1;
     }

    if (mday > s_wMonLengths[0][month])
    {
        return -1;
    }

    /* Day of the month (1-31) */
    return mday;
}

static int get_dayseconds_from_string(const char *strTimeString)
{
    int hour, min, sec = 0;

    if (((strTimeString[0] < '0') && (strTimeString[0] > '9'))
        || ((strTimeString[1] < '0') && (strTimeString[1] > '9')))
    {
        return -1;
    }

    if (((strTimeString[3] < '0') && (strTimeString[3] > '9'))
        || ((strTimeString[4] < '0') && (strTimeString[4] > '9')))
    {
        return -1;
    }

    if (((strTimeString[6] < '0') && (strTimeString[6] > '9'))
        || ((strTimeString[7] < '0') && (strTimeString[7] > '9')))
    {
        return -1;
    }

    if ((strTimeString[2] != ':') || (strTimeString[5] != ':'))
    {
        return -1;
    }


    hour = (strTimeString[0] - '0') * 10 + strTimeString[1] - '0';
    min = (strTimeString[3] - '0') * 10 + strTimeString[4] - '0';
    sec = (strTimeString[6] - '0') * 10 + strTimeString[7] - '0';

    /* Hours (0-23) */
    if (hour < 0 || hour > 23)
    {
        return -1;
    }

    /* Minutes (0-59) */
    if (min < 0 || min > 59)
    {
        return -1;
    }

    /* Seconds (0-59), not consider of the leap seconds in DST. */
    if (sec < 0 || sec > 59)
    {
        return -1;
    }

    return hour * 3600 + min * 60 + sec;
}

static int dst_get_day_of_month(int year, int month, int mweek, int wday)
{
    int firstWeekDay = 0; //first week day in this month of the specified year.
    int firstMdayOfTargetWday = 0; //first target month day in this month of the specified year.
    int TargetMdayOfTargetWday = 0; //the target month day specified by user.
    struct tm stTime = {0};
    INT64 llSeconds = 0;

    stTime.tm_year = year;
    stTime.tm_mon = month;
    stTime.tm_mday = 1;
    stTime.tm_hour = 14;
    stTime.tm_isdst = 0;
#ifdef LOSCFG_AARCH64
    llSeconds = mktime(&stTime);
#else
    llSeconds = mktime64(&stTime);
#endif
    if (-1 == llSeconds)
    {
        return -1;
    }
    /* the first day of 1900 is Monday. */
    firstWeekDay = 1 + llSeconds / SECSPERDAY % DAYSPERWEEK;

    firstMdayOfTargetWday = 1 + (DAYSPERWEEK + wday - firstWeekDay) % DAYSPERWEEK;
    /* users may use 5th weekday to represent the last week of this month
      * for example "Oct-5th-Fri", but there does not exist the 5th Friday in October, so the last monweek is 4th.
      */
    TargetMdayOfTargetWday = firstMdayOfTargetWday + (mweek - 1) * DAYSPERWEEK;
    if (TargetMdayOfTargetWday > s_wMonLengths[(int)isleap(year + TM_YEAR_BASE)][month])
    {
        TargetMdayOfTargetWday -= 7;
    }

    return TargetMdayOfTargetWday;
}

static INT64 dst_config_decode(int year, const char * pstrDstString)
{
    int month, mday, sec, monweek, weekday = 0;
    struct tm stTime = {0};
    INT64 llDstSeconds = 0;

    if (strlen(pstrDstString) == DST_STR_LEN_FORMAT_MDAY) //for example "Feb-03 03:00:00"
    {
        if ((pstrDstString[3] != '-') || (pstrDstString[6] != ' '))
        {
            return -1;
        }
        if (-1 == (month = get_month_from_string(&pstrDstString[0])))
        {
            return -1;
        }
        if (-1 == (mday = get_monthday_from_string(month, &pstrDstString[4])))
        {
            return -1;
        }
        if (-1 == (sec = get_dayseconds_from_string(&pstrDstString[7])))
        {
            return -1;
        }
    }
    else if (strlen(pstrDstString) == DST_STR_LEN_FORMAT_WDAY) //for example "Oct-1st-Fri 02:59:59"
    {
        if ((pstrDstString[3] != '-') || (pstrDstString[7] != '-') || (pstrDstString[11] != ' '))
        {
            return -1;
        }
        if (-1 == (month = get_month_from_string(&pstrDstString[0])))
        {
            return -1;
        }
        if (-1 == (monweek = get_monthweek_from_string(&pstrDstString[4])))
        {
            return -1;
        }
        if (-1 == (weekday = get_weekday_from_string(&pstrDstString[8])))
        {
            return -1;
        }
        if (-1 == (sec = get_dayseconds_from_string(&pstrDstString[12])))
        {
            return -1;
        }

        if (-1 == (mday = dst_get_day_of_month(year, month, monweek, weekday)))
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    /* get the DST period */
    stTime.tm_year = year;
    stTime.tm_mon = month;
    stTime.tm_mday = mday;
    stTime.tm_isdst = 0;
#ifdef LOSCFG_AARCH64
    llDstSeconds = mktime(&stTime);
#else
    llDstSeconds = mktime64(&stTime);
#endif
    if (-1 == llDstSeconds)
    {
        return -1;
    }

    return llDstSeconds + sec;
}

static BOOL dst_config_check(const char * pstrDstStart, const char * pstrDstEnd)
{
    INT64 llDstStart, llDstEnd = 0;

    if ((NULL == pstrDstStart) || (NULL == pstrDstEnd))
    {
        return FALSE;
    }

    if (((llDstStart = dst_config_decode(70, pstrDstStart)) < 0)
        || ((llDstEnd = dst_config_decode(70, pstrDstEnd)) < 0))
    {
        return FALSE;
    }

    if (llDstStart >= llDstEnd)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL check_within_dst_period(const struct tm * const ptm, INT64 llSeconds)
{
    INT64 llDstStart, llDstEnd = 0;
    struct tm stTime = {0};

    if (!bIsDstWork)
    {
        return FALSE;
    }

    if (ptm) //represent a local time.
    {
        (VOID)memcpy(&stTime, ptm, sizeof(struct tm));
        stTime.tm_isdst = 0;
        /* the input-param of seconds is unused in this case. */
#ifdef LOSCFG_AARCH64
        llSeconds = mktime(&stTime);
#else
        llSeconds = mktime64(&stTime);
#endif
        if (-1 == llSeconds)
        {
            return FALSE;
        }
    }
    else //represent a standard time, not care TZ.
    {
#ifdef LOSCFG_AARCH64
        if (NULL == gmtime_r(&llSeconds, &stTime))
#else
        if (NULL == gmtime64_r(&llSeconds, &stTime))
#endif
        {
            return FALSE;
        }
    }

    llDstStart = dst_config_decode(stTime.tm_year, strDstStart);
    llDstEnd = dst_config_decode(stTime.tm_year, strDstEnd);
    if ((-1 == llDstStart) || (-1 == llDstEnd))
    {
        return FALSE;
    }

    if (ptm)
    {
        if (ptm->tm_isdst < 0)
        {
            /* it must to be. */
            if ((llSeconds >= llDstStart) && (llSeconds < llDstStart + swDstForwardSeconds))
            {
                return FALSE;
            }

            /* determine the time period of the second pass, out of the DST period. */
            if ((llSeconds > llDstEnd) && (llSeconds <= llDstEnd + swDstForwardSeconds))
            {
                return TRUE;
            }
        }
        else if (ptm->tm_isdst > 0)
        {
            /* the logical judgment here is the opposite of common sense */
            if ((llSeconds >= llDstStart) && (llSeconds < llDstStart + swDstForwardSeconds))
            {
                return TRUE;
            }
            else
            {
                return TRUE;
            }
        }
        else /* ptm->tm_isdst = 0 */
        {
            return FALSE;
        }
    }

    if ((llSeconds < llDstStart) || (llSeconds >= llDstEnd))
    {
        return FALSE;   //daylight saving time is not effect.
    }

    return TRUE; //daylight saving time is effect.
}

int dst_disable(VOID)
{
    if(time_lock())
    {
        return -1;
    }

    bIsDstWork = FALSE;

    time_unlock();

    return 0;
}

int dst_enable(const char * strDstStartTime, const char * strDstEndTime, int swForwardSeconds)
{
    if(time_lock())
    {
        return -1;
    }

    /* check if the format of dst config is correct. */
    if (TRUE != dst_config_check(strDstStartTime, strDstEndTime))
    {
        time_unlock();
        return -1;
    }

    if ((swForwardSeconds < 0) || (swForwardSeconds >= 24 * 3600))
    {
        time_unlock();
        return -1;
    }



    bIsDstWork = FALSE;
    strncpy(strDstStart, strDstStartTime, DST_SET_LENGTH_MAX);
    strncpy(strDstEnd, strDstEndTime, DST_SET_LENGTH_MAX);
    swDstForwardSeconds = swForwardSeconds;
    bIsDstWork = TRUE;

    time_unlock();

    return 0;
}

int dst_inquire(int year, struct tm *pstDstStart, struct tm *pstDstEnd)
{
    INT64 llDstStart, llDstEnd = 0;

    if(time_lock())
    {
        return -1;
    }

    if (!bIsDstWork)
    {
        time_unlock();
        return -1;
    }

    if ((NULL == pstDstStart) || (NULL == pstDstEnd))
    {
        time_unlock();
        return -1;
    }

    llDstStart = dst_config_decode(year, strDstStart);
    llDstEnd = dst_config_decode(year, strDstEnd);
    if ((-1 == llDstStart) || (-1 == llDstEnd))
    {
        time_unlock();
        return -1;
    }

    llDstStart += timezone;
    llDstEnd += timezone;
#ifdef LOSCFG_AARCH64
    if ((NULL == gmtime_r(&llDstStart, pstDstStart)) || (NULL == gmtime_r(&llDstEnd, pstDstEnd)))
#else
    if ((NULL == gmtime64_r(&llDstStart, pstDstStart)) || (NULL == gmtime64_r(&llDstEnd, pstDstEnd)))
#endif
    {
        time_unlock();
        return -1;
    }

    time_unlock();
    return 0;
}
