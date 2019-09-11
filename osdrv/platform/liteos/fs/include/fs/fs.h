/****************************************************************************
 * include/fs/fs.h
 *
 *   Copyright (C) 2007-2009, 2011-2013 Gregory Nutt. All rights reserved.
 *   Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
 *   All rights reserved.
 *
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/****************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country in
 * which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ****************************************************************************/

/** @defgroup fs FS
 *  @ingroup filesystem
 */


#ifndef __INCLUDE_FS_FS_H
#define __INCLUDE_FS_FS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "vfs_config.h"
#include "compiler.h"

#include "sys/types.h"
#include "sys/stat.h"

#include "stdarg.h"
#include "stdint.h"

#include "sys/vfs.h"

#include "semaphore.h"
#ifdef LOSCFG_FS_FAT_CACHE
#include "bcache.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* POSIX-like OS return values: */

#ifndef  VFS_ERROR
#define VFS_ERROR -1
#endif

#undef  OK
#define OK 0

#ifndef CONFIG_DISABLE_MQUEUE
#  include "mqueue.h"
#endif

#define MS_RDONLY 1
#define PROCFS_MOUNT_POINT  "/proc"
#define PROCFS_MOUNT_POINT_SIZE (sizeof(PROCFS_MOUNT_POINT) - 1)

#define RAMFS_MOUNT_POINT   "/ramfs"
#define RAMFS_MOUNT_POINT_SIZE  (sizeof(RAMFS_MOUNT_POINT) - 1)

#define YAFFS_MOUNT_POINT       "/yaffs"
#define YAFFS_MOUNT_POINT_SIZE  (sizeof(YAFFS_MOUNT_POINT) - 1)

/* Format options (3rd argument of format) */
#define FMT_FAT      0x01
#define FMT_FAT32    0x02
#define FMT_EXFAT    0x04
#define FMT_ANY      0x07

/* system time flag for FAT/exFAT */
#define FAT_SYSTEM_TIME_ENABLE   0x01
#define FAT_SYSTEM_TIME_DISABLE  0x00

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Stream flags for the fs_flags field of in struct file_struct */

#define __FS_FLAG_EOF   (1 << 0) /* EOF detected by a read operation */
#define __FS_FLAG_ERROR (1 << 1) /* Error detected by any operation */
#define FALLOC_FL_KEEP_SIZE 0x01 /* extend size */

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* This structure is provided by devices when they are registered with the
 * system.  It is used to call back to perform device specific operations.
 */

struct file;   /* Forward reference */
struct pollfd; /* Forward reference */
struct inode;  /* Forward reference */
struct tag_poll_table;
typedef struct tag_poll_table poll_table;

struct file_operations_vfs
{
  /* The device driver open method differs from the mountpoint open method */

  int     (*open)(FAR struct file *filep);

  /* The following methods must be identical in signature and position because
   * the struct file_operations and struct mountp_operations are treated like
   * unions.
   */

  int     (*close)(FAR struct file *filep);
  ssize_t (*read)(FAR struct file *filep, FAR char *buffer, size_t buflen);
  ssize_t (*write)(FAR struct file *filep, FAR const char *buffer, size_t buflen);
  off_t   (*seek)(FAR struct file *filep, off_t offset, int whence);
  int     (*ioctl)(FAR struct file *filep, int cmd, unsigned long arg);

  /* The two structures need not be common after this point */

#ifndef CONFIG_DISABLE_POLL
  int     (*poll)(FAR struct file *filep, poll_table *fds);
#endif
  int     (*unlink)(FAR struct inode *inode);
};

/* This structure provides information about the state of a block driver */

#ifndef CONFIG_DISABLE_MOUNTPOINT
struct geometry
{
  bool   geo_available;    /* true: The device is available */
  bool   geo_mediachanged; /* true: The media has changed since last query */
  bool   geo_writeenabled; /* true: It is okay to write to this device */
  unsigned long long geo_nsectors;     /* Number of sectors on the device */
  size_t geo_sectorsize;   /* Size of one sector */
};

