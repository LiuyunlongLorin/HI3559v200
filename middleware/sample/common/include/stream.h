#ifndef STREAM_H
#define STREAM_H
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define MAX_FILE_NAME_LEN 128

typedef struct tagStream_Handle_S {
    char aszFileName[MAX_FILE_NAME_LEN];
    FILE *pFile;
    uint32_t u32FileLen;
    uint32_t u32FileReadOffset;

    uint8_t *pu8BufHeader;
    uint32_t u32BufLen;
    uint32_t u32BufReadPos;
    uint32_t pu8ReadOffset;
    uint32_t u32AvailReadLen;
} Stream_Handle_S;

int32_t Stream_Open(Stream_Handle_S *pstHandle, const char *pszFilePath, uint32_t u32BuffLen);

int32_t Stream_Close(Stream_Handle_S *pstHandle);

int32_t Stream_Reset(Stream_Handle_S *pstHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif
