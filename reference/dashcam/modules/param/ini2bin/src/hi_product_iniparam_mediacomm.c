/**
 * @file      hi_product_iniparam_mediacomm.c
 * @brief     load media common parameter
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

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH264CbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_CBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H264CBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH264VbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_VBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H264VBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH264QvbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H264_QVBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H264QVBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:bit_percent_ul", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32BitPercentUL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:bit_percent_ll", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32BitPercentLL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:psnr_fluctuate_ul", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32PsnrFluctuateUL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:psnr_fluctuate_ll", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32PsnrFluctuateLL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("       BitPercentUL[%d] BitPercentLL[%d] PsnrFluctuateUL[%d] PsnrFluctuateLL[%d]\n",
            pstParam->s32BitPercentUL, pstParam->s32BitPercentLL,
            pstParam->s32PsnrFluctuateUL, pstParam->s32PsnrFluctuateLL);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencH264Attr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.h264", pszPrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:profile", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32H264Profile);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    profile[%u:0-baseline,1-main,2-high]\n", pstParam->u32H264Profile);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH264CbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH264Cbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264CBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH264VbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH264Vbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264VBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH264QvbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH264QVbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264QVBRAttr");
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH265CbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_CBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.cbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H265CBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH265VbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_VBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.vbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H265VBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadH265QvbrAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VENC_ATTR_H265_QVBR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:max_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:min_qp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:max_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MaxIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:min_iqp", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32MinIQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    H265QVBR: MaxQP[%u] MinQP[%u] MaxIQP[%u] MinIQP[%u]\n",
        pstParam->u32MaxQp, pstParam->u32MinQp, pstParam->u32MaxIQp, pstParam->u32MinIQp);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:bit_percent_ul", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32BitPercentUL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:bit_percent_ll", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32BitPercentLL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:psnr_fluctuate_ul", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32PsnrFluctuateUL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.qvbr:psnr_fluctuate_ll", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32PsnrFluctuateLL);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("       BitPercentUL[%d] BitPercentLL[%d] PsnrFluctuateUL[%d] PsnrFluctuateLL[%d]\n",
            pstParam->s32BitPercentUL, pstParam->s32BitPercentLL,
            pstParam->s32PsnrFluctuateUL, pstParam->s32PsnrFluctuateLL);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencH265Attr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.h265", pszPrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:profile", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32H265Profile);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    profile[%u:0-main]\n", pstParam->u32H265Profile);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH265CbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH265Cbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265CBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH265VbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH265Vbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265VBRAttr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadH265QvbrAttr(pszIniModule, szIniNodePrefix, &pstParam->stH265QVbr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265VBRAttr");
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszEncType, PDT_PARAM_MEDIA_COMM_VIDEO_ENC_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.video.%s", pszEncType);
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:scene_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSceneMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("[%s] scene[%d:0-normal,1-dv,2-car]\n",
        szIniNodePrefix, pstParam->enSceneMode);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencH264Attr(pszIniModule, szIniNodePrefix, pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H264Attr");

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencH265Attr(pszIniModule, szIniNodePrefix, pstParam);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "H265Attr");

    pstParam->stMjpegCbr.u32MinQfactor = 75;
    pstParam->stMjpegCbr.u32MaxQfactor = 85;
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszEncType, HI_MAPI_VENC_ATTR_JPEG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.snap.%s", pszEncType);
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable_dcf", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnableDCF);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:quality_factor", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Qfactor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:largethumbnail_num", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAttrMPF.u8LargeThumbNailNum);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

     MLOGD("[%s] bDCF[%d], QualityFactor[%u], LargeThumbNailNum[%d]\n",
        szIniNodePrefix, pstParam->bEnableDCF, pstParam->u32Qfactor, pstParam->stAttrMPF.u8LargeThumbNailNum);
     /*load large thumbnail size*/
    for(i = 0 ;i < 2; i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:largethumbnail%d_width", szIniNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAttrMPF.astLargeThumbNailSize[i].u32Width);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:largethumbnail%d_height", szIniNodePrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAttrMPF.astLargeThumbNailSize[i].u32Height);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("[%s] largethumbnail%d_width[%d],largethumbnail%d_height[%u] \n",
        szIniNodePrefix, i,pstParam->stAttrMPF.astLargeThumbNailSize[i].u32Width,i,pstParam->stAttrMPF.astLargeThumbNailSize[i].u32Height);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadVencHierarchicalQp(const HI_CHAR *pszIniModule,
                VENC_HIERARCHICAL_QP_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32 s32Idx;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "venc.hierarchical.qp");
    MLOGD("NodePrefix[%s]\n", szIniNodePrefix);

    for (s32Idx = 0; s32Idx < 4; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:qp_delta%d", szIniNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->s32HierarchicalQpDelta[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("  QpDelta%d[%d]\n", s32Idx, pstParam->s32HierarchicalQpDelta[s32Idx]);
    }

    for (s32Idx = 0; s32Idx < 4; ++s32Idx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:frame_num%d", szIniNodePrefix, s32Idx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            &pstParam->s32HierarchicalFrameNum[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("  FrameNum%d[%d]\n", s32Idx, pstParam->s32HierarchicalFrameNum[s32Idx]);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_GetCSCMatrixEnum(const HI_CHAR *pszEnumStr,
                VO_CSC_MATRIX_E *penMode)
{
    if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_IDENTITY))
    {
        *penMode = VO_CSC_MATRIX_IDENTITY;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT601_TO_BT709))
    {
        *penMode = VO_CSC_MATRIX_BT601_TO_BT709;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT709_TO_BT601))
    {
        *penMode = VO_CSC_MATRIX_BT709_TO_BT601;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT601_TO_RGB_PC))
    {
        *penMode = VO_CSC_MATRIX_BT601_TO_RGB_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_BT709_TO_RGB_PC))
    {
        *penMode = VO_CSC_MATRIX_BT709_TO_RGB_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_RGB_TO_BT601_PC))
    {
        *penMode = VO_CSC_MATRIX_RGB_TO_BT601_PC;
    }
    else if (!HI_APPCOMM_STRCMP_ENUM(pszEnumStr, VO_CSC_MATRIX_RGB_TO_BT709_PC))
    {
        *penMode = VO_CSC_MATRIX_RGB_TO_BT709_PC;
    }
    else
    {
        MLOGE("Invalid CSCMatrix[%s]\n", pszEnumStr);
        return HI_FAILURE;
    }
    MLOGD("    CSCMatrix[%s] Enum[%d]\n", pszEnumStr, *penMode);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDispCSCAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_MAPI_DISP_CSCATTREX_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR *pszString = NULL;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:cscmatrix", pszPrefix);
    s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName, NULL, &pszString);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    if (NULL != pszString)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_GetCSCMatrixEnum(pszString, &pstParam->stVoCSC.enCscMatrix);
        HI_APPCOMM_SAFE_FREE(pszString);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:luma", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Luma);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:contrast", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Contrast);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:hue", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Hue);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:saturation", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVoCSC.u32Satuature);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("      luma[%u], contrast[%u], hue[%u], saturation[%u]\n", pstParam->stVoCSC.u32Luma,
        pstParam->stVoCSC.u32Contrast, pstParam->stVoCSC.u32Hue, pstParam->stVoCSC.u32Satuature);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDispWndAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_MAPI_DISP_WINDOW_ATTR_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_x", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stRect.s32X);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_y", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stRect.s32Y);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_width", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRect.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_height", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stRect.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:priority", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32Priority);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    WndAttr pos[%u,%u], size[%u,%u], priority[%u]\n",
        pstParam->stRect.s32X, pstParam->stRect.s32Y,
        pstParam->stRect.u32Width, pstParam->stRect.u32Height,
        pstParam->u32Priority);
    return HI_SUCCESS;
}


