/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file      adpt_aenc_aac.c
* @brief     anec aac encoder adapter module.
* @author    HiMobileCam ndk develop team
* @date  2019-3-26
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "adpt_aenc_aac.h"
#include "mapi_aenc_inner.h"
#include "mapi_comm_inner.h"


HI_S32 AACTypeConvert(HI_MAPI_AENC_AAC_TYPE_E enAACType, AuEncoderFormat *encoderFormat)
{
    switch (enAACType) {
        case AAC_TYPE_AACLC:
            *encoderFormat = AACLC;
            break;

        case AAC_TYPE_EAAC:
            *encoderFormat = EAAC;
            break;

        case AAC_TYPE_EAACPLUS:
            *encoderFormat = EAACPLUS;
            break;

        case AAC_TYPE_AACLD:
            *encoderFormat = AACLD;
            break;

        case AAC_TYPE_AACELD:
            *encoderFormat = AACELD;
            break;

        default:
            return HI_ERR_AENC_ILLEGAL_PARAM;
    }
    return HI_SUCCESS;
}

static HI_S32 AencCheckAACAttr(const HI_MAPI_AENC_ATTR_AAC_S *pstAACAttr)
{
    if (pstAACAttr->enBitWidth != AUDIO_BIT_WIDTH_16) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid bitwidth for AAC encoder\n");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pstAACAttr->enSoundMode >= AUDIO_SOUND_MODE_BUTT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid sound mode for AAC encoder\n");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if ((pstAACAttr->enAACType == AAC_TYPE_EAACPLUS) && (pstAACAttr->enSoundMode != AUDIO_SOUND_MODE_STEREO)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid sound mode for AAC encoder\n");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pstAACAttr->enAACType >= AAC_TYPE_BUTT) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid AACType for AAC encoder\n");
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pstAACAttr->enTransType == AAC_TRANS_TYPE_ADTS) {
        if ((pstAACAttr->enAACType == AAC_TYPE_AACLD)
            || (pstAACAttr->enAACType == AAC_TYPE_AACELD)) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD or AACELD not support AAC_TRANS_TYPE_ADTS\n");
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    }

    return HI_SUCCESS;
}

