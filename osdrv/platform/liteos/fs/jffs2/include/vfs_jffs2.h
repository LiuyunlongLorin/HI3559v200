#ifndef __DFS_JFFS2_H__
#define __DFS_JFFS2_H__

#include "mtd_partition.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef NOR_FLASH_BOOT_SIZE
#define NOR_FLASH_BOOT_SIZE 0x100000
#endif

#define JFFS_WAITING_FOREVER              -1    /**< Block forever until get resource. */
//int vfs_jffs2_init(void);
partition_param *init_jffspar_param(partition_param *jffspar_param);
void deinit_jffspar_param(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
