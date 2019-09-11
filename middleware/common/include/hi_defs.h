/**
  * Copyright (C), 2016-2020, Hisilicon Tech. Co., Ltd.
  * All rights reserved.
  *
  * @file        hi_defs.h
  * @brief      common define.
  * @author   HiMobileCam middleware develop team
  * @date      2016.06.29
  */


#ifndef __HI_DEFS_H__
#define __HI_DEFS_H__

#include <time.h>
#include "hi_mw_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef __GNUC__
#define __asm__    asm
#define __inline__ inline
#endif

#define DO_NOTHING

#if HI_OS_TYPE == HI_OS_WIN32
    #ifndef INLINE
        #define INLINE __inline
    #endif
#elif HI_OS_TYPE == HI_OS_LINUX
#define INLINE inline
#endif

/*use for parameter INPUT, *DO NOT Modify the value* */
#define IN
/* use for parameter OUTPUT, the value maybe change when return from the function
 * the init value is ingore in the function.*/
#define OUT
/*use for parameter INPUT and OUTPUT*/
#define IO

/* --------------------------------  */
#ifndef EXTERN
#define EXTERN extern
#endif

#define STATIC static

#define LOCALVAR static
#define GLOBALVAR extern


/**for declaring global variable*/
#define DECLARE_GLOBALVAR

/**for using global variable*/
#define USE_GLOBALVAR extern


#define LOCALFUNC    static
#define EXTERNFUNC   extern

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

/**conculate the aligned start address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_START(v,a) (((v)) & (~((a)-1)))

/**conculate the aligned end address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_END(v,a) (((v) & ~((a)-1)) + ((a)-1) )

/**conculate the aligned next address,according to base address and align byte counts
 * eg. 4 byte aligned,0x80001232 ALIGN_START(0x80001232,4) = 0x80001230
 */
#define ALIGN_NEXT(v,a) ((((v) + ((a)-1)) & (~((a)-1))))

#define ALIGN_LENGTH(l, a) ALIGN_NEXT(l, a)

#define ALIGNTYPE_1BYTE  1
/*zsp*/
#define ALIGNTYPE_2BYTE  2
/*x86... default*/
#define ALIGNTYPE_4BYTE  4

#define ALIGNTYPE_8BYTE  8
/*1 Page*/
#define ALIGNTYPE_4K     4096

#define ALIGNTYPE_ZSP    ALIGNTYPE_2BYTE

#define ALIGNTYPE_VIDEO  ALIGNTYPE_8BYTE

#define PACK_ONE_BYTE  __attribute__((__packed__))

#define MW_UNUSED(x) (void)(x)

/**middleware module id*/
typedef enum hiAPPMOD_ID_E
{
    HI_APPID_REC = 0x0B,                    /**< recoder */
    HI_APPID_SNAP = 0x0C,                   /**< snap */
    HI_APPID_FILEMGR = 0x16,                /**< file manager */
    HI_APPID_STORAGE = 0x18,                /**< storage*/
    HI_APPID_LOG = 0x1F,                    /**< log*/
    HI_APPID_LOCALPLAYER = 0x20,            /**< local player*/
    HI_APPID_RTSPSERVER = 0x22,             /**<rtsp server*/
    HI_APPID_HTTPSERVER = 0x24,             /**<http server*/
    HI_APPID_MBUF = 0x25,                   /**< mbuffer manager */
    HI_APPID_LIVESTREAM = 0x26,             /**< livestream*/
    HI_APPID_RTSPCLIENT = 0x27,             /**< rtsp client */
    HI_APPID_DEMUXER = 0x28,                /**< demuxer */
    HI_APPID_MP4 = 0x29,                    /**< demuxer */
    HI_APPID_DTCF = 0x2A,                   /**< dtcf */
    HI_APPID_TIMER = 0x2C,                  /**< hitimer*/
    HI_APPID_XMP = 0x2B,                    /**< xmp */
    HI_APPID_DCF = 0x2D,                    /**<dcf*/
    HI_APPID_DCF_WRAPPER = 0x2E,            /**<dcf_wrapper*/
    HI_APPID_TS =  0x2F,                    /**< ts */
    HI_APPID_MSGHDL =  0x30,                /**< message handler */
    HI_APPID_HFSM = 0x31,                   /**< Hierarchical state machine */
    HI_APPID_EVTHUB = 0x32,                 /**< event hub */
    HI_APPID_TAG_EDITOR =  0x36,                    /**< tag editro */
    HI_APPID_PICDEMUXER =  0x37,                    /**< pic demuxer */
    HI_APPID_DNG =  0x38,                    /**< dng module */
    HI_APPID_OSCSERVER = 0X3F,              /**< osc server*/
    HI_APPID_NETATE    = 0x40,               /**netate*/
    HI_APPID_BUTT = 0xFF
} APPMOD_ID_E;

#define hi_mw_usleep(usec) \
do { \
    struct timespec req; \
    req.tv_sec  = (usec) / 1000000; \
    req.tv_nsec = ((usec) % 1000000) * 1000; \
    nanosleep (&req, NULL); \
} while (0)

static __inline HI_U64 GetTickCount(HI_VOID)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((HI_U64)ts.tv_sec * 1000L + ts.tv_nsec / 1000000);
}

static __inline HI_VOID Get_waitTimespec(struct timespec *pOuttime, HI_U32 uTimeMs)
{
    clock_gettime(CLOCK_MONOTONIC, pOuttime);
    pOuttime->tv_sec += uTimeMs/1000;
    pOuttime->tv_nsec += (uTimeMs%1000) * (1000 * 1000);
    if (pOuttime->tv_nsec >= (1000 * 1000 * 1000))
    {
        pOuttime->tv_sec += 1;
        pOuttime->tv_nsec = pOuttime->tv_nsec % (1000 * 1000 * 1000);
    }
}

/**
 * @brief Enable proc and log
 * @param[in] HI_BOOL bDebug: enable proc and log if HI_TRUE, otherwise disable the two module.
 *          Proc is debug by default. Log isn't debug(only show error log) by default.
 * @return HI_SUCCESS Enable/Disable debug success.
 * @return HI_FAILURE Enable/Disable debug fail.
 */
HI_S32 HI_MW_EnableDebug(HI_BOOL bDebug);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __HI_DEFS_H__ */