static HI_S32 AencCheckAACLCConfig(const AACENC_CONFIG *pconfig)
{
    HI_S32 s32MinBitRate = 0;
    HI_S32 s32MaxBitRate = 0;

    if (pconfig->coderFormat == AACLC) {
        if (pconfig->nChannelsOut != pconfig->nChannelsIn) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC nChannelsOut(%d) in not equal to nChannelsIn(%d)\n",
                           pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

        if (pconfig->sampleRate == 32000) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 192000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 32000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 44100) {
            s32MinBitRate = 48000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 265000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 44100 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 48000) {
            s32MinBitRate = 48000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 288000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 48000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 16000) {
            s32MinBitRate = 24000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 96000 : 192000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 16000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 8000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 48000 : 96000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 8000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 24000) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 144000 : 288000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 24000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 22050) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 132000 : 265000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC 22050 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLC invalid samplerate(%d)\n", pconfig->sampleRate);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    } else {
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 AencCheckEAACConfig(const AACENC_CONFIG *pconfig)
{
    HI_S32 s32MinBitRate = 0;
    HI_S32 s32MaxBitRate = 0;

    if (pconfig->coderFormat == EAAC) {
        if (pconfig->nChannelsOut != pconfig->nChannelsIn) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC nChannelsOut(%d) is not equal to nChannelsIn(%d)\n",
                           pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

        if (pconfig->sampleRate == 32000) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 64000 : 128000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 32000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 44100) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 64000 : 128000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 44100 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 48000) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 64000 : 128000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 48000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 16000) {
            s32MinBitRate = 24000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 48000 : 96000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 16000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 22050) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 64000 : 128000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 22050 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 24000) {
            s32MinBitRate = 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 64000 : 128000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC 24000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAAC invalid samplerate(%d)\n", pconfig->sampleRate);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    } else {
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 AencCheckEAACPLUSConfig(const AACENC_CONFIG *pconfig)
{
    HI_S32 s32MinBitRate = 0;
    HI_S32 s32MaxBitRate = 0;

    if (pconfig->coderFormat == EAACPLUS) {
        if (pconfig->nChannelsOut != 2 || pconfig->nChannelsIn != 2) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS nChannelsOut(%d) and nChannelsIn(%d) should be 2\n",
                           pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

        if (pconfig->sampleRate == 32000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 64000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 32000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 44100) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 64000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 44100 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 48000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 64000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 48000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 16000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 48000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 16000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 22050) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 64000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 22050 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 24000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = 64000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS 24000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "EAACPLUS invalid samplerate(%d)\n", pconfig->sampleRate);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

    } else {
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 AencCheckAACLDConfig(const AACENC_CONFIG *pconfig)
{
    HI_S32 s32MinBitRate = 0;
    HI_S32 s32MaxBitRate = 0;

    if (pconfig->coderFormat == AACLD) {
        if (pconfig->nChannelsOut != pconfig->nChannelsIn) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD nChannelsOut(%d) in not equal to nChannelsIn(%d)\n",
                           pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

        if (pconfig->sampleRate == 32000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 48000 : 64000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 32000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 44100) {
            s32MinBitRate = 64000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 44100 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 48000) {
            s32MinBitRate = 64000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 48000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 16000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 24000 : 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 192000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 16000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 8000) {
            s32MinBitRate = 16000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 96000 : 192000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 8000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 24000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 32000 : 48000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 256000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 24000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 22050) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 32000 : 48000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 256000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD 22050 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACLD invalid samplerate(%d)\n", pconfig->sampleRate);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

    } else {
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

static HI_S32 AencCheckAACELDConfig(const AACENC_CONFIG *pconfig)
{
    HI_S32 s32MinBitRate = 0;
    HI_S32 s32MaxBitRate = 0;

    if (pconfig->coderFormat == AACELD) {
        if (pconfig->nChannelsOut != pconfig->nChannelsIn) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD nChannelsOut(%d) in not equal to nChannelsIn(%d)\n",
                           pconfig->nChannelsOut, pconfig->nChannelsIn);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

        if (pconfig->sampleRate == 32000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 32000 : 64000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 32000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 44100) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 96000 : 192000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 44100 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 48000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 96000 : 192000;
            s32MaxBitRate = 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 48000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 16000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 16000 : 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 256000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 16000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 8000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 32000 : 64000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 96000 : 192000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 8000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 24000) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 24000 : 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 256000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 24000 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else if (pconfig->sampleRate == 22050) {
            s32MinBitRate = (pconfig->nChannelsIn == 1) ? 24000 : 32000;
            s32MaxBitRate = (pconfig->nChannelsIn == 1) ? 256000 : 320000;
            if (pconfig->bitRate < s32MinBitRate || pconfig->bitRate > s32MaxBitRate) {
                MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD 22050 Hz bitRate(%d) should be %d ~ %d\n", pconfig->bitRate,
                               s32MinBitRate, s32MaxBitRate);
                return HI_ERR_AENC_ILLEGAL_PARAM;
            }
        } else {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACELD invalid samplerate(%d)\n", pconfig->sampleRate);
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }

    } else {
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    return HI_SUCCESS;
}

HI_S32 AencAACCheckConfig(AACENC_CONFIG *pconfig)
{
    HI_S32 s32Ret;

    if (pconfig == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "pconfig is null\n");
        return HI_ERR_AENC_NULL_PTR;
    }

    if (pconfig->coderFormat != AACLC && pconfig->coderFormat != EAAC && pconfig->coderFormat != EAACPLUS &&
        pconfig->coderFormat != AACLD && pconfig->coderFormat != AACELD) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aacenc coderFormat(%d) invalid\n", pconfig->coderFormat);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pconfig->quality != AU_QualityExcellent && pconfig->quality != AU_QualityHigh &&
        pconfig->quality != AU_QualityMedium && pconfig->quality != AU_QualityLow) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aacenc quality(%d) invalid\n", pconfig->quality);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pconfig->bitsPerSample != 16) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "aacenc bitsPerSample(%d) should be 16\n", pconfig->bitsPerSample);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pconfig->transtype > 2) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid transtype(%d), not in [0, 2]\n", pconfig->transtype);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pconfig->bandWidth != 0 && (pconfig->bandWidth < 1000 || pconfig->bandWidth > pconfig->sampleRate / 2)) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AAC bandWidth(%d) should be 0, or 1000 ~ %d\n", pconfig->bandWidth,
                       pconfig->sampleRate / 2);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    if (pconfig->coderFormat == AACLC) {
        s32Ret = AencCheckAACLCConfig(pconfig);
    } else if (pconfig->coderFormat == EAAC) {
        s32Ret = AencCheckEAACConfig(pconfig);
    } else if (pconfig->coderFormat == EAACPLUS) {
        s32Ret = AencCheckEAACPLUSConfig(pconfig);
    }
