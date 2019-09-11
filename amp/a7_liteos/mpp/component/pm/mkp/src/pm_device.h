#ifndef  __PM_DEVICE_H__
#define  __PM_DEVICE_H__

#include "hi_osal.h"

#include "hi_comm_pm.h"
#include "pm_drv_comm.h"
#include "pm_device_adapt.h"


#undef _1K
#define _1K 1000ULL
#undef _1M
#define _1M 1000000ULL
#undef _1G
#define _1G 1000000000ULL

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

struct pm_profile;
struct freq_device;
struct hpm_opp;
struct avs_device;
struct regulator_device;
struct pm_device;
struct pm_domain_device;


typedef int  domain_avs_init(struct pm_domain_device*);
typedef int  domain_avs_exit(struct pm_domain_device*);
typedef int  domain_freq_init(struct pm_domain_device*);
typedef int  domain_freq_exit(struct pm_domain_device*);
typedef void domain_freeze(struct pm_domain_device*);
typedef void domain_restore(struct pm_domain_device*);


typedef enum PM_DOMAIN {
    PM_DOMAIN_CPU   = 0,
    PM_DOMAIN_MEDIA = 1,
    PM_DOMAIN_DDR   = 2,
    PM_DOMAIN_CORE  = 3,
    PM_DOMAIN_GPU   = 4,
    PM_DOMAIN_BUTT
} PM_DOMAIN_E;


struct hpm_opp {
    struct avs_device* avs_dev;

    int freq;          /* unit: kHz */
    int vmin;          /* unit: uV */
    int vmax;          /* unit: uV */
    int hpmrecord;     /* hpm record */
    int div;           /* frequency dividing ratio */
    int temp;          /* temperature */
    int profile;
};

struct avs_device {
    struct pm_domain_device* pm_domain_dev;

    struct hpm_opp* hpm_opp_table;
    int hpm_opp_num;

    int cur_hpm;
    int cur_freq;
    int cur_volt_min;
    int cur_volt_max;
    int div;

    int temp_num;
    int temp;
    int profile_num;
    int cur_profile;

    /*temperature policy */
    int poweron_volt;/* unit:uV */

    int (*handle)(void);
};


struct pm_domain_device {
    struct pm_device* pm_dev;

    /* freq */
    int cur_freq; /* unit:Hz */
    struct freq_device* freq_dev;
    domain_freq_init* domain_freq_init;
    domain_freq_exit* domain_freq_exit;

    /* regulator_dev */
    int cur_volt; /* unit:uV */
    struct regulator_device* regulator_dev;
    domain_regulator_init* domain_regulator_init;
    domain_regulator_exit* domain_regulator_exit;

    /* avs  */
    struct avs_device* avs_dev;
    domain_avs_init* domain_avs_init;
    domain_avs_exit* domain_avs_exit;

    /*global pm ctrl*/
    domain_freeze* domain_freeze;
    domain_restore* domain_restore;
};

struct pm_device {
    int domain_num;

    osal_mutex_t lock;
    int pm_enable;
    int is_tsensor_init;
    int is_init;

    struct pm_domain_device* cpu_domain_device;
    struct pm_domain_device* media_domain_device;
    struct pm_domain_device* core_domain_device;
    //    struct pm_domain_device* ddr_domain_device;
    //    struct pm_domain_device* gpu_domain_device;
};

/* main task timer handler */
typedef int (*timer_handler_func_t)(void*);

void start_timer(timer_handler_func_t pfntimer_handler);



#endif/* End of #ifndef __PM_DEVICE_H__*/

