/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * @file    hi_mapi_venc_define.h
 * @brief   venc module
 * @author  HiMobileCam NDK develop team
 * @date  2019-3-26
 */
#ifndef __HI_VENC_DEFINE_H__
#define __HI_VENC_DEFINE_H__

#include "hi_mapi_comm_define.h"
#include "hi_comm_venc.h"
#include "hi_comm_rc.h"
#include "hi_mapi_errno.h"
#include "mapi_venc_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* pack number of video encoder */
#define HI_MAPI_VENC_PACK_NUM (2)

/* max pack count of video encoder frame */
#define HI_MAPI_VENC_MAX_FRAME_PACKCOUNT (6)

/* limitless count of video encoder frame */
#define HI_MAPI_VENC_LIMITLESS_FRAME_COUNT (-1)

/* max count of video encoder frame */
#define HI_MAPI_VENC_MAX_FRAME_RATE (240)

/* error code for venc */
#define HI_MAPI_VENC_EHANDLE_ILLEGAL \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_INVALID_CHNID)
#define HI_MAPI_VENC_EILLEGAL_PARAM \
    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_ILLEGAL_PARAM)
#define HI_MAPI_VENC_EEXIST         HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_EXIST)
#define HI_MAPI_VENC_EUNEXIST       HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_UNEXIST)
#define HI_MAPI_VENC_ENULL_PTR      HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NULL_PTR)
#define HI_MAPI_VENC_ENOT_PERM      HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOT_PERM)
#define HI_MAPI_VENC_ENOMEM         HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_NOMEM)
#define HI_MAPI_VENC_ENOT_INITED    HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_SYS_NOTREADY)
#define HI_MAPI_VENC_EBUF_EMPTY     HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUF_EMPTY)
#define HI_MAPI_VENC_EBUSY          HI_MAPI_DEF_ERR(HI_MAPI_MOD_VENC, MAPI_EN_ERR_LEVEL_ERROR, MAPI_EN_ERR_BUSY)

/* the attribute of H264 video encoder */
typedef struct hiMAPI_VENC_ATTR_H264_S {
    // to be extend
} HI_MAPI_VENC_ATTR_H264_S;

/* the attribute of H265 video encoder */
typedef struct hiMAPI_VENC_ATTR_H265_S {
    // to be extend
} HI_MAPI_VENC_ATTR_H265_S;

/* the attribute of JPEG encoder */
typedef struct hiMAPI_VENC_ATTR_JPEG_S {
    HI_BOOL bEnableDCF;                       /**<support dcf */
    VENC_MPF_CFG_S stAttrMPF;                 /**<config of mpf */
    HI_U32 u32Qfactor;                        /**< [1, 99] */
    VENC_JPEG_ENCODE_MODE_E enJpegEncodeMode; /**<Jpeg snap mode */
} HI_MAPI_VENC_ATTR_JPEG_S;

/* the attribute of JPEG encoder */
typedef struct hiMAPI_VENC_ATTR_MJPEG_S {
} HI_MAPI_VENC_ATTR_MJPEG_S;

/* the scene mode */
typedef enum hiMAPI_VENC_SCENE_MODE_E {
    HI_MAPI_VENC_SCENE_MODE_NORMAL, /**<normal scene mode */
    HI_MAPI_VENC_SCENE_MODE_DV,     /**<dv scene mode */
    HI_MAPI_VENC_SCENE_MODE_CAR,    /**<car scene mode */
    HI_MAPI_VENC_SCENE_MODE_BUTT,
} HI_MAPI_VENC_SCENE_MODE_E;

/* the attribute of encoder type */
typedef struct hiMAPI_VENC_TYPE_ATTR_S {
    HI_MAPI_PAYLOAD_TYPE_E enType;         /**<the type of payload */
    HI_U32 u32Width;                       /**<width of a picture to be encoded, in pixel */
    HI_U32 u32Height;                      /**<height of a picture to be encoded, in pixel */
    HI_U32 u32BufSize;                     /**<stream buffer size */
    HI_U32 u32Profile;                     /**<H.264:   0: baseline; 1:MP; 2:HP;
                                                                           H.265:   0:MP;
                                                                           Jpege/MJpege:   0:Baseline */
    HI_MAPI_VENC_SCENE_MODE_E enSceneMode; /**<scene mode */
    union {
        HI_MAPI_VENC_ATTR_H264_S stAttrH264e;  /**<attributes of H264e */
        HI_MAPI_VENC_ATTR_H265_S stAttrH265e;  /**<attributes of H265e */
        HI_MAPI_VENC_ATTR_JPEG_S stAttrJpege;  /**<attributes of jpeg  */
        HI_MAPI_VENC_ATTR_MJPEG_S stAttrMjpeg; /**<attributes of Mjpeg */
    };

} HI_MAPI_VENC_TYPE_ATTR_S;

