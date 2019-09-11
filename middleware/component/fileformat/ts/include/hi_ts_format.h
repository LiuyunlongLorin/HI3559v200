/**
* Copyright (C), 2017, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_ts_muxer.h
* @brief     ts muxer module header file
* @author    HiMobileCam middleware develop team
* @date      2017.06.19
*/
#ifndef __HI_TS_FORMAT_H__
#define __HI_TS_FORMAT_H__
#include "hi_error_def.h"
#include "hi_mw_type.h"
#include "hi_defs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef enum hiAPP_TS_ERR_CODE_E {
    /* general error code */
    APP_TS_ERR_HANDLE_INVALID = 0x40, /**<ts handle invalid */
    APP_TS_ERR_INVALID_ARG = 0x41,    /**<param is null or invalid */
    APP_TS_ERR_MALLOC_FAIL = 0x42,    /**<malloc memory fail */
    APP_TS_ERR_CREATE_MUXER = 0x43,   /**<create ts fail */
    APP_TS_ERR_DESTROY_MUXER = 0x44,  /**<destory ts  fail */
    APP_TS_ERR_CREATE_STREAM = 0x45,  /**<create stream fail */
    APP_TS_ERR_DESTROY_STREAM = 0x46, /**<destory stream  fail */
    APP_TS_ERR_NOT_CREATE = 0x47,     /**<ts not created */
    APP_TS_ERR_READ_FRAME = 0x48,     /**<read frame fail */
    APP_TS_ERR_WRITE_FRAME = 0x49,    /**<write frame fail */

    /* file related error code */
    APP_TS_ERR_UNSUPPORT_CODEC = 0x61, /**<not support codec */
    APP_TS_ERR_OPEN_FILE = 0x62,       /**<open file error */
    APP_TS_ERR_CLOSE_FILE = 0x63,      /**<close file error */
    APP_TS_ERR_READ_FILE = 0x64,       /**<read file error */
    APP_TS_ERR_WRITE_FILE = 0x65,      /**<write file error */
    APP_TS_ERR_FALLOCATE_FILE = 0x66,  /**<fallocate error */

    APP_TS_ERR_BUTT = 0xFF
} HI_APP_TS_ERR_CODE_E;

/* general error code */
#define HI_ERR_TS_NULL_PTR       HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_ERR_NULL_PTR)
#define HI_ERR_TS_HANDLE_INVALID HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_HANDLE_INVALID)
#define HI_ERR_TS_INVALIDARG     HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_INVALID_ARG)
#define HI_ERR_TS_MALLOC         HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_MALLOC_FAIL)
#define HI_ERR_TS_CREATE_MUXER   HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_CREATE_MUXER)
#define HI_ERR_TS_DESTROY_MUXER  HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_DESTROY_MUXER)
#define HI_ERR_TS_CREATE_STREAM  HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_CREATE_STREAM)
#define HI_ERR_TS_DESTROY_STREAM HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_DESTROY_STREAM)
#define HI_ERR_TS_NOT_CREATE     HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_NOT_CREATE)
#define HI_ERR_TS_READ_FRAME     HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_READ_FRAME)
#define HI_ERR_TS_WRITE_FRAME    HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_WRITE_FRAME)

/* FILE related error code */
#define HI_ERR_TS_UNSUPPORT_CODEC HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_UNSUPPORT_CODEC)
#define HI_ERR_TS_OPEN_FILE       HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_OPEN_FILE)
#define HI_ERR_TS_END_FILE        HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_CLOSE_FILE)
#define HI_ERR_TS_READ_FILE       HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_READ_FILE)
#define HI_ERR_TS_WRITE_FILE      HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_WRITE_FILE)
#define HI_ERR_TS_FALLOCATE_FILE  HI_APP_DEF_ERR(HI_APPID_TS, APP_ERR_LEVEL_ERROR, APP_TS_ERR_FALLOCATE_FILE)

#define MODULE_NAME_TS          "TS"
#define HI_TS_MAX_AMOUNT        (4)                 /* max ts amount */
#define HI_TS_MAX_STREAM_AMOUNT (6)                 /* max ts track amount */
#define HI_TS_MAX_FILE_NAME     (256)               /* file name path max */
#define HI_TS_VBUF_MAX_SIZE     (5 * 1024 * 1024)   /* VBUF size max */
#define HI_TS_PREALLOC_MAX_SIZE (100 * 1024 * 1024) /* pre allocate unit size max */

