/**
 * \file   hi_product_statemng_common.c
 * \brief  Realize the common functions for statemng module.
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
extern pthread_mutex_t g_StartMutex;
extern pthread_cond_t  g_StartCond;
extern HI_BOOL g_bRecStarted;

static HI_S32 PDT_STATEMNG_GetRecTaskFileName(HI_RECMNG_FILENAME_S* pstFilename, HI_VOID *pvPrivData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstFilename,"pszFilename");

    /** get file name */
    HI_FILEMNG_OBJ_FILENAME_S stFileName;
    memset(&stFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    s32Ret = HI_FILEMNG_GenerateFileName(g_stSTATEMNGContext.stRecMngCtx.u8FileMngObjCfgIdx,
        g_stSTATEMNGContext.stRecMngCtx.bFileMngNewGrp, &stFileName);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get file name");

    /** copy file name to RecMng array */
    pstFilename->u32RecFileCnt = HI_PDT_REC_VSTREAM_MAX_CNT;
    HI_U8 i = 0;
    for(i = 0; ((i < pstFilename->u32RecFileCnt) && (i < HI_RECMNG_TASK_MAX_FILE_CNT)); i++)
    {
        snprintf(pstFilename->aszRecFilename[i], HI_APPCOMM_MAX_FILENAME_LEN, "%s", stFileName.szFileName[i]);
        MLOGI(YELLOW"RecFileName[%d](%s)"NONE"\n", i, pstFilename->aszRecFilename[i]);
#ifdef CONFIG_RAWCAP_ON
        if(HI_TRUE == g_stSTATEMNGContext.bRawCapOn)
        {
            if((0 == i) &&
               (HI_PDT_WORKMODE_NORM_REC == g_stSTATEMNGContext.enCurrentWorkMode))
            {
                (HI_VOID)sscanf(stFileName.szFileName[i], "%[^.].%*[^.]", g_stSTATEMNGContext.szPrefix);
                MLOGD(YELLOW"rawcap task name prefix(%s)\n"NONE, g_stSTATEMNGContext.szPrefix);
            }
        }
#endif
    }
    pstFilename->u32ThmFileCnt = 1;
    snprintf(pstFilename->aszThmFilename[0], HI_APPCOMM_MAX_FILENAME_LEN, "%s", stFileName.szFileName[i]);
    MLOGI(YELLOW"ThmFileName(%s)"NONE"\n", pstFilename->aszThmFilename[0]);

    if(HI_PDT_WORKMODE_LOOP_REC == g_stSTATEMNGContext.enCurrentWorkMode)
    {
        g_stSTATEMNGContext.stRecMngCtx.bFileMngNewGrp = HI_FALSE;
    }

    if(HI_PDT_WORKMODE_RECSNAP == g_stSTATEMNGContext.enCurrentWorkMode)
    {
        g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp = HI_TRUE;
        HI_MUTEX_LOCK(g_StartMutex);
        g_bRecStarted = HI_TRUE;
        HI_COND_SIGNAL(g_StartCond);
        MLOGI(YELLOW"RecMng task got name, post cond signal, set g_bRecStarted HI_TRUE\n"NONE);
        HI_MUTEX_UNLOCK(g_StartMutex);
    }

    return s32Ret;
}

/** get PhotoMng task file name */
static HI_S32 PDT_STATEMNG_GetPhotoTaskFileName(HI_S32 s32FileCnt,
    HI_PHOTOMNG_FILENAME_S* pstFileList, HI_VOID *pvPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameters */
    HI_APPCOMM_CHECK_POINTER(pstFileList, HI_PDT_STATEMNG_ENULLPTR);

    MLOGD(YELLOW"FileCnt(%d)\n"NONE, s32FileCnt);

    /** get file name */
    HI_FILEMNG_OBJ_FILENAME_S stFileName;
    memset(&stFileName, 0, sizeof(HI_FILEMNG_OBJ_FILENAME_S));
    s32Ret = HI_FILEMNG_GenerateFileName(g_stSTATEMNGContext.stPhotoMngCtx.u8FileMngObjCfgIdx,
        g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp, &stFileName);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_PDT_STATEMNG_EINTER);

    /** copy file name to PhotoMng array */
    HI_U8 i =0;
    for(i = 0; i < stFileName.u8FileCnt; i++, pstFileList++)
    {
        snprintf(pstFileList->szFileName, HI_APPCOMM_MAX_PATH_LEN, "%s", stFileName.szFileName[i]);
        pstFileList->u32PreAllocLen = 0; /**<no preallocate */
        MLOGI(YELLOW"PhotoFileName[%d](%s)\n"NONE, i, stFileName.szFileName[i]);
#ifdef CONFIG_RAWCAP_ON
        if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
           (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
        {
            if((0 == i) &&
               (HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode))
            {
                (HI_VOID)sscanf(stFileName.szFileName[i], "%[^.].%*[^.]", g_stSTATEMNGContext.szPrefix);
                MLOGD(YELLOW"rawcap task name prefix(%s)\n"NONE, g_stSTATEMNGContext.szPrefix);
            }
        }
#endif
    }

    if((HI_PDT_WORKMODE_LPSE_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
       (HI_PDT_WORKMODE_BURST == g_stSTATEMNGContext.enCurrentWorkMode) ||
       (HI_PDT_WORKMODE_RECSNAP == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        g_stSTATEMNGContext.stPhotoMngCtx.bFileMngNewGrp = HI_FALSE;
    }

    return s32Ret;
}

static HI_S32 PDT_STATEMNG_GetVcapPipeHdlIndex(HI_HANDLE VcapPipeHdl, HI_S32* ps32Index)
{
    HI_S32 i = 0, j = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr =
            &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i]);

        if(!pstVcapDevAttr->bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr =
                &(pstVcapDevAttr->astVcapPipeAttr[j]);

            if(!pstVcapPipeAttr->bEnable)
            {
                continue;
            }

            if(VcapPipeHdl == pstVcapPipeAttr->VcapPipeHdl)
            {
                *ps32Index = j;
                return HI_SUCCESS;
            }
        }
    }

    return HI_FAILURE;
}

/*Exposure metering method. '0' means unknown, '1' average, '2' center weighted average, '3'
spot, '4' multi-spot, '5' multi-segment, '6' partial, '255' other*/
static HI_S32 PDT_STATEMNG_GetMeteringType(HI_U32*  u32MeteringMode)
{
#define PDT_STATEMNG_PHOTO_METERINGTYPE_UNKNOWN     (0)
#define PDT_STATEMNG_PHOTO_METERINGTYPE_AVERAGE     (1)
#define PDT_STATEMNG_PHOTO_METERINGTYPE_CENTER      (2)
#define PDT_STATEMNG_PHOTO_METERINGTYPE_SPOT        (3)
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_SCENE_METRY_TYPE_E enMeterType;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(g_stSTATEMNGContext.enCurrentWorkMode,
    HI_PDT_PARAM_TYPE_PROTUNE_METRY, (HI_VOID *)&enMeterType);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get Metry param");

    switch (enMeterType)
    {
        case HI_PDT_SCENE_METRY_TYPE_CENTER:
        {
            *u32MeteringMode = PDT_STATEMNG_PHOTO_METERINGTYPE_CENTER;
            break;
        }
        case HI_PDT_SCENE_METRY_TYPE_AVERAGE:
        {
            *u32MeteringMode = PDT_STATEMNG_PHOTO_METERINGTYPE_AVERAGE;
            break;
        }

        case HI_PDT_SCENE_METRY_TYPE_SPOT:
        {
            *u32MeteringMode = PDT_STATEMNG_PHOTO_METERINGTYPE_SPOT;
            break;
        }
        case HI_PDT_SCENE_METRY_TYPE_BUTT:
        {
            *u32MeteringMode = PDT_STATEMNG_PHOTO_METERINGTYPE_UNKNOWN;
            break;
        }
        default:
            MLOGW(RED"unkown MeterType \n"NONE);
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}


#ifdef CFG_POST_PROCESS
/** get PhotoMng param */
static HI_S32 PDT_STATEMNG_GetPhotoParam(HI_PHOTOMNG_PHOTO_PARAM_S* pstPhotoParam,
    HI_VOID* pvPrivateData)
{
    PDT_STATEMNG_CHECK_NULL_PTR(pstPhotoParam,"pstPhotoParam");
    memset(pstPhotoParam, 0, sizeof(HI_PHOTOMNG_PHOTO_PARAM_S));

    /*set exif info*/
    PDT_STATEMNG_GetCamExifInfo(&pstPhotoParam->stExifInfo);
    PDT_STATEMNG_GetMeteringType(&(pstPhotoParam->stExifInfo.u32MeteringMode));

    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapPipeHdl =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.stPhotoSRC.VcapPipeHdl;

    HI_PDT_SCENE_POSTPROCESS_INFO_S stPostProcessInfo;
    HI_PDT_SCENE_LONGEXP_PARAM_S  stLongExpParam;

#ifdef CONFIG_RAWCAP_ON
    HI_HANDLE VprocHdl =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.stPhotoSRC.VprocHdl;
    HI_HANDLE VportHdl =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.stPhotoSRC.VportHdl;

    HI_PHOTO_DUMP_CALLBACK_FUNC_S stPhotoDumpCBFunc;
    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;
#endif

    HI_U8 u8PipeParamIndex = 0;
    HI_PDT_PARAM_SCENE_TYPE_E enSceneType = HI_PDT_PARAM_SCENE_TYPE_BUTT;
    HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_BUTT;

    if ((HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_LPSE_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        switch(g_stSTATEMNGContext.enScene)
        {
            case HI_PDT_PHOTO_SCENE_NORM:
            {
                /** get exptime */
                if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO == 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:%d UI ISO:AUTO\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32SysGain =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpGain(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us, &u32SysGain);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        u32SysGain;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us == 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32Time =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpTime(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO, &u32Time);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        u32Time;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:%d, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else
                {
                    HI_PDT_SCENE_GetDetectPostProcessInfo(VcapPipeHdl, &stPostProcessInfo);

                    if (stPostProcessInfo.stMFNRParam.bMFNR ||
                        stPostProcessInfo.stSFNRParam.bSFNR ||
                        stPostProcessInfo.stHDRParam.bHDR)
                    {
                        enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_POSTPROCESS;
                    }
                    else
                    {
                        enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NORMAL;
                    }

                    s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                        &stLongExpParam);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                    if (HI_PDT_WORKMODE_LPSE_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode)
                    {
                        stPostProcessInfo.stMFNRParam.bMFNR = HI_FALSE;
                        stPostProcessInfo.stSFNRParam.bSFNR = HI_FALSE;
                        stPostProcessInfo.stHDRParam.bHDR = HI_FALSE;
                    }

                    if(stPostProcessInfo.stHDRParam.bHDR)
                    {
                        MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bHDR\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = PHOTO_HDR_FRAME_NUM;
                        pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                            OPERATION_MODE_MANUAL;
#if 1
                        /** HDR can do LongExp*/
                        if(stLongExpParam.bLongExp)
                        {
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                                stLongExpParam.stAeRoute.u32ExpTime/2;
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                                stLongExpParam.stAeRoute.u32SysGain/2;

                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                                stLongExpParam.stAeRoute.u32ExpTime;
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                                stLongExpParam.stAeRoute.u32SysGain;

                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                                stLongExpParam.stAeRoute.u32ExpTime * pow(2.0, 0.5);
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                                stLongExpParam.stAeRoute.u32SysGain * pow(2.0, 0.5);

                            MLOGD("[HDR] ***LongExp***\n");
                        } else {
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32ExpTime;
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32SysGain;

                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32ExpTime;
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32SysGain;

                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32ExpTime;
                            pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                                stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32SysGain;

                            MLOGD("[HDR] ***Dynamic***\n");
                        }
                        MLOGD("[HDR] exp0 = %d, exp1 = %d, exp2 = %d\n",
                              pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0],
                              pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1],
                              pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2]);
#else
                        /** HDR cannot do LongExp*/
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32SysGain;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32SysGain;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                            stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32SysGain;
#endif
                        HI_PDT_SCENE_LockAWB(VcapPipeHdl, HI_TRUE);
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_HDR;
#ifdef CONFIG_RAWCAP_ON
                        if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                        {
                            stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                            stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                            stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                            stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);

                            g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                            g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                        }

                        HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                        enSceneType = HI_PDT_PARAM_SCENE_TYPE_HDR;
                    }
                    else
                    {
                        if (stPostProcessInfo.stMFNRParam.bMFNR ||
                            stPostProcessInfo.stSFNRParam.bSFNR )
                        {
                            enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_POSTPROCESS;
                        }
                        else
                        {
                            enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NORMAL;
                        }

                        s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                            &stLongExpParam);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                        if(stPostProcessInfo.stMFNRParam.bMFNR)
                        {
                            if(stLongExpParam.bLongExp)
                            {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(1), bMFNR\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = PHOTO_MFNR_FRAME_NUM;
                                pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                                    OPERATION_MODE_MANUAL;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[3] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[3] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_MFNR_DE;
#ifdef CONFIG_RAWCAP_ON
                                if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                                {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = HI_RAWCAP_SaveBNR;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_FALSE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_MFNR;
                            }
                            else
                            {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(0), bMFNR\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = PHOTO_MFNR_FRAME_NUM;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_MFNR_DE;
#ifdef CONFIG_RAWCAP_ON
                                if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                                {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = HI_RAWCAP_SaveBNR;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_FALSE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_MFNR;
                                HI_PDT_SCENE_LockAE(VcapPipeHdl, HI_TRUE);
                            }
                            HI_PDT_SCENE_LockAWB(VcapPipeHdl, HI_TRUE);
                        }
                        else if (stPostProcessInfo.stSFNRParam.bSFNR)
                        {
                            if(stLongExpParam.bLongExp)
                            {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(1), bSFNR\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                                pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                                    OPERATION_MODE_MANUAL;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_SFNR;
#ifdef CONFIG_RAWCAP_ON
                                if ((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)) {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE *) & (g_stSTATEMNGContext.RawCapTaskHdl);

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                            } else {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(0), bSFNR\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_SFNR;
#ifdef CONFIG_RAWCAP_ON
                                if ((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt)) {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE *) & (g_stSTATEMNGContext.RawCapTaskHdl);

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                            }
                        } else {
                            if(stLongExpParam.bLongExp && HI_PDT_WORKMODE_LPSE_PHOTO != g_stSTATEMNGContext.enCurrentWorkMode)
                            {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(1), bSFNR(0) and bMFNR(0)\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                                pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                                    OPERATION_MODE_MANUAL;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                                    stLongExpParam.stAeRoute.u32ExpTime;
                                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                                    stLongExpParam.stAeRoute.u32SysGain;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                                if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                                {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                            }
                            else
                            {
                                MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(0), bSFNR(0) and bMFNR(0)\n"NONE);
                                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
                                pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                                if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                                (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                                {
                                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                                }

                                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                                enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                            }
                        }
                    }
                }
                break;
            }
            case HI_PDT_PHOTO_SCENE_HDR:
            {
                MLOGI(YELLOW"Scene:HDR\n"NONE);
                HI_PDT_SCENE_GetDetectPostProcessInfo(VcapPipeHdl, &stPostProcessInfo);
                pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;

                enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NORMAL;
                s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                    &stLongExpParam);
                PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = PHOTO_HDR_FRAME_NUM;
                pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                    OPERATION_MODE_MANUAL;

#if 1
                if(stLongExpParam.bLongExp)
                {
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        stLongExpParam.stAeRoute.u32ExpTime/2;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        stLongExpParam.stAeRoute.u32SysGain/2;

                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                        stLongExpParam.stAeRoute.u32ExpTime;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                        stLongExpParam.stAeRoute.u32SysGain;

                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                        stLongExpParam.stAeRoute.u32ExpTime * pow(2.0, 0.5);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                        stLongExpParam.stAeRoute.u32SysGain * pow(2.0, 0.5);

                    MLOGD("[HDR] ***LongExp***\n");
                } else {
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32ExpTime;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32SysGain;

                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32ExpTime;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32SysGain;

                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32ExpTime;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                        stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32SysGain;

                    MLOGD("[HDR] ***Dynamic***\n");
                }

                MLOGD("[HDR] exp0 = %d, exp1 = %d, exp2 = %d\n",
                      pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0],
                      pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1],
                      pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2]);
#else
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32ExpTime;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[0].u32SysGain;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[1] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32ExpTime;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[1] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[1].u32SysGain;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[2] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32ExpTime;
                pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[2] =
                    stPostProcessInfo.stHDRParam.astHDRAeRoute[2].u32SysGain;
