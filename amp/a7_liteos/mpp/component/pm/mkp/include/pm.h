/******************************************************************************
 Copyright (C), 2018, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : pm.h
Version       : Initial Draft
Author        : Hisilicon multimedia software group
Created       : 2018/09/17
Last Modified :
Description   :
Function List :
******************************************************************************/
#ifndef __PM_H__
#define __PM_H__

#include "hi_osal.h"

#include "vb_ext.h"
#include "hi_defines.h"
#include "hi_comm_pm.h"
//#include "hiisp_dis_local_gyro_fw.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*debug Need*/
#define NEED_MEM_INFO 0

#define CALL_DEV_NUM        16
#define CALL_MODE_NUM      3

#if 0
extern  osal_spinlock_t     g_stPmLock;
#define PM_SPIN_LOCK       VALG_Lock(&g_stPmLock, &(flags))
#define PM_SPIN_UNLOCK     VALG_UnLock(&g_stPmLock, &(flags))
#endif

#define PM_DOWN_SEM()\
    do{\
        if (osal_down_interruptible(&g_stPmSem))\
            return -ERESTARTSYS;\
    }while(0)

#define PM_UP_SEM()    osal_up(&g_stPmSem)


#define PM_MIN(a, b)         (((a) > (b)) ? (b) : (a))

#define PM_TIME_DIFF(timeEnd, timeBegin) (HI_U32)((timeEnd.tv_sec - timeBegin.tv_sec)*1000000LLU + timeEnd.tv_usec - timeBegin.tv_usec)

#define PM_do_gettimeofday(ptime)  \
    do{\
        osal_gettimeofday(ptime);\
    }while(0)\

#define CHECK_NULL_PTR(ptr)\
    do{\
        HI_S32 s32Ret;\
        s32Ret = PM_CheckNullPtr((HI_VOID*)ptr);\
        if(HI_SUCCESS != s32Ret)\
        {\
            return s32Ret;\
        }\
    }while(0)

#define PM_CHECK_MOD_STARTED()\
    do{\
        HI_S32 s32Ret;\
        s32Ret = PM_CheckModStart();\
        if(HI_SUCCESS != s32Ret)\
        {\
            return s32Ret;\
        }\
    }while(0)


#define PM_CHECK_MODID(modid, callermodid)\
    do{\
        if(PM_MAX_CALLER_MOD_NUM <= (callermodid))\
        {\
            HI_TRACE_PM(HI_DBG_ERR,"input illegal param: mod id %d\n",(modid));\
            return HI_ERR_PM_ILLEGAL_PARAM;\
        }\
    }while(0)


typedef enum tagPM_MOD_STATE_E {
    PM_MOD_STATE_STOPPED     = 0,
    PM_MOD_STATE_STARTED     = 1,
    PM_MOD_STATE_STOPPING     = 2,

    PM_MOD_TATE_BUTT
} PM_MOD_STATE_E;

typedef struct tagPM_DBG_INFO_S {

    HI_S32 s32Reserv;
} PM_DBG_INFO_S;

/*PM moudle context */
typedef struct tagPM_CTX_S {
    HI_S32 s32DomainNum;
    HI_BOOL bPmEnable;

    PM_DBG_INFO_S s_stPmDbgInfo; /*PM debug info*/

} PM_CTX_S;


HI_S32 PM_CheckNullPtr(HI_VOID* ptr);
HI_S32 PM_CheckModStart(HI_VOID);

void PM_ModInit(void);
void PM_ModExit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*endof __PM_H__*/
