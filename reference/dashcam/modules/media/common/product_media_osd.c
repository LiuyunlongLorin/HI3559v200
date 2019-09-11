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
#include <pthread.h>
#include <sys/prctl.h>
#include "mpi_isp.h"
#include "mpi_awb.h"
#include "hi_mapi_vcap.h"
#include "hi_mapi_vproc.h"
#include "hi_mapi_venc.h"
#include "hi_mapi_disp.h"
#include "hi_osd.h"
#include "hi_product_media.h"
#if defined(AMP_LINUX_HUAWEILITE)
#include "product_media_msg.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** Maximum TimeOSD String Length */
#define PDT_MEDIA_MAX_TIME_OSD_LEN        (19)
/** Maximum InfoOSD String Length */
#define PDT_MEDIA_MAX_INFO_OSD_LEN        (64)

//#define CONFIG_ISP_OSD_SUPPORT_ON
#if defined(CONFIG_ISP_OSD_SUPPORT_ON)
#define ISP_INFO_OSD
/** PQ Debug Info OSD Posotion in percent */
#define OSD_INFO_START_X        (1)
#define OSD_INFO_START_Y        (80)
#define OSD_INFO_FONT_WIDTH     (36)
#define OSD_INFO_FONT_HEIGHT    (72)
#endif

/*define osd base font size on 1080p*/
#define OSD_BASE_FONT_W (48)
#define OSD_BASE_FONT_H (96)
#define OSD_BASE_RES_W (1920)
#define OSD_BASE_RES_H (1080)

/*-------------------------------------------*/

