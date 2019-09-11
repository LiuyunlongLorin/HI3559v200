/**
 * @file      hi_usb.c
 * @brief     usb interface implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/19
 * @version   1.0

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "hi_uvc.h"
#include "hi_uvc_gadget.h"
#include "hi_appcomm.h"
#include "hi_appcomm_util.h"
#include "hi_usb.h"
#include "hi_uvc.h"
#include "hi_usb_storage.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** usb connect state */
typedef enum tagUSB_CONNECT_STATE_E
{
    USB_CONNECT_STATE_DISCONNECT = 0,
    USB_CONNECT_STATE_CHARGE,
    USB_CONNECT_STATE_HOST,
    USB_CONNECT_STATE_BUTT
} USB_CONNECT_STATE_E;

/** USB Context */
typedef struct tagUSB_CONTEXT_S
{
    HI_BOOL bInit;
    HI_BOOL bCheckTskRun; /**<check task run flag */
    pthread_t CheckTskId; /**<check task thread id */
    HI_USB_MODE_E  enMode;
    HI_USB_STATE_E enState;
    HI_USB_CFG_S stUsbCfg;
    pthread_mutex_t stateMutex;
} USB_CONTEXT_S;
static USB_CONTEXT_S s_stUSBCtx =
{
    .bInit = HI_FALSE,
};

static HI_VOID USB_CheckState(const HI_CHAR *pszProcFile, USB_CONNECT_STATE_E* penState)
{
#ifndef __HuaweiLite__
#define MAX_SCRLIEN (128)
    FILE* fp = NULL;
    HI_CHAR temp[MAX_SCRLIEN];
    HI_CHAR szStateString[MAX_SCRLIEN] = {0,};
    memset(temp,0x0, sizeof(HI_CHAR)*MAX_SCRLIEN);
    fp = fopen(pszProcFile, "r");
    if(fp!= NULL)
    {
        if(!feof(fp))
        {
            while(fgets(temp,(MAX_SCRLIEN-1),fp)!=NULL)
            {
               snprintf(szStateString, MAX_SCRLIEN, "%s", temp);
               break;
            }
        }
        fclose(fp);

        if(strstr(szStateString, "cnt2charger"))
        {
            *penState = USB_CONNECT_STATE_CHARGE;
        }
        else if(strstr(szStateString, "cnt2host"))
        {
            *penState = USB_CONNECT_STATE_HOST;
        }
        else
        {
            *penState = USB_CONNECT_STATE_DISCONNECT;
        }
    }
    else
    {
        MLOGE("fopen %s failed!\n", pszProcFile);
        *penState = USB_CONNECT_STATE_DISCONNECT;
    }
#else
    if (HI_USB_MODE_UVC == s_stUSBCtx.enMode)
    {
        *penState = USB_CONNECT_STATE_HOST;
        extern HI_BOOL UVC_Is_Connect(VOID);
        if (UVC_Is_Connect())
        {
            *penState = USB_CONNECT_STATE_HOST;
        }
        else
        {
            *penState = USB_CONNECT_STATE_CHARGE;
        }
    }
    else if (HI_USB_MODE_STORAGE == s_stUSBCtx.enMode)
    {
        extern HI_S32 g_fmass_status;
        if (g_fmass_status)
        {
            *penState = USB_CONNECT_STATE_HOST;
        }
        else
        {
            *penState = USB_CONNECT_STATE_DISCONNECT;
        }
    }
#endif
}

static HI_VOID USB_CheckChargeState(const HI_CHAR *pszProcFile, HI_BOOL *pbState)
{
    USB_CONNECT_STATE_E enState = USB_CONNECT_STATE_DISCONNECT;
    USB_CheckState(pszProcFile, &enState);
    if(USB_CONNECT_STATE_DISCONNECT != enState)
    {
        *pbState = HI_TRUE;
    }
    else
    {
        *pbState = HI_FALSE;
    }
}

