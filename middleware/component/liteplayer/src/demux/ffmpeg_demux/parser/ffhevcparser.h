#ifndef HEVCPARSER_H
#define HEVCPARSER_H
#include "ffbitsreader.h"

typedef struct HEVCSpsInfo
{
    HI_U8 u8MaxSubLayersMinus1;
    HI_BOOL bTemporalIdNestingFlag;
    HI_U8 u8GeneralProfileSpace;
    HI_U8 u8GeneralTierFlag;
    HI_U8 u8GeneralProfileIdc;
    HI_U32 u32PicWidthInLumaSamples;
    HI_U32 u32PicHeightInLumaSamples;
    HI_U32 u32GeneralProfileCompatibilityFlag;
    HI_U32 u32GeneralFlag0; // generalflag 6 bytes
    HI_U16 u16GeneralFlag1;
    HI_U32 u32MinSpatialSegmentationIdc;
    HI_U32 u32ChromaFormatIdc;
    HI_U32 u32BitDepthLumaMinus8;
    HI_U32 u32BitDepthChromaMinus8;
}HEVC_SPS_INFO;

struct PpsInfo
{
    HI_BOOL bTilesEnabledFlag;
    HI_BOOL bEntropyCodingSyncEnabledFlag;
};

struct VpsInfo
{
    HI_U8 u8MaxSubLayersMinus1;
};

HI_BOOL FFHEVCParser_parseNalUnitHeader(HI_MW_PTR bitReader);
HI_BOOL FFHEVCParser_parseProfileTierLevel(HEVC_SPS_INFO* pstSPSInfo,HI_MW_PTR bitReader, HI_U8 u8ProfilePresentFlag, HI_U8 u8MaxSubLayersMinus1);
HI_BOOL FFHEVCParser_parseScalingListData(HI_MW_PTR bitReader);
HI_BOOL FFHEVCParser_parseStRefPicSet(HI_MW_PTR bitReader, HI_U32 u32StRpsIdx, HI_U32 u32NumShortTermRefPicSets, HI_U32 au32NumDeltaPocs[64]);
HI_BOOL FFHEVCParser_parseVUIParameters(HI_MW_PTR bitReader);
HI_BOOL FFHEVCParser_parseHRDParameters(HI_MW_PTR bitReader, HI_BOOL bCommonInfPresentFlag);


HI_BOOL FFHEVCParser_parseSps(HEVC_SPS_INFO* pstSpsInfo ,HI_U8* pu8Sps, HI_U32 u32SpsLen);


#endif
