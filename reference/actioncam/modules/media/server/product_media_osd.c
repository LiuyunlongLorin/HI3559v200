/**
 * @file      product_media_osd.c
 * @brief     product media osd source
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/3/17
 * @version   1.0

 */

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "mpi_ae.h"
#include "mpi_isp.h"
#include "mpi_awb.h"

#include "hi_mapi_sys.h"
#include "hi_mapi_vcap_define.h"
#include "hi_mapi_venc_define.h"
#include "hi_product_media.h"
#include "product_media_inner.h"
#include "hi_osd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** OSD Based Resolutin */
#define PDT_MEDIA_OSD_BASE_RES_W          (1280)
#define PDT_MEDIA_OSD_BASE_RES_H          (720)

/** Maximum TimeOSD String Length */
#define PDT_MEDIA_MAX_TIME_OSD_LEN        (19)
/** Maximum InfoOSD String Length */
#define PDT_MEDIA_MAX_INFO_OSD_LEN        (64)

/** OSD Handle */
#define PDT_MEDIA_OSD_TIME_HDL            (0)
#define PDT_MEDIA_OSD_INFO_HDL            (2)

/** PQ Debug Info OSD Posotion in percent */
#define PDT_MEDIA_OSD_INFO_START_X        (1)
#define PDT_MEDIA_OSD_INFO_START_Y        (90)

/*-------------------------------------------*/

typedef struct tagPDT_MEDIA_OSD_IDX_NODE_S
{
    HI_S8 s8TimeIdx;
    HI_S8 s8InfoIdx;
} PDT_MEDIA_OSD_IDX_NODE_S;

typedef struct tagPDT_MEDIA_OSD_IDX_S
{
    PDT_MEDIA_OSD_IDX_NODE_S astVcapPipeChnOsdIdx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT][HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT][HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT];
    PDT_MEDIA_OSD_IDX_NODE_S astVpssPortOsdIdx[HI_PDT_MEDIA_VPSS_MAX_CNT][HI_PDT_MEDIA_VPSS_PORT_MAX_CNT];
    PDT_MEDIA_OSD_IDX_NODE_S astVencOsdIdx[HI_PDT_MEDIA_VENC_MAX_CNT];
} PDT_MEDIA_OSD_IDX_S;

typedef struct tagPDT_MEDIA_OSD_CTX_S
{
    HI_BOOL bInit;
    PDT_MEDIA_OSD_IDX_S stOsdIdx;
    HI_PDT_MEDIA_OSD_VIDEO_INFO_S stVideoInfo;
#if defined(CONFIG_PQT_SUPPORT_ON)
    HI_BOOL bInfoTskRun;
    pthread_t InfoTskId;
#endif
} PDT_MEDIA_OSD_CTX_S;
static PDT_MEDIA_OSD_CTX_S s_stMEDIAOsdCtx;

static inline HI_VOID PDT_MEDIA_OSD_CalcFontSize(const SIZE_S* pstBaseFontSize,
                                                 const SIZE_S* pstResoution, SIZE_S* pstFontSize)
{
    pstFontSize->u32Width = pstBaseFontSize->u32Width * pstResoution->u32Width / PDT_MEDIA_OSD_BASE_RES_W;
    pstFontSize->u32Width = HI_APPCOMM_ALIGN(pstFontSize->u32Width, 2);
    pstFontSize->u32Height = pstBaseFontSize->u32Height * pstResoution->u32Height / PDT_MEDIA_OSD_BASE_RES_H;
    pstFontSize->u32Height = HI_APPCOMM_ALIGN(pstFontSize->u32Height, 2);
}

