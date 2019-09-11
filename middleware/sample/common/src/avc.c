#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avc.h"

uint8_t *GetNextNal(uint8_t *pBufPtr, uint32_t u32Len)
{
    uint8_t *pEndPtr = pBufPtr + u32Len;
    while (pBufPtr != pEndPtr) {
        if (0x00 == pBufPtr[0] && 0x00 == pBufPtr[1]
            && 0x00 == pBufPtr[2] && 0x01 == pBufPtr[3]) {
            return pBufPtr;
        } else {
            pBufPtr++;
        }
    }
    return NULL;
}

int32_t AVC_Open(AVC_Handle_S *pstHandle, const char *pszFilePath)
{
    return Stream_Open(&pstHandle->stStream, pszFilePath, 512 * 1024);
}

int32_t AVC_ReadFrame(AVC_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen, uint8_t *pbKeyFrame)
{
    fseek(pstHandle->stStream.pFile, pstHandle->stStream.u32FileReadOffset, SEEK_SET);
    memset(pstHandle->stStream.pu8BufHeader, 0x00, pstHandle->stStream.u32BufLen);
    uint32_t readLen = fread(pstHandle->stStream.pu8BufHeader, 1, pstHandle->stStream.u32BufLen,
                             pstHandle->stStream.pFile);
    if (readLen <= 0) {
        printf("AVC_ReadFrame read file failed\n");
        return -1;
    }
    pstHandle->stStream.u32AvailReadLen = readLen;

    uint8_t *pBufPtr = pstHandle->stStream.pu8BufHeader;
    uint8_t *pEndPtr = pBufPtr + pstHandle->stStream.u32AvailReadLen;
    uint8_t *pFirstNalPtr = GetNextNal(pBufPtr, pstHandle->stStream.u32AvailReadLen);
    uint8_t *pSecondNalPtr = GetNextNal(pFirstNalPtr + 4, pEndPtr - (pFirstNalPtr + 4));
    uint8_t bKeyFrame = 0;

    if (!pFirstNalPtr) {
        printf("avc read error\n");
        pstHandle->stStream.u32FileReadOffset = 0;
        return -1;
    }
    uint8_t *pNalByte = (pFirstNalPtr + 4);
    int32_t s32NalType = pNalByte[0] & 0x1F;

    if (7 == s32NalType) { // SPS
        int32_t s32FrameType = (pSecondNalPtr + 4)[0] & 0x1F;
        while (5 != s32FrameType && pSecondNalPtr && pSecondNalPtr < pEndPtr)  // 5 IDR
        {
            pSecondNalPtr = GetNextNal(pSecondNalPtr + 4, pEndPtr - (pSecondNalPtr + 4));
            s32FrameType = (pSecondNalPtr + 4)[0] & 0x1F;
        }
        if (5 == s32FrameType) {
            bKeyFrame = 1;
            pSecondNalPtr = GetNextNal(pSecondNalPtr + 4, pEndPtr - (pSecondNalPtr + 4));
        }
    } else if (5 == s32NalType) {
        bKeyFrame = 1;
    }

    if (!pSecondNalPtr) {
        printf("read second nal error\n");
        *pu32FrameLen = pstHandle->stStream.u32AvailReadLen;
        *ppFrame = pFirstNalPtr;
        *pbKeyFrame = bKeyFrame;
    } else {
        *pu32FrameLen = (pSecondNalPtr - pFirstNalPtr);
        *ppFrame = pFirstNalPtr;
        *pbKeyFrame = bKeyFrame;
    }
    pstHandle->stStream.u32FileReadOffset += *pu32FrameLen;
    return 0;
}

int32_t AVC_Reset(AVC_Handle_S *pstHandle)
{
    return Stream_Reset(&pstHandle->stStream);
}

int32_t AVC_Close(AVC_Handle_S *pstHandle)
{
    return Stream_Close(&pstHandle->stStream);
}
