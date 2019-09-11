/******************************************************************************

  Copyright (C), 2011-2021, Hisilicon Tech. Co., Ltd.

 ******************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include "vdec_exp.h"
#include "vou_exp.h"


#define VDEC_NOTIFY   0
#ifdef __HuaweiLite__
#define VDEC_SET_SCHEDULER 1
#else
#define VDEC_SET_SCHEDULER 0
#endif
#define VO_NOTIFY     0



/*********************************************************************
 * Set decoding thread Priority. Only support in linux
 *
 * @param[in] void.
 * @return int.
 *
 *************************************************************************/
int hi_sched_setscheduler_dec(void)
{
#ifndef __HuaweiLite__
    struct sched_param param;
    param.sched_priority = 99;

    return sched_setscheduler(current, SCHED_FIFO, &param);
#else
    int sched_priority = 7;
    return LOS_CurTaskPriSet(sched_priority);
#endif
}

/*********************************************************************
 * Set decoding thread Priority. Only support in liteos
 *
 * @param[in] void.
 * @return int.
 *
 *************************************************************************/
int hi_sched_setscheduler_stm(void)
{
#ifdef __HuaweiLite__
    int sched_priority = 7;
    return LOS_CurTaskPriSet(sched_priority);
#else
    return 0;
#endif
}

/*********************************************************************
 * Set decoding thread Priority. Only support in liteos
 *
 * @param[in] void.
 * @return int.
 *
 *************************************************************************/
int hi_sched_setscheduler_syn(void)
{
#ifdef __HuaweiLite__
    int sched_priority = 7;
    return LOS_CurTaskPriSet(sched_priority);
#else
    return 0;
#endif
}

/*********************************************************************
 * event notification from Hisilicon SDK.
 *
 * @param[in] module_id     module id. (MOD_ID_E)
 * @param[in] channel       device channel number.
 * @param[in] event_type    event type.
 *   ex) types of VDEC event
 *         - VDEC_EVNT_STREAM_ERR = 1,
 *         - VDEC_EVNT_UNSUPPORT,
 *         - VDEC_EVNT_OVER_REFTHR,
 *         - VDEC_EVNT_REF_NUM_OVER,
 *         - VDEC_EVNT_SLICE_NUM_OVER,
 *         - VDEC_EVNT_SPS_NUM_OVER,
 *         - VDEC_EVNT_PPS_NUM_OVER,
 *         - VDEC_EVNT_PICBUF_SIZE_ERR,
 *         - VDEC_EVNT_SIZE_OVER,
 *         - VDEC_EVNT_IMG_SIZE_CHANGE,
 *         - VDEC_EVNT_BUTT
 * @param[in] args          additional arguments.
 * @return none.
 *
 *************************************************************************/
void hi_user_notify_vdec_event(int module_id, int channel, int event_type, void* args)
{
    //int *pArgs = (int *)args;

    switch(event_type)
    {
        /* VDEC_EVNT_STREAM_ERR: stream package error. */
        case 1:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_UNSUPPORT: unsupport the stream specification. */
        case 2:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_OVER_REFTHR: stream error rate is larger than s32ChanErrThr. */
        case 3:
            {
                /* pArgs[0] is the stream error rate of current frame. */
                break;
            }

        /* VDEC_EVNT_REF_NUM_OVER: reference num is not enough. */
        case 4:
            {
                /* pArgs[0] is reference num of current frame. */
                break;
            }

        /* VDEC_EVNT_SLICE_NUM_OVER: slice num is not enough. */
        case 5:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_SPS_NUM_OVER: sps num is not enough. */
        case 6:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_PPS_NUM_OVER: pps num is not enough. */
        case 7:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_PICBUF_SIZE_ERR: the picture buffer size is not enough. */
        case 8:
            {
                /* pArgs is null. */
                break;
            }

        /* VDEC_EVNT_SIZE_OVER: picture width or height is larger than chnnel width or height */
        case 9:
            {
                /* pArgs[0] is width, pArgs[1] is height of current frame. */
                break;
            }

        /* VDEC_EVNT_IMG_SIZE_CHANGE: the width or height of stream change */
        case 10:
            {
                /* pArgs[0] is width, pArgs[1] is height of current frame. */
                break;
            }
        default:
            {
                break;
            }
    }
}


void hi_user_notify_vo_event(int module_id, int vodev)
{
    switch ( vodev )
    {
        case 0 :
        {
        /* do something */
            break;
        }
        default:
        {
            break;
        }
    }
}


extern VDEC_EXPORT_SYMBOL_S  g_stVdecExpSymbol;
extern VOU_EXPORT_SYMBOL_S   g_stVouExpSymbol;

int __init hi_user_init(void)
{
#if (VDEC_NOTIFY || VDEC_SET_SCHEDULER)
    {
        VDEC_EXPORT_CALLBACK_S stVdecExpCallback;

        memset(&stVdecExpCallback, 0, sizeof(VDEC_EXPORT_CALLBACK_S));

        if(NULL != g_stVdecExpSymbol.pfnVdecRegisterExpCallback)
        {
            #if VDEC_NOTIFY
            stVdecExpCallback.pfnVdecNotify = hi_user_notify_vdec_event;
            #endif

            #if VDEC_SET_SCHEDULER
            stVdecExpCallback.pfnVdecSetscheduler_Dec = hi_sched_setscheduler_dec;
            stVdecExpCallback.pfnVdecSetscheduler_Stm = hi_sched_setscheduler_stm;
            stVdecExpCallback.pfnVdecSetscheduler_Syn = hi_sched_setscheduler_syn;
            #endif

            g_stVdecExpSymbol.pfnVdecRegisterExpCallback(&stVdecExpCallback);
        }
    }
#endif

#if VO_NOTIFY
    {
        VOU_EXPORT_CALLBACK_S stVoExpCallback;
        memset(&stVoExpCallback, 0, sizeof(VOU_EXPORT_CALLBACK_S));
        if(NULL != g_stVouExpSymbol.pfnVoRegisterExpCallback)
        {
            stVoExpCallback.pfnVoNotify = hi_user_notify_vo_event;
            g_stVouExpSymbol.pfnVoRegisterExpCallback(&stVoExpCallback);
        }
    }
#endif

    return 0;
}

void __exit hi_user_exit(void)
{
#if (VDEC_NOTIFY || VDEC_SET_SCHEDULER)
    {
        VDEC_EXPORT_CALLBACK_S stVdecExpCallback;

        if(NULL != g_stVdecExpSymbol.pfnVdecRegisterExpCallback)
        {
            memset(&stVdecExpCallback, 0, sizeof(VDEC_EXPORT_CALLBACK_S));
            g_stVdecExpSymbol.pfnVdecRegisterExpCallback(&stVdecExpCallback);
        }
    }
#endif

#if VO_NOTIFY
    {
        VOU_EXPORT_CALLBACK_S stVoExpCallback;

        if(NULL != g_stVouExpSymbol.pfnVoRegisterExpCallback)
        {
            memset(&stVoExpCallback, 0, sizeof(VOU_EXPORT_CALLBACK_S));
            g_stVouExpSymbol.pfnVoRegisterExpCallback(&stVoExpCallback);
        }
    }
#endif

    return;
}

#ifndef __HuaweiLite__
module_init(hi_user_init);
module_exit(hi_user_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");

