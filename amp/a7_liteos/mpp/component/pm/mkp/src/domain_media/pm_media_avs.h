#ifndef  __PM_MEDIA_AVS_H__
#define  __PM_MEDIA_AVS_H__

#include "pm_device.h"

int media_domain_avs_init(struct pm_domain_device* pm_domain_dev);
int media_domain_avs_exit(struct pm_domain_device* pm_domain_dev);

int media_domain_getprofile(void);
void media_domain_setprofile(int profile);
int media_domain_setvoltage(int set_volt);

#endif

