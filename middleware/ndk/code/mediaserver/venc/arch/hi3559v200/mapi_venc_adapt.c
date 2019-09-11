/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_venc_adapt.c
 * @brief   venc adapt module function
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#include "mpi_venc.h"

#include "hi_mapi_comm_define.h"
#include "hi_mapi_venc_define.h"
#include "mapi_venc_inner.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

HI_S32 HAL_MAPI_VENC_ConfigH265CarTrans(HI_HANDLE VencHdl, HI_MAPI_PAYLOAD_TYPE_E enType,
                                        HI_MAPI_VENC_SCENE_MODE_E enSceneMode)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VENC_H265_TRANS_S stH265Trans;

    if ((enType == HI_MAPI_PAYLOAD_TYPE_H265) && (enSceneMode == HI_MAPI_VENC_SCENE_MODE_CAR)) {
        s32Ret = HI_MPI_VENC_GetH265Trans(VencHdl, &stH265Trans);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_GetH265Trans fail,Ret:%x\n", s32Ret);
            return s32Ret;
        }

        stH265Trans.bScalingListEnabled = HI_TRUE;
        s32Ret = HI_MPI_VENC_SetH265Trans(VencHdl, &stH265Trans);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_VENC, "HI_MPI_VENC_SetH265Trans fail,Ret:%x\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_VENC_ConfigH265Attr(const HI_MAPI_VENC_RC_ATTR_S *pstRcAttr,
                                    VENC_CHN_ATTR_S *pstVencChnAttr)
{
    CHECK_MAPI_VENC_NULL_PTR(pstRcAttr);
    CHECK_MAPI_VENC_NULL_PTR(pstVencChnAttr);

    switch (pstRcAttr->enRcMode) {
        case HI_MAPI_VENC_RC_MODE_CBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH265Cbr, &pstRcAttr->stAttrCbr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
            break;
        }

        case HI_MAPI_VENC_RC_MODE_VBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH265Vbr, &pstRcAttr->stAttrVbr, sizeof(HI_MAPI_VENC_ATTR_VBR_S));
            break;
        }

        case HI_MAPI_VENC_RC_MODE_QVBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QVBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH265QVbr, &pstRcAttr->stAttrQVbr, sizeof(HI_MAPI_VENC_ATTR_QVBR_S));
            break;
        }

        default:
        {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH265Cbr, &pstRcAttr->stAttrCbr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
            break;
        }
    }

    pstVencChnAttr->stVencAttr.stAttrH265e.bRcnRefShareBuf = HI_TRUE;

    return HI_SUCCESS;
}

HI_S32 HAL_MAPI_VENC_ConfigH264Attr(const HI_MAPI_VENC_RC_ATTR_S *pstRcAttr,
                                    VENC_CHN_ATTR_S *pstVencChnAttr)
{
    CHECK_MAPI_VENC_NULL_PTR(pstRcAttr);
    CHECK_MAPI_VENC_NULL_PTR(pstVencChnAttr);

    switch (pstRcAttr->enRcMode) {
        case HI_MAPI_VENC_RC_MODE_CBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH264Cbr, &pstRcAttr->stAttrCbr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
            break;
        }

        case HI_MAPI_VENC_RC_MODE_VBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH264Vbr, &pstRcAttr->stAttrVbr, sizeof(HI_MAPI_VENC_ATTR_VBR_S));
            break;
        }

        case HI_MAPI_VENC_RC_MODE_QVBR: {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QVBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH264QVbr, &pstRcAttr->stAttrQVbr, sizeof(HI_MAPI_VENC_ATTR_QVBR_S));
            break;
        }

        default:
        {
            pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
            memcpy(&pstVencChnAttr->stRcAttr.stH264Cbr, &pstRcAttr->stAttrCbr, sizeof(HI_MAPI_VENC_ATTR_CBR_S));
            break;
        }
    }

    pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