/* This structure is provided by block devices when they register with the
 * system.  It is used by file systems to perform filesystem transfers.  It
 * differs from the normal driver vtable in several ways -- most notably in
 * that it deals in struct inode vs. struct filep.
 */

struct inode;
struct block_operations
{
  int     (*open)(FAR struct inode *inode);
  int     (*close)(FAR struct inode *inode);
  ssize_t (*read)(FAR struct inode *inode, FAR unsigned char *buffer,
            unsigned long long start_sector, unsigned int nsectors);
  ssize_t (*write)(FAR struct inode *inode, FAR const unsigned char *buffer,
            unsigned long long start_sector, unsigned int nsectors);
  int     (*geometry)(FAR struct inode *inode, FAR struct geometry *geometry);
  int     (*ioctl)(FAR struct inode *inode, int cmd, unsigned long arg);
  int     (*unlink)(FAR struct inode *inode);
};

/* This structure is provided by a filesystem to describe a mount point.
 * Note that this structure differs from file_operations ONLY in the form of
 * the open method.  Once the file is opened, it can be accessed either as a
 * struct file_operations or struct mountpt_operations
 */

struct inode;
struct fs_dirent_s;
struct stat;
struct statfs;
struct mountpt_operations
{
  /* The mountpoint open method differs from the driver open method
   * because it receives (1) the inode that contains the mountpoint
   * private data, (2) the relative path into the mountpoint, and (3)
   * information to manage privileges.
   */

  int     (*open)(FAR struct file *filep, FAR const char *relpath,
            int oflags, mode_t mode);

  /* The following methods must be identical in signature and position
   * because the struct file_operations and struct mountp_operations are
   * treated like unions.
   */

  int     (*close)(FAR struct file *filep);
  ssize_t (*read)(FAR struct file *filep, FAR char *buffer, size_t buflen);
  ssize_t (*write)(FAR struct file *filep, FAR const char *buffer, size_t buflen);
  off_t   (*seek)(FAR struct file *filep, off_t offset, int whence);
  int     (*ioctl)(FAR struct file *filep, int cmd, unsigned long arg);

  /* The two structures need not be common after this point. The following
   * are extended methods needed to deal with the unique needs of mounted
   * file systems.
   *
   * Additional open-file-specific mountpoint operations:
   */

  int     (*sync)(FAR struct file *filep);
  int     (*dup)(FAR const struct file *oldp, FAR struct file *newp);

  /* Directory operations */

  int     (*opendir)(FAR struct inode *mountpt, FAR const char *relpath,
            FAR struct fs_dirent_s *dir);
  int     (*closedir)(FAR struct inode *mountpt,
            FAR struct fs_dirent_s *dir);
  int     (*readdir)(FAR struct inode *mountpt,
            FAR struct fs_dirent_s *dir);
  int     (*rewinddir)(FAR struct inode *mountpt,
            FAR struct fs_dirent_s *dir);

  /* General volume-related mountpoint operations: */

  int     (*bind)(FAR struct inode *blkdriver, FAR const void *data,
            FAR void **handle, FAR const char *realpath);
  int     (*unbind)(FAR void *handle, FAR struct inode **blkdriver);
  int     (*statfs)(FAR struct inode *mountpt, FAR struct statfs *buf);
#ifdef LOSCFG_FS_FAT_VIRTUAL_PARTITION
  int     (*virstatfs)(struct inode *mountpt, const char* relpath, struct statfs *buf);
#endif
  /* Operations on paths */