#endif

                HI_PDT_SCENE_LockAWB(VcapPipeHdl, HI_TRUE);
                pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_HDR;
#ifdef CONFIG_RAWCAP_ON
                if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                {
                    stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                    stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                    stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = HI_RAWCAP_SaveYUV;
                    stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = (HI_HANDLE*)&(g_stSTATEMNGContext.RawCapTaskHdl);

                    g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                    g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_FALSE;
                }

                HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                enSceneType = HI_PDT_PARAM_SCENE_TYPE_HDR;
                break;
            }
            case HI_PDT_PHOTO_SCENE_LL:
            {
                enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NIGHTMODE;

                if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0 &&
                    g_stSTATEMNGContext.stPhotoProTune.u32ISO == 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:%d UI ISO:AUTO\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32SysGain =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpGain(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us, &u32SysGain);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        u32SysGain;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;

#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                    (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us == 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32Time =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpTime(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO, &u32Time);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        u32Time;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:%d, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
#ifdef CONFIG_RAWCAP_ON
                    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                    {
                        stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                        stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                        stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                    }

                    HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                    enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                }
                else
                {
                    s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                        &stLongExpParam);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                    if (stLongExpParam.bLongExp && HI_PDT_WORKMODE_LPSE_PHOTO != g_stSTATEMNGContext.enCurrentWorkMode)
                    {
                        MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, bLongExp(1)\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;

                        pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                            stLongExpParam.stAeRoute.u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                            stLongExpParam.stAeRoute.u32SysGain;

                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;

#ifdef CONFIG_RAWCAP_ON
                        if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                        {
                            stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                            stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                            stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                            stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                            g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                            g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                        }

                        HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                        enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                    }
                    else
                    {
                        MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, bLongExp(0)\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;

 #ifdef CONFIG_RAWCAP_ON
                        if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
                        (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
                        {
                            stPhotoDumpCBFunc.stBNRRawCB.pfunVProcBNRProc = NULL;
                            stPhotoDumpCBFunc.stBNRRawCB.pPrivateData = NULL;
                            stPhotoDumpCBFunc.stYUVDataCB.pfunVProcYUVProc = NULL;
                            stPhotoDumpCBFunc.stYUVDataCB.pPrivateData = NULL;

                            g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_TRUE;
                            g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
                        }

                        HI_MAPI_VPROC_RegPhotoDumpCB(VprocHdl, VportHdl, &stPhotoDumpCBFunc);
#endif
                        enSceneType = HI_PDT_PARAM_SCENE_TYPE_LINEAR;
                    }
                }
                break;
            }
            default:
                MLOGE("scene type err\n");
                break;
        }

        HI_PDT_PARAM_SCENE_CFG_S stSceneCfg;
        HI_PDT_PARAM_GetSceneCfg(g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
            enSceneType, &stSceneCfg);

        HI_S32 s32Index;
        s32Ret = PDT_STATEMNG_GetVcapPipeHdlIndex(VcapPipeHdl, &s32Index);
        PDT_STATEMNG_CHECK_RET(s32Ret, "get vcap pipe handle index");
        u8PipeParamIndex = stSceneCfg.au8SceneParamIdx[s32Index];
        MLOGI(YELLOW"s32Index[%d], u8PipeParamIndex[%d]\n"NONE, s32Index, u8PipeParamIndex);

        s32Ret = HI_PDT_SCENE_RefreshPhotoPipe(VcapPipeHdl, u8PipeParamIndex);
        PDT_STATEMNG_CHECK_RET(s32Ret,"refresh photo pipe");
    }
    else
    {
        MLOGI(YELLOW"other photo mode\n"NONE);
        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
        pstPhotoParam->stSnapAttr.bLoadCCM = HI_FALSE;
        pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
        pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
    }

#ifdef CONFIG_RAWCAP_ON
    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
       (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
    {
        if(SNAP_TYPE_NORMAL == pstPhotoParam->stSnapAttr.enSnapType)
        {
            g_stSTATEMNGContext.u32RawCapFrameCnt =
                pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt;
        }
        else
        {
            g_stSTATEMNGContext.u32RawCapFrameCnt =
                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt;
        }
    }
#endif

    return HI_SUCCESS;
}
#else
/** get PhotoMng param */
static HI_S32 PDT_STATEMNG_GetPhotoParam(HI_PHOTOMNG_PHOTO_PARAM_S* pstPhotoParam,
    HI_VOID* pvPrivateData)
{
    PDT_STATEMNG_CHECK_NULL_PTR(pstPhotoParam,"pstPhotoParam");
    memset(pstPhotoParam, 0, sizeof(HI_PHOTOMNG_PHOTO_PARAM_S));

    /*set exif info*/
    snprintf((HI_CHAR*)pstPhotoParam->stExifInfo.au8ImageDescription,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"Photo test");
    snprintf((HI_CHAR*)pstPhotoParam->stExifInfo.au8Make,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"Hisilicon");
    snprintf((HI_CHAR*)pstPhotoParam->stExifInfo.au8Model,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"DCIM");
    snprintf((HI_CHAR*)pstPhotoParam->stExifInfo.au8Software,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"HiMobileCam");


    pstPhotoParam->stExifInfo.stApertureInfo.u32MinIrisFNOTarget = 1;
    pstPhotoParam->stExifInfo.stApertureInfo.u32MaxIrisFNOTarget = 1024;
    pstPhotoParam->stExifInfo.stGPSInfo.chGPSLatitudeRef = 'N';
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLatitude[0][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLatitude[1][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLatitude[2][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.chGPSLongitudeRef = 'E';
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLongitude[0][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLongitude[1][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSLongitude[2][1]=1;
    pstPhotoParam->stExifInfo.stGPSInfo.au32GPSAltitude[1]=1;
    PDT_STATEMNG_GetMeteringType(&(pstPhotoParam->stExifInfo.u32MeteringMode));

    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE VcapPipeHdl =
        g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.stPhotoSRC.VcapPipeHdl;
    HI_PDT_SCENE_LONGEXP_PARAM_S  stLongExpParam;
    HI_PDT_SCENE_SNAP_LONGEXP_TYPE_E enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_BUTT;

    if ((HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_LPSE_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        switch(g_stSTATEMNGContext.enScene)
        {
            case HI_PDT_PHOTO_SCENE_NORM:
            {
                /** get exptime */
                if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO == 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:%d, UI ISO:AUTO\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32SysGain =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpGain(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us, &u32SysGain);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        u32SysGain;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us == 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32Time =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpTime(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO, &u32Time);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        u32Time;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:Normal, UI ExpTime:%d, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else
                {
                    enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NORMAL;
                    s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                        &stLongExpParam);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                    if(stLongExpParam.bLongExp && HI_PDT_WORKMODE_LPSE_PHOTO != g_stSTATEMNGContext.enCurrentWorkMode)
                    {
                        MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(1), SNAPALG OFF\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                            OPERATION_MODE_MANUAL;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                            stLongExpParam.stAeRoute.u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                            stLongExpParam.stAeRoute.u32SysGain;
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                    }
                    else
                    {
                        MLOGI(YELLOW"Scene:Normal, UI ExpTime:AUTO, bLongExp(0), SNAPALG OFF\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                    }
                }
                break;
            }
            case HI_PDT_PHOTO_SCENE_LL:
            {
                enLongExpType = HI_PDT_SCENE_SNAP_LONGEXP_TYPE_NIGHTMODE;

                if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO == 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:%d, UI ISO:AUTO\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32SysGain =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpGain(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us, &u32SysGain);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        u32SysGain;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us == 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    HI_U32 u32Time =  0;
                    HI_HANDLE VcapVideoPipeHdl;
                    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapVideoPipeHdl);
                    if(HI_SUCCESS != s32Ret)
                    {
                        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapVideoPipeHdl);
                        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
                    }

                    HI_PDT_SCENE_GetLongExpTime(VcapVideoPipeHdl,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO, &u32Time);
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        u32Time;
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us != 0
                    && g_stSTATEMNGContext.stPhotoProTune.u32ISO != 0)
                {
                    MLOGI(YELLOW"Scene:LL, UI ExpTime:%d, UI ISO:%d\n"NONE,
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us,
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO);

                    pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                    pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                    pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                        OPERATION_MODE_MANUAL;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ISO / 100 * 1024;
                    pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us;

                    /** call HI_PDT_SCENE_GetLongExpGain with au32ManExpTime to get Gain */
                    pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                }
                else
                {
                    s32Ret = HI_PDT_SCENE_GetDetectLongExpInfo(VcapPipeHdl, enLongExpType,
                        &stLongExpParam);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"get LongExpInfo");

                    if (stLongExpParam.bLongExp && HI_PDT_WORKMODE_LPSE_PHOTO != g_stSTATEMNGContext.enCurrentWorkMode)
                    {
                        MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, bLongExp(1)\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_PRO;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.enOperationMode =
                            OPERATION_MODE_MANUAL;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManExpTime[0] =
                            stLongExpParam.stAeRoute.u32ExpTime;
                        pstPhotoParam->stSnapAttr.stProAttr.stProParam.stManualParam.au32ManSysgain[0] =
                            stLongExpParam.stAeRoute.u32SysGain;
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                    }
                    else
                    {
                        MLOGI(YELLOW"Scene:LL, UI ExpTime:AUTO, bLongExp(0)\n"NONE);
                        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
                        pstPhotoParam->stSnapAttr.bLoadCCM = HI_TRUE;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
                        pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
                        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
                    }
                }
                break;
            }
            default:
                MLOGE("error scene type\n\n");
                break;
        }

        HI_U8 u8PipeParamIndex = 0;

        HI_PDT_PARAM_SCENE_CFG_S stSceneCfg;
        HI_PDT_PARAM_GetSceneCfg(g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode,
            HI_PDT_PARAM_SCENE_TYPE_LINEAR, &stSceneCfg);

        HI_S32 s32Index;
        s32Ret = PDT_STATEMNG_GetVcapPipeHdlIndex(VcapPipeHdl, &s32Index);
        PDT_STATEMNG_CHECK_RET(s32Ret, "get vcap pipe handle index");
        u8PipeParamIndex = stSceneCfg.au8SceneParamIdx[s32Index];
        MLOGI(YELLOW"s32Index[%d], u8PipeParamIndex[%d]\n"NONE, s32Index, u8PipeParamIndex);

        s32Ret = HI_PDT_SCENE_RefreshPhotoPipe(VcapPipeHdl, u8PipeParamIndex);
        PDT_STATEMNG_CHECK_RET(s32Ret,"refresh photo pipe");
    }
    else
    {
        MLOGD(YELLOW"other photo mode\n\n"NONE);
        pstPhotoParam->enPhotoType = VPROC_PHOTO_TYPE_BUTT;
        pstPhotoParam->stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
        pstPhotoParam->stSnapAttr.bLoadCCM = HI_FALSE;
        pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt = 1;
        pstPhotoParam->stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
    }

#ifdef CONFIG_RAWCAP_ON
    if((HI_TRUE == g_stSTATEMNGContext.bRawCapOn) &&
       (HI_PHOTOMNG_OUTPUT_FORMAT_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
    {
        if(SNAP_TYPE_NORMAL == pstPhotoParam->stSnapAttr.enSnapType)
        {
            g_stSTATEMNGContext.u32RawCapFrameCnt =
                pstPhotoParam->stSnapAttr.stNormalAttr.u32FrameCnt;
        }
        else
        {
            g_stSTATEMNGContext.u32RawCapFrameCnt =
                pstPhotoParam->stSnapAttr.stProAttr.u32FrameCnt;
        }
        g_stSTATEMNGContext.stTriggerMode.bDumpBNR = HI_FALSE;
        g_stSTATEMNGContext.stTriggerMode.bDumpYUV = HI_TRUE;
    }
#endif

    return HI_SUCCESS;
}
#endif

HI_S32 PDT_STATEMNG_UpgradeSrchNewPkt(HI_VOID)
{
    HI_S32 s32Ret;
    HI_UPGRADE_DEV_INFO_S stDevInfo = {};
    HI_STORAGEMNG_CFG_S stStorageCfg = {};

    s32Ret = HI_UPGRADE_Init();
    if(HI_UPGRADE_EINITIALIZED == s32Ret)
    {
        MLOGD(YELLOW"upgrade already inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"init upgrade");
    }

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_DEV_INFO, &stDevInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret,"GetCommParam");

    s32Ret = HI_PDT_PARAM_GetStorageCfg(&stStorageCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret,"GetStorageCfg");

    s32Ret = HI_UPGRADE_SrchNewPkt(stStorageCfg.szMntPath, &stDevInfo);
    if(HI_UPGRADE_PKT_AVAILABLE != s32Ret)
    {
        MLOGD(YELLOW"HI_UPGRADE_SrchNewPkt no pkt\n"NONE);
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_FileMngProc(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_FILEMNG_SetDiskState(HI_TRUE);
    if(HI_FILEMNG_EINTER == s32Ret)
    {
        if(HI_TRUE == g_stSTATEMNGContext.bPoweronAction)
        {
            g_stSTATEMNGContext.bPoweronAction = HI_FALSE;
        }

        s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SD_UNAVAILABLE,
                0, 0, HI_FALSE, 0L, NULL, 0);
        PDT_STATEMNG_CHECK_RET(s32Ret,"publish sd unavailable event");
        MLOGD(YELLOW"publish sd unavailable(0x%08X)\n"NONE, HI_EVENT_STATEMNG_SD_UNAVAILABLE);

        return HI_PROCESS_MSG_RESULTE_OK;
    } else {
        if (HI_SUCCESS == s32Ret) {
            s32Ret = HI_FILEMNG_CheckDiskSpace();
            if (HI_FILEMNG_EFULL == s32Ret) {
                if (HI_SUCCESS != HI_FILEMNG_Repair()) {
                    MLOGE(RED"filemng repair failed\n"NONE);
                }
            }
            PDT_STATEMNG_CHECK_RET(s32Ret, "check disk space err");
        } else {
            MLOGD(YELLOW"filemng set disk state(HI_TRUE) failed\n"NONE);
            return HI_FAILURE;
        }
    }

    /** file repair */
    s32Ret = HI_FILEMNG_Repair();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"filemng repair failed(0x%08X)\n"NONE, s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_DoNotUpgrade(HI_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = PDT_STATEMNG_FileMngProc();
    PDT_STATEMNG_CHECK_RET(s32Ret,"PDT_STATEMNG_FileMngProc");

    s32Ret = PDT_STATEMNG_PublishEvent(HI_EVENT_STATEMNG_SD_AVAILABLE,
                0, 0, HI_FALSE, 0L, NULL, 0);
    PDT_STATEMNG_CHECK_RET(s32Ret,"publish sd available event");

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    PDT_STATEMNG_PublishResultEvent(pstMsg, HI_TRUE);
    g_stSTATEMNGContext.bInProgress = HI_FALSE;
    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);

    return HI_SUCCESS;
}

static HI_VOID PDT_STATEMNG_PrintPhotoAttr(HI_PDT_WORKMODE_E enEnterWorkMode,
                                           const HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    if(NULL == pstPhotoAttr)
        return;

    MLOGD("Photo attr:\n");
    MLOGD(" enPhotoType(%d)\n", pstPhotoAttr->enPhotoType);
    MLOGD(" VcapPipeHdl(%d),VcapPipeChnHdl(%d),VprocHdl(%d),"
          " VportHdl(%d),VencHdl(%d),VencThmHdl(%d),enOutPutFmt(%d)\n",
          pstPhotoAttr->stPhotoSRC.VcapPipeHdl, pstPhotoAttr->stPhotoSRC.VcapPipeChnHdl,
          pstPhotoAttr->stPhotoSRC.VprocHdl, pstPhotoAttr->stPhotoSRC.VportHdl,
          pstPhotoAttr->stPhotoSRC.VencHdl, pstPhotoAttr->stPhotoSRC.VencThmHdl,
          pstPhotoAttr->enOutPutFmt);

    if((HI_PDT_WORKMODE_SING_PHOTO == enEnterWorkMode) ||
       (HI_PDT_WORKMODE_DLAY_PHOTO == enEnterWorkMode))
    {
        MLOGD(" stSingleAttr.s32TimeSec(%d)\n",
            pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.s32Time_sec);
    }

    if((HI_PDT_WORKMODE_LPSE_PHOTO == enEnterWorkMode) ||
       (HI_PDT_WORKMODE_RECSNAP == enEnterWorkMode))
    {
        MLOGD(" stLapseAttr.s32IntervalMilliSec(%d)\n",
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms);
        MLOGD(" stLapseAttr.bNeedTrigger(%d)\n",
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.bNeedTrigger);
    }
}

static HI_BOOL PDT_STATEMNG_IsVideoPipe(HI_PDT_WORKMODE_E enSettingWorkmode)
{
    if((HI_PDT_WORKMODE_NORM_REC == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_LOOP_REC == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_LPSE_REC == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_SLOW_REC == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_RECSNAP == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_BURST == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_HDMI_PREVIEW == enSettingWorkmode))
    {
        return HI_TRUE;
    }
    else
    {
        return HI_FALSE;
    }
}

static HI_BOOL PDT_STATEMNG_IsSnapPipe(HI_PDT_WORKMODE_E enSettingWorkmode)
{
    if((HI_PDT_WORKMODE_SING_PHOTO == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_DLAY_PHOTO == enSettingWorkmode) ||
       (HI_PDT_WORKMODE_LPSE_PHOTO == enSettingWorkmode))
    {
        return HI_TRUE;
    }
    else
    {
        return HI_FALSE;
    }

}

HI_S32 PDT_STATEMNG_GetVideoPipeHdl(HI_HANDLE* pVcapPipeHdl)
{
    HI_S32 i = 0, j = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr =
            &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i]);

        if(!pstVcapDevAttr->bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr =
                &(pstVcapDevAttr->astVcapPipeAttr[j]);

            if(!pstVcapPipeAttr->bEnable)
            {
                continue;
            }

            if(HI_MAPI_PIPE_TYPE_VIDEO == pstVcapPipeAttr->enPipeType)
            {
                *pVcapPipeHdl = pstVcapPipeAttr->VcapPipeHdl;
                return HI_SUCCESS;
            }
        }
    }

    return HI_FAILURE;
}

HI_S32 PDT_STATEMNG_GetSnapPipeHdl(HI_HANDLE* pVcapPipeHdl)
{
    HI_S32 i = 0, j = 0;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        HI_PDT_MEDIA_VCAP_DEV_ATTR_S* pstVcapDevAttr =
            &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[i]);

        if(!pstVcapDevAttr->bEnable)
        {
            continue;
        }

        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr =
                &(pstVcapDevAttr->astVcapPipeAttr[j]);

            if(!pstVcapPipeAttr->bEnable)
            {
                continue;
            }

            if(HI_MAPI_PIPE_TYPE_SNAP == pstVcapPipeAttr->enPipeType)
            {
                *pVcapPipeHdl = pstVcapPipeAttr->VcapPipeHdl;
                return HI_SUCCESS;
            }
        }
    }

    return HI_FAILURE;
}

