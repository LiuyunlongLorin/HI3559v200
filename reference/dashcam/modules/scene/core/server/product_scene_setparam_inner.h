#include "hi_common.h"
#include "hi_comm_dis.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vpss.h"
#include "hi_comm_venc.h"
#include "hi_product_scene_specparam.h"

#ifdef SUPPORT_PQ_HDRMODE
#include "mpi_hdr.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

HI_S32 PDT_SCENE_SetStaticAE(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticAERouteEX(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticAERoute(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticSFAERoute(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticAWB(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticLDCI(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticClut(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticCA(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticCrossTalk(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticNr(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticCAC(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticStatisticsCfg(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed);

HI_S32 PDT_SCENE_SetStaticShading(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticSharpen(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticDRC(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticWDRExposure(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticFSWDR(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticDEHAZE(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticDemosaic(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticDE(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticDPC(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticSaturation(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticCCM(VI_PIPE ViPipe, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetStaticQP(VENC_CHN VeChn, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetDynamicGamma(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetDynamicAE(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);

HI_S32 PDT_SCENE_SetDynamicDEHAZE(VI_PIPE ViPipe, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure);

HI_S32 PDT_SCENE_SetDynamicShading(VI_PIPE ViPipe, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure);

HI_S32 PDT_SCENE_SetDynamicQVBRVENC(VENC_CHN VeChn, HI_U8 u8Index, HI_U64 u64Exposure, HI_U64 u64LastExposure);

HI_S32 PDT_SCENE_SetPipeParam(HI_PDT_SCENE_PIPE_PARAM_S *pstScenePipeParam, HI_U32 u32Num);

HI_S32 PDT_SCENE_SetDynamicFsWdr(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetDynamicWdrExposure(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation,
                                       HI_U64 u64Exposure, HI_U64 u64LastExposure);

HI_S32 PDT_SCENE_SetDynamicClut(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetDynamicDRC(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetDynamicNR(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetDynamicCAC(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetDynamicBlackLevel(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ISO, HI_U32 u32LastISO);

HI_S32 PDT_SCENE_SetPause(HI_BOOL bPause);

HI_BOOL PDT_SCENE_GetPauseState(HI_VOID);

HI_PDT_SCENE_PIPE_PARAM_S *PDT_SCENE_GetParam(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif