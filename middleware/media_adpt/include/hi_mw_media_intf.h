#ifndef HI_MW_MEDIA_INTF_H
#define HI_MW_MEDIA_INTF_H

#include "hi_type.h"

typedef enum hiMW_H264E_NALU_TYPE_E
{
    HI_MW_H264E_NALU_BSLICE = 0,         /**<BSLICE types*/
    HI_MW_H264E_NALU_PSLICE = 1,         /**<PSLICE types*/
    HI_MW_H264E_NALU_ISLICE = 2,         /**<ISLICE types*/
    HI_MW_H264E_NALU_IDRSLICE = 5,       /**<IDRSLICE types*/
    HI_MW_H264E_NALU_SEI    = 6,         /**<SEI types*/
    HI_MW_H264E_NALU_SPS    = 7,         /**<SPS types*/
    HI_MW_H264E_NALU_PPS    = 8,         /**<PPS types*/
    HI_MW_H264E_NALU_BUTT
} HI_MW_VIDEO_H264E_NALU_TYPE_E;

typedef enum hiMW_H265E_NALU_TYPE_E
{
    HI_MW_H265E_NALU_BSLICE = 0,          /**<B SLICE types*/
    HI_MW_H265E_NALU_PSLICE = 1,          /**<P SLICE types*/
    HI_MW_H265E_NALU_ISLICE = 2,          /**<I SLICE types*/
    HI_MW_H265E_NALU_IDRSLICE = 19,       /**<IDR SLICE types*/
    HI_MW_H265E_NALU_VPS    = 32,         /**<VPS types*/
    HI_MW_H265E_NALU_SPS    = 33,         /**<SPS types*/
    HI_MW_H265E_NALU_PPS    = 34,         /**<PPS types*/
    HI_MW_H265E_NALU_SEI    = 39,         /**<SEI types*/
    HI_MW_H265E_NALU_BUTT
} HI_MW_VIDEO_H265E_NALU_TYPE_E;

typedef enum hiMW_DATA_JPEG_PACK_TYPE_E
{
    HI_MW_JPEGE_PACK_ECS = 5,
    HI_MW_JPEGE_PACK_APP = 6,
    HI_MW_JPEGE_PACK_VDO = 7,
    HI_MW_JPEGE_PACK_PIC = 8,
    HI_MW_JPEGE_PACK_BUTT
} HI_MW_VIDEO_JPEGE_PACK_TYPE_E;

typedef enum hiMW_VIDEO_MPEG4E_PACK_TYPE_E
{
    HI_MW_VIDEO_MPEG4E_PACK_VOP_P = 1,
    HI_MW_VIDEO_MPEG4E_PACK_VOP_I = 5,
    HI_MW_VIDEO_MPEG4E_PACK_VOS = 6,
    HI_MW_VIDEO_MPEG4E_PACK_VO = 7,
    HI_MW_VIDEO_MPEG4E_PACK_VOL = 8,
    HI_MW_VIDEO_MPEG4E_PACK_GVOP = 9,
    HI_MW_VIDEO_MPEG4E_PACK_BUTT
} HI_MW_VIDEO_MPEG4E_PACK_TYPE_E;

typedef enum hiMW_PAYLOAD_TYPE_E
{
    HI_MW_PAYLOAD_TYPE_H264,
    HI_MW_PAYLOAD_TYPE_H265,
    HI_MW_PAYLOAD_TYPE_MJPEG,
    HI_MW_PAYLOAD_TYPE_JPEG,
    HI_MW_PAYLOAD_TYPE_AAC,
    HI_MW_PAYLOAD_TYPE_BUTT
} HI_MW_PAYLOAD_TYPE_E;

typedef struct hiHI_MW_MEDIA_TYPE_S
{
    HI_MW_PAYLOAD_TYPE_E enPayloadType;                    /**< H.264/H.265/JPEG/MJPEG*/
    union
    {
        HI_MW_VIDEO_H264E_NALU_TYPE_E    enH264EType;      /**<H264E NALU types*/
        HI_MW_VIDEO_H265E_NALU_TYPE_E    enH265EType;      /**<H265E NALU types*/
        HI_MW_VIDEO_JPEGE_PACK_TYPE_E    enJPEGEType;      /**<JPEGE PACK types*/
        HI_MW_VIDEO_MPEG4E_PACK_TYPE_E   enMPEG4EType;     /**<MJPEGE PACK types*/
    };

} HI_MW_MEDIA_TYPE_S;

