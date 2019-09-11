#ifndef TS_H
#define TS_H
#include <stdio.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define MAX_FILE_NAME_LEN 128
#define PES_JPEG_HEAD_LEN (14)

typedef struct tagTs_Handle_S {
    char aszFileName[MAX_FILE_NAME_LEN];
    FILE *pFile;
    uint32_t readPos;
    uint8_t u8TSPack[188];
    uint8_t *pu8ReadOffset;
} TS_Handle_S;

int32_t TS_Open(TS_Handle_S *pstHandle, const char *pszFilePath);
int32_t TS_ReadTsPack(TS_Handle_S *pstHandle);
int32_t TS_ReadPESPack(TS_Handle_S *pstHandle, int16_t pid, uint8_t *u8Buffer, uint32_t *pLen);
int32_t TS_Close(TS_Handle_S *pstHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
