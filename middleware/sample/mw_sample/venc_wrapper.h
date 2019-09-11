#ifndef VENC_WRAPPER_H
#define VENC_WRAPPER_H
#include "hi_mw_type.h"
#include "hi_track_source.h"
#include "rtsp_wrapper.h"
#include "hi_mw_media_intf.h"

HI_S32 GetMainVEncAttr(HI_MW_VEncAttr *pstVEncAttr);
HI_S32 GetSlaveVEncAttr(HI_MW_VEncAttr *pstVEncAttr);
HI_S32 GetLiveVEncAttr(HI_MW_VEncAttr *pstVEncAttr);
HI_S32 GetAEncAttr(HI_HANDLE *phAEnc);

HI_S32 LapseVencStart(HI_Track_Source_Handle pTrackSource,HI_MW_PTR pCaller);
HI_S32 VencStart(HI_Track_Source_S *pTrackSource,HI_MW_PTR pCaller);
HI_S32 VencStop(HI_Track_Source_S *pTrackSource,HI_MW_PTR pCaller);
HI_S32 Venc4RtspStart(HI_Track_Source_Handle pTrackSource,HI_MW_PTR pCaller);
HI_S32 Venc4RtspStop(HI_Track_Source_Handle pTrackSource,HI_MW_PTR pCaller);
HI_S32 AencStart(HI_Track_Source_S *pTrackSource,HI_MW_PTR pCaller);
HI_S32 AencStop(HI_Track_Source_S *pTrackSource,HI_MW_PTR pCaller);
HI_S32 Aenc4RtspStart(HI_Track_Source_Handle pTrackSource,HI_MW_PTR pCaller);
HI_S32 Aenc4RtspStop(HI_Track_Source_Handle pTrackSource,HI_MW_PTR pCaller);
typedef HI_S32 (*OnDataReceived_FN)(HI_U8 *u8Data, HI_U32 u32DataLen);
HI_S32 RequestThumbnail(OnDataReceived_FN pfnOnDataReceived);
HI_S32 RequestThumbnailEnd(OnDataReceived_FN pfnOnDataReceived);

HI_S32 MediaInit();
HI_S32 MediaDeInit();

#endif