#if 1
    else if (pconfig->coderFormat == AACLD) {
        s32Ret = AencCheckAACLDConfig(pconfig);
    } else if (pconfig->coderFormat == AACELD) {
        s32Ret = AencCheckAACELDConfig(pconfig);
    }
#endif

    return s32Ret;
}

HI_S32 OpenAACEncoder(HI_VOID *pEncoderAttr, HI_VOID **ppEncoder)
{
    AENC_AAC_ENCODER_S *pstEncoder = NULL;
    HI_MAPI_AENC_ATTR_AAC_S *pstAttr = NULL;
    HI_S32 s32Ret;
    AACENC_CONFIG config;

    HI_ASSERT(pEncoderAttr != NULL);
    HI_ASSERT(ppEncoder != NULL);

    /* check attribute of encoder */
    pstAttr = (HI_MAPI_AENC_ATTR_AAC_S *)pEncoderAttr;
    s32Ret = AencCheckAACAttr(pstAttr);
    if (s32Ret) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "s32Ret:0x%x.#########\n", s32Ret);
        return s32Ret;
    }

    /* allocate memory for encoder */
    pstEncoder = (AENC_AAC_ENCODER_S *)malloc(sizeof(AENC_AAC_ENCODER_S));
    if (pstEncoder == NULL) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "no memory\n");
        return HI_ERR_AENC_NOMEM;
    }
    memset(pstEncoder, 0, sizeof(AENC_AAC_ENCODER_S));
    *ppEncoder = (HI_VOID *)pstEncoder;

    /* set default config to encoder */
    s32Ret = AACInitDefaultConfig(&config);
    if (s32Ret) {
        free(pstEncoder);
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "s32Ret:0x%x.#########\n", s32Ret);
        return s32Ret;
    }

    s32Ret = AACTypeConvert(pstAttr->enAACType, &config.coderFormat);
    if (s32Ret != HI_SUCCESS) {
        free(pstEncoder);
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AACTypeConvert error s32Ret:0x%x.#########\n", s32Ret);
        return s32Ret;
    }

    config.bitRate = pstAttr->enBitRate;
    config.bitsPerSample = 8 * (1 << (pstAttr->enBitWidth));
    config.sampleRate = pstAttr->enSmpRate;
    config.bandWidth = pstAttr->s16BandWidth;  // config.sampleRate/2;

    switch (pstAttr->enTransType) {
        case AAC_TRANS_TYPE_ADTS:
            config.transtype = AACENC_ADTS;
            break;

        case AAC_TRANS_TYPE_LOAS:
            config.transtype = AACENC_LOAS;
            break;

        case AAC_TRANS_TYPE_LATM_MCP1:
            config.transtype = AACENC_LATM_MCP1;
            break;

        default:
            break;
    }

    if (pstAttr->enSoundMode == AUDIO_SOUND_MODE_MONO && pstAttr->enAACType != AAC_TYPE_EAACPLUS) {
        config.nChannelsIn = 1;
        config.nChannelsOut = 1;
    } else {
        config.nChannelsIn = 2;
        config.nChannelsOut = 2;
    }

    config.quality = AU_QualityHigh;
    s32Ret = AencAACCheckConfig(&config);
    if (s32Ret) {
        free(pstEncoder);
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "s32Ret:0x%x.#########\n", s32Ret);
        return HI_ERR_AENC_ILLEGAL_PARAM;
    }

    /* create encoder */
    s32Ret = AACEncoderOpen(&pstEncoder->pstAACState, &config);
    if (s32Ret) {
        free(pstEncoder);
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "s32Ret:0x%x.#########\n", s32Ret);
        return s32Ret;
    }

    memcpy(&pstEncoder->stAACAttr, pstAttr, sizeof(HI_MAPI_AENC_ATTR_AAC_S));

    return HI_SUCCESS;
}

