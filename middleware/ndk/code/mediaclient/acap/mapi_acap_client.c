/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    mapi_acap_client.c
 * @brief   NDK acap client functions
 * @author  HiMobileCam NDK develop team
 * @date  2018-2-6
 */
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "hi_type.h"
#include "hi_mapi_acap.h"
#include "mapi_acap_inner.h"
#include "hi_ipcmsg.h"
#include "msg_acap.h"
#include "msg_define.h"
#include "msg_wrapper.h"
#include "mpi_sys.h"
#include "mapi_comm_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static MAPI_ACAP_CONTEXT_S g_astAcapContext[HI_MAPI_ACAP_DEV_MAX_NUM]; /* for resample save info */

HI_S32 MAPI_ACAP_GetChnNum(HI_MAPI_AUDIO_SOUND_MODE_E enSoundMode, HI_S32 *pChnNum)
{
    CHECK_MAPI_ACAP_NULL_PTR(pChnNum);

    switch (enSoundMode) {
        case HI_MAPI_AUDIO_SOUND_MODE_STEREO:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_DOUBLE_RIGHT:
            *pChnNum = 2;
            break;
        case HI_MAPI_AUDIO_SOUND_MODE_LEFT:
        case HI_MAPI_AUDIO_SOUND_MODE_RIGHT:
            *pChnNum = 1;
            break;
        default:
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "not support sound mode\n");
            return HI_MAPI_ACAP_EILLPARAM;
    }
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Init(HI_HANDLE AcapHdl, const HI_MAPI_ACAP_ATTR_S *pstACapAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_MAPI_ACAP_ATTR_S stACapAttr;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstACapAttr);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    memcpy(&stACapAttr, pstACapAttr, sizeof(HI_MAPI_ACAP_ATTR_S));
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_INIT, &stACapAttr, sizeof(HI_MAPI_ACAP_ATTR_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_INIT fail.\n");
        return s32Ret;
    }

    /* creat resampler */
    if (pstACapAttr->enResampleRate != pstACapAttr->enSampleRate) {
        MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "Resampler Create.\n");
        g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate = pstACapAttr->enSampleRate;
        g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate = pstACapAttr->enResampleRate;
    }

    g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx = pstACapAttr->enSoundMode;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Deinit(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_DEINIT, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_DEINIT.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Start(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_START, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_START fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Stop(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_STOP, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_STOP fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_EnableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_ENABLE_VQE, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_ENABLE_VQE fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_DisableVqe(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_DISABLE_VQE, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_DISABLE_VQE fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_SetVolume(HI_HANDLE AcapHdl, HI_S32 s32AudioGain)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_S32 s32Gain = s32AudioGain;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_SET_VOL, &s32Gain, sizeof(HI_S32), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_SET_VOL fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_GetVolume(HI_HANDLE AcapHdl, HI_S32 *ps32AudioGain)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_S32 s32Gain = 0;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_NULL_PTR(ps32AudioGain);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_GET_VOL, &s32Gain, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_GET_VOL fail.\n");
        return s32Ret;
    }
    *ps32AudioGain = s32Gain;

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Mute(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_MUTE, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_MUTE fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_Unmute(HI_HANDLE AcapHdl)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, 0);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_UNMUTE, HI_NULL, 0, HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_UNMUTE fail.\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_GetFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, AUDIO_FRAME_S *pstFrm,
                             AEC_FRAME_S *pstAecFrm)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    HI_U32 u32InSamples = 0;
    HI_U32 u32OutSamples = 0;
    HI_S32 s32ChnNum = 0;
    MSG_ACAP_FRAME_S stAcapFrm;

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstFrm);
    CHECK_MAPI_ACAP_NULL_PTR(pstAecFrm);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    s32Ret = MAPI_ACAP_GetChnNum(g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx, &s32ChnNum);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MAPI_ACAP_GetChnNum fail.\n");
        return s32Ret;
    }

    /* send ipmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_GET_FRAME, &stAcapFrm, sizeof(MSG_ACAP_FRAME_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_GET_FRAME fail.\n");
        return s32Ret;
    }

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "get frame phyAddr = 0x%llx , 0x%llx , frame len = %u \n",
                     stAcapFrm.stFrm.u64PhyAddr[0], stAcapFrm.stFrm.u64PhyAddr[1], stAcapFrm.stFrm.u32Len);

    /* mmap frame
        now use record vqe which not have aec config , so can not get aec frame */
    memcpy(pstFrm, &(stAcapFrm.stFrm), sizeof(AUDIO_FRAME_S));
    if (g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate != g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate) {
        pstFrm->u64VirAddr[0] = (HI_U8 *)HI_MPI_SYS_Mmap((HI_U64)(HI_UL)pstFrm->u64VirAddr[0],
            (sizeof(HI_U16) * HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM * 6));
        if (pstFrm->u64VirAddr[0] == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "pstFrm->u64VirAddr[0]:0x%p mmap error!\n", pstFrm->u64VirAddr[0]);
            return HI_FAILURE;
        }

        if (s32ChnNum == 2) {
            pstFrm->u64VirAddr[1] = pstFrm->u64VirAddr[0] + pstFrm->u32Len;
        }
    } else {
        pstFrm->u64VirAddr[0] = (HI_U8 *)HI_MPI_SYS_Mmap(pstFrm->u64PhyAddr[0], pstFrm->u32Len);
        if (pstFrm->u64VirAddr[0] == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "pstFrm->u64PhyAddr[0]:0x%llx mmap error!\n", pstFrm->u64PhyAddr[0]);
            return HI_FAILURE;
        }

        if (s32ChnNum == 2) {
            pstFrm->u64VirAddr[1] = (HI_U8 *)HI_MPI_SYS_Mmap(pstFrm->u64PhyAddr[1], pstFrm->u32Len);
            if (pstFrm->u64VirAddr[1] == HI_NULL) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "pstFrm->u64PhyAddr[1]:0x%llx mmap error!\n", pstFrm->u64PhyAddr[1]);
                return HI_FAILURE;
            }
        }
    }

    if (stAcapFrm.stAecFrm.bValid == HI_TRUE) {
        memcpy(pstAecFrm, &(stAcapFrm.stAecFrm), sizeof(AEC_FRAME_S));
        pstAecFrm->stRefFrame.u64VirAddr[0] = (HI_U8 *)HI_MPI_SYS_Mmap(pstAecFrm->stRefFrame.u64PhyAddr[0],
                                                                       pstAecFrm->stRefFrame.u32Len);
        if (pstAecFrm->stRefFrame.u64VirAddr[0] == HI_NULL) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "pstAecFrm->stRefFrame.u64PhyAddr[0]:0x%llx mmap error!\n",
                           pstAecFrm->stRefFrame.u64PhyAddr[0]);
            return HI_FAILURE;
        }

        if (s32ChnNum == 2) {
            pstAecFrm->stRefFrame.u64VirAddr[1] = (HI_U8 *)HI_MPI_SYS_Mmap(pstAecFrm->stRefFrame.u64PhyAddr[1],
                                                                           pstAecFrm->stRefFrame.u32Len);
            if (pstAecFrm->stRefFrame.u64VirAddr[1] == HI_NULL) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "pstAecFrm->stRefFrame.u64PhyAddr[1]:0x%llx mmap error!\n",
                               pstAecFrm->stRefFrame.u64PhyAddr[1]);
                return HI_FAILURE;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 HI_MAPI_ACAP_ReleaseFrame(HI_HANDLE AcapHdl, HI_HANDLE AcapChnHdl, const AUDIO_FRAME_S *pstFrm,
                                 const AEC_FRAME_S *pstAecFrm)
{
    HI_S32 s32Ret;
    HI_U32 u32ModFd;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;
    MSG_ACAP_FRAME_S stAcapFrm;
    HI_S32 s32ChnNum = 0;

    MAPI_DEBUG_TRACE(HI_MAPI_MOD_ACAP, "client before check release frame \n");

    /* check attr and state */
    CHECK_MAPI_ACAP_HANDLE_RANGE(AcapHdl);
    CHECK_MAPI_ACAP_CHN_HANDLE_RANGE(AcapChnHdl);
    CHECK_MAPI_ACAP_NULL_PTR(pstFrm);
    CHECK_MAPI_ACAP_NULL_PTR(pstAecFrm);
    CHECK_MAPI_ACAP_CHECK_INIT(MAPI_GetMediaInitStatus());

    s32Ret = MAPI_ACAP_GetChnNum(g_astAcapContext[AcapHdl].stAcapAttrEx.enSoundModeEx, &s32ChnNum);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MAPI_ACAP_GetChnNum fail.\n");
        return s32Ret;
    }

    memcpy(&stFrm, pstFrm, sizeof(AUDIO_FRAME_S));
    memcpy(&stAecFrm, pstAecFrm, sizeof(AEC_FRAME_S));

    /* munmap when resample */
    if (g_astAcapContext[AcapHdl].stAcapAttr.enSamplerate != g_astAcapContext[AcapHdl].stAcapAttrEx.enResampleRate) {
        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)stFrm.u64VirAddr[0],
                                   (sizeof(HI_U16) * HI_MAPI_AIO_MAX_POINT_PER_FRAME * HI_MAPI_ACAP_CHN_MAX_NUM * 6));
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "munmap fail VirAddr:0x%p, s32Ret:0x%x\n", stFrm.u64VirAddr[0], s32Ret);
        }
    } else {
        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)stFrm.u64VirAddr[0], stFrm.u32Len);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "munmap fail stFrm.VirAddr[0]:0x%p, s32Ret:0x%x\n",
                           stFrm.u64VirAddr[0], s32Ret);
        }

        if (s32ChnNum == 2) {
            s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)stFrm.u64VirAddr[1], stFrm.u32Len);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "munmap fail stFrm.VirAddr[1]:0x%p, s32Ret:0x%x\n",
                               stFrm.u64VirAddr[1], s32Ret);
            }
        }
    }

    if (pstAecFrm->bValid == HI_TRUE) {
        s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)pstAecFrm->stRefFrame.u64VirAddr[0], pstAecFrm->stRefFrame.u32Len);
        if (s32Ret != HI_SUCCESS) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "munmap fail VirAddr[0]:0x%p, s32Ret:0x%x\n",
                           pstAecFrm->stRefFrame.u64VirAddr[0], s32Ret);
        }

        if (s32ChnNum == 2) {
            s32Ret = HI_MPI_SYS_Munmap((HI_VOID *)pstAecFrm->stRefFrame.u64VirAddr[1], pstAecFrm->stRefFrame.u32Len);
            if (s32Ret != HI_SUCCESS) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "munmap fail VirAddr[1]:%p, s32Ret:0x%x\n",
                               pstAecFrm->stRefFrame.u64VirAddr[1], s32Ret);
            }
        }
    }

    stAcapFrm.stFrm = stFrm;
    stAcapFrm.stAecFrm = stAecFrm;

    /* send ipcmsg */
    u32ModFd = MODFD(HI_MAPI_MOD_ACAP, AcapHdl, AcapChnHdl);
    s32Ret = MAPI_SendSync(u32ModFd, MSG_CMD_ACAP_RELEASE_FRAME, &stAcapFrm, sizeof(MSG_ACAP_FRAME_S), HI_NULL);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_ACAP, "MSG_CMD_ACAP_RELEASE_FRAME fail\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
