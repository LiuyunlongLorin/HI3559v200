#ifndef HI_FSCOMMON_H
#define HI_FSCOMMON_H

#include "hi_fstool.h"

#ifndef MW_VERSION
#define MW_VERSION "0.0.0"
#endif

typedef unsigned char   u_char;
typedef unsigned int    cl_t;
typedef unsigned int    u_int;
typedef unsigned long long u_int64;

#define FSTOOL_COMM_CLUSTER_CHAIN_USED     (1)                /* This fat chain is used in a file */
#define FSTOOL_COMM_CLUSTER_FREE           (0)                /* 0 means cluster is free */
#define FSTOOL_COMM_CLUSTER_FIRST          (2)                /* 2 is the minimum valid cluster number */
#define FSTOOL_COMM_CLUSTER_RSRVD          (0xffffff6)       /* start of reserved clusters */
#define FSTOOL_COMM_CLUSTER_BAD            (0xffffff7)       /* a cluster with a defect */
#define FSTOOL_COMM_CLUSTER_EOFS           (0xffffff8)       /* start of EOF indicators */
#define FSTOOL_COMM_CLUSTER_EOF            (0xfffffff)       /* standard value for last cluster */
#define FSTOOL_COMM_KB_2_BYTE_FACTOR       (1024)
#define FSTOOL_COMM_FILE_SIZE_MAX          (0x100000000)     /*max file size is 4G*/
/**********************inner err code ********************/

#define FAT32_FLAG      1

#define FSCK_FSOK       0x00        /*OK */
#define FSCK_FSBOOTERR  0x01        /* Boot info err */
#define FSCK_FSDIRMOD   0x02        /* Directory modified */
#define FSCK_FSFATMOD   0x04        /* FAT modified */
#define FSCK_FSERR      0x08        /* Error  */
#define FSCK_FSFATAL    0x10        /* Fail*/
#define FSCK_READERR    0x20        /* read fail */
#define FSCK_WRITERR    0x40        /* write fail */
#define FSCK_OOM        0x80        /* the check may cause system OOM */
#define FSCK_FSFRAG     0x100       /* fragment severely */
#define FSCK_FS_NOTSUPPORT   0x200  /*fs not supported*/

extern HI_FSTOOL_LOG_LEVEL_E g_enFSToolLogLevel;
#define FSTOOL_COMM_LOG(level, levelStr, format, args...)\
do {\
       if (level >= g_enFSToolLogLevel)\
       {\
           PRINTK("[FSTOOL][%s] ",  levelStr);\
           PRINTK(format, ##args);\
       }\
   }while(0)

#define FSTOOL_COMM_LOG_DEBUG(format, args...) FSTOOL_COMM_LOG(HI_FSTOOL_LOG_LEVEL_DEBUG, "DEBUG",format, ##args)
#define FSTOOL_COMM_LOG_INFO(format, args...) FSTOOL_COMM_LOG(HI_FSTOOL_LOG_LEVEL_INFO, "INFO",format, ##args)
#define FSTOOL_COMM_LOG_WARN(format, args...) FSTOOL_COMM_LOG(HI_FSTOOL_LOG_LEVEL_WARN, "WARN",format, ##args)
#define FSTOOL_COMM_LOG_ERROR(format, args...) FSTOOL_COMM_LOG(HI_FSTOOL_LOG_LEVEL_ERROR, "ERROR",format, ##args)
#define FSTOOL_COMM_LOG_FATAL(format, args...) FSTOOL_COMM_LOG(HI_FSTOOL_LOG_LEVEL_FATAL, "FATAL",format, ##args)

extern struct FSCK_CACHE g_stAllocatedCluster;
extern struct FSCK_FRAGMENT g_stFreeCluster,g_stBadCluster;
#endif
