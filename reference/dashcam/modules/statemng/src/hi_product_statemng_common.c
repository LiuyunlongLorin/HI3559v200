/**
 * \file   hi_product_statemng_common.c
 * \brief  Realize the common functions for statemng module.
 * \author HiMobileCam Reference Develop Team
 * \date   2017/10/19
*/
#include <unistd.h>
#include "hi_product_statemng.h"
#include "hi_product_statemng_inner.h"
#include "hi_timedtask.h"
#include "hi_liveserver.h"
#include "hi_product_scene.h"
#ifdef HDMI_TEST
#include "hi_mapi_hdmi_define.h"
#include "hi_mapi_hdmi.h"
#endif
#include "hi_mapi_venc.h"
#include "hi_mapi_aenc.h"
#include "hi_gpsmng.h"
#include "hi_recordmng.h"

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#include "hi_tempcycle.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** statemng context */
static PDT_STATEMNG_CONTEXT s_stSTATEMNGContext = {.Mutex = PTHREAD_MUTEX_INITIALIZER,
                            .bInited = HI_FALSE,
                            .enCurrentWorkMode = HI_PDT_WORKMODE_BUTT,
                                                  };

static HI_S32 PDT_STATEMNG_WaitDiskAvailable(HI_U32 u32Timeout_ms)
{
#define POLLING_INTERVAL_MS (300)
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 cnt = u32Timeout_ms/POLLING_INTERVAL_MS;
    HI_S32 s32FsCheckFlag = 0;
    HI_S32 s32Original;
    HI_EVENT_S stEvent;
    s32Original = cnt;
    do
    {
        if (0 == s32FsCheckFlag)
        {
            s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_FS_CHECKING, &stEvent);

            if (HI_SUCCESS == s32Ret)
            {
                cnt = (10 * 1000 / POLLING_INTERVAL_MS) - (s32Original - cnt);
                s32FsCheckFlag = 1;
            }
        }

        s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STATEMNG_SD_AVAILABLE, &stEvent);

        if (HI_SUCCESS == s32Ret)
        {
            return HI_SUCCESS;
        }

        if (cnt >= 1)
        {
            HI_usleep(POLLING_INTERVAL_MS * 1000);
        }
        else
        {
            HI_usleep((u32Timeout_ms % POLLING_INTERVAL_MS) * 1000);
        }
        cnt--;

    }
    while (cnt >= 0);

    return HI_PDT_STATEMNG_EINTER;
}

static HI_S32 PDT_STATEMNG_GetRecTaskFileName(HI_RECMNG_FILENAME_S* pstFilename, HI_VOID* pPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DTCF_FILE_TYPE_E enDtcfFileType = HI_DTCF_FILE_TYPE_BUTT;
    HI_DTCF_DIR_E enDtcfDir = DTCF_DIR_BUTT;
    /** check parameter */
    PDT_STATEMNG_CHECK_POINTER(pstFilename, HI_PDT_STATEMNG_ENULLPTR, " pszFilename");
    PDT_STATEMNG_CHECK_POINTER(pstFilename, HI_PDT_STATEMNG_ENULLPTR, " pPrivData");
    PDT_STATEMNG_RECTASK_CONTEXT_S* pstRecTaskCtx = (PDT_STATEMNG_RECTASK_CONTEXT_S*)pPrivData;

    switch (pstRecTaskCtx->stRecMngAttr.astFileAttr[0].stMuxerCfg.enMuxerType)
    {
        case HI_RECMNG_MUXER_MP4:
            enDtcfFileType = HI_DTCF_FILE_TYPE_MP4;
            break;

        case HI_RECMNG_MUXER_TS:
            enDtcfFileType = HI_DTCF_FILE_TYPE_TS;
            break;

        default:
            MLOGE("unsupport muxer type[%d] \n", pstRecTaskCtx->stRecMngAttr.astFileAttr[0].stMuxerCfg.enMuxerType);
            return HI_PDT_STATEMNG_EINVAL;
    }

    if (0 == pstRecTaskCtx->s32CamID)
    {
        enDtcfDir = DTCF_DIR_NORM_FRONT;
    }
    else if (1 == pstRecTaskCtx->s32CamID)
    {
        enDtcfDir = DTCF_DIR_NORM_REAR;
    }
    else
    {
        MLOGE("record task hanlde[%d] 's  CamID[%d] error \n", pstRecTaskCtx->RecMngTaskHdl, pstRecTaskCtx->s32CamID);
        return HI_PDT_STATEMNG_EINVAL;
    }

    s32Ret = PDT_STATEMNG_WaitDiskAvailable(5000);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "wait disk available timeout");

    /** get file name */
    HI_FILEMNG_OBJ_FILENAME_S stFileName;
    memset(&stFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    s32Ret = HI_FILEMNG_GenerateFileName(enDtcfFileType, enDtcfDir, HI_FALSE, &stFileName);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get rec file name");

    /** copy file name to RecMng array */
    if (pstFilename->u32RecFileCnt > stFileName.u8FileCnt)
    {
        MLOGE("Rec Task[%d] RecFileCnt[%d] is larger than FileCnt[%d] \n", pstRecTaskCtx->RecMngTaskHdl,
              pstFilename->u32RecFileCnt, stFileName.u8FileCnt);
        return HI_PDT_STATEMNG_EINVAL;
    }

    HI_S32 i = 0;

    for (i = 0; i < pstFilename->u32RecFileCnt ; i++)
    {
        snprintf(pstFilename->aszRecFilename[i], HI_APPCOMM_MAX_FILENAME_LEN, "%s", stFileName.szFileName[i]);
        MLOGI("file idx[%d], RecFilename[%s]\n\n", i, pstFilename->aszRecFilename[i]);
    }

#ifdef CONFIG_RAWCAP_ON

    if (s_stSTATEMNGContext.stRawCapCtx.bDebug)
    {
        MLOGD(YELLOW"get prefix for rawcap task name,CamID:%d \n\n"NONE, pstRecTaskCtx->s32CamID);
        sscanf(stFileName.szFileName[0], "%[^.].%*[^.]", s_stSTATEMNGContext.stRawCapCtx.astRawCapTaskCtx[pstRecTaskCtx->s32CamID].szPrefix);
        MLOGD(YELLOW"prefix for rawcap task name[%s]\n\n"NONE, s_stSTATEMNGContext.stRawCapCtx.astRawCapTaskCtx[pstRecTaskCtx->s32CamID].szPrefix);
    }

#endif

    return s32Ret;
}

/** change workmode to state */
HI_S32 PDT_STATEMNG_WorkMode2State(HI_PDT_WORKMODE_E enWorkmode, HI_CHAR* pszStateName, HI_S32 s32NameLength)
{
    /** check parameter */
    if (NULL == pszStateName || s32NameLength < HI_STATE_NAME_LEN)
    {
        MLOGE(RED"pszStateName is NULL or s32NameLength error\n\n"NONE);
        return HI_STORAGEMNG_EINVAL;
    }

    switch (enWorkmode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_REC_NORMAL, sizeof(PDT_STATEMNG_STATE_REC_NORMAL));
            break;
        case HI_PDT_WORKMODE_PHOTO:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_PHOTO, sizeof(PDT_STATEMNG_STATE_PHOTO));
            break;

        case HI_PDT_WORKMODE_PLAYBACK:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_PLAYBACK, sizeof(PDT_STATEMNG_STATE_PLAYBACK));
            break;

        case HI_PDT_WORKMODE_UVC:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_UVC, sizeof(PDT_STATEMNG_STATE_UVC));
            break;

        case HI_PDT_WORKMODE_USB_STORAGE:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_USB_STORAGE, sizeof(PDT_STATEMNG_STATE_USB_STORAGE));
            break;

        case HI_PDT_WORKMODE_PARKING_REC:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_PARKING_REC, sizeof(PDT_STATEMNG_STATE_PARKING_REC));
            break;

        case HI_PDT_WORKMODE_UPGRADE:
            strncpy(pszStateName,
                    PDT_STATEMNG_STATE_UPGRADE, sizeof(PDT_STATEMNG_STATE_UPGRADE));
            break;

        default:
        {

            MLOGE(RED"unsupport workmode[%d] \n\n"NONE, enWorkmode);
            return HI_FAILURE;
        }
    }

    MLOGD("change enWorkmode(%d) to pszStateName(%s)\n\n", enWorkmode, pszStateName);
    return HI_SUCCESS;
}

/** change state to workmode */
HI_S32 PDT_STATEMNG_State2WorkMode(const HI_CHAR* pszStateName, HI_PDT_WORKMODE_E* penWorkmode)
{
    if (!(strncmp(pszStateName,
                  PDT_STATEMNG_STATE_REC_NORMAL, sizeof(PDT_STATEMNG_STATE_REC_NORMAL))))
    {
        *penWorkmode = HI_PDT_WORKMODE_NORM_REC;
    }
    else if (!(strncmp(pszStateName,
                       PDT_STATEMNG_STATE_PLAYBACK, sizeof(PDT_STATEMNG_STATE_PLAYBACK))))
    {
        *penWorkmode = HI_PDT_WORKMODE_PLAYBACK;
    }
    else if (!(strncmp(pszStateName, PDT_STATEMNG_STATE_UVC, sizeof(PDT_STATEMNG_STATE_UVC))))
    {
        *penWorkmode = HI_PDT_WORKMODE_UVC;
    }
    else if (!(strncmp(pszStateName,
                       PDT_STATEMNG_STATE_USB_STORAGE, sizeof(PDT_STATEMNG_STATE_USB_STORAGE))))
    {
        *penWorkmode = HI_PDT_WORKMODE_USB_STORAGE;
    }
    else if (!(strncmp(pszStateName,
                       PDT_STATEMNG_STATE_PARKING_REC, sizeof(PDT_STATEMNG_STATE_PARKING_REC))))
    {
        *penWorkmode = HI_PDT_WORKMODE_PARKING_REC;
    }
    else if (!(strncmp(pszStateName,
                       PDT_STATEMNG_STATE_UPGRADE, sizeof(PDT_STATEMNG_STATE_UPGRADE))))
    {
        *penWorkmode = HI_PDT_WORKMODE_UPGRADE;
    }
    else if (!(strncmp(pszStateName,
                       PDT_STATEMNG_STATE_PHOTO, sizeof(PDT_STATEMNG_STATE_PHOTO))))
    {
        *penWorkmode = HI_PDT_WORKMODE_PHOTO;
    }
    else
    {
        MLOGE(RED"pszStateName is error\n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

/** publish message process result event */
HI_S32 PDT_STATEMNG_PublishEvent(HI_MESSAGE_S* pstMsg, HI_S32 s32Result)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameter */
    PDT_STATEMNG_CHECK_POINTER(pstMsg, HI_PDT_STATEMNG_ENULLPTR, "pstMsg error");

    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(HI_EVENT_S));
    memcpy(&stEvent, pstMsg, sizeof(HI_EVENT_S));
    stEvent.s32Result = s32Result;

    MLOGD("event(%x), s32Result(0x%x)\n\n", stEvent.EventID, (HI_U32)s32Result);
    s32Ret = HI_EVTHUB_Publish(&stEvent);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "publish event");

    HI_TIME_STAMP;
    return s32Ret;
}


#ifdef HDMI_TEST
HI_VOID HDMI_EventPro(HI_HDMI_EVENT_TYPE_E event, HI_VOID* pPrivateData)
{
    switch (event)
    {
        case HI_HDMI_EVENT_HOTPLUG:
            MLOGI("\033[0;32mhdmi HOTPLUG event! \033[0;39m\n");
            break;

        case HI_HDMI_EVENT_NO_PLUG:
            MLOGI("\033[0;31mhdmi NO_PLUG event! \033[0;39m\n");
            break;

        default :
            MLOGE("\033[0;31minvalid hdmi event! \033[0;39m\n");
            break;
    }
}
#endif

