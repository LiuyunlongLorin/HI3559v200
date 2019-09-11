
#ifndef __MOTIONSENSOR_INNER_H__
#define __MOTIONSENSOR_INNER_H__

#include "hi_type.h"
#include "motionsensor_ext.h"

#define TEST_ON 0


/********************************/
/**\name MAG INTERFACE */
/*******************************/
#define C_BMI160_BYTE_COUNT                 (2)
#define BMI160_SLEEP_STATE                  (0x00)
#define BMI160_WAKEUP_INTR                  (0x00)
#define BMI160_SLEEP_TRIGGER                (0x04)
#define BMI160_WAKEUP_TRIGGER               (0x02)
#define BMI160_ENABLE_FIFO_WM               (0x02)
#define BMI160_MAG_INTERFACE_OFF_PRIMARY_ON (0x00)
#define BMI160_MAG_INTERFACE_ON_PRIMARY_ON  (0x02)

#define BMI160_MODE_SWITCHING_DELAY         (30)


#define CHECK_NULL_PTR(ptr)\
do{\
    if(NULL == ptr)\
     {\
        printk("input NULL Ptr\n");\
        return HI_FAILURE;\
     }\
}while(0)


HI_S32 HI_MOTIONSENSOR_GetProcInfo(HI_VOID);

#endif


