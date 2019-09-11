#ifndef __PRORES_H
#define __PRORES_H

#include <stdint.h>
#include "stream.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
uint8_t *GetNextNal(uint8_t *pBufPtr, uint32_t u32Len);

typedef struct tagPRORES_Handle_S {
    Stream_Handle_S stStream;
} PRORES_Handle_S;

int32_t PRORES_Open(PRORES_Handle_S *stHandle, const char *pszFilePath);
int32_t PRORES_ReadFrame(PRORES_Handle_S *stHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen);
int32_t PRORES_Close(PRORES_Handle_S *stHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
