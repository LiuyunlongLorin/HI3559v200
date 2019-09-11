/**
 * @file      product_media_hdmi.c
 * @brief     product media hdmi source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 * @version   1.0

 */

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "hi_mapi_sys.h"
#include "hi_mapi_hdmi.h"
#include "hi_product_media.h"
#include "hi_eventhub.h"
#include "product_media_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** HDMI ID */
#define PDT_MEDIA_HDMI_ID  (HI_HDMI_ID_0)

/** Media HDMI Context */
typedef struct tagPDT_MEDIA_HDMI_CTX_S
{
    HI_BOOL bInit;
    HI_PDT_MEDIA_HDMI_STATE_E enState;
    HI_MAPI_HDMI_ATTR_S stHDMIAttr;
} PDT_MEDIA_HDMI_CTX_S;
static PDT_MEDIA_HDMI_CTX_S s_stMEDIAHdmiCtx =
{
    .enState = HI_PDT_MEDIA_HDMI_STATE_UNPLUGGED
};


static HI_VOID PDT_MEDIA_HDMI_EventProc(HI_HDMI_EVENT_TYPE_E enEvent, HI_VOID *pvPrivData)
{
    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));

    switch(enEvent)
    {
        case HI_HDMI_EVENT_HOTPLUG:
            if (HI_PDT_MEDIA_HDMI_STATE_CONNECTTED == s_stMEDIAHdmiCtx.enState)
            {
                MLOGD("CurState: PlugIn, ignore new PlugIn Event\n");
                return;
            }
            MLOGD("HDMI PlugIn\n");
            s_stMEDIAHdmiCtx.enState = HI_PDT_MEDIA_HDMI_STATE_CONNECTTED;
            stEvent.EventID = HI_EVENT_PDT_MEDIA_HDMI_IN;
            break;
        case HI_HDMI_EVENT_NO_PLUG:
            if (HI_PDT_MEDIA_HDMI_STATE_UNPLUGGED == s_stMEDIAHdmiCtx.enState)
            {
                MLOGD("CurState: PlugOut, ignore new PlugOut Event\n");
                return;
            }
            MLOGD("HDMI PlugOut\n");
            s_stMEDIAHdmiCtx.enState = HI_PDT_MEDIA_HDMI_STATE_UNPLUGGED;
            stEvent.EventID = HI_EVENT_PDT_MEDIA_HDMI_OUT;
            break;
        default:
            MLOGW("unsupport hdmi event[%x]\n", enEvent);
            return;

    }
    HI_EVTHUB_Publish(&stEvent);
}

/**
 * @brief     get display size with specified vo sync
 * @param[in] enVoSync : VO intf sync
 * @param[out]pstDispSize : display size
 * @param[out]ps32FrameRate : framerate
 * @return    None
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/20
 */