/** change workmode to state */
HI_S32 PDT_STATEMNG_WorkMode2State(HI_PDT_WORKMODE_E enPoweronWorkmode, HI_CHAR *pszStateName, HI_S32 s32NameLength)
{
    /** check parameter */
    if(NULL == pszStateName || s32NameLength < HI_STATE_NAME_LEN)
    {
        MLOGE(RED"param err\n"NONE);
        return HI_STORAGEMNG_EINVAL;
    }

    switch(enPoweronWorkmode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_REC_NORMAL,sizeof(PDT_STATEMNG_STATE_REC_NORMAL));
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_REC_LOOP,sizeof(PDT_STATEMNG_STATE_REC_LOOP));
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_REC_LAPSE,sizeof(PDT_STATEMNG_STATE_REC_LAPSE));
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_REC_SLOW,sizeof(PDT_STATEMNG_STATE_REC_SLOW));
            break;
        case HI_PDT_WORKMODE_SING_PHOTO:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_PHOTO_SINGLE,sizeof(PDT_STATEMNG_STATE_PHOTO_SINGLE));
            break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_PHOTO_DELAY,sizeof(PDT_STATEMNG_STATE_PHOTO_DELAY));
            break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE,sizeof(PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE));
            break;
        case HI_PDT_WORKMODE_BURST:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_MUTIPHOTO_BURST,sizeof(PDT_STATEMNG_STATE_MUTIPHOTO_BURST));
            break;
        case HI_PDT_WORKMODE_RECSNAP:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_REC_SNAP,sizeof(PDT_STATEMNG_STATE_REC_SNAP));
            break;
        case HI_PDT_WORKMODE_PLAYBACK:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_PLAYBACK,sizeof(PDT_STATEMNG_STATE_PLAYBACK));
            break;
        case HI_PDT_WORKMODE_UVC:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_UVC,sizeof(PDT_STATEMNG_STATE_UVC));
            break;
        case HI_PDT_WORKMODE_USB_STORAGE:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_USB_STORAGE,sizeof(PDT_STATEMNG_STATE_USB_STORAGE));
            break;
        case HI_PDT_WORKMODE_SUSPEND:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_SUSPEND,sizeof(PDT_STATEMNG_STATE_SUSPEND));
            break;
        case HI_PDT_WORKMODE_HDMI_PREVIEW:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_HDMI_PREVIEW,sizeof(PDT_STATEMNG_STATE_HDMI_PREVIEW));
            break;
        case HI_PDT_WORKMODE_HDMI_PLAYBACK:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_HDMI_PLAYBACK,sizeof(PDT_STATEMNG_STATE_HDMI_PLAYBACK));
            break;
        case HI_PDT_WORKMODE_UPGRADE:
            strncpy(pszStateName,
                PDT_STATEMNG_STATE_UPGRADE,sizeof(PDT_STATEMNG_STATE_UPGRADE));
            break;
        default:
        {

            MLOGE(RED"param err\n"NONE);
            return HI_FAILURE;
            break;
        }
    }

    //MLOGD("change enPoweronWorkmode(%d) to pszStateName(%s)\n\n", enPoweronWorkmode, pszStateName);
    return HI_SUCCESS;
}

/** change state to workmode */
HI_S32 PDT_STATEMNG_State2WorkMode(const HI_CHAR *pszStateName, HI_PDT_WORKMODE_E *penPoweronWorkmode)
{
    /** check parameter */
    PDT_STATEMNG_CHECK_NULL_PTR(pszStateName,"pszStateName");

    if(!(strncmp(pszStateName,
            PDT_STATEMNG_STATE_REC_NORMAL, sizeof(PDT_STATEMNG_STATE_REC_NORMAL))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_NORM_REC;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_REC_LOOP, sizeof(PDT_STATEMNG_STATE_REC_LOOP))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_LOOP_REC;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_REC_LAPSE, sizeof(PDT_STATEMNG_STATE_REC_LAPSE))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_LPSE_REC;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_REC_SLOW, sizeof(PDT_STATEMNG_STATE_REC_SLOW))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_SLOW_REC;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_PHOTO_SINGLE, sizeof(PDT_STATEMNG_STATE_PHOTO_SINGLE))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_SING_PHOTO;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_PHOTO_DELAY, sizeof(PDT_STATEMNG_STATE_PHOTO_DELAY))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_DLAY_PHOTO;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE, sizeof(PDT_STATEMNG_STATE_MUTIPHOTO_LAPSE))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_LPSE_PHOTO;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_MUTIPHOTO_BURST, sizeof(PDT_STATEMNG_STATE_MUTIPHOTO_BURST))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_BURST;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_REC_SNAP, sizeof(PDT_STATEMNG_STATE_REC_SNAP))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_RECSNAP;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_PLAYBACK, sizeof(PDT_STATEMNG_STATE_PLAYBACK))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_PLAYBACK;
    }
    else if(!(strncmp(pszStateName, PDT_STATEMNG_STATE_UVC, sizeof(PDT_STATEMNG_STATE_UVC))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_UVC;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_USB_STORAGE, sizeof(PDT_STATEMNG_STATE_USB_STORAGE))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_USB_STORAGE;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_SUSPEND, sizeof(PDT_STATEMNG_STATE_SUSPEND))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_SUSPEND;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_HDMI_PREVIEW, sizeof(PDT_STATEMNG_STATE_HDMI_PREVIEW))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_HDMI_PREVIEW;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_HDMI_PLAYBACK, sizeof(PDT_STATEMNG_STATE_HDMI_PLAYBACK))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_HDMI_PLAYBACK;
    }
    else if(!(strncmp(pszStateName,
                PDT_STATEMNG_STATE_UPGRADE, sizeof(PDT_STATEMNG_STATE_UPGRADE))))
    {
        *penPoweronWorkmode = HI_PDT_WORKMODE_UPGRADE;
    }
    else
    {
        MLOGE(RED"param err\n"NONE);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/** publish message process result event */
HI_S32 PDT_STATEMNG_PublishResultEvent(HI_MESSAGE_S *pstMsg, HI_BOOL bResult)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstMsg,"pstMsg");

    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(HI_EVENT_S));
    stEvent.EventID = pstMsg->what;
    stEvent.arg1 = pstMsg->arg1;
    stEvent.arg2 = pstMsg->arg2;
    stEvent.s32Result = bResult;
    memcpy_s(&stEvent.aszPayload, EVENT_PAYLOAD_LEN, pstMsg->aszPayload, MESSAGE_PAYLOAD_LEN);

    s32Ret = HI_EVTHUB_Publish(&stEvent);
    PDT_STATEMNG_CHECK_RET(s32Ret,"publish event");
    MLOGD(YELLOW"publish event(0x%08X), bResult(%d) succeed\n"NONE, stEvent.EventID, bResult);

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

/** publish message process result event */
HI_S32 PDT_STATEMNG_PublishEvent(HI_EVENT_ID EventID, HI_S32 arg1, HI_S32 arg2, HI_BOOL bResult, HI_U64 u64CreateTime, HI_CHAR *szPayload, HI_S32 s32PayloadLen)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(HI_EVENT_S));
    stEvent.EventID = EventID;
    stEvent.arg1 = arg1;
    stEvent.arg2 = arg2;
    stEvent.s32Result = bResult;
    if((NULL != szPayload) &&
       (MESSAGE_PAYLOAD_LEN >= s32PayloadLen) &&
       (EVENT_PAYLOAD_LEN >= s32PayloadLen))
    {
        memcpy_s(&stEvent.aszPayload, EVENT_PAYLOAD_LEN, szPayload, s32PayloadLen);
    }

    s32Ret = HI_EVTHUB_Publish(&stEvent);
    PDT_STATEMNG_CHECK_RET(s32Ret,"publish event");
    MLOGD(YELLOW"publish event(0x%08X), bResult(%d) succeed\n"NONE, stEvent.EventID, bResult);

    HI_TIME_STAMP;
    HI_TIME_PRINT;
    HI_TIME_CLEAN;
    return s32Ret;
}

