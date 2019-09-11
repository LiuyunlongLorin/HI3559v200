#ifndef AVCPARSER_H
#define AVCPARSER_H
#include <inttypes.h>
#include "hi_mw_type.h"

typedef struct AVCSpsInfo
{
    HI_U32 u32ChromaFormatIdc;/*chroma_format_idc*/
    HI_U32 u32PicWidthInMbsMinus1;/*pic_width_in_mbs_minus1*/
    HI_U32 u32PicHeightInMapUnitsMinus1;/*pic_height_in_map_units_minus1*/
    HI_BOOL bFrameMbsOnlyFlag;/*frame_mbs_only_flag*/
    HI_BOOL bAdaptFrameFlieFlag;/*mb_adaptive_frame_field_flag : 0*/
    HI_BOOL bDirectInferenceFlag;/*direct_8x8_inference_flag : 1*/
    HI_BOOL bFameCroppingFlag;/*frame_cropping_flag : 1*/
    HI_U32 u32FrameCropLeftOffset;/*frame_crop_left_offset : 0*/
    HI_U32 u32FrameCropRightOffset;/*frame_crop_right_offset : 0*/
    HI_U32 u32FrameCropTopOffset;/*frame_crop_top_offset : 0*/
    HI_U32  u32FrameCropBottomOffset;/*frame_crop_bottom_offset : 4*/
}AVC_SPS_INFO;

HI_BOOL FFAVCParser_parseSps(AVC_SPS_INFO* pstSpsInfo, HI_U8* pu8Sps, HI_U32 u32SpsLen);
#endif
