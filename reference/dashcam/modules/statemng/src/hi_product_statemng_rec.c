/**
 * \file   hi_product_statemng_rec.c
 * \brief  Realize the interface about record states.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include <unistd.h>
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"
#include "hi_liveserver.h"
#include "hi_ahdmng.h"
#include "hi_product_scene.h"

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#include "hi_tempcycle.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifdef CONFIG_RAWCAP_ON
static HI_S32 PDT_STATEMNG_GetRawFileName(HI_RAWCAP_FILENAME_S* pstFileName,
        PDT_STATEMNG_RAWCAPTASK_CONTEXT_S* pstRawCapCtx)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapPipeHdl = pstRawCapCtx->stRawCapCfg.stDataSource.VcapPipeHdl;
    HI_HANDLE VcapPipeChnHdl = pstRawCapCtx->stRawCapCfg.stDataSource.VcapPipeChnHdl;
    HI_MAPI_VCAP_ATTR_S stVcapAttr;
    s32Ret = HI_MAPI_VCAP_GetAttr(pstRawCapCtx->VcapDevHdl, &stVcapAttr);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED" get Vcap Attr fail,VcapDevHdl:%d,s32Ret:0x%x \n\n"NONE, pstRawCapCtx->VcapDevHdl, s32Ret);
        return HI_PDT_STATEMNG_EINTER;
    }

    HI_MAPI_VCAP_EXPOSURE_INFO_S stExpInfo;
    s32Ret = HI_MAPI_VCAP_GetAttrEx(VcapPipeHdl, VcapPipeChnHdl,
                                    HI_MAPI_VCAP_CMD_ISP_GetExposureInfo,
                                    &stExpInfo, sizeof(HI_MAPI_VCAP_EXPOSURE_INFO_S));
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get vcap attrex");

    HI_CHAR szRawFormat[5] = {0};
    PIXEL_FORMAT_E  enPixelFormat;
    /*TODO*/
    enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    ISP_BAYER_FORMAT_E  enBayerFmt =  stVcapAttr.astVcapPipeAttr[VcapPipeHdl].stIspPubAttr.enBayer;
    HI_S32 s32FrameBits;

    switch (enPixelFormat)
    {
        case PIXEL_FORMAT_RGB_BAYER_8BPP:
            s32FrameBits = 8;
            break;

        case PIXEL_FORMAT_RGB_BAYER_10BPP:
            s32FrameBits = 10;
            break;

        case PIXEL_FORMAT_RGB_BAYER_12BPP:
            s32FrameBits = 12;
            break;

        case PIXEL_FORMAT_RGB_BAYER_14BPP:
            s32FrameBits = 14;
            break;

        case PIXEL_FORMAT_RGB_BAYER_16BPP:
            s32FrameBits = 16;
            break;

        default:
            MLOGI("PixelFormat(%d) is not correct!\n", enPixelFormat);
            return HI_PDT_STATEMNG_EINTER;
    }

    switch (enBayerFmt)
    {
        case BAYER_RGGB:
            snprintf(szRawFormat, sizeof(szRawFormat), "%s", "RGGB");
            break;

        case BAYER_GRBG:
            snprintf(szRawFormat, sizeof(szRawFormat), "%s", "GRBG");
            break;

        case BAYER_GBRG:
            snprintf(szRawFormat, sizeof(szRawFormat), "%s", "GBRG");
            break;

        case BAYER_BGGR:
            snprintf(szRawFormat, sizeof(szRawFormat), "%s", "BGGR");
            break;

        default:
            snprintf(szRawFormat, sizeof(szRawFormat), "%s", "NA");
    }

    if (WDR_MODE_NONE == stVcapAttr.stVcapDevAttr.enWdrMode)
    {
        snprintf(pstFileName->szRaw_FileName,
                 HI_RAWCAP_MAX_FILENAME_LEN,
                 "%s_%dx%d_%dbits_%s_Linear_ExpTime_%d_ag_%d_dg_%d_ispdg_%d",
                 pstRawCapCtx->szPrefix,
                 stVcapAttr.astVcapPipeAttr[VcapPipeHdl].stIspPubAttr.stSize.u32Width,
                 stVcapAttr.astVcapPipeAttr[VcapPipeHdl].stIspPubAttr.stSize.u32Height,
                 s32FrameBits,
                 szRawFormat,
                 stExpInfo.u32ExpTime,
                 stExpInfo.u32AGain,
                 stExpInfo.u32DGain,
                 stExpInfo.u32ISPDGain);
    }
    else
    {
        snprintf(pstFileName->szRaw_FileName,
                 HI_RAWCAP_MAX_FILENAME_LEN,
                 "%s_%dx%d_%dbits_%s_WDR_ExpTime_%d_ag_%d_dg_%d_ispdg_%d",
                 pstRawCapCtx->szPrefix,
                 stVcapAttr.astVcapPipeAttr[VcapPipeHdl].stIspPubAttr.stSize.u32Width,
                 stVcapAttr.astVcapPipeAttr[VcapPipeHdl].stIspPubAttr.stSize.u32Height,
                 s32FrameBits,
                 szRawFormat,
                 stExpInfo.u32ExpTime,
                 stExpInfo.u32AGain,
                 stExpInfo.u32DGain,
                 stExpInfo.u32ISPDGain);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GetRawCapFileNames(HI_RAWCAP_FILENAME_S* pstFilename, HI_VOID* pvPrivateData)
{
    HI_S32 s32Ret = HI_FAILURE;
    PDT_STATEMNG_CHECK_POINTER(pstFilename, HI_PDT_STATEMNG_ENULLPTR, " pszFilename");
    PDT_STATEMNG_CHECK_POINTER(pvPrivateData, HI_PDT_STATEMNG_ENULLPTR, " pvPrivateData");
    PDT_STATEMNG_RAWCAPTASK_CONTEXT_S* pRawCapTaskCtx = (PDT_STATEMNG_RAWCAPTASK_CONTEXT_S*)pvPrivateData;
    memset(pstFilename, 0, sizeof(HI_RAWCAP_FILENAME_S));

    s32Ret = PDT_STATEMNG_GetRawFileName(pstFilename, pRawCapTaskCtx);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(" get raw file name failed,VcapPipeHdl:%d,VcapPipeChnHdl:%d \n", pRawCapTaskCtx->stRawCapCfg.stDataSource.VcapPipeHdl,
              pRawCapTaskCtx->stRawCapCfg.stDataSource.VcapPipeChnHdl);
        return s32Ret;
    }

    if (pRawCapTaskCtx->stRawCapCfg.stDataSource.bDumpYUV)
    {
        HI_MAPI_VPORT_ATTR_S stPortAttr;
        HI_HANDLE VpssHdl = pRawCapTaskCtx->stRawCapCfg.stDataSource.VpssHdl;
        HI_HANDLE VPortHdl = pRawCapTaskCtx->stRawCapCfg.stDataSource.VPortHdl;
        s32Ret = HI_MAPI_VPROC_GetPortAttr(VpssHdl, VPortHdl, &stPortAttr);

        if (HI_SUCCESS != s32Ret)
        {
            MLOGE(" get port attr failed,VpssHdl:%d,VPortHdl:%d \n", VpssHdl, VPortHdl);
            return HI_PDT_STATEMNG_EINTER;
        }

        snprintf(pstFilename->szYUV_FileName, HI_RAWCAP_MAX_FILENAME_LEN, "%s_%dx%d",
                 pRawCapTaskCtx->szPrefix, stPortAttr.u32Width, stPortAttr.u32Height);
    }

    snprintf(pstFilename->szDebugInfo_FileName, HI_RAWCAP_MAX_FILENAME_LEN, "%s", pRawCapTaskCtx->szPrefix);

    return HI_SUCCESS;
}



HI_S32 PDT_STATEMNG_GetRawCapSrc(HI_PDT_PARAM_CAM_MEDIA_CFG_S* pstCamCfg,
                                 HI_RAWCAP_DATA_SOURCE_S* pstDataSrc)
{
    if (!pstCamCfg->astVencCfg[0].bEnable)
    {
        MLOGE(" venc 0 is disable \n");
        return HI_PDT_STATEMNG_EINVAL;
    }

    /*venc0 is main stream handle*/
    if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstCamCfg->astVencCfg[0].enBindedMod)
    {
        pstDataSrc->VcapPipeHdl = pstCamCfg->astVencCfg[0].ModHdl;
        pstDataSrc->VcapPipeChnHdl = pstCamCfg->astVencCfg[0].ChnHdl;
        pstDataSrc->bDumpYUV = HI_FALSE;
    }
    else if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstCamCfg->astVencCfg[0].enBindedMod)
    {
        pstDataSrc->VpssHdl = pstCamCfg->astVencCfg[0].ModHdl;
        pstDataSrc->VPortHdl = pstCamCfg->astVencCfg[0].ChnHdl;
        pstDataSrc->bDumpYUV = HI_FALSE;
    }
    else
    {
        MLOGE("venc 0 bindmode[%d] is unsupport \n", pstCamCfg->astVencCfg[0].enBindedMod);
        return HI_PDT_STATEMNG_EINVAL;
    }

    HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
    HI_S32 i = 0;

    if (pstDataSrc->bDumpYUV)
    {
        for (i = 0; i < HI_PDT_VCAPDEV_VPSS_MAX_CNT; i++)
        {
            pstVpssAttr = &pstCamCfg->astVpssCfg[i];

            if ((pstVpssAttr->bEnable) && (pstVpssAttr->VpssHdl == pstDataSrc->VpssHdl))
            {
                pstDataSrc->VcapPipeHdl = pstVpssAttr->VcapPipeHdl;
                pstDataSrc->VcapPipeChnHdl = pstVpssAttr->VcapPipeChnHdl;
            }

            break;
        }

        if (i >= HI_PDT_VCAPDEV_VPSS_MAX_CNT)
        {
            MLOGE("find vcappipe handle failed \n");
            return HI_PDT_STATEMNG_EINVAL;
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GenerateRawCapCtx(PDT_STATEMNG_RAWCAP_CONTEXT_S* pstRawCapCtx)
{
    PDT_STATEMNG_CHECK_POINTER(pstRawCapCtx, HI_PDT_STATEMNG_ENULLPTR, " pstRawCapCtx");

    HI_PDT_PARAM_CAM_MEDIA_CFG_S astCamCfg[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_PDT_PARAM_SCENE_CAM_VCAP_ATTR_S astSceneVcapAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];

    HI_S32 i = 0, s32Ret = HI_FAILURE;
    HI_PDT_PARAM_CAM_CONTEXT_S* pstCamCtx = PDT_STATEMNG_GetCtx()->astCamCtx;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if (pstCamCtx[i].bEnable)
        {
            s32Ret = HI_PDT_PARAM_GetCamMediaCfg(PDT_STATEMNG_GetCtx()->enCurrentWorkMode, &pstCamCtx[i].stMediaModeCfg,
                                                 &astCamCfg[i], &astSceneVcapAttr[i]);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("get cam[%d] media configure failed \n", i);
                return HI_PDT_STATEMNG_EINVAL;
            }

            pstRawCapCtx->astRawCapTaskCtx[i].s32CamID = i;
            pstRawCapCtx->astRawCapTaskCtx[i].VcapDevHdl = astCamCfg[i].stVcapDevAttr.VcapDevHdl;
            if (!astCamCfg[i].stVcapDevAttr.astVcapPipeAttr[0].bIspBypass)
            {
                pstRawCapCtx->astRawCapTaskCtx[i].bEnable = HI_TRUE;
            }
            else
            {
                pstRawCapCtx->astRawCapTaskCtx[i].bEnable = HI_FALSE;
            }
            /*get data source*/
            s32Ret = PDT_STATEMNG_GetRawCapSrc(&astCamCfg[i], &pstRawCapCtx->astRawCapTaskCtx[i].stRawCapCfg.stDataSource);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("get cam[%d] rawcap data source failed \n", i);
                return HI_PDT_STATEMNG_EINVAL;
            }

            pstRawCapCtx->astRawCapTaskCtx[i].stRawCapCfg.u32RawDepth = PDT_STATEMNG_RAWCAP_DEPTH;
            pstRawCapCtx->astRawCapTaskCtx[i].stRawCapCfg.bRawFileSeparated = HI_TRUE;
            /*set filename callback*/
            pstRawCapCtx->astRawCapTaskCtx[i].stRawCapCfg.stGetFilenameCb.pfnGetFileNameCb = PDT_STATEMNG_GetRawCapFileNames;
            pstRawCapCtx->astRawCapTaskCtx[i].stRawCapCfg.stGetFilenameCb.pvPrivateData = &pstRawCapCtx->astRawCapTaskCtx[i];
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_RawCapCreateTask(HI_VOID)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
            && (pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bEnable))
        {
            s32Ret = HI_RAWCAP_CreateTask(&pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].stRawCapCfg,
                                          &pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].RawCapTaskHdl);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" create rawcap failed,s32Idx:%d,s32Ret[0x%x]\n\n"NONE, s32Idx, s32Ret);
            }
            else
            {
                pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bRawCapTaskCreate = HI_TRUE;
            }
        }
    }

    /*TODO: how to process when just one rawcap task create success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" create rawcap task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_RawCapDestroy(HI_VOID)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable)
           &&(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
           &&(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bEnable)
           &&(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bRawCapTaskCreate))
        {
            s32Ret = HI_RAWCAP_DestroyTask(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].RawCapTaskHdl);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" destroy rawcap task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].RawCapTaskHdl, s32Ret);
            }
            else
            {
                pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bRawCapTaskCreate = HI_FALSE;
            }
        }
    }

    /*TODO: how to process when just one rawcap task destroy success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" destroy rawcap task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_StartRawCap(HI_VOID)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
            &&(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bEnable))
        {
            s32Ret = HI_RAWCAP_StartTask(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].RawCapTaskHdl, 60000, PDT_STATEMNG_RAWCAP_DEPTH);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" start rawcap task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl, s32Ret);
            }
        }
    }

    /*TODO: how to process when just one rawcap task start success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" start rawcap task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_StopRawCap(HI_VOID)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
            &&(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].bEnable))
        {
            s32Ret = HI_RAWCAP_StopTask(pstStateMngCtx->stRawCapCtx.astRawCapTaskCtx[s32Idx].RawCapTaskHdl);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" stop rawcap task[%d] failed\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);
            }
            else
            {
                MLOGD(YELLOW" stop rawcap task success,s32Idx:%d \n\n"NONE, s32Idx);
            }
        }
    }

    /*TODO: how to process when just one rawcap task stop success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" stop rawcap task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

#endif

HI_S32 PDT_STATEMNG_AencStart(HI_HANDLE AencHdl)
{
    HI_S32 s32Idx = 0, s32Ret = HI_SUCCESS;
    PDT_STATEMNG_RECTASK_CONTEXT_S *pstContext = NULL;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        pstContext = &pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx];
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstContext->bEnable))
        {
            if(AencHdl == pstContext->stRecMngAttr.astFileAttr[0].stDataSource.aAencHdl[0])
            {
                if(HI_FALSE == pstContext->bAudioStarted)
                {
                    pstContext->bAudioStarted = HI_TRUE;
                    break;
                }
            }

        }
    }

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        pstContext = &pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx];
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && /** cam enable*/
                (pstContext->bEnable) && /** record task enable*/
                (pstContext->stRecMngAttr.astFileAttr[0].stDataSource.u32AencCnt != 0)) {
            if(HI_FALSE == pstContext->bAudioStarted)
            {
                MLOGI("AENC of record task[%d] not start,can not start aenc\n",s32Idx);
                break;
            }
        }
    }
    if(s32Idx == HI_PDT_RECTASK_MAX_CNT)
    {
        s32Ret = HI_PDT_MEDIA_AencStart(AencHdl);
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "AencStart");
    }
    return s32Ret;
}

