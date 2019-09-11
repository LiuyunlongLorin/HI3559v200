#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prores.h"
#include <stdint.h>
#include <string.h>
#include <limits.h>

static int32_t File_Open(Stream_Handle_S *pstHandle, const char *pszFilePath)
{
    size_t readLen = 0;
    strncpy(pstHandle->aszFileName, pszFilePath, MAX_FILE_NAME_LEN);
    pstHandle->aszFileName[MAX_FILE_NAME_LEN - 1] = '\0';
    char aszResolvedpath[PATH_MAX + 1];
    if (NULL == realpath(pszFilePath, aszResolvedpath)) {
        printf("File_Open real path fail \n");
        return -1;
    }
    pstHandle->pFile = fopen(aszResolvedpath, "r");
    if (NULL == pstHandle->pFile) {
        printf("fopen stream fail\n");
        return -1;
    }
    if (-1 == fseek(pstHandle->pFile, 0, SEEK_END)) {
        printf("fseek stream fail\n");
        fclose(pstHandle->pFile);
        return -1;
    }
    pstHandle->u32FileLen = ftell(pstHandle->pFile);
    if (-1 == fseek(pstHandle->pFile, 0, SEEK_SET)) {
        printf("fseek stream fail\n");
        fclose(pstHandle->pFile);
        return -1;
    }
    pstHandle->pu8BufHeader = NULL;
    pstHandle->u32BufLen = 0;
    pstHandle->u32AvailReadLen = pstHandle->u32FileLen;
    return 0;
}

int32_t File_Close(Stream_Handle_S *pstHandle)
{
    pstHandle->u32BufLen = 0;
    fclose(pstHandle->pFile);
    pstHandle->pFile = NULL;
    pstHandle->u32AvailReadLen = 0;
    if (pstHandle->pu8BufHeader != NULL) {
        free(pstHandle->pu8BufHeader);
    }
    pstHandle->pu8BufHeader = NULL;
    pstHandle->u32FileLen = 0;
    return 0;
}

/* int32_t PRORES_Open(PRORES_Handle_S *pstHandle, const char *pszFilePath)
{
    return Stream_Open(&pstHandle->stStream, pszFilePath);
}

int32_t PRORES_ReadFrame(PRORES_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen)
{
    uint8_t *pBufPtr = pstHandle->stStream.pu8ReadOffset;
    uint8_t *pEndPtr = pBufPtr + pstHandle->stStream.u32AvailReadLen;

    if (pstHandle->stStream.u32AvailReadLen <= 4) {
        perror("prores read error  not read available\n");
        return -1;
    }

    // get frame len
    uint32_t u32Temp = 0;
    u32Temp |= (uint32_t)pBufPtr[3];
    u32Temp |= (uint32_t)pBufPtr[2] << 8;
    u32Temp |= (uint32_t)pBufPtr[1] << 16;
    u32Temp |= (uint32_t)pBufPtr[0] << 24;

    if (pstHandle->stStream.u32AvailReadLen <= u32Temp) {
        perror("prores read error not read available\n");
        return -1;
    }


    * pu32FrameLen = u32Temp;
    * ppFrame = pBufPtr;
    pstHandle->stStream.pu8ReadOffset = pBufPtr + *pu32FrameLen;
    pstHandle->stStream.u32AvailReadLen -= *pu32FrameLen;
    return 0;
}
int32_t PRORES_Close(PRORES_Handle_S *pstHandle)
{
    return Stream_Close(&pstHandle->stStream);
}

*/

int32_t PRORES_Open(PRORES_Handle_S *pstHandle, const char *pszFilePath)
{
    return File_Open(&pstHandle->stStream, pszFilePath);
}

int32_t PRORES_ReadFrame(PRORES_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen)
{
    // read four byte
    if (pstHandle->stStream.u32AvailReadLen <= 4) {
        perror("prores read error  not read available\n");
        return -1;
    }

    // get frame len
    uint8_t au8Buf[4] = { 0 };
    if (4 != fread(au8Buf, 1, 4, pstHandle->stStream.pFile)) {
        perror("prores read error  not read available\n");
        return -1;
    }

    uint32_t u32Temp = 0;
    u32Temp |= (uint32_t)au8Buf[3];
    u32Temp |= (uint32_t)au8Buf[2] << 8;
    u32Temp |= (uint32_t)au8Buf[1] << 16;
    u32Temp |= (uint32_t)au8Buf[0] << 24;

    if ((pstHandle->stStream.u32AvailReadLen <= u32Temp) || (u32Temp < 4)) {
        perror("prores read error not read available\n");
        return -1;
    }

    // read frame

    if (pstHandle->stStream.u32BufLen < u32Temp || NULL == pstHandle->stStream.pu8BufHeader) {
        if (NULL != pstHandle->stStream.pu8BufHeader) {
            free(pstHandle->stStream.pu8BufHeader);
            pstHandle->stStream.pu8BufHeader = NULL;
        }

        pstHandle->stStream.pu8BufHeader = (uint8_t *)malloc(u32Temp);
        if (NULL == pstHandle->stStream.pu8BufHeader) {
            perror("prores malloc error\n");
            return -1;
        }
    }

    pstHandle->stStream.u32BufLen = u32Temp;

    memcpy(pstHandle->stStream.pu8BufHeader, au8Buf, 4);
    uint8_t *pTmpBuf = pstHandle->stStream.pu8BufHeader + 4;

    if ((u32Temp - 4) != fread(pTmpBuf, 1, (u32Temp - 4), pstHandle->stStream.pFile)) {
        perror("prores read error  not read available\n");
        return -1;
    }

    *pu32FrameLen = u32Temp;
    *ppFrame = pstHandle->stStream.pu8BufHeader;
    pstHandle->stStream.u32AvailReadLen -= u32Temp;
    return 0;
}

int32_t PRORES_Close(PRORES_Handle_S *pstHandle)
{
    return File_Close(&pstHandle->stStream);
}
