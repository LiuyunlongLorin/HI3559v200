#include "securec.h"
#include <stdio.h>
#include "ffhevcparser.h"


HI_BOOL FFHEVCParser_parseSps(HEVC_SPS_INFO* pstSpsInfo , HI_U8* pu8Sps, HI_U32 u32SpsLen)
{
    HI_S32 s32Ret = 0;
    if ((NULL == pstSpsInfo ) || (NULL == pu8Sps ) ||  (0 == u32SpsLen ))
    {
        return HI_FALSE;
    }

    HI_MW_PTR bitReader =  0;
    s32Ret = FFBitsReader_create(&bitReader, pu8Sps, u32SpsLen);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FALSE;
    }
    HEVC_SPS_INFO mSpsInfo = {0};

    FFHEVCParser_parseNalUnitHeader(bitReader);
    u_4(bitReader); // video_parameter_set_id
    HI_U8 u8MaxSubLayersMinus1 = u_3(bitReader); // max_sub_layers_minus1
    mSpsInfo.u8MaxSubLayersMinus1 = u8MaxSubLayersMinus1;
    mSpsInfo.bTemporalIdNestingFlag = u_1(bitReader); // temporal_id_nesting_flag
    FFHEVCParser_parseProfileTierLevel(&mSpsInfo, bitReader, 1, u8MaxSubLayersMinus1);
    ue_v(bitReader); // seq_parameter_set_id
    HI_U8 u8ChromaFormatIdc = ue_v(bitReader); // chroma_format_idc
    if (3 == u8ChromaFormatIdc)
    {
        u_1(bitReader); // separate_colour_plane_flag
    }
    mSpsInfo.u32PicWidthInLumaSamples = ue_v(bitReader); // pic_width_in_luma_samples
    mSpsInfo.u32PicHeightInLumaSamples = ue_v(bitReader); // pic_height_in_luma_samples
    HI_U8 u8ConformanceWindowFlag = u_1(bitReader); // conformance_window_flag
    if (u8ConformanceWindowFlag)
    {
        ue_v(bitReader); // conf_win_left_offset
        ue_v(bitReader); // conf_win_right_offset
        ue_v(bitReader); // conf_win_top_offset
        ue_v(bitReader); // conf_win_bottom_offset
    }
    ue_v(bitReader); // bit_depth_luma_minus8
    ue_v(bitReader); // bit_depth_chroma_minus8
    ue_v(bitReader); // log2_max_pic_order_cnt_lsb_minus4
    HI_BOOL bSubLayerOrderingInfoPresentFlag = u_1(bitReader); // sub_layer_ordering_info_present_flag
    for (HI_S32 i = (bSubLayerOrderingInfoPresentFlag ? 0 : u8MaxSubLayersMinus1); i <= u8MaxSubLayersMinus1; i++)
    {
        ue_v(bitReader); // max_dec_pic_buffering_minus1[i]
        ue_v(bitReader); // max_num_reorder_pics[i]
        ue_v(bitReader); // max_latency_increase_plus1[i]
    }
    ue_v(bitReader); // log2_min_luma_coding_block_size_minus3
    ue_v(bitReader); // log2_diff_max_min_luma_coding_block_size
    ue_v(bitReader); // log2_min_luma_transform_block_size_minus2
    ue_v(bitReader); // log2_diff_max_min_luma_transform_block_size
    ue_v(bitReader); // max_transform_hierarchy_depth_inter
    ue_v(bitReader); // max_transform_hierarchy_depth_intra
    HI_BOOL bScalingListEnableFlag = u_1(bitReader); // scaling_list_enabled_flag
    if (bScalingListEnableFlag)
    {
        HI_BOOL bScalingListDataPresentFlag = u_1(bitReader); // scaling_list_data_present_flag
        if (bScalingListDataPresentFlag)
        {
            FFHEVCParser_parseScalingListData(bitReader);
        }
    }
    u_1(bitReader); // amp_enabled_flag
    u_1(bitReader); // sample_adaptive_offset_enabled_flag
    HI_BOOL bPCMEnabledFlag = u_1(bitReader); // pcm_enabled_flag
    if (bPCMEnabledFlag)
    {
        u_4(bitReader); // pcm_sample_bit_depth_luma_minus1
        u_4(bitReader); // pcm_sample_bit_depth_chroma_minus1
        ue_v(bitReader); // log2_min_pcm_luma_coding_block_size_minus3
        ue_v(bitReader); // log2_diff_max_min_pcm_luma_coding_block_size
        u_1(bitReader); // pcm_loop_filter_disabled_flag
    }
    HI_U32 u32NumShortTermRefPicSets = ue_v(bitReader); // num_short_term_ref_pic_sets
    HI_U32 au32NumDeltaPocs[64]; // max short term rps count 64
    for (HI_U32 i = 0; i < u32NumShortTermRefPicSets; i++)
    {
        FFHEVCParser_parseStRefPicSet(bitReader, i, u32NumShortTermRefPicSets, au32NumDeltaPocs);
    }
    HI_BOOL bLongTermRefPicsPresentFlag = u_1(bitReader); // long_term_ref_pics_present_flag
    if (bLongTermRefPicsPresentFlag)
    {
        HI_U32 u32NumLongTermRefPics = ue_v(bitReader); // num_long_term_ref_pics_sps
        for (HI_U32 i = 0; i < u32NumLongTermRefPics; i++)
        {
            // lt_ref_pic_poc_lsb_sps[i]
            u_1(bitReader); // used_by_curr_pic_lt_sps_flag[i]
        }
    }
    u_1(bitReader); // temporal_mvp_enabled_flag
    u_1(bitReader); // strong_intra_smoothing_enabled_flag
    HI_BOOL bVUIParametersPresentFlag = u_1(bitReader); // vui_parameters_present_flag
    if (bVUIParametersPresentFlag)
    {
        FFHEVCParser_parseVUIParameters(bitReader);
    }
    HI_BOOL bExtensionPresentFlag = u_1(bitReader); // sps_extension_present_flag
    HI_BOOL bRangeExtension = HI_FALSE;
    HI_BOOL bMultilayerExtensionFlag = HI_FALSE;
    HI_BOOL b3DExtensionFlag = HI_FALSE;
    if (bExtensionPresentFlag)
    {
        bRangeExtension = u_1(bitReader); // sps_range_extension_flag
        bMultilayerExtensionFlag = u_1(bitReader); // sps_multilayer_extension_flag
        b3DExtensionFlag = u_1(bitReader); // sps_3d_extension_flag
        u_5(bitReader); // sps_extension_5bits
    }

    if (bRangeExtension)
    {
        u_1(bitReader); // transform_skip_rotation_enabled_flag
        u_1(bitReader); // transform_skip_context_enabled_flag
        u_1(bitReader); // implicit_rdpcm_enabled_flag
        u_1(bitReader); // explicit_rdpcm_enabled_flag
        u_1(bitReader); // extended_precision_processing_flag
        u_1(bitReader); // intra_smoothing_disabled_flag
        u_1(bitReader); // high_precision_offsets_enabled_flag
        u_1(bitReader); // persistent_rice_adaptation_enabled_flag
        u_1(bitReader); // cabac_bypass_alignment_enabled_flag
    }
    if (bMultilayerExtensionFlag)
    {
        u_1(bitReader); // inter_view_mv_vert_constraint_flag
    }
    if (b3DExtensionFlag)
    {
        for (HI_S32 d = 0; d <= 1; d++)
        {
            u_1(bitReader); // iv_di_mc_enabled_flag[d]
            u_1(bitReader); // iv_mv_scal_enabled_flag[d]
            if (0 == d)
            {
                ue_v(bitReader); // log2_ivmc_sub_pb_size_minus3[d]
                u_1(bitReader); // iv_res_pred_enabled_flag[d]
                u_1(bitReader); // depth_ref_enabled_flag[d]
                u_1(bitReader); // vps_mc_enabled_flag[d]
                u_1(bitReader); // dbbp_enabled_flag[d]
            }
            else
            {
                u_1(bitReader); // tex_mc_enabled_flag[d]
                ue_v(bitReader); // log2_texmc_sub_pb_size_minus3[d]
                u_1(bitReader); // intra_contour_enabled_flag[d]
                u_1(bitReader); // intra_dc_only_wedge_enabled_flag[d]
                u_1(bitReader); // cqt_cu_part_pred_enabled_flag[d]
                u_1(bitReader); // inter_dc_only_enabled_flag[d]
                u_1(bitReader); // skip_intra_enabled_flag[d]
            }
        }
    }
    // more_rbsp_data


    if (HI_SUCCESS != memcpy_s(pstSpsInfo, sizeof(mSpsInfo), &mSpsInfo, sizeof(mSpsInfo)))
    {
        FFBitsReader_destroy(bitReader);
        return HI_FALSE;
    }
    FFBitsReader_destroy(bitReader);

    return HI_TRUE;
}

