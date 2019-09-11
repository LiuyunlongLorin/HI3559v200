/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_mp4_format.h
* @brief     mp4muxer c style interface
* @author    HiMobileCam middleware develop team
* @date      2016.12.20
*/
#ifndef __HI_MP4_FORMAT_H__
#define __HI_MP4_FORMAT_H__

#include "hi_mw_type.h"
#include "hi_error_def.h"
#include "hi_defs.h"
#include "hi_mp4_format.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
typedef enum hiAPP_MP4_ERR_CODE_E {
    /* general error code */
    APP_MP4_ERR_HANDLE_INVALID = 0x40, /**<mp4 handle invalid */
    APP_MP4_ERR_INVALID_ARG = 0x41,    /**<param is null or invalid */
    APP_MP4_ERR_MALLOC_FAIL = 0x42,    /**<malloc memory fail */
    APP_MP4_ERR_CREATE_MUXER = 0x43,   /**<create mp4 fail */
    APP_MP4_ERR_DESTROY_MUXER = 0x44,  /**<destory mp4  fail */
    APP_MP4_ERR_WRITE_HEAD = 0x45,     /**<start mp4 fail */
    APP_MP4_ERR_WRITE_TAIL = 0x46,     /**<stop mp4 fail */
    APP_MP4_ERR_CREATE_AGAIN = 0x47,   /**<mp4 re created */
    APP_MP4_ERR_NOT_CREATE = 0x48,     /**<mp4 not created */
    APP_MP4_ERR_READ_FRAME = 0x49,     /**<read frame fail */
    APP_MP4_ERR_WRITE_FRAME = 0x50,    /**<write frame fail */
    APP_MP4_ERR_REPAIR = 0x51,         /**<write frame fail */
    APP_MP4_ERR_CREATE_STREAM = 0x52,  /**<create stream fail */
    APP_MP4_ERR_DESTROY_STREAM = 0x53, /**<destory stream  fail */
    APP_MP4_ERR_GET_ATOM = 0x54,       /**<get atom fail */
    APP_MP4_ERR_ADD_ATOM = 0x55,       /**<add atom  fail */
    APP_MP4_ERR_ATOM_LEN = 0x56,       /**<add atom  fail */
    APP_MP4_ERR_DESTROY_REPAIR = 0x59, /**<destory back  fail */
    APP_MP4_ERR_CREATE_REPAIR = 0x60,  /**<create back again */

    /* file related error code */
    APP_MP4_ERR_OPETATION_FAIL = 0x61,     /**<mp4 repair fail */
    APP_MP4_ERR_UNSUPPORT_CODEC = 0x62,    /**<not support codec */
    APP_MP4_ERR_OPEN_FILE = 0x63,          /**<open file error */
    APP_MP4_ERR_CLOSE_FILE = 0x64,         /**<close file error */
    APP_MP4_ERR_READ_FILE = 0x65,          /**<read file error */
    APP_MP4_ERR_SEEK_FILE = 0x66,          /**<seek file error */
    APP_MP4_ERR_WRITE_FILE = 0x67,         /**<write file error */
    APP_MP4_ERR_REMOVE_FILE = 0x68,        /**<remove file error */
    APP_MP4_ERR_FILE_SIZE_OVERFLOW = 0x69, /**<file size overflow error */

    APP_MP4_ERR_BUTT = 0xFF
} HI_APP_MP4_ERR_CODE_E;

/* general error code */
#define HI_ERR_MP4_NULL_PTR       HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_ERR_NULL_PTR)
#define HI_ERR_MP4_HANDLE_INVALID HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_HANDLE_INVALID)
#define HI_ERR_MP4_INVALIDARG     HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_INVALID_ARG)
#define HI_ERR_MP4_MALLOC         HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_MALLOC_FAIL)
#define HI_ERR_MP4_CREATE         HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_CREATE_MUXER)
#define HI_ERR_MP4_DESTROY        HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_DESTROY_MUXER)
#define HI_ERR_MP4_WRITE_HEAD     HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_WRITE_HEAD)
#define HI_ERR_MP4_WRITE_TAIL     HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_WRITE_TAIL)
#define HI_ERR_MP4_CREATE_AGAIN   HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_CREATE_AGAIN)
#define HI_ERR_MP4_NOT_CREATE     HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_NOT_CREATE)
#define HI_ERR_MP4_READ_FRAME     HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_READ_FRAME)
#define HI_ERR_MP4_WRITE_FRAME    HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_WRITE_FRAME)
#define HI_ERR_MP4_REPAIR_UPDATE  HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_REPAIR)
#define HI_ERR_MP4_CREATE_STREAM  HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_CREATE_STREAM)
#define HI_ERR_MP4_DESTROY_STREAM HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_DESTROY_STREAM)
#define HI_ERR_MP4_GET_ATOM       HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_GET_ATOM)
#define HI_ERR_MP4_ADD_ATOM       HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_ADD_ATOM)
#define HI_ERR_MP4_ATOM_LEN       HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_ATOM_LEN)