static HI_VOID USB_CheckPcState(const HI_CHAR *pszProcFile, HI_BOOL *pbReady)
{
    USB_CONNECT_STATE_E enState = USB_CONNECT_STATE_DISCONNECT;
    USB_CheckState(pszProcFile, &enState);
    if(USB_CONNECT_STATE_HOST == enState)
    {
        *pbReady = HI_TRUE;
    }
    else
    {
        *pbReady = HI_FALSE;
    }
}

static inline const HI_CHAR * USB_GetModeStr(HI_USB_MODE_E enMode)
{
    switch(enMode)
    {
        case HI_USB_MODE_CHARGE:
            return "Charge";
        case HI_USB_MODE_UVC:
            return "UVC";
        case HI_USB_MODE_STORAGE:
            return "USBStorage";
        default:
            return "Unknown";
    }
}

static inline const HI_CHAR * USB_GetStateStr(HI_USB_STATE_E enState)
{
    switch(enState)
    {
        case HI_USB_STATE_OUT:
            return "Out";
        case HI_USB_STATE_INSERT:
            return "Insert";
        case HI_USB_STATE_UVC_READY:
            return "UVC Ready";
        case HI_USB_STATE_UVC_PC_READY:
            return "UVC PC Ready";
        case HI_USB_STATE_UVC_MEDIA_READY:
            return "UVC Media Ready";
        case HI_USB_STATE_STORAGE_READY:
            return "Storage Ready";
        case HI_USB_STATE_STORAGE_PC_READY:
            return "Storage PC Ready";
        case HI_USB_STATE_STORAGE_SD_READY:
            return "Storage SD Ready";
        default:
            return "Unknown";
    }
}

static inline HI_VOID USB_SetState(HI_USB_STATE_E enState)
{
    if (enState != s_stUSBCtx.enState)
    {
        MLOGD(YELLOW"%s -> %s"NONE"\n", USB_GetStateStr(s_stUSBCtx.enState), USB_GetStateStr(enState));
        s_stUSBCtx.enState = enState;
    }
}

static inline HI_S32 USB_EventProc(const HI_USB_EVENT_INFO_S* pstEvent)
{
    HI_APPCOMM_CHECK_POINTER(s_stUSBCtx.stUsbCfg.pfnEventProc, HI_FAILURE);
    return s_stUSBCtx.stUsbCfg.pfnEventProc(pstEvent);
}