#define HI_MW_PACK_NUM     2

typedef struct hiMW_MEDIA_PACK_S
{
    HI_U64            u64PhyAddr[HI_MW_PACK_NUM];      /**< the physics address of stream*/
    HI_U8*             pu8Addr[HI_MW_PACK_NUM];       /**< the virtual address of stream*/
    HI_U32            au32Len[HI_MW_PACK_NUM];
    HI_U64            u64PTS;
    HI_MW_MEDIA_TYPE_S  stDataType;
    HI_U32   u32Offset;
    HI_U32   u32SegCnt;                              /**<segment cnt*/
} HI_MW_MEDIA_PACK_S;

#define HI_MW_MAX_FRAME_PACKCOUNT  12

typedef struct hiMW_MEDIA_VIDEO_S
{
    HI_MW_MEDIA_PACK_S astPack[HI_MW_MAX_FRAME_PACKCOUNT];     /**<stream pack attribute*/
    HI_U32              u32PackCount;                             /**<the pack number of one frame stream*/
    HI_U32              u32Seq;                                   /**<the list number of stream*/
    HI_BOOL             bEndOfStream;
} HI_MW_MEDIA_VIDEO_S;

typedef struct hiMW_MEDIA_AUDIO_S
{
    HI_U8* pu8Addr; /**< the virtual address of stream */
    HI_U64 u64PhyAddr; /**< the physics address of stream */
    HI_U32 u32Len; /**< stream length, by bytes */
    HI_U64 u64TimeStamp; /**< frame time stamp*/
    HI_U32 u32Seq; /**< frame sequels,if stream is not a valid frame,u32Seq is 0*/
} HI_MW_MEDIA_AUDIO_S;


typedef struct
{
    HI_HANDLE HANDLE;
    HI_U32 WIDTH;
    HI_U32 HEIGHT;
    HI_U32 FRAMERATE;
} HI_MW_SensorAttr_S;

typedef struct
{
    HI_HANDLE HANDLE;
    HI_U32 WIDTH;
    HI_U32 HEIGHT;
    HI_S32 FRAMERATE;
} HI_MW_VCapAttr_S;

typedef struct
{
    HI_HANDLE HANDLE;
    HI_U32 WIDTH;
    HI_U32 HEIGHT;
} HI_MW_VProcAttr_S;

typedef struct
{
    HI_HANDLE HANDLE;
    HI_U32 WIDTH;
    HI_U32 HEIGHT;
    HI_S32 FRAMERATE;
} HI_MW_VPortAttr;

typedef struct
{
    HI_HANDLE HANDLE;
    HI_MW_PAYLOAD_TYPE_E PAYLOAD_TYPE;
    HI_U32 WIDTH;
    HI_U32 HEIGHT;
    HI_U32 BUFSIZE;
    HI_U32 PROFILE;
    HI_U32 BASE;
    HI_U32 ENHANCE;
    HI_BOOL ENABLEPRED;
    HI_U32 BITRATE;
    HI_U32 GOP;
    HI_U32 FRAMERATE;
} HI_MW_VEncAttr;

typedef struct
{
    HI_MW_VProcAttr_S* pstVProcAttr;
    HI_MW_VPortAttr* pstVPortAttr;
    HI_MW_VEncAttr* pstVEncAttr;
} HI_MW_VideoBindMap_S;


typedef struct
{
    HI_HANDLE HANDLE;
} AEncAttr;

typedef struct
{
    HI_HANDLE HANDLE;
} ACapAttr;

typedef enum
{
    HI_MW_VENC_HEAD_TYPE_VPS,
    HI_MW_VENC_HEAD_TYPE_PPS,
    HI_MW_VENC_HEAD_TYPE_SPS,
    HI_MW_VENC_HEAD_TYPE_BUTT,
} HI_MW_VIDEO_HEAD_TYPE_E;