/* FILE related error code */
#define HI_ERR_MP4_OPERATION_FAILED   HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_OPETATION_FAIL)
#define HI_ERR_MP4_UNSUPPORT_CODEC    HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_UNSUPPORT_CODEC)
#define HI_ERR_MP4_OPEN_FILE          HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_OPEN_FILE)
#define HI_ERR_MP4_END_FILE           HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_CLOSE_FILE)
#define HI_ERR_MP4_READ_FILE          HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_READ_FILE)
#define HI_ERR_MP4_SEEK_FILE          HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_SEEK_FILE)
#define HI_ERR_MP4_WRITE_FILE         HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_WRITE_FILE)
#define HI_ERR_MP4_REMOVE_FILE        HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_REMOVE_FILE)
#define HI_ERR_MP4_FILE_SIZE_OVERFLOW HI_APP_DEF_ERR(HI_APPID_MP4, APP_ERR_LEVEL_ERROR, APP_MP4_ERR_FILE_SIZE_OVERFLOW)

#define HI_MP4_MAX_HDLR_NAME (256) /* handlr name max length 255,one for '/0' */
#define HI_MP4_TRACK_NUM_MAX (6)
#define HI_MP4_MAX_BOX_PATH  (64)  /* atom path  max length */
#define HI_MP4_MAX_FILE_NAME (256) /* file name path max */
#define HI_MP4_TYPE_SIZE     (4)   /* box type length */

typedef enum hiMP4_CONFIG_TYPE_E {
    HI_MP4_CONFIG_MUXER = 1,
    HI_MP4_CONFIG_DEMUXER,
    HI_MP4_CONFIG_BUTT
} HI_MP4_CONFIG_TYPE_E;

typedef enum hiMP4_FORMAT_PROFILE_E {
    HI_MP4_FORMAT_MP42 = 0,  // base media/version 2
    HI_MP4_FORMAT_QT,        // quicktime movie,
    HI_MP4_FORMAT_BUTT
} HI_MP4_FORMAT_PROFILE_E;

typedef enum hiMP4_TRACK_TYPE_E {
    HI_MP4_STREAM_VIDEO = 1,
    HI_MP4_STREAM_AUDIO,
    HI_MP4_STREAM_DATA,
    HI_MP4_STREAM_BUTT
} HI_MP4_TRACK_TYPE_E;

typedef enum hiMP4_CODECID_E {
    HI_MP4_CODEC_ID_H264 = 0,
    HI_MP4_CODEC_ID_H265,
    HI_MP4_CODEC_ID_MJPEG,
    HI_MP4_CODEC_ID_PRORES_422_PROXY,  // apco
    HI_MP4_CODEC_ID_PRORES_422_LT,     // apcs
    HI_MP4_CODEC_ID_PRORES_422,        // apcn
    HI_MP4_CODEC_ID_PRORES_422_HQ,     // apch
    HI_MP4_CODEC_ID_AACLC,
    HI_MP4_CODEC_ID_MP3,
    HI_MP4_CODEC_ID_G726,
    HI_MP4_CODEC_ID_G711_A,
    HI_MP4_CODEC_ID_G711_M,
    HI_MP4_CODEC_ID_PCM,
    HI_MP4_CODEC_ID_BUTT
} HI_MP4_CODECID_E;

typedef struct hiMP4_VIDEOINFO_S {
    HI_U32 u32Width;            /* video width */
    HI_U32 u32Height;           /* video height */
    HI_U32 u32BitRate;          /* bitrate bps */
    HI_U32 u32FrameRate;        /* frame rate fps */
    HI_MP4_CODECID_E enCodecID; /* codec type */
} HI_MP4_VIDEOINFO_S;

typedef struct hiMP4_AUDIOINFO_S {
    HI_U32 u32Channels;         /* audio channel num 2 */
    HI_U32 u32SampleRate;       /* audio sample rate 48k */
    HI_U32 u32SamplePerFrame;   /* audio sample per frame */
    HI_U16 u16SampleSize;       /* bit per sample , 16 */
    HI_MP4_CODECID_E enCodecID; /* codec type */
} HI_MP4_AUDIOINFO_S;

