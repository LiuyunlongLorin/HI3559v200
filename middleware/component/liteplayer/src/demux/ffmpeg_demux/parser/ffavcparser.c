#include "securec.h"
#include "ffavcparser.h"
#include "ffbitsreader.h"


HI_BOOL FFAVCParser_parseSps(AVC_SPS_INFO* pstSpsInfo, HI_U8* pu8Sps, HI_U32 u32SpsSize)
{
    HI_S32 s32Ret = 0;
    if ((NULL == pstSpsInfo ) || (NULL == pu8Sps ) ||  (0 == u32SpsSize ))
    {
        return HI_FALSE;
    }
    AVC_SPS_INFO mSpsInfo = {0};

    HI_MW_PTR bitsReader =  0;
    s32Ret = FFBitsReader_create(&bitsReader, pu8Sps, u32SpsSize);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FALSE;
    }

    u_8(bitsReader); // 0x67 nal type
    HI_U8 u8ProfileIdc = u_8(bitsReader); // profile_idc
    u_8(bitsReader); // constraint_setx_flag
    u_8(bitsReader); // level_idc
    ue_v(bitsReader); // seq_parameter_set_id
    if (100 == u8ProfileIdc || 110 == u8ProfileIdc || 122 == u8ProfileIdc
        || 244 == u8ProfileIdc || 44 == u8ProfileIdc || 83 == u8ProfileIdc
        || 86 == u8ProfileIdc || 118 == u8ProfileIdc || 128 == u8ProfileIdc
        || 138 == u8ProfileIdc || 139 == u8ProfileIdc || 134 == u8ProfileIdc || 135 == u8ProfileIdc)
    {
        //HI_U32 u32ChromaFormatIdc = bitsReader.ue_v();
        mSpsInfo.u32ChromaFormatIdc = ue_v(bitsReader);
        if (3 == mSpsInfo.u32ChromaFormatIdc)
        {
            u_1(bitsReader); // separate_colour_plane_flag
        }
        ue_v(bitsReader); // bits_depth_luma_minus8
        ue_v(bitsReader); // bits_depth_chroma_minus8
        u_1(bitsReader); // qpprime_y_zero_transform_bypass_flag
        HI_U8 u8SeqScalingMatrixFlag = u_1(bitsReader); // seq_scaling_matrix_present_flag
        if (u8SeqScalingMatrixFlag)
        {
            for (HI_S32 i = 0; i < ( (mSpsInfo.u32ChromaFormatIdc != 3) ? 8 : 12); i++)
            {
                HI_U8 u8SeqScalingListFlag = u_1(bitsReader);
                if (u8SeqScalingListFlag)
                {
                    HI_U8 u8LastScale = 8;
                    HI_U8 u8NextScale = 8;
                    HI_U32 u32Scaletimes = (i < 6) ? 16 : 64;
                    for (HI_U32 j = 0; j < u32Scaletimes; j++)
                    {
                        if (u8NextScale != 0)
                        {
                            HI_U32 u32DeltaScale = se_v(bitsReader);
                            u8NextScale = (u8LastScale + u32DeltaScale + 256) % 256;
                        }
                        HI_U8 u8ScaleListJ = (0 == u8NextScale) ? u8LastScale : u8NextScale;
                        u8LastScale = u8ScaleListJ;
                    }
                }
            }
        }
    }
    ue_v(bitsReader); // log2_max_frame_num_minus4
    HI_U32 pic_order_cnt_type = ue_v(bitsReader);
    if (0 == pic_order_cnt_type)
    {
        ue_v(bitsReader); // log2_max_pic_order_cnt_lsb_minus4
    }
    else if (1 == pic_order_cnt_type)
    {
        u_1(bitsReader); // delta_pic_order_always_zero_flag
        se_v(bitsReader); // offset_for_non_ref_pic
        se_v(bitsReader); // offset_for_top_to_bottom_field
        HI_U32 num_ref_frames_in_pic_order_cnt_cycle = ue_v(bitsReader); // num_ref_frames_in_pic_order_cnt_cycle
        for (HI_U32 i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            se_v(bitsReader); // offset_for_ref_frame[i]
        }
    }
    ue_v(bitsReader); // max_num_ref_frames
    u_1(bitsReader); // gaps_in_frame_num_value_allowed_flag
    mSpsInfo.u32PicWidthInMbsMinus1 = ue_v(bitsReader); // pic_width_in_mbs_minus1
    mSpsInfo.u32PicHeightInMapUnitsMinus1 = ue_v(bitsReader); // pic_height_in_map_units_minus1
    mSpsInfo.bFrameMbsOnlyFlag = u_1(bitsReader); // frame_mbs_only_flag
    // no useful data behind
    if (!mSpsInfo.bFrameMbsOnlyFlag )
    {
        mSpsInfo.bAdaptFrameFlieFlag = u_1(bitsReader); // mb_adaptive_frame_field_flag
    }
    mSpsInfo.bDirectInferenceFlag = u_1(bitsReader);/*direct_8x8_inference_flag : 1*/
    mSpsInfo.bFameCroppingFlag = u_1(bitsReader); /*frame_cropping_flag : 1*/
    if (mSpsInfo.bFameCroppingFlag)
    {
        mSpsInfo.u32FrameCropLeftOffset = ue_v(bitsReader);/*frame_crop_left_offset : 0*/
        mSpsInfo.u32FrameCropRightOffset = ue_v(bitsReader);/*frame_crop_right_offset : 0*/
        mSpsInfo.u32FrameCropTopOffset = ue_v(bitsReader);/*frame_crop_top_offset : 0*/
        mSpsInfo.u32FrameCropBottomOffset = ue_v(bitsReader);/*frame_crop_bottom_offset : 4*/
    }

    if (HI_SUCCESS != memcpy_s(pstSpsInfo, sizeof(mSpsInfo), &mSpsInfo, sizeof(mSpsInfo)))
    {
        FFBitsReader_destroy(bitsReader);
        return HI_FALSE;
    }
    FFBitsReader_destroy(bitsReader);
    return HI_TRUE;
}