  int     (*unlink)(FAR struct inode *mountpt, FAR const char *relpath);
  int     (*mkdir)(FAR struct inode *mountpt, FAR const char *relpath,
            mode_t mode);
  int     (*rmdir)(FAR struct inode *mountpt, FAR const char *relpath);
  int     (*rename)(FAR struct inode *mountpt, FAR const char *oldrelpath,
            FAR const char *newrelpath);
  int     (*stat)(FAR struct inode *mountpt, FAR const char *relpath,
            FAR struct stat *buf);
  int     (*stat64)(FAR struct inode *mountpt, FAR const char *relpath,
            FAR struct stat64 *buf);
  int     (*utime)(FAR struct inode *mountpt, FAR const char *relpath,
            FAR const struct tm *times);
  int     (*chattr)(FAR struct inode *mountpt, FAR const char *relpath,
            mode_t mode);
  loff_t   (*seek64)(FAR struct file *filep, loff_t offset, int whence);
  int     (*getlabel)(FAR void *handle,FAR char* label);
  int     (*fallocate)(FAR struct file *filep, int mode, off_t offset, off_t len);
  int     (*fallocate64)(FAR struct file *filep, int mode, off64_t offset, off64_t len);
  int     (*truncate)(FAR struct file *filep, off_t length);
  int     (*truncate64)(FAR struct file *filep, off64_t length);
  int     (*fscheck)(FAR struct inode *mountpt, FAR const char *relpath,
            FAR struct fs_dirent_s *dir);

  /* NOTE:  More operations will be needed here to support:  disk usage
   * stats file stat(), file attributes, file truncation, etc.
   */
};
#endif /* CONFIG_DISABLE_MOUNTPOINT */

#ifdef LOSCFG_FS_FAT_VIRTUAL_PARTITION
#define _MAX_ENTRYLENGTH  16                                       /*MAX virtual partition name length*/
#define _MAX_VIRVOLUMES   5                                        /*MAX virtual partition number*/
typedef struct virtual_partition_info
{
    char *devpartpath;                                             /*partition name need set virtual partition,e.g. /dev/mmcblk0p0*/
    int  virpartnum;                                               /*virtual partition numbers, MAX number is 5*/
    double virpartpercent[_MAX_VIRVOLUMES];                        /*every virtual partition percent,e.g 0.6,0.3,0.1*/
    char virpartname[_MAX_VIRVOLUMES][_MAX_ENTRYLENGTH + 1];       /*every virtual partition name, MAX length is 16*/
}virpartinfo;
#endif

struct fsmap_t
{
    const char                      *fs_filesystemtype;
    const struct mountpt_operations *fs_mops;
    const BOOL                      is_mtd_support;
    const BOOL                      is_bdfs;
};

#define FSMAP_WOW_ENTRY(_l, _name, _mop, _is_mtd_support, _is_bdfs)                      \
    struct fsmap_t _l LOS_HAL_TABLE_WOW_ENTRY(fsmap) =  \
{                                                       \
    _name,                                              \
    &_mop,                                               \
    _is_mtd_support,                                    \
    _is_bdfs                                                \
};

#define FSMAP_SCATTER_ENTRY(_l, _name, _mop, _is_mtd_support, _is_bdfs)                      \
    struct fsmap_t _l LOS_HAL_TABLE_SCATTER_ENTRY(fsmap) =  \
{                                                       \
    _name,                                              \
    &_mop,                                               \
    _is_mtd_support,                                    \
    _is_bdfs                                                \
};

#define FSMAP_ENTRY(_l, _name, _mop, _is_mtd_support, _is_bdfs)                      \
    struct fsmap_t _l LOS_HAL_TABLE_ENTRY(fsmap) =  \
{                                                       \
    _name,                                              \
    &_mop,                                               \
    _is_mtd_support,                                    \
    _is_bdfs                                                \
};

/* Named OS resources are also maintained by the VFS.  This includes:
 *
 *   - Named semaphores:     sem_open(), sem_close(), and sem_unlink()
 *   - POSIX Message Queues: mq_open() and mq_close()
 *   - Shared memory:        shm_open() and shm_unlink();
 *
 * These are a special case in that they do not follow quite the same
 * pattern as the other file system types in that they have operations.
 */

/* These are the various kinds of operations that can be associated with
 * an inode.
 */

union inode_ops_u
{
  FAR const struct file_operations_vfs      *i_ops;    /* Driver operations for inode */
#ifndef CONFIG_DISABLE_MOUNTPOINT
  FAR const struct block_operations     *i_bops;   /* Block driver operations */
  FAR const struct mountpt_operations   *i_mops;   /* Operations on a mountpoint */
#endif
#ifdef CONFIG_FS_NAMED_SEMAPHORES
  FAR struct nsem_inode_s               *i_nsem;   /* Named semaphore */
#endif
#ifndef CONFIG_DISABLE_MQUEUE
  FAR struct mqueue_inode_s             *i_mqueue; /* POSIX message queue */
#endif
};