static HI_VOID PDT_MEDIA_HDMI_GetDispSizeByVoSync(VO_INTF_SYNC_E enVoSync, SIZE_S* pstDispSize, HI_S32* ps32FrameRate)
{
    switch (enVoSync)
    {
        case VO_OUTPUT_PAL:
            pstDispSize->u32Width  = 720;
            pstDispSize->u32Height = 576;
            *ps32FrameRate = 25;
            break;
        case VO_OUTPUT_576P50:
            pstDispSize->u32Width  = 720;
            pstDispSize->u32Height = 576;
            *ps32FrameRate = 50;
            break;

        case VO_OUTPUT_NTSC:
            pstDispSize->u32Width  = 720;
            pstDispSize->u32Height = 480;
            *ps32FrameRate = 30;
            break;
        case VO_OUTPUT_480P60:
            pstDispSize->u32Width  = 720;
            pstDispSize->u32Height = 480;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1080P24:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 24;
            break;
        case VO_OUTPUT_1080P25:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 25;
            break;
        case VO_OUTPUT_1080P30:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 30;
            break;
        case VO_OUTPUT_1080I50:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_1080I60:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 60;
            break;
        case VO_OUTPUT_1080P50:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_1080P60:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1080;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_720P50:
            pstDispSize->u32Width  = 1280;
            pstDispSize->u32Height = 720;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_720P60:
            pstDispSize->u32Width  = 1280;
            pstDispSize->u32Height = 720;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_800x600_60:
            pstDispSize->u32Width  = 800;
            pstDispSize->u32Height = 600;
            *ps32FrameRate = 60;
            break;
        case VO_OUTPUT_800x600_50:
            pstDispSize->u32Width  = 800;
            pstDispSize->u32Height = 600;
            *ps32FrameRate = 50;
            break;

        case VO_OUTPUT_1024x768_60:
            pstDispSize->u32Width  = 1024;
            pstDispSize->u32Height = 768;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1280x1024_60:
            pstDispSize->u32Width  = 1280;
            pstDispSize->u32Height = 1024;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1366x768_60:
            pstDispSize->u32Width  = 1366;
            pstDispSize->u32Height = 768;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1440x900_60:
            pstDispSize->u32Width  = 1440;
            pstDispSize->u32Height = 900;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1280x800_60:
            pstDispSize->u32Width  = 1280;
            pstDispSize->u32Height = 800;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1600x1200_60:
            pstDispSize->u32Width  = 1600;
            pstDispSize->u32Height = 1200;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1680x1050_60:
            pstDispSize->u32Width  = 1680;
            pstDispSize->u32Height = 1050;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1920x1200_60:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 1200;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_640x480_60:
            pstDispSize->u32Width  = 640;
            pstDispSize->u32Height = 480;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_960H_PAL:
            pstDispSize->u32Width  = 960;
            pstDispSize->u32Height = 576;
            *ps32FrameRate = 25;
            break;

        case VO_OUTPUT_960H_NTSC:
            pstDispSize->u32Width  = 960;
            pstDispSize->u32Height = 480;
            *ps32FrameRate = 30;
            break;

        case VO_OUTPUT_1920x2160_30:
            pstDispSize->u32Width  = 1920;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 30;
            break;

        case VO_OUTPUT_2560x1440_30:
            pstDispSize->u32Width  = 2560;
            pstDispSize->u32Height = 1440;
            *ps32FrameRate = 30;
            break;

        case VO_OUTPUT_2560x1600_60:
            pstDispSize->u32Width  = 2560;
            pstDispSize->u32Height = 1600;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_3840x2160_24:
            pstDispSize->u32Width  = 3840;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 24;
            break;
        case VO_OUTPUT_3840x2160_25:
            pstDispSize->u32Width  = 3840;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 25;
            break;
        case VO_OUTPUT_3840x2160_30:
            pstDispSize->u32Width  = 3840;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 30;
            break;
        case VO_OUTPUT_3840x2160_50:
            pstDispSize->u32Width  = 3840;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_3840x2160_60:
            pstDispSize->u32Width  = 3840;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_4096x2160_24:
            pstDispSize->u32Width  = 4096;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 24;
            break;
        case VO_OUTPUT_4096x2160_25:
            pstDispSize->u32Width  = 4096;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 25;
            break;
        case VO_OUTPUT_4096x2160_30:
            pstDispSize->u32Width  = 4096;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 30;
            break;
        case VO_OUTPUT_4096x2160_50:
            pstDispSize->u32Width  = 4096;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_4096x2160_60:
            pstDispSize->u32Width  = 4096;
            pstDispSize->u32Height = 2160;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_320x240_60:
            pstDispSize->u32Width  = 320;
            pstDispSize->u32Height = 240;
            *ps32FrameRate = 60;
            break;
        case VO_OUTPUT_320x240_50:
            pstDispSize->u32Width  = 320;
            pstDispSize->u32Height = 240;
            *ps32FrameRate = 50;
            break;

        case VO_OUTPUT_240x320_50:
            pstDispSize->u32Width  = 240;
            pstDispSize->u32Height = 320;
            *ps32FrameRate = 50;
            break;
        case VO_OUTPUT_240x320_60:
            pstDispSize->u32Width  = 240;
            pstDispSize->u32Height = 320;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_720x1280_60:
            pstDispSize->u32Width  = 720;
            pstDispSize->u32Height = 1280;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_1080x1920_60 :
            pstDispSize->u32Width  = 1080;
            pstDispSize->u32Height = 1920;
            *ps32FrameRate = 60;
            break;

        case VO_OUTPUT_7680x4320_30 :
            pstDispSize->u32Width  = 7680;
            pstDispSize->u32Height = 4320;
            *ps32FrameRate = 30;
            break;
        /* TODO: check user */
        default:
            MLOGE("Invalid VoSync[%d]\n", enVoSync);
            pstDispSize->u32Width  = 0;
            pstDispSize->u32Height = 0;
            *ps32FrameRate = 0;
            break;
    }
    MLOGI("DispSize[%ux%u] FrameRate[%dfps]\n", pstDispSize->u32Width, pstDispSize->u32Height, *ps32FrameRate);
}