HI_BOOL FFHEVCParser_parseVUIParameters(HI_MW_PTR bitReader)
{
    HI_BOOL bAspectRatioInfoPresentFlag = u_1(bitReader); // aspect_ratio_info_present_flag
    const HI_U8 EXTENDED_SAR = 255;
    if (bAspectRatioInfoPresentFlag)
    {
        HI_U8 u8AspectRatioIdc = u_8(bitReader); // aspect_ratio_idc
        if (u8AspectRatioIdc == EXTENDED_SAR)
        {
            u_16(bitReader); // sar_width
            u_16(bitReader); // sar_height
        }
    }
    HI_BOOL bOverscanInfoPresentFlag = u_1(bitReader); // overscan_info_present_flag
    if (bOverscanInfoPresentFlag)
    {
        u_1(bitReader); // overscan_appropriate_flag
    }
    HI_BOOL bVideoSignalTypePresentFlag = u_1(bitReader);
    if (bVideoSignalTypePresentFlag)
    {
        u_3(bitReader); // video_format
        u_1(bitReader); // video_full_range_flag
        HI_BOOL bColourDescPresentFlag = u_1(bitReader); // colour_description_present_flag
        if (bColourDescPresentFlag)
        {
            u_8(bitReader); // colour_primaries
            u_8(bitReader); // transfer_characteristics
            u_8(bitReader); // matrix_coeffs
        }
    }
    HI_BOOL bChromaLocInfoPresentFlag = u_1(bitReader); // chroma_loc_info_present_flag
    if (bChromaLocInfoPresentFlag)
    {
        ue_v(bitReader); // chroma_sample_loc_type_top_field
        ue_v(bitReader); // chroma_sample_loc_type_bottom_field
    }
    u_1(bitReader); // neutral_chroma_indication_flag
    u_1(bitReader); // field_seq_flag
    u_1(bitReader); // frame_field_info_present_flag
    HI_BOOL bDefaultDisplayWindowFlag = u_1(bitReader);
    if (bDefaultDisplayWindowFlag)
    {
        u_1(bitReader); // def_disp_win_left_offset
        u_1(bitReader); // def_disp_win_right_offset
        u_1(bitReader); // def_disp_win_top_offset
        u_1(bitReader); // def_disp_win_bottom_offset
    }
    HI_BOOL bVUITimingInfoPresentInfo = u_1(bitReader); // vui_timing_info_present_flag
    if (bVUITimingInfoPresentInfo)
    {
        u_32(bitReader); // vui_timing_info_present_flag
        u_32(bitReader); // vui_time_scale
        HI_BOOL bVUIPocProportionalToTimingFlag = u_1(bitReader);
        if (bVUIPocProportionalToTimingFlag)
        {
            ue_v(bitReader); // vui_num_ticks_poc_diff_one_minus1
        }
        HI_BOOL bVUIHrdParametersPresentFlag = u_1(bitReader);
        if (bVUIHrdParametersPresentFlag)
        {
            FFHEVCParser_parseHRDParameters(bitReader, HI_TRUE);
        }
    }
    HI_BOOL bBitstreamRestrictionFlag = u_1(bitReader); // bitstream_restriction_flag
    if (bBitstreamRestrictionFlag)
    {
        u_1(bitReader); // tiles_fixed_structure_flag
        u_1(bitReader); // motion_vectors_over_pic_boundaries_flag
        u_1(bitReader); // restricted_ref_pic_lists_flag
        ue_v(bitReader); // min_spatial_segmentation_idc
        ue_v(bitReader); // max_bytes_per_pic_denom
        ue_v(bitReader); // max_bits_per_min_cu_denom
        ue_v(bitReader); // log2_max_mv_length_horizontal
        ue_v(bitReader); // log2_max_mv_length_vertical
    }
    return HI_TRUE;
}