HI_S32 PDT_STATEMNG_GenerateMediaCfg(HI_PDT_MEDIA_CFG_S *pstInMediaModeCfg,
    HI_PDT_MEDIA_CFG_S *pstOutMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** check parameter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstOutMediaCfg,"pstOutMediaCfg");
    PDT_STATEMNG_CHECK_NULL_PTR(pstInMediaModeCfg,"pstInMediaModeCfg");

    /** generate parameter for Media module, fill g_stMediaCfg struct */
    memcpy(pstOutMediaCfg, pstInMediaModeCfg, sizeof(HI_PDT_MEDIA_CFG_S));
    MLOGD(YELLOW"stTimeOsd.bShow(%d)\n"NONE, pstOutMediaCfg->stVideoCfg.stOsdCfg.stTimeOsd.bShow);

    /** special for UVC */
    if(HI_PDT_WORKMODE_UVC == g_stSTATEMNGContext.enCurrentWorkMode)
    {
        /** disable venc */
        HI_S32 i = 0;
        for(i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
        {
            HI_PDT_MEDIA_VENC_CFG_S *pstVencCfg = &(pstOutMediaCfg->stVideoCfg.astVencCfg[i]);
            pstVencCfg->bEnable = HI_FALSE;
        }

        /** disable OSD */
        pstOutMediaCfg->stVideoCfg.stOsdCfg.stTimeOsd.bShow = HI_FALSE;
    }

    /** special for BURST */
    if(HI_PDT_WORKMODE_BURST == g_stSTATEMNGContext.enCurrentWorkMode)
    {
        HI_U32 u32FrameRate = 0;
        HI_PDT_JPG_BURST_TYPE_E enBurstType =  HI_PDT_JPG_BURST_TYPE_BUTT;
        s32Ret = HI_PDT_PARAM_GetBurstType(g_stSTATEMNGContext.stPhotoMngCtx.enEnterMediaMode, &enBurstType);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get burst type");

        switch(enBurstType)
        {
            case HI_PDT_JPG_BURST_TYPE_3_1:
                u32FrameRate = 3;
                break;
            case HI_PDT_JPG_BURST_TYPE_5_1:
                u32FrameRate = 5;
                break;
            case HI_PDT_JPG_BURST_TYPE_10_1:
                u32FrameRate = 10;
                break;
            case HI_PDT_JPG_BURST_TYPE_15_1:
                u32FrameRate = 15;
                break;
            case HI_PDT_JPG_BURST_TYPE_20_2:
                u32FrameRate = 10;
                break;
            case HI_PDT_JPG_BURST_TYPE_30_1:
                u32FrameRate = 30;
                break;
            case HI_PDT_JPG_BURST_TYPE_30_2:
                u32FrameRate = 15;
                break;
            case HI_PDT_JPG_BURST_TYPE_30_3:
                u32FrameRate = 10;
                break;
            default:
            {
                MLOGE(RED"param err\n"NONE);
                return HI_FAILURE;
            }
        }

        /** find VpssHdl and VPortHdl */
        HI_HANDLE VencHdl = PDT_STATEMNG_PHOTO_VENC_HDL;
        HI_HANDLE ModHdl = 0;
        HI_HANDLE ChnHdl = 0;
        HI_S32 s32Idx;
        HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
        for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
        {
            pstVencCfg = &(pstOutMediaCfg->stVideoCfg.astVencCfg[s32Idx]);
            if((HI_TRUE == pstVencCfg->bEnable) &&
               (pstVencCfg->VencHdl == VencHdl))
            {
                MLOGD("find venc handle(%d)\n", VencHdl);
                ModHdl =  pstVencCfg->ModHdl;
                ChnHdl = pstVencCfg->ChnHdl;
                break;
            }
        }

        /** change vprot frame rate */
        if(HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)
        {
            HI_S32 i = 0, j = 0;
            HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
            HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVportAttr = NULL;
            for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
            {
                pstVpssAttr =
                    &(pstOutMediaCfg->stVideoCfg.stVprocCfg.astVpssAttr[i]);
                if((HI_TRUE == pstVpssAttr->bEnable) &&
                   (pstVpssAttr->VpssHdl == ModHdl))
                {
                    MLOGD("find vpss handle(%d)\n", pstVpssAttr->VpssHdl);
                    for(j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
                    {
                        pstVportAttr =
                            &(pstVpssAttr->astVportAttr[j]);
                        if((HI_TRUE == pstVportAttr->bEnable) &&
                           (pstVportAttr->VportHdl == ChnHdl))
                        {
                            MLOGD("orig vport dst frame rate(%d fps)\n", pstVportAttr->stFrameRate.s32DstFrameRate);
                            pstVportAttr->stFrameRate.s32DstFrameRate = u32FrameRate;
                            MLOGD("update vport dst frame rate(%d fps)\n", pstVportAttr->stFrameRate.s32DstFrameRate);
                            break;
                        }
                    }
                }
            }
        }
        else if(HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)/** change vcap pipe chanel frame rate */
        {
            HI_PDT_MEDIA_VCAP_CFG_S*  pstVcapCfg =
                &(pstOutMediaCfg->stVideoCfg.stVcapCfg);

            HI_PDT_MEDIA_VCAP_DEV_ATTR_S*  pstVcapDevAttr = NULL;
            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = NULL;
            HI_PDT_MEDIA_VCAP_PIPE_CHN_ATTR_S* pstPipeChnAttr = NULL;
            HI_S32 i = 0, j = 0, k = 0;
            for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
            {
                pstVcapDevAttr = &(pstVcapCfg->astVcapDevAttr[i]);

                if(HI_TRUE == pstVcapDevAttr->bEnable)
                {
                    for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
                    {
                        pstVcapPipeAttr = &(pstVcapDevAttr->astVcapPipeAttr[j]);

                        if((HI_TRUE == pstVcapPipeAttr->bEnable) &&
                           (pstVcapPipeAttr->VcapPipeHdl == ModHdl))
                        {
                            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
                            {
                                pstPipeChnAttr = &(pstVcapPipeAttr->astPipeChnAttr[k]);
                                if((HI_TRUE == pstPipeChnAttr->bEnable) &&
                                   (pstPipeChnAttr->PipeChnHdl == ChnHdl))
                                {
                                    MLOGD("orig vcap pipe chanel dst frame rate(%d fps)\n",
                                        pstPipeChnAttr->stFrameRate.s32DstFrameRate);
                                    pstPipeChnAttr->stFrameRate.s32DstFrameRate = u32FrameRate;
                                    MLOGD("update vcap pipe chanel dst frame rate(%d fps)\n",
                                        pstPipeChnAttr->stFrameRate.s32DstFrameRate);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // s32Ret = HI_PDT_Media_UpdateDispCfg(pstOutMediaCfg, &pstOutMediaCfg->stVideoOutCfg.astDispCfg[0]);
    // HI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, HI_FAILURE, "update disp cfg");

    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_VOLUME, &pstOutMediaCfg->stAudioOutCfg.astAoCfg[0].s32Volume);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    /** special for ACap & Aenc */
    if((HI_PDT_WORKMODE_NORM_REC != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_LOOP_REC != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_RECSNAP != g_stSTATEMNGContext.enCurrentWorkMode))
    {
        HI_S32 i = 0;
        for (i = 0; i < HI_PDT_MEDIA_ACAP_MAX_CNT; i++)
        {
            pstOutMediaCfg->stAudioCfg.astAcapCfg[i].bEnable = HI_FALSE;
        }

        for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
        {
            pstOutMediaCfg->stAudioCfg.astAencCfg[i].bEnable = HI_FALSE;
        }
    }

#if 0
    /** disable audio for debug, come with record atrr change */
    pstOutMediaCfg->stAudioCfg.astAcapCfg[0].bEnable = 0;
    pstOutMediaCfg->stAudioCfg.astAencCfg[0].bEnable = 0;
    pstOutMediaCfg->stAudioOutCfg.astAoCfg[0].bEnable = 0;
#endif

    return s32Ret;
}

HI_S32 PDT_STATEMNG_StopEncoder(const HI_PDT_MEDIA_CFG_S* pstMediaCfg, HI_PDT_WORKMODE_E enWorkMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i = 0;
    HI_S32 j = 0;

    HI_PDT_WORKMODE_CFG_S stWorkModeCfg;
    s32Ret = HI_PDT_PARAM_GetWorkModeCfg(enWorkMode, &stWorkModeCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    HI_PDT_REC_SRC_S* pstRecDataSrc = NULL;
    switch (enWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stNormRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_LOOP_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stLoopRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_LPSE_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stLapseRecAttr.stDataSrc);
            break;
        case HI_PDT_WORKMODE_SLOW_REC:
            pstRecDataSrc = &(stWorkModeCfg.unModeAttr.stSlowRecAttr.stDataSrc);
            break;
        default:
        {
            MLOGD(YELLOW"param err\n"NONE);
            return HI_FAILURE;
        }
    }

    /**< stop venc */
    for (j = 0; j < HI_PDT_REC_VSTREAM_MAX_CNT; j++)
    {

        for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; i++)
        {
            if ((pstRecDataSrc->aVencHdl[j] == pstMediaCfg->stVideoCfg.astVencCfg[i].VencHdl) &&
                (HI_TRUE == pstMediaCfg->stVideoCfg.astVencCfg[i].bEnable))
            {
                MLOGD("start VencHdl(%d)\n", pstRecDataSrc->aVencHdl[j]);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VENC_Start(pstRecDataSrc->aVencHdl[j], -1);
                HI_PERFORMANCE_TIME_STAMP;
                if (HI_SUCCESS != s32Ret)
                {
                    MLOGE("start VencHdl(%d) failed(0x%08X)\n", pstRecDataSrc->aVencHdl[j], s32Ret);
                    return HI_FAILURE;
                }

                MLOGD("stop VencHdl(%d)\n", pstRecDataSrc->aVencHdl[j]);
                HI_PERFORMANCE_TIME_STAMP;
                s32Ret = HI_MAPI_VENC_Stop(pstRecDataSrc->aVencHdl[j]);
                HI_PERFORMANCE_TIME_STAMP;
                if (HI_SUCCESS != s32Ret)
                {
                    MLOGE("stop VencHdl(%d) failed(0x%08X)\n", pstRecDataSrc->aVencHdl[j], s32Ret);
                    return HI_FAILURE;
                }
                break;
            }
        }
    }

    /**< stop aenc  */
    for (i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
    {
        if ((pstRecDataSrc->AencHdl == pstMediaCfg->stAudioCfg.astAencCfg[i].AencHdl) &&
            (HI_TRUE == pstMediaCfg->stAudioCfg.astAencCfg[i].bEnable))
        {
            MLOGD("start AencHdl(%d)\n", pstRecDataSrc->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_Start(pstRecDataSrc->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("start AencHdl(%d) failed(0x%08X)\n", pstRecDataSrc->AencHdl, s32Ret);
                return HI_FAILURE;
            }

            MLOGD("stop AencHdl(%d)\n", pstRecDataSrc->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_Stop(pstRecDataSrc->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("stop AencHdl(%d) failed(0x%08X)\n", pstRecDataSrc->AencHdl, s32Ret);
                return HI_FAILURE;
            }

            MLOGD("unbind ACapHdl(%d)\n", pstMediaCfg->stAudioCfg.astAencCfg[i].AcapHdl);
            HI_PERFORMANCE_TIME_STAMP;
            s32Ret = HI_MAPI_AENC_UnbindACap(pstMediaCfg->stAudioCfg.astAencCfg[i].AcapHdl,
                pstMediaCfg->stAudioCfg.astAencCfg[i].AcapChnHdl, pstRecDataSrc->AencHdl);
            HI_PERFORMANCE_TIME_STAMP;
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE("unbind ACap failed(0x%08X)\n", s32Ret);
                return HI_FAILURE;
            }
        }
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_UpdateViVpssMode(const HI_PDT_MEDIA_CFG_S *pstSrcMediaCfg,
    HI_PDT_MEDIA_CFG_S *pstDstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bViOffline = HI_FALSE;
    HI_S32 i, j = 0;
    memcpy(pstDstMediaCfg, pstSrcMediaCfg, sizeof(HI_PDT_MEDIA_CFG_S));

#ifdef CONFIG_RAWCAP_ON
    if((HI_PDT_WORKMODE_NORM_REC == g_stSTATEMNGContext.enCurrentWorkMode)  &&
       (HI_TRUE == g_stSTATEMNGContext.bRawCapOn))
    {
        bViOffline = HI_TRUE;
    }
#endif

    if (bViOffline)
    {
        for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
        {
            if (!pstSrcMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
            {
                continue;
            }

            for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
            {
                if (!pstSrcMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].astVcapPipeAttr[j].bEnable)
                {
                    continue;
                }

                switch (pstSrcMediaCfg->stViVpssMode.astMode[i][j].enMode)
                {
                    case VI_ONLINE_VPSS_OFFLINE:
                        pstDstMediaCfg->stViVpssMode.astMode[i][j].enMode = VI_OFFLINE_VPSS_OFFLINE;
                        break;
                    case VI_ONLINE_VPSS_ONLINE:
                        pstDstMediaCfg->stViVpssMode.astMode[i][j].enMode = VI_OFFLINE_VPSS_ONLINE;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return s32Ret;
}


HI_S32 PDT_STATEMNG_ResetMedia(const HI_PDT_MEDIA_CFG_S *pstMediaCfg,
    HI_PDT_SCENE_MODE_S* pstSceneCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_CFG_S stMediaCfg;
    /*set exif info*/
    HI_MAPI_SNAP_EXIF_INFO_S* pstExifInfo =
        (HI_MAPI_SNAP_EXIF_INFO_S*)&pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[0].stExifInfo;
    PDT_STATEMNG_GetCamExifInfo(pstExifInfo);

    /** Video out stop */  //Lorin Modity
    // s32Ret = HI_PDT_MEDIA_VideoOutStop();
    // if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    // {
    //     MLOGD(YELLOW"not inited\n"NONE);
    // }
    // else
    // {
    //     PDT_STATEMNG_CHECK_RET(s32Ret,"stop VO");
    // }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    /** remove all stream */
    HI_LIVESVR_RemoveAllStream();

    HI_BOOL bCompress = HI_TRUE;
#ifdef CONFIG_RAWCAP_ON
    if(((HI_PDT_WORKMODE_NORM_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode)) &&
       (HI_TRUE == g_stSTATEMNGContext.bRawCapOn))
    {
        bCompress = HI_FALSE;
    }
#endif
    if((HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode ||
        HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PHOTOMNG_OUTPUT_FORMAT_DNG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt ||
        HI_PHOTOMNG_OUTPUT_FORMAT_DNG_JPEG == g_stSTATEMNGContext.stPhotoMngCtx.stPhotoAttr.enOutPutFmt))
    {
        bCompress = HI_FALSE;
    }

    HI_BOOL bLoadDsp = HI_FALSE;
#ifdef CFG_POST_PROCESS
    if (HI_PDT_WORKMODE_SING_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode
        || HI_PDT_WORKMODE_DLAY_PHOTO == g_stSTATEMNGContext.enCurrentWorkMode)
    {
        bCompress = HI_FALSE;
        bLoadDsp = HI_TRUE;
    }
#endif

    s32Ret = PDT_STATEMNG_UpdateViVpssMode(pstMediaCfg, &stMediaCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, "PDT_STATEMNG_UpdateViVpssMode");

    HI_BOOL bForceReBuid = g_stSTATEMNGContext.bForceReBuid;
    if(HI_TRUE == g_stSTATEMNGContext.bForceReBuid)
    {
        g_stSTATEMNGContext.bForceReBuid =  HI_FALSE;
    }

    /** set Media config */
    s32Ret = HI_PDT_MEDIA_Reset(&stMediaCfg, bCompress, bLoadDsp, bForceReBuid);
    if(HI_PDT_MEDIA_ENOTINIT == s32Ret)
    {
        MLOGD(YELLOW"not inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"reset video");
    }

    if(HI_TRUE == g_stSTATEMNGContext.bPreRecNoSD)
    {
        PDT_STATEMNG_StopEncoder(&g_stSTATEMNGContext.stMediaCfg, g_stSTATEMNGContext.enCurrentWorkMode);
        g_stSTATEMNGContext.bPreRecNoSD = HI_FALSE;
    }

    /** add streams */
    if((HI_PDT_WORKMODE_LPSE_REC != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_PLAYBACK != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_UVC != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_USB_STORAGE != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_HDMI_PREVIEW != g_stSTATEMNGContext.enCurrentWorkMode) &&
       (HI_PDT_WORKMODE_HDMI_PLAYBACK != g_stSTATEMNGContext.enCurrentWorkMode))
    {
        HI_S32 i = 0;
        const HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
        HI_HANDLE AencHdl = pstMediaCfg->stAudioCfg.astAcapCfg[0].bEnable? 0 : HI_INVALID_HANDLE;
        for(i = 0; i<HI_PDT_MEDIA_VENC_MAX_CNT; i++)
        {
            pstVencCfg = &(pstMediaCfg->stVideoCfg.astVencCfg[i]);
            if(pstVencCfg->bEnable && (PDT_STATEMNG_MAIN_STREAM_VENC_HDL == pstVencCfg->VencHdl))
            {
                s32Ret = HI_LIVESVR_AddStream(PDT_STATEMNG_MAIN_STREAM_VENC_HDL,
                    AencHdl, PDT_STATEMNG_MAIN_STREAM_NAME);
                PDT_STATEMNG_CHECK_RET(s32Ret, "HI_LIVESVR_AddStream main video stream");
            }

            if(pstVencCfg->bEnable && (PDT_STATEMNG_SUB_STREAM_VENC_HDL == pstVencCfg->VencHdl))
            {
                s32Ret = HI_LIVESVR_AddStream(PDT_STATEMNG_SUB_STREAM_VENC_HDL,
                    AencHdl, PDT_STATEMNG_SUB_STREAM_NAME);
                PDT_STATEMNG_CHECK_RET(s32Ret, "HI_LIVESVR_AddStream sub video stream");
            }
        }
    }

    /** process scene */
    HI_PDT_WORKMODE_E enWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
    if(HI_PDT_WORKMODE_HDMI_PREVIEW == enWorkMode)
    {
        s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enWorkMode);
        PDT_STATEMNG_CHECK_RET(s32Ret, "get poweron workmode");
        MLOGI("the workmode(%d) before enter hdmi.\n", enWorkMode);
    }

    if (pstSceneCfg &&
        ((HI_PDT_WORKMODE_PLAYBACK != enWorkMode) &&
         (HI_PDT_WORKMODE_HDMI_PLAYBACK != enWorkMode) &&
         (HI_PDT_WORKMODE_USB_STORAGE != enWorkMode) &&
         (HI_PDT_WORKMODE_UPGRADE != enWorkMode)))
    {

        HI_S32 i = 0;
        for(i = 0; i < HI_PDT_SCENE_PIPE_MAX_NUM; i++)
        {
            MLOGI(YELLOW"astPipeAttr[%d]:\n"NONE, i);
            MLOGI(YELLOW"bEnable(%d) enPipeType(%d) VcapPipeHdl(%d) PipeChnHdl(%d) MainPipeHdl(%d) u8PipeParamIndex(%d)\n"NONE,
                pstSceneCfg->astPipeAttr[i].bEnable,
                pstSceneCfg->astPipeAttr[i].enPipeType,
                pstSceneCfg->astPipeAttr[i].VcapPipeHdl,
                pstSceneCfg->astPipeAttr[i].PipeChnHdl,
                pstSceneCfg->astPipeAttr[i].MainPipeHdl,
                pstSceneCfg->astPipeAttr[i].u8PipeParamIndex);
        }
        s32Ret = HI_PDT_SCENE_SetSceneMode(pstSceneCfg);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set scene mode");

        /** find VcapPipeHdl of video pipe */
        HI_HANDLE VcapPipeHdl = 0;
        s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
        if(HI_SUCCESS != s32Ret)
        {
            s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
            PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
        }

        if (((HI_PDT_WORKMODE_SING_PHOTO == enWorkMode) ||
             (HI_PDT_WORKMODE_DLAY_PHOTO == enWorkMode)) &&
            (HI_PDT_PHOTO_SCENE_HDR == g_stSTATEMNGContext.enScene))
        {
            HI_PDT_SCENE_SetISO(VcapPipeHdl,0);
            HI_PDT_SCENE_SetEV(VcapPipeHdl, HI_PDT_SCENE_EV_0);
            HI_PDT_SCENE_SetWB(VcapPipeHdl, 0);

            HI_PDT_SCENE_METRY_S stMetry;
            stMetry.enMetryType = HI_PDT_SCENE_METRY_TYPE_AVERAGE;
            stMetry.unParam.stAverageMetryParam.u32Index =
                g_stSTATEMNGContext.stPhotoProTune.u16MetryAverageParamIdx;
            HI_PDT_SCENE_SetMetry(VcapPipeHdl, &stMetry);
        }
        else
        {
            if(PDT_STATEMNG_IsVideoPipe(enWorkMode))
            {
                HI_PDT_SCENE_SetExpTime(VcapPipeHdl, 0);
                /** g_stSTATEMNGContext.stVideoProTune not contain ExpTime */
                /** compare frame rate and ExpTime ,max is 30 ms, 30fps */

                MLOGI("u32ISO(%d) enEV(%d) u32WB(%d) enMetryType(%d)\n",
                    g_stSTATEMNGContext.stVideoProTune.u32ISO,
                    g_stSTATEMNGContext.stVideoProTune.enEV,
                    g_stSTATEMNGContext.stVideoProTune.u32WB,
                    g_stSTATEMNGContext.stVideoProTune.enMetryType);
                HI_PDT_SCENE_SetISO(VcapPipeHdl, g_stSTATEMNGContext.stVideoProTune.u32ISO);
                HI_PDT_SCENE_SetEV(VcapPipeHdl, g_stSTATEMNGContext.stVideoProTune.enEV);
                HI_PDT_SCENE_SetWB(VcapPipeHdl, g_stSTATEMNGContext.stVideoProTune.u32WB);

                HI_PDT_SCENE_METRY_S stMetry;
                stMetry.enMetryType = g_stSTATEMNGContext.stVideoProTune.enMetryType;
                if(HI_PDT_SCENE_METRY_TYPE_AVERAGE == stMetry.enMetryType)
                {
                    stMetry.unParam.stCenterMetryParam.u32Index =
                        g_stSTATEMNGContext.stVideoProTune.u16MetryAverageParamIdx;
                }
                else if(HI_PDT_SCENE_METRY_TYPE_CENTER == stMetry.enMetryType)
                {
                    stMetry.unParam.stCenterMetryParam.u32Index =
                        g_stSTATEMNGContext.stVideoProTune.u16MetryCenterParamIdx;
                }
                else if(HI_PDT_SCENE_METRY_TYPE_SPOT == stMetry.enMetryType)
                {
                    memcpy(&(stMetry.unParam.stSpotMetryParam),
                        &(g_stSTATEMNGContext.stSpotMetryParam),
                        sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));
                }
                HI_PDT_SCENE_SetMetry(VcapPipeHdl, &stMetry);
            }
            else if(PDT_STATEMNG_IsSnapPipe(enWorkMode))
            {
                MLOGI("u32ISO(%d) enEV(%d) u32WB(%d) enMetryType(%d)\n",
                    g_stSTATEMNGContext.stPhotoProTune.u32ISO,
                    g_stSTATEMNGContext.stPhotoProTune.enEV,
                    g_stSTATEMNGContext.stPhotoProTune.u32WB,
                    g_stSTATEMNGContext.stPhotoProTune.enMetryType);
                HI_PDT_SCENE_SetISO(VcapPipeHdl, g_stSTATEMNGContext.stPhotoProTune.u32ISO);
                HI_PDT_SCENE_SetEV(VcapPipeHdl, g_stSTATEMNGContext.stPhotoProTune.enEV);
                HI_PDT_SCENE_SetWB(VcapPipeHdl, g_stSTATEMNGContext.stPhotoProTune.u32WB);

                HI_PDT_SCENE_METRY_S stMetry;
                stMetry.enMetryType = g_stSTATEMNGContext.stPhotoProTune.enMetryType;
                if(HI_PDT_SCENE_METRY_TYPE_AVERAGE == stMetry.enMetryType)
                {
                    stMetry.unParam.stCenterMetryParam.u32Index =
                        g_stSTATEMNGContext.stPhotoProTune.u16MetryAverageParamIdx;
                }
                else if(HI_PDT_SCENE_METRY_TYPE_CENTER == stMetry.enMetryType)
                {
                    stMetry.unParam.stCenterMetryParam.u32Index =
                        g_stSTATEMNGContext.stPhotoProTune.u16MetryCenterParamIdx;
                }
                else if(HI_PDT_SCENE_METRY_TYPE_SPOT == stMetry.enMetryType)
                {
                    memcpy(&(stMetry.unParam.stSpotMetryParam),
                        &(g_stSTATEMNGContext.stSpotMetryParam),
                        sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));
                }
                HI_PDT_SCENE_SetMetry(VcapPipeHdl, &stMetry);
            }
        }

        s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
        PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");
    }

    MUTEX_LOCK(g_stSTATEMNGContext.Mutex);
    if(HI_TRUE != g_stSTATEMNGContext.bVOShutdown)

#if 0
    /** init VO */
    s32Ret = HI_PDT_MEDIA_VideoOutInit(&(pstMediaCfg->stVideoOutCfg));
    if(HI_PDT_MEDIA_EINITIALIZED == s32Ret)
    {
        MLOGD(YELLOW"already inited\n"NONE);
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"init VO");
    }
#endif

#if 1
    /** start VO */   //Lorin Modity
    // s32Ret = HI_PDT_MEDIA_VideoOutStart();
    // if(HI_SUCCESS != s32Ret)
    // {
    //     MLOGE(RED"start VO failed(0x%08X)\n"NONE, s32Ret);
    //     MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    //     return HI_STORAGEMNG_EINTER;
    // }
#else
    /** for low power test*/
    MLOGD(YELLOW"not start VO\n"NONE);
#endif

#if defined(CONFIG_SCREEN_ST7796S)
    s32Ret = HI_HAL_SCREEN_SetDisplayState(HI_HAL_SCREEN_IDX_0, HI_HAL_SCREEN_STATE_ON);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set disp state(on) failed(0x%08X)\n"NONE, s32Ret);
        MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
        return HI_STORAGEMNG_EINTER;
    }
    MLOGD("set disp state(on) succeed\n");
#endif

    MUTEX_UNLOCK(g_stSTATEMNGContext.Mutex);
    return s32Ret;
}

HI_S32 PDT_STATEMNG_DelFile(const HI_CHAR *pszFilename)
{
    HI_S32 s32Ret = HI_SUCCESS;

    PDT_STATEMNG_CHECK_NULL_PTR(pszFilename,"pszFilename");

    s32Ret = HI_FILEMNG_RemoveFile(pszFilename);/** ActionCam use DCF */
    if(HI_FILEMNG_ENOTMAIN == s32Ret)
    {
        MLOGD(YELLOW"Not main file(%s)\n"NONE, pszFilename);
        return HI_SUCCESS;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret,"filemng delet file");
    }
    MLOGD(YELLOW"delete file(%s) succeed\n"NONE, pszFilename);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_GenerateRecMngAttr(HI_PDT_WORKMODE_E enEnterWorkMode,
    const HI_PDT_WORKMODE_CFG_S *pstEnterWorkModeCfg, HI_RECMNG_ATTR_S *pstRecMngAttr)
{
    /** check parameter */
    PDT_STATEMNG_CHECK_NULL_PTR(pstRecMngAttr,"pstRecMngAttr");
    PDT_STATEMNG_CHECK_NULL_PTR(pstEnterWorkModeCfg,"pstEnterWorkModeCfg");

    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_WORKMODE_CFG_S *pstWorkModeCfg = (HI_PDT_WORKMODE_CFG_S *)pstEnterWorkModeCfg;
    memset(pstRecMngAttr, 0, sizeof(HI_RECMNG_ATTR_S));
    HI_PDT_REC_SRC_S *pstDataSrc = NULL;
    HI_PDT_REC_COMM_ATTR_S *pstCommAttr = NULL;
    HI_PDT_VIDEO_PROTUNE_ATTR_S *pstProTune =  NULL;

    /** specific processing for different record workmode */
    switch(enEnterWorkMode)
    {
        case HI_PDT_WORKMODE_NORM_REC:
        {
            pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stNormRecAttr.stDataSrc);
            pstCommAttr = &(pstWorkModeCfg->unModeAttr.stNormRecAttr.stCommAttr);
            pstProTune = &(pstWorkModeCfg->unModeAttr.stNormRecAttr.stProTune);
        }
        break;
        case HI_PDT_WORKMODE_LOOP_REC:
        {
            pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stLoopRecAttr.stDataSrc);
            pstCommAttr = &(pstWorkModeCfg->unModeAttr.stLoopRecAttr.stCommAttr);
            pstProTune = &(pstWorkModeCfg->unModeAttr.stLoopRecAttr.stProTune);

            /** loop record unique */
            pstRecMngAttr->stLoopAttr.bLoop = HI_TRUE;

            MLOGD(YELLOW"enSplitType(%d), u32SplitTimeLenSec(%d)\n"NONE,
                pstCommAttr->stSplitAttr.enSplitType,pstCommAttr->stSplitAttr.u32SplitTimeLenSec);

            if(HI_REC_SPLIT_TYPE_TIME == pstCommAttr->stSplitAttr.enSplitType)
            {
                if((pstCommAttr->stSplitAttr.u32SplitTimeLenSec != 0) &&
                   (0 == ((pstWorkModeCfg->unModeAttr.stLoopRecAttr.u32LoopTime_min)*60) %
                    pstCommAttr->stSplitAttr.u32SplitTimeLenSec))
                {
                    pstRecMngAttr->stLoopAttr.u32LoopNum =
                        ((pstWorkModeCfg->unModeAttr.stLoopRecAttr.u32LoopTime_min)*60) /
                        pstCommAttr->stSplitAttr.u32SplitTimeLenSec;
                }
                else
                {
                    MLOGW(YELLOW"u32SplitTimeLenSec is zero or be divided with remainder,"
                        "use default split time(one minute)\n"NONE);
                    pstRecMngAttr->stLoopAttr.u32LoopNum =
                        pstWorkModeCfg->unModeAttr.stLoopRecAttr.u32LoopTime_min;
                }
            }
            else
            {
                MLOGW(YELLOW"type err, use default split time(one minute)\n"NONE);
                pstRecMngAttr->stLoopAttr.u32LoopNum =
                    pstWorkModeCfg->unModeAttr.stLoopRecAttr.u32LoopTime_min;
            }
            pstRecMngAttr->stLoopAttr.u32FileIdx = 0;/** default use main stream */
            pstRecMngAttr->stLoopAttr.pfnDelFile = PDT_STATEMNG_DelFile;
        }
        break;
        case HI_PDT_WORKMODE_LPSE_REC:
        {
            pstRecMngAttr->enRecType = HI_REC_TYPE_LAPSE;
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stLapseRecAttr.stDataSrc);
            pstCommAttr = &(pstWorkModeCfg->unModeAttr.stLapseRecAttr.stCommAttr);
            pstProTune = &(pstWorkModeCfg->unModeAttr.stLapseRecAttr.stProTune);

            /** lapse record unique */
            pstRecMngAttr->unRecordAttr.stLapseAttr.u32IntervalMs =
                pstWorkModeCfg->unModeAttr.stLapseRecAttr.u32Interval_ms;
        }
        break;
        case HI_PDT_WORKMODE_SLOW_REC:
        {
            pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stSlowRecAttr.stDataSrc);
            pstCommAttr = &(pstWorkModeCfg->unModeAttr.stSlowRecAttr.stCommAttr);
            pstProTune = &(pstWorkModeCfg->unModeAttr.stSlowRecAttr.stProTune);
        }
        break;
        case HI_PDT_WORKMODE_RECSNAP:
        {
            pstRecMngAttr->enRecType = HI_REC_TYPE_NORMAL;
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stRecSnapAttr.stDataSrc.stRecSrc);
            pstCommAttr = &(pstWorkModeCfg->unModeAttr.stRecSnapAttr.stRecAttr);
            pstProTune = &(pstWorkModeCfg->unModeAttr.stRecSnapAttr.stProTune);
        }
        break;
        default:
        {
            MLOGE(RED"param err\n"NONE);
            return HI_FAILURE;
        }
    }

    /** store the proTune parameters */
    memcpy(&(g_stSTATEMNGContext.stVideoProTune), pstProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

    /** the common processing */
    memcpy(&(pstRecMngAttr->stSplitAttr), &(pstCommAttr->stSplitAttr), sizeof(HI_REC_SPLIT_ATTR_S));
    pstRecMngAttr->u32FileCnt = 0;

    /** get audio switch from Param module */
    HI_BOOL bEnableAudio = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enEnterWorkMode,
        HI_PDT_PARAM_TYPE_AUDIO, (HI_VOID *)&bEnableAudio);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get bEnableAudio");
    MLOGD(YELLOW"bEnableAudio(%d)\n"NONE, bEnableAudio);

    HI_S32 i = 0;
    for(i = 0; i < HI_PDT_REC_VSTREAM_MAX_CNT; i++)
    {
        HI_RECMNG_FILE_ATTR_S stFileAttr;
        memset(&stFileAttr, 0, sizeof(HI_RECMNG_FILE_ATTR_S));

        stFileAttr.astBufCfg[0].enBufType = HI_RECMNG_BUF_TYPE_OS;

        if(pstDataSrc->aVencHdl[i] != PDT_STATEMNG_INVALID_HANDLE)
        {
            stFileAttr.stDataSource.u32VencCnt = 1;
            stFileAttr.stDataSource.aVencHdl[0] = pstDataSrc->aVencHdl[i];
        }
        else
        {
            MLOGD(YELLOW"param err\n"NONE);
            continue;
        }

        if(bEnableAudio)
        {
            stFileAttr.stDataSource.u32AencCnt = 1;
        }
        else
        {
            stFileAttr.stDataSource.u32AencCnt = 0;
        }
        stFileAttr.stDataSource.aAencHdl[0] = pstDataSrc->AencHdl;
        stFileAttr.stDataSource.ThmHdl = pstDataSrc->ThmHdl;
        switch(enEnterWorkMode)
        {
            case HI_PDT_WORKMODE_NORM_REC:
            case HI_PDT_WORKMODE_LOOP_REC:
            case HI_PDT_WORKMODE_RECSNAP:
                stFileAttr.s32PlayFps = -1;/** use venc fps */
                break;
            case HI_PDT_WORKMODE_LPSE_REC:
                stFileAttr.s32PlayFps = PDT_STATEMNG_RECMNG_LAPSE_PLAY_FPS;
                stFileAttr.stDataSource.u32AencCnt = 0;
                break;
            case HI_PDT_WORKMODE_SLOW_REC:
                stFileAttr.s32PlayFps =
                    pstWorkModeCfg->unModeAttr.stSlowRecAttr.au32PlayFrmRate[i];
                stFileAttr.stDataSource.u32AencCnt = 0;
                break;
            default:
            {
                MLOGE(RED"param err\n"NONE);
                return HI_FAILURE;
            }
        }

        stFileAttr.stMuxerCfg.enMuxerType = pstCommAttr->enMuxerType;
        switch(stFileAttr.stMuxerCfg.enMuxerType)
        {
#if defined(CONFIG_RECORDMNG_MP4)
            case HI_RECMNG_MUXER_MP4:
                stFileAttr.stMuxerCfg.unMuxerCfg.stMp4Cfg.u32RepairUnit =
                    pstCommAttr->u32RepairUnit;
                break;
            case HI_RECMNG_MUXER_MOV:
                stFileAttr.stMuxerCfg.unMuxerCfg.stMovCfg.u32RepairUnit =
                    pstCommAttr->u32RepairUnit;
                break;
#endif

#if defined(CONFIG_RECORDMNG_TS)
            case HI_RECMNG_MUXER_TS:
                break;
#endif

            default:
                break;
        }
        stFileAttr.stMuxerCfg.u32PreAllocUnit = pstCommAttr->u32PreAllocUnit;
        stFileAttr.stMuxerCfg.u32VBufSize = pstCommAttr->au32VBufSize[i];

        memcpy(&pstRecMngAttr->astFileAttr[pstRecMngAttr->u32FileCnt], &stFileAttr, sizeof(HI_RECMNG_FILE_ATTR_S));

        MLOGD("File(%d) attr:\n",pstRecMngAttr->u32FileCnt);
        MLOGD(" VencCnt(%d:%d), AencCnt(%d:%d), s32PlayFps(%d)\n"
              " stMuxerCfg.enMuxerType(%d)\n"
#if defined(CONFIG_RECORDMNG_MP4)
              " stMuxerCfg.unMuxerCfg.stMp4Cfg.u32RepairUnit(%d)\n"
#endif
              " stMuxerCfg.u32PreAllocUnit(%d)\n"
              " stMuxerCfg.u32VBufSize(%d)\n",
              stFileAttr.stDataSource.u32VencCnt, stFileAttr.stDataSource.aVencHdl[0],
              stFileAttr.stDataSource.u32AencCnt, stFileAttr.stDataSource.aAencHdl[0],
              stFileAttr.s32PlayFps, stFileAttr.stMuxerCfg.enMuxerType,
#if defined(CONFIG_RECORDMNG_MP4)
              stFileAttr.stMuxerCfg.unMuxerCfg.stMp4Cfg.u32RepairUnit,
#endif
              stFileAttr.stMuxerCfg.u32PreAllocUnit, stFileAttr.stMuxerCfg.u32VBufSize);

        pstRecMngAttr->u32FileCnt++;
    }

    pstRecMngAttr->enThmType = pstCommAttr->enThmType;
    pstRecMngAttr->stThmAttr.u32ThmCnt = 1;
    pstRecMngAttr->stThmAttr.aThmHdl[0] = pstDataSrc->ThmHdl;
    MLOGI("ThmType(%d) Cnt(%u) Hdl0(%d)\n", pstRecMngAttr->enThmType,
        pstRecMngAttr->stThmAttr.u32ThmCnt, pstRecMngAttr->stThmAttr.aThmHdl[0]);
    pstRecMngAttr->stGetFilename.pfnGetFilenames = PDT_STATEMNG_GetRecTaskFileName;
    pstRecMngAttr->stGetFilename.pvPrivData = (HI_VOID *)NULL;
    pstRecMngAttr->u32BufferTimeMSec = pstCommAttr->u32BufferTimeMSec;
    pstRecMngAttr->u32PreRecTimeSec = pstCommAttr->u32PreRecTimeSec;

    return HI_SUCCESS;
}

HI_S32 PDT_STATEMNG_GeneratePhotoAttr(HI_PDT_WORKMODE_E enEnterWorkMode,
    const HI_PDT_WORKMODE_CFG_S *pstEnterWorkModeCfg,
    HI_PHOTOMNG_PHOTO_ATTR_S *pstPhotoAttr)
{
    PDT_STATEMNG_CHECK_NULL_PTR(pstPhotoAttr,"pstPhotoAttr");
    PDT_STATEMNG_CHECK_NULL_PTR(pstEnterWorkModeCfg,"pstEnterWorkModeCfg");

    HI_PDT_WORKMODE_CFG_S *pstWorkModeCfg = (HI_PDT_WORKMODE_CFG_S *)pstEnterWorkModeCfg;
    memset(pstPhotoAttr, 0, sizeof(HI_PHOTOMNG_PHOTO_ATTR_S));
    HI_PHOTOMNG_PHOTO_SRC_S *pstDataSrc = NULL;

    /** specific processing for different photo workmode */
    switch(enEnterWorkMode)
    {
        case HI_PDT_WORKMODE_SING_PHOTO:
        {
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stSinglePhotoAttr.stDataSrc);
            pstPhotoAttr->enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_SINGLE;
            memcpy(&(pstPhotoAttr->stPhotoSRC), pstDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));
            pstPhotoAttr->enOutPutFmt =
                pstWorkModeCfg->unModeAttr.stSinglePhotoAttr.enOutputFmt;
            pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.s32Time_sec = 0;/** photo immediately */
            pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.bNeedTrigger = HI_TRUE;

            /** store the proTune parameters */
            HI_PDT_PHOTO_PROTUNE_ATTR_S *pstProTune =
                &(pstWorkModeCfg->unModeAttr.stSinglePhotoAttr.stProTune);
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   pstProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
        }
        break;
        case HI_PDT_WORKMODE_DLAY_PHOTO:
        {
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stDelayPhotoAttr.stDataSrc);
            pstPhotoAttr->enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_SINGLE;
            memcpy(&(pstPhotoAttr->stPhotoSRC), pstDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));
            pstPhotoAttr->enOutPutFmt =
                pstWorkModeCfg->unModeAttr.stDelayPhotoAttr.enOutputFmt;
            pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.s32Time_sec =
                pstWorkModeCfg->unModeAttr.stDelayPhotoAttr.u32DelayTime_s;
            pstPhotoAttr->unPhotoTypeAttr.stSingleAttr.bNeedTrigger = HI_TRUE;

            /** store the proTune parameters */
            HI_PDT_PHOTO_PROTUNE_ATTR_S *pstProTune =
                &(pstWorkModeCfg->unModeAttr.stDelayPhotoAttr.stProTune);
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   pstProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
        }
        break;
        case HI_PDT_WORKMODE_LPSE_PHOTO:
        {
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stLapsePhotoAttr.stDataSrc);
            pstPhotoAttr->enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_LAPSE;
            memcpy(&(pstPhotoAttr->stPhotoSRC), pstDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));
            pstPhotoAttr->enOutPutFmt = HI_PHOTOMNG_OUTPUT_FORMAT_JPEG;
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms =
                pstWorkModeCfg->unModeAttr.stLapsePhotoAttr.u32Interval_ms;
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.bNeedTrigger = HI_TRUE;

            /** store the proTune parameters */
            HI_PDT_PHOTO_PROTUNE_ATTR_S *pstProTune =
                &(pstWorkModeCfg->unModeAttr.stLapsePhotoAttr.stProTune);
            memcpy(&(g_stSTATEMNGContext.stPhotoProTune),
                   pstProTune, sizeof(HI_PDT_PHOTO_PROTUNE_ATTR_S));
        }
        break;
        case HI_PDT_WORKMODE_BURST:
        {
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stBurstAttr.stDataSrc);
            pstPhotoAttr->enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_MULTIPLE;
            memcpy(&(pstPhotoAttr->stPhotoSRC), pstDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));
            pstPhotoAttr->enOutPutFmt = HI_PHOTOMNG_OUTPUT_FORMAT_JPEG;

            /** store the proTune parameters */
            HI_PDT_VIDEO_PROTUNE_ATTR_S *pstProTune =
                &(pstWorkModeCfg->unModeAttr.stBurstAttr.stProTune);
            memcpy(&(g_stSTATEMNGContext.stVideoProTune),
                   pstProTune, sizeof(HI_PDT_VIDEO_PROTUNE_ATTR_S));

            switch(pstWorkModeCfg->unModeAttr.stBurstAttr.enJpgBurstType)
            {
                case HI_PDT_JPG_BURST_TYPE_3_1:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 3;
                    break;
                case HI_PDT_JPG_BURST_TYPE_5_1:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 5;
                    break;
                case HI_PDT_JPG_BURST_TYPE_10_1:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 10;
                    break;
                case HI_PDT_JPG_BURST_TYPE_15_1:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 15;
                    break;
                case HI_PDT_JPG_BURST_TYPE_20_2:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 20;
                    break;
                case HI_PDT_JPG_BURST_TYPE_30_1:
                case HI_PDT_JPG_BURST_TYPE_30_2:
                case HI_PDT_JPG_BURST_TYPE_30_3:
                    pstPhotoAttr->unPhotoTypeAttr.stMultipleAttr.s32Count = 30;
                    break;
                default:
                {
                    MLOGE(RED"param err\n"NONE);
                    return HI_FAILURE;
                }
            }
        }
        break;
        case HI_PDT_WORKMODE_RECSNAP:
        {
            pstDataSrc = &(pstWorkModeCfg->unModeAttr.stRecSnapAttr.stDataSrc.stPhotoSrc);
            pstPhotoAttr->enPhotoType = HI_PHOTOMNG_PHOTO_TYPE_LAPSE;
            memcpy(&(pstPhotoAttr->stPhotoSRC), pstDataSrc, sizeof(HI_PHOTOMNG_PHOTO_SRC_S));
            pstPhotoAttr->enOutPutFmt = HI_PHOTOMNG_OUTPUT_FORMAT_JPEG;
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.s32Interval_ms =
                pstWorkModeCfg->unModeAttr.stRecSnapAttr.u32SnapInterval_ms;
            pstPhotoAttr->unPhotoTypeAttr.stLapseAttr.bNeedTrigger = HI_FALSE;
        }
        break;
        default:
        {
            MLOGE(RED"param err\n"NONE);
            return HI_FAILURE;
        }
    }

    PDT_STATEMNG_PrintPhotoAttr(enEnterWorkMode, (const HI_PHOTOMNG_PHOTO_ATTR_S *)pstPhotoAttr);
    return HI_SUCCESS;
}

