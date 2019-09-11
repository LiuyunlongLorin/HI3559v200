#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hi_comm_adec.h"
#include "aacdec.h"
#include "aac_adec_intf.h"


#define AAC_CHECK_NULL(condition) \
do \
{ \
    if(condition == HI_NULL) \
    { \
        printf(""#condition" is NULL error %s %d\n", __FUNCTION__, __LINE__);  \
        return HI_FAILURE; \
    }\
}while(0)

typedef struct hiADEC_AAC_DECODER_S
{
    HAACDecoder         pstAACDec;
} ADEC_AAC_DECODER_S;


static HI_S32 OpenAACDecoder(HI_VOID* pDecoderAttr, HI_VOID** ppDecoder)
{
    ADEC_AAC_DECODER_S* pstDecoder = NULL;
    AACDECTransportType enTransType = AACDEC_ADTS;

    AAC_CHECK_NULL(pDecoderAttr);
    AAC_CHECK_NULL(ppDecoder);


    enTransType = *(AACDECTransportType*)pDecoderAttr;

    pstDecoder = (ADEC_AAC_DECODER_S*)malloc(sizeof(ADEC_AAC_DECODER_S));

    if (NULL == pstDecoder)
    {
        printf("no memory for malloc ADEC_AAC_DECODER_S");
        return HI_FAILURE;
    }

    memset(pstDecoder, 0, sizeof(ADEC_AAC_DECODER_S));
    *ppDecoder = (HI_VOID*)pstDecoder;

    pstDecoder->pstAACDec = AACInitDecoder(enTransType);

    if (!pstDecoder->pstAACDec)
    {
        printf("AACInitDecoder failed");
        free(pstDecoder);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 DecodeAACFrm(HI_VOID* pDecoder, HI_U8** pu8Inbuf, HI_S32* ps32LeftByte,
                    HI_U16* pu16Outbuf, HI_U32* pu32OutLen, HI_U32* pu32Chns)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ADEC_AAC_DECODER_S* pstDecoder = NULL;
    HI_S32 s32Samples, s32FrmLen, s32SampleBytes;
    AACFrameInfo aacFrameInfo;

    AAC_CHECK_NULL(pDecoder);
    AAC_CHECK_NULL(pu8Inbuf);
    AAC_CHECK_NULL(ps32LeftByte);
    AAC_CHECK_NULL(pu16Outbuf);
    AAC_CHECK_NULL(pu32OutLen);
    AAC_CHECK_NULL(pu32Chns);

    *pu32Chns = 1;/*voice encoder only one channle */

    pstDecoder = (ADEC_AAC_DECODER_S*)pDecoder;

    s32FrmLen = AACDecodeFindSyncHeader(pstDecoder->pstAACDec, pu8Inbuf, ps32LeftByte);

    if (s32FrmLen < 0)
    {
        printf("AAC decoder can't find sync header");
        return HI_FAILURE;
    }

    /*Notes: pInbuf will updated*/
    s32Ret = AACDecodeFrame(pstDecoder->pstAACDec, pu8Inbuf, ps32LeftByte, (HI_S16*)pu16Outbuf);
    if (s32Ret)
    {
        return s32Ret;
    }

    AACGetLastFrameInfo(pstDecoder->pstAACDec, &aacFrameInfo);

    /* samples per frame of one sound track*/
    s32Samples = aacFrameInfo.outputSamps / aacFrameInfo.nChans;

    s32SampleBytes = s32Samples * sizeof(HI_U16);
    *pu32Chns = aacFrameInfo.nChans;
    *pu32OutLen = s32SampleBytes;

    return s32Ret;
}

static HI_S32 GetAACFrmInfo(HI_VOID* pDecoder, HI_VOID* pInfo)
{
    ADEC_AAC_DECODER_S* pstDecoder = NULL;
    AACFrameInfo aacFrameInfo;
    HI_AAC_FRAME_INFO_S* pstAacFrm = NULL;

    AAC_CHECK_NULL(pDecoder);
    AAC_CHECK_NULL(pInfo);

    pstDecoder = (ADEC_AAC_DECODER_S*)pDecoder;
    pstAacFrm = (HI_AAC_FRAME_INFO_S*)pInfo;

    AACGetLastFrameInfo(pstDecoder->pstAACDec, &aacFrameInfo);

    pstAacFrm->s32Samplerate = aacFrameInfo.sampRateOut;
    pstAacFrm->s32BitRate = aacFrameInfo.bitRate;
    pstAacFrm->s32Profile = aacFrameInfo.profile;
    pstAacFrm->s32ChnCnt = aacFrameInfo.nChans;
    pstAacFrm->s32Bitwith = aacFrameInfo.bitsPerSample;

    return HI_SUCCESS;
}


static HI_S32 CloseAACDecoder(HI_VOID* pDecoder)
{
    ADEC_AAC_DECODER_S* pstDecoder = NULL;
    AAC_CHECK_NULL(pDecoder);

    pstDecoder = (ADEC_AAC_DECODER_S*)pDecoder;

    AACFreeDecoder(pstDecoder->pstAACDec);

    free(pstDecoder);
    return HI_SUCCESS;
}


static HI_S32 ResetAACDecoder(HI_VOID* pDecoder)
{
    ADEC_AAC_DECODER_S* pstDecoder = NULL;
    AAC_CHECK_NULL(pDecoder);

    pstDecoder = (ADEC_AAC_DECODER_S*)pDecoder;

    AACFreeDecoder(pstDecoder->pstAACDec);

    pstDecoder->pstAACDec = AACInitDecoder(AACDEC_ADTS);

    if (!pstDecoder->pstAACDec)
    {
        printf( "AACResetDecoder failed");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

ADEC_DECODER_S g_stAudioDecoder =
{
    .enType = PT_AAC,
    "aac",
    .pfnOpenDecoder = OpenAACDecoder,
    .pfnDecodeFrm = DecodeAACFrm,
    .pfnGetFrmInfo = GetAACFrmInfo,
    .pfnCloseDecoder = CloseAACDecoder,
    .pfnResetDecoder = ResetAACDecoder,
};
