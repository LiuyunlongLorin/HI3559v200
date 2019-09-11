/**
 * @file      hi_upgrade_define.h
 * @brief     upgrade struct defination
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/7/4
 * @version   1.0

 */

#ifndef __HI_UPGRADE_DEFINE_H__
#define __HI_UPGRADE_DEFINE_H__

#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     UPGRADE */
/** @{ */  /** <!-- [UPGRADE] */

#define HI_UPGRADE_PKT_PREFIX    "upgrade"

/** upgrade partition name maximum length */
#define HI_UPGRADE_PARTITION_NAME_MAX_LEN   (32)

/** upgrade maximum partition count */
#define HI_UPGRADE_MAX_PART_CNT             (10)

/** boot env maximum length */
#define HI_UPGRADE_MAX_ENV_LEN              (512)

/** packet path maximum length */
#define HI_UPGRADE_PKT_PATH_MAX_LEN           (128)

/** upgrade head magic number */
#define HI_UPGRADE_PACKET_HEAD_MAGIC        (0x08122515)
/** upgrade tail magic number */
#define HI_UPGRADE_PACKET_TAIL_MAGIC        (0x0812251d)

/** upgrade partition head information */
typedef struct hiUPGRADE_PARTITION_HEAD_S
{
    HI_CHAR szPartName[HI_UPGRADE_PARTITION_NAME_MAX_LEN]; /**<partition name */
    HI_U32  u32OriDataLen;       /**<image original data length */
    HI_U32  u32DataLen;          /**<image data length in upgrade packet */
} HI_UPGRADE_PARTITION_HEAD_S;

/** upgrade packet head */
typedef struct hiUPGRADE_PKT_HEAD_S
{
    HI_U32  u32Magic;
    HI_U32  u32Crc;         /**<crc number from HeadVersion to end of image-data */
    HI_U32  u32HeadVer;     /**<packet head version: 0x00000001 */
    HI_U32  u32PktLen;      /**<packet total length, including head/data/tail */
    HI_BOOL bCompress;      /**<packet data compress or not */
    HI_CHAR szPktModel[HI_APPCOMM_COMM_STR_LEN]; /**<packet model, eg. hi3556av100_actioncam */
    HI_CHAR szPktSoftVersion[HI_APPCOMM_COMM_STR_LEN];   /**<packet version, eg. 2.0.1.0 */
    HI_CHAR szBootArgs[HI_UPGRADE_MAX_ENV_LEN];
    HI_CHAR szBootCmd[HI_UPGRADE_MAX_ENV_LEN];
    HI_U32  u32ConfigFileOffSet; /**<config file offset in upgrade packet */
    HI_S32  s32PartitionCnt;
    HI_U32  au32PartitionOffSet[HI_UPGRADE_MAX_PART_CNT]; /**<partition offset in upgrade packet */
} HI_UPGRADE_PKT_HEAD_S;

/** upgrade packet tail */
typedef struct hiUPGRADE_PKT_TAIL_S
{
    HI_U32  u32Magic;
} HI_UPGRADE_PKT_TAIL_S;

/** upgrade device information */
typedef struct hiUPGRADE_DEV_INFO_S
{
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN]; /**<software version */
    HI_CHAR szModel[HI_APPCOMM_COMM_STR_LEN];       /**<product model */
} HI_UPGRADE_DEV_INFO_S;

/** upgrade event information */
typedef struct hiUPGRADE_EVENT_INFO_S
{
    HI_CHAR szPktFilePath[HI_UPGRADE_PKT_PATH_MAX_LEN];
    HI_CHAR szSoftVersion[HI_APPCOMM_COMM_STR_LEN];
    HI_U32  u32PktLen;
} HI_UPGRADE_EVENT_INFO_S;

/** upgrade mtd partition information */
typedef struct hiUPGRADE_MTDPART_INFO_S
{
    HI_S32  s32PartitionCnt;
    HI_CHAR aszPartitionName[HI_UPGRADE_MAX_PART_CNT][HI_UPGRADE_PARTITION_NAME_MAX_LEN];
} HI_UPGRADE_MTDPART_INFO_S;


/** @}*/  /** <!-- ==== UPGRADE End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __HI_UPGRADE_DEFINE_H__ */

