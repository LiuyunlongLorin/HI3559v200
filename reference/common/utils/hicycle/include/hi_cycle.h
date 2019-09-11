/**
 * @file    hi_cycle.h
 * @brief   header file for hi_cycle component.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 * @version   1.0

 */
#ifndef __HI_CYCLE_H__
#define __HI_CYCLE_H__
#include "hi_type.h"
#include "hi_flash.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     CYCLE */
/** @{ */  /** <!-- [CYCLE] */

#define CYCLE_MAGIC_HEAD            (0x6379636c)

#define CYCLE_MAGIC_ITEM_START      (0x63796373)

#define CYCLE_MAGIC_ITEM_END        (0x63796365)

/*************************** Structure Definition ****************************/
/** Init Infomation */
typedef struct hiCYCLE_INIT_S
{
    HI_FLASH_TYPE_E enFlashType;/**<[HI_FLASH_TYPE_SPI_0,HI_FLASH_TYPE_NAND_0] */
} HI_CYCLE_INIT_S;

/** Open Infomation */
typedef struct hiCYCLE_OPEN_S
{
    HI_U32 u32CycleFlashSize;   /**<Flash partition size*/
#ifndef __HuaweiLite__
    HI_CHAR* pcPartition;    /**<Flash partition name*/
    HI_CHAR* pcPartitionBackup;/**<Flash partition name*/
#else
    HI_U64 u64Addr;          /**<Flash partition start address*/
    HI_U64 u64BackupAddr;      /**<Flash partition start address*/
#endif
    HI_U32 u32Compress;     /**<[0,1] */
} HI_CYCLE_OPEN_S;

/** Cycle WriteFlag */
typedef enum hiHI_CYCLE_WRITE_FLAG_E
{
    HI_CYCLE_WRITE_FLAG_FAILED = 0,
    HI_CYCLE_WRITE_FLAG_DEFAULT,
    HI_CYCLE_WRITE_FLAG_BURN,
    HI_CYCLE_WRITE_FLAG_BUTT
} HI_CYCLE_WRITE_FLAG_E;

/** Cycle Head Infomation */
typedef struct hiCYCLE_HEAD_S
{
    HI_U32 u32MagicHead;      /**<[CYCLE_MAGIC_HEAD] */
    HI_U32 u32CycleFlashSize; /**<Flash partition size*/
    HI_U32 u32Compress:2;     /**<[0,1] */
    HI_U32 u32WriteFlag:3;    /**<HI_CYCLE_WRITE_FLAG_E */
    HI_U32 u32Reserved:27;
    HI_U32 u32AlignSize;      /**<[16,flash pagesize] */
} HI_CYCLE_HEAD_S;

typedef struct hiCYCLE_ITEM_START_S
{
    HI_U32 u32MagicItemStart;   /**< [CYCLE_MAGIC_ITEM_START] */
    HI_U32 u32ItemLen;      /**only include item data.  if u32Compress is TRUE, the u32ItemLen is compressed len*/
    HI_U32 u32ItemAllLen;   /**include MagicItemStart data/item data/align data*/
    HI_U32 u32ItemOriginLen; /** Origin len of item data*/
} HI_CYCLE_ITEM_START_S;

/**
 * @brief    init hi_cycle. It must be call first.
 * @param[in] pstInit:pointer of init attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Init(HI_CYCLE_INIT_S* pstInit);

/**
 * @brief    deinit hi_cycle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Deinit(HI_VOID);

/**
 * @brief    start hi_cycle. It must be opened first and only once
 * @param[out] pHandle:pointer of hi_cycle handle.
 * @param[in] pstOpen:pointer of HI_CYCLE_OPEN_S attribute.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Open(HI_VOID** pHandle, HI_CYCLE_OPEN_S* pstOpen);

/**
 * @brief    close hi_cycle.
 * @param[in] Handle:hi_cycle handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Close(HI_VOID* Handle);

/**
 * @brief    write hi_cycle.
 * @param[in] Handle:hi_cycle handle.
 * @param[in] pu8Data:pointer of item data.
 * @param[in] u32Len:len of item data.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Write(HI_VOID* Handle, HI_U8* pu8Data, HI_U32 u32Len);

/**
 * @brief    read hi_cycle.
 * @param[in] Handle:hi_cycle handle.
 * @param[in] pu8Data:pointer of item data.
 * @param[in] u32Len:len of item data.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Read(HI_VOID* Handle, HI_U8* pu8Data, HI_U32 u32Len);

/**
 * @brief    clear all data in partition.
 * @param[in] Handle:hi_cycle handle.
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/28
 */
HI_S32 HI_CYCLE_Clear(HI_VOID* Handle);

/** @}*/  /** <!-- ==== CYCLE End ====*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __HI_CYCLE_H__