static HI_S32 USB_UVC_StateProc(USB_CONTEXT_S* pstCtx)
{
    switch(pstCtx->enState)
    {
        case HI_USB_STATE_STORAGE_READY:    /* Storage Ready    -> UVC Ready */
        case HI_USB_STATE_STORAGE_PC_READY: /* Storage PC Ready -> UVC Ready */
        case HI_USB_STATE_STORAGE_SD_READY: /* Storage SD Ready -> UVC Ready */
            USB_STORAGE_Deinit();
            UVC_Init(&pstCtx->stUsbCfg.stUvcCfg.stDevCap, &pstCtx->stUsbCfg.stUvcCfg.stDataSource,
                &pstCtx->stUsbCfg.stUvcCfg.stBufferCfg);
            UVC_Start(pstCtx->stUsbCfg.stUvcCfg.szDevPath);
            USB_SetState(HI_USB_STATE_UVC_READY);
            break;

        case HI_USB_STATE_INSERT:           /* Insert           -> UVC Ready */
            UVC_Init(&pstCtx->stUsbCfg.stUvcCfg.stDevCap, &pstCtx->stUsbCfg.stUvcCfg.stDataSource,
                &pstCtx->stUsbCfg.stUvcCfg.stBufferCfg);
            UVC_Start(pstCtx->stUsbCfg.stUvcCfg.szDevPath);
            USB_SetState(HI_USB_STATE_UVC_READY);
            break;

        case HI_USB_STATE_UVC_READY:
        {
            /* if PC Ready, PreProcess by callback */
            //if (UVC_GetCtx()->bPCConnect)   /* UVC Ready        -> UVC PC Ready */
            HI_BOOL bPcState = HI_FALSE;
            USB_CheckPcState((const HI_CHAR *)&(pstCtx->stUsbCfg.stStorageCfg.szProcFile), &bPcState);
            if (bPcState)
            {
                USB_SetState(HI_USB_STATE_UVC_PC_READY);

                HI_USB_EVENT_INFO_S stEventInfo;
                stEventInfo.s32EventId = HI_EVENT_USB_UVC_READY;
                USB_EventProc(&stEventInfo);/* UVC PC Ready     -> UVC Media Ready */
                USB_SetState(HI_USB_STATE_UVC_MEDIA_READY);

            }
            break;
        }
        default:
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 USB_STORAGE_StateProc(USB_CONTEXT_S* pstCtx)
{
    switch(pstCtx->enState)
    {
        case HI_USB_STATE_UVC_MEDIA_READY:     /* UVC Media Ready -> Storage Ready */
        case HI_USB_STATE_UVC_PC_READY:        /* UVC PC Ready    -> Storage Ready */
        case HI_USB_STATE_UVC_READY:           /* UVC Ready       -> Storage Ready */
        {
            UVC_Stop();
            UVC_Deinit();
            USB_STORAGE_Init();

            /** get storage state */
            HI_BOOL bStorageReady = HI_FALSE;
            if(NULL != pstCtx->stUsbCfg.pfnGetStorageState)
            {
                pstCtx->stUsbCfg.pfnGetStorageState((HI_VOID*)&bStorageReady);
            }

            if(HI_TRUE == bStorageReady)
            {
                USB_SetState(HI_USB_STATE_STORAGE_READY);
            }
            else
            {
                USB_SetState(HI_USB_STATE_INSERT);
            }
            break;
        }
        case HI_USB_STATE_INSERT:              /* Insert          -> Storage Ready */
        {
            //USB_STORAGE_Init();

            /** get storage state */
            HI_BOOL bStorageReady = HI_FALSE;
            if(NULL != pstCtx->stUsbCfg.pfnGetStorageState)
            {
                pstCtx->stUsbCfg.pfnGetStorageState((HI_VOID*)&bStorageReady);
            }

            if(HI_TRUE == bStorageReady)
            {
                USB_SetState(HI_USB_STATE_STORAGE_READY);
            }
            else
            {
                USB_SetState(HI_USB_STATE_INSERT);
            }
            break;
        }
        case HI_USB_STATE_STORAGE_READY:
        {
            HI_BOOL bPcState = HI_FALSE;
            USB_CheckPcState((const HI_CHAR *)&(pstCtx->stUsbCfg.stStorageCfg.szProcFile), &bPcState);
            /* if PC Ready, PreProcess by callback */
            if (bPcState)                      /* Storage Ready   -> Storage PC Ready */
            {
                USB_SetState(HI_USB_STATE_STORAGE_PC_READY);

                HI_USB_EVENT_INFO_S stEventInfo;
                stEventInfo.s32EventId = HI_EVENT_USB_STORAGE_READY;
                USB_EventProc(&stEventInfo);
            }
            break;
        }
        case HI_USB_STATE_STORAGE_PC_READY:    /* Storage PC Ready-> Storage SD Ready */
            USB_STORAGE_PrepareDev(&pstCtx->stUsbCfg.stStorageCfg);
            USB_SetState(HI_USB_STATE_STORAGE_SD_READY);
            break;
        default:
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 USB_CHARGE_StateProc(USB_CONTEXT_S* pstCtx)
{
    switch(pstCtx->enState)
    {
        case HI_USB_STATE_UVC_MEDIA_READY:  /* UVC Media Ready -> Insert */
        case HI_USB_STATE_UVC_PC_READY:     /* UVC PC Ready    -> Insert */
        case HI_USB_STATE_UVC_READY:        /* UVC Ready       -> Insert */
            UVC_Stop();
            UVC_Deinit();
            USB_SetState(HI_USB_STATE_INSERT);
            break;

        case HI_USB_STATE_STORAGE_READY:    /* Storage Ready   -> Insert */
        case HI_USB_STATE_STORAGE_PC_READY: /* Storage PC Ready-> Insert */
        case HI_USB_STATE_STORAGE_SD_READY: /* Storage SD Ready-> Insert */
            USB_STORAGE_Deinit();
            USB_SetState(HI_USB_STATE_INSERT);
            break;

        case HI_USB_STATE_INSERT:
        default:
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 USB_DeinitProc(USB_CONTEXT_S* pstCtx)
{
    switch(pstCtx->enState)
    {
        case HI_USB_STATE_UVC_MEDIA_READY:
        case HI_USB_STATE_UVC_PC_READY:
        case HI_USB_STATE_UVC_READY:
            UVC_Stop();
            UVC_Deinit();
            break;

        case HI_USB_STATE_STORAGE_READY:
        case HI_USB_STATE_STORAGE_PC_READY:
        case HI_USB_STATE_STORAGE_SD_READY:
            USB_STORAGE_Deinit();
            break;

        case HI_USB_STATE_INSERT:
        default:
            break;
    }
    return HI_SUCCESS;
}

static HI_S32 USB_StateProc(USB_CONTEXT_S* pstCtx, HI_BOOL bConnected)
{

    if (HI_USB_STATE_OUT == pstCtx->enState)
    {
        if (bConnected) /* Out -> Insert */
        {
            HI_USB_EVENT_INFO_S stEventInfo;
            USB_SetState(HI_USB_STATE_INSERT);
            MLOGI(YELLOW"publish HI_EVENT_USB_INSERT\n"NONE);
            stEventInfo.s32EventId = HI_EVENT_USB_INSERT;
            USB_EventProc(&stEventInfo);
        }
    }
    else if (!bConnected)
    {
        USB_DeinitProc(pstCtx);
        USB_SetState(HI_USB_STATE_OUT);

        HI_USB_EVENT_INFO_S stEventInfo;
        stEventInfo.s32EventId = HI_EVENT_USB_OUT;
        USB_EventProc(&stEventInfo);

        HI_USB_MODE_E enMode = pstCtx->enMode;
        HI_USB_SetMode(HI_USB_MODE_CHARGE); /* reset usb mode */
        HI_S32 s32Ret = HI_USB_SetMode(enMode); /* reset usb mode */
        MLOGD(YELLOW"s32Ret(%x)\n"NONE, s32Ret);
    }
    else
    {
        if (HI_USB_MODE_UVC == pstCtx->enMode)
        {
            USB_UVC_StateProc(pstCtx);
        }
        else if (HI_USB_MODE_STORAGE == pstCtx->enMode)
        {
            USB_STORAGE_StateProc(pstCtx);
        }
        else
        {
            USB_CHARGE_StateProc(pstCtx);
        }
    }
    return HI_SUCCESS;
}

static HI_VOID* USB_CheckTask(HI_VOID* pvArg)
{
    prctl(PR_SET_NAME, "HiUSB_Check", 0, 0, 0);

    HI_BOOL bAcState = HI_FALSE;
    while(s_stUSBCtx.bCheckTskRun)
    {
        USB_CheckChargeState((const HI_CHAR *)&(s_stUSBCtx.stUsbCfg.stStorageCfg.szProcFile), &bAcState);
        USB_StateProc(&s_stUSBCtx, bAcState);

        sleep(1);
    }
    return NULL;
}

HI_S32 HI_USB_Init(const HI_USB_CFG_S *pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_USB_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstCfg->pfnEventProc, HI_USB_EINVAL);
    if (s_stUSBCtx.bInit)
    {
        MLOGW("has already inited!\n");
        return HI_USB_EINITIALIZED;
    }
    HI_MUTEX_INIT_LOCK(s_stUSBCtx.stateMutex);
    /* record usb configure */
    memcpy(&s_stUSBCtx.stUsbCfg, pstCfg, sizeof(HI_USB_CFG_S));
#ifndef __HuaweiLite__
    if (NULL == realpath(pstCfg->stStorageCfg.szProcFile, s_stUSBCtx.stUsbCfg.stStorageCfg.szProcFile))
    {
        MLOGE("Invalid ProcFile:%s\n",pstCfg->stStorageCfg.szProcFile);
        return HI_USB_EINVAL;
    }
#endif
    /* Create usb check task thread */
    s_stUSBCtx.bCheckTskRun = HI_TRUE;
    if(pthread_create(&s_stUSBCtx.CheckTskId, NULL, USB_CheckTask, NULL))
    {
        MLOGE("USB_CheckTask create failed\n");
        s_stUSBCtx.bCheckTskRun = HI_FALSE;
        return HI_FAILURE;
    }
    MLOGD("USB_CheckTask create successful\n");

    s_stUSBCtx.enMode = HI_USB_MODE_CHARGE;
    s_stUSBCtx.enState = HI_USB_STATE_OUT;
    s_stUSBCtx.bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 HI_USB_Deinit(HI_VOID)
{
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);

    /* Destroy check task */
    s_stUSBCtx.bCheckTskRun = HI_FALSE;
    pthread_join(s_stUSBCtx.CheckTskId, NULL);

    USB_DeinitProc(&s_stUSBCtx);
    s_stUSBCtx.bInit = HI_FALSE;
    HI_MUTEX_DESTROY(s_stUSBCtx.stateMutex);
    return HI_SUCCESS;
}

HI_S32 HI_USB_SetUvcCfg(const HI_UVC_CFG_S* pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_USB_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);
    memcpy(&s_stUSBCtx.stUsbCfg.stUvcCfg, pstCfg, sizeof(HI_UVC_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HI_USB_SetStorageCfg(const HI_USB_STORAGE_CFG_S* pstCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstCfg, HI_USB_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);
    memcpy(&s_stUSBCtx.stUsbCfg.stStorageCfg, pstCfg, sizeof(HI_USB_STORAGE_CFG_S));
    return HI_SUCCESS;
}

HI_S32 HI_USB_SetMode(HI_USB_MODE_E enMode)
{
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);
    HI_APPCOMM_CHECK_EXPR((enMode < HI_USB_MODE_BUTT) && (enMode >= HI_USB_MODE_CHARGE), HI_USB_EINVAL);
    MLOGD("usb mode change[%s->%s]\n", USB_GetModeStr(s_stUSBCtx.enMode), USB_GetModeStr(enMode));
    HI_MUTEX_LOCK(s_stUSBCtx.stateMutex);
    if (s_stUSBCtx.enMode == enMode)
    {
        HI_MUTEX_UNLOCK(s_stUSBCtx.stateMutex);
        return HI_SUCCESS;
    }

    if (HI_USB_MODE_UVC == s_stUSBCtx.enMode)
    {
        UVC_Stop();
        UVC_Deinit();
    }
    else if (HI_USB_MODE_STORAGE == s_stUSBCtx.enMode)
    {
        USB_STORAGE_Deinit();
    }

    if (HI_USB_MODE_UVC == enMode)
    {
        UVC_Init(&s_stUSBCtx.stUsbCfg.stUvcCfg.stDevCap, &s_stUSBCtx.stUsbCfg.stUvcCfg.stDataSource,
            &s_stUSBCtx.stUsbCfg.stUvcCfg.stBufferCfg);
        UVC_Start(s_stUSBCtx.stUsbCfg.stUvcCfg.szDevPath);
    }
    else if (HI_USB_MODE_STORAGE == enMode)
    {
        USB_STORAGE_Init();
    }
    s_stUSBCtx.enMode = enMode;
    HI_MUTEX_UNLOCK(s_stUSBCtx.stateMutex);
    return HI_SUCCESS;
}

HI_S32 HI_USB_GetMode(HI_USB_MODE_E *penMode)
{
    HI_APPCOMM_CHECK_POINTER(penMode, HI_USB_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);
    MLOGD("usb mode[%s]\n", USB_GetModeStr(s_stUSBCtx.enMode));
    *penMode = s_stUSBCtx.enMode;
    return HI_SUCCESS;
}

HI_S32 HI_USB_GetState(HI_USB_STATE_E *penState)
{
    HI_APPCOMM_CHECK_POINTER(penState, HI_USB_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, HI_USB_ENOTINIT);
    MLOGD("usb state[%s]\n", USB_GetStateStr(s_stUSBCtx.enState));
    *penState = s_stUSBCtx.enState;
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