/* the rate control mode */
typedef enum hiMAPI_VENC_RC_MODE_E {
    HI_MAPI_VENC_RC_MODE_CBR,  /**<constant bit rate */
    HI_MAPI_VENC_RC_MODE_VBR,  /**<variable bit rate */
    HI_MAPI_VENC_RC_MODE_QVBR, /**<quality variable bit rate */
    HI_MAPI_VENC_RC_MODE_BUTT,
} HI_MAPI_VENC_RC_MODE_E;

/* the attribute of constant bit rate */
typedef struct hiVENC_H264_CBR_S
    HI_MAPI_VENC_ATTR_CBR_S; /**<H264,H265 and MJPEG attr are the same, Data structure reference MPI */

/* the attribute of variable bit rate */
typedef struct hiVENC_H264_VBR_S
    HI_MAPI_VENC_ATTR_VBR_S; /**<H264,H265 and MJPEG attr are the same, Data structure reference MPI */

/* the attribute of rate control */
typedef struct hiMAPI_VENC_RC_ATTR_S {
    HI_MAPI_VENC_RC_MODE_E enRcMode; /**<the type of rc */
    union {
        HI_MAPI_VENC_ATTR_CBR_S stAttrCbr;   /**<attributes of cbr */
        HI_MAPI_VENC_ATTR_VBR_S stAttrVbr;   /**<attributes of vbr */
        HI_MAPI_VENC_ATTR_QVBR_S stAttrQVbr; /**<attributes of qvbr */
    };
} HI_MAPI_VENC_RC_ATTR_S;

/* the attribute of video encode */
typedef struct hiMAPI_VENC_ATTR_S {
    HI_MAPI_VENC_TYPE_ATTR_S stVencPloadTypeAttr; /**<the attribute of video encoder type */
    HI_MAPI_VENC_RC_ATTR_S stRcAttr;              /**<the attribute of rate  ctrl */
} HI_MAPI_VENC_ATTR_S;

/* the data type of video encode */
typedef struct hiMAPI_VENC_DATA_TYPE_S {
    HI_MAPI_PAYLOAD_TYPE_E enPayloadType; /**<H.264/H.265/JPEG/MJPEG */
    union {
        H264E_NALU_TYPE_E enH264EType; /**<H264E NALU types */
        H265E_NALU_TYPE_E enH265EType; /**<H265E NALU types */
        JPEGE_PACK_TYPE_E enJPEGEType; /**<JPEGE PACK types */
    };

} HI_MAPI_VENC_DATA_TYPE_S;

/* the pack info of VENC */
typedef struct hiMAPI_VENC_FRAME_HEAD_INFO_S {
    HI_MAPI_VENC_DATA_TYPE_S u32PackType; /**<the pack type */
    HI_U32 u32PackOffset; /**<the current stream package data contains the offset of other stream package data */
    HI_U32 u32PackLength; /**<The current stream package data includes the size of other stream package data */
} HI_MAPI_VENC_PACK_INFO_S;

/* the data pack of video encode */
typedef struct hiMAPI_VENC_DATA_PACK_S {
    HI_U64 au64PhyAddr[HI_MAPI_VENC_PACK_NUM]; /**<the physics address of stream */
    HI_U8 *apu8Addr[HI_MAPI_VENC_PACK_NUM];    /**<the virtual address of stream */
    HI_U32 au32Len[HI_MAPI_VENC_PACK_NUM];     /**<the length of stream */
    HI_U64 u64PTS;                             /**<time stamp */
    HI_MAPI_VENC_DATA_TYPE_S stDataType;       /**<the type of stream */
    HI_U32 u32Offset;                          /**<the offset between the Valid data and the start address */
} HI_MAPI_VENC_DATA_PACK_S;

