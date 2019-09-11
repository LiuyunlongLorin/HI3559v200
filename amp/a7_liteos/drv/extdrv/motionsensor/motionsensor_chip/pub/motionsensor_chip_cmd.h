/******************************************************************************

Copyright (C), 2018, Hisilicon Tech. Co., Ltd.

******************************************************************************
File Name     : motionsensor_chip_cmd.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/08/01
Description   :
History       :
1.Date        : 2018/08/01
Author        : 
Modification: Created file

******************************************************************************/

#ifndef __MOTIONSENSOR_CHIP_CMD_H__
#define __MOTIONSENSOR_CHIP_CMD_H__

#include "hi_comm_motionsensor.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MSENSOR_TYPE_CHIP   12


//**********ioctl cmd************
#define MSENSOR_CMD_START            		_IO(MSENSOR_TYPE_CHIP, 0)     
#define MSENSOR_CMD_STOP       			    _IO(MSENSOR_TYPE_CHIP, 1)        
#define MSENSOR_CMD_INIT            		_IOW(MSENSOR_TYPE_CHIP, 2, MSENSOR_PARAM_S)
#define MSENSOR_CMD_DEINIT           		_IO(MSENSOR_TYPE_CHIP, 3)
#define MSENSOR_CMD_GET_PARAM               _IOR(MSENSOR_TYPE_CHIP, 4, MSENSOR_PARAM_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif






