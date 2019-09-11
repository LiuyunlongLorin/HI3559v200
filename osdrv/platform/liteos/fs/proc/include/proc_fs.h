/************************************************************************
 * proc/fs/generic.c --- generic routines for the proc-fs
 *
 * This file contains generic proc-fs routines for handling
 * directories and files.
 *
 * Copyright (C) 1991, 1992 Linus Torvalds.
 * Copyright (C) 1997 Theodore Ts'o

 * Copyright (C) <2014-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 ************************************************************************/

/************************************************************************
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations,
 * which might include those applicable to Huawei LiteOS of U.S. and the country
 * in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in
 * compliance with such applicable export control laws and regulations.
 ************************************************************************/

 /** @defgroup procfs Procfs
  *  @ingroup filesystem
  */

#ifndef _LITEOS_PROC_FS_H
#define _LITEOS_PROC_FS_H
#include "los_config.h"
#include "sys/types.h"

#ifdef   LOSCFG_FS_PROC
#include "sys/stat.h"
#include "linux/spinlock.h"
#include "asm/atomic.h"
#include "fs/fs.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
#define OS_PROC_TEST NO

//typedef atomic_t atomic_long_t;
typedef unsigned short fmode_t;
#define MAX_NAMELEN 32

/* 64bit hashes as llseek() offset (for directories) */
#define FMODE_64BITHASH         ((fmode_t)0x400)
/* 32bit hashes as llseek() offset (for directories) */
#define FMODE_32BITHASH         ((fmode_t)0x200)
/* File is opened using open(.., 3, ..) and is writeable only for ioctls
   (specialy hack for floppy.c) */
#define FMODE_WRITE_IOCTL   ((fmode_t)0x100)
/* File is opened with O_EXCL (only set for block devices) */
#define FMODE_EXCL      ((fmode_t)0x80)
/* File is opened with O_NDELAY (only set for block devices) */
#define FMODE_NDELAY        ((fmode_t)0x40)
/* File is opened for execution with sys_execve / sys_uselib */
#define FMODE_EXEC      ((fmode_t)0x20)
/* file can be accessed using pwrite */
#define FMODE_PWRITE        ((fmode_t)0x10)
/* file can be accessed using pread */
#define FMODE_PREAD     ((fmode_t)0x8)
/* file is seekable */
#define FMODE_LSEEK     ((fmode_t)0x4)
/* file is open for writing */
#define FMODE_WRITE     ((fmode_t)0x2)
/* file is open for reading */
#define FMODE_READ      ((fmode_t)0x1)

struct proc_file;
struct proc_dir_entry;
struct proc_file_operations;

typedef int (*read_proc_t)(struct proc_file *pf, void * buf);
typedef int (*write_proc_t)(struct proc_file *pf, const char  * buf, size_t count, loff_t *ppos);

struct proc_file_operations
{
    char *name;

    /* make a  proc file */
    loff_t (*llseek) (struct proc_file *pf, loff_t offset, int whence);
    int (*open) (struct inode *inode, struct proc_file *pf);
    int (*release) (struct inode *inode, struct proc_file *pf);
    ssize_t (*read) (struct proc_file *pf, char  *buf, size_t count, loff_t *ppos);
    ssize_t (*write) (struct proc_file * pf, const char  *buf, size_t count, loff_t *ppos);
};

struct proc_dir_entry {
    mode_t mode;
    int flags;
    const struct proc_file_operations *proc_fops;
    struct proc_file *pf;
    struct proc_dir_entry *next, *parent, *subdir;
    void *data;
    read_proc_t read_proc;
    write_proc_t write_proc;
    atomic_t count;     /* open file count */
    spinlock_t pde_unload_lock;

    int namelen;
    struct proc_dir_entry *pdir_current;
    char name[32];
};

struct proc_file {

    fmode_t             f_mode;
    spinlock_t          f_lock;
    atomic_t            f_count;
    struct proc_dir_entry *pPDE;
    void                *private_data;
    unsigned long long  f_version;
    loff_t              f_pos;
    char name[32];
};

struct proc_stat
{
    mode_t  st_mode;
    struct proc_dir_entry *pPDE;
    char name[32];
};

struct proc_data
{
    ssize_t  size;
    loff_t   f_pos;
    char     buf[1];
};
#define PROCDATA(n)   (sizeof(struct proc_data) + n)

#define S_IALLUGO   (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)


//Interface for modules using proc below internal proc moudule;

/**
 * @ingroup  procfs
 * @brief create a proc node
 *
 * @par Description:
 * This API is used to create the node by 'name' and parent inode
 *
 * @attention
 * <ul>
 * <li>This interface should be called after system initialization.</li>
 * <li>The parameter name should be a valid string.</li>
 * </ul>
 *
 * @param  name [IN] Type #const char * The name of the node to be created.
 * @param  mode [IN] Type #mode_t the mode of create's node.
 * @param  parent [IN] Type #struct proc_dir_entry * the parent node of the node to be created,
 * if pass NULL, default parent node is "/proc".
 *
 * @retval #NULL                        Create failed.
 * @retval #proc_dir_entry*     Create successfully.
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V100R001C00
 */
extern struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
                        struct proc_dir_entry *parent);

/**
 * @ingroup  procfs
 * @brief remove a proc node
 *
 * @par Description:
 * This API is used to remove the node by 'name' and parent inode
 *
 * @attention
 * <ul>
 * <li>This interface should be called after system initialization.</li>
 * <li>The parameter name should be a valid string.</li>
 * </ul>
 *
 * @param  name [IN] Type #const char * The name of the node to be removed.
 * @param  parent [IN] Type #struct proc_dir_entry * the parent node of the node to be remove.
 *
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V100R001C00
 */
extern void remove_proc_entry(const char *name, struct proc_dir_entry *parent);

/**
 * @ingroup  procfs
 * @brief create a proc directory node
 *
 * @par Description:
 * This API is used to create the directory node by 'name' and parent inode
 *
 * @attention
 * <ul>
 * <li>This interface should be called after system initialization.</li>
 * <li>The parameter name should be a valid string.</li>
 * </ul>
 *
 * @param  name [IN] Type #const char * The name of the node directory to be created.
 * @param  parent [IN] Type #struct proc_dir_entry * the parent node of the directory node to be created,
 * if pass NULL, default parent node is "/proc".
 *
 * @retval #NULL               Create failed.
 * @retval #proc_dir_entry*    Create successfully.
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V100R001C00
 */
extern struct proc_dir_entry *proc_mkdir(const char * name,struct proc_dir_entry * parent);

/**
 * @ingroup  procfs
 * @brief create a proc  node
 *
 * @par Description:
 * This API is used to create the node by 'name' and parent inode,
 * And assignment operation function
 *
 * @attention
 * <ul>
 * <li>This interface should be called after system initialization.</li>
 * <li>The parameter name should be a valid string.</li>
 * </ul>
 *
 * @param  name [IN] Type #const char * The name of the node to be created.
 * @param  mode [IN] Type #mode_t the mode of create's node.
 * @param  parent [IN] Type #struct proc_dir_entry * the parent node of the node to be created.
 * @param  proc_fops [IN] Type #const struct proc_file_operations * operation function of the node.
 *
 * @retval #NULL                Create failed.
 * @retval #proc_dir_entry*    Create successfully.
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see
 *
 * @since Huawei LiteOS V100R001C00
 */
extern struct proc_dir_entry *proc_create(const char *name, mode_t mode,
    struct proc_dir_entry *parent, const struct proc_file_operations *proc_fops);

/**
 * @ingroup  procfs
 * @brief init proc fs
 *
 * @par Description:
 * This API is used to init proc fs.
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  NONE
 *
 * @retval NONE
 * @par Dependency:
 * <ul><li>proc_fs.h: the header file that contains the API declaration.</li></ul>
 * @see proc_fs_init
 *
 * @since Huawei LiteOS V100R001C00
 */
extern void proc_fs_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* LOSCFG_FS_PROC */
#endif /* _LITEOS_PROC_FS_H */