HI_S32 PDT_STATEMNG_AencStop(HI_HANDLE AencHdl)
{
    HI_S32 s32Idx = 0, s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    PDT_STATEMNG_RECTASK_CONTEXT_S *pstContext = NULL;

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        pstContext = &pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx];
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstContext->bEnable))
        {
            if(AencHdl == pstContext->stRecMngAttr.astFileAttr[0].stDataSource.aAencHdl[0])
            {
                if(HI_TRUE == pstContext->bAudioStarted)
                {
                    pstContext->bAudioStarted = HI_FALSE;
                    break;
                }
            }

        }
    }

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        pstContext = &pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx];
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) &&  /** cam enable*/
                (pstContext->bEnable) &&    /** record task enable*/
                (pstContext->stRecMngAttr.astFileAttr[0].stDataSource.u32AencCnt != 0)) {
            if(HI_TRUE == pstContext->bAudioStarted)
            {
                MLOGI("AENC of record task[%d] not stop,can not stop aenc\n",s32Idx);
                break;
            }
        }
    }
    if(s32Idx == HI_PDT_RECTASK_MAX_CNT)
    {
        s32Ret = HI_PDT_MEDIA_AencStop(AencHdl);
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "AencStop");
    }
    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetMediaMode(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS, s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_PDT_MEDIAMODE_E enSettingMediaMode;
    memcpy(&enSettingMediaMode, pstMsg->aszPayload, sizeof(HI_PDT_MEDIAMODE_E));
    MLOGD(YELLOW"setting MediaMode(%d) for workmode(%d)\n\n"NONE, enSettingMediaMode, enSettingWorkMode);

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        if (pstStateMngCtx->s32PreviewCamID == pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID)
        {
            break;
        }
    }

    if (s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("PreviewCamID[%d] error \n", pstStateMngCtx->s32PreviewCamID);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** filter out the same parameter */
    if (enSettingMediaMode == pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.enMediaMode)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingMediaMode value */
    s32Ret = PDT_STATEMNG_CheckCamSettingItemValue(pstMsg, pstStateMngCtx->s32PreviewCamID, (HI_S32)enSettingMediaMode);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"the setting item value not support\n\n"NONE);
        return s32Ret;
    }

    pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.enMediaMode = enSettingMediaMode;
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode, s32Idx, HI_PDT_PARAM_TYPE_MEDIAMODE, &enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "set media mode failed");
    /** get setting MediaModeCfg from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaModeCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stSettingMediaModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StopMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "PDT_STATEMNG_StopMd");
#endif

    /** Reset Media for setting parameter */
    s32Ret = PDT_STATEMNG_ResetMedia(&stSettingMediaModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StartMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "PDT_STATEMNG_StopMd");
#endif

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_MEDIAMODE,
                                      (HI_VOID*)&enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set stSettingMediaMode parameter");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetPlayloadType(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS, s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_MAPI_PAYLOAD_TYPE_E enSettingPlayloadType;
    memcpy(&enSettingPlayloadType, pstMsg->aszPayload, sizeof(HI_MAPI_PAYLOAD_TYPE_E));
    MLOGD(YELLOW"setting PlayloadType(%d) for workmode(%d)\n\n"NONE, enSettingPlayloadType, enSettingWorkMode);

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        if (pstStateMngCtx->s32PreviewCamID == pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID)
        {
            break;
        }
    }

    if (s32Idx >= HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE("PreviewCamID[%d] error \n", pstStateMngCtx->s32PreviewCamID);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** get param to Param module */
    HI_MAPI_PAYLOAD_TYPE_E enCurrentPlayloadType = HI_MAPI_PAYLOAD_TYPE_BUTT;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE,
                                      (HI_VOID*)&enCurrentPlayloadType);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get cam[%d] venc payloadtype failed \n\n"NONE, pstStateMngCtx->s32PreviewCamID);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** filter out the same parameter */
    if (enCurrentPlayloadType == enSettingPlayloadType) /** just set main stream */
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckCamSettingItemValue(pstMsg, pstStateMngCtx->s32PreviewCamID, (HI_S32)enSettingPlayloadType);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"the setting item value not support\n\n"NONE);
        return HI_PDT_STATEMNG_EINVAL;
    }
    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE,
                                      (HI_VOID*)&enSettingPlayloadType);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set main venc payloadtype");

    HI_PDT_MEDIA_VENC_CFG_S stVencCfg;
    memset(&stVencCfg, 0, sizeof(HI_PDT_MEDIA_VENC_CFG_S));
    s32Ret = HI_PDT_PARAM_GetMainVencCfgByPayloadType(enSettingWorkMode, &pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg,
             enSettingPlayloadType, &stVencCfg);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get cam[%d] main VencCfg by Payload type\n\n"NONE, pstStateMngCtx->s32PreviewCamID);
        return HI_PDT_STATEMNG_EINTER;
    }

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "get media config");
    if (HI_PDT_WORKMODE_NORM_REC == pstStateMngCtx->enCurrentWorkMode) {
        HI_PDT_PARAM_RecordUpdateMediaCfg(&stMediaCfg);
    }
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; s32Idx++)
    {
        if(stVencCfg.VencHdl == stMediaCfg.stVideoCfg.astVencCfg[s32Idx].VencHdl)
        {
            memcpy(&stVencCfg.stVencAttr,&stMediaCfg.stVideoCfg.astVencCfg[s32Idx].stVencAttr,
                sizeof(HI_MEDIA_VENC_ATTR_S));
            break;
        }
    }
    /** remove all stream */
    HI_LIVESVR_RemoveAllStream();