/* This structure represents one inode in the Nuttx pseudo-file system */

typedef enum mount_status
{
    STAT_UNMOUNTED = 0,
    STAT_MOUNTED,
}MOUNT_STATE;


struct inode
{
  FAR struct inode *i_peer;     /* Link to same level inode */
  FAR struct inode *i_child;    /* Link to lower level inode */
  int16_t           i_crefs;    /* References to inode */
  uint16_t          i_flags;    /* Flags for inode */
  unsigned long     mountflags; /*Flags for mount*/
  union inode_ops_u u;          /* Inode operations */
#ifdef CONFIG_FILE_MODE
  mode_t            i_mode;     /* Access mode flags */
#endif
  FAR void         *i_private;  /* Per inode driver private data */
  MOUNT_STATE       e_status;
  char              i_name[1];  /* Name of inode (variable) */
};
#define FSNODE_SIZE(n) (sizeof(struct inode) + (n))

/* This is the underlying representation of an open file.  A file
 * descriptor is an index into an array of such types. The type associates
 * the file descriptor to the file state and to a set of inode operations.
 */

struct file
{
  int               f_oflags;   /* Open mode flags */
  FAR struct inode *f_inode;    /* Driver interface */
  loff_t            f_pos;      /* File position */
  char             *f_path;     /* File fullpath */
  void             *f_priv;     /* Per file driver private data */
  const char       *f_relpath;  /* realpath */
};

/* This defines a list of files indexed by the file descriptor */

#if CONFIG_NFILE_DESCRIPTORS > 0
struct filelist
{
  sem_t   fl_sem;               /* Manage access to the file list */
  struct file fl_files[CONFIG_NFILE_DESCRIPTORS];
};

extern struct filelist tg_filelist;
#endif

/* The following structure defines the list of files used for standard C I/O.
 * Note that NuttX can support the standard C APIs without or without buffering
 *
 * When buffering us used, the following described the usage of the I/O buffer.
 * The buffer can be used for reading or writing -- but not both at the same time.
 * An fflush is implied between each change in directionof access.
 *
 * The field fs_bufread determines whether the buffer is being used for reading or
 * for writing as fillows:
 *
 *              BUFFER
 *     +----------------------+ <- fs_bufstart Points to the beginning of the buffer.
 *     | WR: Buffered data    |                WR: Start of buffered write data.
 *     | RD: Already read     |                RD: Start of already read data.
 *     +----------------------+
 *     | WR: Available buffer | <- fs_bufpos   Points to next byte:
 *     | RD: Read-ahead data  |                WR: End+1 of buffered write data.
 *     |                      |                RD: Points to next char to return
 *     +----------------------+
 *     | WR: Available        | <- fs_bufread  Top+1 of buffered read data
 *     | RD: Available        |                WR: =bufstart buffer used for writing.
 *     |                      |                RD: Pointer to last buffered read char+1
 *     +----------------------+
 *                              <- fs_bufend   Points to end end of the buffer+1
 */

#if CONFIG_NFILE_STREAMS > 0
struct file_struct
{
  int                fs_fd;        /* File descriptor associated with stream */
#if CONFIG_STDIO_BUFFER_SIZE > 0
  sem_t              fs_sem;       /* For thread safety */
  pid_t              fs_holder;    /* Holder of sem */
  int                fs_counts;    /* Number of times sem is held */
  FAR unsigned char *fs_bufstart;  /* Pointer to start of buffer */
  FAR unsigned char *fs_bufend;    /* Pointer to 1 past end of buffer */
  FAR unsigned char *fs_bufpos;    /* Current position in buffer */
  FAR unsigned char *fs_bufread;   /* Pointer to 1 past last buffered read char. */
#endif
  uint16_t           fs_oflags;    /* Open mode flags */
  uint8_t            fs_flags;     /* Stream flags */
#if CONFIG_NUNGET_CHARS > 0
  uint8_t            fs_nungotten; /* The number of characters buffered for ungetc */
  unsigned char      fs_ungotten[CONFIG_NUNGET_CHARS];
#endif
};