HI_VOID PDT_STATEMNG_GeneratePhotoMngTaskCfg(HI_PHOTOMNG_TASK_CFG_S* pstPhotoMngTaskConfig)
{
    pstPhotoMngTaskConfig->stGetNameCB.pfnGetNameCallBack = PDT_STATEMNG_GetPhotoTaskFileName;
    pstPhotoMngTaskConfig->stGetNameCB.pvPrivateData = (HI_VOID *)NULL;

    pstPhotoMngTaskConfig->stGetParamCB.pfnGetParamCallBack = PDT_STATEMNG_GetPhotoParam;
    pstPhotoMngTaskConfig->stGetParamCB.pvPrivateData = (HI_VOID *)NULL;
}

/** get EXIF Info */
HI_VOID PDT_STATEMNG_GetCamExifInfo(HI_MAPI_SNAP_EXIF_INFO_S* pstExifInfo)
{
    memset(pstExifInfo, 0, sizeof(HI_MAPI_SNAP_EXIF_INFO_S));

    /*set exif info*/
    snprintf((HI_CHAR*)pstExifInfo->au8ImageDescription,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"single Photo");
    snprintf((HI_CHAR*)pstExifInfo->au8Make,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"Hisilicon");
    snprintf((HI_CHAR*)pstExifInfo->au8Model,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"DCIM");
    snprintf((HI_CHAR*)pstExifInfo->au8Software,
        HI_MAPI_EXIF_DRSCRIPTION_LENGTH,"HiMobileCam");

    pstExifInfo->stApertureInfo.u32MinIrisFNOTarget = 1;
    pstExifInfo->stApertureInfo.u32MaxIrisFNOTarget = 1024;
    pstExifInfo->stGPSInfo.chGPSLatitudeRef = 'N';
    pstExifInfo->stGPSInfo.au32GPSLatitude[0][1]=1;
    pstExifInfo->stGPSInfo.au32GPSLatitude[1][1]=1;
    pstExifInfo->stGPSInfo.au32GPSLatitude[2][1]=1;
    pstExifInfo->stGPSInfo.chGPSLongitudeRef = 'E';
    pstExifInfo->stGPSInfo.au32GPSLongitude[0][1]=1;
    pstExifInfo->stGPSInfo.au32GPSLongitude[1][1]=1;
    pstExifInfo->stGPSInfo.au32GPSLongitude[2][1]=1;
    pstExifInfo->stGPSInfo.au32GPSAltitude[1]=1;
}