#if (!defined(AMP_LINUX_HUAWEILITE))
    /* h265 venc need more MMZ ,rebuild to avoid MMZ hole*/
    if(enCurrentPlayloadType==HI_MAPI_PAYLOAD_TYPE_H264&&enSettingPlayloadType==HI_MAPI_PAYLOAD_TYPE_H265)
    {
        HI_PDT_MEDIA_VideoOutStop();
        HI_PDT_SCENE_Pause(HI_TRUE);
        s32Ret = HI_PDT_MEDIA_Rebuild(&stMediaCfg);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "media rebuild");
        HI_PDT_SCENE_Pause(HI_FALSE);
        HI_PDT_MEDIA_VideoOutStart(HI_NULL,NULL);
    }
    else
#endif
    {
        /** set playload */
        s32Ret = HI_PDT_MEDIA_SetVencAttr(stVencCfg.VencHdl, &stVencCfg.stVencAttr);
        if( HI_SUCCESS != s32Ret )
        {
            MLOGW("set media venc payload type fail,rebuild media!\n");
            HI_PDT_MEDIA_VideoOutStop();
            HI_PDT_SCENE_Pause(HI_TRUE);
            s32Ret = HI_PDT_MEDIA_Rebuild(&stMediaCfg);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "media rebuild");
            HI_PDT_SCENE_Pause(HI_FALSE);
            HI_PDT_MEDIA_VideoOutStart(HI_NULL,NULL);
        }
    }

    /** add all stream */
    s32Ret = PDT_STATEMNG_AddRtspStreams(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "add rtsp streams");

    return s32Ret;
}

static HI_S32 PDT_STATEMNG_SetLapseInterval(HI_MESSAGE_S* pstMsg)
{
    HI_S32 i = 0, s32Idx, s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingLapseInterval = *(HI_U32*)pvPayload;
    MLOGD(YELLOW"set LapseInterval(%d), workmode(%d)\n"NONE, u32SettingLapseInterval, enSettingWorkMode);

    /** filter out the same parameter */
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_RECMNG_ATTR_S* pstRecMngAttr;

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        pstRecMngAttr = &(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr);
        if (pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs == u32SettingLapseInterval)
        {
            i++;
            MLOGI(YELLOW"cam[%d] lapse interval same value\n"NONE,s32Idx );
            continue;
        }
    }
    if (i == HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE(YELLOW"lapse interval same value\n"NONE);
        return HI_SUCCESS;
    }
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID, \
                                 HI_PDT_PARAM_TYPE_LAPSE_INTERVAL,  (HI_VOID*)&u32SettingLapseInterval);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "save LapseInterval param");
    }
    /** get media config from Param module */
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enSettingWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");

    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(enSettingWorkMode, &stEnterWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx,&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset media");

    return s32Ret;
}

static HI_S32 PDT_STATEMNG_SetRecordType(HI_MESSAGE_S* pstMsg)
{
    HI_S32 i = 0, s32Idx, s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingRecordType = *(HI_U32*)pvPayload;
    MLOGI(YELLOW"set u32SettingRecordType(%d), workmode(%d)\n"NONE, u32SettingRecordType, enSettingWorkMode);

    /** filter out the same parameter */
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_RECMNG_ATTR_S* pstRecMngAttr;

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        pstRecMngAttr = &(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr);
        if (pstRecMngAttr->enRecType == u32SettingRecordType)
        {
            i++;
            MLOGI(YELLOW"cam[%d] enRecType same value\n"NONE,s32Idx );
            continue;
        }
        /** filter out wrong enSettingWorkMode/enSettingLapseInterval pair */
        s32Ret = PDT_STATEMNG_CheckCamSettingItemValue(pstMsg, s32Idx, (HI_S32)u32SettingRecordType);
        if (HI_SUCCESS != s32Ret)
        {
            i++;
            MLOGE(RED"set value not support\n"NONE);
            continue;
        }
        pstRecMngAttr->enRecType = u32SettingRecordType;
    }
    if (i == HI_PDT_MEDIA_VCAP_DEV_MAX_CNT)
    {
        MLOGE(YELLOW"enRecType same value\n"NONE);
        return HI_SUCCESS;
    }

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                            pstStateMngCtx->astCamCtx[s32Idx].stMediaModeCfg.s32CamID, \
                            HI_PDT_PARAM_RECORD_TYPE,  (HI_VOID*)&u32SettingRecordType);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "save rectype param");
    }

    /*get media config*/
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enSettingWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");


    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(enSettingWorkMode, &stEnterWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx,&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");


    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset media");


    return s32Ret;
}

#ifdef CONFIG_MOTIONDETECT_ON
static HI_S32 PDT_STATEMNG_SetMdSensitivity(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 j = 0;
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_S32 MdSenNew = *(HI_S32*)pvPayload;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_VIDEODETECT_ALGPROC_CFG_S algProcCfgOld = {0};
    HI_VIDEODETECT_ALGPROC_CFG_S algProcCfgNew = {0};
    HI_HANDLE TaskHdl = pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[PDT_STATEMNG_FRONT_CAMID].VideoDetectTaskHdl;

    algProcCfgOld.algProcType = HI_VIDEODETECT_ALGPROC_TYPE_EVENT;
    algProcCfgOld.svpAlgType = HI_VIDEODETECT_ALG_TYPE_MD;

    algProcCfgNew.algProcType = HI_VIDEODETECT_ALGPROC_TYPE_EVENT;
    algProcCfgNew.svpAlgType = HI_VIDEODETECT_ALG_TYPE_MD;
    algProcCfgNew.unAlgProcAttr.algProcMdAttr.mdSensitivity = MdSenNew;

    HI_PDT_PARAM_VIDEODETECT_CFG_S stVideoDetectCfg;
    s32Ret = HI_PDT_PARAM_GetVideoDetectCfg(PDT_STATEMNG_FRONT_CAMID, &stVideoDetectCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_PDT_PARAM_GetVideoDetectCfg");

    if((HI_TRUE == stVideoDetectCfg.bEnable) && (stVideoDetectCfg.algCnt > 0))
    {
        for(j = 0; j < stVideoDetectCfg.algCnt; j++)
        {
            if(HI_VIDEODETECT_ALG_TYPE_MD == stVideoDetectCfg.algAttr[j].algType)
            {
                algProcCfgOld.unAlgProcAttr.algProcMdAttr.mdSensitivity = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdSensitivity;
                algProcCfgOld.unAlgProcAttr.algProcMdAttr.mdStableCnt = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdStableCnt;
                algProcCfgNew.unAlgProcAttr.algProcMdAttr.mdStableCnt = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdStableCnt;
                break;
            }
        }
    }

    if(MdSenNew == algProcCfgOld.unAlgProcAttr.algProcMdAttr.mdSensitivity)
    {
        MLOGE(YELLOW"Md Sensitivity same value\n"NONE);
        return HI_SUCCESS;
    }
    if((MdSenNew > 0) && (algProcCfgOld.unAlgProcAttr.algProcMdAttr.mdSensitivity == 0))
    {
        s32Ret = HI_VIDEODETECT_AddAlgProc(TaskHdl,  &algProcCfgNew);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_AddAlgProc");
    }
    else if((MdSenNew == 0) && (algProcCfgOld.unAlgProcAttr.algProcMdAttr.mdSensitivity > 0))
    {
        s32Ret = HI_VIDEODETECT_RemoveAlgProc(TaskHdl, &algProcCfgOld);
        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "HI_VIDEODETECT_RemoveAlgProc");
    }
    else
    {
        s32Ret = HI_VIDEODETECT_RemoveAlgProc(TaskHdl, &algProcCfgOld);
        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "HI_VIDEODETECT_RemoveAlgProc");
        s32Ret = HI_VIDEODETECT_AddAlgProc(TaskHdl,  &algProcCfgNew);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_AddAlgProc");
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      PDT_STATEMNG_FRONT_CAMID,
                                      HI_PDT_PARAM_MD_SENSITIVITY,
                                      (HI_VOID*)&MdSenNew);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "SetCamParam");

    return s32Ret;
}
#endif

