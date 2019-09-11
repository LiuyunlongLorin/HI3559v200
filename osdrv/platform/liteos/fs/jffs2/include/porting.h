#ifndef _PORTING_H
#define _PORTING_H

#include "jffs2_config.h"
#include "los_mux.h"
#include "los_event.h"
#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* the following should be same with os_sys_stat.h */
#define JFFS2_S_IFMT	 0x00000f000
#define JFFS2_S_IFDIR	 0040000//(1<<0)
#define JFFS2_S_IFREG	 0100000 //(1<<3)
/*needed by gcthread.c and vfs_jffs2.c*/
#define RT_IPC_FLAG_FIFO                0x00            /**< FIFOed IPC. @ref IPC. */
#define RT_IPC_FLAG_PRIO                0x01            /**< PRIOed IPC. @ref IPC. */

struct jffs2_fs_info
{
	unsigned long   sector_size; /* a erasing block size*/
	unsigned long   nr_blocks; /* number of blocks in flash */
	unsigned long   free_size; /*size of free flash */
};

struct jffs2_stat {
    unsigned long   st_mode;     /* File mode */
    unsigned int    st_ino;      /* File serial number */
    unsigned int    st_dev;      /* ID of device containing file */
    unsigned long  st_nlink;    /* Number of hard links */
    unsigned long  st_uid;      /* User ID of the file owner */
    unsigned long  st_gid;      /* Group ID of the file's group */
    long  st_size;     /* File size (regular files only) */
    long  sddst_atime;    /* Last access time */
    long  sddst_mtime;    /* Last data modification time */
    long  sddst_ctime;    /* Last file status change time */
};

struct jffs2_dirent
{
#ifdef JFFS2PKG_FILEIO_DIRENT_DTYPE

	unsigned long  d_type; // Only supported with FATFS, RAMFS, ROMFS,
	// and JFFS2.
	// d_type is not part of POSIX so
	// should be used with caution.
#endif
	char        d_name[JFFS2_NAME_MAX+1];
};
#define JFFS_ASSERT(EX)                                                         \
if (!(EX))                                                                      \
{                                                                               \
    PRINT_ERR("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__);     \
    return -1;                                                              \
}
//
extern jffs2_s_fileops jffs2_fileops;
extern jffs2_s_fileops jffs2_dirops;
extern struct jffs2_fstab_entry jffs2_fste;

time_t jffs2_get_timestamp(void);
void jffs2_get_info_from_sb(void * data, struct jffs2_fs_info * info);

void jffs_mutex_take(UINT32 *mutex,UINT32 time);
void jffs_mutex_release(UINT32 *mutex);
void jffs_mutex_create(UINT32 *mutex);
void jffs_mutex_delete(UINT32 *mutex);
void jffs_event_create(PEVENT_CB_S pstEventCB);
void jffs_event_send(PEVENT_CB_S pstEventCB ,UINT32 uwEvents );
UINT32 jffs_event_recv(PEVENT_CB_S pstEventCB, UINT32 uwEventMask,UINT32 uwMode,UINT32 uwTimeOut);
void jffs_event_detach(PEVENT_CB_S pstEventCB);
void jffsfs_Lock(void);
void jffsfs_Unlock(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
