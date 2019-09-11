#ifndef __MOTIONSENSOR_PROC_H__
#define __MOTIONSENSOR_PROC_H__
#include "motionsensor_ext.h"
#include "hi_comm_motionsensor.h"

#include "hi_type.h"

#define MAX_LEN (32)


//extern MSENSOR_PARAM_S*         MotionSensorStatus;


HI_S32 MPU_PROC_Init(void);
void MPU_PROC_Exit(void);
//extern HI_S32 HI_BMM150_GetProcInfo(void);



#endif

