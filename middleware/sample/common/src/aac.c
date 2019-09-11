#include <stdint.h>
#include "aac.h"

static uint8_t *GetNextADTS(uint8_t *pBufPtr, uint32_t u32Len)
{
    uint8_t *pEndPtr = pBufPtr + u32Len;
    while (pBufPtr != pEndPtr) {
        if (0xFF == pBufPtr[0] && 0xF1 == pBufPtr[1]) {
            return pBufPtr;
        } else {
            pBufPtr++;
        }
    }
    return NULL;
}

static uint32_t getAACFrameLen(uint8_t *adtsHeader)
{
    uint32_t framelen = 0;
    if (!adtsHeader) {
        return 0;
    }
    int f_bit = adtsHeader[3];
    int m_bit = adtsHeader[4];
    int b_bit = adtsHeader[5];
    framelen += (b_bit >> 5);
    framelen += (m_bit << 3);
    framelen += ((f_bit & 3) << 11);
    return framelen;
}

int32_t AAC_Open(AAC_Handle_S *pstHandle, const char *pszFilePath)
{
    return Stream_Open(&pstHandle->stStream, pszFilePath, 1024);
}

int32_t AAC_ReadFrame(AAC_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen)
{
    fseek(pstHandle->stStream.pFile, pstHandle->stStream.u32FileReadOffset, SEEK_SET);
    memset(pstHandle->stStream.pu8BufHeader, 0x00, pstHandle->stStream.u32BufLen);
    uint32_t readLen = fread(pstHandle->stStream.pu8BufHeader, 1, pstHandle->stStream.u32BufLen,
                             pstHandle->stStream.pFile);
    if (readLen <= 0) {
        printf("AAC_ReadFrame read file failed\n");
        pstHandle->stStream.u32FileReadOffset = 0;
        return -1;
    }
    pstHandle->stStream.u32AvailReadLen = readLen;
    uint8_t *pBufPtr = pstHandle->stStream.pu8BufHeader;
    uint8_t *pEndPtr = pBufPtr + pstHandle->stStream.u32AvailReadLen;
    uint8_t *pADTSHeader = GetNextADTS(pBufPtr, pstHandle->stStream.u32AvailReadLen);
    if (pADTSHeader) {
        *pu32FrameLen = getAACFrameLen(pADTSHeader);
        *ppFrame = pADTSHeader;
        pstHandle->stStream.u32FileReadOffset += *pu32FrameLen;
        return 0;
    } else {
        printf("aac read EOF\n");
        pstHandle->stStream.u32FileReadOffset = 0;
        return -1;
    }
}

int32_t AAC_Close(AAC_Handle_S *pstHandle)
{
    return Stream_Close(&pstHandle->stStream);
}

int32_t AAC_Reset(AAC_Handle_S *pstHandle)
{
    return Stream_Reset(&pstHandle->stStream);
}