HI_S32 PDT_STATEMNG_CheckWorkModeSettingItemValue(HI_MESSAGE_S * pstMsg, HI_S32 s32SettingItemValue)
{
    HI_S32 s32Ret = HI_SUCCESS;

    HI_PDT_WORKMODE_E enWorkMode = pstMsg->arg2;
    HI_PDT_PARAM_TYPE_E enType = pstMsg->arg1;
    MLOGD("WorkMode(%d), setting type(%d), setting value(%d)\n",
        enWorkMode, enType, s32SettingItemValue);

    HI_PDT_ITEM_VALUESET_S stValueSet;
    memset(&stValueSet, 0, sizeof(HI_PDT_ITEM_VALUESET_S));
    s32Ret = HI_PDT_PARAM_GetWorkModeItemValues(enWorkMode, enType, &stValueSet);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode item value");

    HI_S32 i = 0;
    for(i = 0; i < stValueSet.s32Cnt; i++)
    {
        MLOGD("astValues[%d].s32Value(%d)\n", i, stValueSet.astValues[i].s32Value);
        if(stValueSet.astValues[i].s32Value == s32SettingItemValue)
        {
            return HI_SUCCESS;
        }
    }

    return HI_FAILURE;
}

HI_S32 PDT_STATEMNG_SavePoweronWorkMode(HI_PDT_WORKMODE_E enSettingWorkmode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** save poweron workmode */
    HI_PDT_WORKMODE_E enPoweronWorkmode = HI_PDT_WORKMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE, &enPoweronWorkmode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get poweron workmode");
    MLOGD("enPoweronWorkmode(%d)\n", enPoweronWorkmode);

    if(enSettingWorkmode != enPoweronWorkmode)
    {
        s32Ret = HI_PDT_PARAM_SetCommParam(HI_PDT_PARAM_TYPE_POWERON_WORKMODE,
            (HI_VOID *)&(enSettingWorkmode));
        PDT_STATEMNG_CHECK_RET(s32Ret,"save poweron workmode param");

        HI_CHAR szStateName[HI_STATE_NAME_LEN] = {0};
        s32Ret = PDT_STATEMNG_WorkMode2State(enSettingWorkmode, szStateName, HI_STATE_NAME_LEN);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get workmode name");
        MLOGD(YELLOW"save poweron workmode(%s:%d) param succeed\n"NONE,
            szStateName, enSettingWorkmode);
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetAudio(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bEnableAudio = *(HI_BOOL*)pvPayload;
    MLOGD(YELLOW"set audio switcher(%d), workmode(%d)\n"NONE, bEnableAudio, enSettingWorkMode);

    /** get param to Param module */
    HI_BOOL bOldEnableAudio = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_AUDIO, (HI_VOID *)&bOldEnableAudio);
    MLOGD(YELLOW"curr audio switcher(%d), workmode(%d)\n"NONE, bOldEnableAudio, enSettingWorkMode);

    /** filter out the same parameter */
    if(bOldEnableAudio == bEnableAudio)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    if((HI_PDT_WORKMODE_LPSE_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
       (HI_PDT_WORKMODE_SLOW_REC == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        MLOGD(YELLOW"only save param\n"NONE);
    }
    else
    {
        /** remove all streams */
        s32Ret = HI_LIVESVR_RemoveStream(PDT_STATEMNG_SUB_STREAM_NAME);
        PDT_STATEMNG_CHECK_RET(s32Ret, "remove sub stream");
        MLOGD(YELLOW"remove sub stream succeed\n"NONE);

        if((HI_PDT_WORKMODE_NORM_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
           (HI_PDT_WORKMODE_LOOP_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
           (HI_PDT_WORKMODE_RECSNAP == g_stSTATEMNGContext.enCurrentWorkMode))
        {
            s32Ret = HI_LIVESVR_RemoveStream(PDT_STATEMNG_MAIN_STREAM_NAME);
            PDT_STATEMNG_CHECK_RET(s32Ret, "remove main stream");
            MLOGD(YELLOW"remove main stream succeed\n"NONE);
        }

        /** update media cfg */
        HI_S32 i = 0;
        for(i = 0; i < HI_PDT_MEDIA_ACAP_MAX_CNT; i++)
        {
            pstMediaCfg->stAudioCfg.astAcapCfg[i].bEnable = bEnableAudio;
        }

        for(i = 0; i < HI_PDT_MEDIA_AENC_MAX_CNT; i++)
        {
            pstMediaCfg->stAudioCfg.astAencCfg[i].bEnable = bEnableAudio;
        }

        /** destoty RecMng task */
        s32Ret = HI_RECMNG_DestroyTask(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"destroy RecMng task");
        g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl = PDT_STATEMNG_INVALID_HANDLE;

        /** reset Media */
        s32Ret = PDT_STATEMNG_ResetMedia(&(g_stSTATEMNGContext.stMediaCfg),
            &(g_stSTATEMNGContext.stSceneModeCfg));
        PDT_STATEMNG_CHECK_RET(s32Ret,"reset media");

        /** reset record task attr and create RecMng task */

        HI_RECMNG_ATTR_S* pstRecMngAttr =
            &(g_stSTATEMNGContext.stRecMngCtx.stRecMngAttr);
        if(HI_RECMNG_TASK_MAX_FILE_CNT < pstRecMngAttr->u32FileCnt)
        {
            return HI_FAILURE;
        }
        if(!bEnableAudio)
        {
            for(i = 0; i < pstRecMngAttr->u32FileCnt; i++)
            {
                pstRecMngAttr->astFileAttr[i].stDataSource.u32AencCnt = 0;
            }

            s32Ret = HI_RECMNG_CreateTask(pstRecMngAttr,
                &(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl));
            PDT_STATEMNG_CHECK_RET(s32Ret,"create RecMng task");
        }
        else
        {
            for(i = 0; i < pstRecMngAttr->u32FileCnt; i++)
            {
                pstRecMngAttr->astFileAttr[i].stDataSource.u32AencCnt = 1;
            }
            s32Ret = HI_RECMNG_CreateTask(pstRecMngAttr,
                &(g_stSTATEMNGContext.stRecMngCtx.RecMngTaskHdl));
            PDT_STATEMNG_CHECK_RET(s32Ret,"create RecMng task");
        }
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_AUDIO, (HI_VOID *)&bEnableAudio);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save bEnableAudio param");
    MLOGD(YELLOW"save bEnableAudio(%d) param succeed\n"NONE, bEnableAudio);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetFlip(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL  bEnable   = *(HI_BOOL*)pvPayload;
    MLOGD(YELLOW"set Flip switcher(%d), workmode(%d)\n"NONE, bEnable, enSettingWorkMode);

    /** get param to Param module */
    HI_BOOL bOldEnableFlip = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_FLIP, (HI_VOID *)&bOldEnableFlip);
    MLOGD(YELLOW"curr Flip switcher(%d), workmode(%d)\n"NONE, bOldEnableFlip, enSettingWorkMode);

    /** filter out the same parameter */
    if(bOldEnableFlip == bEnable)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_FLIP, (HI_VOID *)&bEnable);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save bEnableFlip param");
    MLOGD(YELLOW"save bEnableFlip(%d) param succeed\n"NONE, bEnable);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetOSD(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bEnableOSD = *(HI_BOOL *)pvPayload;

    MLOGD(YELLOW"set OSD switcher(%d), workmode(%d)\n"NONE, bEnableOSD, enSettingWorkMode);

    /** get param to Param module */
    HI_BOOL bOldEnableOSD = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_OSD, (HI_VOID *)&bOldEnableOSD);
    MLOGD(YELLOW"curr OSD switcher(%d), workmode(%d)\n"NONE, bOldEnableOSD, enSettingWorkMode);

    /** filter out the same parameter */
    if(bOldEnableOSD == bEnableOSD)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    pstMediaCfg->stVideoCfg.stOsdCfg.stTimeOsd.bShow = bEnableOSD;

    s32Ret = HI_PDT_MEDIA_SetTimeOSD(bEnableOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set media TimeOSD");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_OSD, (HI_VOID *)&bEnableOSD);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save bEnableOSD param");
    MLOGD(YELLOW"save bEnableOSD(%d) param succeed\n"NONE, bEnableOSD);

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetDIS(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bEnableDIS = *(HI_BOOL *)pvPayload;
    MLOGD(YELLOW"set DIS switcher(%d), workmode(%d)\n"NONE, bEnableDIS, enSettingWorkMode);

    /** get param to Param module */
    HI_BOOL bOldEnableDIS = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_DIS, (HI_VOID *)&bOldEnableDIS);
    MLOGD(YELLOW"curr DIS switcher(%d), workmode(%d)\n"NONE, bOldEnableDIS, enSettingWorkMode);

    /** filter out the same parameter */
    if(bOldEnableDIS == bEnableDIS)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** get media mode */
    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get mediamode");
    MLOGD("curr mediamode(%d)\n", enMediaMode);

    /** get media capability */
    HI_PDT_PARAM_MEDIA_CAPABILITY_S stCapability;
    memset(&stCapability, 0, sizeof(HI_PDT_PARAM_MEDIA_CAPABILITY_S));
    s32Ret = HI_PDT_PARAM_GetMediaCapability(enMediaMode, &stCapability);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get media capability");

    HI_S32 i = 0, j = 0, k = 0;
    HI_HANDLE VcapPipeHdl = 0;
    HI_HANDLE VcapPipeChnHdl = 0;
    HI_BOOL bTekeEffect = HI_FALSE;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(HI_TRUE != pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstVcapCfg =
            &(pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i]);
        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if(HI_TRUE != pstVcapCfg->astVcapPipeAttr[j].bEnable)
            {
                continue;
            }

            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S *pstVcapPipeAttr = &(pstVcapCfg->astVcapPipeAttr[j]);
            VcapPipeHdl = pstVcapPipeAttr->VcapPipeHdl;

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if(HI_TRUE != pstVcapPipeAttr->astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                VcapPipeChnHdl = pstVcapPipeAttr->astPipeChnAttr[k].PipeChnHdl;

                MLOGD(YELLOW"astVcap(%d).astVcapPipe(%d).astPipeChn(%d).bSupportDIS(%d)\n"NONE,
                    i, j, k, stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportDIS);
                if(stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportDIS)
                {
                    s32Ret = HI_PDT_MEDIA_SetDIS(VcapPipeHdl, VcapPipeChnHdl, bEnableDIS);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"set media DIS");
                    bTekeEffect = HI_TRUE;
                }
            }
        }
    }

    if(bTekeEffect)
    {
        /** save param to Param module */
        s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
            HI_PDT_PARAM_TYPE_DIS, (HI_VOID *)&bEnableDIS);
        PDT_STATEMNG_CHECK_RET(s32Ret,"set bEnableDIS param");
        MLOGD(YELLOW"save bEnableDIS(%d) param succeed\n"NONE, bEnableDIS);
    }
    else
    {
        MLOGD(YELLOW"all pipe not support DIS\n"NONE);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetLDC(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_BOOL bEnableLDC = *(HI_BOOL *)pvPayload;
    MLOGD(YELLOW"set LDC switcher(%d), workmode(%d)\n"NONE, bEnableLDC, enSettingWorkMode);

    /** get param to Param module */
    HI_BOOL bOldEnableLDC = HI_FALSE;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_LDC, (HI_VOID *)&bOldEnableLDC);
    MLOGD(YELLOW"curr LDC switcher(%d), workmode(%d)\n"NONE, bOldEnableLDC, enSettingWorkMode);

    /** filter out the same parameter */
    if(bOldEnableLDC == bEnableLDC)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** get media mode */
    HI_PDT_MEDIAMODE_E enMediaMode = HI_PDT_MEDIAMODE_BUTT;
    s32Ret = HI_PDT_PARAM_GetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_MEDIAMODE, (HI_VOID *)&enMediaMode);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get mediamode");
    MLOGD("curr mediamode(%d)\n", enMediaMode);

    /** get media capability */
    HI_PDT_PARAM_MEDIA_CAPABILITY_S stCapability;
    memset(&stCapability, 0, sizeof(HI_PDT_PARAM_MEDIA_CAPABILITY_S));
    s32Ret = HI_PDT_PARAM_GetMediaCapability(enMediaMode, &stCapability);
    PDT_STATEMNG_CHECK_RET(s32Ret,"get media capability");

    HI_S32 i = 0, j = 0, k = 0;
    HI_HANDLE VcapPipeHdl = 0;
    HI_HANDLE VcapPipeChnHdl = 0;
    HI_BOOL bTekeEffect = HI_FALSE;
    for(i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        if(HI_TRUE != pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i].bEnable)
        {
            continue;
        }

        HI_PDT_MEDIA_VCAP_DEV_ATTR_S *pstVcapCfg =
            &(pstMediaCfg->stVideoCfg.stVcapCfg.astVcapDevAttr[i]);
        for(j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
        {
            if(HI_TRUE != pstVcapCfg->astVcapPipeAttr[j].bEnable)
            {
                continue;
            }

            HI_PDT_MEDIA_VCAP_PIPE_ATTR_S *pstVcapPipeAttr = &(pstVcapCfg->astVcapPipeAttr[j]);
            VcapPipeHdl = pstVcapPipeAttr->VcapPipeHdl;

            for(k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; k++)
            {
                if(HI_TRUE != pstVcapPipeAttr->astPipeChnAttr[k].bEnable)
                {
                    continue;
                }

                VcapPipeChnHdl = pstVcapPipeAttr->astPipeChnAttr[k].PipeChnHdl;

                MLOGD(YELLOW"astVcap(%d).astVcapPipe(%d).astPipeChn(%d).bSupportLDC(%d)\n"NONE,
                    i, j, k, stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportLDC);
                if(stCapability.astVcapDev[i].astVcapPipe[j].astPipeChn[k].bSupportLDC)
                {
                    s32Ret = HI_PDT_MEDIA_SetLDC(VcapPipeHdl, VcapPipeChnHdl, bEnableLDC);
                    PDT_STATEMNG_CHECK_RET(s32Ret,"set media LDC");
                    bTekeEffect = HI_TRUE;
                }
            }
        }
    }

    if(bTekeEffect)
    {
        /** save param to Param module */
        s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
            HI_PDT_PARAM_TYPE_LDC, (HI_VOID *)&bEnableLDC);
        PDT_STATEMNG_CHECK_RET(s32Ret,"save bEnableLDC param");
        MLOGD(YELLOW"save bEnableLDC(%d) param succeed\n"NONE, bEnableLDC);
    }
    else
    {
        MLOGD(YELLOW"all pipe not support LDC\n"NONE);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetExpTime(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 s32SettingProtuneExpTime = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set ExpTime(%d), workmode(%d)\n"NONE, s32SettingProtuneExpTime, enSettingWorkMode);

    /** depend on param for video ExpTime setting */
    MLOGD(YELLOW"curr ExpTime(%d), workmode(%d)\n"NONE,
        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us, enSettingWorkMode);
    /** filter out the same parameter */
    if(g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us == s32SettingProtuneExpTime)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)s32SettingProtuneExpTime);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_HANDLE VcapPipeHdl = 0;
    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
    }

    HI_PDT_WORKMODE_E enWorkMode = g_stSTATEMNGContext.enCurrentWorkMode;
    if ((HI_PDT_WORKMODE_SING_PHOTO == enWorkMode) ||
        (HI_PDT_WORKMODE_DLAY_PHOTO == enWorkMode) || (HI_PDT_WORKMODE_LPSE_PHOTO == enWorkMode))/** get from param */
    {
        g_stSTATEMNGContext.stPhotoProTune.u32ExpTime_us = s32SettingProtuneExpTime;
        MLOGD(YELLOW"update ExpTime\n"NONE);
    }
    else if(PDT_STATEMNG_IsVideoPipe(enWorkMode))
    {
        s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
        PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

        s32Ret = HI_PDT_SCENE_SetExpTime(VcapPipeHdl, 0);//s32SettingProtuneExpTime
        PDT_STATEMNG_CHECK_RET(s32Ret,"set scene ExpTime");

        s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
        PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");
    }

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_EXP_TIME, (HI_VOID *)&s32SettingProtuneExpTime);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save ExpTime param");
    MLOGD(YELLOW"save ExpTime(%d) param succeed\n"NONE, s32SettingProtuneExpTime);
    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetEV(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_SCENE_EV_E enSettingProtuneExpEV = *(HI_PDT_SCENE_EV_E *)pvPayload;
    MLOGD(YELLOW"set ExpEV(%d), workmode(%d)\n"NONE, enSettingProtuneExpEV, enSettingWorkMode);

    HI_PDT_SCENE_EV_E enCurrentProtuneExpEV = HI_PDT_SCENE_EV_BUTT;
    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        enCurrentProtuneExpEV = g_stSTATEMNGContext.stVideoProTune.enEV;
    }
    else
    {
        enCurrentProtuneExpEV = g_stSTATEMNGContext.stPhotoProTune.enEV;
    }
    MLOGD(YELLOW"curr ExpEV(%d), workmode(%d)\n"NONE, enCurrentProtuneExpEV, enSettingWorkMode);

    /** filter out the same parameter */
    if(enCurrentProtuneExpEV == enSettingProtuneExpEV)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingProtuneExpEV);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_HANDLE VcapPipeHdl = 0;
    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
    }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    s32Ret = HI_PDT_SCENE_SetEV(VcapPipeHdl, enSettingProtuneExpEV);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set scene EV");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_EXP_EV, (HI_VOID *)&enSettingProtuneExpEV);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save ExpEV param");
    MLOGD(YELLOW"saving ExpEV(%d) param succeed\n"NONE, enSettingProtuneExpEV);

    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        g_stSTATEMNGContext.stVideoProTune.enEV = enSettingProtuneExpEV;
    }
    else
    {
        g_stSTATEMNGContext.stPhotoProTune.enEV = enSettingProtuneExpEV;
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetISO(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingProtuneISO = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set ISO(%d), workmode(%d)\n"NONE, u32SettingProtuneISO, enSettingWorkMode);

    HI_U32 enCurrentProtuneISO = 0;
    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        enCurrentProtuneISO = g_stSTATEMNGContext.stVideoProTune.u32ISO;
    }
    else
    {
        enCurrentProtuneISO = g_stSTATEMNGContext.stPhotoProTune.u32ISO;
    }

    MLOGD(YELLOW"curr ISO(%d), workmode(%d)\n"NONE, enCurrentProtuneISO, enSettingWorkMode);
    /** filter out the same parameter */
    if(enCurrentProtuneISO == u32SettingProtuneISO)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)u32SettingProtuneISO);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_HANDLE VcapPipeHdl = 0;
    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
    }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    HI_PDT_SCENE_SetISO(VcapPipeHdl, u32SettingProtuneISO);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set scene ISO");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_ISO, (HI_VOID *)&u32SettingProtuneISO);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save ISO param");
    MLOGD(YELLOW"save ISO(%d) param succeed\n"NONE, u32SettingProtuneISO);

    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        g_stSTATEMNGContext.stVideoProTune.u32ISO = u32SettingProtuneISO;
    }
    else
    {
        g_stSTATEMNGContext.stPhotoProTune.u32ISO = u32SettingProtuneISO;
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetWB(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_U32 u32SettingProtuneWB = *(HI_U32 *)pvPayload;
    MLOGD(YELLOW"set WB(%d), workmode(%d)\n"NONE, u32SettingProtuneWB, enSettingWorkMode);

    HI_U32 enCurrentProtuneWB = 0;
    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        enCurrentProtuneWB = g_stSTATEMNGContext.stVideoProTune.u32WB;
    }
    else
    {
        enCurrentProtuneWB = g_stSTATEMNGContext.stPhotoProTune.u32WB;
    }

    MLOGD(YELLOW"curr WB(%d\n"NONE, enCurrentProtuneWB);
    /** filter out the same parameter */
    if(enCurrentProtuneWB == u32SettingProtuneWB)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)u32SettingProtuneWB);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n"NONE);
        return HI_FAILURE;
    }

    HI_HANDLE VcapPipeHdl = 0;
    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
    }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    s32Ret = HI_PDT_SCENE_SetWB(VcapPipeHdl, u32SettingProtuneWB);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set scene WB");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_WB, (HI_VOID *)&u32SettingProtuneWB);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save WB param");
    MLOGD(YELLOW"save WB(%d) param succeed\n"NONE, u32SettingProtuneWB);

    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        g_stSTATEMNGContext.stVideoProTune.u32WB = u32SettingProtuneWB;
    }
    else
    {
        g_stSTATEMNGContext.stPhotoProTune.u32WB = u32SettingProtuneWB;
    }

    return s32Ret;
}

