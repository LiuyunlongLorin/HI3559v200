/******************************************************************************

Copyright (C), 2018, Hisilicon Tech. Co., Ltd.

******************************************************************************
File Name     : motionsensor_mng_cmd.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/08/01
Description   :
History       :
1.Date        : 2018/08/01
Author        : 
Modification: Created file

******************************************************************************/

#ifndef __MOTIONSENSOR_MNG_CMD_H__
#define __MOTIONSENSOR_MNG_CMD_H__

#include "hi_comm_motionsensor.h"



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define MSENSOR_TYPE_MNG    10


#define MSENSOR_CMD_GET_DATA          _IOWR(MSENSOR_TYPE_MNG, 1,  MSENSOR_DATA_INFO_S)
#define MSENSOR_CMD_RELEASE_BUF       _IOWR(MSENSOR_TYPE_MNG, 2,  MSENSOR_DATA_INFO_S)
#define MSENSOR_CMD_ADD_USER          _IOWR(MSENSOR_TYPE_MNG, 3,  HI_S32)
#define MSENSOR_CMD_DELETE_USER       _IOWR(MSENSOR_TYPE_MNG, 4,  HI_S32)
#define MSENSOR_CMD_SEND_DATA         _IOW(MSENSOR_TYPE_MNG, 5,   MSENSOR_DATA_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
