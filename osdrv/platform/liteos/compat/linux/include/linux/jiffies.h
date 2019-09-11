#ifndef _LINUX_JIFFIES_H
#define _LINUX_JIFFIES_H

#include "los_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


extern unsigned long long get_jiffies_64(void);
extern unsigned int jiffies_to_msecs(const unsigned long j);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LINUX_JIFFIES_H */