HI_S32 PDT_STATEMNG_RecStatesSettingMsgProc(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (pstMsg->arg1)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_STATEMNG_RecStatesSetMediaMode(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_STATEMNG_RecStatesSetPlayloadType(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_STATEMNG_SetFlip(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_MIRROR:
            s32Ret = PDT_STATEMNG_SetMirror(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_STATEMNG_SetLDC(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_WDR:
            s32Ret = PDT_STATEMNG_SetWDR(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_VIDEOMODE:
            s32Ret = PDT_STATEMNG_SetVideoMode(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_CROP:
            s32Ret = PDT_STATEMNG_SetCrop(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_REC:
            s32Ret = PDT_STATEMNG_SetRecEnable(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_STATEMNG_SetOSD(pstMsg);
            break;

        case HI_PDT_PARAM_TYPE_AUDIO:
            s32Ret = PDT_STATEMNG_SetAudio(pstMsg);
            break;
        case HI_PDT_PARAM_TYPE_SPLITTIME:
            s32Ret = PDT_STATEMNG_SetRecSplitTime(pstMsg);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_STATEMNG_SetLapseInterval(pstMsg);
            break;
        case HI_PDT_PARAM_RECORD_TYPE:
            s32Ret = PDT_STATEMNG_SetRecordType(pstMsg);
            break;
#ifdef CONFIG_MOTIONDETECT_ON
        case HI_PDT_PARAM_MD_SENSITIVITY:
            s32Ret = PDT_STATEMNG_SetMdSensitivity(pstMsg);
            break;
#endif
        default:
            MLOGE(YELLOW"not support param type(%d)\n\n"NONE, pstMsg->arg1);
            return HI_PDT_STATEMNG_EINVAL;
    }

    return s32Ret;
}

/** enter Rec states */
HI_S32 PDT_STATEMNG_RecStatesEnter(HI_VOID* pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR, "parameter pvArg");

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD(YELLOW"try to enter state(%s), bRunning(%d)\n\n"NONE, pstStateAttr->stState.name, (pstStateAttr->bRunning));
    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "change state to workmode");

    if (HI_PDT_WORKMODE_NORM_REC != enEnterWorkMode)
    {
        MLOGE(RED"enter workmode[%d] error\n\n"NONE, enEnterWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }
    else
    {
        pstStateMngCtx->enCurrentWorkMode = enEnterWorkMode;
        pstStateMngCtx->bRunning = pstStateAttr->bRunning;
    }

    /*get media config*/
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");

    /** generate photo Context */
    for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;s32Idx++)
    {
        s32Ret = PDT_STATEMNG_GeneratePhotoCtx(s32Idx, enEnterWorkMode, &stEnterWorkModeCfg,
                                               &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx]);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate photo context");
        if(HI_TRUE == stEnterMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[s32Idx].astVcapPipeAttr[0].bIspBypass)
        {
            PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].bISPBypss = HI_TRUE;
        }
        else
        {
            PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].bISPBypss = HI_FALSE;
        }
        if(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable == HI_TRUE)
        {
            HI_PHOTOMNG_TASK_CFG_S stPhotoTaskCfg;
            memset(&stPhotoTaskCfg,'\0',sizeof(HI_PHOTOMNG_TASK_CFG_S));
            pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].s32CamID = s32Idx;
            stPhotoTaskCfg.stGetNameCB.pfnGetNameCallBack = PDT_STATEMNG_GetPhotoFileName;
            stPhotoTaskCfg.stGetNameCB.pvPrivateData = &PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx];
            stPhotoTaskCfg.stGetParamCB.pfnGetParamCallBack = PDT_STATEMNG_GetPhotoParam;
            stPhotoTaskCfg.stGetParamCB.pvPrivateData = &PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx];
            MLOGD("photo task create[%d]!!\n",s32Idx);
            s32Ret = HI_PHOTOMNG_TaskCreate(&stPhotoTaskCfg, &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "Photo task create");

            s32Ret = HI_PHOTOMNG_SetPhotoAttr(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl,
                &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].stPhotoAttr);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set photo attr");
        }
    }

    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(enEnterWorkMode, &stEnterWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx,&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset media");

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StartMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "PDT_STATEMNG_StartMd");
#endif


    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    HI_EVENT_S stEvent;
    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_STATEMNG_SWITCH_WORKMODE;
    stEvent.arg2 = enEnterWorkMode;
    stEvent.s32Result = HI_SUCCESS;
    MUTEX_LOCK(pstStateMngCtx->Mutex);
    pstStateMngCtx->bInProgress = HI_FALSE;
    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    s32Ret = HI_EVTHUB_Publish(&stEvent);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event failed\n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }
    else
    {
        MLOGD(YELLOW"publish HI_EVENT_STATEMNG_SWITCH_WORKMODE(%x), arg2(%s) succeed\n\n"NONE,
              stEvent.EventID, pstStateAttr->stState.name);
    }

    /** save poweron workmode */
    s32Ret = PDT_STATEMNG_SetPoweronWorkMode(enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "save poweron workmode param");

    HI_TIME_STAMP;
    return s32Ret;
}

