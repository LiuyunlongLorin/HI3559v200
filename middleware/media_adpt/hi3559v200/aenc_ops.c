#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/hi_mw_media_intf.h"
#include "comm_define.h"

#define MAX_AENC_START_CNT (5)

extern HI_HANDLE g_hACapHdl;
extern HI_HANDLE g_hACapChnHdl;
static pthread_mutex_t s_AEncLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct tagAEncStartAttr_S
{
    HI_U32 u32StartCnt;
    HI_MW_OnAEncDataCB_FN apfnDataCB[MAX_AENC_START_CNT];
    HI_VOID *apPrivData[MAX_AENC_START_CNT];
}AEncStartAttr_S;

static AEncStartAttr_S astAEncStartAttrs[] = 
{
    {0, {NULL,}, {NULL,}},
    {0, {NULL,}, {NULL,}},
    {0, {NULL,}, {NULL,}},
    {0, {NULL,}, {NULL,}},
    {0, {NULL,}, {NULL,}},
};

static HI_S32 AEncDataProc(HI_HANDLE hAEncHdl, AUDIO_STREAM_S* pAStreamData, __attribute__((unused)) HI_VOID* pPrivateData)
{
    HI_MW_MEDIA_AUDIO_S stAudioInfo = {0, 0, 0, 0, 0};
    stAudioInfo.pu8Addr = pAStreamData->pStream;
    stAudioInfo.u64PhyAddr = (HI_U64)pAStreamData->u64PhyAddr;
    stAudioInfo.u32Len = pAStreamData->u32Len;
    stAudioInfo.u64TimeStamp = pAStreamData->u64TimeStamp;
    stAudioInfo.u32Seq = pAStreamData->u32Seq;
    HI_U32 i;
    for (i = 0; i < astAEncStartAttrs[hAEncHdl].u32StartCnt; i++)
    {
        astAEncStartAttrs[hAEncHdl].apfnDataCB[i](hAEncHdl, &stAudioInfo, astAEncStartAttrs[hAEncHdl].apPrivData[i]);
    }
    return 0;
}

HI_S32 HI_MW_AEnc_Start(HI_HANDLE hAEncHdl, HI_MW_OnAEncDataCB_FN pfnDataCB, HI_VOID *pPrivData)
{
    pthread_mutex_lock(&s_AEncLock);
    HI_U32 u32StartCnt = astAEncStartAttrs[hAEncHdl].u32StartCnt;
    if (MAX_AENC_START_CNT-1 == u32StartCnt)
    {
        printf("aenc start too many times");
        pthread_mutex_unlock(&s_AEncLock);
        return HI_FAILURE;
    }
    astAEncStartAttrs[hAEncHdl].apfnDataCB[u32StartCnt] = pfnDataCB;
    astAEncStartAttrs[hAEncHdl].apPrivData[u32StartCnt] = pPrivData;
    if (0 == u32StartCnt)
    {
        HI_MAPI_AENC_CALLBACK_S stAEncCB = {AEncDataProc, NULL};
        HI_MAPI_AENC_RegisterCallback(hAEncHdl, &stAEncCB);
        HI_MAPI_AENC_BindACap(g_hACapHdl, g_hACapChnHdl, hAEncHdl);
        HI_MAPI_AENC_Start(hAEncHdl);
    }
    astAEncStartAttrs[hAEncHdl].u32StartCnt++;
    pthread_mutex_unlock(&s_AEncLock);
    return HI_SUCCESS;
}

HI_S32 HI_MW_AEnc_Stop(HI_HANDLE hAEncHdl, HI_MW_OnAEncDataCB_FN pfnDataCB)
{
    pthread_mutex_lock(&s_AEncLock);
    HI_U32 u32StartCnt = astAEncStartAttrs[hAEncHdl].u32StartCnt;
    HI_U32 i = 0, j = 0;
    for (; i < u32StartCnt; i++)
    {
        if (pfnDataCB == astAEncStartAttrs[hAEncHdl].apfnDataCB[i])
        {
            break;
        }
    }
    if (i == MAX_AENC_START_CNT)
    {
        printf("not fount stop aenc");
        pthread_mutex_unlock(&s_AEncLock);
        return HI_FAILURE;
    }

    for (j = i; j < u32StartCnt; j++)
    {
        astAEncStartAttrs[hAEncHdl].apfnDataCB[j] = astAEncStartAttrs[hAEncHdl].apfnDataCB[j+1];
        astAEncStartAttrs[hAEncHdl].apPrivData[j] = astAEncStartAttrs[hAEncHdl].apPrivData[j+1];
    }
    if (0 == --astAEncStartAttrs[hAEncHdl].u32StartCnt)
    {
        HI_MAPI_AENC_CALLBACK_S stAEncCB = {AEncDataProc, NULL};
        HI_MAPI_AENC_UnregisterCallback(hAEncHdl, &stAEncCB);
        HI_MAPI_AENC_Stop(hAEncHdl);
        HI_MAPI_AENC_UnbindACap(g_hACapHdl, g_hACapChnHdl, hAEncHdl);
    }
    pthread_mutex_unlock(&s_AEncLock);
    return HI_SUCCESS;
}
