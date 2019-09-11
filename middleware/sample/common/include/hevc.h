#ifndef __HEVC_H
#define __HEVC_H

#include <stdint.h>
#include "stream.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
typedef struct tagHEVC_Handle_S {
    Stream_Handle_S stStream;
} HEVC_Handle_S;

int32_t HEVC_Open(HEVC_Handle_S *stHandle, const char *pszFilePath);
int32_t HEVC_ReadFrame(HEVC_Handle_S *stHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen, uint8_t *pbKeyFrame);
int32_t HEVC_Close(HEVC_Handle_S *stHandle);
int32_t HEVC_Reset(HEVC_Handle_S *pstHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
