/**
* Copyright (C), 2016, Hisilicon Tech. Co., Ltd.
* All rights reserved.
*
* @file      hi_datafifo.h
* @brief     Data transform between two processors.
* @author    HiMobileCam middleware develop team
* @date      2016.06.29
*/

#ifndef __HI_DATAFIFO_H
#define __HI_DATAFIFO_H
#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** \addtogroup     DATAFIFO*/
/** @{ */  /** <!-- [DATAFIFO] */

typedef HI_U64 HI_DATAFIFO_HANDLE;
#define HI_DATAFIFO_INVALID_HANDLE (-1)

/** Datafifo Error number base */
#define HI_DATAFIFO_ERRNO_BASE 0x1A00
/** Parameter is invalid */
#define HI_DATAFIFO_ERR_EINVAL_PAEAMETER    (HI_DATAFIFO_ERRNO_BASE + 1)
/** Null pointer*/
#define HI_DATAFIFO_ERR_NULL_PTR            (HI_DATAFIFO_ERRNO_BASE + 2)
/** failure caused by malloc memory */
#define HI_DATAFIFO_ERR_NOMEM               (HI_DATAFIFO_ERRNO_BASE + 3)
/** failure caused by device operation */
#define HI_DATAFIFO_ERR_DEV_OPT             (HI_DATAFIFO_ERRNO_BASE + 4)
/** operation is not permitted, Reader to write or Writer to read */
#define HI_DATAFIFO_ERR_NOT_PERM            (HI_DATAFIFO_ERRNO_BASE + 5)
/** data buffer is empty, no data to read*/
#define HI_DATAFIFO_ERR_NO_DATA             (HI_DATAFIFO_ERRNO_BASE + 6)
/** data buffer is full, no space to write*/
#define HI_DATAFIFO_ERR_NO_SPACE            (HI_DATAFIFO_ERRNO_BASE + 7)
/** read error*/
#define HI_DATAFIFO_ERR_READ                (HI_DATAFIFO_ERRNO_BASE + 8)
/** write error*/
#define HI_DATAFIFO_ERR_WRITE               (HI_DATAFIFO_ERRNO_BASE + 9)


/**
 * @brief Stream release callback. when bDataReleaseByWriter is HI_TRUE,
 * The writer should register this function and do release in this function.
 */
typedef void (*HI_DATAFIFO_RELEASESTREAM_FN_PTR)(void* pStream);

/** Role of caller*/
typedef enum hiDATAFIFO_OPEN_MODE_E
{
    DATAFIFO_READER,
    DATAFIFO_WRITER
} HI_DATAFIFO_OPEN_MODE_E;

/** DATAFIFO parameters */
typedef struct hiDATAFIFO_PARAMS_S
{
    HI_U32 u32EntriesNum; /**< The number of items in the ring buffer*/
    HI_U32 u32CacheLineSize; /**< Item size*/
    HI_BOOL bDataReleaseByWriter; /**<Whether the data buffer release by writer*/
    HI_DATAFIFO_OPEN_MODE_E enOpenMode; /**<READER or WRITER*/
} HI_DATAFIFO_PARAMS_S;

/** DATAFIFO advanced function */
typedef enum hiDATAFIFO_CMD_E
{
    DATAFIFO_CMD_GET_PHY_ADDR, /**<Get the physic address of ring buffer*/
    DATAFIFO_CMD_READ_DONE, /**<When the read buffer read over, the reader should call this function to notify the writer*/
    DATAFIFO_CMD_WRITE_DONE, /**<When the writer buffer is write done, the writer should call this function*/
    DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, /**<When bDataReleaseByWriter is HI_TRUE, the writer should call this to register release callback*/
    DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, /**<Get available write length*/
    DATAFIFO_CMD_GET_AVAIL_READ_LEN /**<Get available read length*/
} HI_DATAFIFO_CMD_E;

/**
 * @brief This function malloc ring buffer and initialize DATAFIFO module.
 * when one side call this, the other side should call ::HI_DATAFIFO_OpenByAddr.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @return HI_SUCCESS Initialize DATAFIFO success.
 * @return HI_FAILURE Initialize DATAFIFO fail.
 */
HI_S32 HI_DATAFIFO_Open(HI_DATAFIFO_HANDLE* Handle, HI_DATAFIFO_PARAMS_S* pstParams);

/**
 * @brief This function map the ring buffer physic address to its virtue address and initialize DATAFIFO module.
 * This function should be called after the other side call ::HI_DATAFIFO_Open because it need physic address of ring buffer.
 * @param[out] handle Handle of DATAFIFO.
 * @param[in] pstParams Parameters of DATAFIFO.
 * @param[in] u32PhyAddr Physic address of ring buffer. Get it from the other side.
 * @return HI_SUCCESS Initialize DATAFIFO success.
 * @return HI_FAILURE Initialize DATAFIFO fail.
 */
HI_S32 HI_DATAFIFO_OpenByAddr(HI_DATAFIFO_HANDLE* Handle, HI_DATAFIFO_PARAMS_S* pstParams, HI_U64 u64PhyAddr);

/**
 * @brief This function will free or unmap ring buffer and deinitialize DATAFIFO.
 * @param[in] handle Handle of DATAFIFO.
 * @return HI_SUCCESS Close success.
 * @return HI_FAILURE Close fail.
 */
HI_S32 HI_DATAFIFO_Close(HI_DATAFIFO_HANDLE Handle);

/**
 * @brief Read data from ring buffer and save it to ppData.
 * every read buffer size is ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[out] ppData Item read.
 * @return HI_SUCCESS Read success.
 * @return HI_FAILURE Read fail.
 */
HI_S32 HI_DATAFIFO_Read(HI_DATAFIFO_HANDLE Handle, HI_VOID** ppData);

/**
 * @brief Write data to ring buffer. data size should be ::u32CacheLineSize
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] pData Item to write.
 * @return HI_SUCCESS Write success.
 * @return HI_FAILURE Write fail.
 */
HI_S32 HI_DATAFIFO_Write(HI_DATAFIFO_HANDLE Handle, HI_VOID* pData);

/**
 * @brief Advanced function. see ::HI_DATAFIFO_CMD_E
 * @param[in] handle Handle of DATAFIFO.
 * @param[in] enCMD Command.
 * @param[in,out] arg Input or output argument.
 * @return HI_SUCCESS Call function success.
 * @return HI_FAILURE Call function fail.
 */
HI_S32 HI_DATAFIFO_CMD(HI_DATAFIFO_HANDLE Handle, HI_DATAFIFO_CMD_E enCMD, HI_VOID* pArg);

/** @}*/  /** <!-- ==== DATAFIFO End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
