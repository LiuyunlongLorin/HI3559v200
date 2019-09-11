#include <linux/module.h>
#include "linux/rtc.h"

#define LEAPS_THRU_END_OF(year) ((year)/4 - (year)/100 + (year)/400)

static const unsigned short days_accumulate_array[13] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static const unsigned char days_in_month_array[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

int rtc_month_days(unsigned int month, unsigned int year)
{
    if(is_leap_year(year) && month == 1) {
        return days_in_month_array[month]+1;
    } else {
        return days_in_month_array[month];
    }
}

void rtc_time_to_tm(unsigned long time, struct rtc_time *rtm)
{
    unsigned int month, year;
    int days;

    days = time / 86400;
    time -= (unsigned int) days * 86400;

    rtm->tm_wday = (days + 4) % 7;

    year = 1970 + days / 365;
    days -= (year - 1970) * 365 + LEAPS_THRU_END_OF(year - 1)
        - LEAPS_THRU_END_OF(1970 - 1);
    if (days < 0) {
        year = year - 1;
        days += 365 + is_leap_year(year);
    }
    rtm->tm_year = year - 1900;
    rtm->tm_yday = days + 1;

    for (month = 0; month < 11; month++) {
        int new_days;

        new_days = days - rtc_month_days(month, year);
        if (new_days < 0)
            break;
        days = new_days;
    }
    rtm->tm_mon = month;
    rtm->tm_mday = days + 1;

    rtm->tm_hour = time / 3600;
    time -= rtm->tm_hour * 3600;
    rtm->tm_min = time / 60;
    rtm->tm_sec = time - rtm->tm_min * 60;

    rtm->tm_isdst = 0;
}

int rtc_valid_tm(struct rtc_time *rtm)
{
    if (rtm->tm_year < 70 || ((unsigned)rtm->tm_mon) >= 12 || rtm->tm_mday < 1)
        return -EINVAL;
    
    if (rtm->tm_mday > rtc_month_days(rtm->tm_mon, rtm->tm_year + 1900))
        return -EINVAL;
    
    if (((unsigned)rtm->tm_hour) >= 24 || ((unsigned)rtm->tm_min) >= 60
        || ((unsigned)rtm->tm_sec) >= 60)
        return -EINVAL;

    return 0;
}

static inline unsigned long mktime_RTC (unsigned int Year, unsigned int Mon,
    unsigned int Day, unsigned int Hour,
    unsigned int Min, unsigned int Sec)
{
    if (0 >= (int) (Mon -= 2)) {
         Mon += 12;
         Year -= 1;
    }

    return ((( (unsigned long) (Year/4 - Year/100 + Year/400 + 367*Mon/12 + Day) +
             Year*365 - 719499
          )*24 + Hour
       )*60 + Min
    )*60 + Sec;
}

int rtc_tm_to_time(struct rtc_time *rtm, unsigned long *time)
{
    *time = mktime_RTC(rtm->tm_year + 1900, rtm->tm_mon + 1, rtm->tm_mday,
            rtm->tm_hour, rtm->tm_min, rtm->tm_sec);
    return 0;
}

int rtc_year_days(unsigned int day, unsigned int month, unsigned int year)
{
    return days_accumulate_array[month]+(is_leap_year(year)&&(month >= 2))+day-1;
}

