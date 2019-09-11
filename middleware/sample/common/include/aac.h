#ifndef AAC_H
#define AAC_H
#include "stream.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
typedef struct tagAAC_Handle_S
{
    Stream_Handle_S stStream;
} AAC_Handle_S;

int32_t AAC_Open(AAC_Handle_S* pstHandle, const char* pszFilePath);

int32_t AAC_ReadFrame(AAC_Handle_S* pstHandle, uint8_t** ppFrame, uint32_t* pu32FrameLen);

int32_t AAC_Close(AAC_Handle_S* pstHandle);

int32_t  AAC_Reset(AAC_Handle_S* pstHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