typedef struct hiMP4_DATAINFO_S {
    HI_U32 u32Width;            /* meta data width */
    HI_U32 u32Height;           /* meta data height */
    HI_U32 u32FrameRate;        /* frame rate fps */
    HI_MP4_CODECID_E enCodecID; /* codec type */
} HI_MP4_DATAINFO_S;

typedef struct hiMP4_TRACK_INFO_S {
    HI_MP4_TRACK_TYPE_E enTrackType;           /* stream type */
    HI_U32 u32TimeScale;                       /* time scale for each trak */
    HI_FLOAT fSpeed;                           /* play speed, (0,1]for slow,(1,~) for fast */
    HI_CHAR aszHdlrName[HI_MP4_MAX_HDLR_NAME]; /* manufacturer  name */
    union {
        HI_MP4_VIDEOINFO_S stVideoInfo; /* video info */
        HI_MP4_AUDIOINFO_S stAudioInfo; /* audio info */
        HI_MP4_DATAINFO_S stDataInfo;   /* metadata info */
    };
} HI_MP4_TRACK_INFO_S;

typedef struct hiMP4_FRAME_DATA_S {
    HI_U64 u64TimeStamp;   /* frame timestamp */
    HI_U8 *pu8DataBuffer;  /* frame data buffer */
    HI_U32 u32DataLength;  /* frame date len */
    HI_BOOL bKeyFrameFlag; /* key frame flag */
} HI_MP4_FRAME_DATA_S;

typedef struct hiMP4_ATOM_INFO_S {
    HI_U32 u32DataLen;                     /* user define atom data len */
    HI_CHAR aszType[HI_MP4_TYPE_SIZE + 1]; /* user define atom type */
    HI_U8 *pu8DataBuf;                     /* user define atom data buffer */
} HI_MP4_ATOM_INFO_S;

/**
 * @brief set duration callback.
 * @param[in] hHANDLE HI_MW_PTR :  muxer handle
 * @param[in] u32Timescale HI_U32 : timescale for muxer
 * @param[in,out] pu64Duration HI_U64* : duration for muxer
 * @return   0 success
 * @return  err num  failure
 */
typedef HI_S32 (*HI_MP4_SETDUTAION_FN)(HI_MW_PTR pMp4, HI_U32 u32Timescale,
               HI_U64 *pu64Duration); /* get and set duration callback */

typedef struct hiMP4_FILE_INFO_S {
    HI_U32 u32Duration;  // ms
    HI_CHAR aszHandlrName[HI_MP4_MAX_HDLR_NAME];
    HI_BOOL bCo64Flag;
    HI_U32 u32TrackNum;
    HI_U32 u32BackupUnit;
} HI_MP4_FILE_INFO_S;

typedef struct hiMP4_CONFIG_MUXER_S {
    HI_U32 u32PreAllocUnit;                   /* pre allocate size in bytes, [0,100M],0 for not use pre allocate function,suggest 20M, unit :byte */
    HI_U32 u32VBufSize;                       /* set the vbuf size for fwrite (0,5M] unit :byte */
    HI_BOOL bCo64Flag;                        /* if true;use co64,or use stco,used for muxer */
    HI_BOOL bConstantFps;                     /* if true, use constant framerate to calculate time,or use pst delta */
    HI_U32 u32BackupUnit;                     /* stbl group unit [500k,500M],if 0 not use backup data and repair */
    HI_MP4_FORMAT_PROFILE_E enFormatProfile;  // mpeg-4 file profile,base medie or quicktime
} HI_MP4_CONFIG_MUXER_S;

typedef struct hiMP4_CONFIG_DEMUXER_S {
    HI_U32 u32VBufSize; /* set the vbuf size for fwrite (0,5M] unit :byte */
} HI_MP4_CONFIG_DEMUXER_S;

typedef struct hiMP4_CONFIG_APPENDS_S {
    HI_U32 u32VBufSize; /* set the vbuf size for fwrite (0,5M] unit :byte */
} HI_MP4_CONFIG_APPEND_S;

typedef struct hiMP4_CONFIG_S {
    HI_CHAR aszFileName[HI_MP4_MAX_FILE_NAME]; /* file path and file name */
    HI_MP4_CONFIG_TYPE_E enConfigType;
    union {
        HI_MP4_CONFIG_MUXER_S stMuxerConfig;
        HI_MP4_CONFIG_DEMUXER_S stDemuxerConfig;
        HI_MP4_CONFIG_APPEND_S stAppendConfig;
    };
} HI_MP4_CONFIG_S;