HI_S32 PDT_STATEMNG_UpgradeSrchNewPkt(HI_VOID)
{
    HI_S32 s32Ret;
    HI_UPGRADE_DEV_INFO_S stUpgradeDevInfo = {};
    HI_STORAGEMNG_CFG_S stStorageCfg = {};
    HI_PDT_DEV_INFO_S stDevInfo;
    s32Ret = HI_UPGRADE_Init();
    if(HI_UPGRADE_EINITIALIZED == s32Ret)
    {
        MLOGD(YELLOW"upgrade already inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"HI_UPGRADE_Init");
    }
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"GetCommParam");

    memcpy(stUpgradeDevInfo.szModel,stDevInfo.szModel,HI_APPCOMM_COMM_STR_LEN);
    memcpy(stUpgradeDevInfo.szSoftVersion,stDevInfo.szSoftVersion,HI_APPCOMM_COMM_STR_LEN);

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"GetStorageCfg");

    s32Ret = HI_UPGRADE_SrchNewPkt(stStorageCfg.szMntPath, &stUpgradeDevInfo);
    if(HI_SUCCESS != s32Ret && HI_UPGRADE_PKT_AVAILABLE != s32Ret)
    {
        MLOGE(RED"HI_UPGRADE_SrchNewPkt failed\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }
    return s32Ret;
}

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
#define STATEMNG_HIGHTEMPERATURE_FRAMERATE     (15)
HI_S32 PDT_STATEMNG_UpdateTempCycleState(HI_BOOL bHighTemp)
{
    HI_S32 i,j,k = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_MAPI_VCAP_ATTR_S stVCapAttr;
    HI_U32 u32SumPixel = 0;
    HI_U32 u32FrameRate = 0;
    HI_U32 u32VcapPipeIndex = 0;


    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    if (bHighTemp == pstStateMngCtx->stTempCycleCtx.bHighTemp)
    {
        MLOGD("---->The HighTemp state is same.\n");
        return HI_SUCCESS;
    }

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(pstStateMngCtx->enCurrentWorkMode, pstStateMngCtx->astCamCtx, &stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get media config");

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        s32Ret = HI_MAPI_VCAP_GetAttr(stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].VcapDevHdl, &stVCapAttr);
        PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"HI_MAPI_VCAP_GetAttr");

        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if (!stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
            {
                continue;
            }

            u32VcapPipeIndex = stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].VcapPipeHdl;

            if (u32VcapPipeIndex >= HI_MAPI_VCAP_MAX_PIPE_NUM)
            {
                MLOGE("The VcapPipeIndex [%d] is bigger then [%d].\n",u32VcapPipeIndex, HI_MAPI_VCAP_MAX_PIPE_NUM);
                return HI_FAILURE;
            }

            if (bHighTemp)
            {
                if(stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bIspBypass)
                {
                    stVCapAttr.astVcapPipeAttr[u32VcapPipeIndex].stFrameRate.s32DstFrameRate = STATEMNG_HIGHTEMPERATURE_FRAMERATE;
                }
            }
            else
            {
                if(stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bIspBypass)
                {

                    stVCapAttr.astVcapPipeAttr[u32VcapPipeIndex].stFrameRate.s32DstFrameRate =
                        stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].stFrameRate.s32DstFrameRate;
                    u32FrameRate = stVCapAttr.astVcapPipeAttr[u32VcapPipeIndex].stFrameRate.s32DstFrameRate;
                }
            }

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if (bHighTemp && (!stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[u32VcapPipeIndex].bIspBypass))
                {
                    stVCapAttr.astVcapPipeAttr[u32VcapPipeIndex].astPipeChnAttr[k].stFrameRate.s32DstFrameRate = STATEMNG_HIGHTEMPERATURE_FRAMERATE;
                }

                if(!bHighTemp)
                {
                    stVCapAttr.astVcapPipeAttr[u32VcapPipeIndex].astPipeChnAttr[k].stFrameRate.s32DstFrameRate =
                       stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].astPipeChnAttr[0].stFrameRate.s32DstFrameRate;
                }
            }
        }

        s32Ret = HI_MAPI_VCAP_SetAttr(stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].VcapDevHdl, &stVCapAttr);
        PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"HI_MAPI_VCAP_GetAttr");

        if (bHighTemp)
        {
            u32FrameRate = STATEMNG_HIGHTEMPERATURE_FRAMERATE;
        }
        else
        {
            u32FrameRate =
                stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[0].astPipeChnAttr[0].stFrameRate.s32DstFrameRate;
        }


        u32SumPixel += stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].stResolution.u32Width *
            stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i].stResolution.u32Height * u32FrameRate;
    }

    /**SDK current only support 4k30 & 4k15*/
    if (bHighTemp)
    {
        if (u32SumPixel > 3840 * 2160 * STATEMNG_HIGHTEMPERATURE_FRAMERATE)
        {
            u32SumPixel = 3840 * 2160 * STATEMNG_HIGHTEMPERATURE_FRAMERATE;
        }
    }
    else
    {
        if (u32SumPixel > 3840 * 2160 * 30)
        {
            u32SumPixel = 3840 * 2160 * 30;
        }
    }

    /**temperory*/
    HI_MPI_PM_SIGLE_MEDIA_CFG_S stSigleMediaCfg = {0};
    stSigleMediaCfg.enMode = HI_PM_MEDIA_PIPEBE;

    stSigleMediaCfg.unMediaCfg.stViCfg.au32VipipeSumPixel[0] = u32SumPixel;
    MLOGI("Update High Temp [%d]. The SumPixel is %d.\n",bHighTemp, u32SumPixel);

    s32Ret =  HI_TEMPCYCLE_SetPMParam(&stSigleMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"HI_TEMPCYCLE_SetPMParam");

    pstStateMngCtx->stTempCycleCtx.bHighTemp = bHighTemp;

    return HI_SUCCESS;

}
#endif

HI_S32 PDT_STATEMNG_ResetMedia(const HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    static HI_BOOL bFirst =  HI_TRUE;
    if(HI_TRUE == bFirst)
    {
        MLOGI("fastboot ignore first vo stop\n");
    }
    else
    {
        /** Video out stop */
        s32Ret = HI_PDT_MEDIA_VideoOutStop();
        if (HI_PDT_MEDIA_ENOTINIT == s32Ret)
        {
            MLOGD(YELLOW"Media or VideoOut not init\n\n"NONE);
        }
        else
        {
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "video out stop");
        }
    }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "scene pause, true");

    /** remove all stream */
    HI_LIVESVR_RemoveAllStream();

    HI_PDT_MEDIA_CFG_S stMediaCfg;
    memcpy(&stMediaCfg,pstMediaCfg,sizeof(HI_PDT_MEDIA_CFG_S));
    HI_BOOL bCompress = HI_TRUE;
#ifdef CONFIG_RAWCAP_ON
    if((HI_PDT_WORKMODE_PLAYBACK != pstStateMngCtx->enCurrentWorkMode)
        &&(HI_TRUE == pstStateMngCtx->stRawCapCtx.bDebug))
    {
        bCompress = HI_FALSE;
        HI_S32 i,j = 0;
        for(i=0;i<HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
        {
            for(j=0;j<HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;j++)
            {
                stMediaCfg.stViVpssMode.astMode[i][j].enMode = VI_OFFLINE_VPSS_OFFLINE;
            }
        }
    }
#endif
    if (HI_PDT_WORKMODE_NORM_REC == pstStateMngCtx->enCurrentWorkMode) {
        /*LAPSE: update venc attr */
        HI_PDT_PARAM_RecordUpdateMediaCfg(&stMediaCfg);
    }

    /** set Media config */
    if(HI_TRUE == pstStateMngCtx->bForceReBuid)
    {
        s32Ret = HI_PDT_MEDIA_Rebuild(&stMediaCfg);
        pstStateMngCtx->bForceReBuid = HI_FALSE;
    }
    else
    {
        s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg,bCompress);
    }
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"Media not init\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset video");
    }
    /** add all stream */
    s32Ret = PDT_STATEMNG_AddRtspStreams(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "add rtsp streams");

    if (HI_PDT_WORKMODE_PLAYBACK != s_stSTATEMNGContext.enCurrentWorkMode
      &&HI_PDT_WORKMODE_USB_STORAGE != s_stSTATEMNGContext.enCurrentWorkMode
      &&HI_PDT_WORKMODE_UPGRADE != s_stSTATEMNGContext.enCurrentWorkMode)
    {
        HI_PDT_SCENE_MODE_S stSceneMode;
        memset(&stSceneMode, 0, sizeof(HI_PDT_SCENE_MODE_S));

        s32Ret = HI_PDT_PARAM_GetSceneCfg(s_stSTATEMNGContext.enCurrentWorkMode, s_stSTATEMNGContext.astCamCtx, &stSceneMode);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get scene configure");

        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "update scene configure");

        s32Ret = HI_PDT_SCENE_SetSceneMode(&stSceneMode);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set scene mode");

        s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "scene pause, false");
    }

#ifdef HDMI_TEST
    /*starts HDMI*/
    HI_MAPI_HDMI_ATTR_S sthdmiattr;
    HI_HANDLE HdmiHdl = 0;
    HI_HDMI_CALLBACK_FUNC_S stHdmiCallBackFunc;

    stHdmiCallBackFunc.pfnHdmiEventCallback = (HI_HDMI_CallBack)HDMI_EventPro;
    stHdmiCallBackFunc.pPrivateData = HI_NULL;

    memset(&sthdmiattr, 0, sizeof(HI_MAPI_HDMI_ATTR_S));
    sthdmiattr.bEnableHdmi = HI_TRUE;
    sthdmiattr.bEnableVideo = HI_TRUE;
    sthdmiattr.enHdmiSync = VO_OUTPUT_1080P60;
    sthdmiattr.bEnableAudio = HI_FALSE;
    sthdmiattr.enSampleRate = AUDIO_SAMPLE_RATE_48000;

    s32Ret = HI_MAPI_HDMI_Init(HdmiHdl, &stHdmiCallBackFunc);
    //HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_HDMI_SetAttr(HdmiHdl, &sthdmiattr);
    //HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    s32Ret = HI_MAPI_HDMI_Start(HdmiHdl);
    //HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
    MLOGD(RED"HDMI Start\n"NONE);
#endif

    if (bFirst)
    {
        MLOGI("fastboot ignore first vo start\n");
        bFirst = HI_FALSE;
    }
    else
    {
        /** video out start */
        s32Ret = HI_PDT_MEDIA_VideoOutStart(NULL,NULL);
        if(HI_SUCCESS != s32Ret)
        {
            MLOGW("video out start fail,rebuild media!\n");
            s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "scene pause, true");
            s32Ret = HI_PDT_MEDIA_Rebuild(&stMediaCfg);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "media rebuild");
            HI_PDT_SCENE_Pause(HI_FALSE);
            s32Ret = HI_PDT_MEDIA_VideoOutStart(HI_NULL,NULL);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "video out start");
         }
    }

    return s32Ret;
}

#ifdef CONFIG_GPS_ON
static HI_S32 STATEMNG_GPSDataProc(HI_GPSMNG_MSG_PACKET* msgPacket, HI_VOID* privDataInfo)
{
    HI_S32 ret = HI_SUCCESS;
    HI_RECMNG_PRIV_DATA_S privData;
    struct timespec times = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &times);
    privData.u32DataLength = sizeof(HI_GPSMNG_MSG_PACKET);
    privData.pu8DataBuffer = (void*)msgPacket;
    privData.u64TimeStamp = (((HI_U64)times.tv_sec)*1000*1000) + (times.tv_nsec / 1000);

    MLOGD("u64TimeStamp[%lld]\n", privData.u64TimeStamp);
    ret = HI_RECMNG_WritePrivateData(s_stSTATEMNGContext.stRecCtx.astRecTaskCtx[0].stRecMngAttr.astFileAttr[0].stDataSource.aPrivDataHdl[0], \
                                    &privData, privDataInfo);
    if (HI_SUCCESS != ret)
    {
        MLOGE("Private Data Proc failed\n");
        return ret;
    }

    return HI_SUCCESS;
}
#endif

