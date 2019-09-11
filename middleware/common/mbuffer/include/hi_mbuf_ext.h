/**
* Copyright (C), 2016-2030, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file       hi_mbuf_ext.h
* @brief     common module header file- interface of mbuffer used outside
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_MBUF_EXT_H__
#define __HI_MBUF_EXT_H__

#include "hi_mbuf_define_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     COMMON */
/** @{ */ /** <!-- [COMMON] */

/**
 * @brief allocate of MBUF.
 * @param[in] HI_VOID   *argv   :passing param
 * @param[in,out] HI_MBUF_CFG_S   *pstMbufCfg : property of Buffer
 * @param[out,in]  HI_MW_PTR*         pHandle:      MbufHandle
 * @return  HI_SUCCESS success
 * @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
 * @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
 */

HI_S32 HI_MBUF_GetBuffer(HI_VOID *argv, HI_MBUF_CFG_S *pstMbufCfg, HI_MW_PTR *pHandle);

/**
 * @brief realse MBUF.
 * @param[in] HI_VOID   *argv   :passing param
 * @param[out,in]  HI_MW_PTR         hHandle:      MbufHandle
 * @return  HI_SUCCESS success
 * @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
 * @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
 */

HI_S32 HI_MBUF_ReleaseBuffer(HI_VOID *argv, HI_MW_PTR hHandle);

/**
 * @brief register MBUF payload type, for example 0 for video 1 for audio.
 * @param[in] HI_MW_PTR hHandle : MbufHandle
 * @param[in]  HI_U8   u8PayloadType:      payload type 0~254
 * @return  HI_SUCCESS success
 * @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
 * @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
 */

HI_S32 HI_MBUF_RegisterPayload(HI_MW_PTR hHandle, HI_U8 u8PayloadType);

/**
 * @brief unregister MBUF payload type.
 * @param[in] HI_MW_PTR hHandle  : MbufHandle
 * @param[in]  HI_U8   u8PayloadType:      payload type 0~254
 * @return  HI_SUCCESS success
 * @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
 * @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
 */
HI_S32 HI_MBUF_UnregisterPayload(HI_MW_PTR hHandle, HI_U8 u8PayloadType);

/**
* @brief enable read/write ptr for MBUF according to the payload type.
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_U8   u8PayloadType:      payload type 0~254
* @param[in]  HI_BOOL bEnableRead :      enable read ptr or not
* @param[in]  HI_BOOL bEnableWrite:      enable write ptr or not
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
*/

HI_S32 HI_MBUF_SetRWEnable(HI_MW_PTR hHandle, HI_U8 u8PayloadType, HI_BOOL bEnableRead, HI_BOOL bEnableWrite);

/**
* @brief get the  read/write enable prt for MBUF according to the payload type.
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_U8   u8PayloadType:      payload type 0~254
* @param[in]  HI_BOOL *bEnableRead :      enable read ptr or not
* @param[in]  HI_BOOL *bEnableWrite:      enable write ptr or not
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
*/

HI_S32 HI_MBUF_GetRWEnable(HI_MW_PTR hHandle, HI_U8 u8PayloadType, HI_BOOL *pbEnableRead, HI_BOOL *pbEnableWrite);

/**
* @brief write slice into MBUF
* @param[in] HI_MW_PTR hHandle : MbufHandle
* @param[in]   HI_MBUF_SLICEINFO_S *pstSliceInfo     slice info data
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
* @return HI_ERR_MBUF_FULL  full of MBUF
*/

HI_S32 HI_MBUF_WriteSlice(HI_MW_PTR hHandle, HI_MBUF_SLICEINFO_S *pstSliceInfo);

/**
* @brief read data from MBUF
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_U8   u8PayloadType    payload type
* @param[out] HI_MBUF_SLICEINFO_S *pstSliceInfo  read out slice info
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
* @return HI_ERR_MBUF_NODATA  no such data of the giving payload type
*/

HI_S32 HI_MBUF_ReadSlice(HI_MW_PTR hHandle, HI_U8 u8PayloadType, HI_MBUF_SLICEINFO_S *pstSliceInfo);

/**
* @brief  forward the read ptr for the payload type data
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_U8   u8PayloadType:    payload type
* @param[out] HI_U32  u32Step:  moving steps for the read ptr,1 for move one slice after
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
*/

HI_S32 HI_MBUF_Forward(HI_MW_PTR hHandle, HI_U8 u8PayloadType, HI_U32 u32Step);

/**
* @brief  print the debug info og MBUF
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_U8   u8PayloadType :   payload type
*/

HI_VOID HI_MBUF_Debug(HI_MW_PTR hHandle, HI_U8 u8PayloadType);

/**
* @brief  get slice data offset
* @param[in] HI_MW_PTR hHandle  : MbufHandle
* @param[in]  HI_MBUF_SLICEINFO_S* pstSliceInfo:   mbuffer slice info
* @param[out] HI_U32* pu32Offset: slice info offset
* @return  HI_SUCCESS success
* @return HI_ERR_MBUF_ILLEGALPARAM  illegal params
* @return HI_ERR_MBUF_UNINTIALIZE  uninit of MBUF
* @return HI_ERR_MBUF_FULL   MBUF is full
*/

HI_S32 HI_MBUF_GetSliceDataOffset(HI_MW_PTR hHandle, HI_MBUF_SLICEINFO_S *pstSliceInfo, HI_U32 *pu32Offset);

/** @} */ /** <!-- ==== COMMON End ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_MBUF_EXT_H__ */