/** exit Rec states */
HI_S32 PDT_STATEMNG_RecStatesExit(HI_VOID* pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx;

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StopMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "PDT_STATEMNG_StopMd");
#endif

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pvArg, HI_PDT_STATEMNG_ENULLPTR, "pvArg");

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD(YELLOW"exit state(%s), bRunning(%d)\n\n"NONE,
          pstStateAttr->stState.name, (pstStateAttr->bRunning));

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "change state to workmode");

    if (HI_PDT_WORKMODE_NORM_REC != enExitWorkMode)
    {
        MLOGE(RED"exit workmode[%d] error \n\n"NONE, enExitWorkMode);
        return HI_PDT_STATEMNG_EINVAL;
    }

    /** exit workmode directly when it is running */
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    if (pstStateAttr->bRunning)
    {
        s32Ret = PDT_STATEMNG_StopRec(HI_TRUE);
        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "stop record");

        /** file data sync */
        HI_async();

        pstStateAttr->bRunning = HI_FALSE;
        MUTEX_LOCK(pstStateMngCtx->Mutex);
        pstStateMngCtx->bRunning = HI_FALSE;
        MUTEX_UNLOCK(pstStateMngCtx->Mutex);
    }

#ifdef CONFIG_RAWCAP_ON
    s32Ret = PDT_STATEMNG_RawCapDestroy();
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "rawcap destroy");
#endif
    for(s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        if(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable == HI_TRUE)
        {
            s32Ret = HI_PHOTOMNG_TaskDestroy(PDT_STATEMNG_GetCtx()->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "photo task destroy");
        }
    }
    return s32Ret;
}