/* privdata */
HI_S32 PDT_STATEMNG_GPSDataStart(HI_HANDLE privDataHdl, HI_VOID* privDataInfo)
{

#ifdef CONFIG_GPS_ON

    HI_S32 ret = HI_SUCCESS;
    if (privDataHdl != s_stSTATEMNGContext.stRecCtx.astRecTaskCtx[0].stRecMngAttr.astFileAttr[0].stDataSource.aPrivDataHdl[0])
    {
        MLOGE("GPS Data START failed,handle illegal\n");
        return HI_FAILURE;
    }

    HI_GPSMNG_CALLBACK pfnGpsCB;
    pfnGpsCB.fnGpsDataCB = STATEMNG_GPSDataProc;
    pfnGpsCB.privateData = privDataInfo;
    ret = HI_GPSMNG_Register(&pfnGpsCB);

    if (ret != HI_SUCCESS)
    {
        MLOGE("GPS MNG Register failed\n");
        return ret;
    }
#endif

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GPSDataStop(HI_HANDLE privDataHdl, HI_VOID* privDataInfo)
{

#ifdef CONFIG_GPS_ON

    HI_S32 ret = HI_SUCCESS;
    HI_GPSMNG_CALLBACK pfnGpsCB;
    pfnGpsCB.fnGpsDataCB = STATEMNG_GPSDataProc;
    pfnGpsCB.privateData = privDataInfo;
    ret = HI_GPSMNG_UnRegister(&pfnGpsCB);

    if (ret != HI_SUCCESS)
    {
        MLOGE("GP SMNG UnRegister failed\n");
        return ret;
    }
#endif

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GetGPSDataCfg(HI_HANDLE PrivDataHdl, HI_RECMNG_PRIV_DATA_INFO_S *privateInfo)
{
    privateInfo->u32FrameRate = 20;
    privateInfo->u32BytesPerSec = 636 * privateInfo->u32FrameRate;
    privateInfo->bStrictSync = HI_FALSE; /* Need to do strict synchronization with video */
    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_RecUpdateAudioCnt(HI_BOOL bAudio)
{
    HI_S32 i = 0, j = 0;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++) {
        if (pstStateMngCtx->astCamCtx[i].bEnable) {
            HI_RECMNG_ATTR_S *pstRecMngAttr = &pstStateMngCtx->stRecCtx.astRecTaskCtx[i].stRecMngAttr;
            for (j = 0; j < pstRecMngAttr->u32FileCnt && j < HI_RECMNG_TASK_MAX_FILE_CNT; j++) {
                if (HI_REC_TYPE_LAPSE == pstStateMngCtx->stRecCtx.astRecTaskCtx[i].stRecMngAttr.enRecType) {
                    pstRecMngAttr->astFileAttr[j].stDataSource.u32AencCnt = 0;
                } else {
                    if (HI_FALSE == bAudio) {
                        pstRecMngAttr->astFileAttr[j].stDataSource.u32AencCnt = 0;
                    }
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_STATEMNG_GenerateRecMngAttr(const HI_PDT_PARAM_RECMNG_ATTR_S *pstEnterWorkModeCfg,
        HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    HI_S32 i = 0, j = 0;
    pstRecMngAttr->stSplitAttr = pstEnterWorkModeCfg->stSplitAttr;
    pstRecMngAttr->u32FileCnt = pstEnterWorkModeCfg->u32FileCnt;
    memcpy(pstRecMngAttr->astFileAttr, pstEnterWorkModeCfg->astFileAttr, sizeof(HI_RECMNG_FILE_ATTR_S)*HI_PDT_RECTASK_FILE_MAX_CNT);
    pstRecMngAttr->enThmType = pstEnterWorkModeCfg->enRecThmType;
    pstRecMngAttr->stThmAttr = pstEnterWorkModeCfg->stThmAttr;
    pstRecMngAttr->u32BufferTimeMSec = pstEnterWorkModeCfg->u32BufferTime_ms;
    pstRecMngAttr->u32PreRecTimeSec = pstEnterWorkModeCfg->u32PreRecTimeSec;

#ifdef CONFIG_GPS_ON

    for (i = 0; i < HI_PDT_RECTASK_FILE_MAX_CNT; i++)
    {
       pstRecMngAttr->astFileAttr[i].stDataSource.u32PrivDataCnt = 1;
       pstRecMngAttr->astFileAttr[i].stDataSource.aPrivDataHdl[0] = 0;
    }

#else

    for (i = 0; i < HI_PDT_RECTASK_FILE_MAX_CNT; i++)
    {
        pstRecMngAttr->astFileAttr[i].stDataSource.u32PrivDataCnt = 0;
        pstRecMngAttr->astFileAttr[i].stDataSource.aPrivDataHdl[0] = -1;
    }

#endif

    MLOGD("u32FileCnt:%u \n", pstRecMngAttr->u32FileCnt);

    for (i = 0; i < pstRecMngAttr->u32FileCnt; i++)
    {
        MLOGD("u32VencCnt:%u ,u32AencCnt:%u \n", pstRecMngAttr->astFileAttr[i].stDataSource.u32VencCnt, pstRecMngAttr->astFileAttr[i].stDataSource.u32AencCnt);

        for (j = 0; j < pstRecMngAttr->astFileAttr[i].stDataSource.u32VencCnt; j++)
        {
            MLOGD("aVencHdl[%d]:%d \n", j, pstRecMngAttr->astFileAttr[i].stDataSource.aVencHdl[j]);
        }

        for (j = 0; j < pstRecMngAttr->astFileAttr[i].stDataSource.u32VencCnt; j++)
        {
            MLOGD("aAencHdl[%d]:%d \n", j, pstRecMngAttr->astFileAttr[i].stDataSource.aAencHdl[j]);
        }

        MLOGD("ThmHdl:%d \n", pstRecMngAttr->astFileAttr[i].stDataSource.ThmHdl);

        for (j = 0; j < HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT; j++)
        {
            pstRecMngAttr->astFileAttr[i].astBufCfg[j].enBufType = HI_RECMNG_BUF_TYPE_MMZ;
            snprintf(pstRecMngAttr->astFileAttr[i].astBufCfg[j].unBufCfg.stMmzBufCfg.szBufName,HI_APPCOMM_COMM_STR_LEN,"Ringbuf");
        }

        pstRecMngAttr->astFileAttr[i].stMuxerCfg.bConstantFps = HI_TRUE;
 #ifdef CONFIG_TEMPRATURE_CYCLE_ON
        pstRecMngAttr->astFileAttr[i].stMuxerCfg.bConstantFps = HI_FALSE;
 #endif
    }

    MLOGD("u32ThmCnt:%u \n", pstRecMngAttr->stThmAttr.u32ThmCnt);

    for (i = 0; i < pstRecMngAttr->stThmAttr.u32ThmCnt; i++)
    {
        MLOGD("aThmHdl[%d]:%u \n", i, pstRecMngAttr->stThmAttr.aThmHdl[i]);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GenerateRecCtx(HI_PDT_WORKMODE_E enEnterWorkMode,
                                   const HI_PDT_WORKMODE_CFG_S* pstEnterWorkModeCfg,
                                   PDT_STATEMNG_REC_CONTEXT_S* pstRecCtx,HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_PDT_WORKMODE_CFG_S* pstWorkModeCfg = (HI_PDT_WORKMODE_CFG_S*)pstEnterWorkModeCfg;

    HI_PDT_PARAM_RECMNG_ATTR_S* pstParamRecAttr = HI_NULL;
    HI_RECMNG_ATTR_S* pstRecMngAttr = HI_NULL;
    HI_S32 i = 0, j = 0, s32Ret = HI_FAILURE;

    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        pstRecMngAttr =  &pstRecCtx->astRecTaskCtx[i].stRecMngAttr;

        /** the specific processing */
        switch (enEnterWorkMode)
        {
            case HI_PDT_WORKMODE_NORM_REC:
            {
                if (HI_REC_TYPE_NORMAL == pstWorkModeCfg->unModeCfg.stNormRecAttr.astRecMngAttr[i].enRecType)
                {
                    pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
                }

                if (HI_REC_TYPE_LAPSE == pstWorkModeCfg->unModeCfg.stNormRecAttr.astRecMngAttr[i].enRecType)
                {
                    pstRecMngAttr->enRecType = HI_REC_TYPE_LAPSE;
                }
            }
                break;

            case HI_PDT_WORKMODE_PARKING_REC:
            {
                pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
            }
                break;

            default:
            {
                MLOGE(RED" workmode[%d] unsupport \n\n"NONE, enEnterWorkMode);
                return HI_PDT_STATEMNG_EINVAL;
            }
        }

        /*update rec context attr */
        pstParamRecAttr = &pstWorkModeCfg->unModeCfg.stNormRecAttr.astRecMngAttr[i];
        s32Ret = PDT_STATEMNG_GenerateRecMngAttr(pstParamRecAttr, pstRecMngAttr);
        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "generate recmng attr");

        if (HI_REC_TYPE_LAPSE == pstRecMngAttr->enRecType)
        {
            pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs =
                pstWorkModeCfg->unModeCfg.stNormRecAttr.astRecMngAttr[i].u32Interval_ms;
            for (j = 0; j < pstRecMngAttr->u32FileCnt; j++)
            {
                if (pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].u32VideoMode == 0)/** PAL */
                {
                    extern HI_S32 PDT_PARAM_UpdateFrmCfg(HI_S32 s32Frm, HI_S32 *pNewFrm);
                    PDT_PARAM_UpdateFrmCfg(PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS, &pstRecMngAttr->astFileAttr[j].s32PlayFps);
                }
                else
                {
                    pstRecMngAttr->astFileAttr[j].s32PlayFps = PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS;
                }
            }
        }
        pstRecCtx->astRecTaskCtx[i].s32CamID = i;
        pstRecCtx->astRecTaskCtx[i].bFileEndRecv = HI_FALSE;
        pstRecCtx->astRecTaskCtx[i].bAudioStarted = HI_FALSE;
        pstRecCtx->astRecTaskCtx[i].bEnable = pstParamRecAttr->bEnable;
        pstRecMngAttr->stGetFilename.pfnGetFilenames = PDT_STATEMNG_GetRecTaskFileName;
        pstRecMngAttr->stGetFilename.pvPrivData = (HI_VOID*)&pstRecCtx->astRecTaskCtx[i];
    }

    /*update record task audio encode count*/
    PDT_STATEMNG_RecUpdateAudioCnt(pstMediaCfg->stAudioCfg.astAencCfg[0].bEnable);

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GeneratePhotoCtx(HI_U32 u32CamID,
                                     HI_PDT_WORKMODE_E enEnterWorkMode,
                                     const HI_PDT_WORKMODE_CFG_S* pstEnterWorkModeCfg,
                                     PDT_STATEMNG_PHOTOTASK_CONTEXT_S* pstPhotoTaskCtx)
{
    pstPhotoTaskCtx->stPhotoAttr.enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_SINGLE;
    pstPhotoTaskCtx->stPhotoAttr.enOutPutFmt = HI_PHOTOMNG_OUTPUT_FORMAT_JPEG;
    pstPhotoTaskCtx->stPhotoAttr.unPhotoTypeAttr.stSingleAttr.s32Time_sec = 0;
    pstPhotoTaskCtx->stPhotoAttr.unPhotoTypeAttr.stSingleAttr.bNeedTrigger = HI_FALSE;

    switch (enEnterWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        {
            pstPhotoTaskCtx->bEnable  = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].bEnable;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VcapPipeHdl = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].VcapPipeHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VcapPipeChnHdl = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].VcapPipeChnHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VprocHdl = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].VpssHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VportHdl = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].VportHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VencHdl = pstEnterWorkModeCfg->unModeCfg.stNormRecAttr.stPhotoSrc[u32CamID].VencHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VencThmHdl = -1;  /*** not use */
        }
        break;

        case HI_PDT_WORKMODE_PHOTO:
        {
            pstPhotoTaskCtx->bEnable = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].bEnable;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VcapPipeHdl = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].VcapPipeHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VcapPipeChnHdl = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].VcapPipeChnHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VprocHdl = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].VpssHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VportHdl = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].VportHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VencHdl = pstEnterWorkModeCfg->unModeCfg.stPhotoCfg.stPhotoSrc[u32CamID].VencHdl;
            pstPhotoTaskCtx->stPhotoAttr.stPhotoSRC.VencThmHdl = -1;  /*** not use */

        }
        break;

        default:
        {
            MLOGE(RED" workmode[%d] unsupport \n\n"NONE, enEnterWorkMode);
            return HI_PDT_STATEMNG_EINVAL;
        }
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_ChangeRecType(HI_REC_TYPE_E enRecType)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32Idx = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /*get media config*/
    HI_PDT_MEDIA_CFG_S stEnterMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    /** get workmode config from Param module */
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_NORM_REC, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stEnterWorkModeCfg parameter");

     /** change rec type */
    for(s32Idx = 0;s32Idx < HI_PDT_RECTASK_MAX_CNT;s32Idx++)
    {
        stEnterWorkModeCfg.unModeCfg.stNormRecAttr.astRecMngAttr[s32Idx].enRecType = enRecType;
    }

    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(HI_PDT_WORKMODE_NORM_REC, &stEnterWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx,&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");

    /** remove all stream */
    HI_LIVESVR_RemoveAllStream();

    HI_BOOL bCompress = HI_TRUE;
#ifdef CONFIG_RAWCAP_ON
    if((HI_PDT_WORKMODE_PLAYBACK != pstStateMngCtx->enCurrentWorkMode)
        &&(HI_TRUE == pstStateMngCtx->stRawCapCtx.bDebug))
    {
        bCompress = HI_FALSE;
        HI_S32 i,j = 0;
        for(i=0;i<HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
        {
            for(j=0;j<HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT;j++)
            {
                stEnterMediaCfg.stViVpssMode.astMode[i][j].enMode = VI_OFFLINE_VPSS_OFFLINE;
            }
        }
    }
#endif

    if(HI_REC_TYPE_LAPSE == enRecType)
    {
        /*LAPSE: update venc attr */
        HI_PDT_PARAM_RecordUpdateMediaCfg(&stEnterMediaCfg);
    }

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StopMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "PDT_STATEMNG_StartMd");
#endif

    /** set Media config */
    s32Ret = HI_PDT_MEDIA_Reset(&stEnterMediaCfg, bCompress);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"Media not init\n\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "reset video");
    }