typedef struct tagPDT_MEDIA_OSD_CTX_S
{
    HI_BOOL bInit;
    HI_PDT_MEDIA_OSD_CFG_S stOsdCfg;
#if defined(ISP_INFO_OSD)
    HI_BOOL abInfoTskRun[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    pthread_t aInfoTskId[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_OSD_ATTR_S astInfoOsdAttr[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
#endif
} PDT_MEDIA_OSD_CTX_S;
static PDT_MEDIA_OSD_CTX_S s_stMEDIAOsdCtx;

#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
/** osd logo data */
#include "../server/logo_160.dat"
#include "../server/logo_86.dat"

/**
 * @brief   get logo bitmap information
 * @param[in]u32ResHeight : venc resoluton height
 * @param[out]pstBitmap : logo bitmap
 * @return 0 success
 * @return -1 failure, invalid resolution height
 */
HI_S32 PDT_MEDIA_LoadLogoBmp(HI_OSD_BITMAP_ATTR_S* pstBitmap)
{
#define LOGO_SIZE_L           (160)
#define LOGO_SIZE_S           (86)

    HI_APPCOMM_CHECK_POINTER(pstBitmap, HI_FAILURE);

    HI_U8* pu8LogoBM = NULL;

    if (pstBitmap->u32Width >= LOGO_SIZE_L)
    {
        pu8LogoBM = logo_160_bin;
    }
    else if (pstBitmap->u32Width >= LOGO_SIZE_S)
    {
        pu8LogoBM = logo_86_bin;
    }
    else
    {
        MLOGE("Invalid Res Height[%ux%u]\n", pstBitmap->u32Width,pstBitmap->u32Height);
        return HI_FAILURE;
    }

    BITMAP_S stBitmap;

    memcpy(&stBitmap.enPixelFormat, pu8LogoBM, sizeof(PIXEL_FORMAT_E));
    pu8LogoBM += sizeof(PIXEL_FORMAT_E);

    memcpy(&stBitmap.u32Width, pu8LogoBM, sizeof(HI_U32));
    pu8LogoBM += sizeof(HI_U32);

    memcpy(&stBitmap.u32Height, pu8LogoBM, sizeof(HI_U32));
    pu8LogoBM += sizeof(HI_U32);

    pstBitmap->enPixelFormat = HI_OSD_PIXEL_FMT_RGB1555;
    pstBitmap->u32Width = stBitmap.u32Width;
    pstBitmap->u32Height = stBitmap.u32Height;
    pstBitmap->pvData = (HI_VOID*)pu8LogoBM;
    pstBitmap->u64PhyAddr = (HI_U64)(HI_UL)pu8LogoBM;
    return HI_SUCCESS;
}
#endif

static HI_S32 PDT_MEDIA_AdaptFontSize(SIZE_S stBaseFontSize,SIZE_S stBaseImageSize,HI_OSD_ATTR_S* pstOsd)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SIZE_S stImageSize;
    HI_S32 s32DispIdx = 0;

    if (stBaseFontSize.u32Width==0||stBaseFontSize.u32Height==0)
    {
        stBaseFontSize.u32Width = OSD_BASE_FONT_W;
        stBaseFontSize.u32Height= OSD_BASE_FONT_H;
    }
    if (stBaseImageSize.u32Width==0||stBaseImageSize.u32Height==0)
    {
        stBaseImageSize.u32Width = OSD_BASE_RES_W;
        stBaseImageSize.u32Height= OSD_BASE_RES_H;
    }

    for (s32DispIdx = 0; s32DispIdx < pstOsd->u32DispNum ; ++s32DispIdx)
    {
        HI_OSD_DISP_ATTR_S* pstDispAttr = &pstOsd->astDispAttr[s32DispIdx];
        switch (pstDispAttr->enBindedMod)
        {
            case HI_OSD_BINDMOD_VI:
            {
                HI_HANDLE VcapDevHdl = pstDispAttr->s32Batch/10;
                HI_MAPI_VCAP_ATTR_S stVCapAttr;
                s32Ret = HI_MAPI_VCAP_GetAttr(VcapDevHdl, &stVCapAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    MLOGW("HI_MAPI_VCAP_GetAttr(%d) fail,Error Code: [0x%08X]\n", VcapDevHdl, s32Ret);
                    return s32Ret;
                }
                stImageSize = stVCapAttr.astVcapPipeAttr[pstDispAttr->ModHdl].astPipeChnAttr[pstDispAttr->ChnHdl].stDestSize;
                break;
            }

            case HI_OSD_BINDMOD_VPSS:
            {
                HI_MAPI_VPORT_ATTR_S stVPortAttr;
                s32Ret = HI_MAPI_VPROC_GetPortAttr(pstDispAttr->ModHdl, pstDispAttr->ChnHdl, &stVPortAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    MLOGW("HI_MAPI_VPROC_GetPortAttr(%d,%d) fail,Error Code: [0x%08X]\n", pstDispAttr->ModHdl, pstDispAttr->ChnHdl, s32Ret);
                    return s32Ret;
                }
                stImageSize.u32Width = stVPortAttr.u32Width;
                stImageSize.u32Height = stVPortAttr.u32Height;
                break;
            }

            case HI_OSD_BINDMOD_VENC:
            {
                HI_MAPI_VENC_ATTR_S stStreamAttr;
                s32Ret = HI_MAPI_VENC_GetAttr(pstDispAttr->ChnHdl, &stStreamAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    MLOGW("HI_MPI_VENC_GetChnAttr(%d) fail,Error Code: [0x%08X]\n", pstDispAttr->ChnHdl, s32Ret);
                    return s32Ret;
                }
                stImageSize.u32Width = stStreamAttr.stVencPloadTypeAttr.u32Width;
                stImageSize.u32Height = stStreamAttr.stVencPloadTypeAttr.u32Height;
                break;
            }

            case HI_OSD_BINDMOD_VO:
            {
                HI_MAPI_DISP_WINDOW_ATTR_S stWndAttr;
                s32Ret = HI_MAPI_DISP_GetWindowAttr(pstDispAttr->ModHdl, pstDispAttr->ChnHdl, &stWndAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    MLOGW("HI_MAPI_DISP_GetWindowAttr(%d,%d) fail,Error Code: [0x%08X]\n", pstDispAttr->ModHdl, pstDispAttr->ChnHdl, s32Ret);
                    return s32Ret;
                }
                stImageSize.u32Width = stWndAttr.stRect.u32Width;
                stImageSize.u32Height = stWndAttr.stRect.u32Height;
                break;
            }

            default:
                MLOGW("invalide BindedMod [%d]\n", pstDispAttr->enBindedMod);
                return HI_EINVAL;
        }
        pstOsd->stContent.stFontSize.u32Width=stImageSize.u32Width*stBaseFontSize.u32Width/stBaseImageSize.u32Width;
        pstOsd->stContent.stFontSize.u32Width=HI_APPCOMM_ALIGN(pstOsd->stContent.stFontSize.u32Width, 2);
        pstOsd->stContent.stFontSize.u32Height=stImageSize.u32Height*stBaseFontSize.u32Height/stBaseImageSize.u32Height;
        pstOsd->stContent.stFontSize.u32Height=HI_APPCOMM_ALIGN(pstOsd->stContent.stFontSize.u32Height, 2);
        return HI_SUCCESS;
    }
    return HI_EINVAL;
}

#if defined(ISP_INFO_OSD)
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
static HI_VOID PDT_MEDIA_OSD_GetInfoStr(VI_PIPE ViPipe, HI_CHAR* pszStr, HI_U32 u32StrLen)
{
    ISP_EXP_INFO_S stExpInfo;
    ISP_WB_INFO_S stWBInfo;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;
    memset(&stExpInfo,0,sizeof(ISP_EXP_INFO_S));
    memset(&stWBInfo,0,sizeof(ISP_WB_INFO_S));
    memset(&stInnerStateInfo,0,sizeof(ISP_INNER_STATE_INFO_S));
    HI_MPI_ISP_QueryExposureInfo(ViPipe, &stExpInfo);
    HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
    HI_MPI_ISP_QueryWBInfo(ViPipe, &stWBInfo);

    snprintf(pszStr, u32StrLen, "S:%u L:%u Ag:%u IspDg:%u R:%u CT:%u",
             stExpInfo.u32ExpTime, stExpInfo.u32ShortExpTime, stExpInfo.u32AGain, stExpInfo.u32ISPDGain,
             stInnerStateInfo.u32WDRExpRatioActual[0], stWBInfo.u16ColorTemp);
    MLOGD("ViPipe[%d], DebugStr[%s]\n", ViPipe, pszStr);
}

static HI_VOID* PDT_MEDIA_OSD_UpdateInfoTsk(HI_VOID* pvParam)
{
    prctl(PR_SET_NAME, "OSDInfoTsk", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg = &s_stMEDIAOsdCtx.stOsdCfg;
    HI_S32 s32DevIndex = *(HI_S32*)pvParam;
    VI_PIPE ViPipe = s32DevIndex;
    HI_S32 s32OsdIdx = pstOsdCfg->s32OsdCnt+s32DevIndex;
    HI_OSD_ATTR_S * pstInfoOsdAttr = &s_stMEDIAOsdCtx.astInfoOsdAttr[s32DevIndex];

    PDT_MEDIA_OSD_GetInfoStr(ViPipe, pstInfoOsdAttr->stContent.szStr, PDT_MEDIA_MAX_INFO_OSD_LEN);
    s32Ret = HI_OSD_SetAttr(s32OsdIdx, pstInfoOsdAttr);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_OSD_SetAttr[%d] failed %x\n", s32OsdIdx,s32Ret);
        return NULL;
    }

    s32Ret = HI_OSD_Start(s32OsdIdx);
    if (HI_SUCCESS != s32Ret)
    {
        MLOGE("HI_OSD_Start[%d] failed %x\n", s32OsdIdx,s32Ret);
        return NULL;
    }

    while (s_stMEDIAOsdCtx.abInfoTskRun[s32DevIndex])
    {
        PDT_MEDIA_OSD_GetInfoStr(ViPipe, pstInfoOsdAttr->stContent.szStr, PDT_MEDIA_MAX_INFO_OSD_LEN);
        s32Ret = HI_OSD_SetAttr(s32OsdIdx, pstInfoOsdAttr);
        if (HI_SUCCESS != s32Ret)
        {
            MLOGE("HI_OSD_SetAttr[%d] failed %x\n", s32OsdIdx,s32Ret);
            break;
        }
        HI_usleep(1*1000*1000);
    }

    HI_OSD_Stop(s32OsdIdx);
    return NULL;
}
#endif
#endif

HI_S32 PDT_MEDIA_StartInfoOsd(HI_VOID)
{
#if defined(ISP_INFO_OSD)
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg = &s_stMEDIAOsdCtx.stOsdCfg;
    static HI_S32 as32VcapDevIdx[HI_PDT_MEDIA_VCAP_DEV_MAX_CNT];
    HI_S32 i = 0;
    HI_S32 s32OsdIdx = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT ; i++)
    {
        as32VcapDevIdx[i] = i;
        HI_OSD_ATTR_S * pstInfoOsdAttr = &s_stMEDIAOsdCtx.astInfoOsdAttr[i];
        for (s32OsdIdx = pstOsdCfg->s32OsdCnt-1; s32OsdIdx >= 0; s32OsdIdx--)
        {
            if (pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.enType==HI_OSD_TYPE_TIME)
            {
                HI_S32 s32DispIdx = 0;
                for (s32DispIdx = 0; s32DispIdx < pstOsdCfg->astOsdAttr[s32OsdIdx].u32DispNum; s32DispIdx++)
                {
                    if (pstOsdCfg->astOsdAttr[s32OsdIdx].astDispAttr[s32DispIdx].s32Batch==as32VcapDevIdx[i]*10+HI_OSD_TYPE_TIME)
                    {
                        pstInfoOsdAttr->astDispAttr[0] = pstOsdCfg->astOsdAttr[s32OsdIdx].astDispAttr[s32DispIdx];
                        //pstInfoOsdAttr->stContent.stFontSize = pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.stFontSize;
                        pstInfoOsdAttr->stContent.u32Color = pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.u32Color;
                        pstInfoOsdAttr->stContent.u32BgColor = pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.u32BgColor;
                        break;
                    }
                }
            }
        }
        pstInfoOsdAttr->u32DispNum = 1;
        pstInfoOsdAttr->stContent.enType = HI_OSD_TYPE_STRING;
        pstInfoOsdAttr->astDispAttr[0].enCoordinate = HI_OSD_COORDINATE_RATIO_COOR;
        pstInfoOsdAttr->astDispAttr[0].stStartPos.s32X = OSD_INFO_START_X;
        pstInfoOsdAttr->astDispAttr[0].stStartPos.s32Y = OSD_INFO_START_Y;
        SIZE_S stBaseFontSize;
        stBaseFontSize.u32Width = OSD_INFO_FONT_WIDTH;
        stBaseFontSize.u32Height= OSD_INFO_FONT_HEIGHT;
        PDT_MEDIA_AdaptFontSize(stBaseFontSize,s_stMEDIAOsdCtx.stOsdCfg.stBaseImageSize,pstInfoOsdAttr);
        if (pstInfoOsdAttr->astDispAttr[0].bShow&&s_stMEDIAOsdCtx.aInfoTskId[i]==0)
        {
            s_stMEDIAOsdCtx.abInfoTskRun[i] = HI_TRUE;
            s32Ret = pthread_create(&s_stMEDIAOsdCtx.aInfoTskId[i], NULL, PDT_MEDIA_OSD_UpdateInfoTsk, &as32VcapDevIdx[i]);
            if (HI_SUCCESS != s32Ret)
            {
                s_stMEDIAOsdCtx.abInfoTskRun[i] = HI_FALSE;
                MLOGE( "s32VcapDevIdx[%d] create OSD_UpdateInfoThread failed %x\n", as32VcapDevIdx[i],s32Ret);
                return s32Ret;
            }
        }
    }
#else
    HI_MSG_SendASync(HI_MSG_MEDIA_INFO_OSD_START, NULL, 0, NULL);
#endif
#endif
    return HI_SUCCESS;
}

HI_VOID PDT_MEDIA_StopInfoOsd(HI_VOID)
{
#if defined(ISP_INFO_OSD)
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
    HI_S32 i = 0;
    for (i = 0; i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT ; i++)
    {
        s_stMEDIAOsdCtx.abInfoTskRun[i] = HI_FALSE;
        if(0!=s_stMEDIAOsdCtx.aInfoTskId[i])
        {
            pthread_join(s_stMEDIAOsdCtx.aInfoTskId[i], NULL);
            s_stMEDIAOsdCtx.aInfoTskId[i]=0;
        }
    }
#else
    HI_MSG_SendSync(HI_MSG_MEDIA_INFO_OSD_STOP, NULL, 0, NULL, NULL, 0);
#endif
#endif
}

HI_S32 PDT_MEDIA_InitOsd(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)/* osd init and Set osd bitmap size */
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (s_stMEDIAOsdCtx.bInit)
    {
        MLOGI("has already init\n");
        return HI_SUCCESS;
    }
#if !(defined(AMP_LINUX_HUAWEILITE) && defined(__LINUX__))
    /* Init OSD */
    extern HI_S32 PDT_MEDIA_InitOsdFontLib(HI_VOID);
    s32Ret = PDT_MEDIA_InitOsdFontLib();
    HI_APPCOMM_CHECK_RETURN(s32Ret, HI_FAILURE);
#endif
    memset(&s_stMEDIAOsdCtx,0,sizeof(PDT_MEDIA_OSD_CTX_S));
    memcpy(&s_stMEDIAOsdCtx.stOsdCfg,pstOsdCfg,sizeof(HI_PDT_MEDIA_OSD_CFG_S));
    s_stMEDIAOsdCtx.bInit = HI_TRUE;
    return s32Ret;
}