static HI_VOID PDT_MEDIA_OSD_GenerateTimeOsdAttr(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg,
                        const SIZE_S* pstResoution, HI_U32 u32DispNum,
                        HI_OSD_ATTR_S* pstOsdAttr)
{
    /* content */
    pstOsdAttr->stContent.enType = HI_OSD_TYPE_TIME;
    pstOsdAttr->stContent.enTimeFmt = pstOsdCfg->stTimeOsd.enFormat;
    pstOsdAttr->stContent.u32Color = pstOsdCfg->stTimeOsd.u32Color;
    pstOsdAttr->stContent.u32BgColor = pstOsdCfg->stTimeOsd.u32BgColor;
    PDT_MEDIA_OSD_CalcFontSize(&pstOsdCfg->stTimeOsd.stFontSize, pstResoution, &pstOsdAttr->stContent.stFontSize);
    MLOGD("Res[%ux%u] FontSize[%ux%u] Color[%#x] BgColor[%#x]\n",
        pstResoution->u32Width, pstResoution->u32Height,
        pstOsdAttr->stContent.stFontSize.u32Width, pstOsdAttr->stContent.stFontSize.u32Height,
        pstOsdAttr->stContent.u32Color, pstOsdAttr->stContent.u32BgColor);

    /* display attribute */
    HI_U32 i;
    pstOsdAttr->u32DispNum = u32DispNum;
    for (i = 0; i < u32DispNum; ++i)
    {
        pstOsdAttr->astDispAttr[i].bShow = pstOsdCfg->stTimeOsd.bShow;
        pstOsdAttr->astDispAttr[i].enBindedMod = pstOsdCfg->enBindMod;
        pstOsdAttr->astDispAttr[i].u32FgAlpha = pstOsdCfg->stTimeOsd.u32FgAlpha;
        pstOsdAttr->astDispAttr[i].u32BgAlpha = pstOsdCfg->stTimeOsd.u32BgAlpha;
        pstOsdAttr->astDispAttr[i].enCoordinate = HI_OSD_COORDINATE_ABS_COOR;
        pstOsdAttr->astDispAttr[i].stStartPos.s32X =
            pstResoution->u32Width * pstOsdCfg->stTimeOsd.stStartPos.s32X / 100;
        pstOsdAttr->astDispAttr[i].stStartPos.s32X =
        HI_APPCOMM_ALIGN(pstOsdAttr->astDispAttr[i].stStartPos.s32X, 2);
        pstOsdAttr->astDispAttr[i].stStartPos.s32Y =
            pstResoution->u32Height * pstOsdCfg->stTimeOsd.stStartPos.s32Y / 100;
        pstOsdAttr->astDispAttr[i].stStartPos.s32Y =
        HI_APPCOMM_ALIGN(pstOsdAttr->astDispAttr[i].stStartPos.s32Y, 2);

        MLOGD("  Show[%d] BindedMod[%d:0-vcap,1-vpss] FgAlpha[%x] BgAlpha[%x] Pos[%u,%u]\n",
            pstOsdAttr->astDispAttr[i].bShow, pstOsdAttr->astDispAttr[i].enBindedMod,
            pstOsdAttr->astDispAttr[i].u32FgAlpha, pstOsdAttr->astDispAttr[i].u32BgAlpha,
            pstOsdAttr->astDispAttr[i].stStartPos.s32X, pstOsdAttr->astDispAttr[i].stStartPos.s32Y);
    }
}

