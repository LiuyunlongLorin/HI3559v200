/**
* @file    hi_hal_touchpad.c
* @brief   hal touchapd implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/23
* @version   1.0

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <sys/wait.h>

#include "hi_appcomm_log.h"
#include "hi_hal_touchpad.h"
#include "hi_hal_common.h"
#include "hi_hal_common_inner.h"
#include "hi_appcomm_util.h"


#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

/*  touchpad init state */
static HI_BOOL s_bTOUCHPADInitState = HI_FALSE;
static HI_S32 s_s32TOUCHPADfd = HAL_FD_INITIALIZATION_VAL;
static HI_BOOL s_bHALTPUCHPADEnable = HI_TRUE; /**< touchpad enable flag */

/* check touchpad init or not */
#define HAL_TOUCHPAD_CHECK_INIT() \
    do { \
        if (!s_bTOUCHPADInitState){ \
            MLOGD("TouchPad has not been inited\n"); \
            return HI_HAL_ENOINIT;  \
        }   \
    } while(0)

#define HAL_TOUCHPAD_CHECK_FD() \
    do { \
        if (HAL_FD_INITIALIZATION_VAL == s_s32TOUCHPADfd){ \
            MLOGD("TouchPad has not been started\n"); \
            return HI_HAL_ENOINIT;  \
        }   \
    } while(0)

/** macro define */
#define HAL_TOUCHPAD_KO_PATH        HI_APPFS_KOMOD_PATH"/gt_911.ko"
#define HAL_TOUCHAPD_DEV           ("/dev/input/event0")
#define HAL_TOUCHAPD_MAX_POINT_NUM (1)
#define HAL_TOUCHAPD_BITLONG       (32)
#define HAL_TOUCHAPD_BITWORD(a) ((a) / HAL_TOUCHAPD_BITLONG)

/** multi touch info */
typedef struct tagHAL_TOUCHPAD_MTSAMPLE_S
{
    HI_S32 s32ID;
    HI_S32 s32X;
    HI_S32 s32Y;
    HI_S32 u32Pressure;
    struct timeval tv;
} HAL_TOUCHPAD_MTSAMPLE_S;

typedef struct tagHAL_TOUCHPAD_MTINPUT_S
{
    HI_S32 as32Filled[HAL_TOUCHAPD_MAX_POINT_NUM];/**< MARK filled flag,array index is id */
    HI_S32 as32X[HAL_TOUCHAPD_MAX_POINT_NUM];
    HI_S32 as32Y[HAL_TOUCHAPD_MAX_POINT_NUM];
    HI_S32 au32Pressure[HAL_TOUCHAPD_MAX_POINT_NUM];
    HI_S32 s32Slot;/**< represent input id */
} HAL_TOUCHPAD_MTINPUT_S;

static  HAL_TOUCHPAD_MTINPUT_S s_stHALTOUCHPADMtInput;

static HI_VOID HAL_TOUCHPAD_PinoutInit(HI_VOID)
{
#if defined(BOARD_DASHCAM_REFB) && defined(HI3559V200)
    /**I2C2 PIN REUSE*/
    himm(0x111F0018, 0x5f1);
    himm(0x111F001C, 0x5f1);

    /**int gpio*/
    himm(0x112F0040, 0x4f4);
#endif
    MLOGD("Set Pin Reuse OK.\n\n");
}

static inline HI_S32 HAL_TOUCHPAD_InputBitCheck(HI_S32 bit, const volatile HI_U32* addr)
{
    return 1UL & (addr[HAL_TOUCHAPD_BITWORD(bit)] >> (bit & (HAL_TOUCHAPD_BITLONG - 1)));
}

static HI_S32 HAL_TOUCHPAD_PushSamp(HAL_TOUCHPAD_MTINPUT_S* pstMtInput, struct input_event* pstInputEvent, HAL_TOUCHPAD_MTSAMPLE_S* pstMtSamp)
{
    HI_S32 s32Num = 0;
    HI_S32 s32Slot = 0;

    for (s32Slot = 0; s32Slot < HAL_TOUCHAPD_MAX_POINT_NUM; s32Slot++)
    {
        if (!pstMtInput->as32Filled[s32Slot])
        {
            continue;/**<s32Slot index has no upadte data*/
        }

        pstMtSamp[s32Num].s32X = pstMtInput->as32X[s32Slot];
        pstMtSamp[s32Num].s32Y = pstMtInput->as32Y[s32Slot];
        pstMtSamp[s32Num].u32Pressure = pstMtInput->au32Pressure[s32Slot];
        pstMtSamp[s32Num].tv = pstInputEvent->time;
        pstMtSamp[s32Num].s32ID = s32Slot;

        pstMtInput->as32Filled[s32Slot] = 0;
        s32Num++;
    }
    return s32Num;
}