/**
 * @brief     get VoSync by hdmi sink capablity
 * @param[in] pstSinkCapability : hdmi sink capability
 * @param[out]penVoSync : vo sync
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/21
 */
static HI_S32 PDT_MEDIA_HDMI_GetVoSync(const HI_MAPI_HDMI_SINKCAPABILITY_S* pstSinkCapability, VO_INTF_SYNC_E* penVoSync)
{
    SIZE_S stDispSize = {0, 0};
    HI_S32 s32FrameRate = 0;

    *penVoSync = VO_OUTPUT_BUTT;
    if ((pstSinkCapability->enNativeIntfSync < VO_OUTPUT_USER)
        && (pstSinkCapability->enNativeIntfSync >= VO_OUTPUT_PAL))
    {
        *penVoSync = pstSinkCapability->enNativeIntfSync;
        PDT_MEDIA_HDMI_GetDispSizeByVoSync(*penVoSync, &stDispSize, &s32FrameRate);
        if((stDispSize.u32Width>PDT_MEDIA_HDMI_MAX_WIDTH)||(stDispSize.u32Height>PDT_MEDIA_HDMI_MAX_HEIGHT))
        {
            stDispSize.u32Width = 0;
            stDispSize.u32Height = 0;
            *penVoSync = VO_OUTPUT_BUTT;
        }
        MLOGD(YELLOW"enNativeIntfSync[%d]"NONE"\n", *penVoSync);
    }

    HI_S32 i;
    for (i = 0; i < VO_OUTPUT_USER; ++i)
    {
        if (pstSinkCapability->abIntfSync[i])
        {
            SIZE_S stDispSizeNew = {0, 0};
            HI_S32 s32FrameRateNew = 0;
            PDT_MEDIA_HDMI_GetDispSizeByVoSync(i, &stDispSizeNew, &s32FrameRateNew);

            if((stDispSizeNew.u32Width>PDT_MEDIA_HDMI_MAX_WIDTH)||(stDispSizeNew.u32Height>PDT_MEDIA_HDMI_MAX_HEIGHT))
            {
                continue;
            }

            if( ((stDispSizeNew.u32Width > stDispSize.u32Width) && (stDispSizeNew.u32Height >= stDispSize.u32Height))
               ||((stDispSizeNew.u32Width >= stDispSize.u32Width) && (stDispSizeNew.u32Height > stDispSize.u32Height))
               ||((stDispSizeNew.u32Width == stDispSize.u32Width) && (stDispSizeNew.u32Height == stDispSize.u32Height)
                    && (s32FrameRateNew > s32FrameRate)) )
            {
               *penVoSync = i;
               stDispSize.u32Width = stDispSizeNew.u32Width;
               stDispSize.u32Height = stDispSizeNew.u32Height;
               s32FrameRate = s32FrameRateNew;
               MLOGD(YELLOW"find better vosync[%d]"NONE"\n", *penVoSync);
            }
        }
    }

    if(VO_OUTPUT_BUTT == *penVoSync)
    {
        MLOGE("no support vo sync\n");
        return HI_PDT_MEDIA_EUNSUPPORT;
    }

    MLOGI(YELLOW"The Best VoSync[%d] Size[%ux%u] FrameRate[%d]"NONE"\n",
        *penVoSync, stDispSize.u32Width, stDispSize.u32Height, s32FrameRate);
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_HDMI_FindMatchEnableVcapPipeChn(HI_PDT_MEDIA_VCAP_CFG_S* pstVcapCfg,
                        HI_HANDLE VcapPipeHdl, HI_HANDLE PipeChnHdl,
                        HI_PDT_MEDIA_VCAP_DEV_ATTR_S**  pstVcapDevAttr,
                        HI_PDT_MEDIA_VCAP_PIPE_ATTR_S** ppstVcapPipeAttr,
                        HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S** ppstPipeChnAttr)
{
    HI_S32 i, j, k;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVcapCfg->astVcapDevAttr[i].bEnable)
        {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            if ((!pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
                || (pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl != VcapPipeHdl))
            {
                continue;
            }
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if ((!pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].bEnable)
                    || (pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k].PipeChnHdl != PipeChnHdl))
                {
                    continue;
                }

                *pstVcapDevAttr   = &pstVcapCfg->astVcapDevAttr[i];
                *ppstVcapPipeAttr = &pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j];
                *ppstPipeChnAttr  = &pstVcapCfg->astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[k];
                MLOGD("FindMatch VcapDev[%d] Pipe[%d] PipeChn[%d]\n", i, j, k);
                return HI_SUCCESS;
            }
        }
    }
    MLOGW("NoMatch enable VcapPipeHdl[%d] PipeChnHdl[%d]\n", VcapPipeHdl, PipeChnHdl);
    return HI_PDT_MEDIA_EINVAL;
}

