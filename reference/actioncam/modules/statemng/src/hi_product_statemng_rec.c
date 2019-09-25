/**
 * \file   hi_product_statemng_rec.c
 * \brief  Realize the interface about record states.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
 */
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern PDT_STATEMNG_CONTEXT g_stSTATEMNGContext;

static HI_U32 PDT_STATEMNG_GCD(HI_U32 x,HI_U32 y)
{
    HI_U32 u32Max = (x>y)?x:y;
    HI_U32 u32Min = (x>y)?y:x;
    HI_U32 z = u32Min;
    while(u32Max%u32Min!=0)
    {
        z = u32Max%u32Min;
        u32Max = u32Min;
        u32Min = z;
    }
    return z;
}

static HI_S32 PDT_STATEMNG_UpdateLapseRecVencAttr(HI_RECMNG_ATTR_S* pstRecMngAttr,
    HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_S32 s32VencIndex)
{
    HI_U32 u32Interval_ms = pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs;
    HI_U32 u32GCD = PDT_STATEMNG_GCD(1000,u32Interval_ms);
    MLOGI(YELLOW"VencIndex(%d), IntervalMs(%d)\n"NONE, s32VencIndex, u32Interval_ms);

    HI_PDT_MEDIA_VENC_CFG_S* pstStreamVencCfg = &pstMediaCfg->stVideoCfg.astVencCfg[s32VencIndex];
    if(pstStreamVencCfg->bEnable)
    {
        HI_MAPI_VENC_ATTR_CBR_S* pstCBRAttr = NULL;
        HI_MAPI_PAYLOAD_TYPE_E  enStreamType = pstStreamVencCfg->stVencAttr.stTypeAttr.enType;
        switch(enStreamType)
        {
            case HI_MAPI_PAYLOAD_TYPE_H264:
                pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH264Cbr.stAttr);
                break;
            case HI_MAPI_PAYLOAD_TYPE_H265:
                pstCBRAttr = &(pstStreamVencCfg->stVencAttr.stRcAttr.unAttr.stH265Cbr.stAttr);
                break;
            default:
               MLOGI(YELLOW"StreamType(%d) err\n"NONE, enStreamType);
        }

        if(pstCBRAttr)
        {
            HI_FR32* pu32srcFrameRate = &pstCBRAttr->u32SrcFrameRate;
            HI_FR32* pu32dstFrameRate = &pstCBRAttr->fr32DstFrameRate;
            HI_U32* pu32BitRate = &pstCBRAttr->u32BitRate;
            MLOGI(YELLOW"BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
                *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);

            *pu32BitRate = (*pu32BitRate)/(u32Interval_ms/1000.0 * PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS);
            *pu32dstFrameRate = (1000/u32GCD)+((u32Interval_ms/u32GCD) << 16);

            if(*pu32BitRate < 2)
            {
                MLOGI(YELLOW"BitRate(%d) small than 2, set BitRate(2)\n"NONE, *pu32BitRate);
                *pu32BitRate = 2;
            }

            MLOGI(YELLOW"updated, BitRate(%d), srcFrameRate(%d), dstFrameRate(%d)\n"NONE,
                *pu32BitRate, *pu32srcFrameRate, *pu32dstFrameRate);
        }
    }

    return HI_SUCCESS;
}