static HI_S32 PDT_MEDIA_OSD_SetVcapTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j, k;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVideoInfo->astVcapDevInfo[i].bEnable)
        {
            continue;
        }
        for (j = 0;  j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            const HI_PDT_MEDIA_OSD_VCAP_PIPE_INFO_S* pstPipeInfo =
                &pstVideoInfo->astVcapDevInfo[i].astPipeInfo[j];
            if (!pstPipeInfo->bEnable)
            {
                continue;
            }
            for (k = 0;  k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstPipeInfo->astPipeChnInfo[k].bEnable
                    || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx))
                {
                    continue;
                }

                if (pstOsdCfg->stTimeOsd.bShow)
                {
                    MLOGD("OsdIdx[%d]: VcapDev[%d] Pipe[%d] PipeChn[%d] Start OSD\n",
                        s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx, i, j, k);
                    HI_OSD_ATTR_S stOsdAttr;
                    PDT_MEDIA_OSD_GenerateTimeOsdAttr(pstOsdCfg,
                        &pstPipeInfo->astPipeChnInfo[k].stDestResolution, 1, &stOsdAttr);
                    stOsdAttr.astDispAttr[0].ModHdl = pstPipeInfo->VcapPipeHdl;
                    stOsdAttr.astDispAttr[0].ChnHdl = pstPipeInfo->astPipeChnInfo[k].PipeChnHdl;

                    s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx, &stOsdAttr);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
                else
                {
                    MLOGD("OsdIdx[%d]: VcapDev[%d] Pipe[%d] PipeChn[%d] Stop OSD\n",
                        s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx, i, j, k);
                    s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }

    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_SetVpssTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        const HI_PDT_MEDIA_OSD_VPSS_INFO_S* pstVpssInfo = &pstVideoInfo->astVpssInfo[i];
        if (!pstVpssInfo->bEnable)
        {
            continue;
        }
        for (j = 0;  j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if (!pstVpssInfo->astPortInfo[j].bEnable
                || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx))
            {
                continue;
            }

            if (pstOsdCfg->stTimeOsd.bShow)
            {
                MLOGI("OsdIdx[%d]: Vpss[%d] Port[%d] Res[%ux%u] Start OSD\n",
                    s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx, i, j,
                    pstVpssInfo->astPortInfo[j].stResolution.u32Width,
                    pstVpssInfo->astPortInfo[j].stResolution.u32Height);
                HI_OSD_ATTR_S stOsdAttr;
                PDT_MEDIA_OSD_GenerateTimeOsdAttr(pstOsdCfg,
                    &pstVpssInfo->astPortInfo[j].stResolution, 1, &stOsdAttr);
                stOsdAttr.astDispAttr[0].ModHdl = pstVpssInfo->VpssHdl;
                stOsdAttr.astDispAttr[0].ChnHdl = pstVpssInfo->astPortInfo[j].VportHdl;
                stOsdAttr.astDispAttr[0].enAttachDest = ATTACH_JPEG_MAIN;

                s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx, &stOsdAttr);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
            else
            {
                MLOGI("OsdIdx[%d]: Vpss[%d] Port[%d] Stop OSD\n",
                    s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx, i, j);
                s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }

    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_SetVencTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        const HI_PDT_MEDIA_OSD_VENC_INFO_S* pstVencInfo = &pstVideoInfo->astVencInfo[i];
        if (!pstVencInfo->bEnable
            || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx))
        {
            continue;
        }

        if (pstOsdCfg->stTimeOsd.bShow)
        {
            MLOGD("OsdIdx[%d]: Venc[%d] Start OSD\n",
                s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx, i);
            HI_OSD_ATTR_S stOsdAttr;
            SIZE_S stResolution;
            stResolution.u32Width  = pstVencInfo->stResolution.u32Width;
            stResolution.u32Height = pstVencInfo->stResolution.u32Height;
            PDT_MEDIA_OSD_GenerateTimeOsdAttr(pstOsdCfg, &stResolution, 1, &stOsdAttr);
            stOsdAttr.astDispAttr[0].ChnHdl = pstVencInfo->VencHdl;

            s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx, &stOsdAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
        else
        {
            MLOGD("OsdIdx[%d]: Venc[%d] Stop OSD\n",
                s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx, i);
            s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
        }
    }
    return HI_SUCCESS;
}

#if defined(CONFIG_PQT_SUPPORT_ON)

static HI_VOID PDT_MEDIA_OSD_GetInfoStr(VI_PIPE ViPipe, HI_CHAR *pszStr, HI_U32 u32StrLen)
{
    ISP_EXP_INFO_S stExpInfo;
    ISP_WB_INFO_S stWBInfo;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;
    HI_MPI_ISP_QueryExposureInfo(ViPipe, &stExpInfo);
    HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
    HI_MPI_ISP_QueryWBInfo(ViPipe,&stWBInfo);

    snprintf(pszStr, u32StrLen, "S:%u L:%u Ag:%u IspDg:%u R:%u CT:%u",
        stExpInfo.u32ExpTime, stExpInfo.u32ShortExpTime, stExpInfo.u32AGain, stExpInfo.u32ISPDGain,
        stInnerStateInfo.u32WDRExpRatioActual[0],stWBInfo.u16ColorTemp);
    MLOGD("ViPipe[%d], DebugStr[%s]\n", ViPipe, pszStr);
}