static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDispAspectAttr(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, ASPECT_RATIO_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mode", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bg_color", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32BgColor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_x", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stVideoRect.s32X);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_y", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->stVideoRect.s32Y);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_width", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rect_height", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stVideoRect.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    aspect mode[%d] bgcolor[%d] start[%u,%u], size[%u,%u]\n",
        pstParam->enMode, pstParam->u32BgColor, pstParam->stVideoRect.s32X,
        pstParam->stVideoRect.s32Y, pstParam->stVideoRect.u32Width, pstParam->stVideoRect.u32Height);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDispWndCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, PDT_PARAM_DISP_WND_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:wndhdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->WndHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:preview_cam_id", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->s32PreViewCamID);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("    Prefix[%s] enable[%d] wndhdl[%d] preview_cam_id[%d] \n", pszPrefix,
        pstParam->bEnable, pstParam->WndHdl,  pstParam->s32PreViewCamID);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:rotate", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enRotate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    Rotate[%d, 0:0, 1:90, 2:180, 3:270]\n", pstParam->enRotate);

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.aspectratio", pszPrefix);
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDispAspectAttr(pszIniModule, szIniNodePrefix,&pstParam->stAspectRatio);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "AspectAttr");

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "%s.wndattr", pszPrefix);
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDispWndAttr(pszIniModule, szIniNodePrefix,&pstParam->stWndAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "WndAttr");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadDispCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                PDT_PARAM_MEDIA_COMM_DISP_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS,i  = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:disphdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->VdispHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Display[%d]: enable[%d] disphdl[%d]\n", s32Idx, pstParam->bEnable, pstParam->VdispHdl);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bg_color", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->u32BgColor);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD(" bg color[%08x]\n", pstParam->u32BgColor);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:windowcnt", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32WindowCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Display[%d]: windowcnt[%d]\n", s32Idx, pstParam->u32WindowCnt);

    /* Load CSC Attribute */
    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.csc", s32Idx);
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stCscAttr.bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    CSCEnable[%d]\n", pstParam->bEnable);

    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDispCSCAttr(pszIniModule, szIniNodePrefix, &pstParam->stCscAttr.stAttrEx);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "CSCAttr");

    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.SRC.%d", s32Idx,i);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bindedmod", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astDispSrc[i].enBindedMod);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:modhdl", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astDispSrc[i].ModHdl);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:chnhdl", szIniNodePrefix);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astDispSrc[i].ChnHdl);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD(" %s enBindedMod[%d] ModHdl[%d],  ChnHdl[%d] \n",szIniNodePrefix, pstParam->astDispSrc[i].enBindedMod,
            pstParam->astDispSrc[i].ModHdl,pstParam->astDispSrc[i].ChnHdl);
    }

    /* Load Window Attribute */
    HI_S32 s32WndIdx = 0;
    for (s32WndIdx = 0; s32WndIdx < HI_PDT_MEDIA_DISP_WND_MAX_CNT; ++s32WndIdx)
    {
        snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "disp.%d.window.%d", s32Idx, s32WndIdx);
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDispWndCfg(pszIniModule, szIniNodePrefix,
            &pstParam->astWndCfg[s32WndIdx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "WndCfg");
    }
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAcapCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_ACAP_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "acapture.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:handle", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:audiogain", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &pstParam->s32AudioGain);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Acap[%d] enable[%d] hdl[%d] audio gain[%d]\n", s32Idx, pstParam->bEnable,
        pstParam->AcapHdl, pstParam->s32AudioGain);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enSampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enBitwidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:work_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enWorkmode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:mixer_mic_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enMixerMicMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:resample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAcapAttr.enResampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    /* Load acap chnl Attribute */
    HI_S32 s32ChnlIdx = 0;
    for (s32ChnlIdx = 0; s32ChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32ChnlIdx)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.chnl.%d:handle", szIniNodePrefix, s32ChnlIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.chnl.%d:enable", szIniNodePrefix, s32ChnlIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlCfg[s32ChnlIdx].bEnable);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.chnl.%d:enable_vqe", szIniNodePrefix, s32ChnlIdx);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlCfg[s32ChnlIdx].bEnableVqe);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    pstParam->stAcapAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
    MLOGD("    samplerate[%d],bitwidth[%d:0-8,1-16,2-24] soundMode[%d:0-left,1-right,2-stereo]\n",
        pstParam->stAcapAttr.enSampleRate, pstParam->stAcapAttr.enBitwidth,
        pstParam->stAcapAttr.enSoundMode);
    MLOGD("    resamplerate[%d] acodec_mixer[%d:0-IN0,1-IN1,2-IN_D] ptNumPerFrm[%u]\n",
        pstParam->stAcapAttr.enResampleRate, pstParam->stAcapAttr.enMixerMicMode,
        pstParam->stAcapAttr.u32PtNumPerFrm);

    for (s32ChnlIdx = 0; s32ChnlIdx < HI_PDT_MEDIA_ACAP_CHNL_MAX_CNT; ++s32ChnlIdx)
    {
        MLOGD("AcapChn%d :hadle[%d], enable[%d], enableVqe[%d]\n",s32ChnlIdx, pstParam->AcapChnlCfg[s32ChnlIdx].AcapChnlHdl,
            pstParam->AcapChnlCfg[s32ChnlIdx].bEnable, pstParam->AcapChnlCfg[s32ChnlIdx].bEnableVqe);
    }

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAencAACCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_MAPI_AENC_ATTR_AAC_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_S32  s32Value;

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "aenc.%d.aac", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aac_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enAACType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBitRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSmpRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enBitWidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:trans_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enTransType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:band_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, &s32Value);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    pstParam->s16BandWidth = s32Value;
    MLOGD("    AAC Type[%d:0-AACLC],bitrate[%d],samplerate[%d]\n",
        pstParam->enAACType, pstParam->enBitRate, pstParam->enSmpRate);
    MLOGD("    bitwidth[%d:0-8,1-16,2-24],soundMode[%d:0-mono,1-stereo] bandwidth[%d]\n",
        pstParam->enBitWidth, pstParam->enSoundMode,pstParam->s16BandWidth);
    MLOGD("    transType[%d:0-ADTS,1-LOAS,2-LATM_MCP1]\n", pstParam->enTransType);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAencCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_AENC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "aenc.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:aenchdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AencHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acaphdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:acapchdl", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AcapChnlHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:audio_format", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->enFormat);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Aenc[%d] enable[%d] aenchdl[%d] acaphdl[%d] acapchdl[%d]  format[%d:0-aac,1-opus] ptNumPerFrm[%u]\n",
        s32Idx, pstParam->bEnable, pstParam->AencHdl, pstParam->AcapHdl,pstParam->AcapChnlHdl,
        pstParam->enFormat, pstParam->u32PtNumPerFrm);

    /* Load Aenc AAC Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAencAACCfg(pszIniModule, s32Idx, &pstParam->unAttr.stAACAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "AencAAC");

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadAoCfg(const HI_CHAR *pszIniModule, HI_S32 s32Idx,
                HI_PDT_MEDIA_AO_CFG_S *pstParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_CHAR szIniNodePrefix[PDT_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(szIniNodePrefix, PDT_INIPARAM_NODE_NAME_LEN, "ao.%d", s32Idx);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:enable", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->bEnable);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:handle", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->AoHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sample_rate", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enSampleRate);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    pstParam->stAoAttr.enInSampleRate = pstParam->stAoAttr.enSampleRate;

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bit_width", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enBitwidth);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:sound_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enSoundMode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("Ao[%d] enable[%d] hdl[%d]\n", s32Idx, pstParam->bEnable, pstParam->AoHdl);
    MLOGD("    samplerate[%d] insamplerate[%d] bitwidth[%d:0-8,1-16,2-24] soundMode[%d:0-left,1-right,2-stereo]\n",
        pstParam->stAoAttr.enSampleRate,pstParam->stAoAttr.enInSampleRate, pstParam->stAoAttr.enBitwidth,
        pstParam->stAoAttr.enSoundMode);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:work_mode", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, AIO_MODE_BUTT, (HI_S32*)&pstParam->stAoAttr.enWorkmode);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ptnum_per_frm", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.u32PtNumPerFrm);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:ao_i2c_type", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->stAoAttr.enI2sType);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:volume", szIniNodePrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0, (HI_S32*)&pstParam->s32Volume);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    MLOGD("    ptNumPerFrm[%u] volume[%d]\n", pstParam->stAoAttr.u32PtNumPerFrm, pstParam->s32Volume);

    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadLiveSrc(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_BIND_SRC_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:bindedmod", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->enBindedMod);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:modhdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->ModHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:chnhdl", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->ChnHdl);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD(" %s enBindedMod[%d] ModHdl[%d],  ChnHdl[%d] \n",szIniNodeName, pstParam->enBindedMod,
        pstParam->ModHdl,pstParam->ChnHdl);
    return HI_SUCCESS;
}

static HI_S32 PDT_INIPARAM_MEDIACOMM_LoadOsdCfg(const HI_CHAR *pszIniModule,
                const HI_CHAR *pszPrefix, HI_PDT_MEDIA_OSD_CFG_S *pstParam)
{
    HI_S32  s32Ret = HI_SUCCESS;
    HI_S32 i,j = 0;
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:osdcnt", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->s32OsdCnt);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:base_font_w", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stBaseFontSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:base_font_h", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stBaseFontSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:base_res_w", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stBaseImageSize.u32Width);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s:base_res_h", pszPrefix);
    s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
        (HI_S32*)&pstParam->stBaseImageSize.u32Height);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

    MLOGD("s32OsdCnt[%d] BaseFontSize[%d,%d] BaseRes[%d,%d]\n",pstParam->s32OsdCnt,pstParam->stBaseFontSize.u32Width,pstParam->stBaseFontSize.u32Height,
        pstParam->stBaseImageSize.u32Width,pstParam->stBaseImageSize.u32Height);

    for(i = 0;i < MIN(pstParam->s32OsdCnt, HI_OSD_MAX_CNT);i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:type", pszPrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, HI_OSD_TYPE_BUTT,
            (HI_S32*)&pstParam->astOsdAttr[i].stContent.enType);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
        MLOGD("osd.%d type[%d]\n",i,pstParam->astOsdAttr[i].stContent.enType);

        if(HI_OSD_TYPE_TIME == pstParam->astOsdAttr[i].stContent.enType)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:timefmt", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].stContent.enTimeFmt);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("timefmt[%d]\n",pstParam->astOsdAttr[i].stContent.enTimeFmt);
        }

        if(HI_OSD_TYPE_STRING == pstParam->astOsdAttr[i].stContent.enType)
        {
            HI_CHAR *pszString = HI_NULL;
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:string", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetString(PDT_INIPARAM, pszIniModule, szIniNodeName,NULL,&pszString);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            if (NULL != pszString)
            {
                snprintf(pstParam->astOsdAttr[i].stContent.szStr, HI_OSD_MAX_STR_LEN, "%s", pszString);
                HI_APPCOMM_SAFE_FREE(pszString);
            }
            MLOGD("string[%s]\n",pstParam->astOsdAttr[i].stContent.szStr);
        }

        if(HI_OSD_TYPE_BITMAP != pstParam->astOsdAttr[i].stContent.enType)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:color", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].stContent.u32Color);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:bg_color", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].stContent.u32BgColor);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            MLOGD("color[%d] bg_color[%d] font_width[%d] font_height[%d]\n",
                pstParam->astOsdAttr[i].stContent.u32Color,pstParam->astOsdAttr[i].stContent.u32BgColor,
                pstParam->astOsdAttr[i].stContent.stFontSize.u32Width,pstParam->astOsdAttr[i].stContent.stFontSize.u32Height);
        }
        else
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:bmp_width", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].stContent.stBitmap.u32Width);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:bmp_height", pszPrefix,i);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].stContent.stBitmap.u32Height);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
            MLOGD("bmp_width[%d] bmp_height[%d]\n",pstParam->astOsdAttr[i].stContent.stBitmap.u32Width,pstParam->astOsdAttr[i].stContent.stBitmap.u32Height);
        }

        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d:dispnum", pszPrefix,i);
        s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
            (HI_S32*)&pstParam->astOsdAttr[i].u32DispNum);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

        MLOGD("dispnum[%d]\n",pstParam->astOsdAttr[i].u32DispNum);

        for(j = 0;j < MIN(pstParam->astOsdAttr[i].u32DispNum, HI_OSD_MAX_DISP_CNT);j++)
        {
            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:show", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].bShow);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:bind_module", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].enBindedMod);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:modhdl", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].ModHdl);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:chnhdl", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].ChnHdl);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:start_x", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].stStartPos.s32X);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:start_y", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].stStartPos.s32Y);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:fg_alpha", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].u32FgAlpha);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:bg_alpha", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].u32BgAlpha);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);

            snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "%s.%d.display.%d:batch", pszPrefix,i,j);
            s32Ret = HI_CONFACCESS_GetInt(PDT_INIPARAM, pszIniModule, szIniNodeName, 0,
                (HI_S32*)&pstParam->astOsdAttr[i].astDispAttr[j].s32Batch);
            PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);


            MLOGD("osd.%d.display.%d show[%d] bind_module[%d] modhdl[%d] chnhdl[%d] (%d,%d) fgalpha[%d] bgalpha[%d] batch[%d]\n",i,j,
                pstParam->astOsdAttr[i].astDispAttr[j].bShow,pstParam->astOsdAttr[i].astDispAttr[j].enBindedMod,
                pstParam->astOsdAttr[i].astDispAttr[j].ModHdl,pstParam->astOsdAttr[i].astDispAttr[j].ChnHdl,
                pstParam->astOsdAttr[i].astDispAttr[j].stStartPos.s32X,pstParam->astOsdAttr[i].astDispAttr[j].stStartPos.s32Y,
                pstParam->astOsdAttr[i].astDispAttr[j].u32FgAlpha,pstParam->astOsdAttr[i].astDispAttr[j].u32BgAlpha,
                pstParam->astOsdAttr[i].astDispAttr[j].s32Batch);
        }
    }

    return HI_SUCCESS;
}

HI_S32 HI_PDT_INIPARAM_LoadMediaCommConf(PDT_PARAM_MEDIA_COMM_CFG_S *pstParam)
{
    HI_APPCOMM_CHECK_POINTER(pstParam, HI_FAILURE);
    HI_S32 i = 0;
    HI_S32  s32Ret = HI_SUCCESS;
    HI_CHAR szIniModuleName[PDT_INIPARAM_MODULE_NAME_LEN] = {0,};
    HI_CHAR szIniNodeName[PDT_INIPARAM_NODE_NAME_LEN] = {0,};
    HI_U32  s32Idx = 0;

    snprintf(szIniModuleName, PDT_INIPARAM_MODULE_NAME_LEN, "%s", PDT_INIPARAM_MEDIA_COMM);

    /* Load VideoEnc Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(szIniModuleName, "main", &pstParam->stMainVideoVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "MainVideoEnc");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVideoEncAttr(szIniModuleName, "sub", &pstParam->stSubVideoVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SubVideoEnc");

    /* Load SnapEnc Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(szIniModuleName, "single", &pstParam->stSingleSnapVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SingSnapAttr");
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadSnapEncAttr(szIniModuleName, "sub", &pstParam->stSubSnapVencAttr);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "SubSnapAttr");

    /* Load Venc HierarchicalQp Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadVencHierarchicalQp(szIniModuleName, &pstParam->stHierarchicalQp);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "HierarchicalQp");

    /* Load venc live Configure */
    for(i = 0;i < HI_PDT_MEDIA_VCAP_DEV_MAX_CNT;i++)
    {
        snprintf(szIniNodeName, PDT_INIPARAM_NODE_NAME_LEN, "venc.live.src.%d", i);
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadLiveSrc(szIniModuleName, szIniNodeName, &pstParam->astLiveSrc[i]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, szIniNodeName);
    }

    /* Load OSD Configure */
    s32Ret = PDT_INIPARAM_MEDIACOMM_LoadOsdCfg(szIniModuleName, "osd",&pstParam->stOsdCfg);
    PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "osd");

    /* Load Display Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_DISP_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadDispCfg(szIniModuleName, s32Idx, &pstParam->astDispCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Vdisp");
    }

    /* Load Acapture Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_ACAP_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAcapCfg(szIniModuleName, s32Idx, &pstParam->astAcapCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Acap");
    }

    /* Load Aencoder Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_AENC_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAencCfg(szIniModuleName, s32Idx, &pstParam->astAencCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Aenc");
    }

    /* Load Ao Configure */
    for (s32Idx = 0; s32Idx < HI_PDT_MEDIA_AO_MAX_CNT; ++s32Idx)
    {
        s32Ret = PDT_INIPARAM_MEDIACOMM_LoadAoCfg(szIniModuleName, s32Idx, &pstParam->astAoCfg[s32Idx]);
        PDT_INIPARAM_CHECK_LOAD_RESULT(s32Ret, "Ao");
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

