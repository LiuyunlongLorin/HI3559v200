#include <stdint.h>
#include "jpg.h"

int32_t JPG_Open(JPG_Handle_S *pstHandle, const char *pszFilePath)
{
    return Stream_Open(&pstHandle->stStream, pszFilePath, 5 * 1024 * 1024);
}

int32_t JPG_ReadFrame(JPG_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen)
{
    fseek(pstHandle->stStream.pFile, pstHandle->stStream.u32FileReadOffset, SEEK_SET);
    memset(pstHandle->stStream.pu8BufHeader, 0x00, pstHandle->stStream.u32BufLen);
    uint32_t readLen = fread(pstHandle->stStream.pu8BufHeader, 1, pstHandle->stStream.u32BufLen,
                             pstHandle->stStream.pFile);
    if (readLen <= 0) {
        printf("read file failed");
        return -1;
    }
    pstHandle->stStream.u32AvailReadLen = readLen;
    uint8_t *pBufPtr = pstHandle->stStream.pu8BufHeader;
    *pu32FrameLen = (pstHandle->stStream.u32AvailReadLen);
    *ppFrame = pBufPtr;
    return 0;
}

int32_t JPG_Close(JPG_Handle_S *pstHandle)
{
    return Stream_Close(&pstHandle->stStream);
}