struct streamlist
{
  sem_t               sl_sem;   /* For thread safety */
  struct file_struct sl_streams[CONFIG_NFILE_STREAMS];
};

extern struct streamlist tg_streamlist;
#endif /* CONFIG_NFILE_STREAMS */

/* Callback used by foreach_mountpoints to traverse all mountpoints in the
 * pseudo-file system.
 */

#ifndef CONFIG_DISABLE_MOUNTPOINT
struct statfs;                    /* Forward reference */
typedef int (*foreach_mountpoint_t)(FAR const char *mountpoint,
                                    FAR struct statfs *statbuf,
                                    FAR void *arg);
#endif

/**
 * @ingroup  fs
 * @brief find mount point
 *
 * @par Description:
 * This API is used to visit each mountpoint in the pseudo-file system.  The traversal is
 *   terminated when the callback 'handler' returns a non-zero value, or when
 *   all of the mountpoints have been visited.
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  handler [IN] Type #foreach_mountpoint_t operation function when find a mount point.
 * @param  arg [IN] Type  #FAR void * Private data.
 *
 * @retval #OK           All of the mountpoints have been visited.
 * @retval #error        Visited failed.
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 * @since Huawei LiteOS V100R001C00
 */

#ifndef CONFIG_DISABLE_MOUNTPOINT
int foreach_mountpoint(foreach_mountpoint_t handler, FAR void *arg);
#endif

/**
 * @ingroup  fs
 * @brief Register a character driver
 *
 * @par Description:
 * This API is used to register a character driver inode the pseudo file system.
 *
 * @attention
 * <li>This function should be called after los_vfs_init has been called.</li>
 * <li>The parameter path must point a valid string, which end with the terminating null byte.</li>
 * <li>The total length of parameter path must less than the value defined by PATH_MAX.</li>
 * <li>The prefix of the parameter path must be /dev/.</li>
 * <li>The parameter fops must pointed the right functions, otherwise the system will crash when the device is being operated.</li>
 * </ul>
 *
 * @param  path [IN] Type #const char * The path that the inode to be created.
 * @param  fops [IN] Type  #const struct file_operations_vfs * The file operations structure.
 * @param  mode [IN] Type  #mode_t  inmode priviledges (not used).
 * @param  priv [IN] Type  #void * Private, user data that will be associated with the inode.
 *
 * @retval #OK                                             Register success.
 * @retval #EINVAL                                         'path' is invalid for this operation.
 * @retval #EEXIST                                         An inode already exists at 'path'.
 * @retval #ENOMEM                                         Failed to allocate in-memory resources for the operation.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see unregister_driver
 * @since Huawei LiteOS V100R001C00
 */

int register_driver(FAR const char *path, FAR const struct file_operations_vfs *fops,
                    mode_t mode, FAR void *priv);

/**
 * @ingroup  fs
 * @brief Register a block driver
 *
 * @par Description:
 * This API is used to register a block driver inode the pseudo file system.
 * @attention
 * <ul>
 * <li>This function should be called after los_vfs_init has been called.</li>
 * <li>The parameter path must point a valid string, which end with the terminating null byte.</li>
 * <li>The length of parameter path must be less than the value defined by PATH_MAX.</li>
 * <li>The prefix of the parameter path must be '/dev/'.</li>
 * <li>The parameter fops must pointed the right functions, otherwise the system will crash when the device is being operated.</li>
 * </ul>
 *
 * @param  path [IN] Type  #const char * The path that the inode to be created.
 * @param  bops [IN] Type  #const struct block_operations * The block driver operations structure.
 * @param  mode [IN] Type  #mode_t  inmode priviledges (not used).
 * @param  priv [IN] Type  #void * Private, ser data that will be associated with the inode.
 *
 * @retval #OK                                             Register success.
 * @retval #EINVAL                                         'path' is invalid for this operation.
 * @retval #EEXIST                                         An inode already exists at 'path'.
 * @retval #ENOMEM                                         Failed to allocate in-memory resources for the operation.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see unregister_blockdriver
 * @since Huawei LiteOS V100R001C00
 */