static HI_S32 PDT_MEDIA_HDMI_FindMatchEnableVpssPort(HI_PDT_MEDIA_VPROC_CFG_S* pstVprocCfg,
                        HI_HANDLE VpssHdl, HI_HANDLE VportHdl,
                        HI_PDT_MEDIA_VPSS_ATTR_S** ppstVpssAttr,
                        HI_PDT_MEDIA_VPSS_PORT_ATTR_S** ppstPortAttr)
{
    HI_S32 i, j;
    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        if ((!pstVprocCfg->astVpssAttr[i].bEnable)
            || (pstVprocCfg->astVpssAttr[i].VpssHdl != VpssHdl))
        {
            continue;
        }
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if ((!pstVprocCfg->astVpssAttr[i].bEnable)
                || (pstVprocCfg->astVpssAttr[i].astVportAttr[j].VportHdl != VportHdl))
            {
                continue;
            }

            *ppstVpssAttr = &pstVprocCfg->astVpssAttr[i];
            *ppstPortAttr = &pstVprocCfg->astVpssAttr[i].astVportAttr[j];
            MLOGD("FindMatch Vpss[%d,hdl-%d] Port[%d,hdl-%d]\n", i, VpssHdl, j, VportHdl);
            return HI_SUCCESS;
        }
    }
    MLOGW("NoMatch enable VpssHdl[%d] PortHdl[%d]\n", VpssHdl, VportHdl);
    return HI_PDT_MEDIA_EINVAL;
}

/**
 * @brief     update display binded vpssport size
 *            if sensor < display, then vpssport keep the same
 *            if sensor >= display, vpssport will be dispsize consider aspect with sensor
 * @param[in] pstDispWndCfg : display window configure
 * @param[in/out] pstMediaCfg : media configure
 * @param[in] pstDispSize : display size
 * @return    0 success,non-zero error code.
 * @exception None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/21
 */