HI_S32 PDT_STATEMNG_SetMetry(HI_MESSAGE_S *pstMsg, HI_PDT_MEDIA_CFG_S *pstMediaCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    /** get setting information */
    HI_PDT_WORKMODE_E enSettingWorkMode = pstMsg->arg2;
    HI_VOID* pvPayload = pstMsg->aszPayload;
    HI_PDT_SCENE_METRY_TYPE_E enSettingProtuneMetry = *(HI_PDT_SCENE_METRY_TYPE_E *)pvPayload;
    MLOGD(YELLOW"set Metry(%d), workmode(%d)\n"NONE, enSettingProtuneMetry, enSettingWorkMode);

    HI_PDT_SCENE_METRY_SPOT_PARAM_S stSpotMetryParam;
    if(HI_PDT_SCENE_METRY_TYPE_SPOT == enSettingProtuneMetry)
    {
        memcpy(&stSpotMetryParam,
           (HI_PDT_SCENE_METRY_SPOT_PARAM_S *)((HI_CHAR *)(pstMsg->aszPayload
            + sizeof(HI_PDT_SCENE_METRY_TYPE_E))),
           sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));

        MLOGD(YELLOW"spotmetry: x(%d), y(%d), width(%d), height(%d)\n"NONE,
            stSpotMetryParam.stPoint.s32X,
            stSpotMetryParam.stPoint.s32Y,
            stSpotMetryParam.stSize.u32Width,
            stSpotMetryParam.stSize.u32Height);
    }

    HI_PDT_SCENE_METRY_TYPE_E enCurrentMetry = HI_PDT_SCENE_METRY_TYPE_BUTT;
    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        enCurrentMetry = g_stSTATEMNGContext.stVideoProTune.enMetryType;
    }
    else
    {
        enCurrentMetry = g_stSTATEMNGContext.stPhotoProTune.enMetryType;
    }

    MLOGD(YELLOW"curr Metry(%d)\n"NONE, enCurrentMetry);
    /** filter out the same parameter */
    if(enCurrentMetry == enSettingProtuneMetry && enCurrentMetry != HI_PDT_SCENE_METRY_TYPE_SPOT)
    {
        MLOGD(YELLOW"same value\n"NONE);
        return HI_SUCCESS;
    }

    /** filter out wrong enSettingWorkMode/enSettingPlayloadType pair */
    s32Ret = PDT_STATEMNG_CheckWorkModeSettingItemValue(pstMsg, (HI_S32)enSettingProtuneMetry);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE(RED"set value not support\n\n"NONE);
        return HI_FAILURE;
    }

    /** find video pipe*/
    HI_HANDLE VcapPipeHdl = 0;
    s32Ret = PDT_STATEMNG_GetVideoPipeHdl(&VcapPipeHdl);
    if(HI_SUCCESS != s32Ret)
    {
        s32Ret = PDT_STATEMNG_GetSnapPipeHdl(&VcapPipeHdl);
        PDT_STATEMNG_CHECK_RET(s32Ret,"get vcap pipeHdl");
    }

    HI_PDT_SCENE_METRY_S stMetry; /**type is g_stSTATEMNGContext.stVideoProTune.enMetryType */
    if(HI_PDT_SCENE_METRY_TYPE_CENTER == enSettingProtuneMetry)
    {
        /** call param function to get stMetry */
        MLOGD(YELLOW"construct center metry struct\n"NONE);
        stMetry.enMetryType = HI_PDT_SCENE_METRY_TYPE_CENTER;

        if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
        {
            stMetry.unParam.stCenterMetryParam.u32Index =
                g_stSTATEMNGContext.stVideoProTune.u16MetryCenterParamIdx;
        }
        else
        {
            stMetry.unParam.stCenterMetryParam.u32Index =
                g_stSTATEMNGContext.stPhotoProTune.u16MetryCenterParamIdx;
        }
    }
    else if(HI_PDT_SCENE_METRY_TYPE_AVERAGE == enSettingProtuneMetry)
    {
        MLOGD(YELLOW"construct average metry struct\n"NONE);
        stMetry.enMetryType = HI_PDT_SCENE_METRY_TYPE_AVERAGE;
        if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
        {
            stMetry.unParam.stCenterMetryParam.u32Index =
                g_stSTATEMNGContext.stVideoProTune.u16MetryAverageParamIdx;
        }
        else
        {
            stMetry.unParam.stCenterMetryParam.u32Index =
                g_stSTATEMNGContext.stPhotoProTune.u16MetryAverageParamIdx;
        }
    }
    else if(HI_PDT_SCENE_METRY_TYPE_SPOT == enSettingProtuneMetry)
    {
        MLOGD(YELLOW"construct spot metry struct\n"NONE);
        stMetry.enMetryType = HI_PDT_SCENE_METRY_TYPE_SPOT;
        memcpy(&(stMetry.unParam.stSpotMetryParam),
            &(stSpotMetryParam),
            sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));
    }

    s32Ret = HI_PDT_SCENE_Pause(HI_TRUE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"pause scene");

    s32Ret = HI_PDT_SCENE_SetMetry(VcapPipeHdl, &stMetry);
    PDT_STATEMNG_CHECK_RET(s32Ret,"set scene metry");

    s32Ret = HI_PDT_SCENE_Pause(HI_FALSE);
    PDT_STATEMNG_CHECK_RET(s32Ret,"resume scene");

    /** save param to Param module */
    s32Ret = HI_PDT_PARAM_SetWorkModeParam(enSettingWorkMode,
        HI_PDT_PARAM_TYPE_PROTUNE_METRY, (HI_VOID *)&enSettingProtuneMetry);
    PDT_STATEMNG_CHECK_RET(s32Ret,"save Metry param");
    MLOGD(YELLOW"save Metry(%d) param succeed\n"NONE, enSettingProtuneMetry);

    if(PDT_STATEMNG_IsVideoPipe(enSettingWorkMode))
    {
        g_stSTATEMNGContext.stVideoProTune.enMetryType = enSettingProtuneMetry;
    }
    else
    {
        g_stSTATEMNGContext.stPhotoProTune.enMetryType = enSettingProtuneMetry;
    }

    if(HI_PDT_SCENE_METRY_TYPE_SPOT == enSettingProtuneMetry)
    {
        MLOGD(YELLOW"update context spotmetry\n"NONE);
        memcpy(&(g_stSTATEMNGContext.stSpotMetryParam),
           &stSpotMetryParam,
           sizeof(HI_PDT_SCENE_METRY_SPOT_PARAM_S));
    }

    return s32Ret;
}