#ifndef CONFIG_DISABLE_MOUNTPOINT
int register_blockdriver(FAR const char *path,
                         FAR const struct block_operations *bops, mode_t mode,
                         FAR void *priv);
#endif

 /**
 * @ingroup  fs
 * @brief Initializes the vfs filesystem
 *
 * @par Description:
 * This API is used to initializes the vfs filesystem
 *
 * @attention
 * <ul>
 * <li>Called only once, multiple calls will cause file system error.</li>
 * </ul>
 *
 * @param none
 *
 * @retval none
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see NULL
 * @since Huawei LiteOS V100R001C00
 */

void los_vfs_init(void);

/**
 * @ingroup  fs
 * @brief Unregister a character driver
 *
 * @par Description:
 * This API is used to remove the character driver inode at 'path' from the pseudo-file system
 *
 * @attention
 * <ul>
 * <li>This function should be called after register_driver has been called.</li>
 * <li>The parameter path must point a valid string, which end with the terminating null byte.</li>
 * <li>The total length of parameter path must be less than the value defined by PATH_MAX.</li>
 * <li>The device node referred by parameter path must be really exist.</li>
 * </ul>
 *
 * @param  path [IN] Type #const char * The path that the inode to be destroyed.
 *
 * @retval #OK                                                Register success.
 * @retval #EBUSY                                             Resource is busy ,not permit for this operation.
 * @retval #ENOENT                                            'path' is invalid for this operation.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see register_driver
 * @since Huawei LiteOS V100R001C00
 */
int unregister_driver(const char *path);


/**
 * @ingroup  fs
 * @brief Unregister a block driver
 *
 * @par Description:
 * This API is used to remove the block driver inode at 'path' from the pseudo-file system
 *
 * @attention
 * <ul>
 * <li>This function should be called after register_blockdriver has been called.</li>
 * <li>The parameter path must point a valid string, which end with the terminating null byte.</li>
 * <li>The total length of parameter path must less than the value defined by PATH_MAX.</li>
 * <li>The block device node referred by parameter path must be really exist.</li>
 * </ul>
 *
 * @param  path [IN] Type #const char * The path that the inode to be destroyed.
 *
 * @retval #OK                                                Register success.
 * @retval #EBUSY                                             Resource is busy ,not permit for this operation.
 * @retval #ENOENT                                            'path' is invalid for this operation.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see register_blockdriver
 * @since Huawei LiteOS V100R001C00
 */
int unregister_blockdriver(const char *path);

/**
 * @ingroup  fs
 * @brief open a block driver
 *
 * @par Description:
 * This API is used to return the inode of the block driver specified by 'pathname'
 *
 * @attention
 * <ul>
 * <li>The parameter path must point a valid string, which end with the terminating null byte.</li>
 * <li>The total length of parameter path must less than the value defined by PATH_MAX.</li>
 * <li>The parameter ppinode must point a valid memory, which size must be enough for storing struct inode.</li>
 * </ul>
 *
 * @param  pathname [IN] Type #const char * The path to the inode to open.
 * @param  mountflags [IN] Type  #int if MS_RDONLY is not set, then driver must support write operations.
 * @param  ppinode [IN] Type  #struct inode **  address of the location to return the inode reference.
 *
 * @retval #OK                                               Open successfully.
 * @retval #EINVAL                                           pathname or ppinode is NULL.
 * @retval #ENOTBLK                                          The inode associated with the pathname is not a block driver.
 * @retval #EACCESS                                          The MS_RDONLY option was not set but this driver does not support write access.
 * @retval #ENOENT                                           No block driver of this name is registered.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see close_blockdriver
 * @since Huawei LiteOS V100R001C00
 */
#if CONFIG_NFILE_DESCRIPTORS > 0
int open_blockdriver(FAR const char *pathname, int mountflags,
                     FAR struct inode **ppinode);
#endif