typedef struct hiMW_TOUCH_INPUTINFO_S
{
    HI_S32      id;/**<input id info, one finger or two fingers*/
    HI_S32		x;/**<x coordinate absolute*/
    HI_S32		y;/**<y coordinate absolute*/
    HI_U32	    pressure;/**<is press on screen: 0, 1*/
    HI_U32      timeStamp;/**<time stamp*/
} MW_TOUCH_INPUTINFO_S;


typedef HI_S32 (*HI_MW_OnVEncDataCB_FN)(HI_HANDLE hVencHdl, HI_MW_MEDIA_VIDEO_S* pstData, HI_VOID* pPrivData);
typedef HI_S32 (*HI_MW_OnAEncDataCB_FN)(HI_HANDLE hVencHdl, HI_MW_MEDIA_AUDIO_S* pstData, HI_VOID* pPrivData);

HI_S32 HI_MW_VEnc_Start(HI_HANDLE hVEncHdl, HI_S32 s32FrameCnt, HI_MW_OnVEncDataCB_FN pfnDataCB, HI_VOID* pPrivData);

HI_S32 HI_MW_VEnc_GetStreamHeadInfo(HI_HANDLE hVencHdl, HI_MW_VIDEO_HEAD_TYPE_E enType,
                                    HI_CHAR* pcHeadInfo, HI_U32* pu32HeadInfoLength);

HI_S32 HI_MW_VEnc_Stop(HI_HANDLE hVEncHdl, HI_MW_OnVEncDataCB_FN pfnDataCB);

HI_S32 HI_MW_AEnc_Start(HI_HANDLE hAEncHdl, HI_MW_OnAEncDataCB_FN pfnDataCB, HI_VOID* pPrivData);
HI_S32 HI_MW_AEnc_Stop(HI_HANDLE hVEncHdl, HI_MW_OnAEncDataCB_FN pfnDataCB);

HI_S32 HI_MW_Media_Init(HI_MW_SensorAttr_S* pstMWSensorAttr,
                        HI_MW_VCapAttr_S* pstMWVCapAttr,
                        HI_MW_VideoBindMap_S astVideoBindMap[], HI_U32 u32BindMapSize,
                        HI_HANDLE hACapHdl, HI_HANDLE hAEncHdl);

HI_S32 HI_MW_Media_DeInit(__attribute__((unused)) HI_MW_SensorAttr_S* pstMWSensorAttr,
                          HI_MW_VCapAttr_S* pstMWVCapAttr,
                          HI_MW_VideoBindMap_S astVideoBindMap[], HI_U32 u32BindMapSize,
                          HI_HANDLE hACapHdl, HI_HANDLE hAEncHdl);

HI_S32 HI_MW_DISP_Open(HI_VOID);

HI_S32 HI_MW_DISP_Close(HI_VOID);

HI_S32 HI_MW_VO_Open(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                     HI_HANDLE hDispHdl, HI_HANDLE hWndHdl, HI_U32 u32Width, HI_U32 u32Height);

HI_S32 HI_MW_VO_Close(HI_HANDLE hVpssGrpHdl, HI_HANDLE hVpssChnHdl,
                      HI_HANDLE hDispHdl, HI_HANDLE hWndHdl);

HI_S32 HI_MW_AO_Open(HI_HANDLE hAoHdl, HI_S32 s32SampleRate, HI_U32 u32ChnCnt);

HI_S32 HI_MW_AO_Close(HI_HANDLE hAoHdl);

HI_S32 HI_MW_VB_Alloc(HI_U32* pPoolId, HI_U32 u32FrameSize, HI_U32 u32FrameCnt);

HI_S32 HI_MW_VB_Free(HI_U32 poolid);

HI_S32 HI_MW_Sys_Init(HI_VOID);

HI_S32 HI_MW_Sys_Deinit(HI_VOID);

HI_S32 HI_MW_AO_GetDevHandle(HI_HANDLE* phAoHdl);

HI_S32 HI_MW_DISP_SetDisplayGraphicCSC(HI_VOID);

HI_S32 HI_MW_TP_Open(HI_S32* ps32Fd);

HI_S32 HI_MW_TP_Close(HI_VOID);

HI_S32 HI_MW_TP_ReadData(MW_TOUCH_INPUTINFO_S* pstInputData);

#endif