/** Rec states message process */
HI_S32 PDT_STATEMNG_RecStatesMsgProc(HI_MESSAGE_S* pstMsg, HI_VOID* pvArg, HI_STATE_ID* pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;
    static HI_BOOL bPowerAction = HI_TRUE;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstStateMngCtx->bInProgress);

    PDT_STATEMNG_STATE_ATTR_S* pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S*)pvArg;
    MLOGD("current state(%s), bRunning(%d)\n\n", pstStateAttr->stState.name, pstStateAttr->bRunning);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "change state to workmode");

    MLOGD(YELLOW" will process message what(%x) \n\n"NONE, pstMsg->what);
    switch (pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            /** check IsRunning */
            if (HI_TRUE == pstStateAttr->bRunning)
            {
                MLOGI(YELLOW" record task already started, busing\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }

            /** check sd state */
            if (HI_TRUE == pstStateMngCtx->bSDAvailable)
            {
                /** update cam enable flag*/
                s32Ret = PDT_STATEMNG_GetMediaModeCfg(enCurrentWorkMode, pstStateMngCtx->astCamCtx);
                PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get media mode config");

                s32Ret = PDT_STATEMNG_StartRec();
                if(bPowerAction)
                {
                    HI_PrintBootTime("frist normal_recoder time");
                    bPowerAction = HI_FALSE;
                }
#ifdef CONFIG_RAWCAP_ON

                if (pstStateMngCtx->stRawCapCtx.bDebug)
                {
                    PDT_STATEMNG_GenerateRawCapCtx(&pstStateMngCtx->stRawCapCtx);
                    /** create rawcap task */
                    PDT_STATEMNG_RawCapCreateTask();
                    PDT_STATEMNG_StartRawCap();
                }
#endif
            }
            else
            {
                MLOGW(RED"sd not available, start emr record task error\n\n"NONE);
            }

            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateAttr->bRunning = ((HI_TRUE == pstStateMngCtx->bSDAvailable) ? HI_TRUE : HI_FALSE);
            pstStateMngCtx->bRunning = pstStateAttr->bRunning;
            pstStateMngCtx->bInProgress = HI_FALSE;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);
            PDT_STATEMNG_PublishEvent(pstMsg, s32Ret);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_STOP:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            /** check IsRunning */
            if (HI_FALSE == pstStateAttr->bRunning)
            {
                MLOGE(YELLOW"record task not started, no need stop\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }

#ifdef CONFIG_RAWCAP_ON
            if (pstStateMngCtx->stRawCapCtx.bDebug)
            {
                PDT_STATEMNG_StopRawCap();
                PDT_STATEMNG_RawCapDestroy();
            }

#endif
            PDT_STATEMNG_StopRec(pstMsg->arg1);
            if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
                PDT_STATEMNG_EmrTaskEndProc(HI_EVENT_RECMNG_BUTT,HI_RECMNG_MAX_TASK_CNT,HI_TRUE);
            if(HI_TRUE == pstMsg->arg1)
                HI_async();

            HI_BOOL bRecTypeDiff = HI_FALSE;
            HI_REC_TYPE_E enRecType = HI_REC_TYPE_BUTT;
            for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32Idx,
                    HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                if(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType != enRecType)
                {
                    bRecTypeDiff = HI_TRUE;
                     break;
                }
            }
            if(bRecTypeDiff)
            {
                /** change rec type to lapse rec */
                s32Ret = PDT_STATEMNG_ChangeRecType(HI_REC_TYPE_LAPSE);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("statemng change rec type failed! s32Ret:%d\n", s32Ret);
                }
            }

            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateAttr->bRunning = HI_FALSE;
            pstStateMngCtx->bRunning = HI_FALSE;
            pstStateMngCtx->bInProgress = HI_FALSE;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);
            PDT_STATEMNG_PublishEvent(pstMsg, s32Ret);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_TRIGGER:
        {
            HI_BOOL bPhotoEnable = HI_FALSE;
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            if (HI_TRUE == pstStateMngCtx->bSDAvailable)
            {
                for(s32Idx = 0;s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
                {
                    if(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].bEnable && pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
                    {
                        s32Ret = HI_PHOTOMNG_TaskStart(pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl);

                        if (HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED" start photo task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[s32Idx].PhotoMngTaskHdl, s32Ret);
                            PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                            return HI_PROCESS_MSG_RESULTE_OK;
                        }
                        else
                        {
                            bPhotoEnable = HI_TRUE;
                        }
                    }
                }
                if(HI_TRUE != bPhotoEnable)
                {
                    MLOGW("No enabled photo task execution!!\n");
                    PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                }
                else
                {
                    PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
                }
            }
            else
            {
                MLOGE(RED"sd not available, start emr record task error\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
            }

            MLOGD("bInProgress:%d \n", pstStateMngCtx->bInProgress);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            MLOGD(YELLOW"no need to process message HI_EVENT_PHOTOMNG_TASK_END(%x)\n\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_SETTING:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);

            /** check IsRunning */
            if (HI_FALSE != pstStateAttr->bRunning && HI_PDT_PARAM_TYPE_CROP != pstMsg->arg1 )
            {
                MLOGE(YELLOW"task started, setting message can not process\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }

            /** process all the setting type*/
            s32Ret = PDT_STATEMNG_RecStatesSettingMsgProc(pstMsg);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE(YELLOW"set msg proc fail, s32Ret(%d)\n\n"NONE, s32Ret);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
            }
            else
            {
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
            }

            return HI_PROCESS_MSG_RESULTE_OK;
        }

#ifdef CONFIG_RAWCAP_ON
        case HI_EVENT_STATEMNG_DEBUG_SWITCH:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);
            /** check IsRunning */
            if (HI_FALSE != pstStateAttr->bRunning)
            {
                MLOGE(YELLOW"task started, debug switch message can not process\n\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            HI_BOOL bDebugOn;
            memcpy(&bDebugOn, pstMsg->aszPayload, sizeof(HI_BOOL));
            if(bDebugOn == pstStateMngCtx->stRawCapCtx.bDebug)
            {
                MLOGD(YELLOW"same value\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_FAILURE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            pstStateMngCtx->stRawCapCtx.bDebug = bDebugOn;
            MLOGD("set bDebug:%d \n", pstStateMngCtx->stRawCapCtx.bDebug);

            if(HI_FALSE == bDebugOn)
            {
                /** destroy Record task */
                s32Ret = PDT_STATEMNG_RawCapDestroy();
                PDT_STATEMNG_CHECK_RET_AND_UPDATESTATUS(pstMsg, s32Ret,s32Ret,"destroy rawcap task");
            }

            /*get media config*/
            HI_PDT_MEDIA_CFG_S stMediaCfg;
            s32Ret = HI_PDT_PARAM_GetMediaCfg(enCurrentWorkMode, pstStateMngCtx->astCamCtx, &stMediaCfg);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get media config");

            /**update disp configure*/
            s32Ret = PDT_STATEMNG_UpdateDispCfg(&stMediaCfg);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "update video out configure");

#ifdef CONFIG_MOTIONDETECT_ON
            s32Ret = PDT_STATEMNG_StopMd();
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "PDT_STATEMNG_StartMd");
#endif

            /** Reset Media for setting parameter */
            s32Ret = PDT_STATEMNG_ResetMedia(&stMediaCfg);
            PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media");

#ifdef CONFIG_MOTIONDETECT_ON
            s32Ret = PDT_STATEMNG_StartMd();
            PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "PDT_STATEMNG_StopMd");
#endif

            PDT_STATEMNG_UPDATESTATUS(pstMsg, HI_SUCCESS, HI_FALSE);
            return HI_PROCESS_MSG_RESULTE_OK;
        }

#endif

        /** need pre-process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            if (HI_STORAGE_STATE_DEV_UNPLUGGED == pstStateMngCtx->enPowerOnState)
            {
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            else if(HI_STORAGE_STATE_IDEL == pstStateMngCtx->enPowerOnState)
            {
                /**let base state to chage POS be unpluggedcd*/
                MLOGI("Storage PowerOn State is %d.\n",pstStateMngCtx->enPowerOnState);
                return HI_PROCESS_MSG_UNHANDLER;
            }
        case HI_EVENT_AHDMNG_PLUG_STATUS:
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_SYATEM_REBOOT:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        {
            if (HI_TRUE == pstStateAttr->bRunning)
            {
                PDT_STATEMNG_StopRec(HI_TRUE);
                /** file data sync */
                sync();
                MUTEX_LOCK(pstStateMngCtx->Mutex);
                pstStateAttr->bRunning = HI_FALSE;
                pstStateMngCtx->bRunning = HI_FALSE;
                MUTEX_UNLOCK(pstStateMngCtx->Mutex);
            }

            /** sd destroy */
            HI_STORAGEMNG_CFG_S stStorageMngCfg;
            memset(&stStorageMngCfg, 0, sizeof(HI_STORAGEMNG_CFG_S));
            HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
            HI_STORAGEMNG_Destroy(stStorageMngCfg.szMntPath);
            return HI_PROCESS_MSG_UNHANDLER;/** need base state precess */
        }

        /** RecMng error */
        case HI_EVENT_RECMNG_ERROR:
        {
            if (HI_TRUE == pstStateAttr->bRunning)
            {
#ifdef CONFIG_RAWCAP_ON
                if (pstStateMngCtx->stRawCapCtx.bDebug)
                {
                    PDT_STATEMNG_StopRawCap();
                }
#endif
                /** for debug HI_EVENT_RECMNG_ERROR ,note statement below temporary*/
                //PDT_STATEMNG_StopRec(HI_TRUE);

                MUTEX_LOCK(pstStateMngCtx->Mutex);
                pstStateAttr->bRunning = HI_FALSE;
                pstStateMngCtx->bRunning = HI_FALSE;
                pstStateMngCtx->bInProgress = HI_FALSE;
                MUTEX_UNLOCK(pstStateMngCtx->Mutex);

                HI_EVENT_S stEvent;
                memset(&stEvent, 0 , sizeof(HI_EVENT_S));
                stEvent.EventID = HI_EVENT_STATEMNG_STOP;
                stEvent.arg2 = enCurrentWorkMode;
                stEvent.s32Result = HI_SUCCESS;
                HI_EVTHUB_Publish(&stEvent);
                if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
                {
                    MUTEX_LOCK(pstStateMngCtx->Mutex);
                    pstStateMngCtx->stRecCtx.bEMRRecord = HI_FALSE;
                    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

                    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
                    stEvent.EventID = HI_EVENT_STATEMNG_EMR_END;
                    stEvent.arg2 = enCurrentWorkMode;
                    stEvent.s32Result = HI_SUCCESS;
                    HI_EVTHUB_Publish(&stEvent);
                }
            }

            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_STATEMNG_EMR_BEGIN:
        {
            PDT_STATEMNG_CHECK_MSG_WORKMODE(enCurrentWorkMode, pstMsg->arg2, pstMsg, pstStateMngCtx->bInProgress);
            /** check sd state */
            HI_BOOL bRunning = HI_FALSE;
            HI_BOOL bEMRRecord = HI_FALSE;

            if(HI_FALSE == pstStateMngCtx->stRecCtx.bEMRRecord)
            {
                if(HI_TRUE == pstStateMngCtx->bSDAvailable)
                {
                    HI_BOOL bForceNormRec = HI_FALSE;
                    for(s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
                    {
                        if(HI_REC_TYPE_LAPSE == pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType)
                        {
                            bForceNormRec = HI_TRUE;
                            break;
                        }
                    }
                    if(bForceNormRec)
                    {
                        /** stop lapse rec */
                        if(HI_TRUE == pstStateMngCtx->bRunning)
                        {
                            s32Ret = PDT_STATEMNG_StopRec(HI_TRUE);
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE("stop rec failed! s32Ret:%d\n", s32Ret);
                            }
                        }

                        /** change rec type to norm rec*/
                        s32Ret = PDT_STATEMNG_ChangeRecType(HI_REC_TYPE_NORMAL);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE("statemng change rec type failed! s32Ret:%d\n", s32Ret);
                        }

                        /** start norm rec */
                        s32Ret = PDT_STATEMNG_StartRec();
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE("start rec failed! s32Ret:%d\n", s32Ret);
                        }

                        s32Ret |= PDT_STATEMNG_RecManualSplit(HI_TRUE);
                    }
                    else
                    {
                        if(HI_TRUE != pstStateAttr->bRunning)
                        {
                            s32Ret = PDT_STATEMNG_StartRec();
                            MLOGW(RED"start emr record task,normal rec started[%d] first\n\n"NONE,s32Ret);
                        }
                        s32Ret |= PDT_STATEMNG_RecManualSplit(HI_FALSE);
                    }

                    bEMRRecord = ((HI_SUCCESS == s32Ret)?HI_TRUE : HI_FALSE);
                    if(HI_TRUE != pstStateAttr->bRunning)
                        bRunning = bEMRRecord;
                    else
                        bRunning = HI_TRUE;

                }
                else
                {
                    s32Ret = HI_FAILURE;
                    MLOGE(RED"sd not available, start emr record task error\n\n"NONE);
                }
            }
            else
            {
                bRunning = HI_TRUE;
                bEMRRecord = HI_TRUE;
                s32Ret = HI_FAILURE;
                MLOGE(RED"emr record task started, start emr record task error\n\n"NONE);
            }
            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateMngCtx->bInProgress = HI_FALSE;
            pstStateAttr->bRunning = bRunning;
            pstStateMngCtx->bRunning = bRunning;
            pstStateMngCtx->stRecCtx.bEMRRecord = bEMRRecord;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);
            HI_EVENT_S stEvent;
            memset(&stEvent, 0 , sizeof(HI_EVENT_S));
            stEvent.EventID = HI_EVENT_STATEMNG_EMR_BEGIN;
            stEvent.arg2 = enCurrentWorkMode;
            stEvent.s32Result = s32Ret;
            HI_EVTHUB_Publish(&stEvent);

            HI_TIME_STAMP;
            return HI_PROCESS_MSG_RESULTE_OK;
        }

        case HI_EVENT_RECMNG_TASK_AUTO_STOP:
        {
            PDT_STATEMNG_EmrTaskEndProc(pstMsg->what,pstMsg->arg1,HI_FALSE);

            if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
            {
                return HI_PROCESS_MSG_RESULTE_OK; /** need wait another emr record task stop end*/
            }
            HI_BOOL bRecTypeDiff = HI_FALSE;
            HI_REC_TYPE_E enRecType = HI_REC_TYPE_BUTT;
            for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
            {
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, s32Idx,
                    HI_PDT_PARAM_RECORD_TYPE, &enRecType);
                if(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType != enRecType)
                {
                    bRecTypeDiff = HI_TRUE;
                     break;
                }
            }
            if(bRecTypeDiff)
            {
                /** stop norm rec */
                PDT_STATEMNG_StopRec(HI_TRUE);

                /** change rec type to lapse rec */
                s32Ret = PDT_STATEMNG_ChangeRecType(HI_REC_TYPE_LAPSE);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE("statemng change rec type failed! s32Ret:%d\n", s32Ret);
                }

                HI_BOOL bMdReady = HI_TRUE;
#ifdef CONFIG_MOTIONDETECT_ON
                HI_S32  s32MdSensitivity;
                s32Ret = HI_PDT_PARAM_GetCamParam(HI_PDT_WORKMODE_NORM_REC, PDT_STATEMNG_FRONT_CAMID, HI_PDT_PARAM_MD_SENSITIVITY, &s32MdSensitivity);
                HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetWorkModeParam");

                if(0 != s32MdSensitivity)
                {
                    HI_EVENT_S stEvent;
                    stEvent.arg1 = 0;
                    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_VIDEODETECT_MD_MOTIONSTATE_CHANGE, &stEvent);
                    if(((HI_SUCCESS != s32Ret) ||(0 == stEvent.arg1)))
                    {
                        bMdReady = HI_FALSE;
                    }
                    else /**motion detect occur*/
                    {
                        bMdReady = HI_TRUE;
                    }
                }
                else
                {
                    bMdReady = HI_TRUE;
                }

