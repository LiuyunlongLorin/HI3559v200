#include <stdio.h>
#include "ffhevcparser.h"
#define HEVC_LAYER_FLAG_LEN (256)
HI_BOOL FFHEVCParser_parseNalUnitHeader(HI_MW_PTR bitReader)
{
    u_1(bitReader); // forbidden_zero_bit
    u_6(bitReader); // nal_unit_type
    u_6(bitReader); // nuh_layer_id
    u_3(bitReader); // nuh_temporal_id_plus1
    return HI_TRUE;
}

HI_BOOL FFHEVCParser_parseProfileTierLevel(HEVC_SPS_INFO* pstSPSInfo, HI_MW_PTR bitReader, HI_U8 u8ProfilePresentFlag, HI_U8 u8MaxSubLayersMinus1)
{
    HI_BOOL abSubLayerProfilePresentFlag[HEVC_LAYER_FLAG_LEN];
    HI_BOOL abSubLayerLevelPresentFlag[HEVC_LAYER_FLAG_LEN];

    if (u8ProfilePresentFlag)
    {
        u_2(bitReader); // general_profile_space
        u_1(bitReader); // general_tier_flag
        u_5(bitReader); // general_profile_idc
        for (HI_U32 j = 0; j < 32; j++)
        {
            u_1(bitReader); // general_profile_compatibility_flag[j]
        }

        pstSPSInfo->u32GeneralFlag0 = u_32(bitReader);
        pstSPSInfo->u16GeneralFlag1 = u_16(bitReader);
    }
    u_8(bitReader); // general_level_idc

    for (HI_S32 i = 0; i < u8MaxSubLayersMinus1; i++)
    {
        abSubLayerProfilePresentFlag[i] = u_1(bitReader); // sub_layer_profile_present_flag[i]
        abSubLayerLevelPresentFlag[i] = u_1(bitReader); // sub_layer_level_present_flag[i]
    }
    if (u8MaxSubLayersMinus1 > 0)
    {
        for (HI_S32 i = u8MaxSubLayersMinus1; i < 8; i++)
        {
            u_2(bitReader); // reserved_zero_2bits
        }
    }
    for (HI_U8 i = 0; i < u8MaxSubLayersMinus1; i++)
    {
        if (abSubLayerProfilePresentFlag[i])
        {
            u_2(bitReader); // sub_layer_profile_space[i]
            u_1(bitReader); // sub_layer_tier_flag[i]
            u_5(bitReader); // sub_layer_profile_idc[i]
            for (HI_S32 j = 0; j < 32; j++)
            {
                u_1(bitReader); // sub_layer_profile_compatibility_flag[i][j]
            }
            u_1(bitReader); // sub_layer_progressive_source_flag[i]
            u_1(bitReader); // sub_layer_HI_S32erlaced_source_flag[i]
            u_1(bitReader); // sub_layer_non_packed_constraint_flag[i]
            u_1(bitReader); // sub_layer_frame_only_constraint_flag[i]
            u_8(bitReader); // skip 43 bits
            u_8(bitReader);
            u_8(bitReader);
            u_8(bitReader);
            u_8(bitReader);
            u_3(bitReader);
            u_1(bitReader); // sub_layer_inbld_flag[i] or sub_layer_reserved_zero_bit[i]
        }
        if (abSubLayerLevelPresentFlag[i])
        {
            u_8(bitReader); // sub_layer_level_idc[i]
        }
    }
    return HI_TRUE;
}

HI_BOOL FFHEVCParser_parseHRDParameters(HI_MW_PTR bitReader, HI_BOOL bCommonInfPresentFlag)
{
    if (bCommonInfPresentFlag)
    {
        HI_BOOL bNalHRDParametersPresentFlag = u_1(bitReader); // nal_hrd_parameters_present_flag
        HI_BOOL bVclHRDParametersPresentFlag = u_1(bitReader); // vcl_hrd_parameters_present_flag
        if (bNalHRDParametersPresentFlag || bVclHRDParametersPresentFlag)
        {
            HI_BOOL bSubPicHRDParametersPresentFlag = u_1(bitReader);
            if (bSubPicHRDParametersPresentFlag)
            {
                u_8(bitReader); // tick_divisor_minus2
                u_5(bitReader); // du_cpb_removal_delay_increment_length_minus1
                u_1(bitReader); // sub_pic_cpb_params_in_pic_timing_sei_flag
                u_5(bitReader); // dpb_output_delay_du_length_minus1
            }
        }
    }
    return HI_TRUE;
}

HI_BOOL FFHEVCParser_parseScalingListData(HI_MW_PTR bitReader)
{
    for (HI_U8 u8SizeId = 0; u8SizeId < 4; u8SizeId++)
    {
        for (HI_U8 u8MatrixId = 0; u8MatrixId < 6; u8MatrixId += (3 == u8SizeId) ? 3 : 1)
        {
            HI_BOOL bScalingListPredModeFlag = u_1(bitReader); // scaling_list_pred_mode_flag[sizeId][matrixId]
            if (!bScalingListPredModeFlag)
            {
                ue_v(bitReader); // scaling_list_pred_matrix_id_delta[sizeId][matrixId]
            }
            else
            {
                HI_U32 u32NextCoef = 8;
                HI_U32 u32CoefNum = (1 << (4 + (u8SizeId << 1)));
                u32CoefNum = u32CoefNum < 64 ? u32CoefNum : 64;
                if (u8SizeId > 1)
                {
                    HI_S32 s32ScalingListDcCoefMinus8 = se_v(bitReader); // scaling_list_dc_coef_minus8[sizeId-2][matrixId]
                    u32NextCoef = s32ScalingListDcCoefMinus8 + 8;
                }
                for (HI_U32 i = 0; i < u32CoefNum; i++)
                {
                    HI_S32 s32ScalingListDeltaCoef = se_v(bitReader); // scaling_list_delta_coef
                    u32NextCoef = (u32NextCoef + s32ScalingListDeltaCoef + 256) % 256;

                }
            }
        }
    }
    return HI_TRUE;
}