typedef enum hiTS_CONFIG_TYPE_E {
    HI_TS_CONFIG_MUXER = 1,
    HI_TS_CONFIG_DEMUXER,
    HI_TS_CONFIG_BUTT
} HI_TS_CONFIG_TYPE_E;

typedef enum hiTS_STREAM_TYPE_E {
    HI_TS_STREAM_VIDEO = 1,
    HI_TS_STREAM_AUDIO,
    HI_TS_STREAM_DATA,
    HI_TS_STREAM_BUTT
} HI_TS_STREAM_TYPE_E;

typedef enum hiTS_CODECID_E {
    HI_TS_CODEC_ID_H264 = 1,
    HI_TS_CODEC_ID_H265,
    HI_TS_CODEC_ID_AAC,
    HI_TS_CODEC_ID_JPEG,
    HI_TS_CODEC_ID_GPS,
    HI_TS_CODEC_ID_BUTT
} HI_TS_CODECID_E;

typedef struct hiTS_STREAM_INFO_S {
    HI_TS_CODECID_E enCodecID; /* codec type */
} HI_TS_STREAM_INFO_S;

typedef struct hiTS_FRAME_DATA_S {
    HI_U64 u64TimeStamp;   /* frame timestamp */
    HI_U8 *pu8DataBuffer;  /* frame data buffer */
    HI_U32 u32DataLength;  /* frame date len */
    HI_BOOL bKeyFrameFlag; /* key frame flag, used for video */
} HI_TS_FRAME_DATA_S;

typedef struct hiTS_MXUER_CONFIG_S {
    HI_U32 u32PreAllocUnit; /* pre allocate size in bytes, [0,100M],0 for not use pre allocate function,suggest 20M, unit :byte */
    HI_U32 u32VBufSize;     /* vbuf size for fwrite (0,5M] unit :byte */
} HI_TS_MUXER_CONFIG_S;

typedef struct hiTS_DEMUXER_CONFIG_S {
    HI_U32 u32VBufSize; /* set the vbuf size for fwrite (0,5M] unit :byte */
} HI_TS_DEMUXER_CONFIG_S;

typedef struct hiTS_CONFIG_S {
    HI_CHAR aszFileName[HI_TS_MAX_FILE_NAME]; /* file path and file name */
    HI_TS_CONFIG_TYPE_E enConfigType;
    union {
        HI_TS_MUXER_CONFIG_S stMuxerConfig;
        HI_TS_DEMUXER_CONFIG_S stDemuxerConfig;
    };
} HI_TS_CONFIG_S;

/**
 * @brief create TS.
 * @param[out] ppTs HI_MW_PTR* : return TS object
 * @param[in] pstTsCfg HI_TS_CONFIG_S* : ts configure
 * @return   0 success
 * @return  err number  failure
 */
HI_S32 HI_TS_Create(HI_TS_CONFIG_S *pstTsCfg, HI_MW_PTR *ppTsMuxer);

/**
 * @brief destroy TS.
 * @param[in] pTs HI_MW_PTR :  TS object
 * @return   0 success
 * @return  err number  failure
 */

HI_S32 HI_TS_Destroy(HI_MW_PTR pTsMuxer);

/**
 * @brief create TS stream.
 * @param[in] pTs HI_MW_PTR : TS object
 * @param[out] ppStream HI_MW_PTR* : return stream object
 * @param[in] pstStreamInfo HI_TS_STREAM_INFO_S* : track info
 * @return   0 success
 * @return  err number  failure
 */
HI_S32 HI_TS_CreateStream(HI_MW_PTR pTsMuxer, HI_MW_PTR *ppStream, HI_TS_STREAM_INFO_S *pstStreamInfo);

/**
 * @brief destroy all TS stream.
 * @param[in] pTs HI_MW_PTR :  TS object
 * @return   0 success
 * @return  err number  failure
 */
HI_S32 HI_TS_DestroyAllStreams(HI_MW_PTR pTsMuxer);

/**
 * @brief write TS frame data.
 * @param[in] pTs HI_MW_PTR : TS object
 * @param[in] pStream HI_MW_PTR : stream object
 * @param[in] pstFrameData HI_TS_FRAME_DATA_S* : frame date information
 * @return   0 success
 * @return  err number  failure
 */
HI_S32 HI_TS_WriteFrame(HI_MW_PTR pTsMuxer, HI_MW_PTR pStream, HI_TS_FRAME_DATA_S *pstFrameData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_TS_FORMAT_H__ */