static HI_S32 HAL_TOUCHPAD_ReadInputEventStatic(HAL_TOUCHPAD_MTSAMPLE_S* pstMtSamp)
{
    struct input_event stInputEvent;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bDataReadCompleted = HI_FALSE;
    HI_S32 s32Pushed = 0;

    /** parm ps32Level check */
    if (NULL == pstMtSamp)
    {
        MLOGE("pstMtSamp is null\n");
        return HI_FAILURE;
    }
    /** init check */
    if (HAL_FD_INITIALIZATION_VAL == s_s32TOUCHPADfd)
    {
        MLOGE("touchpad not initialized\n");
        return HI_FAILURE;
    }
    while (HI_FALSE == bDataReadCompleted)
    {
        s32Ret = read(s_s32TOUCHPADfd, &stInputEvent, sizeof(struct input_event));

        if (s32Ret < (HI_S32)sizeof(struct input_event))
        {
            /** no more data */
            MLOGW("s32Ret(%d) <sizeof(struct input_event)(%d)\n",(HI_S32)s32Ret,(HI_S32)sizeof(struct input_event));
            break;
        }

        switch (stInputEvent.type)
        {
                /** key 0x1*/
            case EV_KEY:

                //MLOGD("event=EV_KEY:\n");
                switch (stInputEvent.code)
                {
                        /** 14a 0 */
                    case BTN_TOUCH:
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;
                }

                break;

                /** 0x00 */
            case EV_SYN:
                switch (stInputEvent.code)
                {
                    case SYN_REPORT:
                    case SYN_MT_REPORT:
                    default:
                        //MLOGD("event=%d.code=%d\n", stInputEvent.type, stInputEvent.code);
                    {
                        s32Pushed = HAL_TOUCHPAD_PushSamp(&s_stHALTOUCHPADMtInput, &stInputEvent, pstMtSamp);
                        pstMtSamp += s32Pushed;
                        bDataReadCompleted = HI_TRUE;
                    }
                    break;
                }

                break;

                /** 0x3 */
            case EV_ABS:
                switch (stInputEvent.code)
                {
                        /** 0x3a Pressure on contact area */
                    case ABS_PRESSURE:
                        //MLOGD("event.code=ABS_PRESSURE.v=%d\n", stInputEvent.value);
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        break;

                        /** 0x2f MT slot being modified */
                    case ABS_MT_SLOT:

                        //MLOGD("event.code=ABS_MT_SLOT.v=%d\n", stInputEvent.value);
                        if (stInputEvent.value < 0)
                        {
                            break;
                        }

                        (&s_stHALTOUCHPADMtInput)->s32Slot = stInputEvent.value;
                        if ((&s_stHALTOUCHPADMtInput)->s32Slot >= HAL_TOUCHAPD_MAX_POINT_NUM)
                        {
                            //MLOGD("slot limit error. MAX_POINT_NUM=%d. temp slot=%d\n", MAX_POINT_NUM, (&s_stmtinput)->temp_slot);
                            (&s_stHALTOUCHPADMtInput)->s32Slot = 0;
                        }
                        break;

                        /** 0X30 Major axis of touching ellipse */
                    case ABS_MT_TOUCH_MAJOR:
                        //MLOGD("event.code=ABS_MT_TOUCH_MAJOR.v=%d\n", stInputEvent.value);
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;

                        /** 0X35 */
                    case ABS_MT_POSITION_X:
                        //MLOGD("event.code=ABS_MT_POSITION_X.v=%d\n", stInputEvent.value);
                        (&s_stHALTOUCHPADMtInput)->as32X[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;

                        break;

                        /** 0X36 */
                    case ABS_MT_POSITION_Y:
                        //MLOGD("event.code=ABS_MT_POSITION_Y.v=%d\n", stInputEvent.value);
                        (&s_stHALTOUCHPADMtInput)->as32Y[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;

                        /** 0X39 */
                    case ABS_MT_TRACKING_ID:

                        //MLOGD("event.code=ABS_MT_TRACKING_ID.v=%d\n", stInputEvent.value);
                        if (stInputEvent.value == -1)
                        {
                            (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        }
                        break;
                }

                break;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_Init(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (HI_FALSE == s_bTOUCHPADInitState)
    {
        HAL_TOUCHPAD_PinoutInit();
        /** insmod touchpad driver */
        s32Ret = HI_insmod(HAL_TOUCHPAD_KO_PATH,NULL);
        if(0 != s32Ret)
        {
            MLOGE("insmod touchpad:failed, errno(%d)\n", errno);
            return HI_HAL_EINVOKESYS;
        }
        s_bTOUCHPADInitState = HI_TRUE;
    }
    else
    {
        MLOGE("touchapd already init\n");
        return HI_HAL_EINITIALIZED;
    }
    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_SetRotate( HI_HAL_TOUCHPAD_ROTATE_E enRotate)
{
    HAL_TOUCHPAD_CHECK_INIT();
    MLOGE("[Errot] Set Rotate not support at present\n");
    return HI_FAILURE;
}

HI_S32 HI_HAL_TOUCHPAD_Suspend(HI_VOID)
{
    HAL_TOUCHPAD_CHECK_INIT();
    s_bHALTPUCHPADEnable = HI_FALSE;
    MLOGD("touchpad suspend\n");
    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_Resume(HI_VOID)
{
    HAL_TOUCHPAD_CHECK_INIT();
    s_bHALTPUCHPADEnable = HI_TRUE;
    MLOGD("touchpad resumme\n");
    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_Start(HI_S32* ps32Fd)
{
    HAL_TOUCHPAD_CHECK_INIT();
    HI_APPCOMM_CHECK_POINTER(ps32Fd, HI_HAL_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 au32Inputbits[(ABS_MAX + 31) / 32];
    if (HAL_FD_INITIALIZATION_VAL != s_s32TOUCHPADfd)
    {
        MLOGE("touchpad already started\n");
        return HI_HAL_ENOSTART;
    }
    memset(&s_stHALTOUCHPADMtInput, 0, sizeof(HAL_TOUCHPAD_MTINPUT_S));
    s_s32TOUCHPADfd = open(HAL_TOUCHAPD_DEV, O_RDONLY);
    if (s_s32TOUCHPADfd < 0)
    {
        MLOGE("open /dev/input/event0 err,errno(%d)\n",errno);
        return HI_HAL_EINVOKESYS;
    }

    s32Ret = ioctl(s_s32TOUCHPADfd, EVIOCGBIT(EV_ABS, sizeof(au32Inputbits)), au32Inputbits);
    if (s32Ret < 0)
    {
        MLOGE("open ioctl err,errno(%d)\n",errno);
        close(s_s32TOUCHPADfd);
        s_s32TOUCHPADfd = HAL_FD_INITIALIZATION_VAL;
        return HI_HAL_EINVOKESYS;
    }

    if ( ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_X, au32Inputbits) )
         || ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_Y, au32Inputbits) )
         || ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_TOUCH_MAJOR, au32Inputbits) )
       )
    {
        MLOGE("error: could not support the device\n");
        MLOGE("EV_SYN=%d\n", HAL_TOUCHPAD_InputBitCheck(EV_SYN, au32Inputbits));
        MLOGE("EV_ABS=%d\n", HAL_TOUCHPAD_InputBitCheck(EV_ABS, au32Inputbits));
        MLOGE("ABS_MT_POSITION_X=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_X, au32Inputbits));
        MLOGE("ABS_MT_POSITION_Y=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_Y, au32Inputbits));
        MLOGE("ABS_MT_TOUCH_MAJOR=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_TOUCH_MAJOR, au32Inputbits));
        close(s_s32TOUCHPADfd);
        s_s32TOUCHPADfd = HAL_FD_INITIALIZATION_VAL;
        return HI_HAL_EINTER;
    }
    *ps32Fd = s_s32TOUCHPADfd;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_Stop(HI_VOID)
{
    HAL_TOUCHPAD_CHECK_INIT();
    HAL_TOUCHPAD_CHECK_FD();

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = close(s_s32TOUCHPADfd);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGD("close err,errno(%d)\n",errno);
        return HI_HAL_EINVOKESYS;
    }
    s_s32TOUCHPADfd = HAL_FD_INITIALIZATION_VAL;
    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_ReadInputEvent(HI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData)
{
    HI_APPCOMM_CHECK_POINTER(pstInputData, HI_HAL_EINVAL);
    HAL_TOUCHPAD_CHECK_INIT();
    HAL_TOUCHPAD_CHECK_FD();

    HI_S32 s32Ret = HI_SUCCESS;

    HAL_TOUCHPAD_MTSAMPLE_S astMtSample[HAL_TOUCHAPD_MAX_POINT_NUM];

    memset(&astMtSample,'\0',sizeof(astMtSample));
    s32Ret = HAL_TOUCHPAD_ReadInputEventStatic(astMtSample);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGD("read_input_event err\n");
        return HI_HAL_EINTER;
    }

    if (HI_TRUE == s_bHALTPUCHPADEnable)
    {
        pstInputData->s32ID = astMtSample[0].s32ID;
        pstInputData->u32Pressure = astMtSample[0].u32Pressure;
        pstInputData->s32X = 400 - astMtSample[0].s32Y;
        pstInputData->s32Y = astMtSample[0].s32X;
        pstInputData->u32TimeStamp = (astMtSample[0].tv.tv_sec) * 1000 + (astMtSample[0].tv.tv_usec) / 1000;
    }
    else
    {
        pstInputData->s32ID = 0;
        pstInputData->u32Pressure = 0;
        pstInputData->s32X = 0;
        pstInputData->s32Y = 0;
        pstInputData->u32TimeStamp = 0;
    }

    return HI_SUCCESS;
}

HI_S32 HI_HAL_TOUCHPAD_Deinit(HI_VOID)
{
    HAL_TOUCHPAD_CHECK_INIT();
    if (HAL_FD_INITIALIZATION_VAL != s_s32TOUCHPADfd)
    {
        MLOGE("touchpad need stop first\n");
        return HI_HAL_ENOSTART;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    /** Deinitial touchpad Device */
    s32Ret = HI_rmmod(HAL_TOUCHPAD_KO_PATH);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("rmmod touchpad:failed, errno(%d)\n", s32Ret);
        return HI_HAL_EINVOKESYS;
    }
    s_bTOUCHPADInitState = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* __cplusplus */
