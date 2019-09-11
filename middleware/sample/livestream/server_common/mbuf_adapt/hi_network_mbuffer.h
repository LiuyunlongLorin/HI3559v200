/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_network_mbuffer.h
* @brief     livestream common mbuffer adapt head file
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_NETWORK_MBUFFER_H__
#define __HI_NETWORK_MBUFFER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_mw_type.h"
#include "hi_mbuf_ext.h"
#include "hi_mbuf_define_ext.h"

#define MAX_PAYLOAD_TYPE (254)
#define MAX_WAIT_COUNT (100)

#define DEFAULT_MBUF_SIZE (1280*720)

typedef enum hiBUF_DATATYPE_E
{
    BUF_DATATYPE_VIDEO = 1,
    BUF_DATATYPE_AUDIO,
    BUF_DATATYPE_DATA,
    BUF_DATATYPE_BUTT
} BUF_DATATYPE_E;

typedef struct hiNETWORK_BUFFER_INFO_S
{
    pthread_mutex_t  mutex;/*lock for mbuffer write pos */

} HI_NETWORK_BUFFER_INFO_S;

typedef enum hiBUF_PAYLOAD_E
{
    HI_BUF_PAYLOAD_G711Mu  = 0,   /**< G.711 Mu           */
    HI_BUF_PAYLOAD_G711A   = 8,   /**< G.711 A            */
    HI_BUF_PAYLOAD_G726    = 97,   /**< G.726              */
    HI_BUF_PAYLOAD_H264  = 96,  /**< H264  */
    HI_BUF_PAYLOAD_H265  = 98,  /**< H265  */
    HI_BUF_PAYLOAD_ADPCM  = 104,   /**< ADPCM              */
    HI_BUF_PAYLOAD_AAC  = 105,   /**< AAC encoder        */
    HI_BUF_PAYLOAD_BUTT         /**< invalid*/
} HI_BUF_PAYLOAD_E;

HI_S32 HI_NETWORK_MBuf_OnCBFrmWrite(HI_MW_PTR hHandle,  HI_MBUF_SLICEINFO_S* pstFrameInfo);

HI_S32 HI_NETWORK_MBufferCreate(HI_VOID* argv, HI_MW_PTR* pBufHandle, HI_U32 u32BufSize, HI_S32 s32MaxPayload);

HI_S32 HI_NETWORK_MBufferDestroy(HI_VOID* argv, HI_MW_PTR hHandle);

HI_S32 HI_NETWORK_MBufferRegister(HI_MW_PTR hHandle, HI_U8 u8MediaType);

HI_S32 HI_NETWORK_MBufferUnRegister(HI_MW_PTR hHandle, HI_U8 u8MediaType);

HI_S32 HI_NETWORK_MBufferRead(HI_MW_PTR hHandle, HI_VOID** paddr, HI_U32* plen,
                              HI_U64* ppts, BUF_DATATYPE_E* type,HI_BOOL* pbKeyFlag);

HI_S32 HI_NETWORK_MBufferSet(HI_MW_PTR hHandle);

HI_S32 HI_NETWORK_MBufferGetPts(HI_MW_PTR hHandle, HI_U8 u8PayLoadType, HI_U64* ppts);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /*__HI_NETWORK_MBUFFER_H__*/