HI_S32 EncodeAACFrm(HI_VOID *pEncoder, const AUDIO_FRAME_S *pstData,
                    HI_U8 *pu8Outbuf, HI_U32 *pu32OutLen)
{
    HI_S32 s32Ret;
    AENC_AAC_ENCODER_S *pstEncoder = NULL;
    HI_U32 u32PtNums;
    HI_S32 i;
    HI_S16 aData[AACENC_BLOCKSIZE * 2 * MAX_CHANNELS];
    HI_S16 s16Len = 0;

    HI_U32 u32WaterLine;

    HI_ASSERT(pEncoder != NULL);
    HI_ASSERT(pstData != NULL);
    HI_ASSERT(pu8Outbuf != NULL);
    HI_ASSERT(pu32OutLen != NULL);

    pstEncoder = (AENC_AAC_ENCODER_S *)pEncoder;

    if (pstEncoder->stAACAttr.enSoundMode == AUDIO_SOUND_MODE_STEREO) {
        /* whether the sound mode of frame and channel is match  */
        if (pstData->enSoundmode != AUDIO_SOUND_MODE_STEREO) {
            MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AAC encode receive a frame which not match its Soundmode\n");
            return HI_ERR_AENC_ILLEGAL_PARAM;
        }
    }

    /* WaterLine, equals to the frame sample frame of protocol */
    if (pstEncoder->stAACAttr.enAACType == AAC_TYPE_AACLC) {
        u32WaterLine = HI_MAPI_AENC_AACLC_SAMPLES_PER_FRAME;
    } else if (pstEncoder->stAACAttr.enAACType == AAC_TYPE_EAAC
               || pstEncoder->stAACAttr.enAACType == AAC_TYPE_EAACPLUS) {
        u32WaterLine = HI_MAPI_AENC_AACPLUS_SAMPLES_PER_FRAME;
    } else if (pstEncoder->stAACAttr.enAACType == AAC_TYPE_AACLD
               || pstEncoder->stAACAttr.enAACType == AAC_TYPE_AACELD) {
        u32WaterLine = HI_MAPI_AENC_AACLD_SAMPLES_PER_FRAME;
    } else {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid AAC coder type\n");
        return HI_ERR_AENC_ILLEGAL_PARAM;
        ;
    }
    /* calculate point number */
    u32PtNums = pstData->u32Len / (pstData->enBitwidth + 1);

    /* if frame sample larger than protocol sample, reject to receive, or buffer will be overflow */
    if (u32PtNums != u32WaterLine) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "invalid u32PtNums%d for AACType:%d\n", u32PtNums,
                       pstEncoder->stAACAttr.enAACType);
        return HI_ERR_AENC_ILLEGAL_PARAM;
        ;
    }

    /* AAC encoder need interleaved data,here change LLLRRR to LRLRLR.
       AACLC will encode 1024*2 point, and AACplus encode 2048*2 point */
    if (pstEncoder->stAACAttr.enSoundMode == AUDIO_SOUND_MODE_STEREO) {
        s16Len = u32WaterLine;
        for (i = s16Len - 1; i >= 0; i--) {
            aData[2 * i] = *((HI_S16 *)pstData->u64VirAddr[0] + i);
            aData[2 * i + 1] = *((HI_S16 *)pstData->u64VirAddr[1] + i);
        }

    } else { /* if inbuf is momo, copy left to right */
        HI_S16 *temp = (HI_S16 *)pstData->u64VirAddr[0];

        s16Len = u32WaterLine;
        for (i = s16Len - 1; i >= 0; i--) {
            aData[i] = *(temp + i);
        }
    }

    s32Ret = AACEncoderFrame(pstEncoder->pstAACState, aData, pu8Outbuf, (HI_S32 *)pu32OutLen);
    if (s32Ret != HI_SUCCESS) {
        MAPI_ERR_TRACE(HI_MAPI_MOD_AENC, "AAC encode failed\n");
    }

    return s32Ret;
}

HI_S32 CloseAACEncoder(HI_VOID *pEncoder)
{
    AENC_AAC_ENCODER_S *pstEncoder = NULL;

    HI_ASSERT(pEncoder != NULL);
    pstEncoder = (AENC_AAC_ENCODER_S *)pEncoder;

    AACEncoderClose(pstEncoder->pstAACState);

    free(pstEncoder);
    return HI_SUCCESS;
}

HI_S32 ADPT_AENC_GetAacEncoder(AENC_ENCODER_S *pstAacEncoder)
{
    CHECK_MAPI_AENC_NULL_PTR(pstAacEncoder);

    pstAacEncoder->enType = PT_AAC;
    snprintf(pstAacEncoder->aszName, 3, "Aac");
    pstAacEncoder->u32MaxFrmLen = HI_MAPI_AENC_MAX_AAC_MAINBUF_SIZE;
    pstAacEncoder->pfnOpenEncoder = OpenAACEncoder;
    pstAacEncoder->pfnEncodeFrm = EncodeAACFrm;
    pstAacEncoder->pfnCloseEncoder = CloseAACEncoder;

    return HI_SUCCESS;
}