static HI_S32 PDT_MEDIA_HDMI_UpdateBindedVpssPortSize(const HI_PDT_MEDIA_DISP_WND_CFG_S* pstDispWndCfg,
                        HI_PDT_MEDIA_CFG_S* pstMediaCfg, const SIZE_S* pstDispSize)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* find match enable vpss/port */
    HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
    HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVportAttr = NULL;
    s32Ret = PDT_MEDIA_HDMI_FindMatchEnableVpssPort(&pstMediaCfg->stVideoCfg.stVprocCfg,
                pstDispWndCfg->ModHdl, pstDispWndCfg->ChnHdl,
                &pstVpssAttr, &pstVportAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* find match vcap dev/pipe/chn */
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S*      pstVcapDev = NULL;
    HI_PDT_MEDIA_VCAP_PIPE_ATTR_S*     pstVcapPipeAttr = NULL;
    HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
    s32Ret = PDT_MEDIA_HDMI_FindMatchEnableVcapPipeChn(&pstMediaCfg->stVideoCfg.stVcapCfg,
                pstVpssAttr->VcapPipeHdl, pstVpssAttr->VcapPipeChnHdl,
                &pstVcapDev, &pstVcapPipeAttr, &pstPipeChnAttr);
    if (HI_SUCCESS != s32Ret
        || pstVcapDev->stSnsAttr.stResolution.u32Width < pstDispSize->u32Width)
    { /* (No sensor input) Or (Sensor < Display) */
        MLOGI("vpss port keep the size[%ux%u]\n",
            pstVportAttr->stResolution.u32Width, pstVportAttr->stResolution.u32Height);
        return HI_SUCCESS;
    }

    /* update vpssport size */
    SIZE_S* pstVportSize = &pstVportAttr->stResolution;
    SIZE_S* pstSnsSize = &pstVcapDev->stSnsAttr.stResolution;

    /* Wsns/Hsns < Wdisp/Hdisp : Left/Right */
    if (pstSnsSize->u32Width * pstDispSize->u32Height < pstDispSize->u32Width * pstSnsSize->u32Height)
    {
        pstVportSize->u32Width  = pstSnsSize->u32Width * pstDispSize->u32Height / pstSnsSize->u32Height;
        pstVportSize->u32Height = pstDispSize->u32Height;
    }
    /* Wsns/Hsns > Wdisp/Hdisp : Top/Bottom */
    else if(pstSnsSize->u32Width * pstDispSize->u32Height > pstDispSize->u32Width * pstSnsSize->u32Height)
    {
        pstVportSize->u32Width  = pstDispSize->u32Width;
        pstVportSize->u32Height = pstSnsSize->u32Height * pstDispSize->u32Width / pstSnsSize->u32Width;
    }
    else
    {
        pstVportSize->u32Width  = pstDispSize->u32Width;
        pstVportSize->u32Height = pstDispSize->u32Height;
    }

    MLOGD("VpssPort Size[%ux%u]\n", pstVportSize->u32Width, pstVportSize->u32Height);
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_HDMI_GetAudioSampleRate(const HI_PDT_MEDIA_CFG_S* pstMediaCfg,
                        const HI_MAPI_HDMI_SINKCAPABILITY_S* pstSinkCapability,
                        AUDIO_SAMPLE_RATE_E* penSampleRate)
{
    HI_S32 i, j;
    const HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = NULL;
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        pstAoCfg = &pstMediaCfg->stAudioOutCfg.astAoCfg[i];
        if (!pstAoCfg->bEnable)
        {
            continue;
        }
        for (j = 0; j < HI_MAPI_AO_MAX_SUPPORT_SAMPLERATE_NUM; ++j)
        {
            if (pstAoCfg->stAoAttr.enSampleRate == pstSinkCapability->aenSupportSampleRate[j])
            {
                *penSampleRate = pstAoCfg->stAoAttr.enSampleRate;
                MLOGD("audio samplerate[%d]\n", *penSampleRate);
                return HI_SUCCESS;
            }
        }
    }
    *penSampleRate = AUDIO_SAMPLE_RATE_BUTT;
    MLOGW("unsupport hdmi audio\n");
    return HI_PDT_MEDIA_EUNSUPPORT;
}

static HI_S32 PDT_MEDIA_HDMI_GenerateHDMIAttr(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_MAPI_HDMI_ATTR_S* pstHDMIAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /* get hdmi sinkcapability */
    HI_MAPI_HDMI_SINKCAPABILITY_S stSinkCapability;
    memset(&stSinkCapability, 0, sizeof(HI_MAPI_HDMI_SINKCAPABILITY_S));
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_GetSinkCapability(PDT_MEDIA_HDMI_ID, &stSinkCapability);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /* HDMI VoSync */
    VO_INTF_SYNC_E enVoSync = stSinkCapability.enNativeIntfSync;
    s32Ret = PDT_MEDIA_HDMI_GetVoSync(&stSinkCapability, &enVoSync);

    /* hdmi audio */
    AUDIO_SAMPLE_RATE_E enSampleRate = AUDIO_SAMPLE_RATE_BUTT;
    s32Ret = PDT_MEDIA_HDMI_GetAudioSampleRate(pstMediaCfg, &stSinkCapability, &enSampleRate);

    /* update hdmi attribute */
    pstHDMIAttr->bEnableHdmi  = HI_TRUE;
    pstHDMIAttr->bEnableVideo = HI_TRUE;
    pstHDMIAttr->enHdmiSync   = enVoSync;
    pstHDMIAttr->bEnableAudio = ((AUDIO_SAMPLE_RATE_BUTT == enSampleRate) ? HI_FALSE : HI_TRUE);
    pstHDMIAttr->enSampleRate = ((AUDIO_SAMPLE_RATE_BUTT == enSampleRate) ? AUDIO_SAMPLE_RATE_48000 : enSampleRate);
    pstHDMIAttr->u32PixClk = 0;
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_UpdateMediaCfg(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

    /* get hdmi attribute */
    HI_MAPI_HDMI_ATTR_S stHDMIAttr;
    s32Ret = PDT_MEDIA_HDMI_GenerateHDMIAttr(pstMediaCfg, &stHDMIAttr);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

    /* get display size */
    SIZE_S stDispSize;
    HI_S32 s32FrameRate = 0;
    PDT_MEDIA_HDMI_GetDispSizeByVoSync(stHDMIAttr.enHdmiSync, &stDispSize, &s32FrameRate);

    /* update display configure: IntfType/IntfSync/WndRect */
    HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg = NULL;
    HI_PDT_MEDIA_DISP_WND_CFG_S* pstDispWndCfg = NULL;
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; ++i)
    {
        if (!pstMediaCfg->stVideoOutCfg.astDispCfg[i].bEnable)
        {
            continue;
        }
        pstDispCfg = &pstMediaCfg->stVideoOutCfg.astDispCfg[i];

        for (j = 0; j < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++j)
        {
            if (!pstDispCfg->astWndCfg[j].bEnable)
            {
                continue;
            }
            pstDispWndCfg = &pstDispCfg->astWndCfg[j];
            pstDispWndCfg->stAspectRatio.enMode = ASPECT_RATIO_NONE;

            pstDispCfg->stDispAttr.stPubAttr.enIntfType = VO_INTF_HDMI;
            pstDispCfg->stDispAttr.stPubAttr.enIntfSync = stHDMIAttr.enHdmiSync;

            SIZE_S stWndSize;
            PDT_MEDIA_HDMI_GetDispSizeByVoSync(stHDMIAttr.enHdmiSync, &stWndSize, &s32FrameRate);
            pstDispWndCfg->stWndAttr.stRect.u32Height = stWndSize.u32Height;
            pstDispWndCfg->stWndAttr.stRect.u32Width  = stWndSize.u32Width;
            break;
        }
    }
    if (!pstDispWndCfg)
    {
        MLOGE("No enable display window\n");
        return HI_PDT_MEDIA_EINVAL;
    }

    /* update binded vpss port configure if bindtype is vpss */
    if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstDispWndCfg->enBindedMod)
    {
        s32Ret = PDT_MEDIA_HDMI_UpdateBindedVpssPortSize(pstDispWndCfg, pstMediaCfg, &stDispSize);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }

    /* update ao configure: samplerate */
    for (i = 0; i < HI_PDT_MEDIA_AO_MAX_CNT; ++i)
    {
        HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = &pstMediaCfg->stAudioOutCfg.astAoCfg[i];
        if (!pstAoCfg->bEnable)
        {
            continue;
        }
        if ((AUDIO_SAMPLE_RATE_BUTT != stHDMIAttr.enSampleRate)
            && (pstAoCfg->stAoAttr.enSampleRate != stHDMIAttr.enSampleRate))
        {
            pstAoCfg->stAoAttr.enSampleRate = stHDMIAttr.enSampleRate;
        }
        MLOGD("Ao SampleRate[%d]\n", pstAoCfg->stAoAttr.enSampleRate);
    }

    /* update HDMI attribute in context */
    memcpy(&s_stMEDIAHdmiCtx.stHDMIAttr, &stHDMIAttr, sizeof(HI_MAPI_HDMI_ATTR_S));

    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_Init(HI_VOID)
{
    if (s_stMEDIAHdmiCtx.bInit)
    {
        MLOGW("HDMI already Init\n");
        return HI_PDT_MEDIA_EINITIALIZED;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_CALLBACK_FUNC_S stHDMICb;

    stHDMICb.pfnHdmiEventCallback = PDT_MEDIA_HDMI_EventProc;
    stHDMICb.pPrivateData = NULL;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_Init(PDT_MEDIA_HDMI_ID, &stHDMICb);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s_stMEDIAHdmiCtx.bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_Deinit(HI_VOID)
{
    if(!s_stMEDIAHdmiCtx.bInit)
    {
        MLOGW("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_Deinit(PDT_MEDIA_HDMI_ID);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s_stMEDIAHdmiCtx.bInit = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_Start(HI_VOID)
{
    if (!s_stMEDIAHdmiCtx.bInit)
    {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_SetAttr(HI_HDMI_ID_0, &s_stMEDIAHdmiCtx.stHDMIAttr);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_Start(HI_HDMI_ID_0);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_Stop(HI_VOID)
{
    if (!s_stMEDIAHdmiCtx.bInit)
    {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    HI_PERFORMANCE_TIME_STAMP;
    s32Ret = HI_MAPI_HDMI_Stop(HI_HDMI_ID_0);
    HI_PERFORMANCE_TIME_STAMP;
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD("Success\n");
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_GetState(HI_PDT_MEDIA_HDMI_STATE_E* penState)
{
    if (!s_stMEDIAHdmiCtx.bInit)
    {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }
    *penState = s_stMEDIAHdmiCtx.enState;
    MLOGD("HDMI State[%d:0-unplugged,1-connectted]\n", *penState);
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_HDMI_GetDispSize(SIZE_S *pstSize)
{
    if (!s_stMEDIAHdmiCtx.bInit)
    {
        MLOGE("HDMI not init yet\n");
        return HI_PDT_MEDIA_ENOTINIT;
    }
    if (HI_PDT_MEDIA_HDMI_STATE_UNPLUGGED == s_stMEDIAHdmiCtx.enState)
    {
        MLOGE("HDMI not connected\n");
        return HI_PDT_MEDIA_EUNSUPPORT;
    }
    HI_S32 s32FrameRate = 0;
    PDT_MEDIA_HDMI_GetDispSizeByVoSync(s_stMEDIAHdmiCtx.stHDMIAttr.enHdmiSync, pstSize, &s32FrameRate);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