#ifdef CONFIG_MOTIONDETECT_ON
    s32Ret = PDT_STATEMNG_StartMd();
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "PDT_STATEMNG_StopMd");
#endif
    /** add all stream */
    s32Ret = PDT_STATEMNG_AddRtspStreams(&stEnterMediaCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "add rtsp streams");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_CheckCamSettingItemValue(HI_MESSAGE_S* pstMsg, HI_S32 s32PreviewCamID, HI_S32 s32SettingItemValue)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_PARAM_TYPE_E enType = pstMsg->arg1;

    HI_PDT_ITEM_VALUESET_S stValueSet;
    memset(&stValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));
    s32Ret = HI_PDT_PARAM_GetCamItemValues(PDT_STATEMNG_GetCtx()->enCurrentWorkMode, s32PreviewCamID, enType, &stValueSet);

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED" get cam[%d] paramtype[%d] valueset failed \n\n"NONE, s32PreviewCamID, enType);
        return HI_PDT_STATEMNG_EINVAL;
    }

    HI_S32 i = 0;

    for (i = 0; i < stValueSet.s32Cnt; i++)
    {
        if (stValueSet.astValues[i].s32Value == s32SettingItemValue)
        {
            return HI_SUCCESS;
        }
    }

    return HI_PDT_STATEMNG_EINVAL;
}

HI_S32 PDT_STATEMNG_SetAudio(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    /** get setting information */
    HI_BOOL bAudio;
    memcpy(&bAudio, pstMsg->aszPayload, sizeof(HI_BOOL));

    /*get media config*/
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");
    if (HI_PDT_WORKMODE_NORM_REC == pstStateMngCtx->enCurrentWorkMode) {
        /* record update media config*/
        HI_PDT_PARAM_RecordUpdateMediaCfg(&stMediaCfg);
    }

    /** filter out the same parameter */
    if (stMediaCfg.stAudioCfg.astAencCfg[0].bEnable == bAudio)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the bEnableAudio to take effect */
    for (i = 0; i < HI_PDT_MEDIA_ACAP_MAX_CNT; i++)
    {
        stMediaCfg.stAudioCfg.astAcapCfg[i].bEnable = bAudio;
    }

    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
    {
        stMediaCfg.stAudioCfg.astAencCfg[i].bEnable = bAudio;
    }

    /** remove all stream */
    HI_LIVESVR_RemoveAllStream();

    HI_BOOL bCompress = HI_TRUE;
#ifdef CONFIG_RAWCAP_ON
    if((HI_PDT_WORKMODE_PLAYBACK != pstStateMngCtx->enCurrentWorkMode)
        &&(HI_TRUE == pstStateMngCtx->stRawCapCtx.bDebug))
    {
        bCompress = HI_FALSE;
    }   /** set Media config */
#endif
    s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg,bCompress);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"Media Reset");

    /** add streams */
    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg =  NULL;
    HI_HANDLE AencHdl = stMediaCfg.stAudioCfg.astAencCfg[0].bEnable ? stMediaCfg.stAudioCfg.astAencCfg[0].AencHdl : HI_INVALID_HANDLE;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
    {
        pstVencCfg = &(stMediaCfg.stVideoCfg.astVencCfg[i]);

#ifdef CONFIG_PQT_SUPPORT_ON
        if (pstVencCfg->bEnable &&
            (HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCfg->enVencType ||
             HI_PDT_MEDIA_VENC_TYPE_REC == pstVencCfg->enVencType))
#else
        if (pstVencCfg->bEnable &&
            (HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCfg->enVencType))
#endif
        {
            HI_CHAR szStreamName[HI_APPCOMM_COMM_STR_LEN];
            snprintf(szStreamName, HI_APPCOMM_COMM_STR_LEN, "%d", pstVencCfg->VencHdl);
            s32Ret = HI_LIVESVR_AddStream(pstVencCfg->VencHdl, AencHdl, szStreamName);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE(RED"add stream venchdl[%d] failed \n\n"NONE, pstVencCfg->VencHdl);
                return HI_PDT_STATEMNG_EINTER;
            }
        }
    }
    /** update record attr */
    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(pstStateMngCtx->enCurrentWorkMode, &stWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stWorkModeCfg parameter");
    s32Ret = PDT_STATEMNG_GenerateRecCtx(pstStateMngCtx->enCurrentWorkMode, &stWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx, &stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");
    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_AUDIO,
                                       (HI_VOID*)&bAudio);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set bEnableAudio parameter");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetFlip(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_BOOL bFlip;
    memcpy(&bFlip, pstMsg->aszPayload, sizeof(HI_BOOL));

    /** get param to Param module */
    HI_BOOL bOldEnableFlip = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_FLIP,
                                      (HI_VOID*)&bOldEnableFlip);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get flip");

    /** filter out the same parameter */
    if (bOldEnableFlip == bFlip)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the Flip to take effect */
    HI_S32 i = 0, j = 0;
    HI_PDT_MEDIA_BIND_SRC_CFG_S stFlipSrc;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, pstStateMngCtx->astCamCtx, &stMediaCfg);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr = &stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[pstStateMngCtx->s32PreviewCamID];
    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; i++)
    {
        if (HI_TRUE != pstVcapDevAttr->astVcapPipeAttr[i].bEnable)
        {
            continue;
        }
        if (HI_TRUE == pstVcapDevAttr->astVcapPipeAttr[i].bIspBypass
            || VI_ONLINE_VPSS_ONLINE == stMediaCfg.stViVpssMode.astMode[pstStateMngCtx->s32PreviewCamID][i].enMode)/** do flip on vpss port */
        {
            break;
        }
        HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = &(pstVcapDevAttr->astVcapPipeAttr[i]);

        stFlipSrc.enBindedMod = HI_PDT_MEDIA_VIDEOMOD_VCAP;
        stFlipSrc.ModHdl = pstVcapPipeAttr->VcapPipeHdl;

        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; j++)
        {
            if (HI_TRUE != pstVcapPipeAttr->astPipeChnAttr[j].bEnable)
            {
                continue;
            }

            stFlipSrc.ChnHdl = pstVcapPipeAttr->astPipeChnAttr[j].PipeChnHdl;
            s32Ret = HI_PDT_MEDIA_SetFlip(&stFlipSrc, bFlip);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set media Flip");
        }
    }

    if(i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT)
    {
        HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr =
            &stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[pstStateMngCtx->s32PreviewCamID];
        stFlipSrc.enBindedMod = HI_PDT_MEDIA_VIDEOMOD_VPSS;
        stFlipSrc.ModHdl = pstVpssAttr->VpssHdl;
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
        {
            if (HI_TRUE != pstVpssAttr->astVportAttr[j].bEnable)
            {
                continue;
            }
            stFlipSrc.ChnHdl = pstVpssAttr->astVportAttr[j].VportHdl;
            s32Ret = HI_PDT_MEDIA_SetFlip(&stFlipSrc, bFlip);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set media Flip");
        }
    }
    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_FLIP,
                                      (HI_VOID*)&bFlip);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save flip");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetMirror(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_BOOL bMirror;
    memcpy(&bMirror, pstMsg->aszPayload, sizeof(HI_BOOL));

    /** get param to Param module */
    HI_BOOL bOldEnableMirror = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_MIRROR,
                                      (HI_VOID*)&bOldEnableMirror);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get mirror");

    /** filter out the same parameter */
    if (bOldEnableMirror == bMirror)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the Flip to take effect */
    HI_S32 i = 0, j = 0;
    HI_PDT_MEDIA_BIND_SRC_CFG_S stMirrorSrc;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, pstStateMngCtx->astCamCtx, &stMediaCfg);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr = &stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[pstStateMngCtx->s32PreviewCamID];
    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; i++)
    {
        if (HI_TRUE != pstVcapDevAttr->astVcapPipeAttr[i].bEnable)
        {
            continue;
        }
        if (HI_TRUE == pstVcapDevAttr->astVcapPipeAttr[i].bIspBypass
            || VI_ONLINE_VPSS_ONLINE == stMediaCfg.stViVpssMode.astMode[pstStateMngCtx->s32PreviewCamID][i].enMode)/** do flip on vpss port */
        {
            break;
        }
        HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = &(pstVcapDevAttr->astVcapPipeAttr[i]);

        stMirrorSrc.enBindedMod = HI_PDT_MEDIA_VIDEOMOD_VCAP;
        stMirrorSrc.ModHdl = pstVcapPipeAttr->VcapPipeHdl;

        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; j++)
        {
            if (HI_TRUE != pstVcapPipeAttr->astPipeChnAttr[j].bEnable)
            {
                continue;
            }

            stMirrorSrc.ChnHdl = pstVcapPipeAttr->astPipeChnAttr[j].PipeChnHdl;
            s32Ret = HI_PDT_MEDIA_SetMirror(&stMirrorSrc, bMirror);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set media Mirror");
        }
    }

    if(i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT)
    {
        HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr =
            &stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[pstStateMngCtx->s32PreviewCamID];
        stMirrorSrc.enBindedMod = HI_PDT_MEDIA_VIDEOMOD_VPSS;
        stMirrorSrc.ModHdl = pstVpssAttr->VpssHdl;
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
        {
            if (HI_TRUE != pstVpssAttr->astVportAttr[j].bEnable)
            {
                continue;
            }
            stMirrorSrc.ChnHdl = pstVpssAttr->astVportAttr[j].VportHdl;
            s32Ret = HI_PDT_MEDIA_SetMirror(&stMirrorSrc, bMirror);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set media Mirror");
        }
    }
    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_MIRROR,
                                      (HI_VOID*)&bMirror);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save Mirror");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_UpdateLiveSwitchOsdState(HI_S32 s32LiveCamID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    HI_OSD_ATTR_S stOsdAttr;
    HI_U32 u32OsdIdx = 0;
    HI_S32 i, j= 0;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,s32Ret,"get media config");

    HI_BOOL bOSD = HI_FALSE;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
     s32Ret = HI_PDT_PARAM_GetCamParam(pstStateMngCtx->enCurrentWorkMode,
                                      s32LiveCamID,
                                      HI_PDT_PARAM_TYPE_OSD,
                                      (HI_VOID*)&bOSD);

    for(u32OsdIdx = 0;u32OsdIdx < MIN(stMediaCfg.stVideoCfg.stOsdCfg.s32OsdCnt, HI_OSD_MAX_CNT);u32OsdIdx++)
    {
        /**do not determine return value*/
        s32Ret = HI_OSD_GetAttr(u32OsdIdx, &stOsdAttr);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_OSD_GetAttr failed. %d.\n", s32Ret);
            return HI_FAILURE;
        }
        for(i = 0;i < MIN(stOsdAttr.u32DispNum, HI_OSD_MAX_DISP_CNT);i++)
        {
            if (HI_OSD_BINDMOD_VENC != stOsdAttr.astDispAttr[i].enBindedMod)
            {
                continue;
            }
            for (j = 0; j < HI_PDT_MEDIA_VENC_MAX_CNT; j++)
            {
                if (stMediaCfg.stVideoCfg.astVencCfg[j].VencHdl!= stOsdAttr.astDispAttr[i].ChnHdl)
                {
                    continue;
                }

                if (HI_PDT_MEDIA_VENC_TYPE_LIVE == stMediaCfg.stVideoCfg.astVencCfg[j].enVencType &&
                    stMediaCfg.stVideoCfg.astVencCfg[j].bEnable)
                {
                    stOsdAttr.astDispAttr[i].s32Batch %= 10;
                    stOsdAttr.astDispAttr[i].s32Batch += 10*s32LiveCamID;
                    stOsdAttr.astDispAttr[i].bShow = bOSD;
                    s32Ret = HI_OSD_SetAttr(u32OsdIdx, &stOsdAttr);
                    if (HI_SUCCESS != s32Ret)
                    {
                        MLOGE("HI_OSD_SetAttr failed. %d.\n", s32Ret);
                        return HI_FAILURE;
                    }
                    break;
                }
            }
        }
    }
    return HI_SUCCESS;
}


