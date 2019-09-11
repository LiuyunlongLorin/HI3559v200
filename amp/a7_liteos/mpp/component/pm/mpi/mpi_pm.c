/******************************************************************************
 Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : mpi_pm.c
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/09/17
Last Modified :
Description   :
Function List :
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdlib.h>
#include "hi_type.h"
#include "hi_common.h"
#include "hi_debug.h"

#include "hi_comm_video.h"
#include "hi_comm_pm.h"
#include "mkp_pm.h"
#include "mpi_pm.h"
#include "mpi_sys.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

static HI_S32 g_s32PmMpiFd = -1;              /* device handle */
static const char* g_pszDevName = "/dev/pm";



static pthread_mutex_t s_PmMutex = PTHREAD_MUTEX_INITIALIZER;

#define PM_MUTEX_LOCK() \
    do { \
        (void)pthread_mutex_lock(&s_PmMutex); \
    }while(0)

#define PM_MUTEX_UNLOCK() \
    do { \
        (void)pthread_mutex_unlock(&s_PmMutex); \
    }while(0)

#define CHECK_PM_OPEN()\
    do{\
        HI_S32 s32Ret;\
        s32Ret = MPI_PM_CheckOpen();\
        if(HI_SUCCESS != s32Ret)\
        {\
            return s32Ret;\
        }\
    } while (0)


#define MPI_CHECK_NULL_PTR(ptr)\
    do{\
        HI_S32 s32Ret;\
        s32Ret = MPI_PM_CheckNullPtr((HI_VOID*)ptr);\
        if(HI_SUCCESS != s32Ret)\
        {\
            return s32Ret;\
        }\
    }while(0)

static HI_S32 MPI_PM_CheckOpen(HI_VOID)
{
    PM_MUTEX_LOCK();
    if (-1 == g_s32PmMpiFd) {
        g_s32PmMpiFd = open(g_pszDevName, O_RDWR, 0);
        if (-1 == g_s32PmMpiFd) {
            PM_MUTEX_UNLOCK();
            printf("func %s open '%s' failed! \n", __FUNCTION__, g_pszDevName);
            return HI_ERR_PM_SYS_NOTREADY;
        }
    }
    PM_MUTEX_UNLOCK();
    return HI_SUCCESS;
}

static HI_S32 MPI_PM_CheckNullPtr(HI_VOID* ptr)
{
    if (NULL == ptr) {
        printf("func %s NULL pointer detected\n", __FUNCTION__);
        return HI_ERR_PM_NULL_PTR;
    }
    return HI_SUCCESS;
}

//==============EXT interface ===========

/*****************************************************************************
 Prototype    : HI_MPI_PM_SetSingleMediaParam
 Description  : set media obj frequency
 Input        : PM_HANDLE  hHandle
 Output       : None
 Return Value :
 Calls        :
 Called By    :
</$>
  History        :
  1.Date         : 2018/09/17
    Author       :
    Modification : Created function
<$/>
*****************************************************************************/
HI_S32 HI_MPI_PM_SetMediaParam(HI_MPI_PM_MEDIA_CFG_S* pstPmParam)
{
    CHECK_PM_OPEN();
    MPI_CHECK_NULL_PTR(pstPmParam);
    return ioctl(g_s32PmMpiFd, PM_SET_MEDIA_PARAM, pstPmParam);
}

/*****************************************************************************
 Prototype    : HI_MPI_PM_GetMediaParam
 Description  : Add a task to a pm job
 Input        : PM_HANDLE  hHandle
 Output       : None
 Return Value :
 Calls        :
 Called By    :
</$>
  History        :
  1.Date         : 2018/09/17
    Author       :
    Modification : Created function
<$/>
*****************************************************************************/
HI_S32 HI_MPI_PM_GetMediaParam(HI_MPI_PM_MEDIA_CFG_S* pstPmParam)
{
    CHECK_PM_OPEN();
    MPI_CHECK_NULL_PTR(pstPmParam);
    return ioctl(g_s32PmMpiFd, PM_GET_MEDIA_PARAM, pstPmParam);
}

/*****************************************************************************
 Prototype    : HI_MPI_PM_SetSingleMediaParam
 Description  : Add a task to a pm job
 Input        : PM_HANDLE  hHandle
 Output       : None
 Return Value :
 Calls        :
 Called By    :
</$>
  History        :
  1.Date         : 2018/09/17
    Author       :
    Modification : Created function
<$/>
*****************************************************************************/
HI_S32 HI_MPI_PM_SetSingleMediaParam(HI_MPI_PM_SIGLE_MEDIA_CFG_S* pstPmsinleParam)
{
    CHECK_PM_OPEN();
    MPI_CHECK_NULL_PTR(pstPmsinleParam);
    return ioctl(g_s32PmMpiFd, PM_SET_SINGLE_MEDIA_PARAM, pstPmsinleParam);
}

/*****************************************************************************
 Prototype    : HI_MPI_PM_GetTemperature
 Description  : Get temperature form PM
 Input        : None
 Output       : temperature
 Return Value :
 Calls        :
 Called By    :
</$>
  History        :
  1.Date         : 2018/09/17
    Author       :
    Modification : Created function
<$/>
*****************************************************************************/

HI_S32 HI_MPI_PM_GetTemperature(HI_S32 * ps32PmTempe)
{
    CHECK_PM_OPEN();
    MPI_CHECK_NULL_PTR(ps32PmTempe);
    return ioctl(g_s32PmMpiFd, PM_GET_TEMPERATURE, ps32PmTempe);
}
/*****************************************************************************
 Prototype    : HI_MPI_PM_SetSingleMediaParam
 Description  : set media obj frequency
 Input        : PM_HANDLE  hHandle
 Output       : None
 Return Value :
 Calls        :
 Called By    :
</$>
  History        :
  1.Date         : 2018/09/17
    Author       :
    Modification : Created function
<$/>
*****************************************************************************/
HI_S32 HI_MPI_PM_EnableSvpAccelerator(HI_BOOL bEnable)
{
    CHECK_PM_OPEN();
    int enable = (int)bEnable;
    return ioctl(g_s32PmMpiFd, PM_SVP_ENABLE_CTRL, enable);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
