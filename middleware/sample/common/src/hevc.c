#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hevc.h"

const static int HEVC_VPS = 0x40;
const static int HEVC_SPS = 0x42;
const static int HEVC_PPS = 0x44;
const static int HEVC_SEI = 0x4E;
const static int HEVC_IDR = 0x26;
const static int HEVC_PSLICE = 0x02;

static uint8_t *GetNextNal(uint8_t *pBufPtr, uint32_t u32Len)
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

int32_t HEVC_Open(HEVC_Handle_S *pstHandle, const char *pszFilePath)
{
    return Stream_Open(&pstHandle->stStream, pszFilePath, 512 * 1024);
}

int32_t HEVC_ReadFrame(HEVC_Handle_S *pstHandle, uint8_t **ppFrame, uint32_t *pu32FrameLen, uint8_t *pbKeyFrame)
{
    fseek(pstHandle->stStream.pFile, pstHandle->stStream.u32FileReadOffset, SEEK_SET);
    memset(pstHandle->stStream.pu8BufHeader, 0x00, pstHandle->stStream.u32BufLen);
    uint32_t readLen = fread(pstHandle->stStream.pu8BufHeader, 1, pstHandle->stStream.u32BufLen,
                             pstHandle->stStream.pFile);
    if (readLen <= 0) {
        printf("read file failed\n");
        return -1;
    }
    pstHandle->stStream.u32AvailReadLen = readLen;
    uint8_t *pBufPtr = pstHandle->stStream.pu8BufHeader;
    uint8_t *pEndPtr = pBufPtr + pstHandle->stStream.u32AvailReadLen;

    uint8_t *pFirstNalPtr = GetNextNal(pBufPtr, pstHandle->stStream.u32AvailReadLen);
    uint8_t *pSecondNalPtr = GetNextNal(pFirstNalPtr + 4, pEndPtr - (pFirstNalPtr + 4));
    uint8_t bKeyFrame = 0;

    if (!pFirstNalPtr) {
        printf("hevc read error\n");
        return -1;
    }
    uint8_t *pNalByte = (pFirstNalPtr + 4);
    int32_t s32NalType = pNalByte[0] & 0xFF;

    if (HEVC_VPS == s32NalType || HEVC_SPS == s32NalType) { // VPS
        int32_t s32FrameType = (pSecondNalPtr + 4)[0] & 0xFF;
        while (HEVC_IDR != s32FrameType && pSecondNalPtr && pSecondNalPtr < pEndPtr)  // 5 IDR
        {
            pSecondNalPtr = GetNextNal(pSecondNalPtr + 4, pEndPtr - (pSecondNalPtr + 4));
            s32FrameType = (pSecondNalPtr + 4)[0] & 0xFF;
        }
        if (HEVC_IDR == s32FrameType) {
            bKeyFrame = 1;
            pSecondNalPtr = GetNextNal(pSecondNalPtr + 4, pEndPtr - (pSecondNalPtr + 4));
        }
    } else if (HEVC_IDR == s32NalType) {
        bKeyFrame = 1;
    }

    if (!pSecondNalPtr) {
        fprintf(stderr, "read second nal error\n");
        *pu32FrameLen = pstHandle->stStream.u32AvailReadLen;
        *ppFrame = pFirstNalPtr;
    } else {
        *pu32FrameLen = (pSecondNalPtr - pFirstNalPtr);
        *ppFrame = pFirstNalPtr;
    }
    *pbKeyFrame = bKeyFrame;
    pstHandle->stStream.u32FileReadOffset += *pu32FrameLen;
    return 0;
}

int32_t HEVC_Close(HEVC_Handle_S *pstHandle)
{
    return Stream_Close(&pstHandle->stStream);
}

int32_t HEVC_Reset(HEVC_Handle_S *pstHandle)
{
    return Stream_Reset(&pstHandle->stStream);
}


