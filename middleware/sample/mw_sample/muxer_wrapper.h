#ifndef MUXER_WRAPPER_H
#define MUXER_WRAPPER_H
#include "hi_mw_type.h"
#include "hi_track_source.h"
#include "hi_recorder_pro.h"

typedef struct tagMuxerConfig_S
{
    HI_S32 s32FileAllocSize;
    HI_MW_PTR hMuxerHandle;
} MuxerConfig_S;

HI_S32 CreateMuxer(HI_CHAR *pszFileName, HI_VOID *pMuxerConfig, HI_MW_PTR *pMuxerHandle);
HI_S32 DestroyMuxer(HI_MW_PTR hMuxerHandle);
HI_S32 CreateTrack(HI_MW_PTR hMuxerHandle, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR *phTrackHandle);
HI_S32 WriteFrame(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S *pstFrameData);
HI_S32 RepairInit(HI_S32 s32RepairFreq);
HI_S32 RepairDeInit();
HI_S32 Repair(HI_CHAR *pszBackFilePath);
HI_S32 ExtractThm(const HI_CHAR *pszMP4File, const HI_CHAR *pszThmFile);

HI_S32 CreateSlaveMuxer(HI_CHAR* pszFileName, HI_VOID* pMuxerConfig, HI_MW_PTR* pMuxerHandle);
HI_S32 DestroySlaveMuxer(HI_MW_PTR hMuxerHandle);
HI_S32 CreateSlaveTrack(HI_MW_PTR hMuxerHandle, HI_Track_Source_Handle pstTrackSrcHandle, HI_MW_PTR* phTrackHandle);
HI_S32 WriteFrameToFirst(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S* pstFrameData);
HI_S32 WriteFrameToSecond(HI_MW_PTR hMuxerHandle, HI_MW_PTR hTrackHandle, HI_REC_FRAME_DATA_S* pstFrameData);

#endif