/* free osd bitmap size */
HI_S32 PDT_MEDIA_DeinitOsd(HI_VOID)
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

HI_S32 PDT_MEDIA_StartOsd(const HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg)
{
    HI_APPCOMM_CHECK_EXPR(s_stMEDIAOsdCtx.bInit, HI_PDT_MEDIA_ENOTINIT);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32OsdIdx = 0;

    for (s32OsdIdx = 0; s32OsdIdx < pstOsdCfg->s32OsdCnt; s32OsdIdx++)
    {
        MLOGD("s32OsdIdx=%d\n",s32OsdIdx);
        if (pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.enType==HI_OSD_TYPE_BITMAP)
        {
#if (defined(AMP_LINUX_HUAWEILITE) && defined(__HuaweiLite__)) || defined(AMP_HUAWEILITE) || defined(AMP_LINUX)
            PDT_MEDIA_LoadLogoBmp((HI_OSD_BITMAP_ATTR_S*)&pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.stBitmap);
#else
            HI_OSD_ATTR_S stAttr;
            s32Ret = HI_OSD_GetAttr(s32OsdIdx, &stAttr);
            HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
            ((HI_PDT_MEDIA_OSD_CFG_S*)pstOsdCfg)->astOsdAttr[s32OsdIdx].stContent.stBitmap = stAttr.stContent.stBitmap;
#endif
        }

        if (pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.enType == HI_OSD_TYPE_TIME
            ||pstOsdCfg->astOsdAttr[s32OsdIdx].stContent.enType == HI_OSD_TYPE_STRING)
        {
            s32Ret = PDT_MEDIA_AdaptFontSize(pstOsdCfg->stBaseFontSize,pstOsdCfg->stBaseImageSize,(HI_OSD_ATTR_S*)&pstOsdCfg->astOsdAttr[s32OsdIdx]);
            if (s32Ret!=HI_SUCCESS)
            {
                MLOGW("PDT_MEDIA_AdaptFontSize for OSD[%d] fail,skip start this osd\n",s32OsdIdx);
                continue;
            }
        }
        s32Ret = HI_OSD_SetAttr(s32OsdIdx, &pstOsdCfg->astOsdAttr[s32OsdIdx]);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        s32Ret = HI_OSD_Start(s32OsdIdx);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
    memcpy(&s_stMEDIAOsdCtx.stOsdCfg,pstOsdCfg,sizeof(HI_PDT_MEDIA_OSD_CFG_S));

#if defined(ISP_INFO_OSD)
    if (pstOsdCfg->s32OsdCnt>0)
    {
        PDT_MEDIA_StartInfoOsd();
    }
#endif
    return HI_SUCCESS;
}

HI_S32 PDT_MEDIA_StopOsd(HI_VOID)
{
    HI_APPCOMM_CHECK_EXPR(s_stMEDIAOsdCtx.bInit, HI_PDT_MEDIA_ENOTINIT);
    HI_S32 s32Ret = HI_SUCCESS;
    HI_PDT_MEDIA_OSD_CFG_S* pstOsdCfg = &s_stMEDIAOsdCtx.stOsdCfg;
    HI_S32 s32OsdIdx = 0;

    for (s32OsdIdx = 0; s32OsdIdx < pstOsdCfg->s32OsdCnt ; s32OsdIdx++)
    {
        MLOGD("s32OsdIdx=%d\n",s32OsdIdx);
        s32Ret = HI_OSD_Stop(s32OsdIdx);
        HI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    }
#if defined(ISP_INFO_OSD)
    PDT_MEDIA_StopInfoOsd();
#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

