/**
 * @file      hi_product_iniparam_workmode.c
 * @brief     load workmode parameter
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hi_product_iniparam.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

static HI_S32 PDT_INIPARAM_WORKMODE_LoadCommCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_COMM_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:poweron_workmode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enPoweronWorkMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("poweron_workmode[%d; 0:normal record, 1:photo]\n", pstParam->enPoweronWorkMode);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadMediaModeCfg(const HI_CHAR *pszIniModule,const HI_CHAR *pszNodePrefix,
                HI_PDT_PARAM_MEDIAMODE_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = HI_NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:CAMID",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->s32CamID);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mediamode",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = HI_PDT_INIPARAM_MEDIAMODE_GetMediaModeEnum(pszString, &pstParam->enMediaMode);
        HI_APPCOMM_SAFE_FREE(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    MLOGD("%s,CAMID[%d] mediamode[%d]\n",pszNodePrefix, pstParam->s32CamID, pstParam->enMediaMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoSrc(const HI_CHAR *pszIniModule,const HI_CHAR *pszNodePrefix,
                HI_PDT_PARAM_PHOTO_SRC_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vcappipehdl",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:pipechnhdl",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VcapPipeChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vpsshdl",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VpssHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:vporthdl",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:venchdl",pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("photo source enable[%d] vcappipehdl[%d],pipechnhdl[%d],vpsshdl[%d],vporthdl[%d],venchdl[%d] \n",
        pstParam->bEnable,pstParam->VcapPipeChnHdl,pstParam->VcapPipeHdl,pstParam->VpssHdl,pstParam->VportHdl,pstParam->VencHdl);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_REC_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    MLOGD("Record mediamode Configure:\n");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        /* Load Record mediamode Configure */
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "common.mediamode.config.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadMediaModeCfg(pszIniModule,szNodePrefix,&pstParam->astMediaModeCfg[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szNodePrefix);

        /* Load photo source */
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "photosource.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoSrc(pszIniModule, szNodePrefix, &pstParam->stPhotoSrc[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoSrc");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPhotoMediaCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_WORKMODE_PHOTO_MEDIA_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Photo mediamode Configure */
    MLOGD("Photo mediamode Configure:\n");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "common.mediamode.config.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadMediaModeCfg(pszIniModule,szNodePrefix,&pstParam->astMediaModeCfg[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szNodePrefix);
    }

   /* Load photo source */
    MLOGD("Photo PhotoSrc\n");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "photosource.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoSrc(pszIniModule, szNodePrefix, &pstParam->stPhotoSrc[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoSrc");
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecCommCfg(const HI_CHAR *pszIniModule,
    PDT_PARAM_REC_COMM_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "record.common:muxer_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRecMuxerType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "record.common:thm_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->enRecThmType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "record.common:repaire_unit");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,(HI_S32*)&pstParam->u32RepairUnit);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "record.common:pre_alloc_unit");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PreAllocUnit);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("record comm  muxer_type[%d,0-mp4,1-mov,2-ts ], thm_type[%d, 0-INDEPENDENCE,1-EMBEDDED,2-NONE],repaire_unit[%u],pre_alloc_unit[%u] \n",
        pstParam->enRecMuxerType,pstParam->enRecThmType,pstParam->u32RepairUnit,pstParam->u32PreAllocUnit);

    return HI_SUCCESS;
}