HI_S32 PDT_STATEMNG_SetOSD(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_BOOL bOSD = HI_FALSE;
    memcpy(&bOSD, pstMsg->aszPayload, sizeof(HI_BOOL));

    /** get param to Param module */
    HI_BOOL bOldOSD = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_OSD,
                                      (HI_VOID*)&bOldOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get osd");

    /** filter out the same parameter */
    if (bOldOSD == bOSD)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the OSD to take effect */
    s32Ret = HI_PDT_MEDIA_SetTimeOSD(pstStateMngCtx->s32PreviewCamID, bOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set media TimeOSD");

    s32Ret = HI_PDT_MEDIA_SetLogoOSD(pstStateMngCtx->s32PreviewCamID, bOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "set media LogoOSD");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_OSD,
                                      (HI_VOID*)&bOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save osd");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetWDR(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_S32 s32WDR = 0;
    memcpy(&s32WDR, pstMsg->aszPayload, sizeof(HI_S32));

    /** get param to Param module */
    WDR_MODE_E enOldWdrMode = WDR_MODE_NONE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_WDR,
                                      (HI_VOID*)&enOldWdrMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get wdr");

    /** filter out the same parameter */
    if ((WDR_MODE_NONE == enOldWdrMode&&0 == s32WDR)||(WDR_MODE_NONE != enOldWdrMode&& 0 != s32WDR))
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the WDR to take effect */
    WDR_MODE_E enWdrMode = s32WDR?WDR_MODE_2To1_LINE:WDR_MODE_NONE;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr = &stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[pstStateMngCtx->s32PreviewCamID];
    pstVcapDevAttr->enWdrMode = enWdrMode;

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_WDR,
                                      (HI_VOID*)&enWdrMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save wdr");

    /** Reset Media for setting parameter */
    pstStateMngCtx->bForceReBuid = HI_TRUE;
    s32Ret = PDT_STATEMNG_ResetMedia(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetVideoMode(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_U32 u32VideoMode;
    memcpy(&u32VideoMode, pstMsg->aszPayload, sizeof(HI_U32));

    /** get param to Param module */
    HI_U32 u32OldVideoMode = 0;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_VIDEOMODE,
                                      (HI_VOID*)&u32OldVideoMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get videomode");

    /** filter out the same parameter */
    if (u32OldVideoMode == u32VideoMode)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_VIDEOMODE,
                                      (HI_VOID*)&u32VideoMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save videomode");

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = PDT_STATEMNG_GetMediaCfg(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get media config");
    /** update record attr */
    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(pstStateMngCtx->enCurrentWorkMode, &stWorkModeCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "get stWorkModeCfg parameter");
    /** generate Rec Context */
    s32Ret = PDT_STATEMNG_GenerateRecCtx(HI_PDT_WORKMODE_NORM_REC, &stWorkModeCfg,
                                         &pstStateMngCtx->stRecCtx,&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "generate record context");
    /** Reset Media for setting parameter */
    s32Ret = PDT_STATEMNG_ResetMedia(&stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "reset media");

    return s32Ret;
}


HI_S32 PDT_STATEMNG_SetLDC(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_BOOL bLDC;
    memcpy(&bLDC, pstMsg->aszPayload, sizeof(HI_BOOL));

    /** get param to Param module */
    HI_BOOL bOldLDC = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_LDC,
                                      (HI_VOID*)&bOldLDC);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get LDC");

    /** filter out the same parameter */
    if (bOldLDC == bLDC)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** set the LDC to take effect */
    HI_S32 i = 0, j = 0;
    HI_HANDLE VcapPipeHdl = 0;
    HI_HANDLE VcapPipeChnHdl = 0;

    /** get media config from Param module */
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    s32Ret = HI_PDT_PARAM_GetMediaCfg(enSettingWorkMode, pstStateMngCtx->astCamCtx, &stMediaCfg);
    HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr =
        &stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[pstStateMngCtx->s32PreviewCamID];

    for (i = 0; i < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; i++)
    {
        if (HI_TRUE != pstVcapDevAttr->astVcapPipeAttr[i].bEnable)
        {
            continue;
        }

        HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = &pstVcapDevAttr->astVcapPipeAttr[i];
        VcapPipeHdl = pstVcapPipeAttr->VcapPipeHdl;

        for (j = 0; j < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; j++)
        {
            if (HI_TRUE != pstVcapPipeAttr->astPipeChnAttr[j].bEnable)
            {
                continue;
            }

            VcapPipeChnHdl = pstVcapPipeAttr->astPipeChnAttr[j].PipeChnHdl;
            s32Ret = HI_PDT_MEDIA_SetLDC(VcapPipeHdl, VcapPipeChnHdl, bLDC);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set media LDC");
        }
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_LDC,
                                      (HI_VOID*)&bLDC);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save LDC");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetCrop(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_S32 s32Direction = 0;
    memcpy(&s32Direction, pstMsg->aszPayload, sizeof(HI_S32));

    /** filter out the same parameter */
    if (0 == s32Direction)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    /** get param to Param module */
    HI_PDT_MEDIA_DISP_WND_CROP_CFG_S stCropCfg;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_CROP,
                                      (HI_VOID*)&stCropCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get CropCfg");


    if (0 < s32Direction)
    {
        if (0 < stCropCfg.u32CurH)
        {
            stCropCfg.u32CurH = stCropCfg.u32CurH - 1;
        }
        else
        {
            stCropCfg.u32CurH = 0;
        }
    }
    else
    {
        stCropCfg.u32CurH = MIN(stCropCfg.u32CurH + 1, stCropCfg.u32MaxH - 1);
    }

    /** set crop to take effect */
    s32Ret = HI_PDT_MEDIA_DispWindowCrop(&stCropCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "set crop");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      pstStateMngCtx->s32PreviewCamID,
                                      HI_PDT_PARAM_TYPE_CROP,
                                      (HI_VOID*)&stCropCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save crop");

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetRecEnable(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_BOOL bRec = HI_FALSE;
    memcpy(&bRec, pstMsg->aszPayload, sizeof(HI_BOOL));

    /** get param to Param module */
    HI_BOOL bRecEnable = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetCamParam(enSettingWorkMode,
                                      PDT_STATEMNG_BACK_CAMID,
                                      HI_PDT_PARAM_TYPE_REC,
                                      (HI_VOID*)&bRecEnable);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get ren enable");

    /** filter out the same parameter */
    if (bRecEnable == bRec)
    {
        MLOGE(YELLOW"the setting item value as same as current setting value\n\n"NONE);
        return HI_SUCCESS;
    }

    HI_S32 i = 0;

    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        if (PDT_STATEMNG_BACK_CAMID == pstStateMngCtx->stRecCtx.astRecTaskCtx[i].s32CamID)
        {
            pstStateMngCtx->stRecCtx.astRecTaskCtx[i].bEnable = bRec;
            break;
        }
    }


    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetCamParam(enSettingWorkMode,
                                      PDT_STATEMNG_BACK_CAMID,
                                      HI_PDT_PARAM_TYPE_REC,
                                      (HI_VOID*)&bRec);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "save rec enable");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetRecSplitTime(HI_MESSAGE_S* pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_U32 u32SplitTimeLenSec;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    memcpy(&u32SplitTimeLenSec, pstMsg->aszPayload, sizeof(HI_U32));
    MLOGI("Rec split time [%d]\n",u32SplitTimeLenSec);

    for (i = 0; i < HI_PDT_RECTASK_MAX_CNT; i++)
    {
        /* change  statemng context Attr  */
        pstStateMngCtx->stRecCtx.astRecTaskCtx[i].stRecMngAttr.stSplitAttr.u32SplitTimeLenSec = u32SplitTimeLenSec;

        /* change  param  Attr */
        s32Ret = HI_PDT_PARAM_SetCamParam(pstStateMngCtx->enCurrentWorkMode,pstStateMngCtx->stRecCtx.astRecTaskCtx[i].s32CamID,
            pstMsg->arg1,&u32SplitTimeLenSec);
        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "set Rec split time failed");
    }

    return s32Ret ;
}

HI_S32 PDT_STATEMNG_SetUsbMode(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_USB_MODE_E enUsbMode;

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &enUsbMode);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"GetUsbMode");

    s32Ret = HI_USB_SetMode(enUsbMode);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"SetUsbMode");

    s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_USB_MODE, &enUsbMode);
    HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret,"SetUsbMode");

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_SetPoweronWorkMode(HI_PDT_WORKMODE_E enSettingWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** save poweron workmode */
    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get poweron workmode");
    MLOGD("enPoweronWorkmode(%d)\n", enPoweronWorkmode);

    if(enSettingWorkmode != enPoweronWorkmode)
    {
        s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE,
            (HI_VOID *)&(enSettingWorkmode));
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "save poweron workmode param");

        HI_CHAR szStateName[HI_STATE_NAME_LEN] = {0};
        s32Ret = PDT_STATEMNG_WorkMode2State(enSettingWorkmode, szStateName, HI_STATE_NAME_LEN);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get workmode name");
        MLOGD(YELLOW"save poweron workmode(%s:%d) param succeed\n"NONE,
            szStateName, enSettingWorkmode);
    }

    return s32Ret;
}


/** init global context structure */
HI_S32 PDT_STATEMNG_ContextInit(const HI_PDT_STATEMNG_CONFIG_S* pstStatemngCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** init context structure */
    s_stSTATEMNGContext.enCurrentWorkMode = HI_PDT_WORKMODE_BUTT;
    s_stSTATEMNGContext.bInProgress = HI_FALSE;
    s_stSTATEMNGContext.bRunning = HI_FALSE;
    s_stSTATEMNGContext.bSDAvailable = HI_FALSE;
    s_stSTATEMNGContext.bForceReBuid = HI_FALSE;
    memcpy(&s_stSTATEMNGContext.stStatemngCfg, pstStatemngCfg, sizeof(HI_PDT_STATEMNG_CONFIG_S));
    s_stSTATEMNGContext.enPowerOnState = HI_STORAGE_STATE_IDEL; /*it meaning unkown state*/

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&(s_stSTATEMNGContext.stStorageMngCfg));

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get storage config failed\n\n"NONE);
    }

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_PREVIEW_CAMID, &PDT_STATEMNG_GetCtx()->s32PreviewCamID);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PROCESS_MSG_RESULTE_OK, "set preview camerid");

    s_stSTATEMNGContext.bScreenDormantStatus = HI_FALSE;

    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&(s_stSTATEMNGContext.stFileMngCfg));

    if (HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"get filemng config failed\n\n"NONE);
    }

#ifdef CONFIG_RAWCAP_ON
    s_stSTATEMNGContext.stRawCapCtx.bDebug = HI_FALSE;
#endif

#ifdef CONFIG_MOTIONDETECT_ON
    memset(&s_stSTATEMNGContext.stVideoDetectCtx, 0x0, sizeof(PDT_STATEMNG_VIDEODETECT_CONTEXT_S));
#endif

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
        memset(&s_stSTATEMNGContext.stTempCycleCtx, 0x0, sizeof(PDT_STATEMNG_TEMPCYCLE_CONTEXT_S));
#endif


    return s32Ret;
}

PDT_STATEMNG_CONTEXT* PDT_STATEMNG_GetCtx(HI_VOID)
{
    return &s_stSTATEMNGContext;
}