static HI_VOID PDT_MEDIA_OSD_GenerateInfoOsdAttr(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg,
                        const SIZE_S* pstResoution, HI_U32 u32DispNum,
                        HI_OSD_ATTR_S* pstOsdAttr)
{
    /* content */
    pstOsdAttr->stContent.enType = HI_OSD_TYPE_STRING;
    pstOsdAttr->stContent.u32Color = pstOsdCfg->stTimeOsd.u32Color;
    pstOsdAttr->stContent.u32BgColor = pstOsdCfg->stTimeOsd.u32BgColor;
    PDT_MEDIA_OSD_CalcFontSize(&pstOsdCfg->stTimeOsd.stFontSize, pstResoution, &pstOsdAttr->stContent.stFontSize);
    MLOGD("Res[%ux%u] FontSize[%ux%u] Color[%#x] BgColor[%#x]\n",
        pstResoution->u32Width, pstResoution->u32Height,
        pstOsdAttr->stContent.stFontSize.u32Width, pstOsdAttr->stContent.stFontSize.u32Height,
        pstOsdAttr->stContent.u32Color, pstOsdAttr->stContent.u32BgColor);

    /* display attribute */
    HI_U32 i;
    pstOsdAttr->u32DispNum = u32DispNum;
    for (i = 0; i < u32DispNum; ++i)
    {
        pstOsdAttr->astDispAttr[i].bShow = pstOsdCfg->stTimeOsd.bShow;
        pstOsdAttr->astDispAttr[i].enBindedMod = pstOsdCfg->enBindMod;
        pstOsdAttr->astDispAttr[i].u32FgAlpha = pstOsdCfg->stTimeOsd.u32FgAlpha;
        pstOsdAttr->astDispAttr[i].u32BgAlpha = pstOsdCfg->stTimeOsd.u32BgAlpha;
        pstOsdAttr->astDispAttr[i].enCoordinate = HI_OSD_COORDINATE_ABS_COOR;
        pstOsdAttr->astDispAttr[i].stStartPos.s32X =
            pstResoution->u32Width * PDT_MEDIA_OSD_INFO_START_X / 100;
        pstOsdAttr->astDispAttr[i].stStartPos.s32X =
        HI_APPCOMM_ALIGN(pstOsdAttr->astDispAttr[i].stStartPos.s32X, 2);
        pstOsdAttr->astDispAttr[i].stStartPos.s32Y =
            pstResoution->u32Height * PDT_MEDIA_OSD_INFO_START_Y / 100;
        pstOsdAttr->astDispAttr[i].stStartPos.s32Y =
        HI_APPCOMM_ALIGN(pstOsdAttr->astDispAttr[i].stStartPos.s32Y, 2);
    }
}

static HI_S32 PDT_MEDIA_OSD_SetVcapInfoOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg,
                                           HI_BOOL bUpdateOnly)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j, k;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        if (!pstVideoInfo->astVcapDevInfo[i].bEnable)
        {
            continue;
        }
        for (j = 0;  j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            const HI_PDT_MEDIA_OSD_VCAP_PIPE_INFO_S* pstPipeInfo =
                &pstVideoInfo->astVcapDevInfo[i].astPipeInfo[j];
            if (!pstPipeInfo->bEnable)
            {
                continue;
            }
            for (k = 0;  k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                if (!pstPipeInfo->astPipeChnInfo[k].bEnable
                    || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx))
                {
                    continue;
                }

                if (pstOsdCfg->stTimeOsd.bShow)
                {
                    HI_OSD_ATTR_S stOsdAttr;
                    PDT_MEDIA_OSD_GenerateInfoOsdAttr(pstOsdCfg,
                        &pstPipeInfo->astPipeChnInfo[k].stDestResolution, 1, &stOsdAttr);
                    PDT_MEDIA_OSD_GetInfoStr(j, stOsdAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);
                    stOsdAttr.astDispAttr[0].ModHdl = pstPipeInfo->VcapPipeHdl;
                    stOsdAttr.astDispAttr[0].ChnHdl = pstPipeInfo->astPipeChnInfo[k].PipeChnHdl;

                    s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx,
                        &stOsdAttr);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    if (!bUpdateOnly)
                    {
                        MLOGD("OsdIdx[%d]: VcapDev[%d] Pipe[%d] PipeChn[%d] Start OSD\n",
                            s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx, i, j, k);
                        s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                    }
                }
                else
                {
                    if (!bUpdateOnly)
                    {
                        MLOGD("OsdIdx[%d]: VcapDev[%d] Pipe[%d] PipeChn[%d] Stop OSD\n",
                            s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx, i, j, k);
                        s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx);
                        HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                    }
                }
            }
        }

    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_SetVpssInfoOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg,
                                           HI_BOOL bUpdateOnly)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        const HI_PDT_MEDIA_OSD_VPSS_INFO_S* pstVpssInfo = &pstVideoInfo->astVpssInfo[i];
        if (!pstVpssInfo->bEnable)
        {
            continue;
        }
        for (j = 0;  j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            if (!pstVpssInfo->astPortInfo[j].bEnable
                || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx))
            {
                continue;
            }

            if (pstOsdCfg->stTimeOsd.bShow)
            {
                HI_OSD_ATTR_S stOsdAttr;
                PDT_MEDIA_OSD_GenerateInfoOsdAttr(pstOsdCfg,
                    &pstVpssInfo->astPortInfo[j].stResolution, 1, &stOsdAttr);
                PDT_MEDIA_OSD_GetInfoStr(pstVpssInfo->VcapPipeHdl,
                    stOsdAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);
                stOsdAttr.astDispAttr[0].ModHdl = pstVpssInfo->VpssHdl;
                stOsdAttr.astDispAttr[0].ChnHdl = pstVpssInfo->astPortInfo[j].VportHdl;

                s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx, &stOsdAttr);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                if (!bUpdateOnly)
                {
                    MLOGD("OsdIdx[%d]: Vpss[%d] Port[%d] Start OSD\n",
                        s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx, i, j);
                    s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
            else
            {
                if (!bUpdateOnly)
                {
                    MLOGD("OsdIdx[%d]: Vpss[%d] Port[%d] Stop OSD\n",
                        s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx, i, j);
                    s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
                }
            }
        }

    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_GetVencRelevantVcapPipeHdl(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_VENC_INFO_S* pstVencInfo,
                                           HI_HANDLE* pstVcapPipeHdl)
{
    if (HI_PDT_MEDIA_VIDEOMOD_VCAP == pstVencInfo->enBindedMod)
    {
        *pstVcapPipeHdl = pstVencInfo->ModHdl;
        return HI_SUCCESS;
    }
    else if (HI_PDT_MEDIA_VIDEOMOD_VPSS == pstVencInfo->enBindedMod)
    {
        HI_S32 i;
        for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
        {
            if (pstVideoInfo->astVpssInfo[i].VpssHdl == pstVencInfo->ModHdl)
            {
                *pstVcapPipeHdl = pstVideoInfo->astVpssInfo[i].VcapPipeHdl;
                return HI_SUCCESS;
            }
        }
    }
    return HI_PDT_MEDIA_EINVAL;
}

