#ifndef _LINUX_RTC_H_
#define _LINUX_RTC_H_

#include "los_typedef.h"
#include "liteos/rtc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * The struct used to pass data via the following ioctl. Similar to the
 * struct tm in <time.h>, but it needs to be here so that the kernel
 * source is self contained, allowing cross-compiles, etc. etc.
 */



static inline int is_leap_year(unsigned int year){return (int)((!(year % 4) && (year % 100)) || !(year % 400));}

int rtc_tm_to_time(struct rtc_time *tm, unsigned long *time);
void rtc_time_to_tm(unsigned long time, struct rtc_time *tm);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LINUX_RTC_H_ */
