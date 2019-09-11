/**
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
*
* @file      hi_mapi_aenc_adpt.h
* @brief     aenc encoder adapt module.
* @author    HiMobileCam ndk develop team
* @date      2018-2-6
*/

#ifndef __HI_MAPI_AENC_ADPT_H__
#define __HI_MAPI_AENC_ADPT_H__

#include "hi_mapi_comm_define.h"
#include "aacenc.h"
#include "hi_comm_aenc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     AENC ADPT */
/** @{ */ /** <!-- [AENC ADPT] */

/** Macro define */
#define HI_MAPI_AENC_AACLD_SAMPLES_PER_FRAME   (512)  /** samples per frame for AACLD */
#define HI_MAPI_AENC_AACLC_SAMPLES_PER_FRAME   (1024) /** samples per frame for AACLC */
#define HI_MAPI_AENC_AACPLUS_SAMPLES_PER_FRAME (2048) /** samples per frame for aacPlus */

#define HI_MAPI_AENC_MAX_AAC_MAINBUF_SIZE (768 * 2) /** max length of AAC stream by bytes */

/** Struct */
typedef enum hiMAPI_AENC_AAC_TYPE_E {
    AAC_TYPE_AACLC = 0,    /* AAC LC */
    AAC_TYPE_EAAC = 1,     /* eAAC  (HEAAC or AAC+  or aacPlusV1) */
    AAC_TYPE_EAACPLUS = 2, /* eAAC+ (AAC++ or aacPlusV2) */
    AAC_TYPE_AACLD = 3,
    AAC_TYPE_AACELD = 4,
    AAC_TYPE_BUTT,
} HI_MAPI_AENC_AAC_TYPE_E;

typedef enum hiMAPI_AENC_AAC_BPS_E {
    AAC_BPS_16K = 16000,
    AAC_BPS_22K = 22000,
    AAC_BPS_24K = 24000,
    AAC_BPS_32K = 32000,
    AAC_BPS_48K = 48000,
    AAC_BPS_64K = 64000,
    AAC_BPS_96K = 96000,
    AAC_BPS_128K = 128000,
    AAC_BPS_256K = 256000,
    AAC_BPS_320K = 320000,
    AAC_BPS_BUTT
} HI_MAPI_AENC_AAC_BPS_E;

/*
    AAC Commendatory Parameter:
    Sampling Rate(HZ)   LC BitRate(Kbit/s)  EAAC BitRate (Kbit/s)   EAAC+ BitRate (Kbit/s)
    48000               128                 48                      32  24
    44100               128                 48                      32  24
    32000               96                  22                      16
    24000               64
    22050               64
    16000               48
*/
typedef enum hiMAPI_AENC_AAC_TRANS_TYPE_E {
    AAC_TRANS_TYPE_ADTS = 0,      /**< ADTS */
    AAC_TRANS_TYPE_LOAS = 1,      /**< LOAS */
    AAC_TRANS_TYPE_LATM_MCP1 = 2, /**< LATM_MCP1 */
    AAC_TRANS_TYPE_BUTT
} HI_MAPI_AENC_AAC_TRANS_TYPE_E;

typedef struct hiMAPI_AENC_ATTR_AAC_S {
    HI_MAPI_AENC_AAC_TYPE_E enAACType;         /**< AAC profile type */
    HI_MAPI_AENC_AAC_BPS_E enBitRate;          /**< AAC bitrate (LC:48~128, EAAC:22~48, EAAC+:16~32) */
    AUDIO_SAMPLE_RATE_E enSmpRate;             /**< AAC sample rate (LC:16~48, EAAC:32~48, EAAC+:32~48) */
    AUDIO_BIT_WIDTH_E enBitWidth;              /**< AAC bit width (only support 16bit) */
    AUDIO_SOUND_MODE_E enSoundMode;            /**< sound type of inferent audio frame */
    HI_MAPI_AENC_AAC_TRANS_TYPE_E enTransType; /**< AAC Trans type */
    HI_S16 s16BandWidth; /**< targeted audio bandwidth in Hz (0 or 1000~enSmpRate/2), the default is 0 */
} HI_MAPI_AENC_ATTR_AAC_S;

/** @} */ /** <!-- ==== AENC ADPT End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