static HI_S32 PDT_MEDIA_OSD_SetVencInfoOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                           const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg,
                                           HI_BOOL bUpdateOnly)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        const HI_PDT_MEDIA_OSD_VENC_INFO_S* pstVencInfo = &pstVideoInfo->astVencInfo[i];
        if (!pstVencInfo->bEnable
            || (0 > s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx))
        {
            continue;
        }

        if (pstOsdCfg->stTimeOsd.bShow)
        {
            HI_HANDLE VcapPipeHdl = -1;
            HI_OSD_ATTR_S stOsdAttr;
            PDT_MEDIA_OSD_GenerateInfoOsdAttr(pstOsdCfg, &pstVencInfo->stResolution, 1, &stOsdAttr);
            s32Ret = PDT_MEDIA_OSD_GetVencRelevantVcapPipeHdl(pstVideoInfo, pstVencInfo, &VcapPipeHdl);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            PDT_MEDIA_OSD_GetInfoStr(VcapPipeHdl, stOsdAttr.stContent.szStr, HI_OSD_MAX_STR_LEN);
            stOsdAttr.astDispAttr[0].ChnHdl = pstVencInfo->VencHdl;
            stOsdAttr.astDispAttr[0].enAttachDest = ATTACH_JPEG_MAIN;

            s32Ret = HI_OSD_SetAttr(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx, &stOsdAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            if (!bUpdateOnly)
            {
                MLOGD("OsdIdx[%d]: Venc[%d] Start OSD\n",
                    s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx, i);
                s32Ret = HI_OSD_Start(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
        else
        {
            if (!bUpdateOnly)
            {
                MLOGD("OsdIdx[%d]: Venc[%d] Stop OSD\n",
                    s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx, i);
                s32Ret = HI_OSD_Stop(s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
            }
        }
    }
    return HI_SUCCESS;
}

static HI_VOID * PDT_MEDIA_OSD_UpdateInfoTsk(HI_VOID* pvParam)
{
    prctl(PR_SET_NAME, "Hi_OSDInfoUpdate", 0, 0, 0);

    while(s_stMEDIAOsdCtx.bInfoTskRun)
    {
        sleep(1);
        switch(s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg.enBindMod)
        {
            case HI_OSD_BINDMOD_VI:
                PDT_MEDIA_OSD_SetVcapInfoOsd(&s_stMEDIAOsdCtx.stVideoInfo,
                    &s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg, HI_TRUE);
                break;
            case HI_OSD_BINDMOD_VPSS:
                PDT_MEDIA_OSD_SetVpssInfoOsd(&s_stMEDIAOsdCtx.stVideoInfo,
                    &s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg, HI_TRUE);
                break;
            case HI_OSD_BINDMOD_VENC:
                PDT_MEDIA_OSD_SetVencInfoOsd(&s_stMEDIAOsdCtx.stVideoInfo,
                    &s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg, HI_TRUE);
                break;
            default:
                MLOGW("unsupport enBindMod[%d]\n", s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg.enBindMod);
        }
    }
    return NULL;
}

static HI_S32 PDT_MEDIA_OSD_SetInfoOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                       const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    switch(pstVideoInfo->stOsdCfg.enBindMod)
    {
        case HI_OSD_BINDMOD_VI:
            s32Ret = PDT_MEDIA_OSD_SetVcapInfoOsd(pstVideoInfo, pstOsdCfg, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_OSD_BINDMOD_VPSS:
            s32Ret = PDT_MEDIA_OSD_SetVpssInfoOsd(pstVideoInfo, pstOsdCfg, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_OSD_BINDMOD_VENC:
            s32Ret = PDT_MEDIA_OSD_SetVencInfoOsd(pstVideoInfo, pstOsdCfg, HI_FALSE);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        default:
            MLOGE("unsupport enBindMod[%d]\n", pstVideoInfo->stOsdCfg.enBindMod);
            return HI_PDT_MEDIA_EUNSUPPORT;
    }

    /* Start/Stop UpdateTsk */
    if (pstOsdCfg->stTimeOsd.bShow)
    {
        if (!s_stMEDIAOsdCtx.bInfoTskRun)
        {
            s_stMEDIAOsdCtx.bInfoTskRun = HI_TRUE;
            s32Ret = pthread_create(&s_stMEDIAOsdCtx.InfoTskId, NULL, PDT_MEDIA_OSD_UpdateInfoTsk, NULL);
            if (HI_SUCCESS != s32Ret)
            {
                MLOGE( "create OSD_UpdateInfoThread failed %x\n", s32Ret);
                return HI_FAILURE;
            }
            MLOGD("Start OsdDebugInfo Update Thread\n");
        }
    }
    else
    {
        if (s_stMEDIAOsdCtx.bInfoTskRun)
        {
            MLOGD("Stop OsdDebugInfo Update Thread\n");
            s_stMEDIAOsdCtx.bInfoTskRun = HI_FALSE;
            pthread_join(s_stMEDIAOsdCtx.InfoTskId, NULL);
        }
    }

    return s32Ret;
}

#endif

static HI_S32 PDT_MEDIA_OSD_SetTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstVideoInfo,
                                       const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstVideoInfo->stOsdCfg.enBindMod)
    {
        case HI_OSD_BINDMOD_VI:
            s32Ret = PDT_MEDIA_OSD_SetVcapTimeOsd(pstVideoInfo, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_OSD_BINDMOD_VPSS:
            s32Ret = PDT_MEDIA_OSD_SetVpssTimeOsd(pstVideoInfo, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        case HI_OSD_BINDMOD_VENC:
            s32Ret = PDT_MEDIA_OSD_SetVencTimeOsd(pstVideoInfo, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        default:
            MLOGE("unsupport enBindMod[%d]\n", pstVideoInfo->stOsdCfg.enBindMod);
            s32Ret = HI_PDT_MEDIA_EUNSUPPORT;
    }

#if defined(CONFIG_PQT_SUPPORT_ON)
    s32Ret = PDT_MEDIA_OSD_SetInfoOsd(pstVideoInfo, pstOsdCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
    return s32Ret;
}

static HI_S32 PDT_MEDIA_OSD_SetVcapBitMapSize(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j, k;
    HI_S32 s32OsdIdx = 0;
    HI_U32 u32MaxBitMapSize = 0;
    SIZE_S stFontSize;

    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VCAPDEV_PIPE_MAX_CNT; ++j)
        {
            for (k = 0; k < HI_PDT_MEDIA_VCAPPIPE_CHN_MAX_CNT; ++k)
            {
                const SIZE_S* pstPipeChnSize =
                    &pstVideoAttr->astVcapDevAttr[i].astPipeAttr[j].astPipeChnAttr[k].stMaxSize;
                if ((0 < pstPipeChnSize->u32Width) && (0 < pstPipeChnSize->u32Height))
                {
                    PDT_MEDIA_OSD_CalcFontSize(&pstOsdCfg->stTimeOsd.stFontSize, pstPipeChnSize, &stFontSize);
                    u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_TIME_OSD_LEN;
                    s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8TimeIdx = s32OsdIdx;
                    MLOGD("VCapDev[%d] Pipe[%d] PipeChn[%d] TimeOsd FontSize[%u,%u] MaxBitMapSize[%u] OsdIdx[%d]\n",
                        i, j, k, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

                    //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    s32OsdIdx++;

#if defined(CONFIG_PQT_SUPPORT_ON)
                    u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_INFO_OSD_LEN;
                    s_stMEDIAOsdCtx.stOsdIdx.astVcapPipeChnOsdIdx[i][j][k].s8InfoIdx = s32OsdIdx;
                    MLOGD("VCapDev[%d] Pipe[%d] PipeChn[%d] InfoOsd FontSize[%u,%u] MaxBitMapSize[%u] OsdIdx[%d]\n",
                        i, j, k, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

                    //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
                    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                    s32OsdIdx++;
#endif
                }
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_SetVpssBitMapSize(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
    HI_S32 s32OsdIdx = 0;
    HI_U32 u32MaxBitMapSize = 0;
    SIZE_S stFontSize;

    for (i = 0; i < HI_PDT_MEDIA_VPSS_MAX_CNT; ++i)
    {
        for (j = 0; j < HI_PDT_MEDIA_VPSS_PORT_MAX_CNT; ++j)
        {
            const SIZE_S* pstVpssPortSize = &pstVideoAttr->astVpssAttr[i].astPortAttr[j].stMaxSize;
            if ((0 < pstVpssPortSize->u32Width) && (0 < pstVpssPortSize->u32Height))
            {
                PDT_MEDIA_OSD_CalcFontSize(&pstOsdCfg->stTimeOsd.stFontSize, pstVpssPortSize, &stFontSize);
                u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_TIME_OSD_LEN;
                s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8TimeIdx = s32OsdIdx;
                MLOGD("Vpss[%d] Port[%d] FontSize[%u,%u] TimeOsd MaxBitMapSize[%u] OsdIdx[%d]\n",
                    i, j, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

                //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32OsdIdx++;

#if defined(CONFIG_PQT_SUPPORT_ON)
                u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_INFO_OSD_LEN;
                s_stMEDIAOsdCtx.stOsdIdx.astVpssPortOsdIdx[i][j].s8InfoIdx = s32OsdIdx;
                MLOGD("Vpss[%d] Port[%d] FontSize[%u,%u] InfoOsd MaxBitMapSize[%u] OsdIdx[%d]\n",
                    i, j, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

                //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
                HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

                s32OsdIdx++;
#endif
            }
        }
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_MEDIA_OSD_SetVencBitMapSize(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_S32 s32OsdIdx = 0;
    HI_U32 u32MaxBitMapSize = 0;
    SIZE_S stFontSize;

    for (i = 0; i < HI_PDT_MEDIA_VENC_MAX_CNT; ++i)
    {
        const SIZE_S* pstVencSize = &pstVideoAttr->astVencAttr[i].stMaxSize;
        if ((0 < pstVencSize->u32Width) && (0 < pstVencSize->u32Height))
        {
            PDT_MEDIA_OSD_CalcFontSize(&pstOsdCfg->stTimeOsd.stFontSize, pstVencSize, &stFontSize);
            u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_TIME_OSD_LEN;
            s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8TimeIdx = s32OsdIdx;
            MLOGD("Venc[%d] FontSize[%u,%u] TimeOsd MaxBitMapSize[%u] OsdIdx[%d]\n",
                i, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

            //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32OsdIdx++;

#if defined(CONFIG_PQT_SUPPORT_ON)
            u32MaxBitMapSize = stFontSize.u32Width * stFontSize.u32Height * PDT_MEDIA_MAX_INFO_OSD_LEN;
            s_stMEDIAOsdCtx.stOsdIdx.astVencOsdIdx[i].s8InfoIdx = s32OsdIdx;
            MLOGD("Venc[%d] FontSize[%u,%u] InfoOsd MaxBitMapSize[%u] OsdIdx[%d]\n",
                i, stFontSize.u32Width, stFontSize.u32Height, u32MaxBitMapSize, s32OsdIdx);

            //s32Ret = HI_OSD_SetMaxBitmapSize(s32OsdIdx, u32MaxBitMapSize);
            HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

            s32OsdIdx++;
#endif
        }
    }
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_Init(const HI_PDT_MEDIA_OSD_VIDEO_ATTR_S* pstVideoAttr,
                          const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_APPCOMM_CHECK_POINTER(pstVideoAttr, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_POINTER(pstOsdCfg, HI_PDT_MEDIA_EINVAL);

    HI_S32 s32Ret = HI_SUCCESS;

    if (s_stMEDIAOsdCtx.bInit)
    {
        MLOGI("has already init\n");
        return HI_SUCCESS;
    }
    HI_PDT_MEDIA_CFG_S* pstMediaCfg = PDT_MEDIA_GetMediaCfg();
    HI_APPCOMM_CHECK_POINTER(pstMediaCfg, HI_PDT_MEDIA_EUNSUPPORT);

    /* Init OSD */
    extern HI_S32 PDT_MEDIA_InitOsdFontLib(HI_VOID);
    s32Ret = PDT_MEDIA_InitOsdFontLib();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    memset(&s_stMEDIAOsdCtx.stOsdIdx, -1, sizeof(PDT_MEDIA_OSD_IDX_S));

    /* Set MaxBitmapSize */
    switch(pstOsdCfg->enBindMod)
    {
        case HI_OSD_BINDMOD_VI:
        {
            s32Ret = PDT_MEDIA_OSD_SetVcapBitMapSize(pstVideoAttr, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        }
        case HI_OSD_BINDMOD_VPSS:
        {
            s32Ret = PDT_MEDIA_OSD_SetVpssBitMapSize(pstVideoAttr, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        }
        case HI_OSD_BINDMOD_VENC:
        {
            s32Ret = PDT_MEDIA_OSD_SetVencBitMapSize(pstVideoAttr, pstOsdCfg);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            break;
        }
        default:
        {
            MLOGW("unsupport bind module\n");
            return HI_PDT_MEDIA_EUNSUPPORT;
        }
    }

    /* Update Context */
    s_stMEDIAOsdCtx.bInit = HI_TRUE;
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_Deinit(HI_VOID)
{
    if (!s_stMEDIAOsdCtx.bInit)
    {
        MLOGI("has already deinit\n");
        return HI_SUCCESS;
    }

    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_OSD_Deinit();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);

    s_stMEDIAOsdCtx.bInit = HI_FALSE;
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_StartTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstOsdInfo, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stMEDIAOsdCtx.bInit, HI_PDT_MEDIA_ENOTINIT);

    HI_S32 s32Ret = HI_SUCCESS;
    if (pstOsdInfo->stOsdCfg.stTimeOsd.bShow)
    {
        memcpy(&s_stMEDIAOsdCtx.stVideoInfo, pstOsdInfo, sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S));
        s32Ret = PDT_MEDIA_OSD_SetTimeOsd(pstOsdInfo, &pstOsdInfo->stOsdCfg);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_OSD_StopTimeOsd(const HI_PDT_MEDIA_OSD_VIDEO_INFO_S* pstOsdInfo)
{
    HI_APPCOMM_CHECK_POINTER(pstOsdInfo, HI_PDT_MEDIA_EINVAL);
    HI_APPCOMM_CHECK_EXPR(s_stMEDIAOsdCtx.bInit, HI_PDT_MEDIA_ENOTINIT);

    HI_S32 s32Ret = HI_SUCCESS;
    memcpy(&s_stMEDIAOsdCtx.stVideoInfo, pstOsdInfo, sizeof(HI_PDT_MEDIA_OSD_VIDEO_INFO_S));
    s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg.stTimeOsd.bShow = HI_FALSE;

    s32Ret = PDT_MEDIA_OSD_SetTimeOsd(pstOsdInfo, &s_stMEDIAOsdCtx.stVideoInfo.stOsdCfg);
    HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

