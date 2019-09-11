#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"

int32_t TS_Open(TS_Handle_S *pstHandle, const char *pszFilePath)
{
    strncpy(pstHandle->aszFileName, pszFilePath, MAX_FILE_NAME_LEN);
    pstHandle->aszFileName[MAX_FILE_NAME_LEN - 1] = '\0';
    char aszResolvedpath[MAX_FILE_NAME_LEN + 1];
    if (NULL == realpath(pszFilePath, aszResolvedpath)) {
        printf(" real path fail \n");
        return -1;
    }
    pstHandle->pFile = fopen(aszResolvedpath, "r");
    if (NULL == pstHandle->pFile) {
        printf("fopen  fail error %s  errno: %d\n", strerror(errno), errno);
        return -1;
    }
    pstHandle->readPos = 0;
    return 0;
}

int32_t TS_ReadTsPack(TS_Handle_S *pstHandle)
{
    int readLen = fread(&pstHandle->u8TSPack, 188, 1, pstHandle->pFile);
    if (readLen <= 0) {
        printf("fread fail error %s  errno: %d\n", strerror(errno), errno);
        fclose(pstHandle->pFile);
        pstHandle->pFile = NULL;
        return -1;
    }
    pstHandle->readPos += 188;
    if (-1 == fseek(pstHandle->pFile, pstHandle->readPos, SEEK_SET)) {
        printf("fseek stream fail error %s  errno: %d\n", strerror(errno), errno);
        fclose(pstHandle->pFile);
        pstHandle->pFile = NULL;
        return -1;
    }
    return 0;
}

int32_t TS_GetTsPID(TS_Handle_S *pstHandle, int16_t *int16Pid)
{
    int16_t Pid;
    Pid = pstHandle->u8TSPack[1];
    Pid = Pid << 8;
    Pid = Pid + pstHandle->u8TSPack[2];
    *int16Pid = Pid & 0x1FFF;
    return 0;
}

int32_t TS_ReadPESPack(TS_Handle_S *pstHandle, int16_t pid, uint8_t *u8Buffer, uint32_t *pLen)
{
    int16_t int16Pid;
    int pesLen = 0;
    *pLen = 0;
    while (0 == TS_ReadTsPack(pstHandle)) {
        TS_GetTsPID(pstHandle, &int16Pid);
        if (pid == int16Pid) {
            memcpy(u8Buffer + pesLen, &pstHandle->u8TSPack[4], 184);
            pesLen += 184;
        } else {
            if (pesLen > 0) {
                *pLen = pesLen;
                break;
            }
        }
    }
    return 0;
}

int32_t TS_Close(TS_Handle_S *pstHandle)
{
    if (NULL != pstHandle->pFile) {
        fclose(pstHandle->pFile);
        pstHandle->pFile = NULL;
    }
    pstHandle->pu8ReadOffset = NULL;
    return 0;
}