/**
 * @ingroup  fs
 * @brief close a block driver
 *
 * @par Description:
 * This API is used to call the close method and release the inode
 *
 * @attention
 * <ul>
 * <li>This function should be called after open_blockdriver has been called.</li>
 * </ul>
 *
 * @param  inode [IN] Type #struct inode * reference to the inode of a block driver returned by open_blockdriver.
 *
 * @retval #OK                                               Open successfully.
 * @retval #EINVAL                                           Inode is NULL.
 * @retval #ENOTBLK                                          The inode is not a block driver.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see open_blockdriver
 * @since Huawei LiteOS V100R001C00
 */
#if CONFIG_NFILE_DESCRIPTORS > 0
int close_blockdriver(FAR struct inode *inode);
#endif

/**
 * @ingroup  fs
 * @brief find a block driver
 *
 * @par Description:
 * This API is used to return the inode of the block driver specified by 'pathname'
 *
 * @attention
 * <ul>
 * <li>The parameter pathname is a full path, which begin with '/'.</li>
 * <li>The parameter ppinode must point a valid memory, which size must be enough for storing struct inode.</li>
 * </ul>
 *
 * @param  pathname [IN] Type #const char * the full path to the block driver to be located
 * @param  mountflags [IN] Type  #int if MS_RDONLY is not set, then driver must support write operations
 * @param  ppinode [IN] Type  #struct inode **  address of the location to return the inode reference
 *
 * @retval #OK                                         Open successfully.
 * @retval #EINVAL                                     Pathname or pinode is NULL.
 * @retval #ENOTBLK                                    The inode associated with the pathname is not a block driver.
 * @retval #EACCESS                                    The MS_RDONLY option was not set but this driver does not support write access.
 * @retval #ENOENT                                     No block driver with this name is registered.
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see NULL
 * @since Huawei LiteOS V100R001C00
 */
#if CONFIG_NFILE_DESCRIPTORS > 0
int find_blockdriver(FAR const char *pathname, int mountflags,
                     FAR struct inode **ppinode);
#endif

/**
 * @ingroup  fs
 * @brief   list directory contents.
 *
 * @par Description:
 * List information about the FILEs (the current directory by default).
 *
 * @attention
 * <ul>
 * <li>The total length of parameter pathname must be less than the value defined by PATH_MAX.</li>
 * </ul>
 *
 * @param pathname [IN]  Type #const char*  The file pathname.
 *
 * @retval
 * <ul>None.</ul>
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see ls
 * @since Huawei LiteOS V100R001C00
 */
 extern void ls(const char *pathname);

/**
* @ingroup  fs
* @brief    locate character in string.
*
* @par Description:
* The API function returns a pointer to the last occurrence of the character c in the string s.
*
* @attention
* <ul>
* <li>The parameter s must point a valid string, which end with the terminating null byte.</li>
* </ul>
*
* @param s [IN] Type #const char*  A pointer to string.
* @param c [IN]  Type #int      The character.
*
* @retval #char*        a pointer to the matched character or NULL if the character is not found.
*
* @par Dependency:
* <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
* @see rindex
* @since Huawei LiteOS V100R001C00
*/
extern char *rindex( const char *s, int c );

/**
 * @ingroup  fs
 * @brief   list directory contents.
 *
 * @par Description:
 * Get the volume label of the FAT partition.
 *
 * @attention
 * <ul>
 * <li>The function support FAT filesystem only.</li>
 * <li>The label must allocated more than 11 charactors space first</li>
 * </ul>
 *
 * @param pathname [IN]  Type #const char*  The file pathname.
 * @param label    [OUT]  Type #const char*  The string pointer transform the label massge back.
 *
 * @retval #int  Point the status which is successed or failed.
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 */

int getlabel(const char *target,char* label);

