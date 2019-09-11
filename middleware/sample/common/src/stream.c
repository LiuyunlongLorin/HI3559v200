#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stream.h"

int32_t Stream_Open(Stream_Handle_S *pstHandle, const char *pszFilePath, uint32_t u32BuffLen)
{
    size_t readLen = 0;
    strncpy(pstHandle->aszFileName, pszFilePath, MAX_FILE_NAME_LEN);
    pstHandle->aszFileName[MAX_FILE_NAME_LEN - 1] = '\0';
    char aszResolvedpath[PATH_MAX + 1];
    if (NULL == realpath(pszFilePath, aszResolvedpath)) {
        printf(" real path fail \n");
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
    if (u32BuffLen > 0) {
        pstHandle->pu8BufHeader = (uint8_t *)malloc(u32BuffLen);
        pstHandle->u32BufLen = u32BuffLen;
        memset(pstHandle->pu8BufHeader, 0x00, u32BuffLen);
        pstHandle->pu8ReadOffset = 0;
    }
    pstHandle->u32FileReadOffset = 0;
    return 0;
}

int32_t Stream_Close(Stream_Handle_S *pstHandle)
{
    free(pstHandle->pu8BufHeader);
    pstHandle->pu8BufHeader = NULL;
    pstHandle->u32BufLen = 0;
    fclose(pstHandle->pFile);
    pstHandle->pFile = NULL;
    pstHandle->pu8ReadOffset = NULL;
    pstHandle->u32AvailReadLen = 0;
    pstHandle->u32FileReadOffset = 0;
    return 0;
}

int32_t Stream_Reset(Stream_Handle_S *pstHandle)
{
    pstHandle->u32FileReadOffset = 0;
    return 0;
}