HI_S32 PDT_STATEMNG_GetMediaModeCfg(HI_PDT_WORKMODE_E enWorkMode, HI_PDT_PARAM_CAM_CONTEXT_S* pstCamCtx)
{
    HI_S32 s32Ret = HI_FAILURE, i = 0;
    HI_PDT_PARAM_CAM_MEDIAMODE_CFG_S stCamMediaMode;

    s32Ret = HI_PDT_PARAM_GetMediaModeCfg(enWorkMode, &stCamMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "get mediamode configure");

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstCamCtx[i].stMediaModeCfg.s32CamID = stCamMediaMode.astMediaModeCfg[i].s32CamID;
        pstCamCtx[i].stMediaModeCfg.enMediaMode = stCamMediaMode.astMediaModeCfg[i].enMediaMode;
        s32Ret = HI_PDT_PARAM_GetCamParam(enWorkMode, i, HI_PDT_PARAM_TYPE_CAM_STATUS,
            &pstCamCtx[i].bEnable);
        MLOGI("CAM[%d],bEnable[%d]\n",i,pstCamCtx[i].bEnable);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_UpdateDispCfg(HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_S32 s32Ret = HI_FAILURE, i = 0, j = 0;
    HI_PDT_MEDIA_BIND_SRC_CFG_S stDispSrc;
    HI_PDT_MEDIA_VIDEOOUT_CFG_S* pstVideoOutCfg = &pstMediaCfg->stVideoOutCfg;

    /*get main preview data source */
    s32Ret = HI_PDT_PARAM_GetDispSrcByCamID(pstStateMngCtx->s32PreviewCamID, &stDispSrc);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "HI_PDT_PARAM_GetDispSrcByCamID");

    for(i=0;i<HI_PDT_MEDIA_DISP_MAX_CNT;i++)
    {
        /* specify main preview data source */
        pstVideoOutCfg->astDispCfg[i].astWndCfg[0].enBindedMod = stDispSrc.enBindedMod;
        pstVideoOutCfg->astDispCfg[i].astWndCfg[0].ModHdl = stDispSrc.ModHdl;
        pstVideoOutCfg->astDispCfg[i].astWndCfg[0].ChnHdl = stDispSrc.ChnHdl;

        for(j=1;j<HI_PDT_MEDIA_DISP_WND_MAX_CNT;j++)
        {
            if ((pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ModHdl == stDispSrc.ModHdl)
                && (pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ChnHdl == stDispSrc.ChnHdl)
                && (pstVideoOutCfg->astDispCfg[i].astWndCfg[j].enBindedMod == stDispSrc.enBindedMod))
            {
                s32Ret = HI_PDT_PARAM_GetDispSrcByCamID(0, &stDispSrc);
                PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "HI_PDT_PARAM_GetDispSrcByCamID");
                /* config slave preview data source */
                pstVideoOutCfg->astDispCfg[i].astWndCfg[j].enBindedMod = stDispSrc.enBindedMod;
                pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ModHdl = stDispSrc.ModHdl;
                pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ChnHdl = stDispSrc.ChnHdl;
            }
            MLOGD("DispHdl[%d],WndHdl[%d],BindModHd[%d],BindChnHd[%d]\n",i,j, \
                pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ModHdl,pstVideoOutCfg->astDispCfg[i].astWndCfg[j].ChnHdl);
            break;
        }
        /*reference cam 0 stResolution */
        s32Ret = HI_PDT_Media_UpdateDispCfg(&pstVideoOutCfg->astDispCfg[i],&pstMediaCfg->stVideoCfg.stVprocCfg,
                                            &pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[0].stResolution);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINVAL, "HI_PDT_Media_UpdateDispCfg");

    }
    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GetMediaCfg(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    /*get media mode config*/
    s32Ret = PDT_STATEMNG_GetMediaModeCfg(pstStateMngCtx->enCurrentWorkMode, pstStateMngCtx->astCamCtx);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get media mode config");

    /** get media config from Param module */
    s32Ret = HI_PDT_PARAM_GetMediaCfg(pstStateMngCtx->enCurrentWorkMode, pstStateMngCtx->astCamCtx, pstMediaCfg);

    /**update disp configure*/
    s32Ret = PDT_STATEMNG_UpdateDispCfg(pstMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "update video out configure");

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_RecManualSplit(HI_BOOL bStopAfterManualSplitEnd)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_REC_MANUAL_SPLIT_ATTR_S stManualSplitAttr;

    stManualSplitAttr.enManualType = HI_REC_POST_MANUAL_SPLIT;
    stManualSplitAttr.stPostSplitAttr.u32AfterSec = HI_PDT_STATEMNG_REC_EMR_TIME_SEC;

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable))
        {
            s32Ret = HI_RECMNG_ManualSplit(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl,
                                           &stManualSplitAttr, bStopAfterManualSplitEnd);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" manualsplit task[%d] failed\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);
            }
            else
            {
                pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bFileEndRecv = HI_FALSE;
            }
        }
    }
    /*TODO: how to process when just one record task manualsplit success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" record manualsplit failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_StartRec(HI_VOID)
{
    HI_S32 s32Idx = 0;
    HI_S32 s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    if(pstStateMngCtx->stRecCtx.bRecStarted)
    {
        MLOGW("record task is started \n\n");
        return HI_SUCCESS;
    }

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable))
        {
#ifndef CONFIG_PQT_SUPPORT_ON
            HI_S32 s32Ret = HI_SUCCESS;
            s32Ret = HI_RECMNG_CreateTask(&pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr,
                                          &pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" create record task failed,s32Idx:%d,s32Ret[0x%x]\n\n"NONE, s32Idx, s32Ret);
            }
            else
            {
                pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bAudioStarted = HI_FALSE;
                s32Ret = HI_RECMNG_StartTask(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl,
                                             PDT_STATEMNG_RECMNG_OPERATION_TIMEOUT_MS);

                if (HI_SUCCESS != s32Ret)
                {
                    HI_RECMNG_DestroyTask(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);
                    s32FailCount++;
                    MLOGE(RED" start record task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl, s32Ret);
                }
                else
                {
                    pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bFileEndRecv = HI_FALSE;
                }
            }
#endif
        }
    }

    /*TODO: how to process when just one record task start success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" start record task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    HI_PDT_WORKMODE_E enCurrentWorkMode = pstStateMngCtx->enCurrentWorkMode;
    HI_BOOL bNormalRec = HI_TRUE;
    for(s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if(HI_REC_TYPE_LAPSE == pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType)
        {
            bNormalRec = HI_FALSE;
        }
    }
    if (HI_PDT_WORKMODE_NORM_REC == enCurrentWorkMode && bNormalRec)
    {
        HI_S32 s32Ret = HI_SUCCESS;
        HI_BOOL bHighTemp = HI_FALSE;
        HI_EVENT_S stEvent = {0};
        stEvent.arg1 = 0;
        s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_TEMPCYCLE_TEMP_CHANGE, &stEvent);
        if(((HI_SUCCESS != s32Ret) ||(0 == stEvent.arg1)))
        {
            bHighTemp = HI_FALSE;
        }
        else /**motion detect occur*/
        {
            bHighTemp = HI_TRUE;
        }

        PDT_STATEMNG_UpdateTempCycleState(bHighTemp);
    }
#endif
    pstStateMngCtx->stRecCtx.bRecStarted = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_StopRec(HI_BOOL bSync)
{
    HI_S32 s32Idx = 0, s32Ret = HI_FAILURE, s32FailCount = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    if(HI_TRUE!=pstStateMngCtx->stRecCtx.bRecStarted)
    {
        MLOGD("record task is not started \n\n");
        return HI_SUCCESS;
    }

    for (s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
    {
        if ((pstStateMngCtx->astCamCtx[s32Idx].bEnable) && (pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable))
        {
            if(bSync)
                s32Ret = HI_RECMNG_StopTask(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl,
                                        HI_TRUE, PDT_STATEMNG_RECMNG_NO_TIMEOUT);
            else
                s32Ret = HI_RECMNG_AsyncStopTask(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl,HI_TRUE);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" stop record task[%d] failed\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);
            }

            s32Ret = HI_RECMNG_DestroyTask(pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl);

            if (HI_SUCCESS != s32Ret)
            {
                s32FailCount++;
                MLOGE(RED" destroy record task[%d] failed,s32Ret[0x%x]\n\n"NONE, pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl, s32Ret);
            }
        }
    }

    /*TODO: how to process when just one record task stop success*/
    if (s32FailCount > 0)
    {
        MLOGE(RED" stop record task failed \n\n"NONE);
        return HI_PDT_STATEMNG_EINTER;
    }

#ifdef CONFIG_TEMPRATURE_CYCLE_ON
    HI_PDT_WORKMODE_E enCurrentWorkMode = pstStateMngCtx->enCurrentWorkMode;
    if (HI_PDT_WORKMODE_NORM_REC == enCurrentWorkMode && pstStateMngCtx->stTempCycleCtx.bHighTemp)
    {
        PDT_STATEMNG_UpdateTempCycleState(HI_FALSE);
    }
#endif

    pstStateMngCtx->stRecCtx.bRecStarted = HI_FALSE;
    return HI_SUCCESS;
}

#ifdef CONFIG_MOTIONDETECT_ON
HI_S32 PDT_STATEMNG_StartMd(HI_VOID)
{
    HI_S32 s32Idx = 0;
    HI_S32 j= 0;
    HI_S32  s32Ret = HI_FAILURE;
    HI_PDT_PARAM_VIDEODETECT_CFG_S stVideoDetectCfg;
    HI_VIDEODETECT_TASK_CFG_S  stVideoDetectTaskCfg = {0};
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        s32Ret = HI_PDT_PARAM_GetVideoDetectCfg(s32Idx, &stVideoDetectCfg);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_PDT_PARAM_GetVideoDetectCfg");

        if((HI_TRUE == stVideoDetectCfg.bEnable) && (stVideoDetectCfg.algCnt > 0))
        {
            if (pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].bVideoDetectTaskCreate)
            {
                MLOGI("[%d]Videodetect has already created. Don't create again.\n", s32Idx);
                break;;
            }
            stVideoDetectTaskCfg.algCnt = stVideoDetectCfg.algCnt;
            memcpy(&stVideoDetectTaskCfg.yuvSrc, &stVideoDetectCfg.yuvSrc, sizeof(HI_VIDEODETECT_YUV_SRC_S));
            for(j = 0; j < stVideoDetectCfg.algCnt; j++)
            {
                if(HI_VIDEODETECT_ALG_TYPE_MD == stVideoDetectCfg.algAttr[j].algType)
                {
                    stVideoDetectTaskCfg.algAttr[j].algType = stVideoDetectCfg.algAttr[j].algType;
                    memcpy(&stVideoDetectTaskCfg.algAttr[j].unAttr.mdAttr, &stVideoDetectCfg.algAttr[j].mdAttr, sizeof(MD_ATTR_S));
                    break;
                }
            }

            s32Ret = HI_VIDEODETECT_TaskCreate(&stVideoDetectTaskCfg,
                &pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].VideoDetectTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_TaskCreate");

            s32Ret = HI_VIDEODETECT_TaskStart(pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].VideoDetectTaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_TaskStart");

            pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].bVideoDetectTaskCreate = HI_TRUE;

            if(0 != stVideoDetectCfg.algAttr[j].algProcMdAttr.mdSensitivity)
            {
                HI_VIDEODETECT_ALGPROC_CFG_S algProcCfg = {0};
                algProcCfg.algProcType = HI_VIDEODETECT_ALGPROC_TYPE_EVENT;
                algProcCfg.svpAlgType = HI_VIDEODETECT_ALG_TYPE_MD;
                algProcCfg.unAlgProcAttr.algProcMdAttr.mdSensitivity = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdSensitivity;
                algProcCfg.unAlgProcAttr.algProcMdAttr.mdStableCnt = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdStableCnt;

                s32Ret = HI_VIDEODETECT_AddAlgProc(pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].VideoDetectTaskHdl,
                    &algProcCfg);
                PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_VIDEODETECT_AddAlgProc");
            }

        }

    }

return HI_SUCCESS;
}
HI_S32 PDT_STATEMNG_StopMd(HI_VOID)
{
    HI_S32 s32Idx = 0;
    HI_S32 j= 0;
    HI_S32  s32Ret = HI_FAILURE;
    HI_PDT_PARAM_VIDEODETECT_CFG_S stVideoDetectCfg;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();

    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; s32Idx++)
    {
        s32Ret = HI_PDT_PARAM_GetVideoDetectCfg(s32Idx, &stVideoDetectCfg);
        PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "HI_PDT_PARAM_GetVideoDetectCfg");

        if((HI_TRUE == stVideoDetectCfg.bEnable) && (stVideoDetectCfg.algCnt > 0))
        {
            HI_VIDEODETECT_ALGPROC_CFG_S algProcCfg = {0};
            HI_HANDLE TaskHdl = pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].VideoDetectTaskHdl;
            algProcCfg.algProcType = HI_VIDEODETECT_ALGPROC_TYPE_EVENT;
            algProcCfg.svpAlgType = HI_VIDEODETECT_ALG_TYPE_MD;
            algProcCfg.unAlgProcAttr.algProcMdAttr.mdStableCnt = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdStableCnt;
            for(j = 0; j < stVideoDetectCfg.algCnt; j++)
            {
                if(HI_VIDEODETECT_ALG_TYPE_MD == stVideoDetectCfg.algAttr[j].algType)
                {
                    algProcCfg.unAlgProcAttr.algProcMdAttr.mdSensitivity = stVideoDetectCfg.algAttr[j].algProcMdAttr.mdSensitivity;
                    if(0 != algProcCfg.unAlgProcAttr.algProcMdAttr.mdSensitivity)
                    {

                        s32Ret = HI_VIDEODETECT_RemoveAlgProc(TaskHdl, &algProcCfg);
                        PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "HI_VIDEODETECT_RemoveAlgProc");

                        break;
                    }
                }
            }

            if (!pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].bVideoDetectTaskCreate)
            {
                MLOGI("[%d]Videodetect has already destroy. Don't destroy again.\n", s32Idx);
                break;;
            }

            s32Ret = HI_VIDEODETECT_TaskStop(TaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "TaskStop");

            s32Ret = HI_VIDEODETECT_TaskDestroy(TaskHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret, s32Ret, "HI_VIDEODETECT_TaskDestroy");

            pstStateMngCtx->stVideoDetectCtx.astVideoDetectTaskCtx[s32Idx].bVideoDetectTaskCreate = HI_FALSE;
        }

    }

    return HI_SUCCESS;
}

