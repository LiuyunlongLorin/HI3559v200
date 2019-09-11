#ifndef JPG_H
#define JPG_H
#include "stream.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
typedef struct tagJPG_Handle_S {
    Stream_Handle_S stStream;
} JPG_Handle_S;

int32_t JPG_Open(JPG_Handle_S *pstHandle, const char *pszFilePath);

int32_t JPG_ReadFrame(JPG_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen);

int32_t JPG_Close(JPG_Handle_S *pstHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