/**
 * @brief create mp4 instance.
 * @param[out] ppMp4 HI_MW_PTR* : return MP4 handle
 * @param[in] pstMp4Cfg HI_MP4_CONFIG_S* : mp4 cfg
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_Create(HI_MW_PTR *ppMp4, HI_MP4_CONFIG_S *pstMp4Cfg);

/**
 * @brief destroy mp4 instance.
 * @param[in] pMp4 HI_MW_PTR :  MP4 handle
 * @param[out] pu64Duration HI_U64* :  file duration
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_MP4_Destroy(HI_MW_PTR pMp4, HI_U64 *pu64Duration);

/**
 * @brief create mp4 track.
 * @param[in] pMp4 HI_MW_PTR : MP4 handle
 * @param[out] ppTrack HI_MW_PTR * : return track handle
 * @param[in] pstTrackInfo HI_MP4_TRACK_INFO_S* :stream info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_CreateTrack(HI_MW_PTR pMp4, HI_MW_PTR *ppTrack, HI_MP4_TRACK_INFO_S *pstTrackInfo);

/**
 * @brief destroy all mp4 track.
 * @param[in] pMp4 HI_MW_PTR :  MP4 handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_DestroyAllTracks(HI_MW_PTR pMp4, HI_MP4_SETDUTAION_FN pfnSetDuration);

/**
 * @brief write mp4 frame data.
 * @param[in] pMp4 HI_MW_PTR : MP4 handle
 * @param[in] pTrack HI_MW_PTR : track handle
 * @param[in] pstFrameData HI_MP4_FRAME_DATA_S* : frame data
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_WriteFrame(HI_MW_PTR pMp4, HI_MW_PTR pTrack, HI_MP4_FRAME_DATA_S *pstFrameData);

/**
 * @brief add mp4 atom.
 * /   top
 * /moov
 * /moov/trak[1]/mdia/tkhd
 * @param[in] pMp4 HI_MW_PTR : MP4 handle
 * @param[in] pszPath HI_CHAR* : box path [1,64]
 * @param[in] pstAtomInfo HI_MP4_ATOM_INFO_S* : data info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_AddAtom(HI_MW_PTR pMp4, HI_CHAR *pszAtomPath, HI_MP4_ATOM_INFO_S *pstAtomInfo);

/**
 * @brief read mp4 box content.
 * @param[in] pMp4 HI_MW_PTR : MP4 handle
 * @param[out] pszPath HI_CHAR* : box path [1,64]
 * @param[in/out] pstAtomInfo HI_MP4_ATOM_INFO_S* : data info
 * @param[in/out] pBufLen HI_U32* : mp4 box buffer len as readProperty ,output
 * @                     actual box len
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_GetAtom(HI_MW_PTR pMp4, HI_CHAR *pszAtomPath, HI_MP4_ATOM_INFO_S *pstAtomInfo);

/**
 * @brief read mp4 info.
 * @param[in] pMp4 HI_MW_PTR : MP4 handle
 * @param[in,out] pstFileInfo HI_MP4_FILE_INFO_S* : mp4 info
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_GetFileInfo(HI_MW_PTR pMp4, HI_MP4_FILE_INFO_S *pstFileInfo);

/**
 * @brief read mp4 Track by Index.
 * @param[in] pMp4 HI_MW_PTR : MP4 Demuxer handle
 * @param[in] u32Index HI_U32 : MP4 track index
 * @param[out] pTrackInfo HI_MP4_TRACK_INFO_S* : mp4 box track Info
 * @param[out] ppTrack HI_MW_PTR* : track handle
 * @return   0 success
 * @return  err num  failure
 */
HI_S32 HI_MP4_GetTrackByIndex(HI_MW_PTR pMp4, HI_U32 u32Index, HI_MP4_TRACK_INFO_S *pTrackInfo,
                              HI_MW_PTR *ppTrack);

/**
 * @brief repair mp4 data.
 * @param[in] filename HI_CHAR* : file name
 * @param[in] bParseExtraMdat HI_BOOL : parse extra mdat data after last backup unit flag
 * @return   0 success
 * @return  err num  failure
 */

HI_S32 HI_MP4_RepairFile(HI_CHAR *aszFilename, HI_BOOL bParseExtraMdat);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