/**
 * @ingroup  fs
 * @brief formatting sd card
 *
 * @par Description:
 * formatting sd card.
 *
 * @attention
 * <ul>
 * <li>The prefix of the parameter dev must be "/dev", and the length must be less than the value defined by PATH_MAX.
 *     There art four kind of format option: FMT_FAT16, FMT_FAT32, FMT_EXFAT, FMT_ANY. If users imput anything else,
 *     the default format option is FMT_ANY. Format option is decided by the number of clusters. Choosing the wrong
 *     option will cause error of format. The detailed information of (FAT16,FAT32,EXFAT) is ff.h.
 * </li>
 * </ul>
 *
 * @param  dev          [IN] Type #const char*  path of the block device to format, which must be a really existing block device node.
 * @param  sectors      [IN] Type #int number of sectors per cluster.
 * @param  option       [IN] Type #int option of format.
 *
 * @retval #0      Format success.
 * @retval #-1     Format failed.
 *
 * @par Dependency:
 * <ul><li>unistd.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V100R001C00
 */
extern int format(const char *dev, int sectors, int option);

/**
 * @ingroup  fs
 * @brief set current system time is valid or invalid for FAT/exFAT file system.
 *
 * @par Description:
 * The function is used for setting current system time is valid or invalid for FAT/exFAT file system.
 * The value can be set as FAT_SYSTEM_TIME_ENABLE/FAT_SYSTEM_TIME_DISABLE.
 *
 * @attention
 * <ul>
 * <li>When the system time is valid, it should set FAT_SYSTEM_TIME_ENABLE.</li>
 * <li>When the system time is invalid, it should set FAT_SYSTEM_TIME_DISABLE.</li>
 * </ul>
 *
 * @param  status             [IN] Type #BOOL    system time status.
 *
 * @retval #0      set status success
 * @retval #-22   Invalid argument
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V200R001C00
 */
extern int los_set_systime_status(BOOL b_status);

/**
 * @ingroup  fs
 * @check the three latest files in path
 *
 * @par Description:
 * The fscheck() function check the latest three files in path and subdirectories.
 *The function will fix the FAT when the file size info of directory is not matched with FAT.
 *
 * @attention
 * <ul>
 * <li>This function only support for FAT32.</li>
 * </ul>
 *
 * @param  path     [IN] Type #const char *   The path of the directory to be checked.
 *
 * @retval #0      truncate success.
 * @retval #-1    truncate failed.
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V200R001C00
 */
FAR int fscheck(FAR const char *path);

#ifdef LOSCFG_FS_FAT_VIRTUAL_PARTITION
/**
 * @ingroup  fs
 * @get the virtual partitions' or free space information in virtual parition feature.
 *
 * @par Description:
 * The virstatfs() function returns the information about the a virtual partition or the free space
 * outside the virtual partition.
 *
 * @attention
 * <ul>
 * <li>This function only support for FAT32.</li>
 * <li>This function only support for the virtual partition feature.</li>
 * <li>The parameter 'buf' need to be allocate enough memeory space outside the function first.</li>
 * </ul>
 *
 * @param  path     [IN] Type #const char *   The path which virtual partition or free space to be checked.
 * @param  buf      [OUT] Type #struct statfs *   The statfs buffer saving the information.
 *
 * @retval #0      virstatfs success.
 * @retval #-1     virstatfs failed.
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V200R002C00
 */
extern int virstatfs(FAR const char *path, FAR struct statfs *buf);
/**
 * @ingroup  fs
 * @set the virtual partition information.
 *
 * @par Description:
 * The los_set_virpartparam() function use for set virtual partition parameter.
 * The parameter include virtual partition number, virtual partition percent, virtual partition name
 * and the partition path which need mount virtual partition.
 *
 * @attention
 * <ul>
 * <li>This function only support for FAT32.</li>
 * <li>This function only support for the virtual partition feature.</li>
 * <li>This function only can be used before mount function.</li>
 * <li>The function can be invoked once before umount partition.</li>
 * <li>Now support set single partition,the partition information will be replaced if it used for set another partition name.</li>
 * <li>The function has no effert if virtual partition information is already in the partition.</li>
 * </ul>
 *
 * @param  virtualinfo  [IN] Type #virpartinfo   The struct which include virtual partition information.
 *
 * @retval #0      los_SetVirPartParam success.
 * @retval #-1    los_SetVirPartParam failed.
 *
 * @par Dependency:
 * <ul><li>fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V200R002C00
 */
int los_set_virpartparam(virpartinfo virtualinfo);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __INCLUDE_FS_FS_H */
