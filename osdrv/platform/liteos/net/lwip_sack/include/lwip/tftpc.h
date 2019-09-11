/**********************************************************************************
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************/

/*
 *********************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which
 * might include those applicable to Huawei LiteOS of U.S. and the country in which you
 * are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance
 * with such applicable export control laws and regulations.
 *********************************************************************************
 */

#ifndef __LWIP_TFTP_H__
#define __LWIP_TFTP_H__

#include "lwip/opt.h"
#include "lwip/sockets.h"

#if LWIP_TFTP /* don't build if not configured for use in lwipopts.h */

#ifdef __cplusplus
extern "C" {
#endif

#define TFTP_NULL_UINT32        ((u32_t)0xffffffffUL)

#define TFTP_NULL_INT32       -1

/** @defgroup TFTP_Interfaces
* @ingroup Enums
* * This section contains the TFTP enums.
*/

/**
* @defgroup TFTPC_TRANSFER_MODE_E
* @ingroup TFTP_Interfaces
* @par Prototype
* @code
* typedef enum tagTFTPC_TransferMode
* {
*    TRANSFER_MODE_ASCII = 0,
*    TRANSFER_MODE_BINARY,
*    TRANSFER_MODE_BUTT
* }TFTPC_TRANSFER_MODE_E;
* @endcode
* @par Description
* This Enum is used to specify the transfer mode to be handled by TFTP client
* This Enum indicates the TFTP client transfer mode of the file
* @datastruct TRANSFER_MODE_ASCII Mode of transfer ASCII
* @datastruct TRANSFER_MODE_BINARY Mode of transfer Binary
* @datastruct TRANSFER_MODE_BUTT Invalid transfer Mode
*/
typedef enum tagTFTPC_TransferMode
{
    TRANSFER_MODE_ASCII = 0,  /* Mode of transfer Ascii */
    TRANSFER_MODE_BINARY,     /* Mode of transfer Binary */
    TRANSFER_MODE_BUTT     /*Invalid transfer Mode */
}TFTPC_TRANSFER_MODE_E;



/**
* @defgroup TFTPC_ERR_CODE_E
* @ingroup TFTP_Interfaces
* @par Prototype
* @code
* typedef enum tagTFTPC_ErrCode
* {
*    TFTPC_SOCKET_FAILURE              = 1,
*    TFTPC_BIND_FAILURE                = 2,
*    TFTPC_SELECT_ERROR                = 3,
*    TFTPC_RECVFROM_ERROR              = 4,
*    TFTPC_SENDTO_ERROR                = 5,
*    TFTPC_FILE_NOT_FOUND              = 6,
*    TFTPC_CANNOT_RESOLVE_HOSTNAME     = 7,
*    TFTPC_INVALID_PARAVALUE           = 8,
*    TFTPC_PROTO_ERROR                 = 9,
*    TFTPC_SYNC_FAILURE                = 10,
*    TFTPC_FILE_TOO_BIG                = 11,
*    TFTPC_SRC_FILENAME_LENGTH_ERROR   = 12,
*    TFTPC_IP_NOT_WITHIN_RANGE         = 13,
*    TFTPC_ACCESS_ERROR                = 14,
*    TFTPC_DISK_FULL                   = 15,
*    TFTPC_FILE_EXISTS                 = 16,
*    TFTPC_FILE_NOT_EXIST              = 17,
*    TFTPC_MEMALLOC_ERROR              = 18,
*    TFTPC_FILEOPEN_ERROR              = 19,
*    TFTPC_FILEREAD_ERROR              = 20,
*    TFTPC_FILECREATE_ERROR            = 21,
*    TFTPC_FILEWRITE_ERROR             = 22,
*    TFTPC_TIMEOUT_ERROR               = 23,
*    TFTPC_PKT_SIZE_ERROR              = 24,
*    TFTPC_ERROR_NOT_DEFINED           = 25,
*    TFTPC_DEST_PATH_LENGTH_ERROR      = 26,
*    TFTPC_UNKNOWN_TRANSFER_ID         = 27,
*    TFTPC_IOCTLSOCKET_FAILURE         = 28,
*}TFTPC_ERR_CODE_E;
* @endcode
* @par Description
* This Enum is used to specify the transfer mode to be handled by TFTP client
* This Enum indicates the TFTP client transfer mode of the file
* @datastruct  TFTPC_SOCKET_FAILURE             Error while creating UDP socket
* @datastruct  TFTPC_BIND_FAILURE                   Error while binding to the UDP socket
* @datastruct  TFTPC_SELECT_ERROR               Error returned by lwip_select() system call
* @datastruct  TFTPC_RECVFROM_ERROR         Error while receiving data from the peer
* @datastruct  TFTPC_SENDTO_ERROR               Error while sending data to the peer
* @datastruct  TFTPC_FILE_NOT_FOUND             Requested file is not found
* @datastruct  TFTPC_CANNOT_RESOLVE_HOSTNAME        This is the error sent by the server when hostname cannot be resolved
* @datastruct  TFTPC_INVALID_PARAVALUE          Input paramters passed to TFTP interfaces are invalid
* @datastruct  TFTPC_PROTO_ERROR             Error detected in TFTP packet or the error received from the TFTP server
* @datastruct  TFTPC_SYNC_FAILURE               Error during packet synhronization while sending or unexpected packet is received
* @datastruct  TFTPC_FILE_TOO_BIG               File size limit crossed, Max block can be 0xFFFF, each block containing 512 bytes
* @datastruct  TFTPC_SRC_FILENAME_LENGTH_ERROR      File name lenght greater than 256
* @datastruct  TFTPC_IP_NOT_WITHIN_RANGE        Hostname IP is not valid
* @datastruct  TFTPC_ACCESS_ERROR           TFTP server returned file access error
* @datastruct  TFTPC_DISK_FULL                  TFTP server returned error signifying that the DISK is full to write
* @datastruct  TFTPC_FILE_EXISTS                TFTP server returned error signifying that the file exist
* @datastruct  TFTPC_FILE_NOT_EXIST         lwip_tftp_put_file_by_filename returned error signifying that thesource file name do not exisits
* @datastruct  TFTPC_MEMALLOC_ERROR         Memory allocaion failed in TFTP client
* @datastruct  TFTPC_FILEOPEN_ERROR         File open failed
* @datastruct  TFTPC_FILEREAD_ERROR         File read error
* @datastruct  TFTPC_FILECREATE_ERROR       File create error
* @datastruct  TFTPC_FILEWRITE_ERROR        File write error
* @datastruct  TFTPC_TIMEOUT_ERROR          Max time expired while waiting for file to be recived
* @datastruct  TFTPC_PKT_SIZE_ERROR         Error when the received packet is less than 4bytes(error lenght) or greater than 512bytes
* @datastruct  TFTPC_ERROR_NOT_DEFINED          Returned by TFTP server for protocol user error
* @datastruct  TFTPC_DEST_PATH_LENGTH_ERROR         If the destination file path enght greater than 256
* @datastruct  TFTPC_UNKNOWN_TRANSFER_ID            Returned by TFTP server for undefined transfer ID
* @datastruct  TFTPC_IOCTLSOCKET_FAILURE            IOCTL fucntion failed at TFTP client while setting the socket to non-block
*/
typedef enum tagTFTPC_ErrCode
{
    TFTPC_SOCKET_FAILURE              = 1,
    TFTPC_BIND_FAILURE                = 2,
    TFTPC_SELECT_ERROR                = 3,
    TFTPC_RECVFROM_ERROR              = 4,
    TFTPC_SENDTO_ERROR                = 5,
    TFTPC_FILE_NOT_FOUND              = 6,
    TFTPC_CANNOT_RESOLVE_HOSTNAME     = 7,
    TFTPC_INVALID_PARAVALUE           = 8,
    TFTPC_PROTO_ERROR                 = 9,
    TFTPC_SYNC_FAILURE                = 10,
    TFTPC_FILE_TOO_BIG                = 11,
    TFTPC_SRC_FILENAME_LENGTH_ERROR   = 12,
    TFTPC_IP_NOT_WITHIN_RANGE         = 13,
    TFTPC_ACCESS_ERROR                = 14,
    TFTPC_DISK_FULL                   = 15,
    TFTPC_FILE_EXISTS                 = 16,
    TFTPC_FILE_NOT_EXIST              = 17,
    TFTPC_MEMALLOC_ERROR              = 18,
    TFTPC_FILEOPEN_ERROR              = 19,
    TFTPC_FILEREAD_ERROR              = 20,
    TFTPC_FILECREATE_ERROR            = 21,
    TFTPC_FILEWRITE_ERROR             = 22,
    TFTPC_TIMEOUT_ERROR               = 23,
    TFTPC_PKT_SIZE_ERROR              = 24,
    TFTPC_ERROR_NOT_DEFINED           = 25,
    TFTPC_DEST_PATH_LENGTH_ERROR      = 26,
    TFTPC_UNKNOWN_TRANSFER_ID         = 27,
    TFTPC_IOCTLSOCKET_FAILURE         = 28,
    TFTPC_MEMCPY_FAILURE              = 29
}TFTPC_ERR_CODE_E;

typedef enum tagTFTPC_OpCode
{
    TFTPC_OP_RRQ = 1,         /* read request */
    TFTPC_OP_WRQ,             /* write request */
    TFTPC_OP_DATA,            /* data packet */
    TFTPC_OP_ACK,             /* acknowledgement */
    TFTPC_OP_ERROR,           /* error code */
    TFTPC_OP_OPT              /* option code*/
} TFTPC_OPCODE_E;

typedef enum tagTFTPC_PROTOCOL_ErrCode
{
    TFTPC_PROTOCOL_USER_DEFINED = 0,
    TFTPC_PROTOCOL_FILE_NOT_FOUND,
    TFTPC_PROTOCOL_ACCESS_ERROR,
    TFTPC_PROTOCOL_DISK_FULL,
    TFTPC_PROTOCOL_PROTO_ERROR,
    TFTPC_PROTOCOL_UNKNOWN_TRANSFER_ID,
    TFTPC_PROTOCOL_FILE_EXISTS,
    TFTPC_PROTOCOL_CANNOT_RESOLVE_HOSTNAME
}TFTPC_PROT_ERRCODE_E;


#ifndef TFTPC_MAX_SEND_REQ_ATTEMPTS
#define TFTPC_MAX_SEND_REQ_ATTEMPTS         5 /*tftp max attempts*/
#endif

#ifndef TFTPC_TIMEOUT_PERIOD
#define TFTPC_TIMEOUT_PERIOD         5 /*tftp timeout period*/
#endif

#define TFTPC_SERVER_PORT          69 /*tftp server well known port no.*/

/*  MAX file size in TFTP is 32 MB.
    Reason for keeping 75 here , is ((75*512=38400bytes)/1024) =  37MB. So the recv/Send Loop can
    receive the complete MAX message from the network
*/
#define TFTPC_MAX_WAIT_IN_LOOP         75

#define TFTP_BLKSIZE               512     /* data block size (IEN-133)*/
#define TFTP_HDRSIZE               4       /* TFTP header size */
#define TFTP_PKTSIZE              (TFTP_BLKSIZE + TFTP_HDRSIZE) /*Packet size*/
#define TFTP_MAX_MODE_SIZE         9  /* max size of mode string */
#define TFTP_MAXERRSTRSIZE         100 /*max size of error message string*/
#define TFTP_MAX_PATH_LENGTH       256 /* Max path or filename length */
#define TFTP_MAX_BLK_NUM          (0xFFFFL)/*MAximum block number*/

/*IP address not including reserved IPs(0 and 127) and multicast addresses(Class D)*/
#define TFTPC_IP_ADDR_MIN         0x01000000
#define TFTPC_IP_ADDR_EX_RESV     0x7effffff
#define TFTPC_IP_ADDR_CLASS_B     0x80000000
#define TFTPC_IP_ADDR_EX_CLASS_DE 0xdfffffff

#define TFTPC_FOUR 4

/****************************************************************************/
/*                            Structure definitions                         */
/****************************************************************************/
/* Tftp data packet */
typedef struct tagTFTPC_DATA
{
    u16_t  usBlknum;                      /* block number */
    u8_t ucDataBuf[TFTP_BLKSIZE];       /* Actual data */
} TFTPC_DATA_S;


/* TFTP error packet */
typedef struct tagTFTPC_ERROR
{
    u16_t  usErrNum;                       /*error number */
    u8_t   ucErrMesg [TFTP_MAXERRSTRSIZE]; /* error message */
} TFTPC_ERROR_S;


/* TFTP packet format */
typedef struct tagTFTPC_PACKET
{
    u16_t  usOpcode; /* Opcode value */
    union
    {
        /* it contains mode and filename */
        s8_t    ucName_Mode[TFTP_MAX_PATH_LENGTH + TFTP_MAX_MODE_SIZE];
        u16_t    usBlknum; /* Block Number */
        TFTPC_DATA_S    stTFTP_Data; /* Data Packet */
        TFTPC_ERROR_S stTFTP_Err; /* Error Packet */
    } u;
} TFTPC_PACKET_S;




/** @defgroup TFTP_Interfaces
* This section contains the TFTP Interfaces
*/
/*
Func Name:  lwip_tftp_get_file_by_filename
*/
/**
* @defgroup lwip_tftp_get_file_by_filename
* @ingroup TFTP_Interfaces
* @par Prototype
* @code
* u32_t lwip_tftp_get_file_by_filename
* (
*    u32_t  ulHostAddr,
*    u16_t  usTftpServPort,
*    u8_t   ucTftpTransMode,
*    s8_t   *szSrcFileName,
*    s8_t   *szDestDirPath
* );
* @endcode
*
* @par Purpose
*  Gets the specified file from server.
*
* @par Description
* This API gets the source file from the server. It then stores the received file in the destination path
* on the client system.
*
* @param[in]    ulHostAddr          IP address of Host. This is the TFTP server IP. [NA]
* @param[in]    usTftpServPort    TFTP server port. If the value is passed as 0 then the default TFTP
*                                                   PORT 69 is used. [NA]
* @param[in]    ucTftpTransMode File transfer mode, either TRANSFER_MODE_BINARY or TRANSFER_MODE_ASCII. [NA]
* @param[in]    szSrcFileName     Source file in the tftp server. [NA]
* @param[in]    szDestDirPath     Destination file path in the in the client. [NA]
* @param[out]  [N/A]
*
* @par Return values
*  ERR_OK: On success \n
*  TFTPC_ERR_CODE_E: On failure
*
* @par Required Header File
* tftpc.h
*
* @par Note
* \n
* The behavior of this API is such that if the destination file already exists, it will be overwritten.
*
* @par Related Topics
* \n
* N/A
*/
u32_t lwip_tftp_get_file_by_filename
(
    u32_t  ulHostAddr,
    u16_t  usTftpServPort,
    u8_t   ucTftpTransMode,
    s8_t   *szSrcFileName,
    s8_t   *szDestDirPath
);


/** @defgroup TFTP_Interfaces
* This section contains the TFTP Interfaces
*/
/*
Func Name:  lwip_tftp_put_file_by_filename
*/
/**
* @defgroup lwip_tftp_put_file_by_filename
* @ingroup TFTP_Interfaces
* @par Prototype
* @code
* u32_t lwip_tftp_put_file_by_filename
* (
*    u32_t  ulHostAddr,
*    u16_t  usTftpServPort,
*    u8_t   ucTftpTransMode,
*    s8_t   *szSrcFileName,
*    s8_t   *szDestDirPath
* );
* @endcode
*
* @par Purpose
*  Gets the specified file from client and stores the content on the server.
*
* @par Description
* This API reads the contents of the source file on the client system and sends it to the server and
* server then receives the data and stores it in the specified destination path.
*
* @param[in]    ulHostAddr          IP address of Host. This is the TFTP server IP. [NA]
* @param[in]    usTftpServPort    TFTP server port. If the value is passed as 0 then the default TFTP
*                                                   PORT 69 is used. [NA]
* @param[in]    ucTftpTransMode File transfer mode, either TRANSFER_MODE_BINARY or TRANSFER_MODE_ASCII. [NA]
* @param[in]    szSrcFileName     Source file in the client. [NA]
* @param[in]    szDestDirPath     Destination file path on the tftp server. [NA]
*
* @par Return values
*  ERR_OK: On success \n
*  TFTPC_ERR_CODE_E: On failure
*
* @par Required Header File
* tftpc.h
*
* @par Note
* \n
* N/A
*
* @par Related Topics
* \n
* N/A
*/
u32_t lwip_tftp_put_file_by_filename
(
    u32_t  ulHostAddr,
    u16_t  usTftpServPort,
    u8_t   ucTftpTransMode,
    s8_t   *szSrcFileName,
    s8_t   *szDestDirPath
);

#ifdef TFTP_TO_RAWMEM
/** @defgroup TFTP_Interfaces
* This section contains the TFTP Interfaces
*/
/*
Func Name:  lwip_tftp_get_file_by_filename_to_rawmem
*/
/**
* @defgroup lwip_tftp_get_file_by_filename_to_rawmem
* @ingroup TFTP_Interfaces
* @par Prototype
* @code
* u32_t lwip_tftp_get_file_by_filename_to_rawmem
* (
*   u32_t  ulHostAddr,
*   u16_t  usTftpServPort,
*   u8_t   ucTftpTransMode,
*   s8_t   *szSrcFileName,
*   s8_t   *szDestMemAddr,
*   u32_t  *ulFileLength
* );
* @endcode
*
* @par Purpose
*  Gets the specified file from server.
*
* @par Description
* This API gets the source file from the server. It then stores the received file in the target memory
* on the client system.
*
* @param[in]    ulHostAddr          IP address of Host. This is the TFTP server IP. [NA]
* @param[in]    usTftpServPort    TFTP server port. If the value is passed as 0 then the default TFTP
*                                                   PORT 69 is used. [NA]
* @param[in]    ucTftpTransMode File transfer mode, either TRANSFER_MODE_BINARY or TRANSFER_MODE_ASCII. [NA]
* @param[in]    szSrcFileName     Source file in the tftp server. [NA]
* @param[in]    szDestMemAddr     The target memory address in the client. [NA]
* @param[in/out]    ulFileLength      The target memory address can cache the size of the content,and The real size of the Source file.
*
* @par Return values
*  ERR_OK: On success
*  TFTPC_ERR_CODE_E: On failure
*
* @par Required Header File
* tftpc.h
*
* @par Note
* \n
* 1.You must define TFTP_TO_RAWMEM when using this API.
* 2.The behavior of this API is such that if the destination file already exists, it will be overwritten.
*
* @par Related Topics
* \n
* N/A
*/

u32_t lwip_tftp_get_file_by_filename_to_rawmem
(
    u32_t  ulHostAddr,
    u16_t  usTftpServPort,
    u8_t   ucTftpTransMode,
    s8_t   *szSrcFileName,
    s8_t   *szDestMemAddr,
    u32_t  *ulFileLength
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWIP_TFTP */

#endif /* __LWIP_TFTP_H__ */