/* Defines the frame type and reference attributes of the H.264/H.265 frame skipping reference streams */
typedef enum hiH264E_REF_TYPE_E
HI_MAPI_REF_TYPE_E; /**<H264 and H265 attr are the same, Data structure reference MPI */

/* the stream information of video encode */
typedef struct hiMAPI_VENC_STREAM_INFO_S {
    HI_MAPI_REF_TYPE_E enRefType; /**<Type of encoded frames in advanced frame skipping reference mode */
    HI_U32 u32StartQp;            /**<the start Qp of encoded frames */
} HI_MAPI_VENC_STREAM_INFO_S;

/* the data of video encode */
typedef struct hiMAPI_VENC_DATA_S {
    HI_MAPI_VENC_DATA_PACK_S astPack[HI_MAPI_VENC_MAX_FRAME_PACKCOUNT]; /**<stream pack attribute */
    HI_U32 u32PackCount;                                                /**<the pack number of one frame stream */
    HI_U32 u32Seq;                                                      /**<the list number of stream */
    HI_BOOL bEndOfStream;                                               /**<frame end flag */
    HI_MAPI_VENC_STREAM_INFO_S stStreamInfo;                            /**<stream info attribute */
} HI_MAPI_VENC_DATA_S;

typedef HI_S32 (*PFN_VENC_DataProc)(HI_HANDLE VencHdl, HI_MAPI_VENC_DATA_S *pVStreamData,
    HI_VOID *pPrivateData);

typedef struct hiMAPI_VENC_CALLBACK_S {
    PFN_VENC_DataProc pfnDataCB;
    HI_VOID *pPrivateData;
} HI_MAPI_VENC_CALLBACK_S;

/* video encoder extend attribute command */
typedef enum hiMAPI_VENC_CMD_E {
    HI_MAPI_VENC_CMD_H264_RC_ATTR_EX,    /**<VENC_RC_PARAM_S */
    HI_MAPI_VENC_CMD_H265_RC_ATTR_EX,    /**<VENC_RC_PARAM_S */
    HI_MAPI_VENC_CMD_MJPEG_RC_ATTR_EX,   /**<VENC_RC_PARAM_S */
    HI_MAPI_VENC_CMD_GOP_MODE,           /**<HI_MAPI_VENC_GOP_ATTR_S */
    HI_MAPI_VENC_CMD_INTRA_REFRESH,      /**<VENC_INTRA_REFRESH_S */
    HI_MAPI_VENC_CMD_FRAME_RATE,         /**<VENC_FRAME_RATE_S */
    HI_MAPI_VENC_CMD_HIERARCHICAL_QP,    /**<VENC_HIERARCHICAL_QP_S */
    HI_MAPI_VENC_CMD_FRAMELOST_STRATEGY, /**<VENC_FRAMELOST_STRATEGY */
    HI_MAPI_VENC_CMD_BUTT
} HI_MAPI_VENC_CMD_E;

/* the gop mode */
typedef enum hiMAPI_VENC_GOP_MODE_E {
    HI_MAPI_VENC_GOPMODE_NORMALP = 0, /**<NORMALP */
    HI_MAPI_VENC_GOPMODE_DUALP = 1,   /**<DUALP */
    HI_MAPI_VENC_GOPMODE_SMARTP = 2,  /**<SMARTP */
    HI_MAPI_VENC_GOPMODE_BIPREDB = 3, /**<BIPREDB */
    HI_MAPI_VENC_GOPMODE_BUTT,
} HI_MAPI_VENC_GOP_MODE_E;

/* the attribute of the gop */
typedef struct hiMAPI_VENC_GOP_ATTR_S {
    HI_MAPI_VENC_GOP_MODE_E enGopMode; /**<Encoding GOP type */
    union {
        VENC_GOP_NORMALP_S stNormalP; /**<the attribute of the normalp */
        VENC_GOP_DUALP_S stDualP;     /**<the attribute of the dualp */
        VENC_GOP_SMARTP_S stSmartP;   /**<the attribute of the smartp */
        VENC_GOP_BIPREDB_S stBipredB; /**<the attribute of the bipredb */
    };
} HI_MAPI_VENC_GOP_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_VENC_DEFINE_H__ */