#else
                bMdReady = HI_TRUE;
#endif

                if(bMdReady && pstStateMngCtx->bRunning)
                {
                    /** start lapse rec */
                    s32Ret = PDT_STATEMNG_StartRec();
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE("start rec failed! s32Ret:%d\n", s32Ret);
                    }
                }
                else
                {
                    HI_async();
                    MUTEX_LOCK(pstStateMngCtx->Mutex);
                    pstStateAttr->bRunning = HI_FALSE;
                    pstStateMngCtx->bRunning = HI_FALSE;
                    pstStateMngCtx->bInProgress = HI_FALSE;
                    pstStateMngCtx->stRecCtx.bRecStarted = HI_FALSE;
                    MUTEX_UNLOCK(pstStateMngCtx->Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }

            }
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END:
        {
            PDT_STATEMNG_EmrTaskEndProc(pstMsg->what,pstMsg->arg1,HI_FALSE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
#ifdef CONFIG_TEMPRATURE_CYCLE_ON
        case HI_EVENT_TEMPCYCLE_TEMP_CHANGE:
        {
            HI_BOOL bHighTemp = pstMsg->arg1;
            for(s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
            {
                if(HI_REC_TYPE_LAPSE == pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType)
                {
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }

            s32Ret = PDT_STATEMNG_UpdateTempCycleState(bHighTemp);
            if (HI_SUCCESS != s32Ret)
            {
                 MLOGE("HighTemp State in RecMode Error is %d.\n\n",bHighTemp);
            }

            return HI_PROCESS_MSG_RESULTE_OK;
        }
#endif
        default:
        {
            MLOGD(YELLOW"can not process this message, upload it to Base state\n\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;/** need base state precess */
        }
    }

    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Rec states, add normal Rec states to HFSM, init RecMng module */
HI_S32 PDT_STATEMNG_RecStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stNormalRec =
    {
        {
            HI_PDT_WORKMODE_NORM_REC,
            PDT_STATEMNG_STATE_REC_NORMAL,
            PDT_STATEMNG_RecStatesEnter,
            PDT_STATEMNG_RecStatesExit,
            PDT_STATEMNG_RecStatesMsgProc,
            NULL
        }, HI_FALSE
    };
    stNormalRec.stState.argv = &stNormalRec;
    s32Ret = HI_HFSM_AddState(PDT_STATEMNG_GetCtx()->pvHfsmHdl,
                              &stNormalRec.stState,
                              (HI_STATE_S*) & (pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HFSM add NormalRec state");

    /** init APP Common RecMng module */
    HI_RECMNG_MEDIA_OPERATE_S stMediaOps = {0};
    stMediaOps.pfnVencStart = HI_PDT_MEDIA_VencStart;
    stMediaOps.pfnVencStop = HI_PDT_MEDIA_VencStop;
    stMediaOps.pfnAencStart = PDT_STATEMNG_AencStart;
    stMediaOps.pfnAencStop = PDT_STATEMNG_AencStop;
    stMediaOps.pfnGetAudioInfo = HI_PDT_MEDIA_GetAudioInfo;
    stMediaOps.pfnGetVideoInfo = HI_PDT_MEDIA_GetVideoInfo;
    /* init privdata */
    stMediaOps.pfnPrivDataStart = PDT_STATEMNG_GPSDataStart;
    stMediaOps.pfnPrivDataStop = PDT_STATEMNG_GPSDataStop;
    stMediaOps.pfnGetPrivDataCfg = PDT_STATEMNG_GetGPSDataCfg;

    s32Ret = HI_RECMNG_Init(&stMediaOps);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "Record init");

    return s32Ret;
}

/** deinit RecMng module */
HI_S32 PDT_STATEMNG_RecStatesDeinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit APP Common RecMng module */
    s32Ret = HI_RECMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "Record Deinit");

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
