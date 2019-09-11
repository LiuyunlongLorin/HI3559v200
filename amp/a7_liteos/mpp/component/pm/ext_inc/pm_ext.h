/******************************************************************************

  Copyright (C), 2018, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vi_ext.h
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       :
  Description   :
  History       :
  1.Date        : 2018/09/17
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __PM_EXT_H__
#define __PM_EXT_H__

#include "hi_type.h"
#include "hi_common.h"
//#include "hi_comm_isp.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */



#define HI_TRACE_PM(level, fmt, ...)\
    do{ \
        HI_TRACE(level, HI_ID_PM,"[Func]:%s [Line]:%d [Info]:"fmt,__FUNCTION__, __LINE__,##__VA_ARGS__);\
    }while(0)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