HI_BOOL FFHEVCParser_parseStRefPicSet(HI_MW_PTR bitReader, HI_U32 u32StRpsIdx, HI_U32 u32NumShortTermRefPicSets, HI_U32 au32NumDeltaPocs[64])
{
    HI_BOOL bInterRefPicSetPredictionFlag = HI_FALSE;
    if (0 != u32StRpsIdx)
    {
        bInterRefPicSetPredictionFlag = u_1(bitReader);
    }
    if (bInterRefPicSetPredictionFlag)
    {
        if (u32StRpsIdx == u32NumShortTermRefPicSets)
        {
            ue_v(bitReader); // delta_idx_minus1
        }
        u_1(bitReader); // delta_rps_sign
        ue_v(bitReader); // abs_delta_rps_minus1
        au32NumDeltaPocs[u32StRpsIdx] = 0;
        for (HI_U32 j = 0; j <= au32NumDeltaPocs[u32StRpsIdx - 1]; j++)
        {
            HI_BOOL bUsedByCurrPicFlag = u_1(bitReader); // used_by_curr_pic_flag[j]
            if (!bUsedByCurrPicFlag)
            {
                u_1(bitReader); // use_delta_flag[j]
            }
        }
    }
    else
    {
        HI_U32 u32NumNegativePics = ue_v(bitReader);
        HI_U32 u32NumPositivePics = ue_v(bitReader);
        for (HI_U32 i = 0; i < u32NumNegativePics; i++)
        {
            ue_v(bitReader); // delta_poc_s0_minus1[i]
            u_1(bitReader); // used_by_curr_pic_s0_flag[i]
        }
        for (HI_U32 i = 0; i < u32NumPositivePics; i++)
        {
            ue_v(bitReader); // delta_poc_s1_minus1[i]
            u_1(bitReader); // used_by_curr_pic_s1_flag[i]
        }
    }
    return HI_TRUE;
}
