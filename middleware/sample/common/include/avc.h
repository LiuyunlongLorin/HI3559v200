#ifndef __AVC_H
#define __AVC_H

#include <stdint.h>
#include "stream.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

uint8_t *GetNextNal(uint8_t *pBufPtr, uint32_t u32Len);

typedef struct tagAVC_Handle_S {
    Stream_Handle_S stStream;
} AVC_Handle_S;

int32_t AVC_Open(AVC_Handle_S *stHandle, const char *pszFilePath);
int32_t AVC_ReadFrame(AVC_Handle_S *stHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen, uint8_t *pbKeyFrame);
int32_t AVC_Close(AVC_Handle_S *stHandle);
int32_t AVC_Reset(AVC_Handle_S *pstHandle);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