#ifdef CONFIG_RAWCAP_ON
static HI_VOID PDT_STATEMNG_GetVcapPipeResolution(HI_U32 *pu32Width, HI_U32 *pu32Height)
{
    HI_HANDLE VcapPipeHdl = g_stSTATEMNGContext.stRawCapCfg.stDataSource.VcapPipeHdl;
    HI_PDT_MEDIA_VCAP_CFG_S*  pstVcapCfg = &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg);

    HI_PDT_MEDIA_VCAP_DEV_ATTR_S*  pstVcapDevAttr = NULL;
    HI_PDT_MEDIA_VCAP_PIPE_ATTR_S* pstVcapPipeAttr = NULL;
    HI_S32 i = 0, j = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; i++)
    {
        pstVcapDevAttr = &(pstVcapCfg->astVcapDevAttr[i]);

        if(HI_TRUE == pstVcapDevAttr->bEnable)
        {
            for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; j++)
            {
                pstVcapPipeAttr = &(pstVcapDevAttr->astVcapPipeAttr[j]);

                if((HI_TRUE == pstVcapPipeAttr->bEnable) &&
                   (pstVcapPipeAttr->VcapPipeHdl == VcapPipeHdl))
                {
                    *pu32Width = pstVcapPipeAttr->stIspPubAttr.stSize.u32Width;
                    *pu32Height = pstVcapPipeAttr->stIspPubAttr.stSize.u32Height;
                    MLOGD("find VcapPipeHdl(%d):width(%d), height(%d)\n",
                        VcapPipeHdl, *pu32Width, *pu32Height);
                    break;
                }
            }
        }
    }
}

static HI_VOID PDT_STATEMNG_GetYuvResolution(HI_U32 *pu32Width, HI_U32 *pu32Height)
{
    HI_S32 i = 0, j = 0;

    HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
    HI_PDT_MEDIA_VPSS_PORT_ATTR_S* pstVportAttr = NULL;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; i++)
    {
        pstVpssAttr =
            &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[i]);
        if((HI_TRUE == pstVpssAttr->bEnable) &&
           (pstVpssAttr->VpssHdl == g_stSTATEMNGContext.stRawCapCfg.stDataSource.VpssHdl))
        {
            MLOGD("find VpssHdl(%d)\n", pstVpssAttr->VpssHdl);
            for(j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; j++)
            {
                pstVportAttr =
                    &(pstVpssAttr->astVportAttr[j]);
                if((HI_TRUE == pstVportAttr->bEnable) &&
                   (pstVportAttr->VportHdl == g_stSTATEMNGContext.stRawCapCfg.stDataSource.VPortHdl))
                {
                    *pu32Width = pstVportAttr->stResolution.u32Width;
                    *pu32Height = pstVportAttr->stResolution.u32Height;
                    MLOGD(" vpss resolution(%dx%d)\n", *pu32Width, *pu32Height);
                    break;
                }
            }
        }
    }
}

static HI_S32 PDT_STATEMNG_GetRawFileName(HI_RAWCAP_FILENAME_S *pstFileName)
{
    HI_U32          u32Width = 0;
    HI_U32          u32Height = 0;
    PDT_STATEMNG_GetVcapPipeResolution(&u32Width, &u32Height);
    PIXEL_FORMAT_E  enPixelFormat = 0;
#if 0
    HI_MAPI_VCAP_EXPOSURE_INFO_S stExpInfo;
    s32Ret = HI_MAPI_VCAP_GetAttrEx(
        g_stSTATEMNGContext.stRawCapCfg.stDataSource.VcapPipeHdl,
        g_stSTATEMNGContext.stRawCapCfg.stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_GetExposureInfo,
        &stExpInfo, sizeof(HI_MAPI_VCAP_EXPOSURE_INFO_S));
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif

    HI_CHAR szRawFormat[10] = {0};
#if 0
    /** get ISP PubAttr for RawFormat(RGGB,GRBG,GBRG,BGGR)*/
    s32Ret = HI_MAPI_VCAP_GetAttrEx(
        g_stSTATEMNGContext->stRawCapCfg.stDataSource.VcapPipeHdl,
        g_stSTATEMNGContext->stRawCapCfg.stDataSource.VcapPipeChnHdl,
        HI_MAPI_VCAP_CMD_ISP_PubAttr,
        &stIspPubAttr, sizeof(stIspPubAttr));
#else
    snprintf(szRawFormat,sizeof(szRawFormat),"%s","NA");
#endif

    HI_S32 s32FrameBits;
    switch(enPixelFormat)
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
            MLOGD("PixelFormat(%d) err\n", enPixelFormat);
            s32FrameBits = 16;
    }

    if(WDR_MODE_NONE == g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVcapCfg.astVcapDevAttr[0].enWdrMode)
    {
        snprintf(pstFileName->szRaw_FileName,
            HI_RAWCAP_MAX_FILENAME_LEN,
            "%s_%dx%d_%dbits_%s_Linear",
            g_stSTATEMNGContext.szPrefix,
            u32Width,
            u32Height,
            s32FrameBits,
            szRawFormat);
    }
    else
    {
        snprintf(pstFileName->szRaw_FileName,
            HI_RAWCAP_MAX_FILENAME_LEN,
            "%s_%dx%d_%dbits_%s_WDR",
            g_stSTATEMNGContext.szPrefix,
            u32Width,
            u32Height,
            s32FrameBits,
            szRawFormat);
    }

    return HI_SUCCESS;
}

HI_VOID PDT_STATEMNG_GetRawCapTaskSrc(HI_PDT_WORKMODE_E enWorkmode,
    HI_RAWCAP_DATA_SOURCE_S *pstDataSource)
{
    HI_HANDLE VencHdl = PDT_STATEMNG_INVALID_HANDLE;
    if(HI_PDT_WORKMODE_SING_PHOTO == enWorkmode)
    {
        VencHdl = PDT_STATEMNG_PHOTO_VENC_HDL;
    }
    else if(HI_PDT_WORKMODE_NORM_REC == enWorkmode)
    {
        VencHdl = PDT_STATEMNG_MAIN_STREAM_VENC_HDL;
    }

    HI_S32 s32Idx;
    HI_PDT_MEDIA_VENC_CFG_S* pstVencCfg = NULL;
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VENC_MAX_CNT; ++s32Idx)
    {
        pstVencCfg = &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.astVencCfg[s32Idx]);
        if((HI_TRUE == pstVencCfg->bEnable) &&
           (pstVencCfg->VencHdl == VencHdl))
        {
            MLOGD("find VencHdl(%d)\n", VencHdl);
            if(HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencCfg->enBindedMod)/** venc bind vcap */
            {
                pstDataSource->VcapPipeHdl = pstVencCfg->ModHdl;
                pstDataSource->VcapPipeChnHdl = pstVencCfg->ChnHdl;
                pstDataSource->bDumpYUV = HI_FALSE;
            }
            else if(HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencCfg->enBindedMod)/** venc bind vpss */
            {
                pstDataSource->VpssHdl = pstVencCfg->ModHdl;
                pstDataSource->VPortHdl = pstVencCfg->ChnHdl;
                pstDataSource->bDumpYUV = HI_TRUE;
            }
            break;
        }
    }

    HI_PDT_MEDIA_VPSS_ATTR_S* pstVpssAttr = NULL;
    if(HI_TRUE == pstDataSource->bDumpYUV)
    {
        for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_VPSS_MAX_CNT; s32Idx++)
        {
            pstVpssAttr =
                &(g_stSTATEMNGContext.stMediaCfg.stVideoCfg.stVprocCfg.astVpssAttr[s32Idx]);
            if((HI_TRUE == pstVpssAttr->bEnable) &&
               (pstVpssAttr->VpssHdl == pstDataSource->VpssHdl))
            {
               MLOGD("find VpssHdl(%d)\n", pstDataSource->VpssHdl);
               pstDataSource->VcapPipeHdl = pstVpssAttr->VcapPipeHdl;
               pstDataSource->VcapPipeChnHdl = pstVpssAttr->VcapPipeChnHdl;
               break;
            }
        }
    }
}

HI_S32 PDT_STATEMNG_GetRawCapFileNames(HI_RAWCAP_FILENAME_S *pstFilename, HI_VOID* pvPrivateData)
{
    memset(pstFilename, 0, sizeof(HI_RAWCAP_FILENAME_S));

    if (HI_NULL == pstFilename)
    {
        MLOGE(RED"pstFilename(null)\n"NONE);
        return HI_FAILURE;
    }

    HI_RAWCAP_FILENAME_S stFileName;
    memset(&stFileName, 0, sizeof(HI_RAWCAP_FILENAME_S));

    PDT_STATEMNG_GetRawFileName(&stFileName);

    HI_U32 u32Width = 0;
    HI_U32 u32Height =  0;
    PDT_STATEMNG_GetYuvResolution(&u32Width, &u32Height);

    snprintf(stFileName.szYUV_FileName, HI_RAWCAP_MAX_FILENAME_LEN, "%s_%dx%d",
        g_stSTATEMNGContext.szPrefix,
        u32Width, u32Height);

    snprintf(stFileName.szDebugInfo_FileName, HI_RAWCAP_MAX_FILENAME_LEN, "%s",
        g_stSTATEMNGContext.szPrefix);

    memcpy(pstFilename, &stFileName, sizeof(HI_RAWCAP_FILENAME_S));
    return HI_SUCCESS;
}
#endif

HI_VOID PDT_STATEMNG_SetSysConfig()
{
    HI_S32 s32Ret = HI_SUCCESS;
    /* Get Storage Status */
    HI_STORAGE_STATE_E enState = HI_STORAGE_STATE_IDEL;
    s32Ret = HI_STORAGEMNG_GetState(g_stSTATEMNGContext.stStorageMngCfg.szMntPath, &enState);
    if(s32Ret != HI_SUCCESS || HI_STORAGE_STATE_MOUNTED != enState)
    {
        return;
    }

    HI_CHAR* pSysConfig = NULL;
    /* Set System Configure for different workmode */
    if ((HI_PDT_WORKMODE_NORM_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_LOOP_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_LPSE_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_SLOW_REC == g_stSTATEMNGContext.enCurrentWorkMode) ||
        (HI_PDT_WORKMODE_RECSNAP == g_stSTATEMNGContext.enCurrentWorkMode))
    {
        pSysConfig = "/app/rec_sys_config.sh";
    }
    else
    {
        pSysConfig = "/app/photo_sys_config.sh";
    }

#if 0
    pid_t pid = 0;
    HI_CHAR* argv[] = {pSysConfig, NULL};
    HI_CHAR* env[]  = {NULL};
    s32Ret = posix_spawn(&pid, argv[0], NULL, NULL, argv, env);
    HI_S32 stat = 0;
    pid = waitpid(pid, &stat, 0);
    if(HI_SUCCESS != s32Ret)
    {
        perror("sys_config.sh err\n");
    }
#else
    HI_CHAR cmd[HI_APPCOMM_MAX_PATH_LEN]={'\0'};
    strncat(cmd, pSysConfig,HI_APPCOMM_MAX_PATH_LEN);
    s32Ret = HI_system(cmd);
    if(s32Ret == HI_FAILURE)
    {
        if(ECHILD != errno)
        {
            MLOGE("system errno(%d)\n",errno);
        }
    }
#endif
    return;
}

HI_S32 PDT_STATEMNG_InitFileMng()
{
    HI_S32 s32Ret = HI_SUCCESS;
    /** init filemng */
    HI_PDT_FILEMNG_CFG_S stFileMngCfg;
    s32Ret = HI_PDT_PARAM_GetFileMngCfg(&stFileMngCfg);
    PDT_STATEMNG_CHECK_RET(s32Ret, "GetFileMngCfg");

    HI_STORAGE_FS_INFO_S stFSInfo;
    s32Ret = HI_STORAGEMNG_GetFSInfo(g_stSTATEMNGContext.stStorageMngCfg.szMntPath, &stFSInfo);
    PDT_STATEMNG_CHECK_RET(s32Ret, "HI_STORAGEMNG_GetFSInfo");

#if defined(CONFIG_FILEMNG_DTCF)
    stFileMngCfg.stDtcfCfg.u32WarningStage =
        stFSInfo.u64TotalSzie * stFileMngCfg.stDtcfCfg.u32WarningStage/100 >> 20;
    stFileMngCfg.stDtcfCfg.u32GuaranteedStage =
        stFSInfo.u64TotalSzie * stFileMngCfg.stDtcfCfg.u32GuaranteedStage/100 >> 20;
    MLOGI(BLUE"DTCF: WarningStage[%u] GuaranteedStage[%u]\n"NONE,
        stFileMngCfg.stDtcfCfg.u32WarningStage, stFileMngCfg.stDtcfCfg.u32GuaranteedStage);
#else
    stFileMngCfg.stDcfCfg.u32WarningStage =
        stFSInfo.u64TotalSize * stFileMngCfg.stDcfCfg.u32WarningStage/100 >> 20;
    MLOGI(BLUE"DCF: WarningStage[%u]\n"NONE,stFileMngCfg.stDcfCfg.u32WarningStage);
#endif

    s32Ret = HI_FILEMNG_Init(&stFileMngCfg.stCommCfg, &stFileMngCfg.stDcfCfg);
    if(HI_FILEMNG_EINITIALIZED == s32Ret)
    {
        MLOGI(BLUE"FileMng already inited\n"NONE);
        s32Ret = HI_SUCCESS;
    }
    else
    {
        PDT_STATEMNG_CHECK_RET(s32Ret, "HI_FILEMNG_Init");
    }

    /**Improving sd writing performance*/
    s32Ret = HI_system("echo 256 > /sys/block/mmcblk0/queue/nr_requests");
    if(HI_SUCCESS != s32Ret)
    {
        MLOGI(BLUE"set nr_requests fail\n"NONE);
        s32Ret = HI_SUCCESS;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
