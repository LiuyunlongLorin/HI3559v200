/**
 * @file    filetrans_http.h
 * @brief   filetrans http module interface.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/26
 * @version   1.0

 */
#ifndef __FILETRANS_HTTP_H__
#define __FILETRANS_HTTP_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** rename module name */
#ifdef HI_MODULE
#undef HI_MODULE
#endif
#define HI_MODULE "FILETRANS"

#define FILETRANS_HTTP_KEEPALIVE  "Keep-Alive"
#define FILETRANS_HTTP_CLOSE      "Close"
#define FILETRANS_HTTP_BUFFLEN (512)
#define FILETRANS_CONTENT_TYPE_LEN (11)
#define FILETRANS_CONNECTION_LEN (11)

typedef struct tagFILETRANS_REQ_INFO_S
{
    HI_CHAR szFileName[HI_APPCOMM_MAX_PATH_LEN];
    HI_BOOL bKeepAlive;
} FILETRANS_REQ_INFO_S;

typedef struct tagFILETRANS_RESP_INFO_S
{
    HI_S32 s32ReturnCode;
    HI_S32 s32ContentLen;
    HI_CHAR szContentType[FILETRANS_CONTENT_TYPE_LEN];
    HI_CHAR szConnection[FILETRANS_CONNECTION_LEN];
} FILETRANS_RESP_INFO_S;

/**
 * @brief    parse the data of http requset.
 * @param[in] pszReq:http requset packet.
 * @param[out] pstReqInfo:requset information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_ParseHttpReq(const HI_CHAR* pszReq, FILETRANS_REQ_INFO_S* pstReqInfo);

/**
 * @brief    send http response packet.
 * @param[in] s32Socket:http connection socket.
 * @param[in] stRespInfo:response information.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendHttpResp(HI_S32 s32Socket, const FILETRANS_RESP_INFO_S* stRespInfo);

/**
 * @brief    recv http requset data.
 * @param[in] s32Socket:http connection socket.
 * @param[out] pszRecvBuf:recv buff.
 * @param[in] u32BufSize:recv buff size.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_RecvHttpReq(HI_S32 s32Socket, HI_CHAR* pszRecvBuf, HI_U32 u32BufSize);

/**
 * @brief    read file and send by s32Socket.
 * @param[in] s32Socket:http connection socket.
 * @param[in] fpFile:file ptr.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendFile(HI_S32 s32Socket, FILE* fpFile);

/**
 * @brief    send data by s32Socket.
 * @param[in] s32Socket:http connection socket.
 * @param[in] pu8Data:data buff.
 * @param[in] u32DataLen:data length.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/1/29
 */
HI_S32 FILETRANS_SendData(HI_S32 s32Socket, HI_U8* pu8Data, HI_U32 u32DataLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif //__FILETRANS_HTTP_H__