#endif

HI_S32 PDT_STATEMNG_AddRtspStreams(const HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    /** add streams */
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg =  NULL;
    HI_HANDLE AencHdl = pstMediaCfg->stAudioCfg.astAencCfg[0].bEnable ? pstMediaCfg->stAudioCfg.astAencCfg[0].AencHdl : HI_INVALID_HANDLE;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
    {
        pstVencCfg = &(pstMediaCfg->stVideoCfg.astVencCfg[i]);

#ifdef CONFIG_PQT_SUPPORT_ON
        if (pstVencCfg->bEnable &&
            (HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCfg->enVencType ||
             HI_PDT_MEDIA_VENC_TYPE_REC == pstVencCfg->enVencType))
#else
        if (pstVencCfg->bEnable &&
            (HI_PDT_MEDIA_VENC_TYPE_LIVE == pstVencCfg->enVencType))
#endif
        {
            HI_CHAR szStreamName[HI_APPCOMM_COMM_STR_LEN];
            snprintf(szStreamName, HI_APPCOMM_COMM_STR_LEN, "%d", pstVencCfg->VencHdl);
            MLOGI(GREEN"add [ %s ]"NONE"\n", szStreamName);
            s32Ret = HI_LIVESVR_AddStream(pstVencCfg->VencHdl, AencHdl, szStreamName);

            if (HI_SUCCESS != s32Ret)
            {
                MLOGE(RED"add stream venchdl[%d] failed \n\n"NONE, pstVencCfg->VencHdl);
                return HI_PDT_STATEMNG_EINTER;
            }
        }
    }
    return HI_SUCCESS;
}

HI_VOID PDT_STATEMNG_MediaCfgResetForPlayback(HI_PDT_MEDIA_CFG_S* pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0, j = 0;
    HI_PDT_WORKMODE_CFG_S stEnterWorkModeCfg;
    pstMediaCfg->stVBCfg.u32MaxPoolCnt = 1;
    pstMediaCfg->stVBCfg.astCommPool[0].u32BlkCnt = 4;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable = HI_FALSE;
    }

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
    {
        pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].bEnable = HI_FALSE;

        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
        { pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i].astVportAttr[j].bEnable = HI_FALSE; }
    }

    /** get workmode config from Param module */
    memset(&stEnterWorkModeCfg, 0, sizeof(HI_PDT_WORKMODE_CFG_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(HI_PDT_WORKMODE_PLAYBACK, &stEnterWorkModeCfg);
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "get stEnterWorkModeCfg parameter");
    HI_PDT_PARAM_PLAYBACK_CFG_S*  pstPlayBackCfg = &stEnterWorkModeCfg.unModeCfg.stPlayBackCfg;


    if (HI_LITEPLAYER_VOUT_TYPE_VPSS == pstPlayBackCfg->stVoutOpt.enVoutType)
    {
        HI_BOOL bFound = HI_FALSE;

        for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
        {
            HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr =
                &(pstMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i]);
            pstVpssAttr->bEnable = HI_FALSE;

            if (pstVpssAttr->VpssHdl == pstPlayBackCfg->stVoutOpt.hModHdl)
            {
                for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
                {
                    if (pstVpssAttr->astVportAttr[j].VportHdl == pstPlayBackCfg->stVoutOpt.hChnHdl)
                    {
                        pstVpssAttr->bEnable = HI_TRUE;
                        pstVpssAttr->stVpssAttr.bNrEn = HI_FALSE;
                        pstVpssAttr->stVpssAttr.enPixelFormat = HI_MEDIA_PIXEL_FMT_YUV_SEMIPLANAR_420;
                        pstVpssAttr->astVportAttr[j].bEnable = HI_TRUE;
                        pstVpssAttr->stVpssAttr.stFrameRate.s32SrcFrameRate = -1;
                        pstVpssAttr->stVpssAttr.stFrameRate.s32DstFrameRate = -1;
                        bFound = HI_TRUE;
                        break;
                    }
                    else
                    {
                        pstVpssAttr->astVportAttr[j].bEnable = HI_FALSE;
                    }
                }

                if (HI_TRUE == bFound)
                { break; }
            }
        }

        HI_APPCOMM_CHECK_EXPR_WITHOUT_RETURN(HI_TRUE == bFound, "vpss param invald");
    }

    /** change dispcfg */
    for (i = 0; i < HI_PDT_MEDIA_DISP_MAX_CNT; i++)
    {
#ifdef REARVIEW

        if (ROTATION_90 == pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].enRotate)
        {
            pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.s32Y =
                pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Height / 2;
            pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Height =
                pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Height / 2 - UI_FILELIST_WINDOW_LEFT_LEN;
        }
        else
        {
            pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.s32X =
                pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Width / 2;
            pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Width =
                pstMediaCfg->stVideoOutCfg.astDispCfg[i].astWndCfg[0].stWndAttr.stRect.u32Width / 2 - UI_FILELIST_WINDOW_LEFT_LEN;
        }

#endif

        HI_PDT_MEDIA_DISP_CFG_S* pstDispCfg =
            &(pstMediaCfg->stVideoOutCfg.astDispCfg[i]);

        if (HI_LITEPLAYER_VOUT_TYPE_VPSS == pstPlayBackCfg->stVoutOpt.enVoutType)
        {
            pstDispCfg->astWndCfg[0].enBindedMod = HI_PDT_MEDIA_VIDEOMOD_VPSS;
            pstDispCfg->astWndCfg[0].ModHdl = pstPlayBackCfg->stVoutOpt.hModHdl;
            pstDispCfg->astWndCfg[0].ChnHdl = pstPlayBackCfg->stVoutOpt.hChnHdl;
        }
        else
        { pstDispCfg->astWndCfg[0].enBindedMod = HI_PDT_MEDIA_VIDEOMOD_DISP; }

        for (i = 0; i < HI_PDT_MEDIA_DISP_WND_MAX_CNT; i++)
        {
            pstDispCfg->astWndCfg[i].stCropCfg.bEnable = HI_FALSE;
        }

        for(i=1;i<HI_PDT_MEDIA_DISP_WND_MAX_CNT;i++)
        {
            pstDispCfg->astWndCfg[i].bEnable = HI_FALSE; /* only open first vo_wnd in playback mode */
        }

    }

    /** change venccfg*/
    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
    {
        pstMediaCfg->stVideoCfg.astVencCfg[i].bEnable = HI_FALSE;
    }

    HI_PDT_MEDIA_AO_CFG_S* pstAoCfg = &(pstMediaCfg->stAudioOutCfg.astAoCfg[0]);
    pstAoCfg->AoHdl = pstPlayBackCfg->stAoutOpt.hAudTrackHdl;

    for(i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
    {
        pstMediaCfg->stAudioCfg.astAencCfg[i].bEnable = HI_FALSE;
    }
    /** change osd*/
    pstMediaCfg->stVideoCfg.stOsdCfg.s32OsdCnt = 0;
}

HI_S32 PDT_STATEMNG_GetPowerSupplyStatue(HI_BOOL* pbIsOn)
{

    HI_APPCOMM_CHECK_POINTER(pbIsOn, HI_FAILURE);
    *pbIsOn = HI_FALSE;
    if (-1 != access("acc_off", F_OK))
    {
        MLOGI("acc_off exist\n");
        *pbIsOn = HI_TRUE;
    }

    return HI_SUCCESS;
}
HI_S32 PDT_STATEMNG_PowerSupplyCheckTaskCB(HI_VOID* pvPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bIsOn = HI_TRUE;
    s32Ret = PDT_STATEMNG_GetPowerSupplyStatue(&bIsOn);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "PowerSupplyCheck callback");

    if (HI_FALSE == bIsOn)
    {
        HI_EVENT_S stEvent;
        memset(&stEvent, 0 , sizeof(HI_EVENT_S));
        stEvent.EventID = HI_EVENT_STATEMNG_CHECK_ACC_OFF;
        HI_EVTHUB_Publish(&stEvent);
    };

    return s32Ret;
}

HI_S32 PDT_STATEMNG_CreatePowerSupplyCheckTask()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_TIMEDTASK_CFG_S stTimeTsdkAttr;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    stTimeTsdkAttr.stAttr.bEnable = HI_TRUE;
    stTimeTsdkAttr.stAttr.u32Time_sec = 1;
    stTimeTsdkAttr.pfnProc = PDT_STATEMNG_PowerSupplyCheckTaskCB;
    stTimeTsdkAttr.pvPrivData = (HI_VOID*)&pstStateMngCtx;
    s32Ret = HI_TIMEDTASK_Create(&stTimeTsdkAttr, &pstStateMngCtx->stParkingRecCtx.TimeTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "creat PowerSupplyCheck task");
    return s32Ret;
}
HI_S32 PDT_STATEMNG_DestroyPowerSupplyCheckTask()
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    s32Ret = HI_TIMEDTASK_Destroy(pstStateMngCtx->stPowerCheckCtx.TimeTaskHdl);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "destroy PowerSupplyCheck task");
    return s32Ret;
}

HI_S32 PDT_STATEMNG_GetLiveCamID(HI_VOID)
{
    return PDT_STATEMNG_GetCtx()->s32LiveCamID;
}

HI_S32 PDT_STATEMNG_GetPreViewCamID(HI_VOID)
{
    return PDT_STATEMNG_GetCtx()->s32PreviewCamID;
}

HI_S32 PDT_STATEMNG_InitFileMng(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();

    /** init filemng */
    HI_PDT_FILEMNG_CFG_S stFileMngCfg;
    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"GetFileMngCfg");

    HI_STORAGE_FS_INFO_S stFSInfo = {0};

    s32Ret = HI_STORAGEMNG_GetFSInfo(pstStateMngCtx->stStorageMngCfg.szMntPath, &stFSInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HI_STORAGEMNG_GetFSInfo");

    stFileMngCfg.stDtcfCfg.u32WarningStage = stFSInfo.u64TotalSize * stFileMngCfg.stDtcfCfg.u32WarningStage/100 >> 20;
    stFileMngCfg.stDtcfCfg.u32GuaranteedStage=stFSInfo.u64TotalSize * stFileMngCfg.stDtcfCfg.u32GuaranteedStage/100 >> 20;
    s32Ret = HI_FILEMNG_Init(&stFileMngCfg.stCommCfg, &stFileMngCfg.stDtcfCfg);
    if(HI_FILEMNG_EINITIALIZED == s32Ret)
    {
        MLOGI(BLUE"FileMng already inited\n"NONE);
        s32Ret = HI_SUCCESS;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,HI_PDT_STATEMNG_EINTER,"HI_FILEMNG_Init");
    }
    MLOGD("Init FileMng ok!\n");
#ifdef __HuaweiLite__
    extern HI_S32 PDT_AutoSync(HI_S32 argc, HI_CHAR* argv[]);
    HI_CHAR* argv[1] = {"20"};
    PDT_AutoSync(1, argv);
#ifndef HI_DTCF_SCAN_FILE_AMOUNT_MAX
#define HI_DTCF_SCAN_FILE_AMOUNT_MAX (10240)
#endif
    extern void set_max_dir_entry(int num);
    set_max_dir_entry(HI_DTCF_SCAN_FILE_AMOUNT_MAX);
    MLOGI("set_max_dir_entry(%d)\n", HI_DTCF_SCAN_FILE_AMOUNT_MAX);
#else
    /**Improving sd writing performance*/
    s32Ret = HI_system("echo 256 > /sys/block/mmcblk0/queue/nr_requests");
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"set nr_requests fail");
    s32Ret = HI_system("echo 4096 > /proc/sys/vm/min_free_kbytes");
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"set nr_requests fail");
#endif
    return s32Ret;
}

HI_VOID PDT_STATEMNG_SendShutdownMsg(HI_VOID)
{
    HI_EVENT_S stEvent;
    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
    stEvent.EventID = HI_EVENT_STATEMNG_SHUTDOWN_PROC;
    HI_EVTHUB_Publish(&stEvent);
}

/** get PhotoMng task file name */
HI_S32 PDT_STATEMNG_GetPhotoFileName(HI_S32 s32FileCnt,
        HI_PHOTOMNG_FILENAME_S* pstFileList,
        HI_VOID* pvPrivateData)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_DTCF_DIR_E enDtcfDir = DTCF_DIR_BUTT;

    /** check parameters */
    PDT_STATEMNG_CHECK_POINTER(pstFileList, HI_PDT_STATEMNG_ENULLPTR, "pstFileList");
    /** TODO: check this expression is correct*/
    PDT_STATEMNG_CHECK_EXPR( (1 == s32FileCnt), HI_PDT_STATEMNG_EINVAL);
    PDT_STATEMNG_PHOTOTASK_CONTEXT_S* pstPhotoTaskCtx = (PDT_STATEMNG_PHOTOTASK_CONTEXT_S*)pvPrivateData;

    /** get file name */
    HI_FILEMNG_OBJ_FILENAME_S stFileName;
    memset(&stFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    if (0 == pstPhotoTaskCtx->s32CamID)
    {
        enDtcfDir = DTCF_DIR_PHOTO_FRONT;
    }
    else if (1 == pstPhotoTaskCtx->s32CamID)
    {
        enDtcfDir = DTCF_DIR_PHOTO_REAR;
    }
    else
    {
        MLOGE("record task hanlde[%d] 's  CamID[%d] error \n", pstPhotoTaskCtx->PhotoMngTaskHdl, pstPhotoTaskCtx->s32CamID);
        return HI_PDT_STATEMNG_EINVAL;
    }
    s32Ret = HI_FILEMNG_GenerateFileName(HI_DTCF_FILE_TYPE_JPG, enDtcfDir, HI_TRUE, &stFileName);
    PDT_STATEMNG_CHECK_RET(s32Ret, HI_PDT_STATEMNG_EINTER, "get photo file name");

    if (s32FileCnt > stFileName.u8FileCnt)
    {
        MLOGE(" PhotoFileCnt[%d] is larger than FileCnt[%d] \n", s32FileCnt, stFileName.u8FileCnt);
        return HI_PDT_STATEMNG_EINVAL;
    }

    HI_S32 i = 0;
    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    MUTEX_LOCK(pstStateMngCtx->Mutex);

    for (i = 0; i < s32FileCnt; i++)
    {
        snprintf((HI_CHAR*)pstFileList[i].szFileName, HI_APPCOMM_MAX_FILENAME_LEN, "%s", stFileName.szFileName[i]);
        pstFileList[i].u32PreAllocLen = pstStateMngCtx->stFileMngCfg.stDtcfCfg.u32PreAllocUnit[pstPhotoTaskCtx->s32CamID];
    }

    MUTEX_UNLOCK(pstStateMngCtx->Mutex);

    return s32Ret;
}

/** get PhotoMng param */
HI_S32 PDT_STATEMNG_GetPhotoParam(HI_PHOTOMNG_PHOTO_PARAM_S* pstPhotoParam, HI_VOID* pvPrivateData)
{
    PDT_STATEMNG_CHECK_POINTER(pstPhotoParam, HI_PDT_STATEMNG_ENULLPTR, "pstPhotoParam");
    PDT_STATEMNG_CHECK_POINTER(pstPhotoParam, HI_PDT_STATEMNG_ENULLPTR, "pvPrivateData");

    MLOGD(YELLOW"call PDT_STATEMNG_GetPhotoParam, use sample data currently\n\n"NONE);
    memset(pstPhotoParam, 0, sizeof(HI_PHOTOMNG_PHOTO_PARAM_S));

    PDT_STATEMNG_CONTEXT* pstStateMngCtx = PDT_STATEMNG_GetCtx();
    for (HI_S32 i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(pvPrivateData == &pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[i])
        {
            if(HI_FALSE == pstStateMngCtx->stPhotoCtx.stPhotoTaskCtx[i].bISPBypss)
            {
                /*init exif info*/
                HI_PDT_PARAM_GetExifInfo(&pstPhotoParam->stExifInfo);
            }
            return HI_SUCCESS;
        }
    }
    MLOGI("NOT found CAM \n");
    return HI_FAILURE;
}

HI_VOID PDT_STATEMNG_EmrTaskEndProc(HI_EVENT_RECMNG_E entype,HI_HANDLE RecMngTaskHdl,HI_BOOL bForceEnd)
{
    HI_S32 s32Idx = 0;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    if(HI_TRUE == bForceEnd)
    {
        if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
        {
            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateMngCtx->stRecCtx.bEMRRecord = HI_FALSE;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);

            HI_EVENT_S stEvent;
            memset(&stEvent, 0 , sizeof(HI_EVENT_S));
            stEvent.EventID = HI_EVENT_STATEMNG_EMR_END;
            stEvent.s32Result = HI_SUCCESS;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("EMR END has been sent\n");
         }
        return;
    }
    switch(entype)
    {
        case HI_EVENT_RECMNG_MANUAL_SPLIT_FILE_END:
            for(s32Idx = 0; s32Idx < HI_PDT_RECTASK_MAX_CNT; s32Idx++)
            {
                if(HI_REC_TYPE_LAPSE ==
                    pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].stRecMngAttr.enRecType)
                {
                    return; /** when lapse rec,emr task end just depend on auto stop event*/
                }
            }
            break;
        case HI_EVENT_RECMNG_TASK_AUTO_STOP:
            break;
        default:
            return; /** No correlation between event with Emr file end*/

    }
    for(s32Idx = 0;s32Idx < HI_PDT_RECTASK_MAX_CNT;s32Idx++)
    {
        if(RecMngTaskHdl == pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].RecMngTaskHdl)
            pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bFileEndRecv = HI_TRUE;
    }
    for(s32Idx = 0;s32Idx < HI_PDT_RECTASK_MAX_CNT;s32Idx++)
    {
        if(pstStateMngCtx->astCamCtx[s32Idx].bEnable &&
            pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bEnable)
        {
            if(HI_FALSE == pstStateMngCtx->stRecCtx.astRecTaskCtx[s32Idx].bFileEndRecv)
                break;
        }
    }

    if(s32Idx == HI_PDT_RECTASK_MAX_CNT)
    {
        if(HI_TRUE == pstStateMngCtx->stRecCtx.bEMRRecord)
        {
            MUTEX_LOCK(pstStateMngCtx->Mutex);
            pstStateMngCtx->stRecCtx.bEMRRecord = HI_FALSE;
            MUTEX_UNLOCK(pstStateMngCtx->Mutex);

            HI_EVENT_S stEvent;
            memset(&stEvent, 0 , sizeof(HI_EVENT_S));
            stEvent.EventID = HI_EVENT_STATEMNG_EMR_END;
            stEvent.s32Result = HI_SUCCESS;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD("EMR END has been sent\n");
         }
    }
    else
    {
        MLOGI("record task s32Idx[%d],file end not rec\n",s32Idx);
    }
}

HI_VOID PDT_STATEMNG_MountedMsgProc(HI_PDT_WORKMODE_E enPoweronWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bFoundNewPkt = HI_FALSE;
    PDT_STATEMNG_CONTEXT *pstStateMngCtx = PDT_STATEMNG_GetCtx();
    HI_EVENT_S stEvent = {0};
    HI_TIME_STAMP
    /**judge upgrade*/
    s32Ret = PDT_STATEMNG_UpgradeSrchNewPkt();
    if(HI_UPGRADE_PKT_AVAILABLE != s32Ret)/**upgrade packet not found*/
    {
        if(HI_SUCCESS != s32Ret)
        {
           MLOGE(RED"PDT_STATEMNG_UpgrateSrchNewPkt failed(0x%08X)\n"NONE, s32Ret);
        }
    }
    else
    {
        bFoundNewPkt = HI_TRUE;
        MLOGI(RED"find upgrade packet\n"NONE);
    }
    HI_TIME_STAMP

    /** init filemng */
    s32Ret = PDT_STATEMNG_InitFileMng();
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"InitFileMng");

    HI_TIME_STAMP
    s32Ret = HI_FILEMNG_SetDiskState(HI_TRUE);
    if(HI_SUCCESS != s32Ret)
    {
        pstStateMngCtx->bSDAvailable = HI_FALSE;
        if(HI_FALSE == bFoundNewPkt)
        {
            HI_EVENT_S stEvent;
            memset(&stEvent, 0 , sizeof(HI_EVENT_S));
            stEvent.EventID = HI_EVENT_STATEMNG_SD_UNAVAILABLE;
            HI_EVTHUB_Publish(&stEvent);
            MLOGD(YELLOW"publish HI_EVENT_STATEMNG_SD_UNAVAILABLE\n\n"NONE);
            HI_FILEMNG_SetDiskState(HI_FALSE);
            if (HI_PDT_WORKMODE_PARKING_REC != enPoweronWorkmode)
            {
                s32Ret = PDT_STATEMNG_SetUsbMode();
                PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
            }
            return;
        }
    }
    else
    {
        if (HI_PDT_WORKMODE_PARKING_REC != enPoweronWorkmode)
        {
            s32Ret = HI_FILEMNG_CheckDiskSpace();
            if(HI_FILEMNG_EUNIDENTIFICATION == s32Ret)
            {
                pstStateMngCtx->bSDAvailable = HI_FALSE;
                if(HI_FALSE == bFoundNewPkt)
                {
                    HI_EVENT_S stEvent;
                    memset(&stEvent, 0 , sizeof(HI_EVENT_S));
                    stEvent.EventID = HI_EVENT_STATEMNG_SD_UNAVAILABLE;
                    HI_EVTHUB_Publish(&stEvent);
                    MLOGD(YELLOW"publish HI_EVENT_STATEMNG_SD_UNAVAILABLE\n\n"NONE);
                    HI_FILEMNG_SetDiskState(HI_FALSE);
                    s32Ret = PDT_STATEMNG_SetUsbMode();
                    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
                    return;
                }
            }
        }
    }

    HI_TIME_STAMP
    s32Ret = HI_FILEMNG_Repair();
    PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"filemng repair");
    HI_TIME_STAMP
    HI_system("echo 3 > /proc/sys/vm/drop_caches");
    pstStateMngCtx->enPowerOnState = HI_STORAGE_STATE_MOUNTED;
    s32Ret = HI_EVTHUB_GetEventHistory(HI_EVENT_STORAGEMNG_MOUNTED, &stEvent);
    if(!(HI_STORAGE_FSTOOL_ERR_FRAGMENT_SEVERELY == stEvent.arg1))
    {
#ifndef NONESCREEN
        /*check SD speed*/
        HI_STORAGEMNG_CFG_S stStorageCfg = {0};
        HI_STORAGE_DEV_INFO_S stDevInfo = {0};
        s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"get storage config failed");
        s32Ret = HI_STORAGEMNG_GetDevInfo(stStorageCfg.szMntPath, &stDevInfo);
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret,"get dev info failed");
        MLOGI(YELLOW"StoarageDevType: %s"NONE"\n", stDevInfo.aszDevType);

        if ((NULL == strstr(stDevInfo.aszDevType, "MMC"))
            && (HI_STORAGE_TRANSMISSION_SPEED_1_4M == stDevInfo.enTranSpeed
                || HI_STORAGE_TRANSMISSION_SPEED_4_10M == stDevInfo.enTranSpeed) )
        {
            MLOGW("SD speed[%d]\n",stDevInfo.enTranSpeed);
        }
        else
#endif
        {

            pstStateMngCtx->bSDAvailable = HI_TRUE;
            stEvent.EventID = HI_EVENT_STATEMNG_SD_AVAILABLE;
            s32Ret = HI_EVTHUB_Publish(&stEvent);
            PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "publish HI_EVENT_STATEMNG_SD_AVAILABLE event");
            MLOGI("HI_EVENT_STATEMNG_SD_AVAILABLE publish success\n");
            HI_PrintBootTime("publish SD_AVAILABLE event");
        }
    }
    if (HI_PDT_WORKMODE_PARKING_REC != enPoweronWorkmode)
    {
        s32Ret = PDT_STATEMNG_SetUsbMode();
        PDT_STATEMNG_CHECK_RET_WITH_UNRETURN(s32Ret, "set usb mode");
    }
    return;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