static HI_S32 PDT_INIPARAM_WORKMODE_LoadRecFileAttr(const HI_CHAR *pszIniModule,HI_CHAR *pszNodePrefix,
                HI_RECMNG_FILE_DATA_SOURCE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:venccnt", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32VencCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("%s venccnt[%d] \n",pszNodePrefix,pstParam->u32VencCnt);

    for(i = 0 ;i < pstParam->u32VencCnt && i < HI_RECMNG_FILE_MAX_VIDEO_TRACK_CNT;i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:venc_hdl%d", pszNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->aVencHdl[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("venc_hdl%d[%d] \n",i,pstParam->aVencHdl[i]);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aenccnt", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32AencCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("%s aenccnt[%d] \n",pszNodePrefix,pstParam->u32AencCnt);

    for(i = 0 ;i < pstParam->u32AencCnt && i < HI_RECMNG_FILE_MAX_AUDIO_TRACK_CNT;i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aenc_hdl%d", pszNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->aAencHdl[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("aenc_hdl%d[%d] \n",i,pstParam->aAencHdl[i]);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:thumbnail_hdl", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->ThmHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("%s thumbnail_hdl[%d] \n",pszNodePrefix,pstParam->ThmHdl);

    return HI_SUCCESS;
}


static HI_S32 PDT_INIPARAM_WORKMODE_LoadNormRecAttr(const HI_CHAR *pszIniModule,HI_CHAR *pszNodePrefix,
                PDT_PARAM_NORM_REC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load Record Norm attr */
    MLOGD("normal record attr :\n");

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:split_type", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.stSplitAttr.enSplitType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:split_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.stSplitAttr.u32SplitTimeLenSec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("normal record attr  enable[%d],split_type[%d],split_time[%d] \n",pstParam->stRecMngAttr.bEnable,
        pstParam->stRecMngAttr.stSplitAttr.enSplitType,pstParam->stRecMngAttr.stSplitAttr.u32SplitTimeLenSec);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:recbuffer_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.u32BufferTime_ms);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:prerec_time", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.u32PreRecTimeSec);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:filecnt", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.u32FileCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:thmcnt", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.stThmAttr.u32ThmCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("normal record attr  recbuffer_time[%d],prerec_time[%dms],filecnt[%d],thmcnt[%d] \n",pstParam->stRecMngAttr.u32BufferTime_ms,
        pstParam->stRecMngAttr.u32PreRecTimeSec,pstParam->stRecMngAttr.u32FileCnt,pstParam->stRecMngAttr.stThmAttr.u32ThmCnt);

    for(i = 0;i < pstParam->stRecMngAttr.stThmAttr.u32ThmCnt;i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:thmhdl%d", pszNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.stThmAttr.aThmHdl[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("thmhdl%d[%d] \n",i,pstParam->stRecMngAttr.stThmAttr.aThmHdl[i]);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:playfps", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.s32PlayFps);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("playfps[%d] \n",pstParam->stRecMngAttr.s32PlayFps);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:record_type", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.enRecType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:lapse_interval", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.u32Interval_ms);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    //MLOGD("record comm:record type[%d], interval[%d]\n", pstParam->stRecMngAttr.enRecType, pstParam->stRecMngAttr.u32Interval_ms);
    for(i = 0; i < pstParam->stRecMngAttr.u32FileCnt && i < HI_PDT_RECTASK_FILE_MAX_CNT; i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.filenum.%d.fileattr", pszNodePrefix,i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadRecFileAttr(pszIniModule, szNodePrefix, &pstParam->stRecMngAttr.astRecFileDataSrc[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "fileattr");

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.filenum.%d.fileattr:recvbuf_size", pszNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRecMngAttr.au32VBufSize[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("recvbuf_size%d[%d] \n",i,pstParam->stRecMngAttr.au32VBufSize[i]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszNodePrefix, HI_UVC_FORMAT_CAP_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_U32  u32Idx;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:count", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Cnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (HI_UVC_VIDEOMODE_BUTT < pstParam->u32Cnt)
    {
        MLOGE("Invalid Cap Count[%u]\n", pstParam->u32Cnt);
        return HI_FAILURE;
    }
    MLOGD("Prefix[%s] Cap Count[%u]\n", pszNodePrefix, pstParam->u32Cnt);

    for (u32Idx = 0; u32Idx < pstParam->u32Cnt; ++u32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_mode%u", pszNodePrefix, u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->astModes[u32Idx].enMode);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_bitrate%u", pszNodePrefix, u32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->astModes[u32Idx].u32BitRate);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("Cap[%u] Mode[%d] BitRate[%d]\n", u32Idx, pstParam->astModes[u32Idx].enMode,pstParam->astModes[u32Idx].u32BitRate);
    }

    /* default videomode */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:video_defmode", pszNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enDefMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Default Mode[%d]\n", pstParam->enDefMode);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCDataSrcCfg(const HI_CHAR *pszIniModule,
                HI_UVC_DATA_SOURCE_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:vproc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VprocHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:vport_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VportHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:venc_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.datasource:acap_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("vprochdl[%d] vporthdl[%d] venchdl[%d] acaphdl[%d]\n",
        pstParam->VprocHdl, pstParam->VportHdl, pstParam->VencHdl, pstParam->AcapHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadPlayBackCfg(const HI_CHAR *pszIniModule,
                HI_PDT_PARAM_PLAYBACK_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* load playback mediamode */
    MLOGD("playback mediamode Configure:\n");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "playback.mediamode.config.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadMediaModeCfg(pszIniModule,szNodePrefix,&pstParam->astMediaModeCfg[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szNodePrefix);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:interval_ms");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 500,
        (HI_S32*)&pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:video_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32VideoEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:audio_buff");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.u32AudioEsBufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.player.config:pause_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stPlayerParam.bPauseMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("player config interval_ms[%d] video_buff[%d] audio_buff[%d] pause_mode[%d]\n",
        pstParam->stPlayerParam.u32PlayPosNotifyIntervalMs,
        pstParam->stPlayerParam.u32VideoEsBufSize,
        pstParam->stPlayerParam.u32AudioEsBufSize,
        pstParam->stPlayerParam.bPauseMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:vout_type");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.enVoutType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:mod_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.vout.opt:chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stVoutOpt.hChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("VoutOpt Type[%d:0-vpss,1-vo] ModHdl[%d] ChnHdl[%d]\n",
        pstParam->stVoutOpt.enVoutType, pstParam->stVoutOpt.hModHdl, pstParam->stVoutOpt.hChnHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.aout.opt:dev_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudDevHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "playback.aout.opt:chn_hdl");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stAoutOpt.hAudTrackHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("AoutOpt AoDev[%d] AudTrackHdl[%d]\n", pstParam->stAoutOpt.hAudDevHdl,pstParam->stAoutOpt.hAudTrackHdl);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUsbCommCfg(const HI_CHAR *pszIniModule,
    PDT_PARAM_USB_COMM_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "common:usb_mode");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enUsbMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("usb comm  usb_mode[%d,0-charge,1-uvc,2-storage ] \n",pstParam->enUsbMode);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUVCCfg(const HI_CHAR *pszIniModule,
                PDT_PARAM_UVC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    /* load uvc mediamode */
    MLOGD("uvc mediamode Configure:\n");
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "uvc.mediamode.config.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadMediaModeCfg(pszIniModule,szNodePrefix,&pstParam->astMediaModeCfg[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szNodePrefix);
    }

    /* Load Device Path */
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc:dev_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->stUvcCfg.szDevPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        HI_APPCOMM_SAFE_FREE(pszString);
    }
    MLOGD("DevPath[%s]\n", pstParam->stUvcCfg.szDevPath);

    /* Load YUV420 Cap Configure */
    MLOGD("YUV420 Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.yuv420",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_YUV420]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "YUV420Cap");

    /* Load MJPEG Cap Configure */
    MLOGD("MJPEG Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.mjpeg",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_MJPEG]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MJPEGCap");

    /* Load H264 Cap Configure */
    MLOGD("H264 Cap:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCapCfg(pszIniModule, "uvc.capability.h264",
        &pstParam->stUvcCfg.stDevCap.astFmtCaps[HI_UVC_STREAM_FORMAT_H264]);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264Cap");

    /* Load Data Source */
    MLOGD("Data Source:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCDataSrcCfg(pszIniModule, &pstParam->stUvcCfg.stDataSource);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "DataSource");

    /*Load UVC Buffer */
    MLOGD("UVC Buffer:\n");
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.buffer:buffer_size");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM,pszIniModule,szIniNodeName,0, (HI_S32*)&pstParam->stUvcCfg.stBufferCfg.u32BufSize);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("buffer_size [%d]\n", pstParam->stUvcCfg.stBufferCfg.u32BufSize);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "uvc.buffer:buffer_count");
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName,0, (HI_S32*)&pstParam->stUvcCfg.stBufferCfg.u32BufCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("buffer_count [%d]\n", pstParam->stUvcCfg.stBufferCfg.u32BufCnt);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_WORKMODE_LoadUsbStorageCfg(const HI_CHAR *pszIniModule,
                HI_USB_STORAGE_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:dev_path");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szDevPath, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        HI_APPCOMM_SAFE_FREE(pszString);
    }
    MLOGD("DevPath[%s]\n", pstParam->szDevPath);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:sysfile");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szSysFile, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("SysFile[%s]\n", pstParam->szSysFile);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "usb.storage:usb_state_proc");
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        snprintf(pstParam->szProcFile, HI_APPCOMM_MAX_PATH_LEN, "%s", pszString);
        free(pszString);
    }
    MLOGD("ProcFile[%s]\n", pstParam->szProcFile);

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadWorkModeConf(PDT_PARAM_WORKMODE_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);

    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    /* Load WorkMode Common Configure */
    MLOGD("WorkMode Common Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_COMM);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadCommCfg(szIniModuleName, &pstParam->stCommCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Common");
    MLOGD("\n\n");

    /* Load WorkMode Photo Media Configure */
    MLOGD("WorkMode Photo Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PHOTO);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPhotoMediaCfg(szIniModuleName, &pstParam->stPhotoMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PhotoMedia");
    MLOGD("\n\n");

    /* Load WorkMode Record Media Configure */
    MLOGD("WorkMode Record Media Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_RECORD);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecMediaCfg(szIniModuleName, &pstParam->stRecMediaCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecMedia");
    MLOGD("\n\n");

    /*load record common configure*/
    s32Ret = PDT_INIPARAM_WORKMODE_LoadRecCommCfg(szIniModuleName,&pstParam->stRecCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "RecComm");
    MLOGD("\n\n");

    /*load normal record configure*/
    for(i = 0; i < HI_PDT_RECTASK_MAX_CNT;i++)
    {
        snprintf(szNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "normal.%d", i);
        s32Ret = PDT_INIPARAM_WORKMODE_LoadNormRecAttr(szIniModuleName,szNodePrefix,&pstParam->astNormRecAttr[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Normal record");
    }
    MLOGD("\n\n");

    /* Load PLAYBACK Configure */
    MLOGD("PLAYBACK Configure:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_PLAYBACK);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadPlayBackCfg(szIniModuleName, &pstParam->stPlayBackCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "PLAYBACK");
    MLOGD("\n\n");

    /* Load USB Configure */
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_USB);
    /* Load USB common configure */
    MLOGD("usb common Configure:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUsbCommCfg(szIniModuleName,&pstParam->stUsbCommAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "UsbComm");
    MLOGD("\n\n");

    /* Load UVC Configure */
    MLOGD("UVC Configure:\n");
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUVCCfg(szIniModuleName, &pstParam->stUvcCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "UVC");
    MLOGD("\n\n");

    /* Load USB Storage Configure */
    MLOGD("USB Storage:\n");
    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_WORKMODE_USB);
    s32Ret = PDT_INIPARAM_WORKMODE_LoadUsbStorageCfg(szIniModuleName, &pstParam->stUsbStorageCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "UsbStorage");
    MLOGD("\n\n");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