/** Reset all(meida/RecTask) for setting parameter */
HI_S32 PDT_STATEMNG_RecStatesResetAll(HI_HANDLE RecMngTaskHdl, HI_PDT_MEDIA_CFG_S *pstMediaCfg,
    HI_RECMNG_ATTR_S *pstRecMngAttr, HI_BOOL bResetMedia)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** destoty RecMng task */
    s32Ret = HI_RECMNG_DestroyTask(RecMngTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"destroy RecMng task");

    /** reset Media */
    if(HI_TRUE == bResetMedia)
    {
        s32Ret = PDT_STATEMNG_ResetMedia(pstMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
        PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");
    }

    /** create RecMng task */
    s32Ret = HI_RECMNG_CreateTask(pstRecMngAttr, &RecMngTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret,"RecMng create task");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetMediaMode(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_MEDIAMODE_E enSettingMediaMode = *(HI_PDT_MEDIAMODE_E *)pvPayload;
    MLOGD(YELLOW"set MediaMode(%d), workmode(%d)\n"NONE, enSettingMediaMode, enSettingWorkMode);

    /** filter out the same parameter */
    if(enSettingMediaMode == g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingMediaMode value */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingMediaMode);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, enSettingMediaMode,
        &stSettingMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stSettingMediaCfg, pstMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** update venc attr*/
    if((HI_PDT_WORKMODE_LPSE_REC == enSettingWorkMode) &&
       (pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs < 64*1000))
    {
        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(pstRecMngAttr,
            pstMediaCfg, PDT_STATEMNG_MAIN_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update main stream venc attr");

        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(pstRecMngAttr,
            pstMediaCfg, PDT_STATEMNG_SUB_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update sub stream venc attr");
    }

    /** Reset all(meida/RecTask) for setting parameter */
    s32Ret = PDT_STATEMNG_RecStatesResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enSettingMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save MediaMode param");
    MLOGD(YELLOW"save MediaMode(%d) param succeed\n"NONE, enSettingMediaMode);

    g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode = enSettingMediaMode;

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetLapseInterval(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingLapseInterval = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set LapseInterval(%d), workmode(%d)\n"NONE, u32SettingLapseInterval, enSettingWorkMode);

    /** filter out the same parameter */
    if(pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs == u32SettingLapseInterval)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingLapseInterval pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)u32SettingLapseInterval);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs = u32SettingLapseInterval;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stSettingMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode,
        &stSettingMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stSettingMediaCfg, pstMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** update venc attr*/
    if((HI_PDT_WORKMODE_LPSE_REC == enSettingWorkMode) &&
       (pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs < 64*1000))
    {
        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(pstRecMngAttr,
            pstMediaCfg, PDT_STATEMNG_MAIN_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update main stream venc attr");

        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(pstRecMngAttr,
            pstMediaCfg, PDT_STATEMNG_SUB_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update sub stream venc attr");
    }

    /** Reset RecTask for setting parameter */
    PDT_STATEMNG_RecStatesResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_TRUE);

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_LAPSE_INTERVAL, (HI_VOID *)&u32SettingLapseInterval);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save LapseInterval param");
    MLOGD(YELLOW"save LapseInterval(%d) parm succeed\n"NONE, u32SettingLapseInterval);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetLoopTime(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingLoopTime = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set LoopTime(%d), workmode(%d)\n"NONE, u32SettingLoopTime, enSettingWorkMode);

    /** filter out the same parameter */
    if(pstRecMngAttr->stLoopAttr.u32LoopNum == u32SettingLoopTime)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingLoopTime pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)u32SettingLoopTime);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    pstRecMngAttr->stLoopAttr.u32LoopNum = u32SettingLoopTime;/** asumme one min per file now */

    /** Reset RecTask for setting parameter */
    PDT_STATEMNG_RecStatesResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_FALSE);

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_LOOP_TIME, (HI_VOID *)&u32SettingLoopTime);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save LoopTime param");
    MLOGD(YELLOW"save LoopTime(%d) param succeed\n"NONE, u32SettingLoopTime);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSetPlayloadType(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_MAPI_PAYLOAD_TYPE_E enSettingPlayloadType = *(HI_MAPI_PAYLOAD_TYPE_E *)pvPayload;
    MLOGD(YELLOW"set PlayloadType(%d), workmode(%d)\n"NONE, enSettingPlayloadType, enSettingWorkMode);

    /** get param to Param module */
    HI_MAPI_PAYLOAD_TYPE_E enCurrentPlayloadType = HI_MAPI_PAYLOAD_TYPE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, (HI_VOID *)&enCurrentPlayloadType);
    MLOGD(YELLOW"curr PlayloadType(%d), workmode(%d)\n"NONE, enCurrentPlayloadType, enSettingWorkMode);

    /** filter out the same parameter */
    if(enCurrentPlayloadType == enSettingPlayloadType)/** just set main stream */
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingPlayloadType);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, "get mediamode");

    HI_MEDIA_VENC_ATTR_S stVencAttr;
    s32Ret = HI_PDT_PARAM_GetVencAttrByPayloadType(enMediaMode,
        enSettingPlayloadType, &stVencAttr);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get VencAttr");

    HI_BOOL bAddMainStream = HI_FALSE;
    if(HI_PDT_WORKMODE_LPSE_REC != g_stSTATEMNGContext.enCurrentWorkMode)
    {
        /** remove main video stream */
        s32Ret = HI_LIVESVR_RemoveStream(PDT_STATEMNG_MAIN_STREAM_NAME);
        if(HI_SUCCESS == s32Ret)
        {
            MLOGD(YELLOW"remove main stream succeed\n"NONE);
            bAddMainStream = HI_TRUE;
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret, "remove main stream");
        }
    }

    memcpy(&(pstMediaCfg->stVideoCfg.astVencCfg[0].stVencAttr), &stVencAttr,
        sizeof(HI_MEDIA_VENC_ATTR_S));

    /** update venc attr for main stream */
    if((HI_PDT_WORKMODE_LPSE_REC == enSettingWorkMode) &&
       (g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr.unRecordAttr.stLapseAttr.u32IntervalMs < 64*1000))
    {
        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(&(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
            &(g_stSTATEMNGContext.stMediaCfg), PDT_STATEMNG_MAIN_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update main stream venc attr");
    }

    s32Ret = HI_PDT_MEDIA_SetVencAttr(PDT_STATEMNG_MAIN_STREAM_VENC_HDL,
        &(pstMediaCfg->stVideoCfg.astVencCfg[0].stVencAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret, "set venc attr");

    /** Reset all(meida/RecTask) for setting parameter */
    PDT_STATEMNG_RecStatesResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
        pstMediaCfg, pstRecMngAttr, HI_FALSE);

    if(HI_PDT_WORKMODE_LPSE_REC != g_stSTATEMNGContext.enCurrentWorkMode)
    {
        /** add main video stream */
        HI_HANDLE AencHdl = pstMediaCfg->stAudioCfg.astAcapCfg[0].bEnable? 0 : HI_INVALID_HANDLE;
        HI_HANDLE VencHdl = pstMediaCfg->stVideoCfg.astVencCfg[0].VencHdl;

        if(bAddMainStream)
        {
            s32Ret = HI_LIVESVR_AddStream(VencHdl, AencHdl, PDT_STATEMNG_MAIN_STREAM_NAME);
            PDT_STATEMNG_CHECK_RET(s32Ret, "add main stream");
        }
    }
    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE, (HI_VOID *)&enSettingPlayloadType);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save PlayloadType param");
    MLOGD(YELLOW"save PlayloadType(%d) param succeed\n"NONE, enSettingPlayloadType);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_RecStatesSettingMsgProc(HI_MESSAGE_S *pstMsg,
    HI_PDT_MEDIA_CFG_S *pstMediaCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->arg1)
    {
        case HI_PDT_PARAM_TYPE_MEDIAMODE:
            s32Ret = PDT_STATEMNG_RecStatesSetMediaMode(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_LAPSE_INTERVAL:
            s32Ret = PDT_STATEMNG_RecStatesSetLapseInterval(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_LOOP_TIME:
            s32Ret = PDT_STATEMNG_RecStatesSetLoopTime(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_ENC_PAYLOAD_TYPE:
            s32Ret = PDT_STATEMNG_RecStatesSetPlayloadType(pstMsg, pstMediaCfg, pstRecMngAttr);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME:
            //s32Ret = PDT_STATEMNG_SetExpTime(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV:
            s32Ret = PDT_STATEMNG_SetEV(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_ISO:
            s32Ret = PDT_STATEMNG_SetISO(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_WB:
            s32Ret = PDT_STATEMNG_SetWB(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PROTUNE_METRY:
            s32Ret = PDT_STATEMNG_SetMetry(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_AUDIO:
            s32Ret = PDT_STATEMNG_SetAudio(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_FLIP:
            s32Ret = PDT_STATEMNG_SetFlip(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_OSD:
            s32Ret = PDT_STATEMNG_SetOSD(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_DIS:
            s32Ret = PDT_STATEMNG_SetDIS(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_LDC:
            s32Ret = PDT_STATEMNG_SetLDC(pstMsg, pstMediaCfg);
            break;
        case HI_PDT_PARAM_TYPE_PHOTO_SCENE:
        case HI_PDT_PARAM_TYPE_PHOTO_OUTPUT_FMT:
        case HI_PDT_PARAM_TYPE_DELAY_TIME:
        case HI_PDT_PARAM_TYPE_BURST_TYPE:
        default:
            MLOGD(YELLOW"not support type(%d)\n"NONE, pstMsg->arg1);
            return HI_FAILURE;
    }

    return s32Ret;
}

/** enter Rec states */
HI_S32 PDT_STATEMNG_RecStatesEnter(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"enter state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get enter workmode and check it */
    HI_PDT_WORKMODE_E enEnterWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enEnterWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"change state to workmode");
    if((HI_PDT_WORKMODE_NORM_REC != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_LOOP_REC != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_LPSE_REC != enEnterWorkMode) &&
       (HI_PDT_WORKMODE_SLOW_REC != enEnterWorkMode))
    {
        MLOGE(RED"param err\n"NONE);
        return HI_FAILURE;
    }
    else
    {
        g_stSTATEMNGContext.enCurrentWorkMode = enEnterWorkMode;
        g_stSTATEMNGContext.bRunning = HI_FALSE;
    }

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enEnterWorkMode, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get WorkModeCfg");

    /** get enter mediamode */
    HI_PDT_REC_COMM_ATTR_S *pstRecCommAttr = NULL;
    switch(enEnterWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            pstRecCommAttr = &(stEnterWorkModeCfg.unModeAttr.stNormRecAttr.stCommAttr);
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            pstRecCommAttr = &(stEnterWorkModeCfg.unModeAttr.stLoopRecAttr.stCommAttr);
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            pstRecCommAttr = &(stEnterWorkModeCfg.unModeAttr.stLapseRecAttr.stCommAttr);
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            pstRecCommAttr = &(stEnterWorkModeCfg.unModeAttr.stSlowRecAttr.stCommAttr);
            break;
        default:
        {
            MLOGE(RED"param err\n"NONE);
            return HI_FAILURE;
        }
    }

    g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl = PDT_STATEMNG_INVALID_HANDLE;
    g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode = pstRecCommAttr->enMediaMode;
    g_stSTATEMNGContext.stRecMngCtx.u8FileMngObjCfgIdx = pstRecCommAttr->u8FileTypeIdx;
    MLOGD(YELLOW"enEnterMediaMode(%d), u8FileTypeIdx(%d)\n"NONE,
                g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode,
                g_stSTATEMNGContext.stRecMngCtx.u8FileMngObjCfgIdx);

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enEnterWorkMode, g_stSTATEMNGContext.stRecMngCtx.enEnterMediaMode,
        &stEnterMediaCfg, &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"get MediaCfg");

    /** generate MediaCfg */
    s32Ret = PDT_STATEMNG_GenerateMediaCfg(&stEnterMediaCfg, &(g_stSTATEMNGContext.stMediaCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate MediaCfg");

    /** generate RecMng atrribute */
    s32Ret = PDT_STATEMNG_GenerateRecMngAttr(enEnterWorkMode, &stEnterWorkModeCfg,
                                    &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr));
    PDT_STATEMNG_CHECK_RET(s32Ret,"generate recordmng attr");

    /** update venc attr*/
    if((HI_PDT_WORKMODE_LPSE_REC == enEnterWorkMode) &&
       (g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr.unRecordAttr.stLapseAttr.u32IntervalMs < 64*1000))
    {
        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(&(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
            &(g_stSTATEMNGContext.stMediaCfg), PDT_STATEMNG_MAIN_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update main stream venc attr");

        s32Ret = PDT_STATEMNG_UpdateLapseRecVencAttr(&(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
            &(g_stSTATEMNGContext.stMediaCfg), PDT_STATEMNG_SUB_STREAM_VENC_HDL);
        PDT_STATEMNG_CHECK_RET(s32Ret,"update sub stream venc attr");
    }

    /** reset Media */
    s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg), &(g_stSTATEMNGContext.stSceneModeCfg));
    PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

    /** set system configure */
    PDT_STATEMNG_SetSysConfig();

    /** create RecMng task */
    s32Ret = HI_RECMNG_CreateTask(&(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr),
        &(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl));
    PDT_STATEMNG_CHECK_RET(s32Ret,"create RecMng task");

    /** publish HI_EVENT_STATEMNG_SWITCH_WORKMODE event to EventHub */
    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SWITCH_WORKMODE,
                0, enEnterWorkMode, HI_TRUE, 0L, NULL, 0);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"publish switch workmode failed(0x%08X)\n"NONE, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_FAILURE;
    }
    else
    {
        MLOGD(YELLOW"publish switch to workmode(%s) msg(0x%08X) succeed\n"NONE,
            pstStateAttr->stState.name, HI_EVENT_STATEMNG_SWITCH_WORKMODE);
        g_stSTATEMNGContext.bInProgress = HI_FALSE;
    }
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    /** save poweron workmode */
    s32Ret = PDT_STATEMNG_SavePoweronWorkMode(g_stSTATEMNGContext.enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save poweron workmode");

    if(g_stSTATEMNGContext.bSuspend)
    {
#ifdef CONFIG_SCREEN
        s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set display state(on)");

        s32Ret = HI_HAL_SCREEN_SetBackLightState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set back light state(on)");
#endif
        g_stSTATEMNGContext.bSuspend = HI_FALSE;
    }

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** exit Rec states */
HI_S32 PDT_STATEMNG_RecStatesExit(HI_VOID *pvArg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_NULL_PTR(pvArg,"pvArg");

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD(YELLOW"exit state(%s)\n"NONE, pstStateAttr->stState.name);

    /** get exit workmode and check it */
    HI_PDT_WORKMODE_E enExitWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enExitWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    if((HI_PDT_WORKMODE_NORM_REC != enExitWorkMode) &&
       (HI_PDT_WORKMODE_LOOP_REC != enExitWorkMode) &&
       (HI_PDT_WORKMODE_LPSE_REC != enExitWorkMode) &&
       (HI_PDT_WORKMODE_SLOW_REC != enExitWorkMode))
    {
        MLOGE(RED"param error\n"NONE);
        return HI_FAILURE;
    }

    /** exit workmode directly when it is running */
    if(g_stSTATEMNGContext.bRunning)
    {
        s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
            HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
        PDT_STATEMNG_CHECK_RET(s32Ret,"stop RecMng task");

        /** file data sync */
        sync();

        MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
        g_stSTATEMNGContext.bRunning = HI_FALSE;
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    }

    if(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl != PDT_STATEMNG_INVALID_HANDLE)
    {
        s32Ret = HI_RECMNG_DestroyTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGW(RED"destroy RecMng task failed(0x%08X)\n"NONE, s32Ret);
            s32Ret = HI_SUCCESS;
        }
        g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl = PDT_STATEMNG_INVALID_HANDLE;
    }

    return s32Ret;
}

/** Rec states message process */
HI_S32 PDT_STATEMNG_RecStatesMsgProc(HI_MESSAGE_S *pstMsg, HI_VOID *pvArg, HI_STATE_ID *pStateID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    PDT_STATEMNG_CHECK_MSGPROC_FUNC_PARAM(g_stSTATEMNGContext.Mutex, pvArg, pStateID,
        pstMsg, g_stSTATEMNGContext.bInProgress);

    PDT_STATEMNG_STATE_ATTR_S *pstStateAttr = (PDT_STATEMNG_STATE_ATTR_S *)pvArg;
    MLOGD("curr state(%s)\n", pstStateAttr->stState.name);

    HI_PDT_WORKMODE_E enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s32Ret = PDT_STATEMNG_State2WorkMode(pstStateAttr->stState.name, &enCurrentWorkMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode");

    switch(pstMsg->what)
    {
        case HI_EVENT_STATEMNG_START:
        {
            MLOGD(YELLOW"process start msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                g_stSTATEMNGContext.stRecMngCtx.bFileMngNewGrp = HI_TRUE;
                s32Ret = HI_RECMNG_StartTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    PDT_STATEMNG_RECMNG_OPERATION_TIMEOUT_MS);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"start RecMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
#ifdef CONFIG_RAWCAP_ON
                    if(HI_TRUE == g_stSTATEMNGContext.bRawCapOn)
                    {
                        /** create and start raw capture task */
                        HI_PDT_WORKMODE_E enCurrWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
                        if ((HI_PDT_WORKMODE_NORM_REC == enCurrWorkMode) &&
                            (HI_FALSE == g_stSTATEMNGContext.bRawCapTaskCreate))
                        {

                            g_stSTATEMNGContext.stRawCapCfg.u32RawDepth = PDT_STATEMNG_RAWCAP_REC_DEPTH;
                            PDT_STATEMNG_GetRawCapTaskSrc(enCurrWorkMode,
                                &(g_stSTATEMNGContext.stRawCapCfg.stDataSource));
                            g_stSTATEMNGContext.stRawCapCfg.stGetFilenameCb.pfnGetFileNameCb =
                                PDT_STATEMNG_GetRawCapFileNames;
                            g_stSTATEMNGContext.stRawCapCfg.bRawFileSeparated = HI_TRUE;
                            g_stSTATEMNGContext.stRawCapCfg.stDataSource.bDumpYUV = HI_FALSE;

                            s32Ret = HI_RAWCAP_CreateTask(&(g_stSTATEMNGContext.stRawCapCfg),
                                &(g_stSTATEMNGContext.RawCapTaskHdl));
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE(RED"create Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                            }
                            else
                            {
                                MLOGD(YELLOW"create Rawcap task succeed\n"NONE);
                                g_stSTATEMNGContext.bRawCapTaskCreate = HI_TRUE;
                            }

                            s32Ret = HI_RAWCAP_StartTask(g_stSTATEMNGContext.RawCapTaskHdl,
                                PDT_STATEMNG_RAWCAP_INTERVAL_TIME,
                                g_stSTATEMNGContext.stRawCapCfg.u32RawDepth);
                            if(HI_SUCCESS != s32Ret)
                            {
                                MLOGE(RED"start Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                            }
                            else
                            {
                                MLOGD(YELLOW"start Rawcap task succeed\n"NONE);
                            }
                        }
                    }
#endif
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                    g_stSTATEMNGContext.bRunning = HI_TRUE;
                    g_stSTATEMNGContext.bInProgress = HI_FALSE;
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"already started, busing\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_STATEMNG_STOP:
        {
            MLOGD(YELLOW"process stop msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
#ifdef CONFIG_RAWCAP_ON
                if(HI_TRUE == g_stSTATEMNGContext.bRawCapOn)
                {
                    /** create raw capture task */
                    HI_PDT_WORKMODE_E enCurrWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
                    if ((HI_PDT_WORKMODE_NORM_REC == enCurrWorkMode) &&
                        (HI_TRUE == g_stSTATEMNGContext.bRawCapTaskCreate))
                    {
                        s32Ret = HI_RAWCAP_StopTask(g_stSTATEMNGContext.RawCapTaskHdl);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED"stop Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                        }

                        s32Ret = HI_RAWCAP_DestroyTask(g_stSTATEMNGContext.RawCapTaskHdl);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED"destroy Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                        }
                        else
                        {
                            g_stSTATEMNGContext.bRawCapTaskCreate = HI_FALSE;
                        }
                    }
                }
#endif
                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"stop RecMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
                    /** file data async */
                    s32Ret = HI_async();
                    PDT_STATEMNG_CHECK_RET(s32Ret,"async");

                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;
                    g_stSTATEMNGContext.bInProgress = HI_FALSE;
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"no need stop\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        case HI_EVENT_PHOTOMNG_TASK_END:
        {
            MLOGD(YELLOW"not process photo task end msg(0x%08X)\n"NONE, pstMsg->what);
            return HI_PROCESS_MSG_RESULTE_OK;
        }
        case HI_EVENT_STATEMNG_SETTING:
        {
            MLOGD(YELLOW"process setting msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            /** check IsRunning */
            if(HI_FALSE == g_stSTATEMNGContext.bRunning)
            {
                /** process all the setting type*/
                s32Ret = PDT_STATEMNG_RecStatesSettingMsgProc(pstMsg,
                    &(g_stSTATEMNGContext.stMediaCfg),
                    &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr));

                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"process failed(%d)\n"NONE, s32Ret);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
                else
                {
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"can not process, busy\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                    g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
        /** need pre-process */
        case HI_EVENT_STORAGEMNG_DEV_UNPLUGED:
        case HI_EVENT_STATEMNG_POWEROFF:
        case HI_EVENT_STATEMNG_FACTORY_RESET:
        case HI_EVENT_STATEMNG_SWITCH_WORKMODE:
        {
            if(pstMsg->what == HI_EVENT_STORAGEMNG_DEV_UNPLUGED)
            {
                MLOGD(YELLOW"process unplug msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_POWEROFF)
            {
                MLOGD(YELLOW"process poweroff msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_FACTORY_RESET)
            {
                MLOGD(YELLOW"process factory reset msg(0x%08X)\n"NONE, pstMsg->what);
            }
            else if(pstMsg->what == HI_EVENT_STATEMNG_SWITCH_WORKMODE && pstMsg->arg2 != HI_PDT_WORKMODE_UPGRADE)
            {
                MLOGI(YELLOW"Lorin add -> upload msg to Base state\n"NONE);//将消息上报到base状态处理
                return HI_PROCESS_MSG_UNHANDLER;
            }

            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"stop RecMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;

                    if(pstMsg->what == HI_EVENT_STATEMNG_POWEROFF)
                    {
                        MLOGI(YELLOW"set enExitMode(HI_EXIT_MODE_POWEROFF)\n"NONE);
                        g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_POWEROFF;
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                    else if(pstMsg->what == HI_EVENT_STATEMNG_FACTORY_RESET)
                    {
                        MLOGI(YELLOW"set enExitMode(HI_EXIT_MODE_RESET)\n"NONE);
                        g_stSTATEMNGContext.enExitMode = HI_EXIT_MODE_RESET;
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                    else
                    {
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_PROCESS_MSG_UNHANDLER;
                    }
                }
            }
            else
            {
                MLOGD(YELLOW"upload msg to Base state\n"NONE);
                return HI_PROCESS_MSG_UNHANDLER;
            }
            break;
        }
        /** RecMng error */
        case HI_EVENT_RECMNG_ERROR:
        /** FileMng space full */
        case HI_EVENT_FILEMNG_SPACE_FULL:
        {
            MLOGD(YELLOW"process err msg(0x%08X)\n"NONE, pstMsg->what);
            if(HI_TRUE == g_stSTATEMNGContext.bRunning)
            {
                s32Ret = HI_RECMNG_StopTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                    HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
                if(HI_SUCCESS != s32Ret)
                {
                    MLOGE(RED"stop RecMng task failed(0x%08X)\n"NONE, s32Ret);
                    return HI_FAILURE;
                }
                else
                {
                    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
                    g_stSTATEMNGContext.bRunning = HI_FALSE;

                    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_STOP,
                                0, enCurrentWorkMode, HI_TRUE, 0L, NULL, 0);
                    if(HI_SUCCESS != s32Ret)
                    {
                        MLOGE(RED"publish stop msg failed(0x%08X)\n"NONE, s32Ret);
                        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                        return HI_FAILURE;
                    }
                    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            break;
        }
#ifdef CONFIG_RAWCAP_ON
        case HI_EVENT_STATEMNG_DEBUG_SWITCH:
        {
            MLOGD(YELLOW"process debug switch msg(0x%08X)\n"NONE, pstMsg->what);
            /** check workmode */
            PDT_STATEMNG_CHECK_MSG_WORKMODE(g_stSTATEMNGContext.Mutex, enCurrentWorkMode,
                pstMsg->arg2, pstMsg, g_stSTATEMNGContext.bInProgress);

            if (HI_PDT_WORKMODE_NORM_REC == enCurrentWorkMode)
            {
                /** check IsRunning */
                if(HI_FALSE == g_stSTATEMNGContext.bRunning)
                {
                    HI_VOID* pvPayload = pstMsg->aszPayload;
                    HI_BOOL bDebugOn = *(HI_BOOL *)pvPayload;
                    MLOGD(YELLOW"set bDebugOn(%d), workmode(%d)\n"NONE, bDebugOn, enCurrentWorkMode);

                    if(bDebugOn == g_stSTATEMNGContext.bRawCapOn)
                    {
                        MLOGD(YELLOW"same value\n"NONE);
                        PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                            g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                        return HI_PROCESS_MSG_RESULTE_OK;
                    }

                    if ((HI_FALSE == bDebugOn) &&
                        (HI_TRUE == g_stSTATEMNGContext.bRawCapTaskCreate))
                    {
                        s32Ret = HI_RAWCAP_DestroyTask(g_stSTATEMNGContext.RawCapTaskHdl);
                        if(HI_SUCCESS != s32Ret)
                        {
                            MLOGE(RED"destroy Rawcap task failed(0x%08X)\n"NONE, s32Ret);
                            PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                                g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                            return HI_PROCESS_MSG_RESULTE_OK;
                        }
                        else
                        {
                            g_stSTATEMNGContext.bRawCapTaskCreate = HI_FALSE;
                        }
                    }

                    g_stSTATEMNGContext.bRawCapOn = bDebugOn;

                    PDT_STATEMNG_RecStatesResetAll(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl,
                        &(g_stSTATEMNGContext.stMediaCfg),
                        &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr), HI_TRUE);

                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
                else
                {
                    MLOGD(YELLOW"not process, busy\n"NONE);
                    PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_FALSE, HI_FALSE);
                    return HI_PROCESS_MSG_RESULTE_OK;
                }
            }
            else
            {
                MLOGD(YELLOW"no need process\n"NONE);
                PDT_STATEMNG_UPDATESTATUS(g_stSTATEMNGContext.Mutex,
                        g_stSTATEMNGContext.bInProgress, pstMsg, HI_TRUE, HI_FALSE);
                return HI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        }
#endif
        default:
        {
            MLOGD(YELLOW"upload msg to Base state\n"NONE);
            return HI_PROCESS_MSG_UNHANDLER;
        }
    }

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    g_stSTATEMNGContext.bInProgress = HI_FALSE;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return HI_PROCESS_MSG_RESULTE_OK;
}

/** init Rec states, add normal/loop/lapse/slow Rec states to HFSM, init RecMng module */
HI_S32 PDT_STATEMNG_RecStatesInit(const PDT_STATEMNG_STATE_ATTR_S* pstBase)
{
    HI_S32 s32Ret = HI_SUCCESS;

    static PDT_STATEMNG_STATE_ATTR_S stNormalRec =
        {{HI_PDT_WORKMODE_NORM_REC,
          PDT_STATEMNG_STATE_REC_NORMAL,
          PDT_STATEMNG_RecStatesEnter,
          PDT_STATEMNG_RecStatesExit,
          PDT_STATEMNG_RecStatesMsgProc,
          NULL}};
    stNormalRec.stState.argv = &stNormalRec;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stNormalRec.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add NormalRec");

    static PDT_STATEMNG_STATE_ATTR_S stLoopRec =
        {{HI_PDT_WORKMODE_LOOP_REC,
          PDT_STATEMNG_STATE_REC_LOOP,
          PDT_STATEMNG_RecStatesEnter,
          PDT_STATEMNG_RecStatesExit,
          PDT_STATEMNG_RecStatesMsgProc,
          NULL}};
    stLoopRec.stState.argv = &stLoopRec;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stLoopRec.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add LoopRec");

    static PDT_STATEMNG_STATE_ATTR_S stLapseRec =
        {{HI_PDT_WORKMODE_LPSE_REC,
          PDT_STATEMNG_STATE_REC_LAPSE,
          PDT_STATEMNG_RecStatesEnter,
          PDT_STATEMNG_RecStatesExit,
          PDT_STATEMNG_RecStatesMsgProc,
          NULL}};
    stLapseRec.stState.argv = &stLapseRec;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stLapseRec.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add LapseRec");

    static PDT_STATEMNG_STATE_ATTR_S stSlowRec =
        {{HI_PDT_WORKMODE_SLOW_REC,
          PDT_STATEMNG_STATE_REC_SLOW,
          PDT_STATEMNG_RecStatesEnter,
          PDT_STATEMNG_RecStatesExit,
          PDT_STATEMNG_RecStatesMsgProc,
          NULL}};
    stSlowRec.stState.argv = &stSlowRec;
    s32Ret = HI_HFSM_AddState(g_stSTATEMNGContext.pvHfsmHdl,
        &stSlowRec.stState, (HI_STATE_S*)&(pstBase->stState));
    PDT_STATEMNG_CHECK_RET(s32Ret,"add SlowRec");

    /** init APP Common RecMng module */
    HI_RECMNG_MEDIA_OPERATE_S stMediaOps;
    stMediaOps.pfnVencStart = HI_PDT_MEDIA_VencStart;
    stMediaOps.pfnVencStop = HI_PDT_MEDIA_VencStop;
    stMediaOps.pfnAencStart = HI_PDT_MEDIA_AencStart;
    stMediaOps.pfnAencStop = HI_PDT_MEDIA_AencStop;
    stMediaOps.pfnGetAudioInfo = HI_PDT_MEDIA_GetAudioInfo;
    stMediaOps.pfnGetVideoInfo = HI_PDT_MEDIA_GetVideoInfo;
    stMediaOps.pfnGetPrivDataCfg = NULL;
    s32Ret = HI_RECMNG_Init(&stMediaOps);
    PDT_STATEMNG_CHECK_RET(s32Ret,"init RecMng");

    return s32Ret;
}

/** deinit RecMng module */
HI_S32 PDT_STATEMNG_RecStatesDeinit()
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** deinit APP Common RecMng module */
    s32Ret = HI_RECMNG_Deinit();
    PDT_STATEMNG_CHECK_RET(s32Ret,"deinit RecMng");

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
